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

/* This file (auth.c) handles authentication requests, passing it to the
 * particular type (auth-passwd, auth-pubkey). */

#include "includes.h"
#include "dbutil.h"
#include "session.h"
#include "buffer.h"
#include "ssh.h"
#include "packet.h"
#include "auth.h"
#include "runopts.h"
#include "random.h"
#ifdef FAILED_LOGIN_ATTEMPTS_LIMIT
#include <linux/kernel.h>
#include <sys/sysinfo.h>
#endif

static void authclear();
static int checkusername(unsigned char *username, unsigned int userlen);
static void send_msg_userauth_banner();

/* initialise the first time for a session, resetting all parameters */
void svr_authinitialise() {

	ses.authstate.failcount = 0;
	ses.authstate.pw_name = NULL;
	ses.authstate.pw_dir = NULL;
	ses.authstate.pw_shell = NULL;
	ses.authstate.pw_passwd = NULL;
#if ALLOW_SERVICE_AUTH
	ses.authstate.srv_type = 0;
#endif
	authclear();
	
}

/* Reset the auth state, but don't reset the failcount. This is for if the
 * user decides to try with a different username etc, and is also invoked
 * on initialisation */
static void authclear() {
	
	memset(&ses.authstate, 0, sizeof(ses.authstate));
#ifdef ENABLE_SVR_PUBKEY_AUTH
	ses.authstate.authtypes |= AUTH_TYPE_PUBKEY;
#endif
#if defined(ENABLE_SVR_PASSWORD_AUTH) || defined(ENABLE_SVR_PAM_AUTH)
	if (!svr_opts.noauthpass) {
		ses.authstate.authtypes |= AUTH_TYPE_PASSWORD;
	}
#endif
	if (ses.authstate.pw_name) {
		m_free(ses.authstate.pw_name);
	}
	if (ses.authstate.pw_shell) {
		m_free(ses.authstate.pw_shell);
	}
	if (ses.authstate.pw_dir) {
		m_free(ses.authstate.pw_dir);
	}
	if (ses.authstate.pw_passwd) {
		m_free(ses.authstate.pw_passwd);
	}
#if ALLOW_SERVICE_AUTH
	ses.authstate.srv_type = 0;
#endif
}

/* Send a banner message if specified to the client. The client might
 * ignore this, but possibly serves as a legal "no trespassing" sign */
static void send_msg_userauth_banner() {

	TRACE(("enter send_msg_userauth_banner"))
	if (svr_opts.banner == NULL) {
		TRACE(("leave send_msg_userauth_banner: banner is NULL"))
		return;
	}

	CHECKCLEARTOWRITE();

	buf_putbyte(ses.writepayload, SSH_MSG_USERAUTH_BANNER);
	buf_putstring(ses.writepayload, buf_getptr(svr_opts.banner,
				svr_opts.banner->len), svr_opts.banner->len);
	buf_putstring(ses.writepayload, "en", 2);

	encrypt_packet();
	buf_free(svr_opts.banner);
	svr_opts.banner = NULL;

	TRACE(("leave send_msg_userauth_banner"))
}

#ifdef FAILED_LOGIN_ATTEMPTS_LIMIT
/* get uptime of system */
static long tick_get()
{
	struct sysinfo info;
	
	memset(&info, 0, sizeof(struct sysinfo));
	sysinfo(&info);
	
	return info.uptime;
}

