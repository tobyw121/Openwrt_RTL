#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/wireless.h>

//#include <linux/if_arp.h>
#include <linux/if_ether.h>
//#include <linux/if_packet.h>

#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/sysinfo.h>

//#include "subr_wlan.h"
#include "common.h"
#include "wlan_manager.h"
#include "crossband_daemon.h"


char wlan0_ifName[IFNAMSIZ];
char wlan1_ifName[IFNAMSIZ];

struct mibValue mibValueOf5G ={
    .rssi_threshold = 15,
    .cu_threshold = 150,
    .noise_threshold = 50,
    .rssi_weight = 2,
    .cu_weight = 3,
    .noise_weight = 2
    };

struct mibValue mibValueOf2G = {
    .rssi_threshold = 5,
    .cu_threshold = 100,
    .noise_threshold = 30,
    .rssi_weight = 4,
    .cu_weight = 12,
    .noise_weight = 5
    };

struct envinfo_data recordOf5G = {
    .rssi_metric = 0,
    .cu_metric = 0,
    .noise_metric = 0
    };

struct envinfo_data recordOf2G = {
    .rssi_metric = 0,
    .cu_metric = 0,
    .noise_metric = 0
    };

//static unsigned int cross_band_index;
struct mibValue* mibValuePointer = &mibValueOf5G;
struct envinfo_data* envinfo_dataPointer = &recordOf5G;
unsigned char not_prefer = 0;
unsigned char prefer = 1;
unsigned char wlan0_is5G = 0;
unsigned char wlan1_is5G = 0;
unsigned int  wlan0_ax_support = 0;
unsigned int  wlan1_ax_support = 0;
unsigned int wlan0_bandRating = 0;
unsigned int wlan1_bandRating = 0;
unsigned char tempbuf[16384];
int crossband_enable = 0;
static unsigned long last_path_switch_time=0;
const char *crossband_enable_mib = "crossband_enable";
const char *pathready_mib = "crossband_pathReady";
const char *assoc_mib = "crossband_assoc";
const char *preferband_mib = "crossband_preferBand";

#if 0
static int pidfile_acquire(char *pidfile)
{
    int pid_fd;

    if(pidfile == NULL)
        return -1;

    pid_fd = open(pidfile, O_CREAT | O_WRONLY, 0644);
    if (pid_fd < 0)
        printf("Unable to open pidfile %s\n", pidfile);
    else
        lockf(pid_fd, F_LOCK, 0);

    return pid_fd;
}

static void pidfile_write_release(int pid_fd)
{
    FILE *out;

    if(pid_fd < 0)
        return;

    if((out = fdopen(pid_fd, "w")) != NULL)
    {
        fprintf(out, "%d\n", getpid());
        fclose(out);
    }
    lockf(pid_fd, F_UNLCK, 0);
    close(pid_fd);
}
#endif

static int crossband_set_mib(const char *interfacename, unsigned int is_ax_support, const char *mibname, char value)
{
    int skfd, cmd_id;
    struct iwreq wrq;
    char tmp[30];

    sprintf(tmp, "%s=%d", mibname, value);

    if((skfd = socket(AF_INET, SOCK_DGRAM, 0)) <0)
    {
        printf("[%s %d] socket error \n", __FUNCTION__, __LINE__);
        return -1;
    }

    /* Set device name */
    memset(&wrq, 0, sizeof(wrq));
    strncpy(wrq.ifr_name, interfacename, IFNAMSIZ);

    if(is_ax_support)
    {
        wrq.u.data.flags = RTL8192CD_IOCTL_SET_MIB;
        cmd_id = SIOCDEVPRIVATEAXEXT;
    }
    else
    {
        cmd_id = RTL8192CD_IOCTL_SET_MIB;
    }

    wrq.u.data.pointer = (caddr_t)tmp;
    wrq.u.data.length = strlen((char *)tmp)+1;

    /* Do the request */
    if(ioctl(skfd, cmd_id, &wrq) < 0)
    {
        printf("[%s %d] ioctl[RTL8192CD_IOCTL_SET_MIB]", __FUNCTION__, __LINE__);
        close(skfd);
        return -1;
    }

    close(skfd);
    return 0;
}


