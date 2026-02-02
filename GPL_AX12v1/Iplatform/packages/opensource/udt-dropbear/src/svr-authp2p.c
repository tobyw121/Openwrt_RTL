#include "includes.h"
#include "session.h"
#include "buffer.h"
#include "dbutil.h"
#include "auth.h"

#ifdef ENABLE_SVR_P2P_AUTH

p2pauth_cb _p2p_check_passwd = generic_p2p_check_passwd;

int generic_p2p_check_passwd(const unsigned char *username, 
    const unsigned char *password, void* p2pses) {
    
    dropbear_log(LOG_INFO, "generc_p2p_check_passwd()");
    
    /* return false in default */
    return 0;
}

void svr_auth_p2p() {
	unsigned char * password;
	unsigned char * username;
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
	username = ses.authstate.username;

	/* check for empty password */
	if (password[0] == '\0') {
		dropbear_log(LOG_WARNING, "User has blank password, rejected");
		send_msg_userauth_failure(0, 1);
		return;
	}

	/* Check username */
	if (username == NULL || username[0] == '\0') {
		dropbear_log(LOG_WARNING, "User name is empty, rejected");
		send_msg_userauth_failure(0, 1);
		return;
	}

	if (_p2p_check_passwd(username, password, svr_ses.p2pses)) {
		/* successful authentication */
		dropbear_log(LOG_NOTICE,
					 "Password auth succeeded from %s",
					 svr_ses.addrstring);
		send_msg_userauth_success();
	} else {
		dropbear_log(LOG_WARNING,
					 "Bad password attempt from %s",
					 svr_ses.addrstring);
		send_msg_userauth_failure(0, 1);
	}

	m_burn(password, passwordlen);
	m_free(password);
}

#endif
