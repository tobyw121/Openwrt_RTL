/*
 * WPA Supplicant / main() function for UNIX like OSes and MinGW
 * Copyright (c) 2003-2013, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include "includes.h"
#ifdef __linux__
#include <fcntl.h>
#endif /* __linux__ */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "common.h"
#include "fst/fst.h"
#include "wpa_supplicant_i.h"
#include "driver_i.h"
#include "p2p_supplicant.h"


static void usage(void)
{
	int i;
	printf("%s\n\n%s\n"
	       "usage:\n"
	       "  wpa_supplicant [-BddhKLqq"
#ifdef CONFIG_DEBUG_SYSLOG
	       "s"
#endif /* CONFIG_DEBUG_SYSLOG */
	       "t"
#ifdef CONFIG_CTRL_IFACE_DBUS_NEW
	       "u"
#endif /* CONFIG_CTRL_IFACE_DBUS_NEW */
	       "vW] [-P<pid file>] "
	       "[-g<global ctrl>] \\\n"
	       "        [-G<group>] \\\n"
	       "        -i<ifname> -c<config file> [-C<ctrl>] [-D<driver>] "
	       "[-p<driver_param>] \\\n"
	       "        [-b<br_ifname>] [-e<entropy file>]"
#ifdef CONFIG_DEBUG_FILE
	       " [-f<debug file>]"
#endif /* CONFIG_DEBUG_FILE */
	       " \\\n"
	       "        [-o<override driver>] [-O<override ctrl>] \\\n"
	       "        [-N -i<ifname> -c<conf> [-C<ctrl>] "
	       "[-D<driver>] \\\n"
#ifdef CONFIG_P2P
	       "        [-m<P2P Device config file>] \\\n"
#endif /* CONFIG_P2P */
	       "        [-p<driver_param>] [-b<br_ifname>] [-I<config file>] "
	       "...]\n"
	       "\n"
	       "drivers:\n",
	       wpa_supplicant_version, wpa_supplicant_license);

	for (i = 0; wpa_drivers[i]; i++) {
		printf("  %s = %s\n",
		       wpa_drivers[i]->name,
		       wpa_drivers[i]->desc);
	}

#ifndef CONFIG_NO_STDOUT_DEBUG
	printf("options:\n"
	       "  -b = optional bridge interface name\n"
	       "  -B = run daemon in the background\n"
	       "  -c = Configuration file\n"
	       "  -C = ctrl_interface parameter (only used if -c is not)\n"
	       "  -d = increase debugging verbosity (-dd even more)\n"
	       "  -D = driver name (can be multiple drivers: nl80211,wext)\n"
	       "  -e = entropy file\n"
#ifdef CONFIG_DEBUG_FILE
	       "  -f = log output to debug file instead of stdout\n"
#endif /* CONFIG_DEBUG_FILE */
	       "  -g = global ctrl_interface\n"
	       "  -G = global ctrl_interface group\n"
	       "  -h = show this help text\n"
	       "  -i = interface name\n"
	       "  -I = additional configuration file\n"
	       "  -K = include keys (passwords, etc.) in debug output\n"
	       "  -L = show license (BSD)\n"
#ifdef CONFIG_P2P
	       "  -m = Configuration file for the P2P Device interface\n"
#endif /* CONFIG_P2P */
#ifdef CONFIG_MATCH_IFACE
	       "  -M = start describing new matching interface\n"
#endif /* CONFIG_MATCH_IFACE */
	       "  -N = start describing new interface\n"
	       "  -o = override driver parameter for new interfaces\n"
	       "  -O = override ctrl_interface parameter for new interfaces\n"
	       "  -p = driver parameters\n"
	       "  -P = PID file\n"
	       "  -q = decrease debugging verbosity (-qq even less)\n"
#ifdef CONFIG_DEBUG_SYSLOG
	       "  -s = log output to syslog instead of stdout\n"
#endif /* CONFIG_DEBUG_SYSLOG */
	       "  -t = include timestamp in debug messages\n"
#ifdef CONFIG_DEBUG_LINUX_TRACING
	       "  -T = record to Linux tracing in addition to logging\n"
	       "       (records all messages regardless of debug verbosity)\n"
#endif /* CONFIG_DEBUG_LINUX_TRACING */
#ifdef CONFIG_CTRL_IFACE_DBUS_NEW
	       "  -u = enable DBus control interface\n"
#endif /* CONFIG_CTRL_IFACE_DBUS_NEW */
	       "  -v = show version\n"
	       "  -W = wait for a control interface monitor before starting\n");

	printf("example:\n"
	       "  wpa_supplicant -D%s -iwlan0 -c/etc/wpa_supplicant.conf\n",
	       wpa_drivers[0] ? wpa_drivers[0]->name : "nl80211");
#endif /* CONFIG_NO_STDOUT_DEBUG */
}


