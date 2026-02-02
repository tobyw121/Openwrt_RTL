#include "wlan_manager.h"
#ifdef CONFIG_BAND_STEERING
#include "band_steering.h"
#endif
#include <poll.h>
#include <errno.h>
#ifdef CONFIG_CROSSBAND_REPEATER
#include "crossband_daemon.h"
#endif

sigset_t sig;
static s32 sock_fd;
static struct nlmsghdr *nlh = NULL;
static struct iovec iov;
static struct msghdr msgh;
static struct sockaddr_nl s_addr, d_addr;
static struct com_device global_device = {0};
u8 global_dbg_level = _MSG_INFO_; /* TBD */

#ifdef CONFIG_USER_RTK_COMMON_NETLINK
static s32 sock_fd_sec;
static struct nlmsghdr *nlh_sec = NULL;
static struct iovec iov_sec;
static struct msghdr msgh_sec;
static struct sockaddr_nl s_sec_addr, d_sec_addr;
#endif

#if defined(CONFIG_BAND_STEERING) && defined(INITIATIVE_DUAL_BAND_DETECT)
unsigned char null_mac[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
#endif

/* ---------------------------------------- */
/* ---------- wlan manager APIs  ---------- */
/* ---------------------------------------- */
void _wlan_manager_hostapd_cli_bss_tm_req(
	s8 *intf_name, u8 *sta_mac, u8 *bss_mac, u16 bss_ch,
	u16 bss_info, u8 bss_reg_class, u8 bss_phy_type,
	u8 disassoc_imminent, u32 disassoc_timer)
{
	s8 sys_cmd[256] = {0};
	/* hostapd_cli bss_tm_req parameters */
	u8 valid_int = 100;
	u8 pref = 1;
	u8 preference = 255;
	u8 abridged = 1;

#if 0
	/* add neighbor report */
	sprintf(sys_cmd,
		"echo add "MAC_FMT" %u %u %u %u %u > "PROC_NB_REPORT_FMT"",
		MAC_ARG(bss_mac), bss_info, bss_reg_class,
		bss_ch, bss_phy_type, preference, intf_name);

	MSG_PRINT(WLAN_MANAGER_STR, "%s.", sys_cmd);
	system(sys_cmd);

	os_memset(sys_cmd, 0, sizeof(sys_cmd));

	/* tag neighbor report */
	sprintf(sys_cmd,
		"echo tag "MAC_FMT" 1 > "PROC_NB_REPORT_FMT"",
		MAC_ARG(bss_mac), intf_name);

	MSG_PRINT(WLAN_MANAGER_STR, "%s.", sys_cmd);
	system(sys_cmd);
#endif

	os_memset(sys_cmd, 0, sizeof(sys_cmd));

	/* send bss tm req */
#if defined(CONFIG_BAND_STEERING) && defined(INITIATIVE_DUAL_BAND_DETECT)
	if (!os_memcmp(bss_mac, null_mac, 6))
		sprintf(sys_cmd, "hostapd_cli -i %s bss_tm_req "MAC_FMT"",
			intf_name, MAC_ARG(sta_mac));
	else
#endif
	sprintf(sys_cmd,
		"hostapd_cli -i %s bss_tm_req "MAC_FMT" "BSS_TM_REQ_PARAMS_FMT"",
		intf_name, MAC_ARG(sta_mac), valid_int, pref, disassoc_imminent,
		disassoc_timer, abridged,
		MAC_ARG(bss_mac), bss_info, bss_reg_class, bss_ch, bss_phy_type);

	MSG_PRINT(WLAN_MANAGER_STR, "%s.", sys_cmd);
	system(sys_cmd);

	return;
}

void _wlan_manager_hostapd_cli_deauth(s8 *intf_name, u8 *sta_mac)
{
	s8 sys_cmd[128] = {0};

	/* send deauth */
	sprintf(sys_cmd,
		"hostapd_cli -i %s deauthenticate "MAC_FMT"",
		intf_name, MAC_ARG(sta_mac));

	MSG_PRINT(WLAN_MANAGER_STR, "%s.", sys_cmd);
	system(sys_cmd);

	return;
}

void _wlan_manager_set_msg(
	struct nl_message *msg, u32 *msg_len, void *elm, u32 elm_len)
{
	os_memcpy(msg->content + (*msg_len), elm, elm_len);
	(*msg_len) += elm_len;
}

void _wlan_manager_send_drv(void *msg, u32 msg_len)
{
	s32 status;

	os_memset(nlh, 0, NLMSG_SPACE(NL_MAX_MSG_SIZE));
	nlh->nlmsg_len = NLMSG_SPACE(NL_MAX_MSG_SIZE);
	nlh->nlmsg_pid = NL_WLAN_MANAGER_PID;
	nlh->nlmsg_flags = 0;
	os_memcpy(NLMSG_DATA(nlh), msg, msg_len);

	/* send message */
	status = sendmsg(sock_fd, &msgh, 0);
	if (status < 0)
		MSG_ERR(WLAN_MANAGER_STR, "%s send kernel error %u %s.",
			__FUNCTION__, errno, strerror(errno));

	return;
}

#ifdef CONFIG_USER_RTK_COMMON_NETLINK
void _wlan_manager_send_sec_drv(void *msg, u32 msg_len)
{
	s32 status;

	os_memset(nlh_sec, 0, NLMSG_SPACE(NL_MAX_MSG_SIZE));
	nlh_sec->nlmsg_len = NLMSG_SPACE(NL_MAX_MSG_SIZE);
	nlh_sec->nlmsg_pid = NL_WLAN_MANAGER_PID;
	nlh_sec->nlmsg_flags = 0;
	os_memcpy(NLMSG_DATA(nlh_sec), msg, msg_len);

	/* send message */
	status = sendmsg(sock_fd_sec, &msgh_sec, 0);
	if (status < 0)
		MSG_ERR(WLAN_MANAGER_STR, "%s send kernel error %u %s.",
			__FUNCTION__, errno, strerror(errno));

	return;
}
#endif

void _wlan_manager_send_daemon(void *msg, u32 msg_len, pid_t pid)
{
	s32 status;

	/* create netlink */
	sock_fd = socket(AF_NETLINK, SOCK_RAW, NL_RTK_PROTOCOL);
	if (sock_fd == -1) {
		MSG_ERR(WLAN_MANAGER_STR, "create socket error.");
		return;
	}

	/* source address */
	os_memset(&s_addr, 0, sizeof(s_addr));
	s_addr.nl_family = AF_NETLINK;
	s_addr.nl_pid = getpid();
	s_addr.nl_groups = 0;

	/* destination address */
	os_memset(&d_addr, 0, sizeof(d_addr));
	d_addr.nl_family = AF_NETLINK;
	d_addr.nl_pid = pid;
	d_addr.nl_groups = 0;

	/* bind socket */
	if (bind(sock_fd, (struct sockaddr *)&s_addr, sizeof(s_addr)) != 0) {
		MSG_ERR(WLAN_MANAGER_STR, "bind socket error.");
		close(sock_fd);
		return;
	}

	/* allocate netlink header */
	nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(NL_MAX_MSG_SIZE));
	if (!nlh) {
		MSG_ERR(WLAN_MANAGER_STR, "malloc nlmsghdr error.");
		close(sock_fd);
		return;
	}
	os_memset(nlh, 0, NLMSG_SPACE(NL_MAX_MSG_SIZE));
	nlh->nlmsg_len = NLMSG_SPACE(NL_MAX_MSG_SIZE);
	nlh->nlmsg_pid = NL_WLAN_MANAGER_PID;
	nlh->nlmsg_flags = 0;
	os_memcpy(NLMSG_DATA(nlh), msg, msg_len);

	/* iov structure */
	iov.iov_base = (void *)nlh;
	iov.iov_len = NLMSG_SPACE(NL_MAX_MSG_SIZE);

	/* msghdr */
	os_memset(&msgh, 0, sizeof(msgh));
	msgh.msg_name = (void *)&d_addr;
	msgh.msg_namelen = sizeof(d_addr);
	msgh.msg_iov = &iov;
	msgh.msg_iovlen = 1;

	/* send message */
	status = sendmsg(sock_fd, &msgh, 0);
	if (status < 0)
		MSG_ERR(WLAN_MANAGER_STR, "send error.");

	close(sock_fd);

	return;
}

