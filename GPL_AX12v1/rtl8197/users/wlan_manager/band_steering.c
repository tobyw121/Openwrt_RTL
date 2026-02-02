#include "wlan_manager.h"
#include "band_steering.h"

#ifdef CONFIG_BAND_STEERING

static struct b_steer_context ctx = {0};

/* ------------------------------------------- */
/* ---------- use wlan_manager APIs ---------- */
/* ------------------------------------------- */
static void send_b_steer_cmd_msg(u8 band, u8 ssid)
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
	msg.type = NL_B_STEER_CMD_TYPE;
	msg.len = msg_len;

	/* length += (type + len) */
	msg_len = msg.len + 8;
	_wlan_manager_send_daemon((void *)&msg, msg_len, NL_WLAN_MANAGER_PID);

	return;
}

static void send_b_steer_block_add_msg(u8 *mac, u8 band, u8 ssid)
{
	u32 msg_len = 0;
	struct nl_message msg = {0};
	struct elm_header hdr = {0};
	struct elm_intf intf = {0};
	struct elm_sta_info sta_info = {0};

	/* element header */
	hdr.id = ELM_INTF_ID;
	hdr.len = ELM_INTF_LEN;
	_wlan_manager_set_msg(&msg, &msg_len, (void *)&hdr, ELM_HEADER_LEN);

	/* element: ELM_INTF_ID */
	intf.band = band;
	intf.ssid = ssid;
	_wlan_manager_set_msg(&msg, &msg_len, (void *)&intf, ELM_INTF_LEN);

	/* element header */
	hdr.id = ELM_STA_INFO_ID;
	hdr.len = ELM_STA_INFO_LEN;
	_wlan_manager_set_msg(&msg, &msg_len, (void *)&hdr, ELM_HEADER_LEN);

	/* element: ELM_STA_INFO_ID */
	os_memcpy(sta_info.mac, mac, 6);
	_wlan_manager_set_msg(&msg, &msg_len, (void *)&sta_info, ELM_STA_INFO_LEN);

	/* finish message */
	msg.type = NL_B_STEER_BLOCK_ADD_TYPE;
	msg.len = msg_len;

	/* length += (type + len) */
	msg_len += 8;
	_wlan_manager_send_drv((void *)&msg, msg_len);
#ifdef CONFIG_USER_RTK_COMMON_NETLINK
	_wlan_manager_send_sec_drv((void *)&msg, msg_len);
#endif
	return;
}

static void send_b_steer_block_del_msg(u8 *mac, u8 band, u8 ssid)
{
	u32 msg_len = 0;
	struct nl_message msg = {0};
	struct elm_header hdr = {0};
	struct elm_intf intf = {0};
	struct elm_sta_info sta_info = {0};

	/* element header */
	hdr.id = ELM_INTF_ID;
	hdr.len = ELM_INTF_LEN;
	_wlan_manager_set_msg(&msg, &msg_len, (void *)&hdr, ELM_HEADER_LEN);

	/* element: ELM_INTF_ID */
	intf.band = band;
	intf.ssid = ssid;
	_wlan_manager_set_msg(&msg, &msg_len, (void *)&intf, ELM_INTF_LEN);

	/* element header */
	hdr.id = ELM_STA_INFO_ID;
	hdr.len = ELM_STA_INFO_LEN;
	_wlan_manager_set_msg(&msg, &msg_len, (void *)&hdr, ELM_HEADER_LEN);

	/* element: ELM_STA_INFO_ID */
	os_memcpy(sta_info.mac, mac, 6);
	_wlan_manager_set_msg(&msg, &msg_len, (void *)&sta_info, ELM_STA_INFO_LEN);

	/* finish message */
	msg.type = NL_B_STEER_BLOCK_DEL_TYPE;
	msg.len = msg_len;

	/* length += (type + len) */
	msg_len += 8;
	_wlan_manager_send_drv((void *)&msg, msg_len);
#ifdef CONFIG_USER_RTK_COMMON_NETLINK
	_wlan_manager_send_sec_drv((void *)&msg, msg_len);
#endif
	return;
}

