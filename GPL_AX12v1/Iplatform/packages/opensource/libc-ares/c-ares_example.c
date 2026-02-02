/*  Copyright(c) 2009-2019 Shenzhen TP-LINK Technologies Co.Ltd.
 *
 * file		c-ares_example.c
 * brief		提供简单的libcares的使用示例
 * details	    在libcares api的基础上实现了可定制超时思路，作为设置dns超时的参考
 *              
 *              c-ares功能简介
 *              1、支持异步DNS查询
 *              2、支持ipv4，ipv6服务器以及期望获取到的地址类型
 *              3、其实现不使用信号，不使用多线程，基本没有全局变量和静态变量，可保证线程安全
 *              4、支持基于UDP和TCP的DNS查询
 *              5、支持域名解析和反向域名解析
 *              6、一次可支持多服务器，多域名查询。
 *              7、可高度定制化
 *              
 *              关于libcares的使用，可以参考源代码目录下的adig.c/acountry.c/ahost.c以及本文件
 *              的示例。
 *
 *              关于api的说明，可以到官网c-ares官网进行查看，或者查看man手册，或者看源代码
 *              目录下的《c-ares api》.docx
 *
 * local compile      gcc c-ares_example.c -o example  -lrt -lcares(需先安装lcares库)
 *
 * author	Chen Chaoxiang
 * version	1.0
 * date		11Jul19
 *
 * history 	\arg 	
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <limits.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <ares.h>

/**************************************************************************************************/
/*                                           DEFINES                                              */
/**************************************************************************************************/
#define IP_LEN                  64

#define FREE(a)         \
    do{                 \
        if (a != NULL)  \
        {               \
            free(a);    \
            a = NULL;   \
        }               \
    }while(0)


/**************************************************************************************************/
/*                                           TYPES                                                */
/**************************************************************************************************/
typedef struct 
{
    char            host[64];
    char            ip[10][IP_LEN];
    int             count;
    int             is_down;
    int             status;
} IP_LIST;

/**************************************************************************************************/
/*                                           EXTERN_PROTOTYPES                                    */
/**************************************************************************************************/

/**************************************************************************************************/
/*                                           LOCAL_PROTOTYPES                                     */
/**************************************************************************************************/

/**************************************************************************************************/
/*                                           VARIABLES                                            */
/**************************************************************************************************/

/**************************************************************************************************/
/*                                           LOCAL_FUNCTIONS                                      */
/**************************************************************************************************/
static struct timeval tvnow(void)
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
    
    return now;
}

static long tvdiff(struct timeval newer, struct timeval older)
{
    return (newer.tv_sec - older.tv_sec) * 1000 +
        (newer.tv_usec - older.tv_usec) / 1000;
}


/*
 * typedef void (*ares_host_callback)(void *arg,
 *                                  int status,
 *                                  int timeouts,
 *                                  struct hostent *hostent);
 *
 *
 * gethost系列的的callback回调
 * 因为示例程序要打印IP，所以把IP转成了字符串形式，如果需要其他格式可根据需求转化
 */
static void dns_callback(void * arg, int status, int timeouts, struct hostent * hostent) 
{
    int                i     = 0;
    IP_LIST           *ips   = (IP_LIST *)arg; 
    char             **pptr;

    if (ips == NULL)
    {
        ips->status = -1;
        return;
    }
    
    /* 确保status为ARES_SUCCESS时才可以操作hostent */
    if (status == ARES_SUCCESS)
    {
        strncpy(ips->host, hostent->h_name, sizeof(ips->host));
        for (i = 0, pptr = hostent->h_addr_list; *pptr != NULL && i < 10; pptr++, ++i)
        {
            inet_ntop(hostent->h_addrtype, *pptr, ips->ip[i], IP_LEN);
            ips->count++;
        }
        ips->is_down = 1;
    }
    
    ips->status = status;
}