/* ---------------------------------------- */
/* ---------- wlan manager core  ---------- */
/* ---------------------------------------- */
static void send_daemon_on_msg(void)
{
	u32 msg_len = 0;
	struct nl_message msg = {0};

	/* finish message */
	msg.type = NL_DAEMON_ON_TYPE;
	msg.len = msg_len;

	/* length += (type + len) */
	msg_len = msg.len + 8;
	_wlan_manager_send_drv((void *)&msg, msg_len);
#ifdef CONFIG_USER_RTK_COMMON_NETLINK
	_wlan_manager_send_sec_drv((void *)&msg, msg_len);
#endif
	return;
}

static void send_daemon_off_msg(void)
{
	u32 msg_len = 0;
	struct nl_message msg = {0};

	/* finish message */
	msg.type = NL_DAEMON_OFF_TYPE;
	msg.len = msg_len;

	/* length += (type + len) */
	msg_len = msg.len + 8;
	_wlan_manager_send_drv((void *)&msg, msg_len);
#ifdef CONFIG_USER_RTK_COMMON_NETLINK
	_wlan_manager_send_sec_drv((void *)&msg, msg_len);
#endif

	exit(0);
}

/* TBD: msg */
static void send_del_sta_msg(struct com_device *device, u8 band, u8 *mac)
{
	u8 i;
	struct com_priv *priv = NULL;

	for (i = 0; i < SSID_NUM; i++) {
		priv = &(device->priv[band][i]);

		if (priv->active)
			_wlan_manager_hostapd_cli_deauth(priv->name, mac);
	}

	return;
}

static void send_priv_info_cmd_msg(u8 band, u8 ssid)
{
	u32 msg_len = 0;
	struct nl_message msg = {0};
	struct elm_header hdr = {0};
	struct elm_intf intf = {0};

	/* element header */
	hdr.id = ELM_INTF_ID;
	hdr.len = ELM_INTF_LEN;
	_wlan_manager_set_msg(&msg, &msg_len, (void *)&hdr, ELM_HEADER_LEN);

	/* element: ELM_INTF_ID */
	intf.band = band;
	intf.ssid = ssid;
	_wlan_manager_set_msg(&msg, &msg_len, (void *)&intf, ELM_INTF_LEN);

	/* finish message */
	msg.type = NL_PRIV_INFO_CMD_TYPE;
	msg.len = msg_len;

	/* length += (type + len) */
	msg_len = msg.len + 8;
	_wlan_manager_send_daemon((void *)&msg, msg_len, NL_WLAN_MANAGER_PID);

	return;
}