static int crossband_get_mib(const char *interfacename, unsigned int is_ax_support, const char* mibname ,void *result,int size)
{
    int skfd, cmd_id;
    struct iwreq wrq;
    unsigned char tmp[32];

    if((skfd = socket(AF_INET, SOCK_DGRAM, 0)) <0)
    {
        printf("[%s %d] socket error \n", __FUNCTION__, __LINE__);
        return -1;
    }

    /* Set device name */
	strncpy(wrq.ifr_name, interfacename, IFNAMSIZ);
	memset(tmp, 0, sizeof(tmp));
    strncpy((char *)tmp,mibname,strlen(mibname));

    if(is_ax_support)
    {
        wrq.u.data.flags = RTL8192CD_IOCTL_GET_MIB;
        cmd_id = SIOCDEVPRIVATEAXEXT;
    }
    else
    {
        cmd_id = RTL8192CD_IOCTL_GET_MIB;
    }
    wrq.u.data.pointer = tmp;
    wrq.u.data.length = strlen((char *)tmp);

    /* Do the request */
    if(ioctl(skfd, cmd_id, &wrq) < 0)
    {
        printf("[%s %d] ioctl[RTL8192CD_IOCTL_GET_MIB]", __FUNCTION__, __LINE__);
        close(skfd);
        return -1;
    }

    close(skfd);
    if(size)
    {
        memcpy(result,tmp, size);
    }
    else
        strncpy(result, (char *)tmp, sizeof(tmp));
    return 0;
}

static int get_env_info(const char *ifname, unsigned int is_ax_support, struct envinfo_data* pointer)
{
    int sock, cmd_id;
    struct iwreq wrq;
    int ret = -1;
    int err;

    /*** Inizializzazione socket ***/
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        err = errno;
        printf("[%s %d]: Can't create socket for ioctl. %s(%d)", __FUNCTION__, __LINE__, ifname, err);
        goto out;
    }

    /*** Inizializzazione struttura iwreq ***/
    memset((void *)pointer, 0, sizeof(struct envinfo_data));
    strncpy(wrq.ifr_name, ifname, IFNAMSIZ);

	if(is_ax_support)
    {
        wrq.u.data.flags = SIOCROSSBANDINFOREQ;
        cmd_id = SIOCDEVPRIVATEAXEXT;
    }
    else
    {
        cmd_id = SIOCROSSBANDINFOREQ;
    }

    /*** give parameter and buffer ***/
    wrq.u.data.pointer = (caddr_t)pointer;
    wrq.u.data.length = sizeof(struct envinfo_data);

    /*** ioctl ***/
    if(ioctl(sock, cmd_id, &wrq) < 0)
    {
        err = errno;
        printf("[%s %d]: ioctl Error.%s(%d)", __FUNCTION__, __LINE__, ifname, err);
        goto out;
    }
    ret = 0;

out:
    close(sock);
    return ret;
}

#if 0
static int initialize_mibValues( const char* interfacename, struct mibValue* pointer)
{
    if(crossband_set_mib(interfacename, cuThreshold_mib, pointer->cu_threshold ) == -1 )
        goto error;
    if(crossband_set_mib(interfacename, noiseThreshold_mib, pointer->noise_threshold ) == -1 )
        goto error;
    if(crossband_set_mib(interfacename, rssiThreshold_mib, pointer->rssi_threshold ) == -1 )
        goto error;
    if(crossband_set_mib(interfacename, cuWeight_mib, pointer->cu_weight ) == -1 )
        goto error;
    if(crossband_set_mib(interfacename, rssiWeight_mib, pointer->rssi_weight ) == -1 )
        goto error;
    if(crossband_set_mib(interfacename, noiseWeight_mib, pointer->noise_weight ) == -1 )
        goto error;

error:
    return -1;
}