/* get mac from arp table by ip */
int get_mac_from_arptable(char *mac_addr, int mac_len)
{
	char ip[SSH2_MAX_IP_LEN]           = {0};
	char hw_type[SSH2_MAX_HW_TYPE_LEN] = {0};
	char flags[SSH2_MAX_FLAGS_LEN]     = {0};
	char hw_addr[SSH2_MAX_MAC_LEN]     = {0};
	char mask[SSH2_MAX_MASK_LEN]       = {0};
	char device[SSH2_MAX_DEVICE_LEN]   = {0};
	int ret = 0;
	char buf[1024] = {0}; /* for skip first line */
	FILE * arp = NULL;
	char remote_ip_addr[SSH2_MAX_IP_LEN] = {0};
	char *colon_location = NULL;

	if (mac_len < SSH2_MAX_MAC_LEN)
	{
		return DROPBEAR_FAILURE;
	}
	
	colon_location = strstr(svr_ses.addrstring, ":");

	if (colon_location == NULL || (colon_location - svr_ses.addrstring) > (SSH2_MAX_IP_LEN - 1))
	{
		return DROPBEAR_FAILURE;
	}

	/* get remote ip addr */
	sscanf(svr_ses.addrstring, "%[^:]", remote_ip_addr);
	if (strlen(remote_ip_addr) == 0)
	{
		return DROPBEAR_FAILURE;
	}
	/* open and handle the arp table, if match the remote ip, then return mac addr */
	arp = fopen(DROPBEAR_ARP_TABLE, "r");
	if (arp)
	{
		fgets(buf, sizeof(buf), arp); /* skip first line of titles */
		do
		{
			memset(ip, '\0', SSH2_MAX_IP_LEN);
			memset(hw_addr, '\0', SSH2_MAX_MAC_LEN);
			ret = fscanf(arp, "%s %s %s %s %s %s\n", ip, hw_type, flags, hw_addr, mask, device);
			if (ret == EOF)
			{
				fclose(arp);
				return DROPBEAR_FAILURE;
			}
			else if (ret != 6) /* 6 is the number of read args */
			{
				continue;
			}
			if (strlen(ip) == 0 || strlen(hw_addr) == 0)
			{
				fclose(arp);
				return DROPBEAR_FAILURE;
			}
			if (strncmp(remote_ip_addr, ip, SSH2_MAX_IP_LEN) == 0)
			{
				memcpy(mac_addr, hw_addr, mac_len);
				break;
			}
		} while (ret >= 0);
		fclose(arp);
	}
	else
	{
		return DROPBEAR_FAILURE;
	}

	return DROPBEAR_SUCCESS;
}

/* if remote device auth failed more than 10 times, then lock it for 2 hours */
static int passwd_auth_fail_delay()
{
	long now = tick_get();
	long start_lock_time   = 0;
	long remain_second     = 0;
	int  attemp_fail_count = 0;
	int  offset = 0;
	int  ret    = 0;
	FILE *count_file= NULL;
	char mac[SSH2_MAX_MAC_LEN] = {0};
	char mac_in_count_file[SSH2_MAX_MAC_LEN] = {0};

	count_file = fopen(DROPBEAR_FAIL_COUNT, "r+");
	if (count_file == NULL) {
		count_file = fopen(DROPBEAR_FAIL_COUNT, "w+");
		if (count_file == NULL)
		{	
			return DROPBEAR_FAILURE;
		}
	}

	ret = get_mac_from_arptable(mac, SSH2_MAX_MAC_LEN);
	if (ret == DROPBEAR_FAILURE)
	{
		fclose(count_file);
		return ret;
	}

	do
	{
		memset(mac_in_count_file, '\0', SSH2_MAX_MAC_LEN);
		offset = ftell(count_file);
		ret = fscanf(count_file, "%s %ld %d\n", mac_in_count_file, &attemp_fail_count, &start_lock_time);
		if (ret == EOF)
		{
			break;
		}
		else if (ret != 3) /*3 is the number of read args */
		{
			continue;
		}
		
		if (!strncmp(mac_in_count_file, mac, SSH2_MAX_MAC_LEN))
		{
			fseek(count_file, offset, SEEK_SET); /* find the location of begin of target line */
			break;
		}
	} while (ret != EOF);
	
	if (ret == EOF)
	{
		/* if not match any mac in the file then add new message */
		attemp_fail_count = 0;
		start_lock_time = 0;
		fprintf(count_file, "%s %02d %012ld\n", mac, attemp_fail_count, start_lock_time);
		fclose(count_file);
		return DROPBEAR_SUCCESS;
	}

	/* if failed times of login >= 10, then create the lock time */
	if (attemp_fail_count >= SSH2_PASSWD_FAILURE_MAX_TIMES)
	{
		if (now - start_lock_time < 0)
		{
			fclose(count_file);
			return DROPBEAR_FAILURE;
		}
		remain_second = SSH2_PASSWD_FAILURE_LOCK_MINUTE * 60 - (now - start_lock_time);
		
		if (remain_second > 0)
		{
			fclose(count_file);
			return SSH2_LOCK_TIME_NOT_OVER;
		}
		else
		{
			attemp_fail_count = 0;
			start_lock_time = 0;
			fprintf(count_file, "%s %02d %012ld\n", mac, attemp_fail_count, start_lock_time);
		}
	}
	fclose(count_file);

	return DROPBEAR_SUCCESS;
}
#endif

