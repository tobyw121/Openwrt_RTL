#ifdef FFS_SUPPORT

#include <stdbool.h>
#include "dnsmasq.h"
#include "libubox/list.h"
#include "uci.h"

#define FFS_FW_FORWARD_WHITE_CHAIN          "forward_lan_amazon_wifi"
#define DOMAIN_MAXLEN                       (255)
#define PROTO_MAXLEN                        (16)

typedef struct t_white_dns_list
{
    struct      list_head list;
    char        name[DOMAIN_MAXLEN];
}T_WHITE_DNS_LIST;

typedef struct t_white_proto_list
{
    struct      list_head list;
    char        proto[PROTO_MAXLEN];
}T_WHITE_PROTO_LIST;


struct t_ffs_config
{
    bool                ffs_enable;
    struct list_head    domain_list;
    struct list_head    proto_list;
    char               *multiports;
};

typedef int (* uci_config_get_cb)(char *);

static struct t_ffs_config ffs_config;

static int ffs_uci_get_config(const char *config, const char *type, const char *name,
                                    const char *option, uci_config_get_cb callback)
{
    struct uci_context  *ctx = NULL;
    struct uci_package  *pkg = NULL;
    struct uci_element  *element = NULL;
    char                *pvalue_date = NULL;
    int                  ret = 0;

    if (NULL == config || NULL == option || NULL == name || NULL == callback)
        return -1;
    
    ctx = uci_alloc_context();
    if (NULL == ctx)
        return -1;

    if (UCI_OK != uci_load(ctx, config, &pkg))
        goto cleanup;

    uci_foreach_element(&pkg->sections, element)
    {
        struct uci_section *sec = uci_to_section(element);
        if (NULL == sec)
            continue;
        
        if (type && (0 != strcmp(sec->type, type)))
            continue;

        if ((sec->anonymous == false && (0 != strcmp(sec->e.name, name))) || sec->anonymous == true)
            continue;
        
        struct uci_option *opt = uci_lookup_option(ctx, sec, option);
        if (NULL == opt)
            continue;

        if (UCI_TYPE_LIST == opt->type)
        {
            struct uci_element *list_e;

            uci_foreach_element(&opt->v.list, list_e)
            {
                ret = callback(list_e->name);
                if (ret != 0)
                    break;
            }
        }
        else if (UCI_TYPE_STRING == opt->type)
        {            
            ret = callback(opt->v.string);
            if (ret != 0)
                break;
        }

        break;
    }

    uci_unload(ctx, pkg);
cleanup:

    uci_free_context(ctx);
    ctx = NULL;

    return 0;
}


int ffs_domain_list_add(const char *name)
{
    T_WHITE_DNS_LIST   *ptnew = NULL;

    if(NULL == name)
        return -1;

    ptnew = safe_malloc(sizeof(T_WHITE_DNS_LIST));
    memset(ptnew, 0, sizeof(T_WHITE_DNS_LIST));

    strncpy(ptnew->name, name, DOMAIN_MAXLEN);

    list_add(&ptnew->list, &ffs_config.domain_list);

    return 0;
}

int ffs_proto_list_add(const char *proto)
{
    T_WHITE_PROTO_LIST   *ptnew = NULL;

    if(NULL == proto)
        return -1;

    ptnew = safe_malloc(sizeof(T_WHITE_PROTO_LIST));
    memset(ptnew, 0, sizeof(T_WHITE_PROTO_LIST));

    strncpy(ptnew->proto, proto, PROTO_MAXLEN);

    list_add(&ptnew->list, &ffs_config.proto_list);

    return 0;
}

int ffs_port_array_add(const char* port)
{
    char            *multiports = ffs_config.multiports;
    int              orig_len = multiports != NULL ? strlen(multiports) : 0;
    
    if (NULL == port)
        return -1;
                    
    ffs_config.multiports = multiports = realloc(multiports, orig_len + strlen(port) + 1);
    if (NULL == multiports)
        return -1;

    if (0 == orig_len)
        sprintf(multiports, port);
    else
        sprintf(multiports + orig_len, ",%s", port);

    return 0;
}

int ffs_enable_set(const char* enable)
{
    if (NULL == enable)
        return -1;

    if (!strcmp(enable, "on"))
        ffs_config.ffs_enable = true;
    else
        ffs_config.ffs_enable = false;
}