static void send_config_update_cmd_msg(u8 *config)
{
	u32 msg_len = 0;
	struct nl_message msg = {0};
	struct elm_header hdr = {0};
	struct elm_buffer buffer = {0};

	/* element header */
	hdr.id = ELM_BUFFER_ID;
	hdr.len = ELM_BUFFER_LEN;
	_wlan_manager_set_msg(&msg, &msg_len, (void *)&hdr, ELM_HEADER_LEN);

	/* element: ELM_BUFFER_ID */
	os_memcpy(buffer.buf, config, 255);
	_wlan_manager_set_msg(&msg, &msg_len, (void *)&buffer, ELM_BUFFER_LEN);

	/* finish message */
	msg.type = NL_CONFIG_UPDATE_TYPE;
	msg.len = msg_len;

	/* length += (type + len) */
	msg_len = msg.len + 8;
	_wlan_manager_send_daemon((void *)&msg, msg_len, NL_WLAN_MANAGER_PID);

	return;
}

static s8 config_fname_parse_arg(
	u8 *argn, s32 argc, s8 *argv[], struct com_device *device)
{
	if (*argn + 1 >= argc) {
		MSG_WARN(WLAN_MANAGER_STR, "wrong argument.");
		return -1;
	}

	/* TBD */
	os_strcpy(device->config_fname, argv[*argn + 1]);

	MSG_PRINT(WLAN_MANAGER_STR, "config: %s.", device->config_fname);

	*argn = *argn + 1;

	return 0;
}

static void priv_info_parse_arg(u8 *argn, s32 argc, s8 *argv[])
{
	u8 band = 255;
	u8 ssid = 255;

	if (*argn + 2 >= argc) {
		MSG_WARN(WLAN_MANAGER_STR, "wrong argument.");
		return;
	}

	if (!os_strcmp(argv[*argn + 1], "2g"))
		band = BAND_ON_24G;
	else if (!os_strcmp(argv[*argn + 1], "5g"))
		band = BAND_ON_5G;

	ssid = argv[*argn + 2][0] - '0';

	if (band > BAND_ON_5G || ssid >= SSID_NUM) {
		MSG_WARN(WLAN_MANAGER_STR, "band should be '2g' or '5g'. ssid should be 0-%d.",
			SSID_NUM - 1);
		return;
	}

	send_priv_info_cmd_msg(band, ssid);

	*argn = *argn + 2;

	return;
}

static void config_update_parse_arg(u8 *argn, s32 argc, s8 *argv[])
{
	u8 config[255] = {0};

	if (*argn + 1 >= argc) {
		MSG_WARN(WLAN_MANAGER_STR, "wrong argument.");
		return;
	}

	os_strcpy(config, argv[*argn + 1]);

	send_config_update_cmd_msg(config);

	*argn = *argn + 1;

	return;
}

static s32 parse_argument(
	int argc, char *argv[], struct com_device *device)
{
	u8 argn = 1;

	while (argn < argc) {
		if (!os_strcmp(argv[argn], "-config_fname")) {
			if (config_fname_parse_arg(&argn, argc, argv, device) < 0)
				return -1;
		}
		else if (!os_strcmp(argv[argn], "-priv_info")) {
			priv_info_parse_arg(&argn, argc, argv);
			return -1;
		}
		else if (!os_strcmp(argv[argn], "-config_update")) {
			config_update_parse_arg(&argn, argc, argv);
			return -1;
		}
#ifdef CONFIG_BAND_STEERING
		else if (!os_strcmp(argv[argn], "-band_steering")) {
			_band_steering_parse_arg(&argn, argc, argv);
			return -1;
		}
#endif
#ifdef CONFIG_CROSSBAND_REPEATER
		else if (!os_strcmp(argv[argn], "-crossband_dump")) {
			config_crossband_dump_parse_arg(&argn, argc, argv);
			return -1;
		}
#endif
		else {
			MSG_WARN(WLAN_MANAGER_STR, "invalid argument (%s).",
				argv[argn]);
			return -1;
		}
		argn++;
	}

	return 0;
}

static struct com_sta *sta_info_lookup(
	struct com_priv *priv, u8 *mac)
{
	u64 offset;
	u32 hash_idx;
	struct com_wifi_list *list;
	struct com_sta *sta = NULL;

	offset = (u64)(&((struct com_sta *)0)->list);
	hash_idx = wifi_mac_hash(mac);
	list = priv->sta_hash_tbl[hash_idx];

	while (list) {
		sta = (struct com_sta *)((u64)list - offset);
		if (!os_memcmp(sta->mac, mac, 6))
			return sta;

		list = list->next;
	}

	return NULL;
}

static struct com_frame *frame_info_lookup(
	struct com_priv *priv, u8 *mac)
{
	u64 offset;
	u32 hash_idx;
	struct com_wifi_list *list;
	struct com_frame *frame = NULL;

	offset = (u64)(&((struct com_frame *)0)->list);
	hash_idx = wifi_mac_hash(mac);
	list = priv->frame_hash_tbl[hash_idx];

