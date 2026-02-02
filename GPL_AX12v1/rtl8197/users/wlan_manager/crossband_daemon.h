#ifndef _CROSSBAND_DAEMON_H_
#define _CROSSBAND_DAEMON_H_

#define CROSSBAND_DAEMON_OUTPUT						"/var/run/crossband_daemon.out"
#define C_BAND_STR									"[CROSSBAND_DAEMON]"
#define RTL8192CD_IOCTL_SET_MIB                 0x89f1 //set mib 0x89f1
#define RTL8192CD_IOCTL_GET_MIB                0x89f2  // get mib
#define SIOCROSSBANDINFOREQ             0x8CFC //get information
#define SIOCGIWRTLSTANUM				0x8B31  // get the number of stations in table
#define SIOCDEVPRIVATEAXEXT 0x89f2 //SIOCDEVPRIVATE+2

#define PHYBAND_2G                     1
#define PHYBAND_5G                     2

//#define INFO_RECORD_INTERVAL            1
#define PATH_SWITCH_INTERVAL           30

struct mibValue{
    unsigned char rssi_threshold;
    unsigned char cu_threshold;
    unsigned char noise_threshold;
    unsigned char rssi_weight;
    unsigned char cu_weight;
    unsigned char noise_weight;
};

struct envinfo_data{
    unsigned int rssi_metric;
    unsigned int cu_metric;
    unsigned int noise_metric;
};

struct driver_mib_info{
	unsigned char enable;
	unsigned char pathReady;
	unsigned char assoc;
	unsigned char preferband;
};

#if 0
static char *pidfile = "/var/run/crossband.pid";

const char *preferband_mib = "crossband_preferBand";
const char *cuThreshold_mib = "crossband_cuThreshold";
const char *rssiThreshold_mib = "crossband_rssiThreshold";
const char *noiseThreshold_mib = "crossband_noiseThreshold";
const char *cuWeight_mib = "crossband_cuWeight";
const char *rssiWeight_mib = "crossband_rssiWeight";
const char *noiseWeight_mib = "crossband_noiseWeight";
#endif



/* ------------------------------------------- */
/* ---------- APIs for wlan_manager ---------- */
/* ------------------------------------------- */
void do_crossband(void);
int crossband_init(struct com_device *device);
void _crossband_daemon_on_config_update(s8 *config);
void _crossband_daemon_on_cmd(void);
void config_crossband_dump_parse_arg(u8 *argn, s32 argc, s8 *argv[]);

#endif