static int retrieve_mibValues( const char* interfacename, struct mibValue* pointer)
{
    if(crossband_get_mib(interfacename, cuThreshold_mib, &(pointer->cu_threshold), 1) == -1 )
        goto error;
    if(crossband_get_mib(interfacename, noiseThreshold_mib, &(pointer->noise_threshold), 1) == -1 )
        goto error;
    if(crossband_get_mib(interfacename, rssiThreshold_mib, &(pointer->rssi_threshold), 1) == -1 )
        goto error;
    if(crossband_get_mib(interfacename, cuWeight_mib, &(pointer->cu_weight), 1) == -1 )
        goto error;
    if(crossband_get_mib(interfacename, noiseWeight_mib, &(pointer->noise_weight), 1) == -1 )
        goto error;
    if(crossband_get_mib(interfacename, rssiWeight_mib, &(pointer->rssi_weight), 1) == -1 )
        goto error;

error:
    return -1;
}
#endif

static int retrieve_drivermibValues( const char* interfacename, unsigned int is_ax_support, struct driver_mib_info* pointer)
{
    if(crossband_get_mib(interfacename, is_ax_support, crossband_enable_mib, &(pointer->enable), 1) == -1 )
        goto error;
    if(crossband_get_mib(interfacename, is_ax_support, pathready_mib, &(pointer->pathReady), 1) == -1 )
        goto error;
    if(crossband_get_mib(interfacename, is_ax_support, assoc_mib, &(pointer->assoc), 1) == -1 )
        goto error;
    if(crossband_get_mib(interfacename, is_ax_support, preferband_mib, &(pointer->preferband), 1) == -1 )
        goto error;

error:
    return -1;
}

static int calculate_bandrating(struct envinfo_data* data, struct mibValue* pointer, unsigned int* bandRating)
{

    unsigned int rssiScore = (data->rssi_metric < pointer->rssi_threshold)?(100 - data->rssi_metric)<<2:(100 - data->rssi_metric);
    unsigned int cuScore = (data->cu_metric > pointer->cu_threshold)?(data->cu_metric<<1):data->cu_metric;
    unsigned int noiseScore = (data->cu_metric > pointer->cu_threshold
        && data->noise_metric > pointer->noise_threshold)?data->noise_metric:0;

    *bandRating = (rssiScore * pointer->rssi_weight) + (cuScore * pointer->cu_weight) +
        (noiseScore * pointer->noise_weight);

	return 0;
}

static inline int
iw_get_ext(int                  skfd,           /* Socket to the kernel */
           const char *               ifname,         /* Device name */
           int                  request,        /* WE ID */
           struct iwreq *       pwrq)           /* Fixed part of the request */
{
  /* Set device name */
  strncpy(pwrq->ifr_name, ifname, IFNAMSIZ);
  /* Do the request */
  return(ioctl(skfd, request, pwrq));
}

static int rtk_wlan_get_wlan_sta_num( char *interface, int *num, unsigned int is_ax_support )
{
    int skfd=0, cmd_id;
    unsigned short staNum;
    struct iwreq wrq;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd==-1)
		return -1;

    /* Get wireless name */
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0){
      /* If no wireless name : no wireless extensions */
      close( skfd );
      return -1;
	}

    if(is_ax_support)
    {
        wrq.u.data.flags = SIOCGIWRTLSTANUM;
        cmd_id = SIOCDEVPRIVATEAXEXT;
    }
    else
    {
        cmd_id = SIOCGIWRTLSTANUM;
    }

    wrq.u.data.pointer = (caddr_t)&staNum;
    wrq.u.data.length = sizeof(staNum);

    if (iw_get_ext(skfd, interface, cmd_id, &wrq) < 0){
    	 close( skfd );
    	 return -1;
	}
    *num  = (int)staNum;

    close( skfd );

    return 0;
}