static void send_b_steer_roam_msg(
	u8 *sta_mac, u8 band, u8 ssid, u8 *bss_mac, u16 bss_ch,
	u16 bss_info, u8 bss_reg_class, u8 bss_phy_type,
	u8 method, s8 *intf_name)
{
	u32 msg_len = 0;
	struct nl_message msg = {0};
	struct elm_header hdr = {0};
	struct elm_intf intf = {0};
	struct elm_roam_info roam_info = {0};

	/* element header */
	hdr.id = ELM_INTF_ID;
	hdr.len = ELM_INTF_LEN;
	_wlan_manager_set_msg(&msg, &msg_len, (void *)&hdr, ELM_HEADER_LEN);

	/* element: ELM_INTF_ID */
	intf.band = band;
	intf.ssid = ssid;
	_wlan_manager_set_msg(&msg, &msg_len, (void *)&intf, ELM_INTF_LEN);

	/* element header */
	hdr.id = ELM_ROAM_INFO_ID;
	hdr.len = ELM_ROAM_INFO_LEN;
	_wlan_manager_set_msg(&msg, &msg_len, (void *)&hdr, ELM_HEADER_LEN);

	/* element: ELM_ROAM_INFO_ID */
	os_memcpy(roam_info.sta_mac, sta_mac, 6);
	os_memcpy(roam_info.bss_mac, bss_mac, 6);
	roam_info.bss_ch = bss_ch;
	roam_info.method = method;
	_wlan_manager_set_msg(&msg, &msg_len, (void *)&roam_info, ELM_ROAM_INFO_LEN);

	/* finish message */
	msg.type = NL_B_STEER_ROAM_TYPE;
	msg.len = msg_len;

	/* length += (type + len) */
	msg_len += 8;
	_wlan_manager_send_drv((void *)&msg, msg_len);
#ifdef CONFIG_USER_RTK_COMMON_NETLINK
	_wlan_manager_send_sec_drv((void *)&msg, msg_len);
#endif

	if (method == 0)
		_wlan_manager_hostapd_cli_bss_tm_req(
			intf_name, sta_mac, bss_mac, bss_ch,
			bss_info, bss_reg_class, bss_phy_type,
			CTX.bss_tm_req_disassoc_imminent, CTX.bss_tm_req_disassoc_timer);
	else
		_wlan_manager_hostapd_cli_deauth(intf_name, sta_mac);

	return;
}

/* ---------------------------------------- */
/* ---------- band steering core ---------- */
/* ---------------------------------------- */
static struct b_steer_ignore_entry *non_prefer_band_ignore_entry_lookup(
	u8 *mac, u8 ssid)
{
	u8 i;
	struct b_steer_ignore_entry *ent = NULL;

	for (i = 0; i < B_STEER_ENTRY_NUM; i++) {
		ent = &(CTX.np_band_ignore_list[ssid][i]);
		if (ent->used && !os_memcmp(mac, ent->mac, 6))
			return ent;
	}

	return NULL;
}

static void non_prefer_band_ignore_entry_add(u8 *mac, u8 ssid)
{
	u8 i;
	struct b_steer_ignore_entry *ent = NULL;

	ent = non_prefer_band_ignore_entry_lookup(mac, ssid);

	/* already exist */
	if (ent)
		goto func_return;

	/* find an empty entry */
	for (i = 0; i < B_STEER_ENTRY_NUM; i++) {
		if (!CTX.np_band_ignore_list[ssid][i].used) {
			ent = &(CTX.np_band_ignore_list[ssid][i]);
			break;
		}
	}

	/* add the entry */
	if (ent) {
		ent->used = 1;
		os_memcpy(ent->mac, mac, 6);
		ent->entry_exp = B_STEER_ENTRY_IGNORE_PERIOD;
	}

func_return:
	return;
}

static struct b_steer_non_prefer_band_entry *non_prefer_band_entry_lookup(
	u8 *mac, u8 ssid)
{
	u8 i;
	struct b_steer_non_prefer_band_entry *ent = NULL;

	for (i = 0; i < B_STEER_ENTRY_NUM; i++) {
		ent = &(CTX.np_band_list[ssid][i]);
		if (ent->used && !os_memcmp(mac, ent->mac, 6))
			return ent;
	}

	return NULL;
}

