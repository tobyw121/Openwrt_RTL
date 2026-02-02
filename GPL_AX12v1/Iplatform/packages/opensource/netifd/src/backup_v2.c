/*! Copyright(c) 2008-2014 Shenzhen TP-LINK Technologies Co.Ltd.
 *
 *\file     detect_online.c
 *\brief    Only apply to ipv4 temporarily.
 *\details
 *
 *\author   Feng JiaFa <fengjiafa@tp-link.net>
 *\version  1.0.0
 *\date     19Nov14
 *
 *\warning
 *
 *\history \arg 19Nov14, Teng Fei, create the file.
 */
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>        /* fcntl() */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip_icmp.h>
#include <uci.h>

#include "netifd.h"
#include "interface.h"
#include "interface-ip.h"
#include "proto.h"
#include "connect.h"
#include "config.h"
#include "system.h"
#include "backup_config_v2.h"
#include "usbmodem_log.h"
#include "ubus.h"

#include <arpa/inet.h>
#include <netdb.h>

#ifdef _HAVE_CARES_
#include <time.h>
#include <sys/time.h>
#include <limits.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ares.h>

typedef struct
{
    int status;
    int is_done;
    union if_addr ip;
} CALLBACK_DATA;

#endif /* _HAVE_CARES_ */ 

/* ways for online-detect */
#define    DETECT_DNS_PORT        0x35

/* invalid socket fd implies to close current socket. */
#define    DETECT_SOCK_INVALID    -1

/* args */
typedef struct
{
    uint8_t  ip_index;
    union    if_addr addr;
    uint16_t detect_icmp_seq;
    struct   uloop_fd sockfd;
    struct   timeval send_time_stamp;
    bool     recv_success;
} t_detect_data;

typedef struct
{
    const enum        network_type type;
    const uint8_t     route_table_num;
    struct timeval    domain_lookup_time;
    bool              already_add_route;
    union if_addr     detect_server_ips[DETECT_IP_MAX];
    uint8_t           detect_server_ips_count;
    struct interface *detect_if;
    t_detect_data     detect_data[DETECT_IP_MAX];
    bool              this_cycle_dns_detect_has_result;
    int               this_cycle_dns_detect_success_count;
    int               this_cycle_dns_detect_fail_count;
} t_detect_args;

typedef struct
{
    t_detect_args        detect_args;
    struct uloop_timeout detect_timer;
    struct uloop_timeout detect_timeout_timer;
} t_detect_uloop_timer;

static t_detect_uloop_timer g_detect_uloop_timer[NETWORK_TYPE_MAX] =
{
    [NETWORK_TYPE_WIRED]  = {.detect_args = {.type = NETWORK_TYPE_WIRED,  .route_table_num = 200, .already_add_route = false}},
    [NETWORK_TYPE_MOBILE] = {.detect_args = {.type = NETWORK_TYPE_MOBILE, .route_table_num = 201, .already_add_route = false}}
};

t_detect_result detect_result;
struct nw_backup_cfg curr_backup_cfg;

/**
 * @brief         获取当前的时间戳，优先使用clock_gettime获取，其次使用gettimeofday
 * @param         {timeval *} tv            时间戳指针，可以为NULL，只通过返回值获取时间戳
 * @return        {struct timeval}          返回值为时间戳，同时通过传入的指针与返回值传值
 */
static struct timeval tvnow(struct timeval * tv)
{
    /*
     * clock_gettime() is granted to be increased monotonically when the
     * monotonic clock is queried. Time starting point is unspecified, it
     * could be the system start-up time, the Epoch, or something else,
     * in any case the time starting point does not change once that the
     * system has started up.
     */
    struct timeval      now;
    struct timespec     tsnow;
    if (0 == clock_gettime(CLOCK_MONOTONIC, &tsnow))
    {
        now.tv_sec = tsnow.tv_sec;
        now.tv_usec = tsnow.tv_nsec / 1000;
    }
    else
    {
        /*
         * Even when the configure process has truly detected monotonic clock
         * availability, it might happen that it is not actually available at
         * run-time. When this occurs simply fallback to other time source.
         */
        (void)gettimeofday(&now, NULL);
    }
    if(tv)
    {
        memcpy(tv, &now, sizeof(struct timeval));
    }
    return now;
}

/**
 * @brief         获取end-start的时间差值，返回单位为微秒
 * @param         {timeval} *start 起始时间戳
 * @param         {timeval} *end   结束时间戳
 * @return        {int64_t}        时间差值，单位为微秒
 */
static int64_t difftimeval(const struct timeval *end, const struct timeval *start)
{
        int64_t d;
        time_t s;
        suseconds_t u;

        s = end->tv_sec - start->tv_sec;
        u = end->tv_usec - start->tv_usec;
        //if (u < 0)
        //        --s;

        d = s;
        d *= 1000000;//1 秒 = 10^6 微秒
        d += u;

        return d;
}

/**
 * @brief         判断一个地址是否为私有地址或环回地址
 * @param         {uint32_t} addr 传入的ipv4地址，网络字节序
 * @return        {bool}     ture为私有地址    
 */
static bool is_addr_rfc1918(uint32_t addr)
{
    return   addr == 0 ||
           ((addr >= 0x0A000000) && (addr <= 0x0AFFFFFF)) || //A类 10.0.0.0 --10.255.255.255
           ((addr >= 0xAC100000) && (addr <= 0xAC1FFFFF)) || //B类 172.16.0.0–172.31.255.255
           ((addr >= 0xC0A80000) && (addr <= 0xC0A8FFFF)) || //C类 192.168.0.0–192.168.255.255
           ((addr >= 0x7F000000) && (addr <= 0x7FFFFFFF));;  //环回地址

}

/**
 * @brief         将指定接口的iface名称复制入buf中
 * @param         {interface} *iface    接口指针
 * @param         {char*} buf           字符串指针 
 * @param         {int} buf_size        字符串最大长度
 * @return        {bool} ture成功/false失败                
 */
static bool get_ifname(struct interface *iface, char* buf, int buf_size)
{
    bool ret = false;
    if (buf == NULL)
    {
        goto exit;
    }

    memset(buf, 0, buf_size);
    if (iface->l3_dev.dev && buf_size > strlen(iface->l3_dev.dev->ifname))
    {
        strcpy(buf, iface->l3_dev.dev->ifname);
        ret = true;
    }
    else if (iface->ifname && buf_size > strlen(iface->ifname))
    {
        strcpy(buf, iface->ifname);
        ret = true;
    }

exit:
    return ret;
}

/**
 * @brief         获取指定接口的ipv4地址
 * @param         {interface} *iface           接口指针
 * @param         {union if_addr*} ip_addr     传出结果的ip指针，网络字节序
 * @return        {bool}                       ture成功/false失败
 */
static bool get_ipv4_addr(struct interface *iface, union if_addr* ip_addr)
{
    int j = 0;
    struct interface_ip_settings *ip = NULL;
    struct device_addr *addr = NULL;
    bool get_ip = false;
    
    if (iface == NULL)
    {
        goto exit;
    }
    for (ip = &iface->config_ip, j = 0; j < 0x2; ip = &iface->proto_ip, ++j)
    {
        if(get_ip)
            break;
        vlist_for_each_element(&ip->addr, addr, node) {
            if (addr->enabled != true)
                continue;
            if ((addr->flags & DEVADDR_FAMILY) != DEVADDR_INET4)
                continue;
            *ip_addr = addr->addr;
            get_ip = true;
            break;                
        }
    }
exit:
    return get_ip;
}

/**
 * @brief         获取指定接口的ipv4网关地址(作为路由)
 * @param         {interface} *iface           接口指针
 * @param         {union if_addr*} route_addr  传出网关结果的ip指针，网络字节序
 * @return        {bool}                       ture成功/false失败
 */
static bool get_ipv4_route(struct interface *iface, union if_addr* route_addr)
{
    int j = 0;
    struct interface_ip_settings *ip = NULL;
    struct device_route *route = NULL;
    bool found_dft_route = false;
    
    if (iface == NULL)
    {
        goto exit;
    }

    for (ip = &iface->proto_ip, j = 0; j < 0x2; ip = &iface->config_ip, ++j)
    {
        if(found_dft_route)
            break;

        vlist_for_each_element(&ip->route, route, node) 
        {
            if (!route->enabled)    
                continue;
            
            if ((route->flags & DEVADDR_FAMILY) != DEVADDR_INET4)
                continue;

            if (route->nexthop.in.s_addr == 0)
                continue;

            if (route->addr.in.s_addr != 0)
                continue;
            *route_addr = route->nexthop;
            found_dft_route = 1;
            break;
        }
    }
    
exit:
    return found_dft_route;
}

#ifdef _HAVE_CARES_
/**
 * @brief         通过libc-ares查询指定域名的DNS结果，收到DNS回复的回调函数
 * @param         {void *} arg          指向CALLBACK_DATA的指针(根据ares_gethostbyname调用的最后一个参数)
 * @param         {int} status          收到DNS回复的状态，回复正常为ARES_SUCCESS
 * @param         {int} timeouts        --
 * @param         {hostent *} hostent   储存了dns的回复结果
 * @return        {void}
 */