	while (list) {
		frame = (struct com_frame *)((u64)list - offset);
		if (!os_memcmp(frame->sa, mac, 6))
			return frame;

		list = list->next;
	}

	return NULL;
}

static void on_del_sta(
	struct com_priv *priv,
	struct elm_sta_info *sta_info)
{
	u64 offset;
	u32 hash_idx;
	u8 *mac = sta_info->mac;
	struct com_wifi_list *list;
	struct com_sta *sta = NULL;

	sta = sta_info_lookup(priv, mac);

	if (sta == NULL)
		return;

	offset = (u64)(&((struct com_sta *)0)->list);
	hash_idx = wifi_mac_hash(mac);
	list = priv->sta_hash_tbl[hash_idx];

	while (list) {
		sta = (struct com_sta *)((u64)list - offset);
		if (!os_memcmp(sta->mac, mac, 6)) {
			sta->used = 0;
			priv->sta_num--;
			wifi_list_unlink(list);
		}

		list = list->next;
	}

	return;
}

static void on_new_sta(
	struct com_device *device,
	struct com_priv *priv,
	struct elm_sta_info *sta_info)
{
	u8  i;
	u32 hash_idx;
	u8  *mac = sta_info->mac;
	u8  del_band = (priv->band == BAND_ON_24G) ? BAND_ON_5G : BAND_ON_24G;
	struct com_sta *sta = NULL;

	MSG_INFO(WLAN_MANAGER_STR, "new station "MAC_FMT" on %s.",
		MAC_ARG(mac), priv->name);

	/* delete first */
	on_del_sta(priv, sta_info);

	/* find an empty entry */
	for (i = 0; i < MAX_STA_NUM; i++) {
		if (!priv->sta_list[i].used) {
			sta = &(priv->sta_list[i]);
			break;
		}
	}

	/* add the entry */
	if (sta) {
		os_memset(sta, 0, sizeof(*sta));
		sta->used = 1;
		os_memcpy(sta->mac, mac, 6);
		sta->rssi = sta_info->rssi;
		sta->link_time = 0;
		sta->tx_tp = 0;
		sta->rx_tp = 0;
		sta->is_dual_band = 0;
#ifdef CONFIG_BAND_STEERING
		sta->b_steer_roam_cnt = 0;
		sta->b_steer_roam_detect = 0;
#endif
		priv->sta_num++;
		/* hash update */
		hash_idx = wifi_mac_hash(mac);
		wifi_list_link(&(sta->list), &(priv->sta_hash_tbl[hash_idx]));
	}

	/* Wi-Fi driver deletes the station on another band */
	send_del_sta_msg(device, del_band, mac);

	return;
}

static void intf_update(
	struct com_priv *priv,
	struct elm_intf *intf)
{
	priv->active = 1;
	os_memcpy(priv->mac, intf->mac, 6);
	os_memcpy(priv->name, intf->name, 16);

	return;
}

static void intf_info_update(
	struct com_priv *priv,
	struct elm_intf_info *intf_info)
{
	priv->ch = intf_info->ch;
	priv->ch_clm = intf_info->ch_clm;

	priv->self_nb_rpt.bss_info = intf_info->bss_info;
	priv->self_nb_rpt.reg_class = intf_info->reg_class;
	priv->self_nb_rpt.phy_type = intf_info->phy_type;

	return;
}

static void on_intf_rpt(
	struct com_priv *priv,
	struct elm_intf *intf,
	struct elm_intf_info *intf_info)
{
	intf_update(priv, intf);
	intf_info_update(priv, intf_info);

	return;
}

static void on_sta_rpt(
	struct com_device *device,
	struct com_priv *priv,
	struct elm_sta_info *sta_info)
{
	u8 i;
	u8 *mac = sta_info->mac;
	u8 dual_band = (priv->band == BAND_ON_24G) ? BAND_ON_5G : BAND_ON_24G;
	struct com_sta *sta = NULL;
	struct com_frame *frame = NULL;
	struct com_priv *dual_priv = NULL;

	sta = sta_info_lookup(priv, mac);

	if (sta == NULL) /* maybe the sta_list is already full */
		return;

	sta->rssi = (sta_info->rssi) ? sta_info->rssi : sta->rssi;
	sta->link_time = sta_info->link_time;
	sta->tx_tp = sta_info->tx_tp;
	sta->rx_tp = sta_info->rx_tp;

	/* check if the station is dual band */
	if (sta->is_dual_band != 1) {
		for (i = 0; i < SSID_NUM; i++) {
			dual_priv = &(device->priv[dual_band][i]);
			frame = frame_info_lookup(dual_priv, mac);
			if (frame) {
				sta->is_dual_band = 1;
				break;
			}
		}
	}

#if defined(CONFIG_BAND_STEERING) && defined(INITIATIVE_DUAL_BAND_DETECT)
	/* if sta mac not found in cached frames, trigger sta to send probe request*/
	if (priv->band_steering_enable
	&& (sta->is_dual_band != 1)
	&& (sta->b_steer_bss_tm_req_cnt < BSS_TM_REQ_RETRY_LIMIT)
	&& ((sta->link_time % BSS_TM_REQ_INTERVAL) <= 1)
	&& (sta_info->tx_tp < BSS_TM_REQ_TP_LIMIT)
	&& (sta_info->rx_tp < BSS_TM_REQ_TP_LIMIT))
	{
		struct com_priv *grp_priv = priv->grp_priv;

		if (grp_priv && grp_priv->band_steering_enable) {
			_wlan_manager_hostapd_cli_bss_tm_req(
				priv->name, sta->mac, &null_mac,
				0, 0, 0, 0, 0, 0);
			sta->b_steer_bss_tm_req_cnt++;
		}
	}
#endif