static void non_prefer_band_entry_add(u8 *mac, u8 ssid)
{
	u8 i;
	struct b_steer_non_prefer_band_entry *ent = NULL;

	/* ignore */
	if (non_prefer_band_ignore_entry_lookup(mac, ssid))
		goto func_return;

	ent = non_prefer_band_entry_lookup(mac, ssid);
	/* already exist */
	if (ent) {
		send_b_steer_block_add_msg(mac, CTX.non_prefer_band, ssid);
		goto func_return;
	}

	/* find an empty entry */
	for (i = 0; i < B_STEER_ENTRY_NUM; i++) {
		if (!CTX.np_band_list[ssid][i].used) {
			ent = &(CTX.np_band_list[ssid][i]);
			break;
		}
	}

	/* add the entry */
	if (ent) {
		ent->used = 1;
		os_memcpy(ent->mac, mac, 6);
		ent->entry_exp_phase1 = CTX.entry_exp_phase1;
		ent->assoc_rty_lmt = CTX.assoc_rty_lmt;
		ent->entry_exp_phase2 = 0;
		send_b_steer_block_add_msg(mac, CTX.non_prefer_band, ssid);
	}

func_return:
	return;
}

static void non_prefer_band_entry_del(u8 *mac, u8 ssid)
{
	struct b_steer_non_prefer_band_entry *ent = NULL;

	ent = non_prefer_band_entry_lookup(mac, ssid);
	if (ent) {
		ent->used = 0;
		send_b_steer_block_del_msg(mac, CTX.non_prefer_band, ssid);
	}

	return;
}

static void non_prefer_band_expire(u8 ssid)
{
	u8 i;
	struct b_steer_ignore_entry *ent1 = NULL;
	struct b_steer_non_prefer_band_entry *ent2 = NULL;

	/* non prefer band ignore entry expire */
	for (i = 0; i < B_STEER_ENTRY_NUM; i++) {
		ent1 = &(CTX.np_band_ignore_list[ssid][i]);
		if (!ent1->used)
			continue;

		if (ent1->entry_exp) {
			ent1->entry_exp--;
			if (ent1->entry_exp == 0)
				ent1->used = 0;
		}
	}

	/* non prefer band entry expire */
	for (i = 0; i < B_STEER_ENTRY_NUM; i++) {
		ent2 = &(CTX.np_band_list[ssid][i]);
		if (!ent2->used)
			continue;

		if (ent2->entry_exp_phase2) {
			ent2->entry_exp_phase2--;
			if (ent2->entry_exp_phase2 == 0) {
				ent2->used = 0;
				non_prefer_band_ignore_entry_add(ent2->mac, ssid);
				send_b_steer_block_del_msg(
					ent2->mac, CTX.non_prefer_band, ssid);
			}
		}
		else if (ent2->entry_exp_phase1) {
			ent2->entry_exp_phase1--;
			if (ent2->entry_exp_phase1 == 0)
				ent2->entry_exp_phase2 = CTX.entry_exp_phase2;
		}
	}

	return;
}

static void non_prefer_band_on_assocreq(u8 *mac, u8 ssid)
{
	struct b_steer_non_prefer_band_entry *ent = NULL;

	ent = non_prefer_band_entry_lookup(mac, ssid);
	if (ent) {
		if (ent->assoc_rty_lmt) {
			ent->assoc_rty_lmt--;
			if (ent->assoc_rty_lmt == 0)
				ent->entry_exp_phase2 = CTX.entry_exp_phase2;
		}
	}

	return;
}

static void non_prefer_band_roam_detect(struct com_priv *priv)
{
	u8  i;
	u32 sta_tp;
	struct com_priv *grp_priv = priv->grp_priv;
	struct com_sta *sta = NULL;

	for (i = 0; i < MAX_STA_NUM; i++) {
		sta = &(priv->sta_list[i]);
		if (!sta->used || !sta->is_dual_band)
			continue;

		sta_tp = sta->tx_tp + sta->rx_tp;

		if ((CTX.roam_sta_tp_th && sta_tp > CTX.roam_sta_tp_th)
			|| (CTX.roam_ch_clm_th && grp_priv->ch_clm > CTX.roam_ch_clm_th))
			sta->b_steer_roam_detect = 0;
		else if (sta->rssi > CTX.prefer_band_rssi_high)
			sta->b_steer_roam_detect++;
		else if (sta->b_steer_roam_detect > 1)
			sta->b_steer_roam_detect -= 2;
		else
			sta->b_steer_roam_detect = 0;

		if (sta->b_steer_roam_detect > CTX.roam_detect_th)
			sta->b_steer_roam_detect = CTX.roam_detect_th;
	}

	return;
}