static void dns_callback(void * arg, int status, int timeouts, struct hostent * hostent) 
{
    CALLBACK_DATA *cb_res = (CALLBACK_DATA *)arg;
    char **pptr;
    char ipaddr[INET6_ADDRSTRLEN] = {0};
    struct in_addr inp;
    if (NULL == cb_res)
    {
        netifd_log_message(L_ERROR, "invalid parameter cb_res!\n");
        return;
    }
    cb_res->status = status;
    if (status != ARES_SUCCESS)
    {
        netifd_log_message(L_WARNING, "status: %d-%s, return...\n", status, ares_strerror(status));
        return;
    }
    if(hostent->h_addrtype == AF_INET)
    {
        for (pptr = hostent->h_addr_list; *pptr != NULL; pptr++)
        {
            if (!inet_ntop(hostent->h_addrtype, *pptr, ipaddr, INET6_ADDRSTRLEN - 1))
            {
                netifd_log_message(L_ERROR,"inet_ntop: %s\n", ares_strerror(status));
            }
            else
            {
                netifd_log_message(L_DEBUG,"%s\n", ipaddr);
                inet_aton(ipaddr, &inp);
                cb_res->ip.in.s_addr = inp.s_addr;
                cb_res->is_done = 1;
                return;
            }
        }        
    }
    else
    {
        netifd_log_message(L_WARNING, "Can't get ipv4 address\n");
        cb_res->ip.in.s_addr = 0;
        cb_res->is_done = 1;
        return;
    }
    cb_res->is_done = 1;
}

/**
 * @brief         通过libc-ares查询指定域名的DNS结果,具体查询实现函数
 * @param         {char} *server             指定DNS查询使用的Server
 * @param         {interface} *iface         指定DNS查询使用的网络接口
 * @param         {char} *host               指定DNS查询的域名
 * @param         {int} family               指定DNS查询的种类 -- AF_INET
 * @param         {long} timeout_ms          指定DNS查询的超时时间
 * @param         {CALLBACK_DATA} *cb_res    传入的参数指针，用于回调函数中储存结果及查询状态
 * @return        {int} 0--成功/其他失败
 */
static int dns_resolve(const char *server, struct interface *iface, const char *host, int family, long timeout_ms, CALLBACK_DATA *cb_res)
{
    ares_channel    channel;
    int             status;
    int             nfds;
    int             count;
    fd_set          read_fds, write_fds;
    struct timeval  now;
    status = ares_library_init(ARES_LIB_INIT_ALL);
    char wan_ifname[30];
    union if_addr ip_addr = {.in = {.s_addr = 0}};
    if (status != ARES_SUCCESS)
    {
        netifd_log_message(L_WARNING, "ares_library_init: %s\n", ares_strerror(status));
        return -1;
    }
    status = ares_init(&channel);
    if (status != ARES_SUCCESS)
    {
        netifd_log_message(L_WARNING, "ares_init_options: %s\n", ares_strerror(status));
        return -1;
    }
    if (server != NULL)
    {
        ares_set_servers_csv(channel, server);
    }
    if (iface != NULL)
    {    
        get_ifname(iface, wan_ifname, sizeof(wan_ifname));
        get_ipv4_addr(iface, &ip_addr);
        ares_set_local_dev(channel, wan_ifname);
        ares_set_local_ip4(channel, ntohl(ip_addr.in.s_addr));
    }
    tvnow(&now);
    ares_gethostbyname(channel, host, family, dns_callback, (void *)cb_res);
    while (1)
    {
        struct timeval *tvp, tv, store;
        store.tv_sec = timeout_ms / 1000;
        store.tv_usec = (timeout_ms % 1000) * 1000;
        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);
        nfds = ares_fds(channel, &read_fds, &write_fds); 
        if (nfds == 0)
        {
            break;
        }
        tvp = ares_timeout(channel, &store, &tv);
        count = select(nfds, &read_fds, &write_fds, NULL, tvp);
        if (count < 0)
        {
            break;
        }
        ares_process(channel, &read_fds, &write_fds);
        if(cb_res->is_done == 1)
        {
            break;
        }
        struct timeval now2 = tvnow(NULL);
        if(difftimeval(&now2, &now) > timeout_ms * 1000)
        {
            ares_cancel(channel);
            break;
        }
    }
    ares_destroy(channel);
    ares_library_cleanup();
    return cb_res->status;
}

/**
 * @brief         通过libc-ares查询指定域名的ipv4 DNS结果
 * @param         {char *} domain_name  指定DNS查询的域名
 * @param         {interface} *iface    指定DNS查询使用的接口
 * @return        {union if_addr}       查询结果，DNS返回的第一个ip，网络字节序，若失败则为0.0.0.0
 */
static union if_addr gethostbyname_libcares(char * domain_name, struct interface *iface)
{ 
    int  timeout     = curr_backup_cfg.ping_update_domain_ip_timout;
    int  ret         = 0;
    CALLBACK_DATA    cb_date; 
    char server[0x80];
    union if_addr route_addr;
    memset(&cb_date, 0, sizeof(CALLBACK_DATA));
    get_ipv4_route(iface, &route_addr);
    inet_ntop(AF_INET, &route_addr, server, sizeof(server));
    ret = dns_resolve(server, iface, domain_name, AF_INET, timeout * 1000, &cb_date);
    if (ARES_SUCCESS != ret)
    {
        netifd_log_message(L_ERROR, "resolve failed!, ret = %d\n", ret);
    }
    return cb_date.ip;
}
#endif /* _HAVE_CARES_ */

void switch_network_type(enum network_type type, struct interface *iface);
/**
 * @brief         通过ip route replace指令替换默认路由表，达到切换网络的目的；
 * @param         {enum network_type} type     待切换为的network_type；
 * @param         {interface} *iface           待切换为的network_type对应的interface
 * @return        {void}
 */
void switch_network_type(enum network_type type, struct interface *iface)
{
    char wan_ifname[30];
    char buf[0x80];
    union if_addr route_addr;
    if (iface == NULL)
    {
        return;
    }
 
    if (detect_result.current_nw != type)
    {
        detect_result.current_nw = type;
        memset(buf, 0, sizeof(buf));
        get_ipv4_route(iface, &route_addr);
        inet_ntop(AF_INET, &route_addr, buf, sizeof(buf));
        netifd_log_message(L_INFO, "switch_network_type to %d!\n", type);
        if (strlen(buf) > 0 && get_ifname(iface, wan_ifname, sizeof(wan_ifname)))
        {
            system_exec_fmt("ip route replace default via %s dev %s", buf, wan_ifname);
            interface_write_resolv_conf();
#ifdef SUPPORT_VPN_CLIENT_IN_3G4G_MODULES
            set_vpn_client_parent(iface->name);
#endif
        }
        else
        {
            /* fail to set default route */
        }
    }
    return;
}

/**
 * @brief         将指定的接口加入策略路由，这样即使匹配不到默认路由仍然有路由出口，保证可以发送ping数据包，实际调用的shell指令类似如下：
 *                ip rule add oif eth1 table 200                        (有线device)
 *                ip route add default via 192.168.99.1 table 200       (有线device对应的网关)
 *                ip rule add oif usb0 table 201                        (usb device)
 *                ip route add default via 192.168.1.1 table 201        (usb device对应的网关)
 * @param         {t_detect_args*} detect_args      里面包括
 *                detect_args->detect_if            需要添加的接口指针
 *                detect_args->already_add_route    此接口是否已经添加过策略路由(添加过就不再添加)
 *                detect_args->route_table_num      此接口的策略路由表编号
 * @return        {int} 0--成功/其他失败
 */
static int add_ip_rule_route(t_detect_args* detect_args)
{
    struct interface *iface = detect_args->detect_if;
    char wan_ifname[30];
    char buf[0x80];
    union if_addr route_addr;
    if(detect_args->already_add_route)
    {
        return 1;
    }
    if (iface == NULL)
    {
        netifd_log_message(L_ERROR, "fail to add_ip_rule_route for network %d!\n", detect_args->type);
        return -1;
    }
    get_ipv4_route(iface, &route_addr);
    inet_ntop(AF_INET, &route_addr, buf, sizeof(buf));
    if (strlen(buf) > 0 && get_ifname(iface, wan_ifname, sizeof(wan_ifname)))
    {
        system_exec_fmt("ip rule add oif %s table %d", wan_ifname, detect_args->route_table_num);
        system_exec_fmt("ip route add default via %s dev %s table %d", buf, wan_ifname, detect_args->route_table_num);
        detect_args->already_add_route = true;
    }
    else
    {
        /* fail to set default route */
        netifd_log_message(L_ERROR, "fail to add_ip_rule_route for %s!\n", wan_ifname);
        return -2;
    }
    netifd_log_message(L_INFO, "add_ip_rule_route for %s!\n", wan_ifname);
    return 0;
}

/**
 * @brief         将指定的接口从策略路由中删除，实际调用的shell指令类似如下：
 *                while ip rule del table 200 2>/dev/null; do true; done     (有线device对应的策略路由表编号 -- 直接执行/etc/init.d/network restart会导致添加多条相同的策略路由，此时使用while指令全部删除)
 *                ip route del table 200                                     (有线device对应的策略路由表编号)
 *                while ip rule del table 201 2>/dev/null; do true; done     (usb device对应的策略路由表编号 -- 直接执行/etc/init.d/network restart会导致添加多条相同的策略路由，此时使用while指令全部删除)
 *                ip route del table 201                                    (usb device对应的策略路由表编号)
 * @param         {t_detect_args*} detect_args      里面包括
 *                detect_args->already_add_route    此接口是否已经添加过策略路由(删除后置为false)
 *                detect_args->route_table_num      此接口的策略路由表编号
 * @return        {void}
 */
