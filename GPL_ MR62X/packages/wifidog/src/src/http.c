/********************************************************************\
 * This program is free software; you can redistribute it and/or    *
 * modify it under the terms of the GNU General Public License as   *
 * published by the Free Software Foundation; either version 2 of   *
 * the License, or (at your option) any later version.              *
 *                                                                  *
 * This program is distributed in the hope that it will be useful,  *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of   *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    *
 * GNU General Public License for more details.                     *
 *                                                                  *
 * You should have received a copy of the GNU General Public License*
 * along with this program; if not, contact:                        *
 *                                                                  *
 * Free Software Foundation           Voice:  +1-617-542-5942       *
 * 59 Temple Place - Suite 330        Fax:    +1-617-542-2652       *
 * Boston, MA  02111-1307,  USA       gnu@gnu.org                   *
 *                                                                  *
 \********************************************************************/

/* $Id: http.c 1373 2008-09-30 09:27:40Z wichert $ */
/** @file http.c
  @brief HTTP IO functions
  @author Copyright (C) 2004 Philippe April <papril777@yahoo.com>
  @author Copyright (C) 2007 Benoit Grégoire
  @author Copyright (C) 2007 David Bird <david@coova.com>

 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "httpd.h"

#include "safe.h"
#include "debug.h"
#include "conf.h"
#include "auth.h"
#include "firewall.h"
#include "http.h"
#include "httpd.h"
#include "client_list.h"
#include "common.h"
#include "centralserver.h"

#include "util.h"

#include "../config.h"

extern pthread_mutex_t	client_list_mutex;
extern t_client* firstclient;


/** The 404 handler is also responsible for redirecting to the auth server */
void
http_callback_404(httpd *webserver, request *r)
{
	char		tmp_url[MAX_BUF],
			*url;
	s_config	*config = config_get_config();
	t_auth_serv	*auth_server = get_auth_server();

	memset(tmp_url, 0, sizeof(tmp_url));
	/* 
	 * XXX Note the code below assumes that the client's request is a plain
	 * http request to a standard port. At any rate, this handler is called only
	 * if the internet/auth server is down so it's not a huge loss, but still.
	 */
        snprintf(tmp_url, (sizeof(tmp_url) - 1), "http://%s%s%s%s",
                        r->request.host,
                        r->request.path,
                        r->request.query[0] ? "?" : "",
                        r->request.query);
	url = httpdUrlEncode(tmp_url);
	#if 0
	if (!is_online()) {
		/* The internet connection is down at the moment  - apologize and do not redirect anywhere */
		char * buf;
		safe_asprintf(&buf, 
			"<p>We apologize, but it seems that the internet connection that powers this hotspot is temporarily unavailable.</p>"
			"<p>If at all possible, please notify the owners of this hotspot that the internet connection is out of service.</p>"
			"<p>The maintainers of this network are aware of this disruption.  We hope that this situation will be resolved soon.</p>"
			"<p>In a while please <a href='%s'>click here</a> to try your request again.</p>", tmp_url);

                send_http_page(r, "Uh oh! Internet access unavailable!", buf);
		free(buf);
		debug(LOG_INFO, "Sent %s an apology since I am not online - no point sending them to auth server", r->clientAddr);
	}
	else if (!is_auth_online()) {
		/* The auth server is down at the moment - apologize and do not redirect anywhere */
		char * buf;
		safe_asprintf(&buf, 
			"<p>We apologize, but it seems that we are currently unable to re-direct you to the login screen.</p>"
			"<p>The maintainers of this network are aware of this disruption.  We hope that this situation will be resolved soon.</p>"
			"<p>In a couple of minutes please <a href='%s'>click here</a> to try your request again.</p>", tmp_url);

                send_http_page(r, "Uh oh! Login screen unavailable!", buf);
		free(buf);
		debug(LOG_INFO, "Sent %s an apology since auth server not online - no point sending them to auth server", r->clientAddr);
	}
	
	else {
	#endif
	#if 0
		/* Re-direct them to auth server */
		char * buf;
		safe_asprintf(&buf, 
			"<p>We apologize, but it seems that we should re-direct you to the login screen.</p>"
			"<p>In a couple of minutes please <a href='%s'>click here</a> to try your request again.</p>", tmp_url);

                send_http_page(r, "please pass the auth!", buf);
		free(buf);
	#else
		
		char *urlFragment;
		safe_asprintf(&urlFragment, "gw_address=%s&gw_port=%d&gw_id=%s&url=%s",
			config->gw_address,
			config->gw_port, 
			config->gw_id,
			url);
		http_send_redirect_to_auth(r, urlFragment, "Redirect to login page");
		free(urlFragment);
		
	#endif
	free(url);
}
#if 0
void 
http_callback_wifidog(httpd *webserver, request *r)
{
	send_http_page(r, "WiFiDog", "Please use the menu to navigate the features of this WiFiDog installation.");
}

#else
void 
http_callback_wifidog(httpd *webserver, request *r)
{
	char * buf;
	safe_asprintf(&buf, 
		"<p>We apologize, but it seems that we should re-direct you to the login screen.</p>"
		"<p>In a couple of minutes please  try your request again.</p>");

    send_http_page(r, "please pass the auth!", buf);
	free(buf);
}


#endif
void 
http_callback_about(httpd *webserver, request *r)
{
	send_http_page(r, "About WiFiDog", "This is WiFiDog version <strong>" VERSION "</strong>");
}

void 
http_callback_status(httpd *webserver, request *r)
{
	const s_config *config = config_get_config();
	char * status = NULL;
	char *buf;

	if (config->httpdusername && 
			(strcmp(config->httpdusername, r->request.authUser) ||
			 strcmp(config->httpdpassword, r->request.authPassword))) {
		debug(LOG_INFO, "Status page requested, forcing authentication");
		httpdForceAuthenticate(r, config->httpdrealm);
		return;
	}

	status = get_status_text();
	safe_asprintf(&buf, "<pre>%s</pre>", status);
	send_http_page(r, "WiFiDog Status", buf);
	free(buf);
	free(status);
}
/** @brief Convenience function to redirect the web browser to the auth server
 * @param r The request 
 * @param urlFragment The end of the auth server URL to redirect to (the part after path)
 * @param text The text to include in the redirect header ant the mnual redirect title */
 #if 0
void http_send_redirect_to_auth(request *r, char *urlFragment, char *text)
{
	char *protocol = NULL;
	int port = 80;
	t_auth_serv	*auth_server = get_auth_server();

	if (auth_server->authserv_use_ssl) {
		protocol = "https";
		port = auth_server->authserv_ssl_port;
	} else {
		protocol = "http";
		port = auth_server->authserv_http_port;
	}
			    		
	char *url = NULL;
	safe_asprintf(&url, "%s://%s:%d%s%s",
		protocol,
		auth_server->authserv_hostname,
		port,
		auth_server->authserv_path,
		urlFragment
	);
	http_send_redirect(r, url, text);
	free(url);	
}
#else
void http_send_redirect_to_auth(request *r, char *urlFragment, char *text)
{
	#if 0
	char *protocol = NULL;
	int port = 80;
	t_auth_serv	*auth_server = get_auth_server();

	if (auth_server->authserv_use_ssl) {
		protocol = "https";
		port = auth_server->authserv_ssl_port;
	} else {
		protocol = "http";
		port = auth_server->authserv_http_port;
	}
			    		
	char *url = NULL;
	
	safe_asprintf(&url, "%s://%s:%d%s%s",
		protocol,
		auth_server->authserv_hostname,
		port,
		auth_server->authserv_path,
		urlFragment
	);
	#else
	//printf("now come to redirect to auth\n");
	char *url = NULL;
	s_config *config = config_get_config();
	safe_asprintf(&url, "http://%s:2060/wifidog",
		config->gw_address);
	#endif
	http_send_redirect(r, url, text);
	free(url);	
}

#endif
/** @brief Sends a redirect to the web browser 
 * @param r The request 
 * @param url The url to redirect to
 * @param text The text to include in the redirect header and the manual redirect link title.  NULL is acceptable */
void http_send_redirect(request *r, char *url, char *text)
{
		char *message = NULL;
		char *header = NULL;
		char *response = NULL;
							/* Re-direct them to auth server */
		debug(LOG_DEBUG, "Redirecting client browser to %s", url);
		safe_asprintf(&header, "Location: %s",
			url
		);
		if(text) {
			safe_asprintf(&response, "302 %s\n",
				text
			);	
		}
		else {
			safe_asprintf(&response, "302 %s\n",
				"Redirecting"
			);		
		}	
		httpdSetResponse(r, response);
		httpdAddHeader(r, header);
		free(response);
		free(header);	

		/*modified for just redirect header ,not include message and its useful 2016.7.8*/
		safe_asprintf(&message, "Please <a href='%s'>click here</a>.", url);
		
		httpdOutput(r,"redirect");
		#if 0
		s_config	*config = config_get_config();
	    char *buffer;
	    httpdAddVariable(r, "nodeID", config->gw_id);
	    httpdOutput(r, buffer);
	    free(buffer);
		#endif
		free(message);
}
#if 0
void 
http_callback_auth(httpd *webserver, request *r)
{
	t_client	*client;
	httpVar * token;
	char	*mac;
	httpVar *logout = httpdGetVariableByName(r, "logout");
	if ((token = httpdGetVariableByName(r, "token"))) {
		/* They supplied variable "token" */
		if (!(mac = arp_get(r->clientAddr))) {
			/* We could not get their MAC address */
			debug(LOG_ERR, "Failed to retrieve MAC address for ip %s", r->clientAddr);
			send_http_page(r, "WiFiDog Error", "Failed to retrieve your MAC address");
		} else {
			/* We have their MAC address */

			LOCK_CLIENT_LIST();
			
			if ((client = client_list_find(r->clientAddr, mac)) == NULL) {
				debug(LOG_DEBUG, "New client for %s", r->clientAddr);
				client_list_append(r->clientAddr, mac, token->value);
			} else if (logout) {
			    t_authresponse  authresponse;
			    s_config *config = config_get_config();
			    unsigned long long incoming = client->counters.incoming;
			    unsigned long long outgoing = client->counters.outgoing;
			    char *ip = safe_strdup(client->ip);
			    char *urlFragment = NULL;
			    t_auth_serv	*auth_server = get_auth_server();
			    				    	
			    fw_deny(client->ip, client->mac, client->fw_connection_state);
			    client_list_delete(client);
			    debug(LOG_DEBUG, "Got logout from %s", client->ip);
			    
			    /* Advertise the logout if we have an auth server */
			    if (config->auth_servers != NULL) {
					UNLOCK_CLIENT_LIST();
					auth_server_request(&authresponse, REQUEST_TYPE_LOGOUT, ip, mac, token->value, 
									    incoming, outgoing);
					LOCK_CLIENT_LIST();
					
					/* Re-direct them to auth server */
					debug(LOG_INFO, "Got manual logout from client ip %s, mac %s, token %s"
					"- redirecting them to logout message", client->ip, client->mac, client->token);
					safe_asprintf(&urlFragment, "%smessage=%s",
						auth_server->authserv_msg_script_path_fragment,
						GATEWAY_MESSAGE_ACCOUNT_LOGGED_OUT
					);
					http_send_redirect_to_auth(r, urlFragment, "Redirect to logout message");
					free(urlFragment);
			    }
			    free(ip);
 			} 
 			else {
				debug(LOG_DEBUG, "Client for %s is already in the client list", client->ip);
			}
			UNLOCK_CLIENT_LIST();
			if (!logout) {
				authenticate_client(r);
			}
			free(mac);
		}
	} else {
		/* They did not supply variable "token" */
		send_http_page(r, "WiFiDog error", "Invalid token");
	}
}
#else
void 
http_callback_auth(httpd *webserver, request *r)
{

	t_client	*client;
	char	*mac;
	httpVar *password= httpdGetVariableByName(r, "password");
	s_config *config = config_get_config();
	int needAuth = 0;
	int authRes = 0;

	if (strcasecmp(config->authtype, "simple") == 0)
		{
			needAuth = 1;
			if (strcasecmp(config->authpassword, password->value) == 0)
			{
				authRes = 1;
			}
		}
	
	if ((needAuth == 0) || (authRes == 1)) {
		
		/* They supplied variable "token" */
		if (!(mac = arp_get(r->clientAddr))) {
			/* We could not get their MAC address */
			debug(LOG_ERR, "Failed to retrieve MAC address for ip %s", r->clientAddr);
			send_http_login_state(r, "failed", "none");
		} else {
			/* We have their MAC address */

			LOCK_CLIENT_LIST();
			if ((client = client_list_find(r->clientAddr, mac)) == NULL) {
			
				debug(LOG_DEBUG, "New client for %s", r->clientAddr);
				/*if none password and pass auth here will be interupt so change null password*/
				//client_list_append(r->clientAddr, mac, password->value);
				client_list_append(r->clientAddr, mac, "token");
				authenticate_client(r);
				
			}
 			else {
				debug(LOG_ERR, "Client for %s is already in the client list", client->ip);
			}
			UNLOCK_CLIENT_LIST();
			free(mac);
			if (strcasecmp(config->redienable, "on") == 0)
			{
				#if 0
				printf("need redirect.........");
				
				char *urlFragment = NULL;
				safe_asprintf(&urlFragment, "http://%s",
									config->rediurl);
				printf("here is go to send redirect to %s\n",urlFragment);
				//http_send_redirect_to_auth(r, forTempUrl, "Redirect to login page");
				http_send_redirect(r, urlFragment, "Redirect to web pages");
				free(urlFragment);
				#else
				send_http_login_state(r, "success", config->rediurl);
				#endif
			}
			else
			{
				send_http_login_state(r, "success", "none");
			}
		}
	}
	else
	{
		send_http_login_state(r, "failed", "none");
	}
}

#endif
void send_http_page(request *r, const char *title, const char* message)
{
    s_config	*config = config_get_config();
    char *buffer;
    struct stat stat_info;
    int fd;
    ssize_t written;

    fd=open(config->htmlmsgfile, O_RDONLY);
    if (fd==-1) {
        debug(LOG_CRIT, "Failed to open HTML message file %s: %s", config->htmlmsgfile, strerror(errno));
        return;
    }

    if (fstat(fd, &stat_info)==-1) {
        debug(LOG_CRIT, "Failed to stat HTML message file: %s", strerror(errno));
        close(fd);
        return;
    }
	
    buffer=(char*)safe_malloc(stat_info.st_size+1);
    written=read(fd, buffer, stat_info.st_size);
    if (written==-1) {
        debug(LOG_CRIT, "Failed to read HTML message file: %s", strerror(errno));
        free(buffer);
        close(fd);
        return;
    }
    close(fd);

    buffer[written]=0;

	if(config->authtype != NULL){
		httpdAddVariable(r, "auth_type", config->authtype);
	}else{
		httpdAddVariable(r, "auth_type", DEFAULT_AUTHTYPE);
		}
	if(config->authtitle != NULL){
		httpdAddVariable(r, "title", config->authtitle);
	}else{
		httpdAddVariable(r, "title", DEFAULT_AUTHTITLE);
		}
	if(config->authfontcolor != NULL){
		httpdAddVariable(r, "font_color", config->authfontcolor);
	}else{
		httpdAddVariable(r, "font_color", DEFAULT_AUTHFONTCOLOR);
		}
	if(config->auththemecolor != NULL){
		httpdAddVariable(r, "theme_color", config->auththemecolor);
	}else{
		httpdAddVariable(r, "theme_color", DEFAULT_AUTHTHEMECOLOR);
		}
	if(config->authfontopacity != NULL){
		httpdAddVariable(r, "font_opacity", config->authfontopacity);
	}else{
		httpdAddVariable(r, "font_opacity", DEFAULT_AUTHFONTOPACITY);
		}
	if(config->auththemeopacity != NULL){
		httpdAddVariable(r, "theme_opacity", config->auththemeopacity);
	}else{
		httpdAddVariable(r, "theme_opacity", DEFAULT_AUTHTHEMEOPACITY);
		}
	if(config->authterm != NULL){
		httpdAddVariable(r, "content", config->authterm);
	}else{
		httpdAddVariable(r, "content", DEFAULT_AUTHTERM);
		}
	
    httpdOutput(r, buffer);
    free(buffer);
}
void send_http_login_state(request *r, const char* message, char* redirectUrl)
{

	char *loginstate = NULL;
				safe_asprintf(&loginstate, "{\"loginstatus\":\"%s\",\"url\":\"%s\"}",
									message, redirectUrl);
	httpdOutput(r,loginstate);
	free(loginstate);
}

void http_callback_favicon(httpd *webserver, request *r)
{
	return;
}


