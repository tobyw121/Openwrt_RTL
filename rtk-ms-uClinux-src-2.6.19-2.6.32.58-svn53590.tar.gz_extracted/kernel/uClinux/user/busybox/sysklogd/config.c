#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <paths.h>
#include <unistd.h>

#include <syslog.h>

#include <../../turnkey/config/autoconf.h>

#include "syslogd.h"
#include "syslogd_config.h"
/*
#define __LOG_FILE          "/var/log/ram.log"
#define __LOG_FILE_FLASH    "/mnt/flash.log"
*/
static int syslogd_level(const char *name)
{
	int pri;

	if (strcmp(name, "none") == 0) {
		return LOG_NONE;
	}

	pri = syslog_name_to_pri(name);
	if (pri < 0) {
		debug_printf("Warning: Level %s not found, returning debug", name);
		pri = LOG_DEBUG;
	}
	return pri;
}

/*support 4k ~16M*/
static int syslog_maxsize_get(int index)
{
    int maxsize;

    if(index < SYSLOG_MIN_INDEX)
            maxsize = 1 << (SYSLOG_MIN_INDEX - 10);
    else if(index > SYSLOG_MAX_INDEX)
            maxsize = 1 << (SYSLOG_MAX_INDEX - 10);
    else
            maxsize = 1 << (index - 10);

    return maxsize;
}

int syslogd_load_config(const char *filename, syslogd_config_t *config)
{
	FILE *fh = fopen(filename, "r");
	char buf[512];
    int maxsize_ram = 0, maxsize_flash = 0;
        
	/* Initialise the default config */
	memset(config, 0, sizeof(*config));

#ifdef CONFIG_SYS_SYSLOG
    maxsize_ram = syslog_maxsize_get(CONFIG_USER_SYSLOG_RAM_SIZE);
    maxsize_flash = syslog_maxsize_get(CONFIG_USER_SYSLOG_FLASH_SIZE);
#endif
       
	config->local.common.target = SYSLOG_TARGET_LOCAL;
	config->local.common.msgnum = 0;
	config->local.common.level = 0;/*1 << LOG_DEBUG;*/
	config->local.common.priv = 0;
	config->local.common.next = 0;
	//config->local.maxsize = 16;
    config->local.maxsize = maxsize_ram;
	config->local.logfile = __LOG_FILE;
	config->local.numfiles = 1;

#ifndef EMBED
	config->local.markinterval = 20 * 60;
#endif

	if (!fh) {
		return 1;
	}

	while (fgets(buf, sizeof(buf), fh)) {
		syslogd_target_t *target;

		char *type = strtok(buf, " \t\n");
		if (type && *type && *type != '#') {
			char *token;

			if (strcmp(type, "global") == 0) {
				target = 0;
			}
			else if (strcmp(type, "local-ram") == 0) {
				target = &config->local.common;
			}
			else if (strncmp(type, "server", strlen("server")) == 0) {
				syslogd_remote_config_t *remote = malloc(sizeof(*remote));

				memset(remote, 0, sizeof(*remote));

				target = &remote->common;
				target->target = SYSLOG_TARGET_REMOTE;
				target->msgnum = 0;
				target->level = 0;/*1 << LOG_DEBUG;*/
				target->next = config->local.common.next;
				config->local.common.next = target;

				remote->port = 514;
			}
			else if (strcmp(type, "email") == 0) {
				syslogd_email_config_t *email = malloc(sizeof(*email));

				memset(email, 0, sizeof(*email));

				target = &email->common;
				target->target = SYSLOG_TARGET_EMAIL;
				target->msgnum = 0;
				target->level = 0;/*1 << LOG_ERR;*/
				target->next = config->local.common.next;
				config->local.common.next = target;

				email->delay = 60;
			}
			else if (strcmp(type, "local-flash") == 0) {
				syslogd_local_config_t *flash = malloc(sizeof(*flash));

				memset(flash, 0, sizeof(*flash));

                flash->logfile = __LOG_FILE_FLASH;
                flash->numfiles= 1;
               // flash->maxsize = 16;
                flash->maxsize = maxsize_flash;
#ifndef EMBED
                flash->markinterval = 20 * 60;
#endif

				target = &flash->common;
				target->target = SYSLOG_TARGET_LOCAL_FLASH;
				target->level = 0;/*1 << LOG_WARNING;*/
				target->msgnum = 0;

				target->next = config->local.common.next;
				config->local.common.next = target;
			}
           else if (strcmp(type, "local-console") == 0) {
                syslogd_local_config_t *console = malloc(sizeof(*console));

                memset(console, 0, sizeof(*console));

                console->logfile = _PATH_CONSOLE;
                console->numfiles= 1;
               // console->maxsize = 16;
                console->maxsize = 0;
#ifndef EMBED
                console->markinterval = 20 * 60;
#endif

                target = &console->common;
                target->target = SYSLOG_TARGET_LOCAL_CONSOLE;
                target->level = 0;/*1 << LOG_WARNING;*/
                target->msgnum = 0;

                target->next = config->local.common.next;
                config->local.common.next = target;
            }
			else {
				debug_printf("Unknown target type: %s", type);
				continue;
			}

			/* Now fill in the parameters */
			while ((token = strtok(0, " \t\n")) != 0) {
				char *value = strchr(token, '=');
				if (value) {
					*value++ = 0;

					/* Now we finally have type, token and value */
					if (target == 0) {
						if (strcmp(token, "iso") == 0) {
							config->iso = atoi(value);
						}
						else if (strcmp(token, "repeat") == 0) {
							config->repeat = atoi(value);
						}
						else {
							debug_printf("Unknown %s: %s=%s", type, token, value);
						}
						continue;
					}

					if (strcmp(token, "level") == 0) {
						//target->level = syslogd_level(value);
					    int loglevel = syslogd_level(value);

					    if (LOG_NONE == loglevel)
					        target->level = 0;
					    else
						    target->level |= 1 << loglevel;

						continue;
					}

					switch (target->target) {
						case SYSLOG_TARGET_LOCAL:
                        case SYSLOG_TARGET_LOCAL_FLASH:
							{
								syslogd_local_config_t *local = (syslogd_local_config_t *)target;

								if (strcmp(token, "maxsize") == 0) {
									local->maxsize = atoi(value);
								}
								else if (strcmp(token, "markinterval") == 0) {
									local->markinterval = atoi(value) * 60;
								}
								else if (strcmp(token, "numfiles") == 0) {
									local->numfiles = atoi(value);
								}
								else if (strcmp(token, "logfile") == 0) {
									local->logfile = strdup(value);
								}
								else {
									debug_printf("Unknown %s: %s=%s", type, token, value);
								}
							}
							break;

						case SYSLOG_TARGET_REMOTE:
							{
								syslogd_remote_config_t *remote = (syslogd_remote_config_t *)target;

								if (strcmp(token, "ip") == 0) {
									remote->host = strdup(value);
								}
								else if (strcmp(token, "port") == 0) {
									remote->port = atoi(value);
								}
								else if (strcmp(token, "facility") == 0) {
									remote->facility = atoi(value);
								}
								else {
									debug_printf("Unknown %s: %s=%s", type, token, value);
								}
							}
							break;

						case SYSLOG_TARGET_EMAIL:
							{
								syslogd_email_config_t *email = (syslogd_email_config_t *)target;

								if (strcmp(token, "server") == 0) {
									email->server = strdup(value);
								}
								else if (strcmp(token, "addr") == 0) {
									if (!email->addr) {
										email->addr = strdup(value);
									}
									else {
										/* Append this one */
										char *pt = malloc(strlen(email->addr) + strlen(value) + 2);
										sprintf(pt, "%s %s", email->addr, value);
										free(email->addr);
										email->addr = pt;
									}
								}
								else if (strcmp(token, "sender") == 0) {
									email->sender = strdup(value);
								}
								else if (strcmp(token, "fromhost") == 0) {
									email->fromhost = strdup(value);
								}
								else if (strcmp(token, "delay") == 0) {
									email->delay = atoi(value);
								}
								else if (strcmp(token, "freq") == 0) {
									email->freq = atoi(value);
								}
								else {
									debug_printf("Unknown %s: %s=%s", type, token, value);
								}
							}
					}
				}
			}

			/* REVISIT: Validate that the required fields are set for each type */
		}
	}

	fclose(fh);

	return 0;
}

