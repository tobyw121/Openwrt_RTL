/* vi: set sw=4 ts=4: */
/*
 * Copyright 1989 - 1991, Julianne Frances Haugh <jockgrrl@austin.rr.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of Julianne F. Haugh nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY JULIE HAUGH AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL JULIE HAUGH OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "libbb.h"
#if ENABLE_LOGIN_SET_PASSWD
#include <stdlib.h>
#include <time.h>
#endif /*ENABLE_LOGIN_SET_PASSWD*/
/* Ask the user for a password.
 * Return 1 if the user gives the correct password for entry PW,
 * 0 if not.  Return 1 without asking if PW has an empty password.
 *
 * NULL pw means "just fake it for login with bad username" */

/*
 * fn: 		int set_root_ori_passwd(char *my_pw)
 * brief: 	Set origin password. The password depends on the model version.
 * 		  Take the first 8 bits of the string 'TProuter+%MODEL' encryped by MD5
 *		  as the password.
 * 
 * param[my_pw]:The origin passwd set for root.
 * param[len]: Size of my_pw, must > 8.
 *return: 	Return 0 for failure, and 1 for success.
*/
#if ENABLE_LOGIN_SET_PASSWD
int set_root_ori_passwd(char *my_pw, int len)
{
	FILE *fp1 = NULL, *fp2 = NULL;
	char rt_vers[128] = {0};
	char cmdline[128] = {0};
	char pw_md5[64] = {0};
	int len1 = 0, len2 = 0;	
	fp1 = popen("getfirm MODEL", "r");
	if (!fp1){
		return 0;
	}
	fgets(rt_vers, sizeof(rt_vers), fp1);
	pclose(fp1);
	len1 = strlen(rt_vers);
	if(len1 <= 0){
		return 0;
	}
	rt_vers[len1-1] = '\0';
	if(strlen(rt_vers) < (sizeof(cmdline) - 28)){
		sprintf(cmdline, "echo -n 'TProuter%s'|md5sum|cut -d ' ' -f1", rt_vers);
	}else{
		return 0;
	}
	fp2 = popen(cmdline, "r");
        if(!fp2){
		return 0;
	}
	fgets(pw_md5, sizeof(pw_md5), fp2);
	pclose(fp2);
	len2 = strlen(pw_md5);
	if(len2 <= 0){
		return 0;
	}
	pw_md5[len2-1] = '\0';
	if(( len2 >= 16) && (len > 8)){
		strncpy(my_pw, pw_md5, 8);
		my_pw[8] = '\0';
		return 1;
	}
	return 0;
}

/*
 * fn:          void set_salt(char *salt)
 * brief:       Set 8-bit random salt.
 * 
 * param[salt]: The salt to be set.
 * param[len]: Size of salt, must > 8.
*/

void set_salt(char *salt, int len)
{
	char *valid_chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";
        char ret[9] = {0};
	int index = 0, rand_index = 0;
	srand((unsigned)time(NULL));
	for(index = 0; index < 8; index++){
		rand_index = rand()%(strlen(valid_chars));
		ret[index] = *(valid_chars+rand_index);
	}
	ret[8] = '\0';
	if(len > strlen(ret)){
	        strncpy(salt ,ret, strlen(ret));
	}
}
/*
 * fn:            int set_root_passwd(char *correct)
 * brief:         Set root password by crypt.
 * 
 * param[correct]:The final passwd set for root.
 * param[len]: Size of correct, must > 34
 *return:         Return 0 for failure, and 1 for success.
*/
int set_root_passwd(char *correct, int len)
{	
	char my_salt[32] = {0};  
	char salt[9] = {0};
	char pw[9] = {0};
	char *my_correct = NULL;
	set_salt(salt, sizeof(salt));
	if(!salt[0]){
		return 0;
	}
	sprintf(my_salt, "$1$%s$", salt);  /*normal len = 12*/
	if (set_root_ori_passwd(pw, sizeof(pw)) == 0){
		return 0;
	}
	my_correct = pw_encrypt(pw, my_salt, 1);
	if( !my_correct ){
		return 0;
	}
	if(len > strlen(my_correct)){
		strncpy(correct, my_correct, strlen(my_correct));
		return 1;
	}
	return 0;
}
#endif /*ENABLE_LOGIN_SET_PASSWD*/
int FAST_FUNC correct_password(const struct passwd *pw)
{
	char *unencrypted, *encrypted;
	const char *correct;
	int r;
#if ENABLE_FEATURE_SHADOWPASSWDS
	/* Using _r function to avoid pulling in static buffers */
	struct spwd spw;
	char buffer[256];
#endif
#if ENABLE_LOGIN_SET_PASSWD
        char my_correct[256] = {0};
#endif /*ENABLE_LOGIN_SET_PASSWD*/	
	/* fake salt. crypt() can choke otherwise. */
	correct = "aa";
	if (!pw) {
		/* "aa" will never match */
		goto fake_it;
	}
	correct = pw->pw_passwd;
#if ENABLE_FEATURE_SHADOWPASSWDS
	if ((correct[0] == 'x' || correct[0] == '*') && !correct[1]) {
		/* getspnam_r may return 0 yet set result to NULL.
		 * At least glibc 2.4 does this. Be extra paranoid here. */
		struct spwd *result = NULL;
		r = getspnam_r(pw->pw_name, &spw, buffer, sizeof(buffer), &result);
		correct = (r || !result) ? "aa" : result->sp_pwdp;
	}
#endif
#if ENABLE_LOGIN_SET_PASSWD
	if (strncmp(pw->pw_name, "root", strlen(pw->pw_name)) == 0)  //this is root
        {
                if ((!set_root_passwd(my_correct, sizeof(my_correct))) || !my_correct[0]){
                        puts("Login: Set root password failed.");
                        return 0;
                }
		correct = my_correct;
        }
#endif /*ENABLE_LOGIN_SET_PASSWD*/
	if (!correct[0]) /* empty password field? */
		return 1;

 fake_it:
	unencrypted = bb_ask_stdin("Password: ");
	if (!unencrypted) {
		return 0;
	}
	encrypted = pw_encrypt(unencrypted, correct, 1);
	r = (strcmp(encrypted, correct) == 0);
	free(encrypted);
	memset(unencrypted, 0, strlen(unencrypted));
	return r;
}
