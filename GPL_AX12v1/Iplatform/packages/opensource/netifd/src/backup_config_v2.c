#include "backup_config_v2.h"
#include "usbmodem_log.h"
#include "netifd.h"
#include "interface.h"
#include "proto.h"
#include "ubus.h"
#include "system.h"
#include "connect.h"
#include <string.h>


static struct uci_context *bk_uci_ctx = NULL;
static struct uci_package *bk_uci_pkg = NULL;
extern t_detect_result detect_result;

extern struct nw_backup_cfg curr_backup_cfg;
char *network_type_str[] = {
    "",
    "WIRED",
    "MOBILE",
    ""
};
#ifdef SUPPORT_VPN_CLIENT_IN_3G4G_MODULES
#define CONNECT_IFACE_VPN            "vpn"
#endif

/**
 * @brief         获取name对应的network_type
 * @param         {char*} name  一般为iface->name
 * @return        {enum network_type} NETWORK_TYPE_WIRED(有线)/NETWORK_TYPE_MOBILE(3_4G)/NETWORK_TYPE_MAX
 */
enum network_type get_network_type(const char* name)
{
    if (strcmp(name, CONNECT_IFACE_MOBILE) == 0)
    {
        return NETWORK_TYPE_MOBILE;
    }
    else
    {
        return NETWORK_TYPE_WIRED;
    }
}

/**
 * @brief         获取当前backup功能的开关，供ubus及netifd其他模块使用
 * @return        {bool} 当前backup功能的开关
 */
bool get_backup_enable()
{
    return curr_backup_cfg.use_backup;    /* false = off, true = on*/
}

/**
 * @brief         判断传入接口是都需要自动up(拨号)
 *                若当前backup功能关闭,                   若传入的接口是mobile，返回false；否则返回true
 *                当前模式为CONN_MODE_CABLE_PREFERRED时， 均返回true(若此时wired有网，且传入的接口是mobile，也需要拨号，否则无法持续探测mobile)
 *                当前模式为CONN_MODE_CABLE_ONLY时，      若传入的接口是mobile，返回false；其他情况均返回true
 *                当前模式为CONN_MODE_MOBILE_PREFERRED时，均返回true(若此时mobile有网，且传入的接口是wired，也需要拨号，否则无法持续探测wired)
 *                当前模式为CONN_MODE_MOBILE_ONLY时，     若传入的接口是wired，返回false；其他情况均返回true
 * @param         {char*} ifname      传入的接口名
 * @return        {bool}              true:传入的接口需要自动ip(拨号) ; false:传入的接口不需要自动ip(拨号) 
 */
bool backup_auto_dial(const char* ifname)
{
    enum network_type nt = get_network_type(ifname);
    bool ret = true;
#ifdef FEATURE_3G4G_MODULES_INDEPENDENT
    if (1)
#else
    if (curr_backup_cfg.use_backup)
#endif
    {
        /*
         * In x_ONLY mode: auto dial ifname that belongs to x
         * In x_PREFERRED mode: if primary connection is offline, auto dial any ifname
         *                      else do what x_ONLY mode do
         */
        switch (curr_backup_cfg.mode)
        {
        case CONN_MODE_CABLE_PREFERRED:
            ret = true;
            break;
        case CONN_MODE_CABLE_ONLY:
            if (nt == NETWORK_TYPE_MOBILE)
            {
                ret = false;
            }
            break;
        case CONN_MODE_MOBILE_PREFERRED:
            ret = true;
            break;
        case CONN_MODE_MOBILE_ONLY:
            if (nt == NETWORK_TYPE_WIRED)
            {
                ret = false;
            }
            break;
        default:
            break;
        }
    }
    else    /* If backup NOT enabled, NEVER auto dial mobile iface*/
    {
        if (nt == NETWORK_TYPE_MOBILE)
        {
            ret = false;
        }
    }

    return ret;
}
/**
 * @brief         设置当前的工作网络供ubus读取
 * @param         {enum network_type} type，待设置的当前的工作网络类型
 * @return        {int} 0为成功，其他失败
 */