	return;
}

static void on_frame_rpt(
	struct com_priv *priv,
	struct elm_frame_info *frame_info)
{
	u8  i;
	u32 tmp_i = 0;
	u32 tmp_aging = 0;
	u32 hash_idx;
	u8  *mac = frame_info->sa;
	struct com_frame *frame = NULL;

	frame = frame_info_lookup(priv, mac);

	/* already exist */
	if (frame) {
		frame->aging = 0;
		return;
	}

	/* find an empty entry */
	for (i = 0; i < MAX_STA_NUM; i++) {
		if (!priv->frame_db[i].used) {
			frame = &(priv->frame_db[i]);
			break;
		}
	}

	/* no empty, find the oldest one */
	if (frame == NULL) {
		for (i = 0; i < MAX_STA_NUM; i++) {
			if (tmp_aging < priv->frame_db[i].aging) {
				tmp_aging = priv->frame_db[i].aging;
				tmp_i = i;
			}
		}
		frame = &(priv->frame_db[tmp_i]);
		wifi_list_unlink(&(frame->list));
	}

	/* add the entry */
	frame->used = 1;
	frame->frame_type = frame_info->frame_type;
	os_memcpy(frame->sa, mac, 6);
	frame->aging = 0;
	/* hash update */
	hash_idx = wifi_mac_hash(mac);
	wifi_list_link(&(frame->list), &(priv->frame_hash_tbl[hash_idx]));

	return;
}

static void on_sta_ext_rpt(
	struct com_device *device,
	struct com_priv *priv,
	struct elm_sta_info_ext *sta_info_ext)
{
	u8 i;
	u8 *mac = sta_info_ext->mac;
	u8 dual_band = (priv->band == BAND_ON_24G) ? BAND_ON_5G : BAND_ON_24G;
	struct com_sta *sta = NULL;
	struct com_frame *frame = NULL;
	struct com_priv *dual_priv = NULL;

	sta = sta_info_lookup(priv, mac);

	if (sta == NULL) /* maybe the sta_list is already full */
		return;

	/* check if the station is dual band */
	if (sta_info_ext->supported_band & BAND_CAP_2G &&
			sta_info_ext->supported_band & BAND_CAP_5G) {
		sta->is_dual_band |= 1;
	}

	return;
}

static void on_priv_info_cmd(struct com_priv *priv)
{
	u8 i;
	u8 band = priv->band;
	u8 ssid = priv->ssid;
	s8 sys_cmd[64] = {0};
	FILE *fp;
	struct com_sta *sta;

	if (band >= BAND_NUM || ssid >= SSID_NUM) {
		MSG_WARN(WLAN_MANAGER_STR, "wrong argument.");
		return;
	}

	fp = fopen(PRIV_INFO_OUTPUT, "w");
	if (fp == NULL) {
		MSG_WARN(WLAN_MANAGER_STR, "can't open [%s].", PRIV_INFO_OUTPUT);
		return;
	}

	fprintf(fp, "[WLAN_MANAGER] priv_info.\n");
	fprintf(fp, "mac: "MAC_FMT"\n", MAC_ARG(priv->mac));
	fprintf(fp, "band: %s\n", BAND_NAME(priv->band));
	fprintf(fp, "ssid: %u\n", priv->ssid);
	fprintf(fp, "name: %s\n", priv->name);
	fprintf(fp, "ch: %u\n", priv->ch);
	fprintf(fp, "ch: %u%%\n", priv->ch_clm);
	fprintf(fp, "sta_num: %u\n", priv->sta_num);
	fprintf(fp, "======== STA LIST ========\n");
	for (i = 0; i < MAX_STA_NUM; i++) {
		sta = &(priv->sta_list[i]);
		if (!sta->used)
			continue;

		fprintf(fp, "mac: "MAC_FMT"\n", MAC_ARG(sta->mac));
		fprintf(fp, "rssi: %u\n", sta->rssi);
		fprintf(fp, "tx_tp: %u\n", sta->tx_tp);
		fprintf(fp, "rx_tp: %u\n", sta->rx_tp);
		fprintf(fp, "is_dual_band: %u\n", sta->is_dual_band);
#ifdef CONFIG_BAND_STEERING
		fprintf(fp, "b_steer_roam_cnt: %u\n", sta->b_steer_roam_cnt);
		fprintf(fp, "b_steer_roam_detect: %u\n", sta->b_steer_roam_detect);
#ifdef INITIATIVE_DUAL_BAND_DETECT
		fprintf(fp, "b_steer_bss_tm_req_cnt: %u\n", sta->b_steer_bss_tm_req_cnt);
#endif
#endif
		fprintf(fp, "--------------------------\n");
	}

	fclose(fp);

	sprintf(sys_cmd, "cat %s", PRIV_INFO_OUTPUT);
	system(sys_cmd);

	return;
}