void do_crossband(void)
{
	char wlan0_vxd_ifName[IFNAMSIZ]={0}, wlan1_vxd_ifName[IFNAMSIZ]={0};
	int sta_num=0, sta_num1=0;
	struct sysinfo s_info;
	struct driver_mib_info wlan0_mib, wlan1_mib;

	if(!crossband_enable)
		return;

	sysinfo(&s_info);
	if(s_info.uptime >= last_path_switch_time + PATH_SWITCH_INTERVAL)
	{
		if(wlan0_is5G){
			envinfo_dataPointer = &recordOf5G;
			get_env_info(wlan0_ifName, wlan0_ax_support, envinfo_dataPointer);
			mibValuePointer = &mibValueOf5G;
//			retrieve_mibValues(wlan0_ifName, &mibValueOf5G);
			calculate_bandrating(envinfo_dataPointer, mibValuePointer, &(wlan0_bandRating));

			envinfo_dataPointer = &recordOf2G;
			get_env_info(wlan1_ifName, wlan1_ax_support, envinfo_dataPointer);
			mibValuePointer = &mibValueOf2G;
//			retrieve_mibValues(wlan1_ifName, &mibValueOf2G);
			calculate_bandrating(envinfo_dataPointer, mibValuePointer, &(wlan1_bandRating));
		}
		else{
			envinfo_dataPointer = &recordOf2G;
			get_env_info(wlan0_ifName, wlan0_ax_support, envinfo_dataPointer);
			mibValuePointer = &mibValueOf2G;
// 			retrieve_mibValues(wlan0_ifName, &mibValueOf2G);
			calculate_bandrating(envinfo_dataPointer, mibValuePointer, &(wlan0_bandRating));

			envinfo_dataPointer = &recordOf5G;
			get_env_info(wlan1_ifName, wlan1_ax_support, envinfo_dataPointer);
			mibValuePointer = &mibValueOf5G;
//			retrieve_mibValues(wlan1_ifName, &mibValueOf5G);
			calculate_bandrating(envinfo_dataPointer, mibValuePointer, &(wlan1_bandRating));
		}

#if 0
		memset(wlan0_vxd_ifName, 0, sizeof(wlan0_vxd_ifName));
		memset(wlan1_vxd_ifName, 0, sizeof(wlan1_vxd_ifName));
		snprintf(wlan0_vxd_ifName,sizeof(wlan0_vxd_ifName),VXD_IF,0);
		snprintf(wlan1_vxd_ifName,sizeof(wlan1_vxd_ifName),VXD_IF,1);
		rtk_wlan_get_wlan_sta_num(wlan0_vxd_ifName,&sta_num,wlan0_ax_support);
		rtk_wlan_get_wlan_sta_num(wlan1_vxd_ifName,&sta_num1,wlan1_ax_support);

		if(sta_num == 0)
			wlan0_bandRating = 65535;

		if(sta_num1 == 0)
			wlan1_bandRating = 65535;
#else
		memset(&wlan0_mib, 0, sizeof(struct driver_mib_info));
		memset(&wlan1_mib, 0, sizeof(struct driver_mib_info));
		retrieve_drivermibValues(wlan0_ifName, wlan0_ax_support, &wlan0_mib);
		retrieve_drivermibValues(wlan1_ifName, wlan1_ax_support, &wlan1_mib);

		if(wlan0_mib.assoc == 0)
			wlan0_bandRating = 65535;

		if(wlan1_mib.assoc == 0)
			wlan1_bandRating = 65535;
#endif

#if 1
		printf("5G\n");
		printf("rssiT:%u cuT:%u noiseT:%u\n", mibValueOf5G.rssi_threshold, mibValueOf5G.cu_threshold, mibValueOf5G.noise_threshold);
		printf("rssiW:%u cuW:%u noiseW:%u\n", mibValueOf5G.rssi_weight, mibValueOf5G.cu_weight, mibValueOf5G.noise_weight);
		printf("rssiM:%u cuM:%u noiseM:%u\n\n", recordOf5G.rssi_metric, recordOf5G.cu_metric, recordOf5G.noise_metric);

		printf("2G\n");
		printf("rssiT:%u cuT:%u noiseT:%u\n", mibValueOf2G.rssi_threshold, mibValueOf2G.cu_threshold, mibValueOf2G.noise_threshold);
		printf("rssiW:%u cuW:%u noiseW:%u\n", mibValueOf2G.rssi_weight, mibValueOf2G.cu_weight, mibValueOf2G.noise_weight);
		printf("rssiM:%u cuM:%u noiseM:%u\n\n", recordOf2G.rssi_metric, recordOf2G.cu_metric, recordOf2G.noise_metric);

		printf("wlan0 interface band rating: %u\n", wlan0_bandRating);
		printf("wlan1 interface band rating: %u\n\n", wlan1_bandRating);
#endif
		if(wlan0_bandRating < wlan1_bandRating){
			crossband_set_mib(wlan0_ifName, wlan0_ax_support, preferband_mib, prefer);
			crossband_set_mib(wlan1_ifName, wlan1_ax_support, preferband_mib, not_prefer);
		}
		else if(wlan0_bandRating > wlan1_bandRating){
			crossband_set_mib(wlan0_ifName, wlan0_ax_support, preferband_mib, not_prefer);
			crossband_set_mib(wlan1_ifName, wlan1_ax_support, preferband_mib, prefer);
		}
		last_path_switch_time = s_info.uptime;
    }
}