static struct b_steer_prefer_band_entry *prefer_band_entry_lookup(
	u8 *mac, u8 ssid)
{
	u8 i;
	struct b_steer_prefer_band_entry *ent = NULL;

	for (i = 0; i < B_STEER_ENTRY_NUM; i++) {
		ent = &(CTX.p_band_list[ssid][i]);
		if (ent->used && !os_memcmp(mac, ent->mac, 6))
			return ent;
	}

	return NULL;
}

static void prefer_band_entry_add(u8 *mac, u8 rssi, u8 ssid)
{
	u8  i;
	u32 tmp_i = 0;
	u32 tmp_aging = 0;
	struct b_steer_prefer_band_entry *ent = NULL;

	ent = prefer_band_entry_lookup(mac, ssid);
	/* already exist */
	if (ent) {
		ent->rssi = rssi;
		ent->aging = 0;
		goto func_return;
	}

	/* find an empty entry */
	for (i = 0; i < B_STEER_ENTRY_NUM; i++) {
		if (!CTX.p_band_list[ssid][i].used) {
			ent = &(CTX.p_band_list[ssid][i]);
			break;
		}
	}

	/* no empty, find the oldest one */
	if (ent == NULL) {
		for (i = 0; i < B_STEER_ENTRY_NUM; i++) {
			if (tmp_aging < CTX.p_band_list[ssid][i].aging) {
				tmp_aging = CTX.p_band_list[ssid][i].aging;
				tmp_i = i;
			}
		}
		ent = &(CTX.p_band_list[ssid][tmp_i]);
	}

	/* add the entry */
	ent->used = 1;
	os_memcpy(ent->mac, mac, 6);
	ent->rssi = rssi;
	ent->aging = 0;

func_return:
	return;
}

static void prefer_band_entry_del(u8 *mac, u8 ssid)
{
	struct b_steer_prefer_band_entry *ent = NULL;

	ent = prefer_band_entry_lookup(mac, ssid);
	if (ent)
		ent->used = 0;

	return;
}

static void prefer_band_expire(u8 ssid)
{
	u8 i;
	struct b_steer_prefer_band_entry *ent = NULL;

	for (i = 0; i < B_STEER_ENTRY_NUM; i++) {
		ent = &(CTX.p_band_list[ssid][i]);
		if (!ent->used)
			continue;

		ent->aging++;
		if (ent->aging > B_STEER_PREFER_BAND_ENTRY_TIMEOUT)
			ent->used = 0;
	}

	return;
}

static void prefer_band_roam_detect(struct com_priv *priv)
{
	u8 i;
	u32 sta_tp;
	struct com_sta *sta = NULL;

	for (i = 0; i < MAX_STA_NUM; i++) {
		sta = &(priv->sta_list[i]);
		if (!sta->used || !sta->is_dual_band)
			continue;

		sta_tp = sta->tx_tp + sta->rx_tp;

		if (CTX.roam_sta_tp_th && sta_tp > CTX.roam_sta_tp_th)
			sta->b_steer_roam_detect = 0;
		else if (sta->rssi < CTX.prefer_band_rssi_low)
			sta->b_steer_roam_detect++;
		else if (sta->b_steer_roam_detect > 1)
			sta->b_steer_roam_detect -= 2;
		else
			sta->b_steer_roam_detect = 0;

		if (sta->b_steer_roam_detect > CTX.roam_detect_th)
			sta->b_steer_roam_detect = CTX.roam_detect_th;
	}

	return;
}

static void band_steering_on_probereq(u8 *mac, u8 rssi, u8 band, u8 ssid)
{
	u8 grp_ssid = CTX.prefer_band_grp_ssid[ssid];

	/* prefer band */
	if (band == CTX.prefer_band) {
		prefer_band_entry_add(mac, rssi, ssid);

		if (rssi < CTX.prefer_band_rssi_low)
			non_prefer_band_entry_del(mac, grp_ssid);
		else if (rssi > CTX.prefer_band_rssi_high)
			non_prefer_band_entry_add(mac, grp_ssid);
	}

	return;
}