/* 
 * fn		static int dns_resolve(const char * server, const char * host, int family, long timeout, IP_LIST *ips)
 * brief	示例域名解析函数，支持制定固定格式的dns服务器，单个域名，ipv4或ipv6，总超时时间
 * details	
 *
 * param[in]	server  ：dns服务器，字符串格式，字符串格式需满足c-ares的规定
 * param[in]	host    ：待解析主机名
 * param[in]	family  ：期望获取地址的ip版本
 * param[in]	timeout ：总超时时间，单位ms
 * param[out]	ips     ：解析到的ip地址
 *
 * return	int
 * retval   参考ares.h中定义的各种状态值
 *
 * note		
 */
static int dns_resolve(const char * server, const char * host, int family, long timeout, IP_LIST *ips)
{
    ares_channel    channel;
    int             status;
    int             nfds;
    int             count;
    int             rc;
    fd_set          read_fds,
                    write_fds;
    struct timeval  now = tvnow();

    /* 库的初始化，linux平台可不加，加了也不影响 */
    status = ares_library_init(ARES_LIB_INIT_ALL);
    if (status != ARES_SUCCESS)
    {
        fprintf(stderr, "ares_library_init: %s\n", ares_strerror(status));
        return 1;
    }

    /* channel进行初始化
     * ares_init：
     *      是简单初始化，参数都是用默认值。
     * 
     * ares_init_options：
     *      可以根据入参定制化初始化的内容
     */
    status = ares_init(&channel);
    if (status != ARES_SUCCESS)
    {
        fprintf(stderr, "ares_init_options: %s\n", ares_strerror(status));
        return 1;
    }

    /* 设置dns server 及 port
     * ares_set_servers_csv：
     *      设置服务器地址，地址类型可为ipv4和ipv6，入参为字符串形式，但字符串有一定格式
     *      ，具体参考该接口的注释说明
     * ares_set_servers_ports_csv：
     *      设置服务器地址及端口号，地址类型可为ipv4和ipv6，入参为字符串形式，但字符串有
     *      一定格式，具体参考该接口的注释说明
     * ares_set_servers：
     *      设置服务器地址，需打包成制定结构体传入
     * ares_set_servers_ports：
     *      设置服务器地址及端口号，需打包成制定结构体传入
     */
    if (server != NULL)
        ares_set_servers_csv(channel, server);

    /*
     * 域名解析或反向域名解析，可使用的接口挺多
     * ares_gethostbyname   ：Initiate a host query by address
     * ares_gethostbyaddr   ：Initiate a host query by name
     * ares_gethostbyname_file：Lookup a name in the system's hosts file
     * ares_getnameinfo     ：Address-to-nodename translation in protocol-independent manner
     *
     * 更加定制化的使用可以参考下面这两个接口：
     * ares_search          ：Initiate a DNS query with domain search
     * ares_query           ：Initiate a single-question DNS query
     */
     
    /* 
     * fn       void ares_gethostbyname(ares_channel channel, const char *name, int family,
     *                                  ares_host_callback callback, void *arg)
     *
     * brief    与gethostbyname功能类似，不过是异步的，所以需要提供一个回调接口来处理解析的结果，
     *          回掉接口的定义参考本文件中的回调接口
     *
     * param[in] channel     ：c-ares的关键参数
     * param[in] name        ：待解析域名，字符串格式
     * param[in] family      : 协议族，制定查找结果为v6还是v4的地址
     * param[in] callback    ：回调接口
     * param[in] arg         ：传递给回调接口的参数，一般用来存储解析到的域名。
     * 
     * return   void
     */
    ares_gethostbyname(channel, host, family, dns_callback, (void *)ips);
    /* 上面的过程只是进行了tcp的连接，或者udp的发包，因为tcp和udp都设置的是非阻塞，所以很快
     * 就会返回，接下来的main loop就是用来进行报文交互及处理了 */


    /* main loop, 必须要有，在此可以定制超时时间 */
    while (1)
    {
        struct timeval *tvp, tv, store;
        long            timediff;
        int             itimeout;
        int             timeout_ms;

        /* 根据总超时剩余时间计算本次loop select的最大超时时间 */
        itimeout = (timeout > (long)INT_MAX) ? INT_MAX : (int)timeout;
        store.tv_sec = itimeout/1000;
        store.tv_usec = (itimeout % 1000) * 1000;
        
        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);
        
        /* 获取channel中正在使用的fd */
        nfds = ares_fds(channel, &read_fds, &write_fds); 
        if (nfds == 0)
            break;
        /* 
         * fn       struct timeval *ares_timeout(ares_channel channel, struct timeval *maxtv,
         *                                       struct timeval *tvbuf)
         * brief    采用ares_timeout来计算比较合理的select的超时时间
         *
         * param[in]    channel: 
         * param[in]    maxtv  : 可以指定的最大超时时间
         * param[in]    tv     : 可写的一个buf
         *
         * return       tvp
         * retval       这个指针可能指向maxtv， 也可能指向tv
         *
         * note     这里的超时只是select的超时，并不是整个dns解析过程的总超时时间，详细
         *          控制总超时时间需要添加更多的时间计算
         */
        tvp = ares_timeout(channel, &store, &tv);
        count = select(nfds, &read_fds, &write_fds, NULL, tvp);
        if (count < 0)
            break;

        /*
         * 处理报文
         * ares_process     ：select来实现超时
         * ares_process_fd  ：其他非select来实现超时，例如poll，可参照curl中的实现
         */
        ares_process(channel, &read_fds, &write_fds);

        if(ips->is_down == 1)
            break;

        /* 该部分用来实现总超时 */
        struct timeval now2 = tvnow();
        timediff = tvdiff(now2, now);       /* 计算这次loop花费时间 */
        timeout -= timediff ? timediff : 1;         
        now = now2;                                 /* for next loop */
        if(timeout < 0) {
            /* 总超时时间到了，那就结束这次dns请求 */
            ares_cancel(channel);
            break;
        }
    }

    ares_destroy(channel);

    /* linux平台可以不加 */
    ares_library_cleanup();

    return ips->status;
}