static void del_ip_rule_route(t_detect_args* detect_args)
{
    if(detect_args)
    {
        system_exec_fmt("while ip rule del table %d 2>/dev/null; do true; done", detect_args->route_table_num);
        system_exec_fmt("ip route del table %d", detect_args->route_table_num);
        detect_args->already_add_route = false;
        netifd_log_message(L_INFO, "del_ip_rule_route for network %d!\n", detect_args->type);    
    }
}

struct interface * target_network_up(enum network_type type);
/**
 * @brief         获取与指定neteork_type相关联，且已经up的interface
 * @param         {enum network_type} type 
 * @return        {struct interface *}与输入type对应的interface指针，若无则返回NULL
 */
struct interface * target_network_up(enum network_type type)
{
    struct interface *iface = NULL;
    union if_addr tmp_ip_addr;
    struct interface *internet_iface = NULL;
    if(type == NETWORK_TYPE_WIRED)
    {
        vlist_for_each_element(&interfaces, iface, node)
        {
            if (strcmp(iface->name, CONNECT_IFACE_INTERNET) == 0)
            {
                internet_iface = iface;
                break;
            }
        }
        if(internet_iface)
        {
            if (internet_iface->state == IFS_UP && get_ipv4_route(internet_iface, &tmp_ip_addr) && get_ipv4_addr(internet_iface, &tmp_ip_addr))
            { 
                netifd_log_message(L_INFO, "detecting network %s!\n", internet_iface->name);    
                return internet_iface;
            }
            else
            {
                return NULL;
            }
        }        
    }
    vlist_for_each_element(&interfaces, iface, node)
    {
        if (get_network_type(iface->name) != type)
        {
            continue;
        }
        if (iface->state == IFS_UP && get_ipv4_route(iface, &tmp_ip_addr) && get_ipv4_addr(iface, &tmp_ip_addr))
        { 
            netifd_log_message(L_INFO, "detecting network %s!\n", iface->name);    
            return iface;
        }
    }
    return NULL;
}

/**
 * @brief         当设置为CONN_MODE_MOBILE_ONLY或CONN_MODE_CABLE_ONLY时，处理backup相关网络
 *                直接强制将当前的网络接口置为第一优先级网络，switch_network_type中有去重措施，避免一直执行shell指令
 * @return        {void}
 */
static void backup_prefer_only()
{
    enum network_type type = curr_backup_cfg.primary_nw;
    struct interface *iface = NULL;

    if ((iface = target_network_up(type)) != NULL)
    {
        switch_network_type(type, iface);
    }

    return;
}


/**
 * @brief         按照ifreq中的IFNAMSIZ(15)将字符串拷贝
 * @param         {char} *dst      目标字符串
 * @param         {char} *src      源字符串，储存有接口名
 * @return        {char *}         返回最终复制的字符串，即dst
 */
static char * strncpy_IFNAMSIZ(char *dst, const char *src)
{
    return strncpy(dst, src, 16);
}

/**
 * @brief         使得指定的socket使用指定的接口发送(使用SO_BINDTODEVICE)
 * @param         {int} fd            指定的socket
 * @param         {char} *iface        指定的接口
 * @return        {int}                setsockopt返回的结果
 */
static int setsockopt_bindtodevice(int fd, const char *iface)
{
    struct ifreq ifr;
    strncpy_IFNAMSIZ(ifr.ifr_name, iface);
    return setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, &ifr, sizeof(ifr));
}

/**
 * @brief         使用指定的data内interface及ip发送DNS请求，请求的域名为tp-link.com
 * @param         {t_detect_data*} detect_data      里面包括
 *                detect_data->addr                 指定的DNS server
 *                detect_data->sockfd.fd            已提前创建好、绑定过device 的文件描述符
 * @return        {int}    0-发送成功，其他均为失败
 */
static int send_dns_query(t_detect_data* detect_data)
{
    struct sockaddr_in serv_addr;
    int nbytes       = 0;
    char buf[128]    = {0};
    char *ptr        = NULL;
    uint16_t one = 0;
    
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr = detect_data->addr.in;
    serv_addr.sin_port = htons(DETECT_DNS_PORT); /* 53 */

    ptr = buf;
    *((u_short *)ptr) = htons(1234);    /* id */
    ptr += 2;
    *((u_short *)ptr) = htons(0x0100);    /* flag */
    ptr += 2;
    *((u_short *)ptr) = htons(1);        /* number of questions */
    ptr += 2;
    *((u_short *)ptr) = htons(0);        /* number of answers RR */
    ptr += 2;
    *((u_short *)ptr) = htons(0);        /* number of Authority RR */
    ptr += 2;
    *((u_short *)ptr) = htons(0);        /* number of Additional RR */
    ptr += 2;

    // memcpy(ptr,"\001a\014root-servers\003net\000", 20);
    memcpy(ptr,"\007tp-link\003com\000", 13);
    ptr += 13;
    one = htons(1);
    memcpy(ptr, &one, 2);        /* query type = A */
    ptr += 2;
    memcpy(ptr, &one, 2);        /* query class = 1 (IP addr) */
    ptr += 2;

    nbytes = 29;
    
    
    if (sendto(detect_data->sockfd.fd, buf, nbytes, 0, (struct sockaddr *)&serv_addr, 
        sizeof(struct sockaddr_in)) != nbytes)
    {
        /* perror("sendto"); */
        netifd_log_message(L_INFO, "dns sendto %s failed!\n", inet_ntoa(serv_addr.sin_addr));
        close(detect_data->sockfd.fd);
        detect_data->sockfd.fd = -1;
        return -3;
    }
    return 0;
}

/**
 * @brief         DNS接收回调函数，通过uloop监听fd实现；即受到消息后进行内容解析
 *                若接收成功，计算本轮接收成功的总次数，若成功的次数达到reliability，标记为此轮成功，并修改detect_result结果
 *                若接收失败，计算本轮接收失败的总次数，若失败的次数过多，以至于本轮未回复的数据包即使都成功也无法达到reliability，标记为此轮失败，并修改detect_result结果，
 *                若接收失败，同时关闭对应的socket，下轮重新创建(避免超时错误，导致数据包在下轮探测时到达而影响下轮的探测结果)
 * @param         {uloop_fd} *u             uloop监听的结构体
 * @param         {unsigned int} events     uloop监听触发的时间
 * @return        {void}
 */
static void recv_dns_query(struct uloop_fd *u, unsigned int events)
{
    char rbuf[256]   = {0};
    char *ptmp       = NULL;
    uint16_t n = 0;
    t_detect_data *p_detect_data = container_of(u, t_detect_data, sockfd);
    t_detect_args* detect_arg = container_of(&p_detect_data[-p_detect_data->ip_index], t_detect_args, detect_data);
    netifd_log_message(L_INFO, "dns response:events=%u, detect_type=%d, ip=%s, eof=%d, error=%d, registered=%d, flags=%d!\n", events, detect_arg->type, inet_ntoa(p_detect_data->addr.in), u->eof, u->error, u->registered, u->flags);
    if (events & ULOOP_READ)
    {
        n = recvfrom(u->fd, rbuf, sizeof(rbuf), 0, (struct sockaddr *)0, (socklen_t *)0);
        if (n == -1 && (errno == EAGAIN /*|| *perrno == EAGAIN*/))
        {
            goto fail;
        }
        if (n >= 0)
        {
            /* good answer */
            /* If DNS response is longer than 256, the rbuf will be full and n will be 256, AVOID rbuf[256] */
            if (n >= 256)
            {
                rbuf[255] = 0;
            }
            else
            {
                rbuf[n] = 0;
            }
            ptmp = &rbuf[6];
            if (htons(*(uint16_t *)ptmp) >= 1) /* good we got an answer */
            {
                p_detect_data->recv_success = true;
                detect_arg->this_cycle_dns_detect_success_count++;
                netifd_log_message(L_INFO, "dns response checked! detect_type=%d, ip=%s, this cycle detect success count=%d\n", detect_arg->type, inet_ntoa(p_detect_data->addr.in), detect_arg->this_cycle_dns_detect_success_count);
                if(!detect_arg->this_cycle_dns_detect_has_result && detect_arg->this_cycle_dns_detect_success_count >= curr_backup_cfg.reliability)
                {//成功的次数达到reliability，标记为此轮成功，并修改结果
                    netifd_log_message(L_INFO, "detect_type=%d, this cycle is success!\n", detect_arg->type);
                    detect_arg->this_cycle_dns_detect_has_result = true;
                    detect_result.consecutive_success_count[detect_arg->type]++;
                    detect_result.consecutive_failed_count[detect_arg->type] = 0;
                    detect_result.recent_detect_result[detect_arg->type]     = (detect_result.recent_detect_result[detect_arg->type] << 1) | 1;
                }
                return;
            }
        }
        goto fail;
    }
fail:
    netifd_log_message(L_INFO, "detecting %d in ip %s failed!\n", detect_arg->type, inet_ntoa(p_detect_data->addr.in));
    detect_arg->this_cycle_dns_detect_fail_count++;
    p_detect_data->recv_success = false;
    if(!detect_arg->this_cycle_dns_detect_has_result && detect_arg->this_cycle_dns_detect_success_count >= curr_backup_cfg.reliability)
    {//失败次数多，已经不可能达到reliability，标记为此轮失败，并修改结果
        netifd_log_message(L_INFO, "detect_type=%d, this cycle is failed!\n", detect_arg->type);
        detect_arg->this_cycle_dns_detect_has_result = true;
        detect_result.consecutive_success_count[detect_arg->type] = 0;
        detect_result.consecutive_failed_count[detect_arg->type]++;
        detect_result.recent_detect_result[detect_arg->type] = (detect_result.recent_detect_result[detect_arg->type]) << 1;
    }    
    netifd_log_message(L_INFO, "detecting %d in ip %s socket close!\n", detect_arg->type, inet_ntoa(p_detect_data->addr.in));
    uloop_fd_delete(u);
    close(u->fd);
    u->fd = -1;
}
#define ICMP_DATA_LEN 56        //ICMP默认数据长度
#define ICMP_HEAD_LEN 8            //ICMP默认头部长度
#define ICMP_LEN  (ICMP_DATA_LEN + ICMP_HEAD_LEN)
/**
 * @brief         计算ICMP的校验和
 * @param         {icmp} *pIcmp  icmp数据包指针
 * @return        {uint16_t}     ICMP数据包校验和结果
 */