static void license(void)
{
#ifndef CONFIG_NO_STDOUT_DEBUG
	printf("%s\n\n%s%s%s%s%s\n",
	       wpa_supplicant_version,
	       wpa_supplicant_full_license1,
	       wpa_supplicant_full_license2,
	       wpa_supplicant_full_license3,
	       wpa_supplicant_full_license4,
	       wpa_supplicant_full_license5);
#endif /* CONFIG_NO_STDOUT_DEBUG */
}


static void wpa_supplicant_fd_workaround(int start)
{
#ifdef __linux__
	static int fd[3] = { -1, -1, -1 };
	int i;
	/* When started from pcmcia-cs scripts, wpa_supplicant might start with
	 * fd 0, 1, and 2 closed. This will cause some issues because many
	 * places in wpa_supplicant are still printing out to stdout. As a
	 * workaround, make sure that fd's 0, 1, and 2 are not used for other
	 * sockets. */
	if (start) {
		for (i = 0; i < 3; i++) {
			fd[i] = open("/dev/null", O_RDWR);
			if (fd[i] > 2) {
				close(fd[i]);
				fd[i] = -1;
				break;
			}
		}
	} else {
		for (i = 0; i < 3; i++) {
			if (fd[i] >= 0) {
				close(fd[i]);
				fd[i] = -1;
			}
		}
	}
#endif /* __linux__ */
}

#ifdef WPAS_DYNAMIC_DEBUG
static int debug_on = MSG_ERROR;
#define FILE_WPAS_DEBUG 	"/tmp/wpas_debug"

static void wpas_write_file(char *path_name, char *MSG)
{
	int fd;

	if ((fd = open(path_name, O_RDWR | O_CREAT,S_IRWXU)) == -1) {
		 printf("open file:%s fail\n",path_name);
		 return;
        }
	if (write(fd, MSG, strlen(MSG)) == -1) {
	 	printf("write %s to %s fail\n", MSG, path_name);
	}

	close(fd);
}

static void issue_signal_to_wpas(int SIG_NUMBER ,char *interName)
{
	int pid;
	FILE *fp;
	char line[100]={0}, pid_filename[100]={0}, ifname[100]={0}, *p = NULL;

	snprintf(ifname, sizeof
(
ifname), "%s", interName);

	if(p = strstr(ifname, "-"))
		*p = '_';

	snprintf(pid_filename, sizeof(pid_filename), "/var/run/wpas_%s_pid.pid", ifname);

	if ((fp = fopen(pid_filename, "r")) != NULL) {
		fgets(line, sizeof(line), fp);
		if (sscanf(line, "%d", &pid)) {
			if (pid > 1) {
				kill(pid, SIG_NUMBER);
			}
		}
		fclose(fp);
	}else
		printf("!!!!!!!open %s fail !!!!\n", pid_filename);

	return;
}

//MSG_EXCESSIVE, MSG_MSGDUMP, MSG_DEBUG, MSG_INFO, MSG_WARNING, MSG_ERROR
char *cmd2str(int cmd)
{
	if(cmd==MSG_EXCESSIVE)
		return "MSG_EXCESSIVE";
	else if(cmd==MSG_MSGDUMP)
		return "MSG_MSGDUMP";
	else if(cmd==MSG_DEBUG)
		return "MSG_DEBUG";
	else if(cmd==MSG_INFO)
		return "MSG_INFO";
	else if(cmd==MSG_WARNING)
		return "MSG_WARNING";
	else if(cmd==MSG_ERROR)
		return "MSG_ERROR";
	else
		return "UNKNOWN";
}

void printf_debug_info()
{
	printf("	%d:%s\n", MSG_EXCESSIVE, cmd2str(MSG_EXCESSIVE));
	printf("	%d:%s\n", MSG_MSGDUMP, cmd2str(MSG_MSGDUMP));
	printf("	%d:%s\n", MSG_DEBUG, cmd2str(MSG_DEBUG));
	printf("	%d:%s\n", MSG_INFO, cmd2str(MSG_INFO));
	printf("	%d:%s\n", MSG_WARNING, cmd2str(MSG_WARNING));
	printf("	%d:%s\n", MSG_ERROR, cmd2str(MSG_ERROR));

	return ;
}