static void crossband_daemon_config_fill(const s8 *buf, s8 *pos)
{
	/* TBD: should check the data type and data size */

	struct mibValue *mibPointerwlan0 = NULL;
	struct mibValue *mibPointerwlan1 = NULL;

	if(wlan0_is5G)
	{
		mibPointerwlan0 = &mibValueOf5G;
		mibPointerwlan1 = &mibValueOf2G;
	}
	else
	{
		mibPointerwlan0 = &mibValueOf2G;
		mibPointerwlan1 = &mibValueOf5G;
	}
	if (!os_strcmp(buf, "crossband_enable")) {
		if (!os_strcmp(pos, "0")) {
			crossband_enable = 0;
		}
		else if (!os_strcmp(pos, "1")) {
			crossband_enable = 1;
		}
		else {
			MSG_WARN(C_BAND_STR, "invalid config: %s.");
		}
	}
	else if (!os_strcmp(buf, "wlan0_ifname")) {
		snprintf(wlan0_ifName, sizeof(wlan0_ifName), "%s", pos);
	}
	else if (!os_strcmp(buf, "wlan0_phyband_select")) {
		wlan0_is5G = atoi(pos);
	}
	else if (!os_strcmp(buf, "wlan0_ax_support")) {
		wlan0_ax_support = atoi(pos);
	}
	else if (!os_strcmp(buf, "wlan0_rssi_thres")) {
		mibPointerwlan0->rssi_threshold = atoi(pos);
	}
	else if (!os_strcmp(buf, "wlan0_cu_thres")) {
		mibPointerwlan0->cu_threshold = atoi(pos);
	}
	else if (!os_strcmp(buf, "wlan0_noise_thres")) {
		mibPointerwlan0->noise_threshold = atoi(pos);
	}
	else if (!os_strcmp(buf, "wlan0_rssi_weight")) {
		mibPointerwlan0->rssi_weight = atoi(pos);
	}
	else if (!os_strcmp(buf, "wlan0_cu_weight")) {
		mibPointerwlan0->cu_weight = atoi(pos);
	}
	else if (!os_strcmp(buf, "wlan0_noise_weight")) {
		mibPointerwlan0->noise_weight = atoi(pos);
	}
	else if (!os_strcmp(buf, "wlan1_ifname")) {
		snprintf(wlan1_ifName, sizeof(wlan1_ifName), "%s", pos);
	}
	else if (!os_strcmp(buf, "wlan1_phyband_select")) {
		wlan1_is5G = atoi(pos);
	}
	else if (!os_strcmp(buf, "wlan1_ax_support")) {
		wlan1_ax_support = atoi(pos);
	}
	else if (!os_strcmp(buf, "wlan1_rssi_thres")) {
		mibPointerwlan1->rssi_threshold = atoi(pos);
	}
	else if (!os_strcmp(buf, "wlan1_cu_thres")) {
		mibPointerwlan1->cu_threshold = atoi(pos);
	}
	else if (!os_strcmp(buf, "wlan1_noise_thres")) {
		mibPointerwlan1->noise_threshold = atoi(pos);
	}
	else if (!os_strcmp(buf, "wlan1_rssi_weight")) {
		mibPointerwlan1->rssi_weight = atoi(pos);
	}
	else if (!os_strcmp(buf, "wlan1_cu_weight")) {
		mibPointerwlan1->cu_weight = atoi(pos);
	}
	else if (!os_strcmp(buf, "wlan1_noise_weight")) {
		mibPointerwlan1->noise_weight = atoi(pos);
	}

	return;
}