/* handle a userauth request, check validity, pass to password or pubkey
 * checking, and handle success or failure */
void recv_msg_userauth_request() {

	unsigned char *username = NULL, *servicename = NULL, *methodname = NULL;
	unsigned int userlen, servicelen, methodlen;
#ifdef FAILED_LOGIN_ATTEMPTS_LIMIT
	int ret = 0;
#endif

	TRACE(("enter recv_msg_userauth_request"))

	/* ignore packets if auth is already done */
	if (ses.authstate.authdone == 1) {
		TRACE(("leave recv_msg_userauth_request: authdone already"))
		return;
	}

	/* send the banner if it exists, it will only exist once */
	if (svr_opts.banner) {
		send_msg_userauth_banner();
	}
	
	username = buf_getstring(ses.payload, &userlen);
	servicename = buf_getstring(ses.payload, &servicelen);
	methodname = buf_getstring(ses.payload, &methodlen);

	/* only handle 'ssh-connection' currently */
	if (servicelen != SSH_SERVICE_CONNECTION_LEN
			&& (strncmp(servicename, SSH_SERVICE_CONNECTION,
					SSH_SERVICE_CONNECTION_LEN) != 0)) {
		
		/* TODO - disconnect here */
		m_free(username);
		m_free(servicename);
		m_free(methodname);
		dropbear_exit("unknown service in auth");
	}
#ifdef FAILED_LOGIN_ATTEMPTS_LIMIT
	ret = passwd_auth_fail_delay();
	if (ret == SSH2_LOCK_TIME_NOT_OVER)
	{
		TRACE(("recv_msg_userauth_request: delay time of passwd auth fail is not over"))
		send_msg_userauth_failure(0, 0);
		goto out;
	}
	else if (ret == DROPBEAR_FAILURE)
	{
		TRACE(("recv_msg_userauth_request: system error such as reading files failed."))
		goto out;
	}
#endif

	/* user wants to know what methods are supported */
	if (methodlen == AUTH_METHOD_NONE_LEN &&
			strncmp(methodname, AUTH_METHOD_NONE,
				AUTH_METHOD_NONE_LEN) == 0) {
		TRACE(("recv_msg_userauth_request: 'none' request"))
		send_msg_userauth_failure(0, 0);
		goto out;
	}
	
	/* check username is good before continuing */
	if (checkusername(username, userlen) == DROPBEAR_FAILURE) {
		/* username is invalid/no shell/etc - send failure */
		TRACE(("sending checkusername failure"))
		send_msg_userauth_failure(0, 1);
		goto out;
	}

#ifdef ENABLE_SVR_PASSWORD_AUTH
	if (!svr_opts.noauthpass &&
			!(svr_opts.norootpass && ses.authstate.pw_uid == 0) ) {
		/* user wants to try password auth */
		if (methodlen == AUTH_METHOD_PASSWORD_LEN &&
				strncmp(methodname, AUTH_METHOD_PASSWORD,
					AUTH_METHOD_PASSWORD_LEN) == 0) {
			svr_auth_password();
			goto out;
		}
	}
#endif

#ifdef ENABLE_SVR_PAM_AUTH
	if (!svr_opts.noauthpass &&
			!(svr_opts.norootpass && ses.authstate.pw_uid == 0) ) {
		/* user wants to try password auth */
		if (methodlen == AUTH_METHOD_PASSWORD_LEN &&
				strncmp(methodname, AUTH_METHOD_PASSWORD,
					AUTH_METHOD_PASSWORD_LEN) == 0) {
			svr_auth_pam();
			goto out;
		}
	}
#endif

#ifdef ENABLE_SVR_PUBKEY_AUTH
	/* user wants to try pubkey auth */
	if (methodlen == AUTH_METHOD_PUBKEY_LEN &&
			strncmp(methodname, AUTH_METHOD_PUBKEY,
				AUTH_METHOD_PUBKEY_LEN) == 0) {
		svr_auth_pubkey();
		goto out;
	}
#endif

	/* nothing matched, we just fail */
	send_msg_userauth_failure(0, 1);

out:

	m_free(username);
	m_free(servicename);
	m_free(methodname);
}

/* Check that the username exists, has a non-empty password, and has a valid
 * shell.
 * returns DROPBEAR_SUCCESS on valid username, DROPBEAR_FAILURE on failure */