int set_ubus_curr_nw(enum network_type type)
{
    if (type != NETWORK_TYPE_WIRED && type != NETWORK_TYPE_MOBILE)
    {
        return -1;
    }
    netifd_log_message(L_INFO, "set curr nw to %d without switch route!\n", type);
    detect_result.current_nw = type;
    if (curr_backup_cfg.use_backup)
    {
        USBMODEM_DBG(LOG_BK_SWITCH_NW, LP_STRING, network_type_str[type]);
    }
    return 0;
}

void switch_network_type(enum network_type type, struct interface *iface);
struct interface * target_network_up(enum network_type type);
/**
 * @brief         通过ubus切换当前的工作网络
 * @param         {enum network_type} type，待切换的当前的工作网络类型
 * @return        {int} 0为成功，其他失败
 */
int switch_ubus_curr_nw(enum network_type type)
{
    struct interface *iface = NULL;
    if (type != NETWORK_TYPE_WIRED && type != NETWORK_TYPE_MOBILE)
    {
        return -1;
    }
    if ((iface = target_network_up(type)) != NULL)
    {
        switch_network_type(type, iface);
    }
    else
    {
        return -2;
    }
    netifd_log_message(L_INFO, "switch curr nw to %d through ubus!\n", type);
    return 0;
}

/**
 * @brief         从全局变量current_network中读取当前的工作网络
 * @return        {enum network_type} 返回NETWORK_TYPE_WIRED/NETWORK_TYPE_MOBILE
 */
enum network_type get_curr_nw()
{
    enum network_type type = detect_result.current_nw;
    if(type == NETWORK_TYPE_NONE)
    {
        type = curr_backup_cfg.primary_nw;
    }
    return type;
}

/**
 * @brief         从全局变量current_network中读取当前的工作网络
 * @return        {char*} 返回全局的字符串，NETWORK_TYPE_WIRED对应“WIRED”，NETWORK_TYPE_MOBILE对应“MOBILE”
 */
char* get_ubus_curr_nw()
{
    enum network_type type = detect_result.current_nw;
    if(type == NETWORK_TYPE_NONE)
    {
        type = curr_backup_cfg.primary_nw;
    }
    if (type < NETWORK_TYPE_NONE || type > NETWORK_TYPE_MAX)
    {
        return NULL;
    }
    return network_type_str[type];
}

/**
 * @brief         从全局变量current_network中读取各个网络的在线状态,供ubus读取
 * @param         {bool*} connect_status    connect_status指针，长度NETWORK_TYPE_MAX
 * @return        {void}
 */
void get_ubus_backup_network_status(bool* connect_status)
{
    memcpy(connect_status, detect_result.is_connect, sizeof(detect_result.is_connect));
    return;
}

/**
 * @brief         从backup的uci中读取指定的section和name，string类型   如 protocol.backup.detect_way section即backup，name即detect_way
 * @param         {char} *section_name     指定的section，基本为"backup"
 * @param         {char} *name             指定的name
 * @return        {char*}                  读取出来的string
 */
static char* bk_config_option_str(const char *section_name, const char *name)
{
    struct uci_package *pkg = bk_uci_pkg;
    struct uci_section *section = NULL;

    section = uci_lookup_section(bk_uci_ctx, pkg, section_name);
    if (!section)
    {
        return NULL;
    }
    return (char *)uci_lookup_option_string(bk_uci_ctx, section, name);
}

/**
 * @brief         从backup的uci中读取指定的section和name，string转为bool类型，"on"视为true;"off"视为false
 * @param         {char} *section_name     指定的section，基本为"backup"
 * @param         {char} *name             指定的name
 * @param         {bool} *enable           读取出来的bool指针
 * @return        {int}                    0为成功，其他失败
 */
static int config_option_bool(const char *section_name, const char *name, bool *enable)
{
    char *val = bk_config_option_str(section_name, name);
    if (!val)
    {
        return -1;
    }

    if (strcmp(val, "on") == 0)
    {
        *enable = true;
    }
    else
    {
        *enable = false;
    }

    return 0;
}

/**
 * @brief         从backup的uci中读取指定长度的section和name，string类型
 * @param         {char} *section_name     指定的section，基本为"backup"
 * @param         {char} *name             指定的name
 * @param         {char} *str              读取出来的string
 * @param         {int} length             读取出来string的最长长度
 * @return        {int}                    0为成功，其他失败
 */