static uint16_t compute_cksum(struct icmp *pIcmp)
{
    uint16_t *data = (uint16_t *)pIcmp;
    int len = ICMP_LEN;
    uint32_t sum = 0;
    
    while (len > 1)
    {
        sum += *data++;
        len -= 2;
    }
    if (1 == len)
    {
        u_int16_t tmp = *data;
        tmp &= 0xff00;
        sum += tmp;
    }
 
    //ICMP校验和带进位
    while (sum >> 16)
        sum = (sum >> 16) + (sum & 0x0000ffff);
    sum = ~sum;
    
    return sum;
}

/**
 * @brief         使用指定的data内的interface及ip，发送ping请求
 * @param         {t_detect_data*} detect_data      里面包括
 *                detect_data->addr                 指定的ping目标ip地址
 *                detect_data->sockfd.fd            已提前创建好、绑定过device的文件描述符
 *                detect_data->detect_icmp_seq      ping目标ip地址的序列号，每次会+1
 * @return        {int}    0-发送成功，其他均为失败
 */
static int send_ping_query(t_detect_data* detect_data)
{
    int sockfd = detect_data->sockfd.fd;
    t_detect_args* detect_arg = container_of(&detect_data[-detect_data->ip_index], t_detect_args, detect_data);
    struct sockaddr_in ping_addr;
    char sndPacket[192];
    struct icmp *sndPkt = NULL;
    uint16_t n = 0;
    struct timeval *pTime;
    uint16_t pid = getpid() + detect_arg->type * DETECT_IP_MAX + detect_data->ip_index;

    memset(&ping_addr, 0x0, sizeof(struct sockaddr_in));
    ping_addr.sin_family = AF_INET;
    ping_addr.sin_addr = detect_data->addr.in;

    detect_data->detect_icmp_seq++;
    detect_data->detect_icmp_seq &= 0x0fff;

    sndPkt = (struct icmp *)sndPacket;
    memset(sndPkt, 0x0, sizeof(sndPacket));
    /* 类型和代码分别为ICMP_ECHO,0代表请求回送 */
    sndPkt->icmp_type  = ICMP_ECHO;
    sndPkt->icmp_code  = 0;
    sndPkt->icmp_cksum = 0;                                    //校验和
    sndPkt->icmp_seq   = detect_data->detect_icmp_seq;          //序号
    sndPkt->icmp_id    = pid;                                  //取进程号作为标志
    pTime              = (struct timeval *)sndPkt->icmp_data;
    tvnow(pTime);    //数据段存放发送时间
    sndPkt->icmp_cksum = compute_cksum(sndPkt);

    n = sendto(sockfd, sndPkt, ICMP_LEN, 0x0,     (struct sockaddr *)&ping_addr, sizeof(struct sockaddr_in));
    if (n < 0)
    {
        netifd_log_message(L_WARNING, "ping request send failed! detect_type=%d, ip=%s!\n", detect_arg->type, inet_ntoa(detect_data->addr.in));
        return -2;
    }
    netifd_log_message(L_INFO, "ping request send socket=%d! detect_type=%d, ip=%s, seq=%hd, id=%hd!\n", sockfd, detect_arg->type, inet_ntoa(detect_data->addr.in), sndPkt->icmp_seq, sndPkt->icmp_id);
    return 0;
}
/**
 * @brief         ping接收回调函数，通过uloop监听fd实现；即受到消息后进行内容解析
 *                若接收成功，计算本轮接收成功的总次数，若成功的次数达到reliability，标记为此轮成功，并修改detect_result结果
 *                若接收失败，不会计入本轮接收失败的次数，原因是其他程序的ICMP包也会触发此callback，因此不能处理
 *                由于发送/接收超时并不会进uloop的callback，在另一个线程中单独处理超时
 * @param         {uloop_fd} *u             uloop监听的结构体
 * @param         {unsigned int} events     uloop监听触发的时间
 * @return        {void}
 */
static void recv_ping_query(struct uloop_fd *u, unsigned int events)
{
    char rbuf[256]   = {0};
    struct icmp *rcvPkt = NULL;
    struct sockaddr_in peer_addr;
    int addr_len = sizeof(peer_addr);
    uint16_t n = 0;
    t_detect_data *p_detect_data = container_of(u, t_detect_data, sockfd); 
    t_detect_args* detect_arg = container_of(&p_detect_data[-p_detect_data->ip_index], t_detect_args, detect_data);
    uint16_t pid = getpid();
    uint16_t ip_index = 0;
    memset(&peer_addr, 0x0, sizeof(struct sockaddr_in));
    netifd_log_message(L_INFO, "ping response:events=%u, detect_type=%d!\n", events, detect_arg->type);
    if (events & ULOOP_READ)
    {
        n = recvfrom(u->fd, rbuf, sizeof(rbuf), 0, (struct sockaddr *)&peer_addr, (socklen_t *)&addr_len);
        if (n == -1 && (errno == EAGAIN /*|| *perrno == EAGAIN*/))
        {
            goto fail;
        }
        else if (n >= 76)
        {
            struct iphdr *iphdr = (struct iphdr *)rbuf;            /* ip + icmp */
            rcvPkt = (struct icmp *)(rbuf + (iphdr->ihl << 2));    /* skip ip hdr */ 
            if(rcvPkt->icmp_type != ICMP_ECHOREPLY)
            {
                goto fail;
            }
            ip_index = rcvPkt->icmp_id - pid - detect_arg->type * DETECT_IP_MAX;
            if(ip_index >= DETECT_IP_MAX)
            {//其他应用程序发出的ping包
                goto fail;
            }
            p_detect_data = &detect_arg->detect_data[ip_index];
            if (rcvPkt->icmp_seq == p_detect_data->detect_icmp_seq)
            {
                p_detect_data->recv_success = true;
                detect_arg->this_cycle_dns_detect_success_count++;
                netifd_log_message(L_INFO, "ping response checked socket=%d! detect_type=%d, ip=%s, seq=%hd, id=%hd, this cycle detect success count=%d\n", u->fd, detect_arg->type, inet_ntoa(peer_addr.sin_addr), rcvPkt->icmp_seq, rcvPkt->icmp_id, detect_arg->this_cycle_dns_detect_success_count);
                if(!detect_arg->this_cycle_dns_detect_has_result && detect_arg->this_cycle_dns_detect_success_count >= curr_backup_cfg.reliability)
                {//成功的次数达到reliability，标记为此轮成功，并修改结果
                    netifd_log_message(L_INFO, "detect_type=%d, this cycle is success!\n", detect_arg->type);
                    detect_arg->this_cycle_dns_detect_has_result = true;
                    detect_result.consecutive_success_count[detect_arg->type]++;
                    detect_result.consecutive_failed_count[detect_arg->type] = 0;
                    detect_result.recent_detect_result[detect_arg->type]     = (detect_result.recent_detect_result[detect_arg->type] << 1) | 1;
                }
                return;
            }
            else if (((p_detect_data->detect_icmp_seq + 0x1000 - rcvPkt->icmp_seq) & 0x0FFF) <= 2)
            {//收到的ping包是之前发出的(之前两轮内)，此时不标记成功也不标记失败
                return;
            }
            else
            {
                goto fail;
            }
        }
    }

fail:
//发送超时并不会进uloop的callback，此段程序没有必要，同时其他引用程序的ICMP包也会触发此callback，因此只计成功次数；避免由于其他程序导致失败次数误增加；timeouti在detect_timeout_check中执行
//    netifd_log_message(L_INFO, "detecting %d in ip %s failed! socket=%d!\n", detect_arg->type, inet_ntoa(peer_addr.sin_addr), u->fd);
//    detect_arg->this_cycle_dns_detect_fail_count++;
//    p_detect_data->recv_success = false;
//    if(!detect_arg->this_cycle_dns_detect_has_result && detect_arg->this_cycle_dns_detect_success_count >= curr_backup_cfg.reliability)
//    {//失败次数多，已经不可能达到reliability，标记为此轮失败，并修改结果
//        netifd_log_message(L_INFO, "detect_type=%d, this cycle is failed!\n", detect_arg->type);
//        detect_arg->this_cycle_dns_detect_has_result = true;
//        detect_result.consecutive_success_count[detect_arg->type] = 0;
//        detect_result.consecutive_failed_count[detect_arg->type]++;
//        detect_result.recent_detect_result[detect_arg->type] = (detect_result.recent_detect_result[detect_arg->type]) << 1;
//    }
    return;
}