void syslogd_discard_config(syslogd_config_t *config)
{
	while (config->local.common.next) {
		syslogd_target_t *target = config->local.common.next;

		config->local.common.next = target->next;

		free(target->priv);

		switch (target->target) {

            case SYSLOG_TARGET_LOCAL_FLASH:
                {
                    syslogd_local_config_t *flash = (syslogd_local_config_t *)target;

                    if (flash->logfile != __LOG_FILE_FLASH)
                        free(flash->logfile);
                }
                break;


#ifdef CONFIG_FEATURE_REMOTE_LOG
			case SYSLOG_TARGET_REMOTE:
				{
					syslogd_remote_config_t *remote = (syslogd_remote_config_t *)target;

					free(remote->host);
				}
				break;
#endif

#ifdef CONFIG_USER_SMTP_SMTPCLIENT
			case SYSLOG_TARGET_EMAIL:
				{
					syslogd_email_config_t *email = (syslogd_email_config_t *)target;

					free(email->server);
					free(email->addr);
					free(email->fromhost);
					free(email->sender);
				}
				break;
#endif
			default:
				break;
		}
		free(target);
	}

	if (config->local.logfile != __LOG_FILE)
		free(config->local.logfile);
	free(config->local.common.priv);
}

int
syslogd_write_pid(char* filename)
{
    char buf[4] = {0};

	FILE *fh = fopen(filename, "w");

	if (NULL == fh) return 0;

    sprintf(buf, "%d\n", getpid());
    fputs(buf, fh);
	fclose(fh);

	return 1;
}