static int config_option_string(const char *section_name, const char *name, char *str, int length)
{
    char *val = bk_config_option_str(section_name, name);
    if (!val)
    {
        return -1;
    }
    memcpy(str, val, length);
    return 0;
}

/**
 * @brief         从backup的uci中读取指定的section和name，string转为int类型
 * @param         {char} *section_name     指定的section，基本为"backup"
 * @param         {char} *name             指定的name
 * @param         {int} *val               读取出来的int指针
 * @return        {int}                    0为成功，其他失败
 */
static int config_option_int(const char *section_name, const char *name, int *val)
{
    char *str = bk_config_option_str(section_name, name);
    unsigned long tmp;

    if (!str)
    {
        return -1;
    }

    tmp = strtoul(str, NULL, 10);
    if (tmp == ULONG_MAX)
    {
        return -1;
    }

    *val = (int)tmp;

    return 0;
}

/**
 * @brief         从backup的uci中读取指定的section和name，读取ip(域名)list
 * @param         {char} *section_name     指定的section，基本为"backup"
 * @param         {char} *name             指定的name，基本为"track_ip"
 * @param         {union if_addr} *ip      ip数组,以网络字节序储存读取出的ip，ip会去重
 * @param         {char} *domain           域名字符串数组,储存读取出的域名
 * @param         {uint8_t*} ip_count      成功读取到的去重后的ip的数目，传入指针，修改指针指向的值
 * @param         {uint8_t*} domain_count  成功读取到的域名的数目，传入指针，修改指针指向的值
 * @param         {int} ip_max             需要从uci中读取的最大ip+域名的数量
 * @return        {*}
 */
static int config_option_ip_list(const char *section_name, const char *name, union if_addr *ip, char (*domain)[MAX_DOMAIN_LENGTH],
                                    uint8_t* ip_count, uint8_t* domain_count, const int ip_max)
{
    
    struct uci_package *pkg = bk_uci_pkg;
    struct uci_section *section = NULL;
    struct uci_option *option = NULL;
    struct uci_element *element = NULL;
    in_addr_t inaddr;        //ip地址（网络字节序）
    int i = 0;
    bool same_ip = false;
    section = uci_lookup_section(bk_uci_ctx, pkg, section_name);
    if (!section)
    {
        return -1;
    }
    
    option = uci_lookup_option(bk_uci_ctx, section, name);
    
    if (!option)
    {
        return -1;
    }
    *ip_count = 0;
    *domain_count = 0;
    if(UCI_TYPE_LIST == option->type)
    {
        uci_foreach_element(&option->v.list, element)        //遍历list
        {    
            same_ip = false;
            if((*ip_count + *domain_count) >= ip_max)
            {
                break;
            }
            
            if ((inaddr = inet_addr(element->name)) == INADDR_NONE)
            {
                /* 转换失败，表明是主机名,需通过主机名获取ip */
                strncpy(*(domain+*domain_count),element->name, MAX_DOMAIN_LENGTH);
                *domain_count = *domain_count + 1;
                continue;
            }
            ip[*ip_count].in.s_addr = inaddr;
            //inet_pton(AF_INET, element->name, &ip[*ip_count].in);
            for(i = 0; i < *ip_count; ++i)
            {////避免添加重复相同的IP
                if(ip[*ip_count].in.s_addr == ip[i].in.s_addr)
                {
                    netifd_log_message(L_INFO, "Skip the same ip %s processing backup config!\n", inet_ntoa(ip[i].in));
                    same_ip = true;
                    break;
                }
            }
            if(same_ip)
            {
                ip[*ip_count].in.s_addr = 0;
                continue;
            }
            *ip_count = *ip_count + 1;
        }
    }
    return 0;
}

/**
 * @brief         backup uci初始化，打开"protocol" uci 配置
 * @return        {*}
 */
static int bk_uci_init()
{
    int ret = 0;
    bk_uci_ctx = uci_alloc_context();
    if (bk_uci_ctx == NULL)
    {
        return -1;
    }

    ret = uci_load(bk_uci_ctx, BACKUP_UCI_CFG, &bk_uci_pkg);
    if (ret != 0)
    {
        uci_free_context(bk_uci_ctx);
        bk_uci_ctx = NULL;
    }

    return ret;
}
/**
 * @brief         backup uci销毁，释放资源
 * @return        {*}
 */