static int checkusername(unsigned char *username, unsigned int userlen) {

	char* listshell = NULL;
	char* usershell = NULL;
	TRACE(("enter checkusername"))
	if (userlen > MAX_USERNAME_LEN) {
		return DROPBEAR_FAILURE;
	}

	/* new user or username has changed */
	if (ses.authstate.username == NULL ||
		strcmp(username, ses.authstate.username) != 0) {
			/* the username needs resetting */
			if (ses.authstate.username != NULL) {
				dropbear_log(LOG_WARNING, "Client trying multiple usernames from %s",
							svr_ses.addrstring);
				m_free(ses.authstate.username);
			}
			authclear();
			fill_passwd(username);
			ses.authstate.username = m_strdup(username);
	}

	/* check that user exists */
	if (!ses.authstate.pw_name) {
		TRACE(("leave checkusername: user '%s' doesn't exist", username))
		dropbear_log(LOG_WARNING,
				"Login attempt for nonexistent user from %s",
				svr_ses.addrstring);
		send_msg_userauth_failure(0, 1);
		return DROPBEAR_FAILURE;
	}

	/* check for non-root if desired */
	if (svr_opts.norootlogin && ses.authstate.pw_uid == 0) {
		TRACE(("leave checkusername: root login disabled"))
		dropbear_log(LOG_WARNING, "root login rejected");
		send_msg_userauth_failure(0, 1);
		return DROPBEAR_FAILURE;
	}

	TRACE(("shell is %s", ses.authstate.pw_shell))

	/* check that the shell is set */
	usershell = ses.authstate.pw_shell;
	if (usershell[0] == '\0') {
		/* empty shell in /etc/passwd means /bin/sh according to passwd(5) */
		usershell = "/bin/sh";
	}

	/* check the shell is valid. If /etc/shells doesn't exist, getusershell()
	 * should return some standard shells like "/bin/sh" and "/bin/csh" (this
	 * is platform-specific) */
	setusershell();
	while ((listshell = getusershell()) != NULL) {
		TRACE(("test shell is '%s'", listshell))
		if (strcmp(listshell, usershell) == 0) {
			/* have a match */
			goto goodshell;
		}
	}
	/* no matching shell */
	endusershell();
	TRACE(("no matching shell"))
	dropbear_log(LOG_WARNING, "User '%s' has invalid shell, rejected",
				ses.authstate.pw_name);
	send_msg_userauth_failure(0, 1);
	return DROPBEAR_FAILURE;
	
goodshell:
	endusershell();
	TRACE(("matching shell"))

	TRACE(("uid = %d", ses.authstate.pw_uid))
	TRACE(("leave checkusername"))
	return DROPBEAR_SUCCESS;

}

/* Send a failure message to the client, in responds to a userauth_request.
 * Partial indicates whether to set the "partial success" flag,
 * incrfail is whether to count this failure in the failure count (which
 * is limited. This function also handles disconnection after too many
 * failures */