static void band_steering_on_assocreq(u8 *mac, u8 rssi, u8 band, u8 ssid)
{
	u8 grp_ssid = CTX.prefer_band_grp_ssid[ssid];

	/* prefer band */
	if (band == CTX.prefer_band) {
		prefer_band_entry_add(mac, rssi, ssid);

		if (rssi < CTX.prefer_band_rssi_low)
			non_prefer_band_entry_del(mac, grp_ssid);
		else if (rssi > CTX.prefer_band_rssi_high)
			non_prefer_band_entry_add(mac, grp_ssid);
	}
	/* non prefer band */
	else if (band == CTX.non_prefer_band) {
		non_prefer_band_on_assocreq(mac, ssid);
	}

	return;
}

static void band_steering_on_auth(u8 *mac, u8 rssi, u8 band, u8 ssid)
{
	u8 grp_ssid = CTX.prefer_band_grp_ssid[ssid];

	/* prefer band */
	if (band == CTX.prefer_band) {
		prefer_band_entry_add(mac, rssi, ssid);

		if (rssi < CTX.prefer_band_rssi_low)
			non_prefer_band_entry_del(mac, grp_ssid);
		else if (rssi > CTX.prefer_band_rssi_high)
			non_prefer_band_entry_add(mac, grp_ssid);
	}

	return;
}

static void band_steering_roam_start(struct com_priv *priv)
{
	u8 i;
	u8 method;
	u8 roam_num = 0;
	struct com_priv *grp_priv = priv->grp_priv;
	struct com_sta *sta = NULL;

	for (i = 0; i < MAX_STA_NUM; i++) {
		sta = &(priv->sta_list[i]);
		if (!sta->used)
			continue;

		if (sta->b_steer_roam_detect >= CTX.roam_detect_th) {
			method = (sta->b_steer_roam_cnt < CTX.bss_tm_req_th) ? 0 : 1;

			send_b_steer_roam_msg(
				sta->mac, priv->band, priv->ssid,
				grp_priv->mac, grp_priv->ch,
				grp_priv->self_nb_rpt.bss_info,
				grp_priv->self_nb_rpt.reg_class,
				grp_priv->self_nb_rpt.phy_type,
				method, priv->name);

			sta->b_steer_roam_cnt++;
			sta->b_steer_roam_detect = CTX.roam_detect_th / 2;

			roam_num++;
			if (roam_num == B_STEER_ROAM_STA_ONCE_NUMBER)
				break;
		}
	}

	return;
}