static void bk_uci_exit()
{
    if (bk_uci_ctx && bk_uci_pkg)
    {
        uci_unload(bk_uci_ctx, bk_uci_pkg);
        bk_uci_pkg = NULL;
        uci_free_context(bk_uci_ctx);
        bk_uci_ctx = NULL;
    }
    return;
}

/**
 * Init the uci context object.
 * @return uci context object
 */
static struct uci_context *_uci_context_init(const char *config_path)
{
    struct uci_context *uci_ctx = NULL;
    uci_ctx = uci_alloc_context();
    if (uci_ctx) {
        uci_set_confdir(uci_ctx, config_path);
    }

    return uci_ctx;
}

/**
 * Free the uci context object.
 * @param uci_ctx uci context object
 */
static void _uci_context_free(struct uci_context *uci_ctx)
{
    if (uci_ctx) {
        uci_free_context(uci_ctx);
    }
}
/*!
*\fn           _uci_get_value()
*\brief        Get an element's value
*\param[in]    p_uci_str: uci tuple string to look up
*\param[out]   p_value:       value of the option
*\return       OK/ERROR
*/
static int _uci_get_value(char * uci_str, char* p_value)
{
    struct uci_context *uci_ctx = NULL;
    struct uci_element *e = NULL;
    struct uci_ptr p_uci;
    char p_uci_str[65] = {0};
    if (NULL == p_uci_str || NULL == p_value) {
        netifd_log_message(L_ERROR, "p_uci_str or p_value is null\n");
        goto error;
    }
    strncpy(p_uci_str, uci_str, strlen(uci_str));
    uci_ctx = _uci_context_init("/etc/config");
    if (!uci_ctx) {
        netifd_log_message(L_ERROR, "fail to init uci context:%s\n", p_uci_str);
        goto error;
    }
    if (UCI_OK != uci_lookup_ptr(uci_ctx, &p_uci, p_uci_str, true)) {
        netifd_log_message(L_INFO, "fail to get ptr %s \n", p_uci_str);
        goto error;
    }
    e = p_uci.last;
    if (UCI_TYPE_OPTION != e->type) {
        netifd_log_message(L_INFO, "element type is not option:%d\n", e->type);
        goto error;
    }
    if (UCI_TYPE_STRING != p_uci.o->type) {
        netifd_log_message(L_INFO, "option type is not string:%d\n", p_uci.o->type);
        goto error;
    }

    strncpy(p_value, p_uci.o->v.string, 64);
    //netifd_log_message(L_DEBUG, "Success to get uci value %s=%s\n", uci_str, p_uci.o->v.string);
    _uci_context_free(uci_ctx);
    return 0;

error:
    _uci_context_free(uci_ctx);
    return -1;
}

static int get_user_prefered_mode()
{
    int mode = CONN_MODE_CABLE_ONLY;
    char uci_value[64] = "";
    int ret = _uci_get_value("feature.sys_feature.internet_type", uci_value);
    if (ret != 0)
    {
        netifd_log_message(L_INFO, "Failed to get feature.sys_feature.internet_type!\n");
        mode = CONN_MODE_CABLE_ONLY;
    }
    else if (strcmp(uci_value, "usb_tethering") == 0 || strcmp(uci_value, "34G_modem") == 0)
    {
        mode = CONN_MODE_MOBILE_ONLY;
    }
    else
    {
        mode = CONN_MODE_CABLE_ONLY;
    }
    return mode;
}

/**
 * @brief         从uci配置protocol.backup中解析配置信息
 * @param         {nw_backup_cfg} *bk_cfg, 待填入的配置信息
 * @return        {int} 0 for success, others for failed
 */