static void usage(void)
{
    printf("usage: example [-s {server addr}] [-f {a|aaaa|u}] [-t {timeout/ms}] {host} ...\n");
    exit(1);
}


/**************************************************************************************************/
/*                                           PUBLIC_FUNCTIONS                                     */
/**************************************************************************************************/

/**************************************************************************************************/
/*                                           GLOBAL_FUNCTIONS                                     */
/**************************************************************************************************/

int main(int argc, char * *argv)
{
    int              c;
    int              i = 0; 
    IP_LIST          ips;
    long             timeout;
    int              addr_family = AF_INET;
    char            *domain_server = NULL;

    while ((c = getopt(argc, argv, "s:t:f:")) != -1)
    {
        switch (c)
        {
        case 's':
            domain_server = strdup(optarg);
            break;
        case 't':
            timeout = strtol(optarg, NULL, 0);
            break;
        case 'f':
            if (!strcasecmp(optarg, "a"))
                addr_family = AF_INET;
            else if (!strcasecmp(optarg, "aaaa"))
                addr_family = AF_INET6;
            else if (!strcasecmp(optarg, "u"))
                addr_family = AF_UNSPEC;
            else
                usage();
            break;
        case 'h':
        default:
            usage();
            break;
        }
    }

    argc -= optind;
    argv += optind;
    if (argc < 1)
        usage();

    memset(&ips, 0, sizeof(ips));

    if (ARES_SUCCESS != dns_resolve(domain_server, *argv, addr_family, timeout, &ips))
    {
        printf("resolve failed!, ret = %d\n", ips.status);
        FREE(domain_server);
        return -1;
    }

    if (ips.is_down)
    {
        printf("================resolve success===================\n");
        printf("hostname  : %s\n", ips.host);
        for (i = 0; i < ips.count; ++i)
        {
            printf("ip address: %s\n", ips.ip[i]);
        }
        printf("==================================================\n");
    }

    
    FREE(domain_server);
    return 0;
}

