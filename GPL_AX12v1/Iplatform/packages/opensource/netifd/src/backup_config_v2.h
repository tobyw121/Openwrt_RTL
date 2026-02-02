#ifndef __BACKUP_CONFIG_V2_H
#define __BACKUP_CONFIG_V2_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uci.h>
#include <limits.h>
#include "netifd.h"
#include "interface-ip.h"

#define BACKUP_UCI_CFG    "/etc/config/protocol"

enum network_type
{
    NETWORK_TYPE_NONE = 0,
    NETWORK_TYPE_WIRED,
    NETWORK_TYPE_MOBILE,
    NETWORK_TYPE_MAX
};

#define    DETECT_IP_MAX              4
#define DETECT_DNS_IP_MAX             2   //make sure that DETECT_DNS_IP_MAX <= DETECT_IP_MAX
#define MAX_DOMAIN_LENGTH             64



enum detect_way
{
    DETECT_WAY_DNS = 0,
    DETECT_WAY_PING,
    DETECT_WAY_MAX
};

enum conn_mode
{
    CONN_MODE_NONE = -1,
    CONN_MODE_MOBILE_PREFERRED = 0,
    CONN_MODE_MOBILE_ONLY,
    CONN_MODE_CABLE_PREFERRED,
    CONN_MODE_CABLE_ONLY,
    CONN_MODE_MAX 
};
typedef struct
{
    bool     enable;
    int8_t   priority;
} t_network_attributes;
typedef struct
{
    bool     is_connect[NETWORK_TYPE_MAX];                   //true为在线，false为不在线，读取is_connect[NETWORK_TYPE_WIRED]/is_connect[NETWORK_TYPE_WIRED]
    uint64_t recent_detect_result[NETWORK_TYPE_MAX];         //最近64次的真实记录(成功发送)，读取最近第i次(i=1,2..64)的记录为:(recent_detect_result[NETWORK_TYPE_WIRED]>>(i-1))&1  1代表此次至少有reliability各ip可以ping通/获取到了DNS返回
    uint32_t consecutive_success_count[NETWORK_TYPE_MAX];  //连续成功记录，接口非up/出现一次fail时会清空此值
    uint32_t consecutive_failed_count[NETWORK_TYPE_MAX];     //连续失败记录，接口出现一次success时会清空此值
    enum     network_type current_nw;                        //当前使用的网络类型    
} t_detect_result;

struct nw_backup_cfg
{
    bool use_backup;                            //backup功能开关                    
    enum conn_mode mode;                        //优先级选择
    t_network_attributes network_attributes[NETWORK_TYPE_MAX];  //原始优先级、开关输入
    enum network_type primary_nw;               //第一优先级网络接口
    enum network_type secondary_nw;             //第二优先级网络接口
    enum detect_way way;                        //网络探测方法

    uint32_t track_interval;                          //多次ping之间的间隔时间
    uint32_t change_to_connect_threshold;             //连续出现几次成功时就认定该接口处于在线状态
    uint32_t change_to_disconnect_threshold;          //连续出现出现几次不通时就认定该接口处于掉线状态
    uint32_t ping_timeout;                            //ping/dns后如果超时，超时几秒认为失败
    uint32_t reliability;                             //可靠性，测试以下IP的可靠性，必须是有响应的链接的个数。简单说就是要求下面的ip几个能ping通
    union    if_addr detect_ping_ips[DETECT_IP_MAX];  //测试IP，验证是否能ping的通
    char     detect_ping_ori_domain[DETECT_IP_MAX][MAX_DOMAIN_LENGTH];////测试域名，验证是否能ping的通
    uint8_t  detect_ping_ips_count;
    uint8_t  detect_ping_domain_count;
    uint16_t fast_update_domain_ip_peroid;            //如果设置了域名，有可能没法拿到DNS，或拿到的DNS是私有地址，此时可以ping的ip总数量小于detect_ping_ips_count+detect_ping_domain_count；需要以更快的频率去查询域名的DNS
    uint16_t update_domain_ip_peroid;                 //如果设置了域名，拿到的DNS全部为公网ip，此时大概率域名对应的ip不会变化，不需要以太快的间隔刷新域名的DNS结果
    uint8_t  ping_update_domain_ip_timout;            //如果设置了域名，需要通过dns获取域名的ip，设置此超时时间，超过说明没有拿到对应的DNS，判定为此域名ping失败
};
#define DETECT_WAY_DEFAULT                         DETECT_WAY_PING
#define TRACK_INTERVAL_DEFAULT                     5
#define CAHNGE_TO_CONNECT_THRESHOLD_DEFAULT        6
#define CAHNGE_TO_DISCONNECT_THRESHOLD_DEFAULT     3
#define PING_TIMEOUT_DEFAULT                       2
#define RELIABILITY_DEFAULT                        1
#define PING_SEND_TIMEOUT_DEFAULT                  (curr_backup_cfg.ping_timeout/2)
#define FAST_UPDATE_DOMAIN_IP_PERIOD_DEFAULT       60
#define UPDATE_DOMAIN_IP_PERIOD_DEFAULT            3600
#define PING_DIMAIN_DNS_TIMEOUT_DEFAULT            2

int backup_cfg_init();
enum network_type get_network_type(const char* name);
void get_ubus_backup_network_status(bool* connect_status);
int switch_ubus_curr_nw(enum network_type type);
int uci_bk_cfg_get(struct nw_backup_cfg *bk_cfg);
void unset_nw(enum network_type type);
bool get_backup_enable();
int set_ubus_curr_nw(enum network_type type);
char* get_ubus_curr_nw();
enum network_type get_curr_nw();
bool backup_auto_dial(const char* ifname);

#ifdef SUPPORT_VPN_CLIENT_IN_3G4G_MODULES
int set_vpn_client_parent(const char* ifname);
#endif


#endif
