#ifndef __UDTDB_RUNOPTS_H__
#define __UDTDB_RUNOPTS_H__
/*
 * file		udtdb_runopts.h
 * brief	export functions and variables	
 * details	
 *
 * author	WangLian
 * version	1.0.0
 * date		29Jun17
 *
 * history 	\arg 1.0.0  29Jun17, wanglian, create the file	
 */

#include <time.h>

#include "options.h"
#include "config.h"


/**************************************************************************************************/
/*                                           DEFINES                                              */
/**************************************************************************************************/

/**************************************************************************************************/
/*                                           TYPES                                                */
/**************************************************************************************************/
typedef struct runopts {

#if defined(ENABLE_SVR_REMOTETCPFWD) || defined(ENABLE_CLI_LOCALTCPFWD)
	int listen_fwd_all;
#endif
	unsigned int recv_window;
	time_t keepalive_secs; /* Time between sending keepalives. 0 is off */
	time_t idle_timeout_secs; /* Exit if no traffic is sent/received in this time */

#ifndef DISABLE_ZLIB
	/* TODO: add a commandline flag. Currently this is on by default if compression
	 * is compiled in, but disabled for a client's non-final multihop stages. (The
	 * intermediate stages are compressed streams, so are uncompressible. */
	enum {
		DROPBEAR_COMPRESS_DELAYED, /* Server only */
		DROPBEAR_COMPRESS_ON,
		DROPBEAR_COMPRESS_OFF,
	} compress_mode;
#endif

#ifdef ENABLE_USER_ALGO_LIST
	char *cipher_list;
	char *mac_list;
#endif

} runopts;


typedef struct svr_runopts {

	char * bannerfile;

	int forkbg;
	int usingsyslog;

	/* ports is an array of the portcount listening ports */
	char *ports[DROPBEAR_MAX_PORTS];
	unsigned int portcount;
	char *addresses[DROPBEAR_MAX_PORTS];

	int inetdmode;

	/* Flags indicating whether to use ipv4 and ipv6 */
	/* not used yet
	int ipv4;
	int ipv6;
	*/

#ifdef DO_MOTD
	/* whether to print the MOTD */
	int domotd;
#endif

	int norootlogin;

	int noauthpass;
	int norootpass;
	int allowblankpass;

#ifdef ENABLE_SVR_REMOTETCPFWD
	int noremotetcp;
#endif
#ifdef ENABLE_SVR_LOCALTCPFWD
	int nolocaltcp;
#endif

	//void *hostkey; /* sign_key *hostkey; */

	int delay_hostkey;

	char *hostkey_files[MAX_HOSTKEYS];
	int num_hostkey_files;

	void * banner;  /* buffer * banner; */
	char * pidfile;

} svr_runopts;


/**************************************************************************************************/
/*                                           VARIABLES                                            */
/**************************************************************************************************/

extern runopts opts;
extern svr_runopts svr_opts;

/**************************************************************************************************/
/*                                           FUNCTIONS                                            */
/**************************************************************************************************/

void load_all_hostkeys();


#endif /* __UDTDB_RUNOPTS_H__ */