static s32 daemon_init(struct com_device *device)
{
	u8 i, j;

	/* signal */
	sigemptyset(&sig);
	signal(SIGTERM, (void *)&send_daemon_off_msg);

	/* create netlink */
	sock_fd = socket(AF_NETLINK, SOCK_RAW, NL_RTK_PROTOCOL);
	if (sock_fd == -1) {
		MSG_ERR(WLAN_MANAGER_STR, "create socket error.");
		return -1;
	}

	/* source address */
	os_memset(&s_addr, 0, sizeof(s_addr));
	s_addr.nl_family = AF_NETLINK;
	s_addr.nl_pid = NL_WLAN_MANAGER_PID;
	s_addr.nl_groups = 0;

	/* destination address */
	os_memset(&d_addr, 0, sizeof(d_addr));
	d_addr.nl_family = AF_NETLINK;
	d_addr.nl_pid = 0;
	d_addr.nl_groups = 0;

	/* bind socket */
	if (bind(sock_fd, (struct sockaddr *)&s_addr, sizeof(s_addr)) != 0) {
		MSG_ERR(WLAN_MANAGER_STR, "bind socket error.");
		close(sock_fd);
		return -1;
	}

	/* allocate netlink header */
	nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(NL_MAX_MSG_SIZE));
	if (!nlh) {
		MSG_ERR(WLAN_MANAGER_STR, "malloc nlmsghdr error.");
		close(sock_fd);
		return -1;
	}
	os_memset(nlh, 0, NLMSG_SPACE(NL_MAX_MSG_SIZE));
	nlh->nlmsg_len = NLMSG_SPACE(NL_MAX_MSG_SIZE);
	nlh->nlmsg_pid = NL_WLAN_MANAGER_PID;
	nlh->nlmsg_flags = 0;

	/* iov structure */
	iov.iov_base = (void *)nlh;
	iov.iov_len = NLMSG_SPACE(NL_MAX_MSG_SIZE);

	/* msghdr */
	os_memset(&msgh, 0, sizeof(msgh));
	msgh.msg_name = (void *)&d_addr;
	msgh.msg_namelen = sizeof(d_addr);
	msgh.msg_iov = &iov;
	msgh.msg_iovlen = 1;

#ifdef CONFIG_USER_RTK_COMMON_NETLINK
	/* create netlink */
	sock_fd_sec = socket(AF_NETLINK, SOCK_RAW, NL_RTK_PROTOCOL_SEC_DRV);
	if (sock_fd_sec == -1) {
		MSG_ERR(WLAN_MANAGER_STR, "create socket error.");
		return -1;
	}

	/* source address */
	os_memset(&s_sec_addr, 0, sizeof(s_sec_addr));
	s_sec_addr.nl_family = AF_NETLINK;
	s_sec_addr.nl_pid = NL_WLAN_MANAGER_PID;
	s_sec_addr.nl_groups = 0;

	/* destination address */
	os_memset(&d_sec_addr, 0, sizeof(d_sec_addr));
	d_sec_addr.nl_family = AF_NETLINK;
	d_sec_addr.nl_pid = 0;
	d_sec_addr.nl_groups = 0;

	/* bind socket */
	if (bind(sock_fd_sec, (struct sockaddr *)&s_sec_addr, sizeof(s_sec_addr)) != 0) {
		MSG_ERR(WLAN_MANAGER_STR, "bind socket error.");
		close(sock_fd_sec);
		return -1;
	}

	/* allocate netlink header */
	nlh_sec = (struct nlmsghdr *)malloc(NLMSG_SPACE(NL_MAX_MSG_SIZE));
	if (!nlh_sec) {
		MSG_ERR(WLAN_MANAGER_STR, "malloc nlmsghdr error.");
		close(sock_fd_sec);
		return -1;
	}
	os_memset(nlh_sec, 0, NLMSG_SPACE(NL_MAX_MSG_SIZE));
	nlh_sec->nlmsg_len = NLMSG_SPACE(NL_MAX_MSG_SIZE);
	nlh_sec->nlmsg_pid = NL_WLAN_MANAGER_PID;
	nlh_sec->nlmsg_flags = 0;

	/* iov structure */
	iov_sec.iov_base = (void *)nlh_sec;
	iov_sec.iov_len = NLMSG_SPACE(NL_MAX_MSG_SIZE);

	/* msghdr */
	os_memset(&msgh_sec, 0, sizeof(msgh_sec));
	msgh_sec.msg_name = (void *)&d_sec_addr;
	msgh_sec.msg_namelen = sizeof(d_sec_addr);
	msgh_sec.msg_iov = &iov_sec;
	msgh_sec.msg_iovlen = 1;
#endif

	/* data structure init */
	for (i = 0; i < BAND_NUM; i++) {
		for (j = 0; j < SSID_NUM; j++) {
			device->priv[i][j].band = i;
			device->priv[i][j].ssid = j;
		}
	}

#ifdef CONFIG_BAND_STEERING
	_band_steering_init(device);
#endif
#ifdef CONFIG_CROSSBAND_REPEATER
	if(crossband_init(device) < 0)
	{
		MSG_ERR(WLAN_MANAGER_STR, "crossband_init error");
		return -1;
	}
#endif

	return 0;
}