static void band_steering_config_fill(const s8 *buf, s8 *pos)
{
	/* TBD: should check the data type and data size */

	if (!os_strcmp(buf, "band_steering_enable")) {
		if (!os_strcmp(pos, "0")) {
			CTX.band_steering_enable = 0;
		}
		else if (!os_strcmp(pos, "1")) {
			CTX.band_steering_enable = 1;
		}
		else {
			MSG_WARN(B_STEER_STR, "invalid config: %s.", pos);
		}
	}
	else if (!os_strcmp(buf, "prefer_band")) {
		if (!os_strcmp(pos, "5G")) {
			CTX.prefer_band = BAND_ON_5G;
			CTX.non_prefer_band = BAND_ON_24G;
		}
		else if (!os_strcmp(pos, "24G")) {
			CTX.prefer_band = BAND_ON_24G;
			CTX.non_prefer_band = BAND_ON_5G;
		}
		else {
			MSG_WARN(B_STEER_STR, "invalid config: %s.", pos);
		}
	}
	else if (!os_strcmp(buf, "bss_tm_req_th")) {
		CTX.bss_tm_req_th = atoi(pos);
	}
	else if (!os_strcmp(buf, "bss_tm_req_disassoc_imminent")) {
		CTX.bss_tm_req_disassoc_imminent = atoi(pos);
	}
	else if (!os_strcmp(buf, "bss_tm_req_disassoc_timer")) {
		CTX.bss_tm_req_disassoc_timer = atoi(pos);
	}
	else if (!os_strcmp(buf, "roam_sta_tp_th")) {
		CTX.roam_sta_tp_th = atoi(pos);
	}
	else if (!os_strcmp(buf, "roam_detect_th")) {
		CTX.roam_detect_th = atoi(pos);
	}
	else if (!os_strcmp(buf, "roam_ch_clm_th")) {
		CTX.roam_ch_clm_th = atoi(pos);
	}
	else if (!os_strcmp(buf, "prefer_band_rssi_high")) {
		CTX.prefer_band_rssi_high = atoi(pos);
	}
	else if (!os_strcmp(buf, "prefer_band_rssi_low")) {
		CTX.prefer_band_rssi_low = atoi(pos);
	}
	else if (!os_strcmp(buf, "entry_exp_phase1")) {
		CTX.entry_exp_phase1 = atoi(pos);
	}
	else if (!os_strcmp(buf, "assoc_rty_lmt")) {
		CTX.assoc_rty_lmt = atoi(pos);
	}
	else if (!os_strcmp(buf, "entry_exp_phase2")) {
		CTX.entry_exp_phase2 = atoi(pos);
	}
	else if (!os_strcmp(buf, "prefer_band_grp_ssid0")) {
		CTX.prefer_band_grp_ssid[0] = atoi(pos);
	}
	else if (!os_strcmp(buf, "non_prefer_band_grp_ssid0")) {
		CTX.non_prefer_band_grp_ssid[0] = atoi(pos);
	}
#if (SSID_NUM > 1)
	else if (!os_strcmp(buf, "prefer_band_grp_ssid1")) {
		CTX.prefer_band_grp_ssid[1] = atoi(pos);
	}
	else if (!os_strcmp(buf, "non_prefer_band_grp_ssid1")) {
		CTX.non_prefer_band_grp_ssid[1] = atoi(pos);
	}
#endif
#if (SSID_NUM > 2)
	else if (!os_strcmp(buf, "prefer_band_grp_ssid2")) {
		CTX.prefer_band_grp_ssid[2] = atoi(pos);
	}
	else if (!os_strcmp(buf, "non_prefer_band_grp_ssid2")) {
		CTX.non_prefer_band_grp_ssid[2] = atoi(pos);
	}
#endif
#if (SSID_NUM > 3)
	else if (!os_strcmp(buf, "prefer_band_grp_ssid3")) {
		CTX.prefer_band_grp_ssid[3] = atoi(pos);
	}
	else if (!os_strcmp(buf, "non_prefer_band_grp_ssid3")) {
		CTX.non_prefer_band_grp_ssid[3] = atoi(pos);
	}
#endif
#if (SSID_NUM > 4)
	else if (!os_strcmp(buf, "prefer_band_grp_ssid4")) {
		CTX.prefer_band_grp_ssid[4] = atoi(pos);
	}
	else if (!os_strcmp(buf, "non_prefer_band_grp_ssid4")) {
		CTX.non_prefer_band_grp_ssid[4] = atoi(pos);
	}
#endif
	else {
		MSG_WARN(B_STEER_STR, "unknown config: %s.", buf);
	}

	return;
}

static void band_steering_config_read(const s8 *fname)
{
	FILE *fp;
	s8 buf[4096];
	s8 *pos;

	fp = fopen(fname, "r");
	if (fp == NULL)
		return;

	while (fgets(buf, sizeof(buf), fp)) {
		if (buf[0] == '#')
			continue;

		pos = buf;
		while (*pos != '\0') {
			if (*pos == '\n') {
				*pos = '\0';
				break;
			}
			pos++;
		}
		if (buf[0] == '\0')
			continue;

		if (!os_strcmp(buf, "band_steering_end"))
			break;

		pos = os_strchr(buf, '=');
		if (pos == NULL)
			continue;

		*pos = '\0';
		pos++;
		band_steering_config_fill(buf, pos);
	}

	fclose(fp);

	return;
}

