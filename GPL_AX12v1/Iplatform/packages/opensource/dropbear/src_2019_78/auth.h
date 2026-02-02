/*
 * Dropbear - a SSH2 server
 * 
 * Copyright (c) 2002,2003 Matt Johnston
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. */

#ifndef DROPBEAR_AUTH_H_
#define DROPBEAR_AUTH_H_

#include "includes.h"
#include "signkey.h"
#include "chansession.h"

void svr_authinitialise(void);
void cli_authinitialise(void);

/* Server functions */
void recv_msg_userauth_request(void);
void send_msg_userauth_failure(int partial, int incrfail);
void send_msg_userauth_success(void);
void send_msg_userauth_banner(const buffer *msg);
void svr_auth_password();
void svr_auth_pubkey(int valid_user);
void svr_auth_pam(int valid_user);
void svr_auth_uci();
#ifdef FAILED_LOGIN_ATTEMPTS_LIMIT
int get_mac_from_arptable(char *mac_addr, int mac_len);
#endif

#if DROPBEAR_SVR_PUBKEY_OPTIONS_BUILT
int svr_pubkey_allows_agentfwd(void);
int svr_pubkey_allows_tcpfwd(void);
int svr_pubkey_allows_x11fwd(void);
int svr_pubkey_allows_pty(void);
void svr_pubkey_set_forced_command(struct ChanSess *chansess);
void svr_pubkey_options_cleanup(void);
int svr_add_pubkey_options(buffer *options_buf, int line_num, const char* filename);
#else
/* no option : success */
#define svr_pubkey_allows_agentfwd() 1
#define svr_pubkey_allows_tcpfwd() 1
#define svr_pubkey_allows_x11fwd() 1
#define svr_pubkey_allows_pty() 1
static inline void svr_pubkey_set_forced_command(struct ChanSess *chansess) { }
static inline void svr_pubkey_options_cleanup(void) { }
#define svr_add_pubkey_options(x,y,z) DROPBEAR_SUCCESS
#endif

/* Client functions */
void recv_msg_userauth_failure(void);
void recv_msg_userauth_success(void);
void recv_msg_userauth_specific_60(void);
void recv_msg_userauth_pk_ok(void);
void recv_msg_userauth_info_request(void);
void cli_get_user(void);
void cli_auth_getmethods(void);
int cli_auth_try(void);
void recv_msg_userauth_banner(void);
void cli_pubkeyfail(void);
void cli_auth_password(void);
int cli_auth_pubkey(void);
void cli_auth_interactive(void);
char* getpass_or_cancel(const char* prompt);
void cli_auth_pubkey_cleanup(void);


#define MAX_USERNAME_LEN 64 /* arbitrary for the moment */

#define AUTH_TYPE_NONE      1
#define AUTH_TYPE_PUBKEY    (1 << 1)
#define AUTH_TYPE_PASSWORD  (1 << 2)
#define AUTH_TYPE_INTERACT  (1 << 3)

#define AUTH_METHOD_NONE "none"
#define AUTH_METHOD_NONE_LEN 4
#define AUTH_METHOD_PUBKEY "publickey"
#define AUTH_METHOD_PUBKEY_LEN 9
#define AUTH_METHOD_PASSWORD "password"
#define AUTH_METHOD_PASSWORD_LEN 8
#define AUTH_METHOD_INTERACT "keyboard-interactive"
#define AUTH_METHOD_INTERACT_LEN 20

/* #define ALLOW_SERVICE_AUTH 1 */

#if defined(ALLOW_SERVICE_AUTH)
#define ALLOW_SERVICE_TYPENUM	1

#define SERVICE_MARK			0xfe

#define MAX_SFTP_USER_NUM		5

#define BIT_ONEMESH_PRI_SYNCONFIG_AUTH_MATCH	0B10000
#define BIT_ONEMESH_PUB_HANDSHAKE_AUTH_MATCH	0B1000
#define BIT_SFTP_AUTH_MATCH						0B100
#define	BIT_VOICE_AUTH_MATCH					0B010
#define BIT_TETHER_AUTH_MATCH					0B001