int uci_bk_cfg_get(struct nw_backup_cfg *bk_cfg)
{
    int ret = 0;
    int tmp = 0;
    bool bool_tmp = false;
    char str[64] = "";
    if (bk_cfg == NULL)
    {
        return -1;
    }
    
    memset(bk_cfg, 0, sizeof(struct nw_backup_cfg));

    ret = bk_uci_init();
    if (ret != 0)
    {
        return ret;
    }
    
    ret = config_option_bool("backup", "enable", &bk_cfg->use_backup);
    if (ret != 0)
    {
        goto exit;
    }
    if(!bk_cfg->use_backup)
    {
        goto exit;
    }
    
    ret = config_option_bool("backup", "wired_enable", &bool_tmp);
    if (ret != 0)
    {
        goto exit;
    }
    bk_cfg->network_attributes[NETWORK_TYPE_WIRED].enable = bool_tmp;
    ret = config_option_int("backup", "wired_priority", &tmp);
    if (ret != 0)
    {
        goto exit;
    }
    bk_cfg->network_attributes[NETWORK_TYPE_WIRED].priority = tmp;

    ret = config_option_bool("backup", "mobile_enable", &bool_tmp);
    if (ret != 0)
    {
        goto exit;
    }
    bk_cfg->network_attributes[NETWORK_TYPE_MOBILE].enable = bool_tmp;
    ret = config_option_int("backup", "mobile_priority", &tmp);
    if (ret != 0)
    {
        goto exit;
    }
    bk_cfg->network_attributes[NETWORK_TYPE_MOBILE].priority = tmp;

    ret = config_option_string("backup", "detect_way", str, sizeof(str));
    if (ret != 0)
    {
        tmp = DETECT_WAY_DEFAULT;
        ret = 0;
    }
    else if (strcmp(str, "dns") == 0)
    {
        tmp = DETECT_WAY_DNS;
    }
    else if (strcmp(str, "ping") == 0)
    {
        tmp = DETECT_WAY_PING;
    }
    else
    {
        tmp = DETECT_WAY_DEFAULT;
    }
    bk_cfg->way = tmp;
    
    ret = config_option_int("backup", "track_interval", &tmp);
    if (ret != 0)
    {
        tmp = TRACK_INTERVAL_DEFAULT;
        ret = 0;
    }
    bk_cfg->track_interval = tmp;
    
    ret = config_option_int("backup", "change_to_connect_threshold", &tmp);
    if (ret != 0)
    {
        tmp = CAHNGE_TO_CONNECT_THRESHOLD_DEFAULT;
        ret = 0;
    }
    bk_cfg->change_to_connect_threshold = tmp;
    
    ret = config_option_int("backup", "change_to_disconnect_threshold", &tmp);
    if (ret != 0)
    {
        tmp = CAHNGE_TO_DISCONNECT_THRESHOLD_DEFAULT;
        ret = 0;
    }
    bk_cfg->change_to_disconnect_threshold = tmp;
    
    ret = config_option_int("backup", "ping_timeout", &tmp);
    if (ret != 0)
    {
        tmp = PING_TIMEOUT_DEFAULT;
        ret = 0;
    }
    bk_cfg->ping_timeout = tmp > bk_cfg->track_interval ? bk_cfg->track_interval : tmp;
    
    ret = config_option_ip_list("backup", "track_ip", bk_cfg->detect_ping_ips, bk_cfg->detect_ping_ori_domain,
        &bk_cfg->detect_ping_ips_count, &bk_cfg->detect_ping_domain_count, DETECT_IP_MAX);
    if(ret != 0)
    {
        memset(bk_cfg->detect_ping_ips, 0, sizeof(bk_cfg->detect_ping_ips));
        bk_cfg->detect_ping_ips_count = 0;
        memset(bk_cfg->detect_ping_ori_domain, 0, sizeof(bk_cfg->detect_ping_ori_domain));
        bk_cfg->detect_ping_domain_count = 0;
        ret = 0;
    }
    
    ret = config_option_int("backup", "fast_update_domain_ip_peroid", &tmp);
    if (ret != 0)
    {
        tmp = FAST_UPDATE_DOMAIN_IP_PERIOD_DEFAULT;
        ret = 0;
    }
    bk_cfg->fast_update_domain_ip_peroid = tmp;
    
    ret = config_option_int("backup", "update_domain_ip_peroid", &tmp);
    if (ret != 0)
    {
        tmp = UPDATE_DOMAIN_IP_PERIOD_DEFAULT;
        ret = 0;
    }
    bk_cfg->update_domain_ip_peroid = tmp;
    
    ret = config_option_int("backup", "ping_update_domain_ip_timout", &tmp);
    if (ret != 0)
    {
        tmp = PING_DIMAIN_DNS_TIMEOUT_DEFAULT;
        ret = 0;
    }
    bk_cfg->ping_update_domain_ip_timout = tmp;
    
    ret = config_option_int("backup", "reliability", &tmp);
    if (ret != 0 || tmp > DETECT_IP_MAX)
    {
        tmp = RELIABILITY_DEFAULT;
        ret = 0;
    }
    if(bk_cfg->detect_ping_ips_count == 0 && bk_cfg->detect_ping_domain_count == 0)
    {
        bk_cfg->way = DETECT_WAY_DNS;
    }

    if(bk_cfg->way == DETECT_WAY_PING)
    {
        bk_cfg->reliability = tmp > (bk_cfg->detect_ping_ips_count + bk_cfg->detect_ping_domain_count) ? (bk_cfg->detect_ping_ips_count + bk_cfg->detect_ping_domain_count) : tmp;
    }
    else
    {
        bk_cfg->reliability = 1;
    }

exit:
    bk_uci_exit();

    return ret;
}