static void crossband_daemon_config_read(const s8 *fname)
{
	FILE *fp;
	s8 buf[4096];
	s8 *pos;

	fp = fopen(fname, "r");
	if (fp == NULL)
		return;

	while (fgets(buf, sizeof(buf), fp)) {
		if (buf[0] == '#')
			continue;

		pos = buf;
		while (*pos != '\0') {
			if (*pos == '\n') {
				*pos = '\0';
				break;
			}
			pos++;
		}
		if (buf[0] == '\0')
			continue;

		pos = os_strchr(buf, '=');
		if (pos == NULL)
			continue;

		*pos = '\0';
		pos++;
		crossband_daemon_config_fill(buf, pos);
	}

	fclose(fp);

	return;
}

void _crossband_daemon_on_config_update(s8 *config)
{
	crossband_daemon_config_read(config);

	return;
}

static void send_crossband_dump_cmd_msg(void)
{
	u32 msg_len = 0;
	struct nl_message msg = {0};
	struct elm_header hdr = {0};
	struct elm_buffer buffer = {0};

	/* element header */
	hdr.id = ELM_BUFFER_ID;
	hdr.len = ELM_BUFFER_LEN;
	_wlan_manager_set_msg(&msg, &msg_len, (void *)&hdr, ELM_HEADER_LEN);

	/* element: ELM_BUFFER_ID */
	_wlan_manager_set_msg(&msg, &msg_len, (void *)&buffer, ELM_BUFFER_LEN);

	/* finish message */
	msg.type = NL_CROSSBAND_DUMP_TYPE;
	msg.len = msg_len;

	/* length += (type + len) */
	msg_len = msg.len + 8;
	_wlan_manager_send_daemon((void *)&msg, msg_len, NL_WLAN_MANAGER_PID);

	return;
}

void config_crossband_dump_parse_arg(u8 *argn, s32 argc, s8 *argv[])
{
	send_crossband_dump_cmd_msg();
	return;
}