static int parse_argument(int argc, char *argv[])
{
	int argNum=1, i, ret=0;
	char tmpbuf[100], ifname[100]={0};

	while (argNum < argc) {
		if ( !strcmp(argv[argNum], "-debug")) {
			if(argc<4){
				printf("[usage]hostapd -debug [wlanx] [level]\n");
				printf_debug_info();
				return -1;
			}

			snprintf(ifname, sizeof(ifname), "%s", argv[++argNum]);
			debug_on = atoi(argv[++argNum]);
			printf("ifname:%s, debug level:%s\n", ifname, cmd2str(debug_on));
			snprintf(tmpbuf, sizeof(tmpbuf), "%d", debug_on);
			wpas_write_file(FILE_WPAS_DEBUG, tmpbuf);
			issue_signal_to_wpas(SIGUSR2 ,ifname);
			return -1;
		}
		argNum++;
	}

	return ret;
}
#endif


#ifdef CONFIG_MATCH_IFACE
static int wpa_supplicant_init_match(struct wpa_global *global)
{
	/*
	 * The assumption is that the first driver is the primary driver and
	 * will handle the arrival / departure of interfaces.
	 */
	if (wpa_drivers[0]->global_init && !global->drv_priv[0]) {
		global->drv_priv[0] = wpa_drivers[0]->global_init(global);
		if (!global->drv_priv[0]) {
			wpa_printf(MSG_ERROR,
				   "Failed to initialize driver '%s'",
				   wpa_drivers[0]->name);
			return -1;
		}
	}

	return 0;
}
#endif /* CONFIG_MATCH_IFACE */