/**
 * @brief         从uci配置protocol.backup中解析配置信息，完成curr_backup_cfg的初始化
 * @return        {int} 0 for success, 1 for failed
 */
int backup_cfg_init()
{
    int ret = 0;
    ret = uci_bk_cfg_get(&curr_backup_cfg);
    if (ret != 0)
    {
        return ret;
    }

#ifdef FEATURE_3G4G_MODULES_INDEPENDENT
    if (!curr_backup_cfg.use_backup)
    {
        curr_backup_cfg.mode = get_user_prefered_mode();
    }
    else
#endif
    {
        if(curr_backup_cfg.network_attributes[NETWORK_TYPE_WIRED].enable && curr_backup_cfg.network_attributes[NETWORK_TYPE_MOBILE].enable)
        {
            if(curr_backup_cfg.network_attributes[NETWORK_TYPE_WIRED].priority <= curr_backup_cfg.network_attributes[NETWORK_TYPE_MOBILE].priority)
            {
                curr_backup_cfg.primary_nw = NETWORK_TYPE_WIRED;
                curr_backup_cfg.secondary_nw = NETWORK_TYPE_MOBILE;
                curr_backup_cfg.mode = CONN_MODE_CABLE_PREFERRED;
            }
            else
            {
                curr_backup_cfg.primary_nw = NETWORK_TYPE_MOBILE;
                curr_backup_cfg.secondary_nw = NETWORK_TYPE_WIRED;
                curr_backup_cfg.mode = CONN_MODE_MOBILE_PREFERRED;
            }
        }
        else if(!curr_backup_cfg.network_attributes[NETWORK_TYPE_WIRED].enable && curr_backup_cfg.network_attributes[NETWORK_TYPE_MOBILE].enable)
        {
            curr_backup_cfg.primary_nw = NETWORK_TYPE_MOBILE;
            curr_backup_cfg.secondary_nw = NETWORK_TYPE_WIRED;
            curr_backup_cfg.mode = CONN_MODE_MOBILE_ONLY;
        }
        else if(curr_backup_cfg.network_attributes[NETWORK_TYPE_WIRED].enable && !curr_backup_cfg.network_attributes[NETWORK_TYPE_MOBILE].enable)
        {
            curr_backup_cfg.primary_nw = NETWORK_TYPE_WIRED;
            curr_backup_cfg.secondary_nw = NETWORK_TYPE_MOBILE;
            curr_backup_cfg.mode = CONN_MODE_CABLE_ONLY;
        }
        else
        {
            curr_backup_cfg.use_backup = false;
        }
    }

    netifd_log_message(L_INFO, "bk_cfg.use_backup=%s\n", curr_backup_cfg.use_backup?"true":"false");
    netifd_log_message(L_INFO, "bk_cfg.mode=%d\n", curr_backup_cfg.mode);
    netifd_log_message(L_INFO, "bk_cfg.way=%d\n", curr_backup_cfg.way);
    netifd_log_message(L_INFO, "bk_cfg.track_interval=%u\n", curr_backup_cfg.track_interval);
    netifd_log_message(L_INFO, "bk_cfg.change_to_connect_threshold=%u\n", curr_backup_cfg.change_to_connect_threshold);
    netifd_log_message(L_INFO, "bk_cfg.change_to_disconnect_threshold=%u\n", curr_backup_cfg.change_to_disconnect_threshold);
    netifd_log_message(L_INFO, "bk_cfg.ping_timeout=%u\n", curr_backup_cfg.ping_timeout);
    netifd_log_message(L_INFO, "bk_cfg.reliability=%u\n", curr_backup_cfg.reliability);
    netifd_log_message(L_INFO, "bk_cfg.detect_ping_ips_count=%d\n", curr_backup_cfg.detect_ping_ips_count);
    for(int i = 0; i < curr_backup_cfg.detect_ping_ips_count; ++i)
    {
        netifd_log_message(L_INFO, "bk_cfg.track_ip[%d]=%s\n", i, inet_ntoa(curr_backup_cfg.detect_ping_ips[i].in));
    }
    netifd_log_message(L_INFO, "bk_cfg.detect_ping_domain_count=%d\n", curr_backup_cfg.detect_ping_domain_count);
    for(int i = 0; i < curr_backup_cfg.detect_ping_domain_count; ++i)
    {
        netifd_log_message(L_INFO, "bk_cfg.track_ip[%d]=%s\n", i, curr_backup_cfg.detect_ping_ori_domain[i]);
    }
    netifd_log_message(L_INFO, "bk_cfg.fast_update_domain_ip_peroid=%u\n", curr_backup_cfg.fast_update_domain_ip_peroid);
    netifd_log_message(L_INFO, "bk_cfg.update_domain_ip_peroid=%u\n", curr_backup_cfg.update_domain_ip_peroid);
    netifd_log_message(L_INFO, "bk_cfg.ping_update_domain_ip_timout=%u\n", curr_backup_cfg.ping_update_domain_ip_timout);

    return ret;
}