static void process_recv_message(struct nl_message *msg, struct com_device *device){
	struct com_priv *priv = NULL;
	struct elm_header *hdr = NULL;
	struct elm_frame_info *frame_info = NULL;
	struct elm_intf *intf = NULL;
	struct elm_intf_info *intf_info = NULL;
	struct elm_sta_info *sta_info = NULL;
	struct elm_buffer *buffer = NULL;
	struct elm_sta_info_ext *sta_info_ext = NULL;

	u32 offset = 0;

#ifdef CONFIG_CROSSBAND_REPEATER
	if (msg->type != NL_CONFIG_UPDATE_TYPE && msg->type != NL_CROSSBAND_DUMP_TYPE) {
#else
	if (msg->type != NL_CONFIG_UPDATE_TYPE) {
#endif
		/* check the 1st element: ELM_INTF_ID */
		if (msg->len < ELM_HEADER_LEN + ELM_INTF_LEN)
			return;

		hdr = (struct elm_header *)(msg->content + offset);
		offset += ELM_HEADER_LEN;

		if (hdr->id != ELM_INTF_ID)
			return;

		intf = (struct elm_intf *)(msg->content + offset);
		offset += hdr->len;
		priv = &(device->priv[intf->band][intf->ssid]);
	}

	/* message type */
	switch(msg->type) {
	case NL_DEL_STA_TYPE:
		while (offset < msg->len) {
			hdr = (struct elm_header *)(msg->content + offset);
			offset += ELM_HEADER_LEN;

			/* element id: should handle wrong length */
			switch(hdr->id) {
			case ELM_STA_INFO_ID:
				sta_info =
					(struct elm_sta_info *)(msg->content + offset);
				break;
			default:
				MSG_WARN(WLAN_MANAGER_STR, "message(%u): unknown element id.",
					msg->type);
				break;
			}
			offset += hdr->len;
		}
		if (sta_info)
			on_del_sta(priv, sta_info);

		break;
	case NL_NEW_STA_TYPE:
		while (offset < msg->len) {
			hdr = (struct elm_header *)(msg->content + offset);
			offset += ELM_HEADER_LEN;

			/* element id: should handle wrong length */
			switch(hdr->id) {
			case ELM_STA_INFO_ID:
				sta_info =
					(struct elm_sta_info *)(msg->content + offset);
				break;
			default:
				MSG_WARN(WLAN_MANAGER_STR, "message(%u): unknown element id.",
					msg->type);
				break;
			}
			offset += hdr->len;
		}
		if (sta_info)
			on_new_sta(device, priv, sta_info);

		break;
	case NL_INTF_RPT_TYPE:
		while (offset < msg->len) {
			hdr = (struct elm_header *)(msg->content + offset);
			offset += ELM_HEADER_LEN;

			/* element id: should handle wrong length */
			switch(hdr->id) {
			case ELM_INTF_INFO_ID:
				intf_info =
					(struct elm_intf_info *)(msg->content + offset);
				break;
			default:
				MSG_WARN(WLAN_MANAGER_STR, "message(%u): unknown element id.",
					msg->type);
				break;
			}
			offset += hdr->len;
		}
		if (intf_info)
			on_intf_rpt(priv, intf, intf_info);
		break;
	case NL_STA_RPT_TYPE:
		while (offset < msg->len) {
			hdr = (struct elm_header *)(msg->content + offset);
			offset += ELM_HEADER_LEN;

			/* element id: should handle wrong length */
			switch(hdr->id) {
			case ELM_STA_INFO_ID:
				sta_info =
					(struct elm_sta_info *)(msg->content + offset);
				break;
			case ELM_STA_INFO_EXT_ID:
				sta_info_ext =
					(struct elm_sta_info_ext *)(msg->content + offset);
				break;
			default:
				MSG_WARN(WLAN_MANAGER_STR, "message(%u): unknown element id.",
					msg->type);
				break;
			}
			offset += hdr->len;
		}
		if (sta_info)
			on_sta_rpt(device, priv, sta_info);
		if (sta_info_ext)
			on_sta_ext_rpt(device, priv, sta_info_ext);
		break;
	case NL_FRAME_RPT_TYPE:
		while (offset < msg->len) {
			hdr = (struct elm_header *)(msg->content + offset);
			offset += ELM_HEADER_LEN;

			/* element id: should handle wrong length */
			switch(hdr->id) {
			case ELM_FRAME_INFO_ID:
				frame_info =
					(struct elm_frame_info *)(msg->content + offset);
				break;
			default:
				MSG_WARN(WLAN_MANAGER_STR, "message(%u): unknown element id.",
					msg->type);
				break;
			}
			offset += hdr->len;
		}
		if (frame_info) {
			on_frame_rpt(priv, frame_info);
#ifdef CONFIG_BAND_STEERING
			_band_steering_on_frame_rpt(priv,
				frame_info->frame_type,
				frame_info->sa,
				frame_info->rssi);
#endif
		}
		break;
	case NL_TIME_TICK_TYPE:
		while (offset < msg->len) {
			hdr = (struct elm_header *)(msg->content + offset);
			offset += ELM_HEADER_LEN;

			/* element id: should handle wrong length */
			switch(hdr->id) {
			default:
				MSG_WARN(WLAN_MANAGER_STR, "message(%u): unknown element id.",
					msg->type);
				break;
			}
			offset += hdr->len;
		}
#ifdef CONFIG_BAND_STEERING
		if (intf) {
			_band_steering_on_time_tick(priv);
			_band_steering_roam_detect(priv);
			_band_steering_roam_start(priv);
		}
#endif
		break;
	case NL_PRIV_INFO_CMD_TYPE:
		while (offset < msg->len) {
			hdr = (struct elm_header *)(msg->content + offset);
			offset += ELM_HEADER_LEN;

			/* element id: should handle wrong length */
			switch(hdr->id) {
			default:
				MSG_WARN(WLAN_MANAGER_STR, "message(%u): unknown element id.",
					msg->type);
				break;
			}
			offset += hdr->len;
		}
		on_priv_info_cmd(priv);
		break;
#ifdef CONFIG_BAND_STEERING
	case NL_B_STEER_CMD_TYPE:
		while (offset < msg->len) {
			hdr = (struct elm_header *)(msg->content + offset);
			offset += ELM_HEADER_LEN;

			/* element id: should handle wrong length */
			switch(hdr->id) {
			default:
				MSG_WARN(WLAN_MANAGER_STR, "message(%u): unknown element id.",
					msg->type);
				break;
			}
			offset += hdr->len;
		}
		_band_steering_on_cmd(priv);
		break;
#endif
	case NL_CONFIG_UPDATE_TYPE:
		while (offset < msg->len) {
			hdr = (struct elm_header *)(msg->content + offset);
			offset += ELM_HEADER_LEN;

			/* element id: should handle wrong length */
			switch(hdr->id) {
			case ELM_BUFFER_ID:
				buffer =
					(struct elm_buffer *)(msg->content + offset);
				break;
			default:
				MSG_WARN(WLAN_MANAGER_STR, "message(%u): unknown element id.",
					msg->type);
				break;
			}
			offset += hdr->len;
		}
		if (buffer) {
			MSG_PRINT(WLAN_MANAGER_STR, "CONFIG_UPDATE: source = %s.", buffer->buf);
#ifdef CONFIG_BAND_STEERING
			_band_steering_on_config_update(buffer->buf);
#endif
#ifdef CONFIG_CROSSBAND_REPEATER
			_crossband_daemon_on_config_update(buffer->buf);
#endif
		}
		break;
#ifdef CONFIG_CROSSBAND_REPEATER
	case NL_CROSSBAND_DUMP_TYPE:
		while (offset < msg->len) {
			hdr = (struct elm_header *)(msg->content + offset);
			offset += ELM_HEADER_LEN;

			/* element id: should handle wrong length */
			switch(hdr->id) {
			default:
				MSG_WARN(WLAN_MANAGER_STR, "message(%u): unknown element id.",
					msg->type);
				break;
			}
			offset += hdr->len;
		}
		_crossband_daemon_on_cmd();
		break;
#endif
	default:
		MSG_WARN(WLAN_MANAGER_STR, "unknown message type.");
		break;
	}

}

int main(int argc, char *argv[])
{
	struct com_device *device = &(global_device);
#ifdef CONFIG_USER_RTK_COMMON_NETLINK
	struct pollfd fdset[2];
#else
	struct pollfd fdset[1];
#endif
	/* parse argument */
	if (parse_argument(argc, argv, device) < 0)
		return 0;

	MSG_PRINT(WLAN_MANAGER_STR, "%s.", WLAN_MANAGER_VER);

	/* init */
	if(daemon_init(device) < 0)
		return -1;

	send_daemon_on_msg();

	sleep(1);

	/* recv message from kernel */
	while (1) {
		s32 status;
		s32 nfds = 0;
		struct nl_message *msg = NULL;

#ifdef CONFIG_CROSSBAND_REPEATER
		do_crossband();
#endif

		memset((void *)fdset, 0, sizeof(fdset));
		fdset[0].fd     = sock_fd;
		fdset[0].events = POLLIN;

#ifdef CONFIG_USER_RTK_COMMON_NETLINK
		fdset[1].fd     = sock_fd_sec;
		fdset[1].events = POLLIN;

		nfds = 2;
#else
		nfds = 1;
#endif
		if (0 > poll(fdset, nfds, -1)) {
			MSG_ERR(WLAN_MANAGER_STR, "poll fail with errno=%d (%s)", errno, strerror(errno));
			break;
		}

		if (fdset[0].revents & POLLIN) {
			os_memset(nlh, 0, NLMSG_SPACE(NL_MAX_MSG_SIZE));
			status = recvmsg(fdset[0].fd, &msgh, 0);
			if(-1 == status) {
				MSG_ERR(WLAN_MANAGER_STR, "recv error");
				continue;
			}
			msg = (struct nl_message *)NLMSG_DATA(nlh);
			process_recv_message(msg, device);
		}

#ifdef CONFIG_USER_RTK_COMMON_NETLINK
		if (fdset[1].revents & POLLIN) {
			os_memset(nlh_sec, 0, NLMSG_SPACE(NL_MAX_MSG_SIZE));
			status = recvmsg(fdset[1].fd, &msgh_sec, 0);
			if(-1 == status) {
				MSG_ERR(WLAN_MANAGER_STR, "recv error from second drv");
				continue;
			}
			msg = (struct nl_message *)NLMSG_DATA(nlh_sec);
			process_recv_message(msg, device);
		}
#endif
	}

	return 0;
}