/**
 * @brief         使用DNS方法探测时，对某一接口每轮探测调用的函数
 *                首先判断上轮有没有出结果，没有的话说明有的请求超时了还没进callback(都进下一轮探测了还没收到回复肯定超时了)，标记为上轮失败，并修改结果--理论上不该进这个判断(有detect_timeout_timer进行处理)
 *                接着初始化detect_data，除了上一回收到有效回复的；其他出错/未收到的统一关闭socket,重新初始化socket，绑定接口等
 *                其次挨个给此接口的每个dns server发送dns报文
 *                最后打开uloop_timer，在指定的超时时间结束后调用detect_timeout_timer，统一处理DNS的超时报文
 * @param         {t_detect_args*} detect_arg
 * @return        {void}
 */
static void detect_all_dns_query(t_detect_args* detect_arg)
{
    int i = 0;
    char rbuf[128]    = {0};
    struct sockaddr_in cli_addr;
    struct timeval tv;
    int opt_old = 0;
    int fl = 0;
    t_detect_data* detect_data = detect_arg->detect_data;
    t_detect_uloop_timer *detect_uloop_timer = container_of(detect_arg, t_detect_uloop_timer, detect_args);
    if(detect_arg->this_cycle_dns_detect_has_result == false)
    {//进入callback的次数不够，标记为上轮失败，并修改结果
        netifd_log_message(L_INFO, "No enough recv callback detecting %d, mark the previous round as a failure, modify the results\n", detect_arg->type);
        detect_result.consecutive_success_count[detect_arg->type] = 0;
        detect_result.consecutive_failed_count[detect_arg->type]++;
        detect_result.recent_detect_result[detect_arg->type] = (detect_result.recent_detect_result[detect_arg->type]) << 1;
    }
    detect_arg->this_cycle_dns_detect_has_result = false;
    detect_arg->this_cycle_dns_detect_fail_count = 0;
    detect_arg->this_cycle_dns_detect_success_count = 0;
    //初始化detect_data
    for(i = 0; i < detect_arg->detect_server_ips_count; i++)
    {
        if(detect_data[i].recv_success == false && detect_data[i].sockfd.fd > 0)
        {//上一回的还没收到有效回复,收到超时的有效恢复已经在接收函数中做了以下处理
            netifd_log_message(L_INFO, "detecting %d in ip %s Not received a valid reply!\n", detect_arg->type, inet_ntoa(detect_data[i].addr.in));
            uloop_fd_delete(&detect_data[i].sockfd);
            netifd_log_message(L_INFO, "detecting %d in ip %s socket close!\n", detect_arg->type, inet_ntoa(detect_data[i].addr.in));
            close(detect_data[i].sockfd.fd);
            detect_data[i].sockfd.fd = -1;
        }
        else if(detect_data[i].recv_success && detect_data[i].sockfd.fd > 0)
        {//上一回的收到了有效回复
            detect_data[i].recv_success = false;
            continue;
        }
        //重新初始化socket
        detect_data[i].ip_index = i;
        detect_data[i].addr = detect_arg->detect_server_ips[i];
        detect_data[i].recv_success = false;
        detect_data[i].sockfd.cb = recv_dns_query;
        netifd_log_message(L_INFO, "detecting %d in ip %s is in initial!\n", detect_arg->type, inet_ntoa(detect_data[i].addr.in));
        if ((detect_data[i].sockfd.fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        {
            detect_data[i].sockfd.fd = -1;
            netifd_log_message(L_ERROR, "detect_all_dns_query create socket %d fail!\n", i);
            continue;
        }

        if (get_ifname(detect_arg->detect_if, rbuf, sizeof(rbuf)) == false)
        {
            close(detect_data[i].sockfd.fd);
            netifd_log_message(L_ERROR, "detect_all_dns_query get_ifname %d fail!\n", i);
            detect_data[i].sockfd.fd = -1;
            continue;
        }
        if (setsockopt_bindtodevice(detect_data[i].sockfd.fd, rbuf) < 0)
        {
            close(detect_data[i].sockfd.fd);
            netifd_log_message(L_ERROR, "detect_all_dns_query bindtodevice %d fail!\n", i);
            detect_data[i].sockfd.fd = -1;
            continue;
        }
        if (setsockopt(detect_data[i].sockfd.fd, SOL_SOCKET, SO_REUSEADDR, &opt_old, sizeof(opt_old)) == -1) {
            close(detect_data[i].sockfd.fd);
            netifd_log_message(L_ERROR, "Failed to set socket %d option 'SO_REUSE\n", i);
            detect_data[i].sockfd.fd = -1;
            continue;
        }
        tv.tv_sec = PING_SEND_TIMEOUT_DEFAULT;
        tv.tv_usec =  0;
        setsockopt(detect_data[i].sockfd.fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)); 
        tv.tv_sec = curr_backup_cfg.ping_timeout;
        tv.tv_usec =  0;
        setsockopt(detect_data[i].sockfd.fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)); 
        /* make fd non-blocking */
        /* actually, uloop_fd_add will also do this */
        fl = fcntl(detect_data[i].sockfd.fd, F_GETFL, 0);
        fl |= O_NONBLOCK;
        fcntl(detect_data[i].sockfd.fd, F_SETFL, fl);

        memset(&cli_addr, 0, sizeof(cli_addr));
        cli_addr.sin_family = AF_INET;
        cli_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        cli_addr.sin_port = htons(0);
        if (bind(detect_data[i].sockfd.fd, (struct sockaddr *)&cli_addr, sizeof(cli_addr)) <0)
        {
            close(detect_data[i].sockfd.fd);
            netifd_log_message(L_ERROR, "detect_all_dns_query bind socket %d fail!\n", i);
            detect_data[i].sockfd.fd = -1;
            continue;
        }
    }
    for(i = 0; i < detect_arg->detect_server_ips_count; i++)
    {
        if(detect_data[i].sockfd.fd > 0)
        {
            tvnow(&detect_data[i].send_time_stamp);
            uloop_fd_delete(&detect_data[i].sockfd);
            if(send_dns_query(&detect_data[i]) == 0)
            {
                uloop_fd_add(&detect_data[i].sockfd, ULOOP_READ | ULOOP_BLOCKING | ULOOP_ERROR_CB);                
            }
        }
    }
    uloop_timeout_set(&detect_uloop_timer->detect_timeout_timer, curr_backup_cfg.ping_timeout * 1000);
}

/**
 * @brief         使用ping方法探测时，对某一接口每轮探测调用的函数
 *                首先判断上轮有没有出结果，没有的话说明有的请求超时了还没进callback(都进下一轮探测了还没收到回复肯定超时了)，标记为上轮失败，并修改结果--理论上不该进这个判断(有detect_timeout_timer进行处理)
 *                接着初始化detect_data，此socket只在backup功能开启/刚切换至ping探测方法时会初始化；由于此socket的device、源ip等相同，因此对于同一接口使用同一socket给不同的ip发ping指令，即使失败也不关闭socket
 *                其次判断需要给几个ip发送ping(因为有可能指定的是域名，未获得有效ip)，少到以至于这几个ip全部成功也小于reliability，拿就没必要发送无谓的数据包了，直接判定为此轮失败
 *                然后给每个ip发送ping报文
 *                最后打开uloop_timer，在指定的超时时间结束后调用detect_timeout_timer，统一处理超时未收到回复的ping
 * @param         {t_detect_args*} detect_arg
 * @return        {void}
 */
