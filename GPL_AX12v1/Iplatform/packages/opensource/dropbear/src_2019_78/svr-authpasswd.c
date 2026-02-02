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

/* Validates a user password */

#include "includes.h"
#include "session.h"
#include "buffer.h"
#include "dbutil.h"
#include "auth.h"
#include "runopts.h"
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#ifdef FAILED_LOGIN_ATTEMPTS_LIMIT
#include "ssh.h"
#endif

#if DROPBEAR_SVR_PASSWORD_AUTH

int docloudlogin(char* username, char* password)
{
	long int ret = 0;
	const char *buf = NULL;
	lua_State *L = luaL_newstate();

	if (L) {
		luaL_openlibs(L);
		lua_getglobal(L, "require");
		lua_pushstring(L, "cloud_req.cloud_account");

		if (lua_pcall(L, 1, 1, 0) != 0)	{
			lua_close(L);
			return -1;
		}

		if (!lua_istable(L, -1)) {
			lua_close(L);
			return -1;
		}
	}

	lua_getfield(L, -1, "account_login");
	if (!lua_isfunction(L, -1))	{
		lua_pop(L, 1);
		return -1;
	}
	lua_pushstring(L, username);
	lua_pushstring(L, password);
	if (lua_pcall(L, 2, 1, 0) != 0)
	{
		dropbear_log(LOG_ERR,"error running function 'account_login' : %s", lua_tostring(L, -1));
	}

	buf = lua_tostring(L, -1);
	if (buf != NULL)
	{
		ret = strtol(buf, NULL, 10);
	}
	lua_close(L);

	return (ret == 0)?0:1;
}

/* not constant time when strings are differing lengths. 
 string content isn't leaked, and crypt hashes are predictable length. */
static int constant_time_strcmp(const char* a, const char* b) {
	size_t la = strlen(a);
	size_t lb = strlen(b);

	if (la != lb) {
		return 1;
	}

	return constant_time_memcmp(a, b, la);
}

/* Process a password auth request, sending success or failure messages as
 * appropriate */
void svr_auth_password_ori(int valid_user) {
	
	char * passwdcrypt = NULL; /* the crypt from /etc/passwd or /etc/shadow */
	char * testcrypt = NULL; /* crypt generated from the user's password sent */
	char * password = NULL;
	unsigned int passwordlen;
	unsigned int changepw;

	/* check if client wants to change password */
	changepw = buf_getbool(ses.payload);
	if (changepw) {
		/* not implemented by this server */
		send_msg_userauth_failure(0, 1);
		return;
	}

	password = buf_getstring(ses.payload, &passwordlen);
	if (valid_user && passwordlen <= DROPBEAR_MAX_PASSWORD_LEN) {
		/* the first bytes of passwdcrypt are the salt */
		passwdcrypt = ses.authstate.pw_passwd;
		testcrypt = crypt(password, passwdcrypt);
	}
	m_burn(password, passwordlen);
	m_free(password);

	/* After we have got the payload contents we can exit if the username
	is invalid. Invalid users have already been logged. */
	if (!valid_user) {
		send_msg_userauth_failure(0, 1);
		return;
	}

	if (passwordlen > DROPBEAR_MAX_PASSWORD_LEN) {
		dropbear_log(LOG_WARNING,
				"Too-long password attempt for '%s' from %s",
				ses.authstate.pw_name,
				svr_ses.addrstring);
		send_msg_userauth_failure(0, 1);
		return;
	}

	if (testcrypt == NULL) {
		/* crypt() with an invalid salt like "!!" */
		dropbear_log(LOG_WARNING, "User account '%s' is locked",
				ses.authstate.pw_name);
		send_msg_userauth_failure(0, 1);
		return;
	}

	/* check for empty password */
	if (passwdcrypt[0] == '\0') {
		dropbear_log(LOG_WARNING, "User '%s' has blank password, rejected",
				ses.authstate.pw_name);
		send_msg_userauth_failure(0, 1);
		return;
	}

	if (constant_time_strcmp(testcrypt, passwdcrypt) == 0) {
		/* successful authentication */
		dropbear_log(LOG_NOTICE, 
				"Password auth succeeded for '%s' from %s",
				ses.authstate.pw_name,
				svr_ses.addrstring);
		send_msg_userauth_success();
	} else {
		dropbear_log(LOG_WARNING,
				"Bad password attempt for '%s' from %s",
				ses.authstate.pw_name,
				svr_ses.addrstring);
		send_msg_userauth_failure(0, 1);
	}
}