#define CHK_ONEMESH_PUB_HANDSHAKE_AUTH(x)		((x) &  BIT_ONEMESH_PUB_HANDSHAKE_AUTH_MATCH)
#define SET_ONEMESH_PUB_HANDSHAKE_AUTH(x)		((x) |= BIT_ONEMESH_PUB_HANDSHAKE_AUTH_MATCH)
#define CLR_ONEMESH_PUB_HANDSHAKE_AUTH(x)		((x) &= (~BIT_ONEMESH_PUB_HANDSHAKE_AUTH_MATCH))

#define CHK_ONEMESH_PRI_SYNCONFIG_AUTH(x)		((x) &  BIT_ONEMESH_PRI_SYNCONFIG_AUTH_MATCH)
#define SET_ONEMESH_PRI_SYNCONFIG_AUTH(x)		((x) |= BIT_ONEMESH_PRI_SYNCONFIG_AUTH_MATCH)
#define CLR_ONEMESH_PRI_SYNCONFIG_AUTH(x)		((x) &= (~BIT_ONEMESH_PRI_SYNCONFIG_AUTH_MATCH))

#define CHK_SFTP_AUTH(x)		((x) & BIT_SFTP_AUTH_MATCH)
#define CHK_VOICE_AUTH(x)		((x) & BIT_VOICE_AUTH_MATCH)
#define CHK_TETHER_AUTH(x)		((x) & BIT_TETHER_AUTH_MATCH)
#define SET_SFTP_AUTH(x)		((x) |= BIT_SFTP_AUTH_MATCH)
#define SET_VOICE_AUTH(x)		((x) |= BIT_VOICE_AUTH_MATCH)
#define SET_TETHER_AUTH(x)		((x) |= BIT_TETHER_AUTH_MATCH)
#define CLR_SFTP_AUTH(x)		((x) &= (~BIT_SFTP_AUTH_MATCH))
#define CLR_VOICE_AUTH(x)		((x) &= (~BIT_VOICE_AUTH_MATCH))
#define CLR_TETHER_AUTH(x)		((x) &= (~BIT_TETHER_AUTH_MATCH))
	
#define ADMIN_USER_NAME						"admin"
#define ONEMESH_PUB_HANDSHAKE_USER_NAME		"onemesh_pub_handshake"
#define ONEMESH_PRI_SYNCONFIG_USER_NAME		"onemesh_pri_synconfig"

#endif/* ALLOW_SERVICE_AUTH */
/* end added */

/* This structure is shared between server and client - it contains
 * relatively little extraneous bits when used for the client rather than the
 * server */
struct AuthState {
	char *username; /* This is the username the client presents to check. It
					   is updated each run through, used for auth checking */
	unsigned char authtypes; /* Flags indicating which auth types are still 
								valid */
	unsigned int failcount; /* Number of (failed) authentication attempts.*/
	unsigned int authdone; /* 0 if we haven't authed, 1 if we have. Applies for
							  client and server (though has differing 
							  meanings). */

	unsigned int perm_warn; /* Server only, set if bad permissions on 
							   ~/.ssh/authorized_keys have already been
							   logged. */
	unsigned int checkusername_failed;  /* Server only, set if checkusername
	                                has already failed */
	struct timespec auth_starttime; /* Server only, time of receiving current 
									SSH_MSG_USERAUTH_REQUEST */

	/* These are only used for the server */
	uid_t pw_uid;
	gid_t pw_gid;
	char *pw_dir;
	char *pw_shell;
	char *pw_name;
	char *pw_passwd;
#if DROPBEAR_SVR_PUBKEY_OPTIONS_BUILT
	struct PubKeyOptions* pubkey_options;
#endif

#if defined(ALLOW_SERVICE_AUTH)
	unsigned char srv_type;	//bit[0]: tether, bit[1]: voice app, bit[2]: sftp, and also can add more bit define here
#endif
};

#if DROPBEAR_SVR_PUBKEY_OPTIONS_BUILT
struct PubKeyOptions;
struct PubKeyOptions {
	/* Flags */
	int no_port_forwarding_flag;
	int no_agent_forwarding_flag;
	int no_x11_forwarding_flag;
	int no_pty_flag;
	/* "command=" option. */
	char * forced_command;
};
#endif

#endif /* DROPBEAR_AUTH_H_ */
