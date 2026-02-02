#ifndef INCLUDE_SYSUTILITY_H
#define INCLUDE_SYSUTILITY_H




#define IFACE_FLAG_T 0x01
#define IP_ADDR_T 0x02
#define NET_MASK_T 0x04
#define HW_ADDR_T 0x08
#define HW_NAT_LIMIT_NETMASK 0xFFFFFF00 //for arp table 512 limitation,
//net mask at lease 255.255.255.0,or disable hw_nat
typedef enum { LAN_NETWORK=0, WAN_NETWORK } DHCPC_NETWORK_TYPE_T;
#ifndef _PATH_PROCNET_ROUTE
#define _PATH_PROCNET_ROUTE	"/proc/net/route"
#endif
#ifndef RTF_UP
#define RTF_UP			0x0001          /* route usable                 */
#endif
#ifndef RTF_GATEWAY
#define RTF_GATEWAY		0x0002          /* destination is a gateway     */
#endif

enum _WLAN_FLAG
{
	IS_EXIST=1,
	IS_RUN
};

typedef struct wapi_AsServer_conf {
		unsigned char valid;
		unsigned char wapi_cert_sel;
		char wapi_asip[4];
		char network_inf[128]; /*wlan0,wlan0-va0,.....*/
	} WAPI_ASSERVER_CONF_T, *WAPI_ASSERVER_CONF_Tp;
int setInAddr( char *interface, char *Ipaddr, char *Netmask, char *HwMac, int type);
int getInAddr( char *interface, int type, void *pAddr );
int DoCmd(char *const argv[], char *file);
int RunSystemCmd(char *filepath, ...);
int isFileExist(char *file_name);
int setPid_toFile(char *file_name);
int getPid_fromFile(char *file_name);
int if_readlist_proc(char *target, char *key, char *exclude);
char *get_name(char *name, char *p);
void string_casecade(char *dest, char *src);
int write_line_to_file(char *filename, int mode, char *line_data);
void Create_script(char *script_path, char *iface, int network, char *ipaddr, char *mask, char *gateway);
//unsigned char *gettoken(const unsigned char *str,unsigned int index,unsigned char symbol);
extern int find_pid_by_name( char* pidName);
void reinit_webs();
int getDefaultRoute(char *interface, struct in_addr *route);
int getDataFormFile(char* fileName,char* dataName,char* data,char number);
int killDaemonByPidFile(char *pidFile);
int changeDividerToESC(char *src,unsigned int size,const char*dividerChars);
void update_dns_resolv_file();
int get_wan_port_num(void);

int read_pid(const char *filename);
int is_interface_run(const char *interface);
int rtk_get_interface_flag(const char *ifname, int count, int flag);

#ifdef CONFIG_USER_RENICE_PROCESS
struct process_nice_table_entry {
	char processName[64];
	char pidFile[64];
	int nice;
};
void renice_processes();
#endif

#ifdef CONFIG_RTL_DNS_TRAP
#ifndef HOSTS_FILE
#define HOSTS_FILE "/var/hosts"
#endif
#ifndef DNS_PROC_FILE
#define DNS_PROC_FILE "/proc/rtl_dnstrap/domain_name"
#endif

void strtolower(char *str, int len);
int create_hosts_file(char* domain_name);
#endif

#endif