void _crossband_daemon_on_cmd(void)
{
	s8 sys_cmd[64] = {0};
	FILE *fp;
	struct driver_mib_info wlan0_mib, wlan1_mib;
	struct mibValue *mibPointerwlan0 = NULL;
	struct mibValue *mibPointerwlan1 = NULL;
	struct envinfo_data *envinfoPointerwlan0 = NULL;
	struct envinfo_data *envinfoPointerwlan1 = NULL;
	struct sysinfo s_info;

	if(wlan0_is5G)
	{
		mibPointerwlan0 = &mibValueOf5G;
		mibPointerwlan1 = &mibValueOf2G;
		envinfoPointerwlan0 = &recordOf5G;
		envinfoPointerwlan1 = &recordOf2G;
	}
	else
	{
		mibPointerwlan0 = &mibValueOf2G;
		mibPointerwlan1 = &mibValueOf5G;
		envinfoPointerwlan0 = &recordOf2G;
		envinfoPointerwlan1 = &recordOf5G;
	}

	memset(&wlan0_mib, 0, sizeof(struct driver_mib_info));
	memset(&wlan1_mib, 0, sizeof(struct driver_mib_info));
	retrieve_drivermibValues(wlan0_ifName, wlan0_ax_support, &wlan0_mib);
	retrieve_drivermibValues(wlan1_ifName, wlan1_ax_support, &wlan1_mib);

	sysinfo(&s_info);
	fp = fopen(CROSSBAND_DAEMON_OUTPUT, "w");
	if (fp == NULL) {
		MSG_WARN(C_BAND_STR, "can't open [%s].", CROSSBAND_DAEMON_OUTPUT);
		return;
	}

	fprintf(fp, "[CROSSBAND] dump_info.\n");
	fprintf(fp, "crossband_enable: %d\n", crossband_enable);
	fprintf(fp, "last_path_switch_time: %ld\n", last_path_switch_time);
	fprintf(fp, "system_uptime: %ld\n", s_info.uptime);
	fprintf(fp, "\n");
	/* wlan0 */
	fprintf(fp, "wlan0 info:\n");
	fprintf(fp, "wlan0_ifname: %s\n", wlan0_ifName);
	fprintf(fp, "wlan0_enable: %u\n", wlan0_mib.enable);
	fprintf(fp, "wlan0_pathready: %u\n", wlan0_mib.pathReady);
	fprintf(fp, "wlan0_assoc: %u\n", wlan0_mib.assoc);
	fprintf(fp, "wlan0_preferband: %u\n", wlan0_mib.preferband);
	fprintf(fp, "wlan0_rssi_thres: %u\n", mibPointerwlan0->rssi_threshold);
	fprintf(fp, "wlan0_cu_thres: %u\n", mibPointerwlan0->cu_threshold);
	fprintf(fp, "wlan0_noise_thres: %u\n", mibPointerwlan0->noise_threshold);
	fprintf(fp, "wlan0_rssi_weight: %u\n", mibPointerwlan0->rssi_weight);
	fprintf(fp, "wlan0_cu_weight: %u\n", mibPointerwlan0->cu_weight);
	fprintf(fp, "wlan0_noise_weight: %u\n", mibPointerwlan0->noise_weight);
	fprintf(fp, "wlan0_rssi_metric: %u\n", envinfoPointerwlan0->rssi_metric);
	fprintf(fp, "wlan0_cu_metric: %u\n", envinfoPointerwlan0->cu_metric);
	fprintf(fp, "wlan0_noise_metric: %u\n", envinfoPointerwlan0->noise_metric);
	fprintf(fp, "\n");
	/* wlan1 */
	fprintf(fp, "wlan1 info:\n");
	fprintf(fp, "wlan1_ifname: %s\n", wlan1_ifName);
	fprintf(fp, "wlan1_enable: %u\n", wlan1_mib.enable);
	fprintf(fp, "wlan1_pathready: %u\n", wlan1_mib.pathReady);
	fprintf(fp, "wlan1_assoc: %u\n", wlan1_mib.assoc);
	fprintf(fp, "wlan1_preferband: %u\n", wlan1_mib.preferband);
	fprintf(fp, "wlan1_rssi_thres: %u\n", mibPointerwlan1->rssi_threshold);
	fprintf(fp, "wlan1_cu_thres: %u\n", mibPointerwlan1->cu_threshold);
	fprintf(fp, "wlan1_noise_thres: %u\n", mibPointerwlan1->noise_threshold);
	fprintf(fp, "wlan1_rssi_weight: %u\n", mibPointerwlan1->rssi_weight);
	fprintf(fp, "wlan1_cu_weight: %u\n", mibPointerwlan1->cu_weight);
	fprintf(fp, "wlan1_noise_weight: %u\n", mibPointerwlan1->noise_weight);
	fprintf(fp, "wlan1_rssi_metric: %u\n", envinfoPointerwlan1->rssi_metric);
	fprintf(fp, "wlan1_cu_metric: %u\n", envinfoPointerwlan1->cu_metric);
	fprintf(fp, "wlan1_noise_metric: %u\n", envinfoPointerwlan1->noise_metric);
	fprintf(fp, "--------------------------\n");

	fclose(fp);

	sprintf(sys_cmd, "cat %s", CROSSBAND_DAEMON_OUTPUT);
	system(sys_cmd);

	return;
}

int crossband_init(struct com_device *device)
{
	memset(wlan0_ifName, 0, sizeof(wlan0_ifName));
	memset(wlan1_ifName, 0, sizeof(wlan0_ifName));
#if 0
	snprintf(wlan0_ifName, sizeof(wlan0_ifName), WLAN_IF, 0);
	snprintf(wlan1_ifName, sizeof(wlan1_ifName), WLAN_IF, 1);

	mib_get_s(MIB_WLAN_PHY_BAND_SELECT, (void *)&wlan0_is5G, sizeof(wlan0_is5G));
	mib_get_s(MIB_WLAN1_PHY_BAND_SELECT, (void *)&wlan1_is5G, sizeof(wlan0_is5G));

	if(wlan0_is5G != PHYBAND_5G && wlan1_is5G != PHYBAND_5G)
		return -1;
#endif
	wlan0_bandRating = 0;
    wlan1_bandRating = 0;
	last_path_switch_time = 0;
	crossband_enable = 0;
	crossband_daemon_config_read(device->config_fname);
	printf("Crossband daemon - Mib initialization successful\n");

	return 0;
}