int ffs_init(void)
{
    memset(&ffs_config, 0, sizeof(struct t_ffs_config));

    ffs_config.ffs_enable = false;
    INIT_LIST_HEAD(&ffs_config.domain_list);    
    INIT_LIST_HEAD(&ffs_config.proto_list);
    ffs_config.multiports = NULL;
    
    return 0;
}
int ffs_fini(void)
{
    T_WHITE_DNS_LIST      *pos_dns = NULL, *n_dns = NULL;
    T_WHITE_PROTO_LIST    *pos_proto = NULL, *n_proto = NULL;

    list_for_each_entry_safe(pos_dns, n_dns, &ffs_config.domain_list, list)
    {
        list_del(&pos_dns->list);
        free(pos_dns);
    }

    list_for_each_entry_safe(pos_proto, n_proto, &ffs_config.proto_list, list)
    {
        list_del(&pos_proto->list);
        free(pos_proto);
    }

    free(ffs_config.multiports);

    return 0;
}

int ffs_dump(void)
{
    T_WHITE_DNS_LIST      *pos_dns = NULL;
    T_WHITE_PROTO_LIST    *pos_proto = NULL;

    list_for_each_entry(pos_dns, &ffs_config.domain_list, list)
    {
        my_syslog(LOG_INFO, _("ffs domain list : %s"), pos_dns->name);
    }

    list_for_each_entry(pos_proto, &ffs_config.proto_list, list)
    {
        my_syslog(LOG_INFO, _("ffs protocol list : %s"), pos_proto->proto);
    }

    my_syslog(LOG_INFO, _("ffs port list : %s"), ffs_config.multiports);

    my_syslog(LOG_INFO, _("ffs enable : %s"), ffs_config.ffs_enable == true ? "on" : "off");

    return 0;
}


static bool ffs_is_in_fw_chain(const char *ipaddr)
{
	FILE            *output = NULL;
    char             script[128] = {0};
    char             line[4] = {0};

    if (NULL == ipaddr)
    {
        my_syslog(LOG_ERR, _("parameter is NULL"));
        return true;
    }
    
    snprintf(script, sizeof(script), "iptables -t filter -v -n -L %s | grep -w -c %s", FFS_FW_FORWARD_WHITE_CHAIN, ipaddr);

    output = popen(script, "r");
    if (NULL == output)
    {
        my_syslog(LOG_ERR, _("popen failed"));
        return true;
    }
    
    if (NULL == fgets(line, sizeof(line), output))
    {
        my_syslog(LOG_ERR, _("fgets nothing"));
        pclose(output);
        return true;
    }

    if (atoi(line) > 0)
    {
        pclose(output);
        return true;
    }

    pclose(output);    
    return false;
}

static int ffs_add_white_rule(const char *ipaddr)
{
    char                 script[256] = {0};
    T_WHITE_PROTO_LIST  *pos = NULL;

    if (NULL == ipaddr)
    {
        my_syslog(LOG_ERR, _("parameter is NULL"));
        return -1;
    }

    list_for_each_entry(pos, &ffs_config.proto_list, list)
    {
        if (!strcmp(pos->proto, "tcp") || !strcmp(pos->proto, "udp"))
        {
            snprintf(script, sizeof(script), "iptables -t filter -I %s -d %s -p %s -m multiport --ports %s -j ACCEPT", 
                        FFS_FW_FORWARD_WHITE_CHAIN, ipaddr, pos->proto, ffs_config.multiports);
        }
        else 
        {
            snprintf(script, sizeof(script), "iptables -t filter -I %s -d %s -p %s -j ACCEPT", 
                        FFS_FW_FORWARD_WHITE_CHAIN, ipaddr, pos->proto);
        }
        
        my_syslog(LOG_INFO, _("run cmd: %s"), script);
        system(script);
    }
    
    return 0;
}

int ffs_dns_intercept_hook(const char *name, struct all_addr *addr)
{
    T_WHITE_DNS_LIST     *pos      = NULL;
    struct crec          *crecp    = NULL;
    bool                  find     = false;
    char                  ipaddr[INET6_ADDRSTRLEN] = {0};

    if (NULL == name || NULL == addr)
    {
        my_syslog(LOG_ERR, _("parameter is NULL"));
        return -1;
    }

    if (false == ffs_config.ffs_enable)
        return 0;
    
    /* 判断域名白名单 */    
    list_for_each_entry(pos, &ffs_config.domain_list, list)
    {
        if (hostname_isequal(name, pos->name))
        {
            find = true;
            break;
        }
    }

    if (!find)
        return;
    
    if ( NULL == inet_ntop(AF_INET, &addr->addr, ipaddr, INET6_ADDRSTRLEN))
        return;
    /* AF_INET6 to be conntinue */

    my_syslog(LOG_INFO, _("%s is in white list, ipaddr: %s"), name, ipaddr);

    /* 判断IP是否已加入iptables白名单 */
    if (ffs_is_in_fw_chain(ipaddr))
        return;
    
    /* 将新IP加入iptables白名单 */
    ffs_add_white_rule(ipaddr);

    return 0;
}

#endif /* DFFS_SUPPORT */