int main(int argc, char *argv[])
{
	int c, i;
	struct wpa_interface *ifaces, *iface;
	int iface_count, exitcode = -1;
	struct wpa_params params;
	struct wpa_global *global;

	if (os_program_init())
		return -1;

	os_memset(&params, 0, sizeof(params));
#ifdef WPAS_DYNAMIC_DEBUG
	params.wpa_debug_level = MSG_ERROR;
#else
	params.wpa_debug_level = MSG_INFO;
#endif

	iface = ifaces = os_zalloc(sizeof(struct wpa_interface));
	if (ifaces == NULL)
		return -1;
	iface_count = 1;

	wpa_supplicant_fd_workaround(1);

#ifdef WPAS_DYNAMIC_DEBUG
	if(parse_argument(argc, argv)<0){
		return 1;
	}
#endif

	for (;;) {
		c = getopt(argc, argv,
			   "b:Bc:C:D:de:f:g:G:hi:I:KLMm:No:O:p:P:qsTtuvW");
		if (c < 0)
			break;
		switch (c) {
		case 'b':
			iface->bridge_ifname = optarg;
			break;
		case 'B':
			params.daemonize++;
			break;
		case 'c':
			iface->confname = optarg;
			break;
		case 'C':
			iface->ctrl_interface = optarg;
			break;
		case 'D':
			iface->driver = optarg;
			break;
		case 'd':
#ifdef CONFIG_NO_STDOUT_DEBUG
			printf("Debugging disabled with "
			       "CONFIG_NO_STDOUT_DEBUG=y build time "
			       "option.\n");
			goto out;
#else /* CONFIG_NO_STDOUT_DEBUG */
			params.wpa_debug_level--;
			break;
#endif /* CONFIG_NO_STDOUT_DEBUG */
		case 'e':
			params.entropy_file = optarg;
			break;
#ifdef CONFIG_DEBUG_FILE
		case 'f':
			params.wpa_debug_file_path = optarg;
			break;
#endif /* CONFIG_DEBUG_FILE */
		case 'g':
			params.ctrl_interface = optarg;
			break;
		case 'G':
			params.ctrl_interface_group = optarg;
			break;
		case 'h':
			usage();
			exitcode = 0;
			goto out;
		case 'i':
			iface->ifname = optarg;
			break;
		case 'I':
			iface->confanother = optarg;
			break;
		case 'K':
			params.wpa_debug_show_keys++;
			break;
		case 'L':
			license();
			exitcode = 0;
			goto out;
#ifdef CONFIG_P2P
		case 'm':
			params.conf_p2p_dev = optarg;
			break;
#endif /* CONFIG_P2P */
		case 'o':
			params.override_driver = optarg;
			break;
		case 'O':
			params.override_ctrl_interface = optarg;
			break;
		case 'p':
			iface->driver_param = optarg;
			break;
		case 'P':
			os_free(params.pid_file);
			params.pid_file = os_rel2abs_path(optarg);
			break;
		case 'q':
			params.wpa_debug_level++;
			break;
#ifdef CONFIG_DEBUG_SYSLOG
		case 's':
			params.wpa_debug_syslog++;
			break;
#endif /* CONFIG_DEBUG_SYSLOG */
#ifdef CONFIG_DEBUG_LINUX_TRACING
		case 'T':
			params.wpa_debug_tracing++;
			break;
#endif /* CONFIG_DEBUG_LINUX_TRACING */
		case 't':
			params.wpa_debug_timestamp++;
			break;
#ifdef CONFIG_CTRL_IFACE_DBUS_NEW
		case 'u':
			params.dbus_ctrl_interface = 1;
			break;
#endif /* CONFIG_CTRL_IFACE_DBUS_NEW */
		case 'v':
			printf("%s\n", wpa_supplicant_version);
			exitcode = 0;
			goto out;
		case 'W':
			params.wait_for_monitor++;
			break;
#ifdef CONFIG_MATCH_IFACE
		case 'M':
			params.match_iface_count++;
			iface = os_realloc_array(params.match_ifaces,
						 params.match_iface_count,
						 sizeof(struct wpa_interface));
			if (!iface)
				goto out;
			params.match_ifaces = iface;
			iface = &params.match_ifaces[params.match_iface_count -
						     1];
			os_memset(iface, 0, sizeof(*iface));
			break;
#endif /* CONFIG_MATCH_IFACE */
		case 'N':
			iface_count++;
			iface = os_realloc_array(ifaces, iface_count,
						 sizeof(struct wpa_interface));
			if (iface == NULL)
				goto out;
			ifaces = iface;
			iface = &ifaces[iface_count - 1];
			os_memset(iface, 0, sizeof(*iface));
			break;
		default:
			usage();
			exitcode = 0;
			goto out;
		}
	}

	exitcode = 0;
	global = wpa_supplicant_init(&params);
	if (global == NULL) {
		wpa_printf(MSG_ERROR, "Failed to initialize wpa_supplicant");
		exitcode = -1;
		goto out;
	} else {
		wpa_printf(MSG_INFO, "Successfully initialized "
			   "wpa_supplicant");
	}

	if (fst_global_init()) {
		wpa_printf(MSG_ERROR, "Failed to initialize FST");
		exitcode = -1;
		goto out;
	}

#if defined(CONFIG_FST) && defined(CONFIG_CTRL_IFACE)
	if (!fst_global_add_ctrl(fst_ctrl_cli))
		wpa_printf(MSG_WARNING, "Failed to add CLI FST ctrl");
#endif

	for (i = 0; exitcode == 0 && i < iface_count; i++) {
		struct wpa_supplicant *wpa_s;

		if ((ifaces[i].confname == NULL &&
		     ifaces[i].ctrl_interface == NULL) ||
		    ifaces[i].ifname == NULL) {
			if (iface_count == 1 && (params.ctrl_interface ||
#ifdef CONFIG_MATCH_IFACE
						 params.match_iface_count ||
#endif /* CONFIG_MATCH_IFACE */
						 params.dbus_ctrl_interface))
				break;
			usage();
			exitcode = -1;
			break;
		}
		wpa_s = wpa_supplicant_add_iface(global, &ifaces[i], NULL);
		if (wpa_s == NULL) {
			exitcode = -1;
			break;
		}
	}

#ifdef CONFIG_MATCH_IFACE
	if (exitcode == 0)
		exitcode = wpa_supplicant_init_match(global);
#endif /* CONFIG_MATCH_IFACE */

	if (exitcode == 0)
		exitcode = wpa_supplicant_run(global);

	wpa_supplicant_deinit(global);

	fst_global_deinit();

out:
	wpa_supplicant_fd_workaround(0);
	os_free(ifaces);
#ifdef CONFIG_MATCH_IFACE
	os_free(params.match_ifaces);
#endif /* CONFIG_MATCH_IFACE */
	os_free(params.pid_file);

	os_program_deinit();

	return exitcode;
}