static void detect_all_ping_query(t_detect_args* detect_arg)
{
    int i = 0;
    char rbuf[128] = {0};
    struct timeval tv;
    int opt_old = 0;
    int fl = 0;
    t_detect_data* detect_data = detect_arg->detect_data;
    t_detect_uloop_timer *detect_uloop_timer = container_of(detect_arg, t_detect_uloop_timer, detect_args);
    if(detect_arg->this_cycle_dns_detect_has_result == false)
    {//进入callback的次数不够，标记为上轮失败，并修改结果
        netifd_log_message(L_INFO, "No enough recv callback detecting %d, mark the previous round as a failure, modify the results\n", detect_arg->type);
        detect_result.consecutive_success_count[detect_arg->type] = 0;
        detect_result.consecutive_failed_count[detect_arg->type]++;
        detect_result.recent_detect_result[detect_arg->type] = (detect_result.recent_detect_result[detect_arg->type]) << 1;
    }
    detect_arg->this_cycle_dns_detect_has_result = false;
    detect_arg->this_cycle_dns_detect_fail_count = 0;
    detect_arg->this_cycle_dns_detect_success_count = 0;
    //初始化detect_data
    if(detect_data[0].sockfd.fd <= 0)
    {
        detect_data[0].sockfd.fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);    
        if(detect_data[0].sockfd.fd <= 0)
        {
            detect_data[0].sockfd.fd = -1;
            netifd_log_message(L_ERROR, "detect_all_ping_query create socket fail!\n");
            return;
        }
        if (get_ifname(detect_arg->detect_if, rbuf, sizeof(rbuf)) == false)
        {
            close(detect_data[0].sockfd.fd);
            netifd_log_message(L_ERROR, "detect_all_ping_query get_ifname fail!\n");
            detect_data[0].sockfd.fd = -1;
            return;
        }
        if (setsockopt_bindtodevice(detect_data[0].sockfd.fd, rbuf) < 0)
        {
            close(detect_data[0].sockfd.fd);
            netifd_log_message(L_ERROR, "detect_all_ping_query bindtodevice fail!\n");
            detect_data[0].sockfd.fd = -1;
            return;
        }
        
        if (setsockopt(detect_data[0].sockfd.fd, SOL_SOCKET, SO_REUSEADDR, &opt_old, sizeof(opt_old)) == -1) {
            close(detect_data[0].sockfd.fd);
            netifd_log_message(L_ERROR, "Failed to set socket %d option 'SO_REUSE\n", i);
            detect_data[0].sockfd.fd = -1;
            return;
        }
        tv.tv_sec = PING_SEND_TIMEOUT_DEFAULT;
        tv.tv_usec =  0;
        setsockopt(detect_data[0].sockfd.fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)); 
        tv.tv_sec = curr_backup_cfg.ping_timeout;
        tv.tv_usec =  0;
        setsockopt(detect_data[0].sockfd.fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)); 
        /* make fd non-blocking */
        /* actually, uloop_fd_add will also do this */
        fl = fcntl(detect_data[0].sockfd.fd, F_GETFL, 0);
        fl |= O_NONBLOCK;
        fcntl(detect_data[0].sockfd.fd, F_SETFL, fl);

        for(i = 0; i < curr_backup_cfg.detect_ping_ips_count + curr_backup_cfg.detect_ping_domain_count; i++)
        {//初始化socket
            detect_data[i].ip_index = i;
            detect_data[i].sockfd.fd      = detect_data[0].sockfd.fd;
            detect_data[i].addr           = detect_arg->detect_server_ips[i];
            detect_data[i].detect_icmp_seq = rand() & 0x0FFF;
            detect_data[i].recv_success   = false;
            detect_data[i].sockfd.cb = recv_ping_query;
            netifd_log_message(L_INFO, "detecting %d in ip %s is in initial!\n", detect_arg->type, inet_ntoa(detect_data[i].addr.in));
        }
        netifd_log_message(L_INFO, "detecting %d create socket:%d!\n", detect_arg->type, detect_data[0].sockfd.fd);
    }
    if(detect_arg->detect_server_ips_count < curr_backup_cfg.reliability)
    {//获得的ip太少，不可能探测成功
        netifd_log_message(L_INFO, "Skip this cycle detecting, no enough ip, mark the result as fail!\n");
        detect_result.consecutive_success_count[detect_arg->type] = 0;
        detect_result.consecutive_failed_count[detect_arg->type]++;
        detect_result.recent_detect_result[detect_arg->type] = (detect_result.recent_detect_result[detect_arg->type]) << 1;
        detect_arg->this_cycle_dns_detect_has_result = true;
        goto end;
    }
    if(detect_data[0].sockfd.fd > 0)
    {
        uloop_fd_delete(&detect_data[0].sockfd);
    }
    for(i = 0; i < detect_arg->detect_server_ips_count; i++)
    {
        tvnow(&detect_data[i].send_time_stamp);
        if(detect_data[i].sockfd.fd > 0 && detect_data[i].addr.in.s_addr != 0)
        {
            send_ping_query(&detect_data[i]);
        }
    }
    if(detect_data[0].sockfd.fd > 0)
    {
        uloop_fd_add(&detect_data[0].sockfd, ULOOP_READ | ULOOP_BLOCKING | ULOOP_ERROR_CB);
    }
end:
    uloop_timeout_set(&detect_uloop_timer->detect_timeout_timer, curr_backup_cfg.ping_timeout * 1000);
    
}


/**
 * @brief         uloop定时回调函数，主timer-1，用于做决断；若当前没有网络，则按优先级遍历看哪个接口有网并切换；若当前有网，则遍历更高的优先级看哪个接口有网并切换
 * @param         {uloop_timeout} *timeout   uloop_timeout结构体指针
 * @return        {void}
 */
static void switch_network_judge_loop(struct uloop_timeout *timeout)
{
    int i = 0;
    struct interface *iface = NULL;
    netifd_log_message(L_INFO, "detect_result.is_connect = {%d,%d}\n", detect_result.is_connect[1], detect_result.is_connect[2]);
    netifd_log_message(L_INFO, "detect_result.recent_detect_result = {0x%016llx,0x%016llx}\n", detect_result.recent_detect_result[1], detect_result.recent_detect_result[2]);
    netifd_log_message(L_INFO, "detect_result.consecutive_success_count = {%u,%u}\n", detect_result.consecutive_success_count[1], detect_result.consecutive_success_count[2]);
    netifd_log_message(L_INFO, "detect_result.consecutive_failed_count = {%u,%u}\n", detect_result.consecutive_failed_count[1], detect_result.consecutive_failed_count[2]);
    netifd_log_message(L_INFO, "current_nw = %d\n", detect_result.current_nw);
    //如果有设置的mode为CONN_MODE_CABLE_ONLY或CONN_MODE_MOBILE_ONLY，无需持续检测其在线情况
    
    if (curr_backup_cfg.mode == CONN_MODE_CABLE_ONLY || curr_backup_cfg.mode == CONN_MODE_MOBILE_ONLY)
    {
        backup_prefer_only();
        goto go_next;
    }
    for(i = NETWORK_TYPE_WIRED; i < NETWORK_TYPE_MAX; ++i)
    {
        if(!detect_result.is_connect[i] && detect_result.consecutive_success_count[i] >= curr_backup_cfg.change_to_connect_threshold)
        {
            netifd_log_message(L_INFO, "Network %d change to online!\n", i);
            detect_result.is_connect[i] = true;
        }
        else if(detect_result.is_connect[i] && detect_result.consecutive_failed_count[i] >= curr_backup_cfg.change_to_disconnect_threshold)
        {
            netifd_log_message(L_INFO, "Network %d change to offline!\n", i);
            detect_result.is_connect[i] = false;
        }
    }
    if(detect_result.current_nw == NETWORK_TYPE_NONE)
    {
        if(detect_result.consecutive_success_count[curr_backup_cfg.primary_nw] >= 1)
        {
            netifd_log_message(L_INFO, "Network First time switch to primary_nw!\n");
            iface = target_network_up(curr_backup_cfg.primary_nw);
            if (iface)
            {
                switch_network_type(curr_backup_cfg.primary_nw, iface);
            }
            detect_result.is_connect[curr_backup_cfg.primary_nw] = 1;
        }
        else if(detect_result.consecutive_success_count[curr_backup_cfg.secondary_nw] >= 1)
        {
            netifd_log_message(L_INFO, "Network First time switch to secondary_nw!\n");
            iface = target_network_up(curr_backup_cfg.secondary_nw);
            if (iface)
            {
                switch_network_type(curr_backup_cfg.secondary_nw, iface);
            }
            detect_result.is_connect[curr_backup_cfg.secondary_nw] = 1;
        }
        goto go_next;
    }
    if(!detect_result.is_connect[detect_result.current_nw])
    {//若当前没有网
        if(detect_result.is_connect[curr_backup_cfg.primary_nw])
        {
            netifd_log_message(L_INFO, "Network switch to primary_nw due to current_nw offline!\n");
            iface = target_network_up(curr_backup_cfg.primary_nw);
            if (iface)
            {
                switch_network_type(curr_backup_cfg.primary_nw, iface);
            }
        }
        else if(detect_result.is_connect[curr_backup_cfg.secondary_nw])
        {
            netifd_log_message(L_INFO, "Network switch to secondary_nw due to current_nw offline!\n");
            iface = target_network_up(curr_backup_cfg.secondary_nw);
            if (iface)
            {
                switch_network_type(curr_backup_cfg.secondary_nw, iface);
            }
        }
        else
        {//若所有优先级都没有网，只要有一个包成功探测就切换到那个网络
            if(detect_result.consecutive_success_count[curr_backup_cfg.primary_nw] >= 1)
            {
                netifd_log_message(L_INFO, "Network switch to primary_nw due to all network offline!\n");
                iface = target_network_up(curr_backup_cfg.primary_nw);
                if (iface)
                {
                    switch_network_type(curr_backup_cfg.primary_nw, iface);
                }
                detect_result.is_connect[curr_backup_cfg.primary_nw] = 1;
            }
            else if(detect_result.consecutive_success_count[curr_backup_cfg.secondary_nw] >= 1)
            {
                netifd_log_message(L_INFO, "Network switch to secondary_nw due to all network offline!\n");
                iface = target_network_up(curr_backup_cfg.secondary_nw);
                if (iface)
                {
                    switch_network_type(curr_backup_cfg.secondary_nw, iface);
                }
                detect_result.is_connect[curr_backup_cfg.secondary_nw] = 1;
            }
        }
        goto go_next;
    }
    else if(detect_result.is_connect[curr_backup_cfg.primary_nw] && detect_result.current_nw != curr_backup_cfg.primary_nw)
    {//若当前有网，检测第一优先级是否也有网
        netifd_log_message(L_INFO, "Network switch to primary_nw due to primary_nw online!\n");
        iface = target_network_up(curr_backup_cfg.primary_nw);
        if (iface)
        {
            switch_network_type(curr_backup_cfg.primary_nw, iface);
        }
        goto go_next;
    }
go_next:
    (void)uloop_timeout_set(timeout, 1000);                
    return ;
}

/**
 * @brief         uloop定时回调函数，timer-4&5(每个接口独享一个timer)，在每轮循环向所有的ip发送ping/dns包之后，在指定的ping_timeout(默认2s)后进入，判断此轮的结果
 * @param         {uloop_timeout} *timeout     uloop_timeout结构体指针
 * @return        {*}
 */