#ifdef SUPPORT_VPN_CLIENT_IN_3G4G_MODULES
int set_vpn_client_parent(const char* ifname)
{
    struct uci_context *uci_ctx;
    struct uci_package *uci_network;
    struct uci_section *section = NULL; 
    const char *old_parent = NULL;
    struct interface *iface = NULL;
    int ret = 0;

    uci_ctx = uci_alloc_context();
    uci_set_confdir(uci_ctx, "/etc/config");
    if (UCI_OK != uci_load(uci_ctx, "network", &uci_network))
    {
        uci_perror(uci_ctx, "network");
        goto not_exist;
    }
    
    if (NULL != ( section = uci_lookup_section(uci_ctx, uci_network, "vpn") ))    
    {    
        old_parent = uci_lookup_option_string(uci_ctx, section, "parent");

        if ( !strcmp(old_parent, ifname) )
        {
            //system("echo old_parent == new_parent, pass > /dev/console");
            goto not_exist;
        }
        
        struct uci_ptr parent_op_ptr = {
            .package = "network",
            .section = "vpn",
            .option = "parent",
            .value = ifname,
        };

        if (UCI_OK != uci_set(uci_ctx, &parent_op_ptr))
        {
            netifd_log_message(L_ERROR, "Failed to set vpn parent\n");
            goto not_exist;
        }
        if (UCI_OK != uci_commit(uci_ctx, &uci_network, false))
        {
            netifd_log_message(L_ERROR, "Failed to commit vpn parent set\n");
            goto not_exist;
        }
        //system("echo success reset_vpn_client_parent > /dev/console");

        ret = 1;
    
    }  
    
    
not_exist:
    uci_unload(uci_ctx, uci_network);
    uci_free_context(uci_ctx);
    uci_ctx = NULL;
    

    if (ret)
    {
        vlist_for_each_element(&interfaces, iface, node)
        {
            if (strcmp(iface->name, CONNECT_IFACE_VPN) != 0)
            {
                continue;
            }

            interface_set_down(iface);
            interface_set_available(iface, true);
            interface_set_up(iface);
        }
        //system("echo have restart vpn iface > /dev/console");
    }
    
    //system("echo end set_vpn_client_parent5 > /dev/console");
    
    return ret;  

}

#endif