void svr_auth_password_cum() {
	char * passwdcrypt = NULL; /* the crypt from /etc/passwd or /etc/shadow */
	unsigned char * password;
	int success_blank = 0;
	unsigned int passwordlen;
	unsigned int changepw;
#ifdef FAILED_LOGIN_ATTEMPTS_LIMIT
	FILE * count_file = NULL;
	int count  = 0;
	int offset = 0;
	int ret    = 0;
	char mac[SSH2_MAX_MAC_LEN] = {0};
	long start_lock_time = 0;
	char mac_in_count_file[SSH2_MAX_MAC_LEN] = {0};
#endif

#ifdef SSH_DISABLE_DEFAULT
	char remoteip[SSH2_MAX_IP_LEN] = {0};
	char filePath[50] = {0};
	int fd = -1;
#endif

	passwdcrypt = ses.authstate.pw_passwd;

	/* check if client wants to change password */
	changepw = buf_getbool(ses.payload);
	if (changepw) {
		/* not implemented by this server */
		send_msg_userauth_failure(0, 1);
		return;
	}

	password = buf_getstring(ses.payload, &passwordlen);

	/* check for empty password */
	if (passwdcrypt[0] == '\0') {
		if(svr_opts.allowblankpass)
		{
			success_blank = 1;
		}
		else
		{
			dropbear_log(LOG_WARNING, "User '%s' has blank password, rejected",
					ses.authstate.pw_name);
			send_msg_userauth_failure(0, 1);
			m_burn(password, passwordlen);
			m_free(password);
			return;
		}
	}

#ifdef PASSWD_IN_SHADOW
	if (success_blank || check_passwd("admin", password) || strcmp(password, passwdcrypt) == 0)
#else
	if (success_blank || strcmp(password, passwdcrypt) == 0)
#endif
	{
		/* successful authentication */
		dropbear_log(LOG_NOTICE,
				"Password auth succeeded for '%s' from %s",
				ses.authstate.pw_name,
				svr_ses.addrstring);
#ifdef FAILED_LOGIN_ATTEMPTS_LIMIT

		count_file = fopen(DROPBEAR_FAIL_COUNT, "r+");
		if (count_file) {
			ret = get_mac_from_arptable(mac, SSH2_MAX_MAC_LEN);
			if (ret == DROPBEAR_FAILURE)
			{
				fclose(count_file);
				m_burn(password, passwordlen);
				m_free(password);
				return;
			}
			
			do
			{
				memset(mac_in_count_file, '\0', SSH2_MAX_MAC_LEN);
				offset = ftell(count_file);
				ret = fscanf(count_file, "%s %d %ld\n", mac_in_count_file, &count, &start_lock_time);
				if (ret == EOF)
				{
					fclose(count_file);
					m_burn(password, passwordlen);
					m_free(password);
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
			
			if (count < SSH2_PASSWD_FAILURE_MAX_TIMES)
			{
				fprintf(count_file, "%s %02d %012ld\n", mac, 0, 0);
			}
			fclose(count_file);	
		}
#endif
		send_msg_userauth_success();
#ifdef SSH_DISABLE_DEFAULT
		sscanf(svr_ses.addrstring, "%[^:]", remoteip);
		snprintf(filePath, sizeof(filePath), "/tmp/dropbear/succ_cli/%s", remoteip);

		if (0 > (fd = open(filePath, O_RDWR | O_CREAT, 0600)))
		{
			// Record this fault , we can only try another way to create it.
			dropbear_log(LOG_WARNING, "Can't create or open file %s", filePath);
			// Use filePath as a command.
			memset(filePath, 0, sizeof(filePath));
			snprintf(filePath, sizeof(filePath), "touch /tmp/dropbear/succ_cli/%s", remoteip);
			system(filePath);
		}
		if (0 < fd)
		{
			close(fd);
		}
#endif
	} 
	else if (strcmp(ses.authstate.pw_name, "dropbear") != 0)		//cloud account may not be synchronized
	{
		if (docloudlogin(ses.authstate.pw_name, password) == 0)
		{
			/* successful authentication */
			send_msg_userauth_success();
		}
		else
		{
			send_msg_userauth_failure(0, 1);
		}
	}
	else
	{
		dropbear_log(LOG_WARNING,
				"Bad password attempt for '%s' from %s",
				ses.authstate.pw_name,
				svr_ses.addrstring);
		send_msg_userauth_failure(0, 1);
	}

	m_burn(password, passwordlen);
	m_free(password);
}

void svr_auth_password()
{
	if (0 == svr_opts.customAuth)
	{
		svr_auth_password_ori(1);
	}
	else
	{
		svr_auth_password_cum();
	}
}
#endif