static void detect_timeout_check(struct uloop_timeout *timeout)
{
    t_detect_uloop_timer *detect_uloop_timer = container_of(timeout, t_detect_uloop_timer, detect_timeout_timer);
    t_detect_args* detect_arg = &detect_uloop_timer->detect_args;
    if(detect_arg->this_cycle_dns_detect_has_result == false)
    {//timeout时还未获得有效结果，说明在timeouthi贱内进入callback的次数不够，标记为此轮失败，并修改结果
    //后续一段时间还有可能收到正确回复，但是此处已经出了结果，因此此轮的探测结果仍然是正确的；此外DNS的探测后续若收到正确回复，可以避免再次创建socket
        netifd_log_message(L_INFO, "Timeout!! No enough recv callback detecting %d, mark the result as fail\n", detect_arg->type);
        detect_result.consecutive_success_count[detect_arg->type] = 0;
        detect_result.consecutive_failed_count[detect_arg->type]++;
        detect_result.recent_detect_result[detect_arg->type] = (detect_result.recent_detect_result[detect_arg->type]) << 1;
        detect_arg->this_cycle_dns_detect_has_result = true;
    }
}

/**
 * @brief         uloop定时回调函数，timer-2&3(每个接口独享一个timer)，为wan及mobile接口进行每轮循环的查询，每轮循环向所有的ip发送ping/dns包；detect周期由track_interval决定
 *                如果此接口非up状态，直接将其connect状态置为false，等待下一个detect周期
 *                添加接口对应的策略路由表，
 *                对于dns探测，每此进入都会提取接口对应的dns server(最多DETECT_DNS_IP_MAX=2个)，等待一会儿向这些ip发送dns查询报文
 *                对于ping探测，需要对域名进行DNS解析，之后才可以ping；加入了冷却机制，不同情况下dns的查询速率不同
 *                    判定为接口有网最少需要n个ip成功，但是此时设置的ip+域名解析出的公网ip<n，此时不再发送发送无谓的icmp报文，直接判定为失败；同时每轮查询中都去查域名的DNS
 *                    设置的ip+域名数为m，设置的ip+域名解析出的公网ip<m，此时每60s（可配置项）才去查域名的DNS
 *                    设置的ip+域名数为m，设置的ip+域名解析出的公网ip==m，此时每3600s（可配置项）才去查域名的DNS
 *                注：某些(华为)的USB modem在断网(未断开USB连接)时，会劫持DNS，导致所有域名均返回其局域网网关地址：
 *                    使用库函数gethostbyname进行DNS查询，会通过默认网关及其DNS进行查询，导致如果在使用USB modem时断网，此时默认路由还是USB modem，wan/mobile的DNS接口均返回局域网网关地址，一定可以ping通
 *                解决方案: 过滤局域网ip，获取到局域网ip等同于为获取DNS失败
 *                          使用c-ares库进行DNS请求，配置发送接口及对应接口的DNS服务器，每个接口通过自己查DNS(在这种情况下，有线的内网拿tp-link.com会返回172开头的局域网地址而导致此域名一直失败，而usb接口就可以通过公网正常ping通，表现符合)、
 *                最后调用dns/ping的探测函数
 * @param         {uloop_timeout} *timeout     uloop_timeout结构体指针
 * @return        {void}
 */
static void detect_loop(struct uloop_timeout *timeout)
{
    int period = 0;
    struct interface_ip_settings *ip = NULL;
    struct dns_server *dns = NULL;
    char buf[0x80];
    char new_wan_ifname[30] = {0};
    int i = 0;
    bool same_ip = false;  
    struct interface *new_detect_if = NULL;
    t_detect_uloop_timer *detect_uloop_timer = container_of(timeout, t_detect_uloop_timer, detect_timer);
    t_detect_args* detect_arg = &detect_uloop_timer->detect_args;
    struct timeval begin_time,end_time;
    netifd_log_message(L_INFO, "detect_loop %d begin!\n", detect_arg->type);
    tvnow(&begin_time);
    //如果此接口非up状态，直接将其connect状态置为false
    new_detect_if = target_network_up(detect_arg->type);
    if(detect_arg->detect_if == NULL || new_detect_if == NULL)
    {
        detect_arg->detect_if = new_detect_if;
    }
    if(new_detect_if != detect_arg->detect_if)
    {   
        get_ifname(new_detect_if, new_wan_ifname, sizeof(new_wan_ifname));
        if(1)
        {
            netifd_log_message(L_NOTICE, "Network %d device has changed to %s! Renew the ip table %d\n", detect_arg->type, new_wan_ifname, detect_arg->route_table_num);
            if(detect_arg->already_add_route)
            {//删除所有相关的route，一会儿重新添加
                del_ip_rule_route(detect_arg);
            }
            if(detect_arg->detect_data[0].sockfd.fd > 0)
            {
                uloop_fd_delete(&detect_arg->detect_data[0].sockfd);
                close(detect_arg->detect_data[0].sockfd.fd);
                detect_arg->detect_data[0].sockfd.fd = -1;
            }
        }
        detect_arg->detect_if = new_detect_if;
    }
    if(!detect_arg->detect_if)
    {
        netifd_log_message(L_INFO, "Network %d not up!\n", detect_arg->type);
        detect_result.is_connect[detect_arg->type] = false;
        detect_result.consecutive_success_count[detect_arg->type] = 0;
        if(detect_arg->already_add_route)
        {//接口kown掉时会删除所有相关的route，再次up时会导致自己添加的规则找不到了
            del_ip_rule_route(detect_arg);
        }
        if(detect_arg->detect_data[0].sockfd.fd > 0)
        {
            uloop_fd_delete(&detect_arg->detect_data[0].sockfd);
            close(detect_arg->detect_data[0].sockfd.fd);
            detect_arg->detect_data[0].sockfd.fd = -1;
            //立即开始switch network
            uloop_timeout_set(&g_detect_uloop_timer[0].detect_timer, 0);
        }
        goto go_next;
    }
    add_ip_rule_route(detect_arg);
    //如果有设置的mode为CONN_MODE_CABLE_ONLY或CONN_MODE_MOBILE_ONLY，无需持续检测其在线情况
    if (curr_backup_cfg.mode == CONN_MODE_CABLE_ONLY || curr_backup_cfg.mode == CONN_MODE_MOBILE_ONLY)
    {
        goto go_next;
    }
    if(curr_backup_cfg.way == DETECT_WAY_DNS)
    {//Fill dns server ip
        if (detect_arg->detect_if->proto_handler && strcmp(detect_arg->detect_if->proto_handler->name, "static") == 0)
        {
            ip = &detect_arg->detect_if->config_ip;
        }
        else
        {
            ip = &detect_arg->detect_if->proto_ip;
            if (ip->no_dns)
            {
                ip = &detect_arg->detect_if->config_ip;
                if (ip->no_dns)
                {
                    netifd_log_message(L_WARNING, "Network %d no available dns!\n", detect_arg->type);
                    goto go_next;
                }
            }
        }
        detect_arg->detect_server_ips_count = 0;
        memset(buf, 0, sizeof(buf));
        vlist_simple_for_each_element(&ip->dns_servers, dns, node) 
        {
            same_ip = false;
            if (dns->af != AF_INET || dns->addr.in.s_addr == 0)
            {
                continue;
            }
            for(i = 0; i < detect_arg->detect_server_ips_count; i++)
            {//避免添加重复相同的IP
                if(dns->addr.in.s_addr == detect_arg->detect_server_ips[i].in.s_addr)
                {
                    netifd_log_message(L_INFO, "Skip the same dns ip %s for detect type %d!\n", inet_ntoa(dns->addr.in), detect_arg->type);
                    same_ip = true;
                    break;
                }
            }
            if(same_ip)
            {
                continue;
            }
            detect_arg->detect_server_ips[detect_arg->detect_server_ips_count++] = dns->addr;    
            if (i == DETECT_DNS_IP_MAX)
            {
                break;
            }
        }
        netifd_log_message(L_INFO, "Network %d get %d dns ip!\n", detect_arg->type, detect_arg->detect_server_ips_count);
    }
    else //DETECT_WAY_PING
    {    
        //如果获得的ip太少，不可能探测成功，这种情况每轮都需要查询DNS
        if(detect_arg->detect_server_ips_count < curr_backup_cfg.reliability || 
            difftimeval(&begin_time, &detect_arg->domain_lookup_time) >= ((int64_t)((detect_arg->detect_server_ips_count != curr_backup_cfg.detect_ping_ips_count + curr_backup_cfg.detect_ping_domain_count) ? curr_backup_cfg.fast_update_domain_ip_peroid : curr_backup_cfg.update_domain_ip_peroid) * (int64_t)1000000))
        {
//            int next_timeout_time[NETWORK_TYPE_MAX] = {0};
//            for(i = NETWORK_TYPE_WIRED; i < NETWORK_TYPE_MAX; ++i)
//            {//此步骤较浪费时间，由于uloop是串行，因此此处延长各个timeout回调函数的时间
//                next_timeout_time[i] = uloop_timeout_remaining(&g_detect_uloop_timer[i].detect_timeout_timer);
//                if(next_timeout_time[i] > 0)
//                {//先按照最大时间延长
//                    uloop_timeout_set(&g_detect_uloop_timer[i].detect_timeout_timer, next_timeout_time[i] + PING_DIMAIN_DNS_TIMEOUT_DEFAULT * 1000);
//                }
//            }
            detect_arg->detect_server_ips_count = curr_backup_cfg.detect_ping_ips_count;
            memset(detect_arg->detect_server_ips, 0, sizeof(detect_arg->detect_server_ips));
            for(i = 0; i < curr_backup_cfg.detect_ping_ips_count; i++)
            {
                detect_arg->detect_server_ips[i] = curr_backup_cfg.detect_ping_ips[i];
            }
            netifd_log_message(L_INFO, "Start to lookup the domains!\n");
            for(i = 0; i < curr_backup_cfg.detect_ping_domain_count; i++)
            {
                union if_addr ip;
#ifdef _HAVE_CARES_
                ip = gethostbyname_libcares(curr_backup_cfg.detect_ping_ori_domain[i], detect_arg->detect_if);
                if (ip.in.s_addr == 0)
                {
                    netifd_log_message(L_WARNING, "can't parse the domain as ipv4:%s!\n", curr_backup_cfg.detect_ping_ori_domain[i]);
                    continue;
                }
#else
                struct hostent * pHost = NULL;  
                if ((pHost = gethostbyname(curr_backup_cfg.detect_ping_ori_domain[i])) == NULL || pHost->h_addr_list == NULL || pHost->h_addr_list[0] == NULL || pHost->h_addrtype != AF_INET)
                {
                    netifd_log_message(L_WARNING, "can't parse the domain as ipv4:%s!\n", curr_backup_cfg.detect_ping_ori_domain[i]);
                    continue;
                }
                ip.in.s_addr = *(in_addr_t*)pHost->h_addr;
#endif
                if(is_addr_rfc1918(ntohl(ip.in.s_addr)))
                {
                    netifd_log_message(L_WARNING, "the domain ipv4:%s is private!\n", inet_ntoa(ip.in));
                    continue;
                }
                detect_arg->detect_server_ips[detect_arg->detect_server_ips_count] = ip;
                netifd_log_message(L_INFO, "parse the domain:%s -- %s!\n", curr_backup_cfg.detect_ping_ori_domain[i], inet_ntoa(detect_arg->detect_server_ips[detect_arg->detect_server_ips_count].in));
                detect_arg->detect_data[detect_arg->detect_server_ips_count].addr = detect_arg->detect_server_ips[detect_arg->detect_server_ips_count];
                detect_arg->detect_data[detect_arg->detect_server_ips_count].recv_success   = false;
                detect_arg->detect_server_ips_count++;
            }    
            tvnow(&detect_arg->domain_lookup_time);
            period = difftimeval( &detect_arg->domain_lookup_time, &begin_time);//获取实际DNS的执行时间
            netifd_log_message(L_INFO, "parse the domain spend time: %dms!\n", period/1000);
//            for(i = NETWORK_TYPE_WIRED; i < NETWORK_TYPE_MAX; ++i)
//            {//此步骤较浪费时间，由于uloop是串行，因此此处延长各个timeout回调函数的时间
//                if(period < PING_DIMAIN_DNS_TIMEOUT_DEFAULT * 1000000)
//                {//没有达到最大延时，将timeout时间恢复，实现在DNS查询期间，timeout进程“停止计时”的效果
//                    uloop_timeout_set(&g_detect_uloop_timer[i].detect_timeout_timer, next_timeout_time[i] - PING_DIMAIN_DNS_TIMEOUT_DEFAULT * 1000 + period / 1000);
//                }
//            }
        }
    }
//d_detect:
    if(curr_backup_cfg.way == DETECT_WAY_DNS)
    {
        detect_all_dns_query(detect_arg);
    }
    else
    {
        detect_all_ping_query(detect_arg);
    }   
go_next:
    tvnow(&end_time);
    begin_time.tv_sec += curr_backup_cfg.track_interval;
    period = difftimeval(&begin_time, &end_time);
    netifd_log_message(L_INFO, "detect_loop %d wait_time=%d!\n", detect_arg->type, period);
    if(period > 0)
    {
        (void)uloop_timeout_set(timeout, period/1000);
    }
    else
    {
        (void)uloop_timeout_set(timeout, 0);
    }
    return;
}