void send_msg_userauth_failure(int partial, int incrfail) {

	buffer *typebuf = NULL;

	TRACE(("enter send_msg_userauth_failure"))
#ifdef FAILED_LOGIN_ATTEMPTS_LIMIT
	char failedAttempt[90] = {0};
	int remainAttempts = 0;
	int count          = 0;
	int ret            = 0;
	int offset         = 0;
	char mac[SSH2_MAX_MAC_LEN] = {0};
	long start_lock_time = 0;
	FILE * count_file = NULL;
	char mac_in_count_file[SSH2_MAX_MAC_LEN] = {0};
#endif
	CHECKCLEARTOWRITE();
	
	buf_putbyte(ses.writepayload, SSH_MSG_USERAUTH_FAILURE);

	/* put a list of allowed types */
#ifdef FAILED_LOGIN_ATTEMPTS_LIMIT
	typebuf = buf_new(120); /* long enough for PUBKEY, PASSWORD and failed count msg */
#else
	typebuf = buf_new(30); /* long enough for PUBKEY and PASSWORD */
#endif
	if (ses.authstate.authtypes & AUTH_TYPE_PUBKEY) {
		buf_putbytes(typebuf, AUTH_METHOD_PUBKEY, AUTH_METHOD_PUBKEY_LEN);
		if (ses.authstate.authtypes & AUTH_TYPE_PASSWORD) {
			buf_putbyte(typebuf, ',');
		}
	}
	
	if (ses.authstate.authtypes & AUTH_TYPE_PASSWORD) {
		buf_putbytes(typebuf, AUTH_METHOD_PASSWORD, AUTH_METHOD_PASSWORD_LEN);
	}
#ifdef FAILED_LOGIN_ATTEMPTS_LIMIT

	count_file = fopen(DROPBEAR_FAIL_COUNT, "r+");
	if (count_file)
	{
		ret = get_mac_from_arptable(mac, SSH2_MAX_MAC_LEN);
		if (ret == DROPBEAR_FAILURE)
		{
			fclose(count_file);
			return;
		}
		
		do
		{
			memset(mac_in_count_file, '\0', SSH2_MAX_MAC_LEN);
			offset = ftell(count_file);
			ret = fscanf(count_file, "%s %d %ld\n", mac_in_count_file, &count, &start_lock_time);
			/* if cant't find the target line or read failed, not add new line here, and return */
			if (ret == EOF)
			{
				fclose(count_file);
				return;
			}
			else if (ret != 3) /*3 is the number of read args */
			{
				continue;
			}			

			if (!strncmp(mac_in_count_file, mac, SSH2_MAX_MAC_LEN))
			{
				fseek(count_file, offset, SEEK_SET); /* find the location of begin of target line */
				break;
			}
		} while (ret != EOF);
		
		if (incrfail)
		{
			/* record the times of failed login */
			if (count < SSH2_PASSWD_FAILURE_MAX_TIMES)
			{
				count++;
			}
			/* record the start time of lock */
			if (count >= SSH2_PASSWD_FAILURE_MAX_TIMES)
			{
				start_lock_time = tick_get();
			}
			fprintf(count_file, "%s %02d %012ld\n", mac, count, start_lock_time);
		}

		fclose(count_file);
	}
	
	remainAttempts = SSH2_PASSWD_FAILURE_MAX_TIMES - count;
	sprintf(failedAttempt, ",lockedMinute:%d,failedAttempts:%d,remainAttempts:%d", 
	        SSH2_PASSWD_FAILURE_LOCK_MINUTE, count, remainAttempts);
	
	buf_putbytes(typebuf, failedAttempt, strlen(failedAttempt));
#endif
	
	buf_setpos(typebuf, 0);
	buf_putstring(ses.writepayload, buf_getptr(typebuf, typebuf->len),
			typebuf->len);

	TRACE(("auth fail: methods %d, '%s'", ses.authstate.authtypes,
				buf_getptr(typebuf, typebuf->len)));

	buf_free(typebuf);

	buf_putbyte(ses.writepayload, partial ? 1 : 0);
	encrypt_packet();

	if (incrfail) {
		unsigned int delay;
		genrandom((unsigned char*)&delay, sizeof(delay));
		/* We delay for 300ms +- 50ms, 0.1ms granularity */
		delay = 250000 + (delay % 1000)*100;
		usleep(delay);
		ses.authstate.failcount++;
	}

	if (ses.authstate.failcount >= MAX_AUTH_TRIES) {
		char * userstr;
		/* XXX - send disconnect ? */
		TRACE(("Max auth tries reached, exiting"))

		if (ses.authstate.pw_name == NULL) {
			userstr = "is invalid";
		} else {
			userstr = ses.authstate.pw_name;
		}
		dropbear_exit("Max auth tries reached - user '%s' from %s",
				userstr, svr_ses.addrstring);
	}
	
	TRACE(("leave send_msg_userauth_failure"))
}

/* Send a success message to the user, and set the "authdone" flag */
void send_msg_userauth_success() {

	TRACE(("enter send_msg_userauth_success"))

	CHECKCLEARTOWRITE();

	buf_putbyte(ses.writepayload, SSH_MSG_USERAUTH_SUCCESS);
	encrypt_packet();

	/* authdone must be set after encrypt_packet() for 
	 * delayed-zlib mode */
	ses.authstate.authdone = 1;
	ses.connect_time = 0;


	if (ses.authstate.pw_uid == 0) {
		ses.allowprivport = 1;
	}

	/* Remove from the list of pre-auth sockets. Should be m_close(), since if
	 * we fail, we might end up leaking connection slots, and disallow new
	 * logins - a nasty situation. */							
	m_close(svr_ses.childpipe);

	TRACE(("leave send_msg_userauth_success"))

}