/* ------------------------------------------- */
/* ---------- APIs for wlan_manager ---------- */
/* ------------------------------------------- */
void _band_steering_parse_arg(u8 *argn, s32 argc, s8 *argv[])
{
	u8 band = 255;
	u8 ssid = 255;

	if (*argn + 2 >= argc) {
		MSG_WARN(B_STEER_STR, "wrong argument.");
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

	send_b_steer_cmd_msg(band, ssid);

	*argn = *argn + 2;

	return;
}

void _band_steering_on_frame_rpt(
	struct com_priv *priv, u16 frame_type, u8 *mac, u8 rssi)
{
	u8 band = priv->band;
	u8 ssid = priv->ssid;

	if (!priv->band_steering_enable)
		return;

	if (frame_type == WIFI_PROBEREQ)
		band_steering_on_probereq(mac, rssi, band, ssid);
	else if (frame_type == WIFI_ASSOCREQ)
		band_steering_on_assocreq(mac, rssi, band, ssid);
	else if (frame_type == WIFI_AUTH)
		band_steering_on_auth(mac, rssi, band, ssid);

	return;
}

void _band_steering_on_time_tick(struct com_priv *priv)
{
	u8 band = priv->band;
	u8 ssid = priv->ssid;

	if (!priv->band_steering_enable)
		return;

	/* prefer band */
	if (band == CTX.prefer_band)
		prefer_band_expire(ssid);
	/* non prefer band */
	else if (band == CTX.non_prefer_band)
		non_prefer_band_expire(ssid);

	return;
}

void _band_steering_roam_detect(struct com_priv *priv)
{
	u8 band = priv->band;
	u8 ssid = priv->ssid;

	if (!priv->band_steering_enable)
		return;

	/* prefer band */
	if (band == CTX.prefer_band)
		prefer_band_roam_detect(priv);
	/* non prefer band */
	else if (band == CTX.non_prefer_band)
		non_prefer_band_roam_detect(priv);

	return;
}

void _band_steering_roam_start(struct com_priv *priv)
{
	if (!priv->band_steering_enable)
		return;

	band_steering_roam_start(priv);

	return;
}

void _band_steering_on_cmd(struct com_priv *priv)
{
	u8 i;
	u8 band = priv->band;
	u8 ssid = priv->ssid;
	s8 sys_cmd[64] = {0};
	FILE *fp;

	if (band >= BAND_NUM || ssid >= SSID_NUM) {
		MSG_WARN(B_STEER_STR, "wrong argument.");
		return;
	}

	fp = fopen(B_STEER_OUTPUT, "w");
	if (fp == NULL) {
		MSG_WARN(B_STEER_STR, "can't open [%s].", B_STEER_OUTPUT);
		return;
	}

	fprintf(fp, "[BAND_STEERING] Common Parameters.\n");
	fprintf(fp, "prefer_band: %s\n", BAND_NAME(CTX.prefer_band));
	fprintf(fp, "non_prefer_band: %s\n", BAND_NAME(CTX.non_prefer_band));
	fprintf(fp, "bss_tm_req_th: %u\n", CTX.bss_tm_req_th);
	fprintf(fp, "bss_tm_req_disassoc_imminent: %u\n", CTX.bss_tm_req_disassoc_imminent);
	fprintf(fp, "bss_tm_req_disassoc_timer: %u\n", CTX.bss_tm_req_disassoc_timer);
	fprintf(fp, "roam_detect_th: %u\n", CTX.roam_detect_th);
	fprintf(fp, "roam_sta_tp_th(kbits): %u\n", CTX.roam_sta_tp_th);
	fprintf(fp, "roam_ch_clm_th(%%): %u\n", CTX.roam_ch_clm_th);
	fprintf(fp, "================================\n");

	/* show prefer band list */
	if (band == CTX.prefer_band) {
		struct b_steer_prefer_band_entry *ent;

		fprintf(fp, "[BAND_STEERING] prefer_band.\n");
		fprintf(fp, "band: %s\n", BAND_NAME(band));
		fprintf(fp, "ssid: %u\n", ssid);
		fprintf(fp, "grp_ssid: %u\n", CTX.prefer_band_grp_ssid[ssid]);
		fprintf(fp, "prefer_band_rssi_high: %u\n", CTX.prefer_band_rssi_high);
		fprintf(fp, "prefer_band_rssi_low: %u\n", CTX.prefer_band_rssi_low);
		fprintf(fp, "================================\n");
		for (i = 0; i < B_STEER_ENTRY_NUM; i++) {
			ent = &(CTX.p_band_list[ssid][i]);
			if (ent->used) {
				fprintf(fp, "mac: "MAC_FMT"\n", MAC_ARG(ent->mac));
				fprintf(fp, "rssi: %u\n", ent->rssi);
				fprintf(fp, "aging: %u\n", ent->aging);
				fprintf(fp, "------------------------------\n");
			}
		}
	}
	/* show non prefer band list */
	else if (band == CTX.non_prefer_band) {
		struct b_steer_non_prefer_band_entry *ent;

		fprintf(fp, "[BAND_STEERING] non_prefer_band.\n");
		fprintf(fp, "band: %s\n", BAND_NAME(band));
		fprintf(fp, "ssid: %u\n", ssid);
		fprintf(fp, "grp_ssid: %u\n", CTX.non_prefer_band_grp_ssid[ssid]);
		fprintf(fp, "entry_exp_phase1: %u\n", CTX.entry_exp_phase1);
		fprintf(fp, "assoc_rty_lmt: %u\n", CTX.assoc_rty_lmt);
		fprintf(fp, "entry_exp_phase2: %u\n", CTX.entry_exp_phase2);
		fprintf(fp, "================================\n");
		for (i = 0; i < B_STEER_ENTRY_NUM; i++) {
			ent = &(CTX.np_band_list[ssid][i]);
			if (ent->used) {
				fprintf(fp, "mac: "MAC_FMT"\n", MAC_ARG(ent->mac));
				fprintf(fp, "entry_exp_phase1: %u\n", ent->entry_exp_phase1);
				fprintf(fp, "assoc_rty_lmt: %u\n", ent->assoc_rty_lmt);
				fprintf(fp, "entry_exp_phase2: %u\n", ent->entry_exp_phase2);
				fprintf(fp, "------------------------------\n");
			}
		}
	}
	else {
		MSG_WARN(B_STEER_STR, "wrong argument.");
	}

	fclose(fp);

	sprintf(sys_cmd, "cat %s", B_STEER_OUTPUT);
	system(sys_cmd);

	return;
}

void _band_steering_on_config_update(s8 *config)
{
	band_steering_config_read(config);

	return;
}

void _band_steering_init(struct com_device *device)
{
	u8 i;
	u8 grp_ssid;
	struct com_priv *priv;
	struct com_priv *grp_priv;

	/* common parameters */
	CTX.prefer_band = BAND_ON_5G;
	CTX.non_prefer_band = BAND_ON_24G;
	/* bss_tm_req parameters */
	CTX.bss_tm_req_th = 2;
	CTX.bss_tm_req_disassoc_imminent = 1;
	CTX.bss_tm_req_disassoc_timer = 100;

	/* band roaming parameters */
	CTX.roam_detect_th = 10;
	CTX.roam_sta_tp_th = 0;
	/* CLM: non prefer band --> prefer band */
	CTX.roam_ch_clm_th = 0;

	/* rssi parameters */
	CTX.prefer_band_rssi_high = 40;
	CTX.prefer_band_rssi_low = 20;

	/* non prefer band parameter */
	CTX.entry_exp_phase1 = 60;
	CTX.assoc_rty_lmt = 5;
	CTX.entry_exp_phase2 = 5;

	/* data structure */
	os_memset(CTX.p_band_list, 0,	sizeof(CTX.p_band_list));
	os_memset(CTX.np_band_list, 0, sizeof(CTX.np_band_list));
	os_memset(CTX.np_band_ignore_list, 0, sizeof(CTX.np_band_ignore_list));

	/* group */
	for (i = 0; i < SSID_NUM; i++) {
		CTX.prefer_band_grp_ssid[i] = 0xff;
		CTX.non_prefer_band_grp_ssid[i] = 0xff;
	}
	CTX.prefer_band_grp_ssid[0] = 0;
	CTX.non_prefer_band_grp_ssid[0] = 0;

	band_steering_config_read(device->config_fname);

	if (!CTX.band_steering_enable) {
		MSG_INFO(B_STEER_STR, "band steering not enabled.");
		return;
	}

	/* prefer band group non prfer band */
	for (i = 0; i < SSID_NUM; i++) {
		grp_ssid = CTX.prefer_band_grp_ssid[i];
		if (grp_ssid >= SSID_NUM)
			continue;

		/* point each others */
		priv = &(device->priv[CTX.prefer_band][i]);
		grp_priv = &(device->priv[CTX.non_prefer_band][grp_ssid]);
		priv->grp_priv = grp_priv;
		grp_priv->grp_priv = priv;
	}

	/* non prefer band group check */
	for (i = 0; i < SSID_NUM; i++) {
		grp_ssid = CTX.non_prefer_band_grp_ssid[i];
		if (grp_ssid >= SSID_NUM)
			continue;

		priv = &(device->priv[CTX.non_prefer_band][i]);
		grp_priv = priv->grp_priv;
		if (grp_priv && grp_priv->ssid == grp_ssid) {
			priv->band_steering_enable = 1;
			grp_priv->band_steering_enable = 1;
		}
	}

	return;
}

void _band_steering_deinit(void)
{
	return;
}

#endif