/**
 * @brief         当前模式为CONN_MODE_MOBILE_PREFERRED 或CONN_MODE_CABLE_PREFERRED时，且传入的网络类型失效/被down，若另一接口为up状态，直接切换为另一接口
 * @param         {enum network_type} type      传入的失效/被down的网络类型
 * @return        {void}
 */
void unset_nw(enum network_type type)
{
//    enum network_type target = NETWORK_TYPE_NONE;
//    struct interface *iface = NULL;
    int i = 0;
    if(curr_backup_cfg.mode == CONN_MODE_MOBILE_ONLY || curr_backup_cfg.mode == CONN_MODE_CABLE_ONLY)
    {
        return;
    }
    for(i = NETWORK_TYPE_WIRED; i < NETWORK_TYPE_MAX; ++i)
    {
        uloop_timeout_set(&g_detect_uloop_timer[i].detect_timer, 500);
    }
    // if (type == NETWORK_TYPE_WIRED )
    // {
    //     target = NETWORK_TYPE_MOBILE;
    // }
    // else if (type == NETWORK_TYPE_MOBILE)
    // {
    //     target = NETWORK_TYPE_WIRED;
    // }
    // else
    // {
    //     return;
    // }

    // /* USBMODEM_DBG(LOG_BK_UNSET_NW, LP_INT32, type); */

    // iface = target_network_up(target);
    // if (iface)
    // {
    //     switch_network_type(target, iface);
    // }

    return;
}

/**
 * @brief         backup函数入口，进行init/de-init
 *                初始化时初始化uloop_timer1&2&3；de-init时删除添加的各项策略路由，关闭socket，释放各类资源
 * @param         {bool} init      true为初始化,false为de-init
 * @return        {int}            初始化时返回0代表成功，非0则初始化失败；de-init时返回0
 */
int detect_init_exit(bool init)
{
    static bool detect_inited = false;
    int ret = 0;
    int i = 0, j = 0;
//    enum network_type current_nw_bak = NETWORK_TYPE_NONE;
    /* exit case */
    netifd_log_message(L_INFO, "Enter detect_init_exit %d!\n", init);
    if (!init)
    {
        netifd_log_message(L_INFO, "Enter detect_exit!\n");
        if (detect_inited)
        {
            for(i = NETWORK_TYPE_WIRED; i < NETWORK_TYPE_MAX; ++i)
            {
                del_ip_rule_route(&g_detect_uloop_timer[i].detect_args);              
                if(curr_backup_cfg.way == DETECT_WAY_DNS)
                {
                    for(j = 0; j < g_detect_uloop_timer[i].detect_args.detect_server_ips_count; j++)
                    {
                        if(g_detect_uloop_timer[i].detect_args.detect_data[j].sockfd.fd > 0)
                        {
                            uloop_fd_delete(&g_detect_uloop_timer[i].detect_args.detect_data[j].sockfd);
                            close(g_detect_uloop_timer[i].detect_args.detect_data[j].sockfd.fd);
                            g_detect_uloop_timer[i].detect_args.detect_data[j].sockfd.fd = -1;        
                        }
                    }
                }
                else
                {
                    if(g_detect_uloop_timer[i].detect_args.detect_data[0].sockfd.fd > 0)
                    {
                        uloop_fd_delete(&g_detect_uloop_timer[i].detect_args.detect_data[0].sockfd);
                        close(g_detect_uloop_timer[i].detect_args.detect_data[0].sockfd.fd);
                        g_detect_uloop_timer[i].detect_args.detect_data[0].sockfd.fd = -1;
                    }
                }
                g_detect_uloop_timer[i].detect_args.domain_lookup_time.tv_sec = 0;
                g_detect_uloop_timer[i].detect_args.domain_lookup_time.tv_usec = 0;
                g_detect_uloop_timer[i].detect_args.detect_server_ips_count = 0;
                g_detect_uloop_timer[i].detect_args.this_cycle_dns_detect_has_result = false;
                g_detect_uloop_timer[i].detect_args.this_cycle_dns_detect_success_count = 0;
                g_detect_uloop_timer[i].detect_args.this_cycle_dns_detect_fail_count = 0;
                g_detect_uloop_timer[i].detect_args.detect_if = NULL;
                memset(g_detect_uloop_timer[i].detect_args.detect_server_ips, 0, sizeof(g_detect_uloop_timer[i].detect_args.detect_server_ips));
                memset(g_detect_uloop_timer[i].detect_args.detect_data, 0, sizeof(g_detect_uloop_timer[i].detect_args.detect_data));
            }
            uloop_timeout_cancel(&g_detect_uloop_timer[0].detect_timer);
            for(i = NETWORK_TYPE_WIRED; i < NETWORK_TYPE_MAX; ++i)
            {
                uloop_timeout_cancel(&g_detect_uloop_timer[i].detect_timer);
                uloop_timeout_cancel(&g_detect_uloop_timer[i].detect_timeout_timer);
            }
            interface_write_resolv_conf();
            detect_inited = false;
        }
        return ret;
    }

    /* init case */
    if (!detect_inited)
    {
        netifd_log_message(L_INFO, "Enter detect_init!\n");
        ret = backup_cfg_init();
        srand((unsigned)time(NULL));
        //不再清空历史记录
        // current_nw_bak = detect_result.current_nw;
        // memset(&detect_result, 0, sizeof(detect_result));//重新初始化时，清空历史在线记录
        // detect_result.current_nw = current_nw_bak;//需要保留之前的current_nw
        if (ret != 0 || !curr_backup_cfg.use_backup)
        {
            return ret;
        }
        g_detect_uloop_timer[0].detect_timer.cb = switch_network_judge_loop;
        uloop_timeout_set(&g_detect_uloop_timer[0].detect_timer, 6500);
        for(i = NETWORK_TYPE_WIRED; i < NETWORK_TYPE_MAX; ++i)
        {
            g_detect_uloop_timer[i].detect_timer.cb = detect_loop;
            g_detect_uloop_timer[i].detect_timeout_timer.cb = detect_timeout_check;
            uloop_timeout_set(&g_detect_uloop_timer[i].detect_timer, 6500);
        }
        interface_write_resolv_conf();
        detect_inited = true;
    }
    return ret;
}

