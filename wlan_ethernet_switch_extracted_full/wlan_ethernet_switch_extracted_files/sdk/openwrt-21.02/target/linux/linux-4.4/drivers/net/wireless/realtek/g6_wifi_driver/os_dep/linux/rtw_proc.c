/******************************************************************************
 *
 * Copyright(c) 2007 - 2019 Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 *****************************************************************************/

#ifndef PLATFORM_ECOS
#include <linux/ctype.h>	/* tolower() */
#include <drv_types.h>
#include "rtw_proc.h"
#include <rtw_btc.h>
#include "_hal_rate.h"
#include "../phl/phl_headers.h"
#endif /* PLATFORM_ECOS */

#define MAX_PRC_CMD		10
#define PROC_CMD_LEN	20

#ifdef CONFIG_PROC_DEBUG
#ifndef PLATFORM_ECOS

static struct proc_dir_entry *rtw_proc = NULL;

inline struct proc_dir_entry *get_rtw_drv_proc(void)
{
	return rtw_proc;
}
EXPORT_SYMBOL(get_rtw_drv_proc);

#define RTW_PROC_NAME DRV_NAME

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 9, 0))
#define file_inode(file) ((file)->f_dentry->d_inode)
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0))
#define PDE_DATA(inode) PDE((inode))->data
#define proc_get_parent_data(inode) PDE((inode))->parent->data
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24))
#define get_proc_net proc_net
#else
#define get_proc_net init_net.proc_net
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0))
int single_open_size(struct file *file, int (*show)(struct seq_file *, void *),
		void *data, size_t size)
{
	char *buf = kmalloc(size, GFP_KERNEL);
	int ret;
	if (!buf)
		return -ENOMEM;
	ret = single_open(file, show, data);
	if (ret) {
		kfree(buf);
		return ret;
	}
	((struct seq_file *)file->private_data)->buf = buf;
	((struct seq_file *)file->private_data)->size = size;
	return 0;
}
#endif

inline struct proc_dir_entry *rtw_proc_create_dir(const char *name, struct proc_dir_entry *parent, void *data)
{
	struct proc_dir_entry *entry;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
	entry = proc_mkdir_data(name, S_IRUGO | S_IXUGO, parent, data);
#else
	/* entry = proc_mkdir_mode(name, S_IRUGO|S_IXUGO, parent); */
	entry = proc_mkdir(name, parent);
	if (entry)
		entry->data = data;
#endif

	return entry;
}

inline struct proc_dir_entry *rtw_proc_create_entry(const char *name, struct proc_dir_entry *parent,
	const struct rtw_proc_ops *fops, void * data)
{
	struct proc_dir_entry *entry;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 26))
	entry = proc_create_data(name,  S_IFREG | S_IRUGO | S_IWUGO, parent, fops, data);
#else
	entry = create_proc_entry(name, S_IFREG | S_IRUGO | S_IWUGO, parent);
	if (entry) {
		entry->data = data;
		entry->proc_fops = fops;
	}
#endif

	return entry;
}

static int proc_get_dummy(struct seq_file *m, void *v)
{
	return 0;
}
#endif /* PLATFORM_ECOS */

static int proc_get_drv_version(struct seq_file *m, void *v)
{
	dump_drv_version(m);
	return 0;
}

static int proc_get_log_level(struct seq_file *m, void *v)
{
	dump_log_level(m);
	return 0;
}

static int proc_get_drv_cfg(struct seq_file *m, void *v)
{
	dump_drv_cfg(m);
	return 0;
}

extern u32 rtw_no_sta_alive_check;
static int proc_get_check_alive(struct seq_file *m, void *v)
{
	RTW_PRINT_SEL(m, "Station alive check: %u\n", !rtw_no_sta_alive_check);

	return 0;
}

#if !defined(__ECOS)
#if !defined(CONFIG_WFO_VIRT_SAME_CPU)
#if defined(WFO_SKIP_PCIE_SLOT)
extern u32 wfo_pci_dev_id;
static int proc_get_wfo_pci_dev_id(struct seq_file *m, void *v)
{
	RTW_PRINT_SEL(m, "0x%x\n", wfo_pci_dev_id);

	return 0;
}

static int proc_get_wfo_pci_dev_id_status(struct seq_file *m, void *v)
{
	if (RTK_SHM_DRAM->pci_param.pci_status == 1)
		seq_puts(m, "PCI ID Not found\n");
	else if (RTK_SHM_DRAM->pci_param.pci_status == 2)
		seq_puts(m, "OK\n");
	else
		seq_puts(m, "Not ready\n");

	return 0;
}

#endif /* WFO_SKIP_PCIE_SLOT */
#endif /* !CONFIG_WFO_VIRT_SAME_CPU */
#endif /* !__ECOS */

static ssize_t proc_set_check_alive(struct file *file,
                                    const char __user *buffer,
                                    size_t count, loff_t *pos,
                                    void *data)
{
	char tmp[32];

	if (count < 1)
		return -EINVAL;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		int value;
		int num = sscanf(tmp, "%d ", &value);
		if (num) {
			rtw_no_sta_alive_check = (value == 0);
			RTW_PRINT("Set station alive check to %u.\n", !rtw_no_sta_alive_check);
		} else
			return -EFAULT;
	} else
		return -EFAULT;

	return count;
}


static ssize_t proc_set_log_level(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	char tmp[32];
	int log_level_1, log_level_2;

	if (count < 1)
		return -EINVAL;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

#ifdef CONFIG_RTW_DEBUG
	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%d %d", &log_level_1, &log_level_2);

		if (num == 2 &&
		    (log_level_1 >= _DRV_NONE_ && log_level_1 <= _DRV_MAX_) &&
		   	(log_level_2 >= _DRV_NONE_ && log_level_2 <= _DRV_MAX_)) {
			rtw_drv_log_level = log_level_1;
			phl_log_level = log_level_2;
#if 0//def PLATFORM_ECOS
			if (rtw_drv_log_level > _DRV_ERR_ || phl_log_level > _DRV_WARNING_ )
			{
				log_cfg.pass_log.enable_linux_output = 1;
				log_cfg.pass_log.log_level_enable = 1;
			}
			else
			{
				if (log_cfg.pass_log.rssi_enable == 0)
					log_cfg.pass_log.enable_linux_output = 0;

				log_cfg.pass_log.log_level_enable = 0;
			}
#endif
			_dbgdump("rtw_drv_log_level:%d phl_log_level:%d\n", rtw_drv_log_level, phl_log_level);
		}
	} else
		return -EFAULT;
#else
	_dbgdump("CONFIG_RTW_DEBUG is disabled\n");
#endif

	return count;
}

#ifdef DBG_MEM_ALLOC
/*****************************************************************
* NOTE:
* The user space will parse the content of the following file,
* please DO NOT change the format of the output!
******************************************************************/
static int proc_get_mstat(struct seq_file *m, void *v)
{
	rtw_mstat_dump(m);
	return 0;
}
#endif /* DBG_MEM_ALLOC */

static int proc_get_country_chplan_map(struct seq_file *m, void *v)
{
	dump_country_chplan_map(m);
	return 0;
}

static int proc_get_chplan_id_list(struct seq_file *m, void *v)
{
	dump_chplan_id_list(m);
	return 0;
}

#if CONFIG_IEEE80211_BAND_6GHZ
static int proc_get_chplan_6g_id_list(struct seq_file *m, void *v)
{
	dump_chplan_6g_id_list(m);
	return 0;
}

static int proc_get_chplan_6g_country_list(struct seq_file *m, void *v)
{
	dump_chplan_6g_country_list(m);
	return 0;
}
#endif
static int proc_get_chplan_test(struct seq_file *m, void *v)
{
	dump_chplan_test(m);
	return 0;
}

static int proc_get_chplan_ver(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	struct _ADAPTER *adapter = (_adapter *)rtw_netdev_priv(dev);

	dump_chplan_ver(m, adapter);
	return 0;
}


static int proc_get_global_op_class(struct seq_file *m, void *v)
{
	dump_global_op_class(m);
	return 0;
}


extern void rtw_hal_get_version(char *str, u32 len);

static int proc_get_hal_info(struct seq_file *m, void *v)
{
	char ver[30] = {0};

	rtw_hal_get_version(ver, 30);

	RTW_PRINT_SEL(m, "version: %s\n", ver);

	return 0;
}


/*
* rtw_drv_proc:
* init/deinit when register/unregister driver
*/
const struct rtw_proc_hdl drv_proc_hdls[] = {
	RTW_PROC_HDL_SSEQ("ver_info", proc_get_drv_version, NULL),
#ifndef PLATFORM_ECOS
	RTW_PROC_HDL_SSEQ("log_level", proc_get_log_level, proc_set_log_level),
#endif
	RTW_PROC_HDL_SSEQ("drv_cfg", proc_get_drv_cfg, NULL),
#ifdef DBG_MEM_ALLOC
	RTW_PROC_HDL_SSEQ("mstat", proc_get_mstat, NULL),
#endif /* DBG_MEM_ALLOC */
	RTW_PROC_HDL_SSEQ("country_chplan_map", proc_get_country_chplan_map, NULL),
	RTW_PROC_HDL_SSEQ("chplan_id_list", proc_get_chplan_id_list, NULL),
#if CONFIG_IEEE80211_BAND_6GHZ
	RTW_PROC_HDL_SSEQ("chplan_6g_id_list", proc_get_chplan_6g_id_list, NULL),
	RTW_PROC_HDL_SSEQ("chplan_6g_country_list", proc_get_chplan_6g_country_list, NULL),
#endif
	RTW_PROC_HDL_SSEQ("chplan_test", proc_get_chplan_test, NULL),
	RTW_PROC_HDL_SSEQ("global_op_class", proc_get_global_op_class, NULL),
	RTW_PROC_HDL_SSEQ("hal_info", proc_get_hal_info, NULL),
	RTW_PROC_HDL_SSEQ("check_alive", proc_get_check_alive, proc_set_check_alive),
#if !defined(__ECOS)
#if !defined(CONFIG_WFO_VIRT_SAME_CPU)
#if defined(WFO_SKIP_PCIE_SLOT)
	RTW_PROC_HDL_SSEQ("wfo_pci_dev_id", proc_get_wfo_pci_dev_id, NULL),
	RTW_PROC_HDL_SSEQ("wfo_pci_dev_id_status", proc_get_wfo_pci_dev_id_status, NULL),
#endif /* WFO_SKIP_PCIE_SLOT */
#endif /* !CONFIG_WFO_VIRT_SAME_CPU */
#endif /* !__ECOS */
};

const int drv_proc_hdls_num = sizeof(drv_proc_hdls) / sizeof(struct rtw_proc_hdl);

#ifndef PLATFORM_ECOS
static int rtw_drv_proc_open(struct inode *inode, struct file *file)
{
	/* struct net_device *dev = proc_get_parent_data(inode); */
	ssize_t index = (ssize_t)PDE_DATA(inode);
	const struct rtw_proc_hdl *hdl = drv_proc_hdls + index;
	void *private = NULL;

	if (hdl->type == RTW_PROC_HDL_TYPE_SEQ) {
		int res = seq_open(file, hdl->u.seq_op);

		if (res == 0)
			((struct seq_file *)file->private_data)->private = private;

		return res;
	} else if (hdl->type == RTW_PROC_HDL_TYPE_SSEQ) {
		int (*show)(struct seq_file *, void *) = hdl->u.show ? hdl->u.show : proc_get_dummy;

		return single_open(file, show, private);
	} else if (hdl->type == RTW_PROC_HDL_TYPE_SZSEQ) {
		int (*show)(struct seq_file *, void *) = hdl->u.sz.show ? hdl->u.sz.show : proc_get_dummy;
		#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
		return single_open_size(file, show, private, hdl->u.sz.size);
		#else
		return single_open(file, show, private);
		#endif
	} else {
		return -EROFS;
	}
}

static ssize_t rtw_drv_proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *pos)
{
	ssize_t index = (ssize_t)PDE_DATA(file_inode(file));
	const struct rtw_proc_hdl *hdl = drv_proc_hdls + index;
	ssize_t (*write)(struct file *, const char __user *, size_t, loff_t *, void *) = hdl->write;

	if (write)
		return write(file, buffer, count, pos, NULL);

	return -EROFS;
}

static const struct rtw_proc_ops rtw_drv_proc_seq_fops = {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0))
	.proc_open = rtw_drv_proc_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
	.proc_write = rtw_drv_proc_write,
#else
	.owner = THIS_MODULE,
	.open = rtw_drv_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
	.write = rtw_drv_proc_write,
#endif
};

static const struct rtw_proc_ops rtw_drv_proc_sseq_fops = {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0))
	.proc_open = rtw_drv_proc_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
	.proc_write = rtw_drv_proc_write,
#else
	.owner = THIS_MODULE,
	.open = rtw_drv_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
	.write = rtw_drv_proc_write,
#endif
};

int rtw_drv_proc_init(void)
{
	int ret = _FAIL;
	ssize_t i;
	struct proc_dir_entry *entry = NULL;

	if (rtw_proc != NULL) {
		rtw_warn_on(1);
		goto exit;
	}

	rtw_proc = rtw_proc_create_dir(RTW_PROC_NAME, get_proc_net, NULL);

	if (rtw_proc == NULL) {
		rtw_warn_on(1);
		goto exit;
	}

	for (i = 0; i < drv_proc_hdls_num; i++) {
		if (drv_proc_hdls[i].type == RTW_PROC_HDL_TYPE_SEQ)
			entry = rtw_proc_create_entry(drv_proc_hdls[i].name, rtw_proc, &rtw_drv_proc_seq_fops, (void *)i);
		else if (drv_proc_hdls[i].type == RTW_PROC_HDL_TYPE_SSEQ ||
			drv_proc_hdls[i].type == RTW_PROC_HDL_TYPE_SZSEQ)
			entry = rtw_proc_create_entry(drv_proc_hdls[i].name, rtw_proc, &rtw_drv_proc_sseq_fops, (void *)i);
		else
			entry = NULL;

		if (!entry) {
			rtw_warn_on(1);
			goto exit;
		}
	}

	ret = _SUCCESS;

exit:
	return ret;
}

void rtw_drv_proc_deinit(void)
{
	int i;

	if (rtw_proc == NULL)
		return;

	for (i = 0; i < drv_proc_hdls_num; i++)
		remove_proc_entry(drv_proc_hdls[i].name, rtw_proc);

	remove_proc_entry(RTW_PROC_NAME, get_proc_net);
	rtw_proc = NULL;
}
#endif /* PLATFORM_ECOS */

#ifdef CTC_WIFI_DIAG
enum {
	CTCWIFI_2G = 0,
	CTCWIFI_5G
};

enum {
	CTCWIFI_TYPE_DIAG_LOG = 0,
	CTCWIFI_TYPE_ASSOC_ERR,
	CTCWIFI_TYPE_FRAME_BODY
};

struct ctcwifi_proc_data {
	unsigned char	index;
	_adapter *padapter;
};

struct ctcwifi_diag_log_record {
	unsigned char	time_rec[20];
	unsigned char	sta_mac[MAC_ADDR_LEN];
	unsigned char	ssid[WLAN_SSID_MAXLEN+1];
	unsigned char	msg[DIAG_LOG_MSG_MAXLEN];
};

#ifdef CONFIG_CTC_FEATURE_SUPPORT /* rtl8192cd's config */
extern struct proc_dir_entry *ctcwifi_proc_export;
#endif
struct proc_dir_entry *ctcwifi_proc_root = NULL;
_timer ctcwifi_diag_timer;
struct ctcwifi_diag_log_record *diag_log_record[2] = {NULL, NULL};
struct ctcwifi_diag_log_record *assoc_error_record[2] = {NULL, NULL};

unsigned char diag_enable[2] = {0, 0};
unsigned int diag_duration[2] = {60, 60};
unsigned int temp_diag_duration[2] = {0, 0};
unsigned int diag_log_head[2] = {0, 0}, diag_log_tail[2] = {0, 0};
unsigned int assoc_error_head[2] = {0, 0}, assoc_error_tail[2] = {0, 0};


void ctcwifi_diag_timer_func(unsigned long ctx)
{
	unsigned int idx;

#if CONFIG_IEEE80211_BAND_5GHZ
	for (idx = 0; idx < 2; idx++)
#else
	for (idx = 0; idx < 1; idx++)
#endif
	{
		if (diag_enable[idx] == 1)
		{
			//_dbgdump("<diag> enable = %d, duration = %d, temp_duration = %d\n", diag_enable[idx], diag_duration[idx], temp_diag_duration[idx]);
			if (temp_diag_duration[idx] > 0) {
				temp_diag_duration[idx]--;
			} else {
				_dbgdump("diag_duration (%d sec) expires!\n", diag_duration[idx]);
				diag_enable[idx] = 0;
			}
		}
	}

	_set_timer(&ctcwifi_diag_timer, 1000);
}

void atoh(char *ascii_ptr, char *hex_ptr,int len)
{
	int i;

	for(i = 0; i < (len / 2); i++) {
		*(hex_ptr+i) = (*(ascii_ptr+(2*i)) <= '9') ? ((*(ascii_ptr+(2*i)) - '0') * 16 ) : (((*(ascii_ptr+(2*i)) - 'A') + 10) << 4);
		*(hex_ptr+i) |= (*(ascii_ptr+(2*i)+1) <= '9') ? ((*(ascii_ptr+(2*i)+1) - '0') & 0x0f) : ((*(ascii_ptr+(2*i)+1) - 'A' + 10) & 0x0f);
	}
}

int proc_ctcwifi_association_errors_read(struct seq_file *m, void *v)
{
	_adapter *padapter = (_adapter *)m->private;
	unsigned char tmpbuf[128];
	unsigned int head = 0, tail = 0;
	struct ctcwifi_diag_log_record *record, *rec;

	if (!padapter) {
		RTW_INFO("padapter is NULL!! (%s %d)\n", __FUNCTION__, __LINE__);
		return 0;
	}

	if (rtw_get_oper_ch(padapter) > 14) {
		record = assoc_error_record[CTCWIFI_5G];
		head = assoc_error_head[CTCWIFI_5G];
		tail = assoc_error_tail[CTCWIFI_5G];

	} else {
		record = assoc_error_record[CTCWIFI_2G];
		head = assoc_error_head[CTCWIFI_2G];
		tail = assoc_error_tail[CTCWIFI_2G];
	}

	if (record == NULL) {
		RTW_INFO("assoc_error_record (ch %d) is NULL!! (%s %d)\n", rtw_get_oper_ch(padapter), __FUNCTION__, __LINE__);
		return 0;
	}

	while ((tail != head) && record != NULL) {
		rec = record + tail;
		tail = (tail + 1) & (DIAG_LOG_REC_MAXNUM-1);

		snprintf(tmpbuf, 128, "%s %02X:%02X:%02X:%02X:%02X:%02X %s: %s",
			rec->time_rec,
			rec->sta_mac[0], rec->sta_mac[1], rec->sta_mac[2],
			rec->sta_mac[3], rec->sta_mac[4], rec->sta_mac[5],
			rec->ssid,
			rec->msg);

		RTW_PRINT_SEL(m, "%s\n", tmpbuf);
	}

	return 0;
}

ssize_t proc_ctcwifi_association_errors_write(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	_adapter *padapter = (_adapter *)data;
	char tmp[128];
	unsigned int dur = 0;
	unsigned int idx = 0;
	u8 mac_addr[MAC_ALEN];

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (!padapter) {
		RTW_INFO("padapter is NULL!! (%s %d)\n", __FUNCTION__, __LINE__);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		char *loc1 = strstr(tmp, "mac:");
		char *loc2 = strstr(tmp, "msg:");
		if((loc1 != NULL) && (loc2 != NULL)) {
			atoh(loc1+4,mac_addr,12);
			//_dbgdump("mac: %x %x %x %x %x %x\n", mac_addr[0],mac_addr[1],mac_addr[2],mac_addr[3],mac_addr[4],mac_addr[5]);
			ctcwifi_assoc_err(padapter,mac_addr,loc2+4);
		}

		RTW_INFO("[write] proc_ctcwifi_association_errors_write = %s (ch %d)\n", tmp, rtw_get_oper_ch(padapter));
	}

	return count;
}

int proc_ctcwifi_stats(struct seq_file *m, void *v)
{
	_adapter *padapter = (_adapter *)m->private;
	struct dvobj_priv *dvobj = NULL;
	struct xmit_priv *pxmitpriv;
	struct recv_priv *precvpriv;
	unsigned int idx = 0;
	#if (PHL_VER_CODE >= PHL_VERSION(0001, 0017, 0000, 0000))
	struct rtw_bcn_stats *bcn_stats;
	#endif

	u32 tx_fail = 0;
	u32 tx_ok_mgmt = 0;
	u32 tx_fail_mgmt = 0;
	u32 bcn_ok = 0;
	u32 bcn_fail = 0;

	if (!padapter) {
		RTW_INFO("padapter is NULL!! (%s %d)\n", __FUNCTION__, __LINE__);
		return 0;
	}

	dvobj = adapter_to_dvobj(padapter);

	pxmitpriv = &padapter->xmitpriv;
	precvpriv = &padapter->recvpriv;

	rtw_phl_get_hw_cnt_tx_fail(dvobj->phl, &tx_fail, &tx_fail_mgmt);

	/* get bcn counters from HW */
	#if (PHL_VER_CODE >= PHL_VERSION(0001, 0017, 0000, 0000))
	rtw_phl_get_beacon_cnt(GET_HAL_INFO(padapter->dvobj), padapter->iface_id,
			       &bcn_stats);
	bcn_ok = bcn_stats->bcn_ok;
	bcn_fail = bcn_stats->bcn_fail;
	#else
	rtw_phl_get_beacon_cnt(GET_HAL_INFO(padapter->dvobj), padapter->iface_id, &bcn_ok, &bcn_fail);
	#endif /* PHL_VER_CODE */

	RTW_PRINT_SEL(m, "    tx_packets:    %llu\n", pxmitpriv->tx_pkts);
	RTW_PRINT_SEL(m, "    rx_packets:    %llu\n", precvpriv->rx_pkts);
	RTW_PRINT_SEL(m, "    tx_fails:      %d\n", tx_fail);
	RTW_PRINT_SEL(m, "    tx_drops:      %llu\n", pxmitpriv->tx_drop);

	RTW_PRINT_SEL(m, "    beacon_ok:     %d\n", bcn_ok);
	RTW_PRINT_SEL(m, "    beacon_er:     %d\n", bcn_fail);

	return 0;
}

int proc_ctcwifi_diag_enable_read(struct seq_file *m, void *v)
{
	_adapter *padapter = (_adapter *)m->private;
	unsigned int idx = 0;

	if (!padapter) {
		RTW_INFO("padapter is NULL!! (%s %d)\n", __FUNCTION__, __LINE__);
		return 0;
	}

	if (rtw_get_oper_ch(padapter) > 14)
		idx = CTCWIFI_5G;
	else
		idx = CTCWIFI_2G;

	RTW_PRINT_SEL(m, "%d\n", diag_enable[idx]);

	return 0;
}

ssize_t proc_ctcwifi_diag_enable_write(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	_adapter *padapter = (_adapter *)data;
	char tmp[32];
	unsigned char en = 0;
	unsigned int idx = 0;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (!padapter) {
		RTW_INFO("padapter is NULL!! (%s %d)\n", __FUNCTION__, __LINE__);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, 1)) {
		if ((tmp[0] - '0' >=0) && (tmp[0] - '0' <=2))
			en = tmp[0] - '0';
	}

	if (en > 2) {
		_dbgdump("diag_enable can only be 0,1,2! Set it to 1!\n");
		en = 1;
	}

	if (rtw_get_oper_ch(padapter) > 14)
		idx = CTCWIFI_5G;
	else
		idx = CTCWIFI_2G;

	if (diag_log_record[idx] == NULL) {
		RTW_INFO("diag_log_record[%d] is NULL!! (%s %d)\n", idx, __FUNCTION__, __LINE__);
		return -EFAULT;
	}

	diag_enable[idx] = en;
	if (en) {
		temp_diag_duration[idx] = diag_duration[idx];
		memset(diag_log_record[idx], 0, sizeof(struct ctcwifi_diag_log_record) * DIAG_LOG_REC_MAXNUM);
		diag_log_head[idx] = diag_log_tail[idx] = 0;
	}
	else
		temp_diag_duration[idx] = 0;

	_dbgdump("[write] diag_enable = %d\n", diag_enable[idx]);

	return count;
}

int proc_ctcwifi_diag_duration_read(struct seq_file *m, void *v)
{
	_adapter *padapter = (_adapter *)m->private;
	unsigned int idx = 0;

	if (!padapter) {
		RTW_INFO("padapter is NULL!! (%s %d)\n", __FUNCTION__, __LINE__);
		return 0;
	}

	if (rtw_get_oper_ch(padapter) > 14)
		idx = CTCWIFI_5G;
	else
		idx = CTCWIFI_2G;

	RTW_PRINT_SEL(m, "%d\n", diag_duration[idx]);

	return 0;
}

ssize_t proc_ctcwifi_diag_duration_write(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	_adapter *padapter = (_adapter *)data;
	char tmp[32];
	unsigned int dur = 0;
	unsigned int idx = 0;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (!padapter) {
		RTW_INFO("padapter is NULL!! (%s %d)\n", __FUNCTION__, __LINE__);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, 3)) {
		dur = rtw_atoi(tmp);
	}

	if (dur > 300) {
		_dbgdump("diag_duration cannot be set larger than 300! Set it to 300 sec!\n");
		dur = 300;
	}

	if (rtw_get_oper_ch(padapter) > 14)
		idx = CTCWIFI_5G;
	else
		idx = CTCWIFI_2G;

	diag_duration[idx] = dur;
	if (diag_enable[idx])
		temp_diag_duration[idx] = diag_duration[idx];

	_dbgdump("[write] diag_duration = %d\n", diag_duration[idx]);

	return count;
}

int proc_ctcwifi_diag_log(struct seq_file *m, void *v)
{
	_adapter *padapter = (_adapter *)m->private;
	unsigned char tmpbuf[255];
	unsigned int head, tail;
	struct ctcwifi_diag_log_record *record;
	unsigned int idx;

	if (!padapter) {
		RTW_INFO("padapter is NULL!! (%s %d)\n", __FUNCTION__, __LINE__);
		return 0;
	}

	if (rtw_get_oper_ch(padapter) > 14)
		idx = CTCWIFI_5G;
	else
		idx = CTCWIFI_2G;

	if (diag_log_record[idx] == NULL) {
		RTW_INFO("diag_log_record[%d] is NULL!! (%s %d)\n", idx, __FUNCTION__, __LINE__);
		return 0;
	}

	head = diag_log_head[idx];
	tail = diag_log_tail[idx];
	while (tail != head) {
		record = diag_log_record[idx] + tail;
		tail = (tail + 1) & (DIAG_LOG_REC_MAXNUM-1);

		if (record->time_rec[0] == 0) {
			RTW_PRINT_SEL(m, "%s\n", record->msg);
		} else {
			snprintf(tmpbuf, 255, "%s %s %s", record->time_rec, record->ssid, record->msg);
			RTW_PRINT_SEL(m, "%s\n", tmpbuf);
		}
	}

	return 0;
}

int proc_ctcwifi_channel_occupancy(struct seq_file *m, void *v)
{
	_adapter *padapter = (_adapter *)m->private;
	unsigned char tmpbuf[30];
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	struct rtw_env_report *env_rpt = &(dvobj->env_rpt);
	unsigned char ch_util_percent = 0;

	RTW_PRINT_SEL(m, "%s\n", "Channel	band	occupancy");

	if (!padapter) {
		RTW_INFO("padapter is NULL!! (%s %d)\n", __FUNCTION__, __LINE__);
		return 0;
	}

	if (padapter->netif_up == _FALSE || GET_PRIMARY_ADAPTER(padapter)->netif_up == _FALSE)
		return 0;

	ch_util_percent = env_rpt->nhm_cca_ratio;

	if (rtw_get_oper_ch(padapter) > 14)
		snprintf(tmpbuf, 30, "%-7d	%s	%d", rtw_get_oper_ch(padapter), "5G  ", ch_util_percent);
	else
		snprintf(tmpbuf, 30, "%-7d	%s	%d", rtw_get_oper_ch(padapter), "2G  ", ch_util_percent);

	RTW_PRINT_SEL(m, "%s\n", tmpbuf);

	return 0;
}

int proc_ctcwifi_sta_stats(struct seq_file *m, void *v)
{
	_adapter *padapter = (_adapter *)m->private;
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct rx_logs *rx_logs = &padapter->rx_logs;
	struct sta_info *psta;
	_list	*plist, *phead;

	unsigned char tmpbuf[50];
	int i, j;

	RTW_PRINT_SEL(m, "%s\n", "STA                SNR  RSSI  TXSUCC");

	if (!padapter) {
		RTW_INFO("padapter is NULL!! (%s %d)\n", __FUNCTION__, __LINE__);
		return 0;
	}

	_rtw_spinlock_bh(&pstapriv->sta_hash_lock);

	for (i = 0; i < NUM_STA; i++) {
		phead = &(pstapriv->sta_hash[i]);
		plist = get_next(phead);

		while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
			psta = LIST_CONTAINOR(plist, struct sta_info, hash_list);

			plist = get_next(plist);

			if (psta &&
				((psta->state & (WIFI_ASOC_STATE|WIFI_FW_ASSOC_SUCCESS))
					== (WIFI_ASOC_STATE|WIFI_FW_ASSOC_SUCCESS))) {

				snprintf(tmpbuf, 50, "%02X:%02X:%02X:%02X:%02X:%02X  %-3d  %-4u  %-3u",
					psta->phl_sta->mac_addr[0], psta->phl_sta->mac_addr[1], psta->phl_sta->mac_addr[2], psta->phl_sta->mac_addr[3], psta->phl_sta->mac_addr[4], psta->phl_sta->mac_addr[5],
					psta->phl_sta->hal_sta->rssi_stat.snr_ma >> 4,
					rtw_phl_get_sta_rssi(psta->phl_sta),
					100 - psta->sta_stats.tx_retry_ratio);

				RTW_PRINT_SEL(m, "%s\n", tmpbuf);
			}
		}
	}

	_rtw_spinunlock_bh(&pstapriv->sta_hash_lock);

	if (rtw_get_oper_ch(padapter) > 14)
		RTW_PRINT_SEL(m, "RXCRCERR-5G        %-d\n", rx_logs->core_rx_crc_err);
	else
		RTW_PRINT_SEL(m, "RXCRCERR-2G        %-d\n", rx_logs->core_rx_crc_err);

	return 0;
}

/*
* rtw_ctcwifi_proc:
* init/deinit when register/unregister net_device, along with rtw_adapter_proc
*/
const struct rtw_proc_hdl ctcwifi_proc_hdls[] = {
#ifndef CONFIG_CTC_FEATURE_SUPPORT /* rtl8192cd's config */
	RTW_PROC_HDL_SSEQ("association_errors_2G", proc_ctcwifi_association_errors_read, proc_ctcwifi_association_errors_write),
	RTW_PROC_HDL_SSEQ("Stat2G", proc_ctcwifi_stats, NULL),
	RTW_PROC_HDL_SSEQ("diag_enable_2G", proc_ctcwifi_diag_enable_read, proc_ctcwifi_diag_enable_write),
	RTW_PROC_HDL_SSEQ("diag_duration_2G", proc_ctcwifi_diag_duration_read, proc_ctcwifi_diag_duration_write),
	RTW_PROC_HDL_SSEQ("diag_log_2G", proc_ctcwifi_diag_log, NULL),
	RTW_PROC_HDL_SSEQ("channel_occupancy_2G", proc_ctcwifi_channel_occupancy, NULL),
	RTW_PROC_HDL_SSEQ("stats_2G", proc_ctcwifi_sta_stats, NULL),
#endif
#if CONFIG_IEEE80211_BAND_5GHZ
	RTW_PROC_HDL_SSEQ("association_errors_5G", proc_ctcwifi_association_errors_read, proc_ctcwifi_association_errors_write),
	RTW_PROC_HDL_SSEQ("Stat5G", proc_ctcwifi_stats, NULL),
	RTW_PROC_HDL_SSEQ("diag_enable_5G", proc_ctcwifi_diag_enable_read, proc_ctcwifi_diag_enable_write),
	RTW_PROC_HDL_SSEQ("diag_duration_5G", proc_ctcwifi_diag_duration_read, proc_ctcwifi_diag_duration_write),
	RTW_PROC_HDL_SSEQ("diag_log_5G", proc_ctcwifi_diag_log, NULL),
	RTW_PROC_HDL_SSEQ("channel_occupancy_5G", proc_ctcwifi_channel_occupancy, NULL),
	RTW_PROC_HDL_SSEQ("stats_5G", proc_ctcwifi_sta_stats, NULL),
#endif
};

const int ctcwifi_proc_hdls_num = sizeof(ctcwifi_proc_hdls) / sizeof(struct rtw_proc_hdl);

struct ctcwifi_proc_data ctcwifi_proc_data_array[sizeof(ctcwifi_proc_hdls) / sizeof(struct rtw_proc_hdl)];


static int rtw_ctcwifi_proc_open(struct inode *inode, struct file *file)
{
	struct ctcwifi_proc_data *data = (struct ctcwifi_proc_data *)PDE_DATA(inode);
	ssize_t index = data->index;

	const struct rtw_proc_hdl *hdl = ctcwifi_proc_hdls + index;
	void *private = data->padapter;

	if (hdl->type == RTW_PROC_HDL_TYPE_SEQ) {
		int res = seq_open(file, hdl->u.seq_op);

		if (res == 0)
			((struct seq_file *)file->private_data)->private = private;

		return res;
	} else if (hdl->type == RTW_PROC_HDL_TYPE_SSEQ) {
		int (*show)(struct seq_file *, void *) = hdl->u.show ? hdl->u.show : proc_get_dummy;

		return single_open(file, show, private);
	} else if (hdl->type == RTW_PROC_HDL_TYPE_SZSEQ) {
		int (*show)(struct seq_file *, void *) = hdl->u.sz.show ? hdl->u.sz.show : proc_get_dummy;
		#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
		return single_open_size(file, show, private, hdl->u.sz.size);
		#else
		return single_open(file, show, private);
		#endif
	} else {
		return -EROFS;
	}
}

static ssize_t rtw_ctcwifi_proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *pos)
{
	struct ctcwifi_proc_data *data = (struct ctcwifi_proc_data *)PDE_DATA(file_inode(file));
	ssize_t index = data->index;
	const struct rtw_proc_hdl *hdl = ctcwifi_proc_hdls + index;
	ssize_t (*write)(struct file *, const char __user *, size_t, loff_t *, void *) = hdl->write;

	if (write)
		return write(file, buffer, count, pos, ((struct seq_file *)file->private_data)->private);

	return -EROFS;
}

static const struct rtw_proc_ops rtw_ctcwifi_proc_seq_fops = {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0))
	.proc_open = rtw_ctcwifi_proc_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
	.proc_write = rtw_ctcwifi_proc_write,
#else
	.owner = THIS_MODULE,
	.open = rtw_ctcwifi_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
	.write = rtw_ctcwifi_proc_write,
#endif
};

static const struct rtw_proc_ops rtw_ctcwifi_proc_sseq_fops = {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0))
	.proc_open = rtw_ctcwifi_proc_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
	.proc_write = rtw_ctcwifi_proc_write,
#else
	.owner = THIS_MODULE,
	.open = rtw_ctcwifi_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
	.write = rtw_ctcwifi_proc_write,
#endif
};

void rtw_ctcwifi_adapter_set(_adapter *adapter)
{
	ssize_t i, j;
	struct dvobj_priv *dvobj = NULL;

	if (!adapter) {
		RTW_INFO("padapter is NULL!! (%s %d)\n", __FUNCTION__, __LINE__);
		return;
	}
	dvobj = adapter_to_dvobj(adapter);

#if CONFIG_IEEE80211_BAND_5GHZ
	j = ctcwifi_proc_hdls_num / 2;
#else
	j = ctcwifi_proc_hdls_num;
#endif

#ifdef CONFIG_CTC_FEATURE_SUPPORT /* rtl8192cd's config */
	j = 0;
#endif

	if (GET_HAL_SPEC(dvobj)->band_cap == BAND_CAP_5G) { /* 5G */
		for (i = j; i < ctcwifi_proc_hdls_num; i++)
			ctcwifi_proc_data_array[i].padapter = adapter;
	} else if (GET_HAL_SPEC(dvobj)->band_cap == BAND_CAP_2G) { /* 2G */
		for (i = 0; i < j; i++)
			ctcwifi_proc_data_array[i].padapter = adapter;
	} else {
		_dbgdump("rtw_ctcwifi_adapter_set error (%d)\n", adapter->setband);
	}
}

int rtw_ctcwifi_proc_init(void)
{
	int ret = _FAIL;
	ssize_t i, j;
	struct proc_dir_entry *ctc_root = NULL;
	struct proc_dir_entry *entry = NULL;
	unsigned long *dev = NULL;

#ifdef CONFIG_CTC_FEATURE_SUPPORT /* 8192cd's config */
	ctc_root = ctcwifi_proc_export;
#else
	ctc_root = rtw_proc_create_dir("ctcwifi", NULL, NULL);
#endif
	if (ctc_root == NULL) {
		DBGP("create ctc root failed!\n");
		goto exit;
	}
	ctcwifi_proc_root = ctc_root;

	for (i = 0; i < ctcwifi_proc_hdls_num; i++) {
		ctcwifi_proc_data_array[i].index = i;
		ctcwifi_proc_data_array[i].padapter = NULL;

		dev = (unsigned long *)&ctcwifi_proc_data_array[i];

		if (ctcwifi_proc_hdls[i].type == RTW_PROC_HDL_TYPE_SEQ)
			entry = rtw_proc_create_entry(ctcwifi_proc_hdls[i].name, ctcwifi_proc_root, &rtw_ctcwifi_proc_seq_fops, (void *)dev);
		else if (ctcwifi_proc_hdls[i].type == RTW_PROC_HDL_TYPE_SSEQ ||
			ctcwifi_proc_hdls[i].type == RTW_PROC_HDL_TYPE_SZSEQ)
			entry = rtw_proc_create_entry(ctcwifi_proc_hdls[i].name, ctcwifi_proc_root, &rtw_ctcwifi_proc_sseq_fops, (void *)dev);
		else
			entry = NULL;

		if (!entry) {
			rtw_warn_on(1);
			goto exit;
		}
	}

	diag_log_record[CTCWIFI_2G] = rtw_malloc(sizeof(struct ctcwifi_diag_log_record) * DIAG_LOG_REC_MAXNUM);
	if (!diag_log_record[CTCWIFI_2G]) {
		_dbgdump("allocate diag_log_record failed\n");
		goto exit;
	}
	memset(diag_log_record[CTCWIFI_2G], 0, sizeof(struct ctcwifi_diag_log_record) * DIAG_LOG_REC_MAXNUM);

	assoc_error_record[CTCWIFI_2G] = rtw_malloc(sizeof(struct ctcwifi_diag_log_record) * DIAG_LOG_REC_MAXNUM);
	if (!assoc_error_record[CTCWIFI_2G]) {
		_dbgdump("allocate assoc_error_record failed\n");
		goto exit;
	}
	memset(assoc_error_record[CTCWIFI_2G], 0, sizeof(struct ctcwifi_diag_log_record) * DIAG_LOG_REC_MAXNUM);

#if CONFIG_IEEE80211_BAND_5GHZ
	diag_log_record[CTCWIFI_5G] = rtw_malloc(sizeof(struct ctcwifi_diag_log_record) * DIAG_LOG_REC_MAXNUM);
	if (!diag_log_record[CTCWIFI_5G]) {
		_dbgdump("allocate diag_log_record failed\n");
		goto exit;
	}
	memset(diag_log_record[CTCWIFI_5G], 0, sizeof(struct ctcwifi_diag_log_record) * DIAG_LOG_REC_MAXNUM);

	assoc_error_record[CTCWIFI_5G] = rtw_malloc(sizeof(struct ctcwifi_diag_log_record) * DIAG_LOG_REC_MAXNUM);
	if (!assoc_error_record[CTCWIFI_5G]) {
		_dbgdump("allocate assoc_error_record failed\n");
		goto exit;
	}
	memset(assoc_error_record[CTCWIFI_5G], 0, sizeof(struct ctcwifi_diag_log_record) * DIAG_LOG_REC_MAXNUM);
#endif

	dev = NULL;

	rtw_init_timer(&ctcwifi_diag_timer,
		ctcwifi_diag_timer_func, NULL);
	_set_timer(&ctcwifi_diag_timer, 1000);

	ret = _SUCCESS;

exit:
	return ret;
}

void rtw_ctcwifi_proc_deinit(void)
{
	int i;

	if (ctcwifi_proc_root == NULL)
		return;

	_cancel_timer_ex(&ctcwifi_diag_timer);

	for (i = 0; i < 2; i++) {
		rtw_mfree(diag_log_record[i], sizeof(struct ctcwifi_diag_log_record) * DIAG_LOG_REC_MAXNUM);
		rtw_mfree(assoc_error_record[i], sizeof(struct ctcwifi_diag_log_record) * DIAG_LOG_REC_MAXNUM);
	}

	for (i = 0; i < ctcwifi_proc_hdls_num; i++)
		remove_proc_entry(ctcwifi_proc_hdls[i].name, ctcwifi_proc_root);

#ifndef CONFIG_CTC_FEATURE_SUPPORT /* rtl8192cd's config */
	remove_proc_entry("ctcwifi", NULL);
#endif
	ctcwifi_proc_root = NULL;
}

void ctcwifi_conn_record(_adapter *padapter, unsigned char *mac, const unsigned char *msg, unsigned int msg_type)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
	struct tm timeinfo;
#endif
	int i, n;
	unsigned char temp_buf[20];
	struct ctcwifi_diag_log_record *record = NULL;
	unsigned int *head, *tail;
	struct mlme_priv *pmlmepriv;
	struct wlan_network *cur_network;

	if (!padapter) {
		RTW_INFO("padapter is NULL!! (%s %d)\n", __FUNCTION__, __LINE__);
		return;
	}

	pmlmepriv = &(padapter->mlmepriv);
	cur_network = &(pmlmepriv->cur_network);

	if (rtw_get_oper_ch(padapter) > 14)
		n = CTCWIFI_5G;
	else
		n = CTCWIFI_2G;

	if (msg_type == CTCWIFI_TYPE_DIAG_LOG || msg_type == CTCWIFI_TYPE_FRAME_BODY) {
		record = diag_log_record[n];
		head = &diag_log_head[n];
		tail = &diag_log_tail[n];
	} else if (msg_type == CTCWIFI_TYPE_ASSOC_ERR) {
		record = assoc_error_record[n];
		head = &assoc_error_head[n];
		tail = &assoc_error_tail[n];
	}

	if (record == NULL)
		return;
	if ((msg_type == CTCWIFI_TYPE_DIAG_LOG) && !diag_enable[n])
		return;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
	rtw_time_to_tm(get_seconds()-(sys_tz.tz_minuteswest * 60), 0, &timeinfo);
#endif

	record += *head;
	*head = (*head + 1) & (DIAG_LOG_REC_MAXNUM-1);
	if (*head == *tail) {
		*tail = (*tail + 1) & (DIAG_LOG_REC_MAXNUM-1);
	}

	if (msg_type == CTCWIFI_TYPE_FRAME_BODY) {
		record->time_rec[0] = 0;
	} else {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
		snprintf(temp_buf, 20, "%04ld-%02d-%02d %02d:%02d:%02d",
			(1900+timeinfo.tm_year), (1+timeinfo.tm_mon), timeinfo.tm_mday, (timeinfo.tm_hour), (timeinfo.tm_min), (timeinfo.tm_sec));
#else
		snprintf(temp_buf, 20, "%d", get_seconds());
#endif
		memcpy(record->time_rec, temp_buf, sizeof(temp_buf));
		memcpy(record->sta_mac, mac, MAC_ADDR_LEN);
		memcpy(record->ssid, cur_network->network.Ssid.Ssid, sizeof(cur_network->network.Ssid.Ssid));
	}

	n = strlen(msg);
	if (n < DIAG_LOG_MSG_MAXLEN)
		strncpy(record->msg, msg, n);
	else
		strncpy(record->msg, msg, DIAG_LOG_MSG_MAXLEN-1);
}

void __ctcwifi_diag_log(_adapter *padapter, unsigned char *mac, unsigned int direction, unsigned char *frame_type, unsigned char *frame_body, unsigned int frame_len)
{
	unsigned char _msg[197];
	int i, n;
	unsigned char hex[3];
	unsigned char bcmac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

	if (frame_body == NULL)
		return;

	if (!padapter) {
		RTW_INFO("padapter is NULL!! (%s %d)\n", __FUNCTION__, __LINE__);
		return;
	}

	if (rtw_get_oper_ch(padapter) > 14)
		n = CTCWIFI_5G;
	else
		n = CTCWIFI_2G;

	if (!diag_enable[n])
		return;

	if (mac == NULL)
		mac = bcmac;

	if (direction)
		snprintf(_msg, 196, "sends %s to %02X:%02X:%02X:%02X:%02X:%02X [", frame_type, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	else
		snprintf(_msg, 196, "receives %s from %02X:%02X:%02X:%02X:%02X:%02X [", frame_type, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	ctcwifi_conn_record(padapter, mac, _msg, 0);

	_msg[0] = 0;
	for (i = 0; i < frame_len; i++) {
		sprintf(hex, "%02X", frame_body[i]);
		strcat(_msg, hex);
		if ((i > 0) && (((i+1) % (196/2)) == 0)) {
			ctcwifi_conn_record(padapter, mac, _msg, 2);
			_msg[0] = 0;
		}
	}
	if ((i % (196/2)) == 0)
		ctcwifi_conn_record(padapter, mac, "]\n", 2);
	else {
		strcat(_msg, "]\n");
		ctcwifi_conn_record(padapter, mac, _msg, 2);
	}
}
#endif /* PLATFORM_ECOS */

#ifndef RTW_SEQ_FILE_TEST
#define RTW_SEQ_FILE_TEST 0
#endif

#if RTW_SEQ_FILE_TEST
#define RTW_SEQ_FILE_TEST_SHOW_LIMIT 300
static void *proc_start_seq_file_test(struct seq_file *m, loff_t *pos)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	RTW_PRINT(FUNC_ADPT_FMT"\n", FUNC_ADPT_ARG(adapter));
	if (*pos >= RTW_SEQ_FILE_TEST_SHOW_LIMIT) {
		RTW_PRINT(FUNC_ADPT_FMT" pos:%llu, out of range return\n", FUNC_ADPT_ARG(adapter), *pos);
		return NULL;
	}

	RTW_PRINT(FUNC_ADPT_FMT" return pos:%lld\n", FUNC_ADPT_ARG(adapter), *pos);
	return pos;
}
void proc_stop_seq_file_test(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	RTW_PRINT(FUNC_ADPT_FMT"\n", FUNC_ADPT_ARG(adapter));
}

void *proc_next_seq_file_test(struct seq_file *m, void *v, loff_t *pos)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	(*pos)++;
	if (*pos >= RTW_SEQ_FILE_TEST_SHOW_LIMIT) {
		RTW_PRINT(FUNC_ADPT_FMT" pos:%lld, out of range return\n", FUNC_ADPT_ARG(adapter), *pos);
		return NULL;
	}

	RTW_PRINT(FUNC_ADPT_FMT" return pos:%lld\n", FUNC_ADPT_ARG(adapter), *pos);
	return pos;
}

static int proc_get_seq_file_test(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	u32 pos = *((loff_t *)(v));
	RTW_PRINT(FUNC_ADPT_FMT" pos:%d\n", FUNC_ADPT_ARG(adapter), pos);
	RTW_PRINT_SEL(m, FUNC_ADPT_FMT" pos:%d\n", FUNC_ADPT_ARG(adapter), pos);
	return 0;
}

struct seq_operations seq_file_test = {
	.start = proc_start_seq_file_test,
	.stop  = proc_stop_seq_file_test,
	.next  = proc_next_seq_file_test,
	.show  = proc_get_seq_file_test,
};
#endif /* RTW_SEQ_FILE_TEST */

#ifdef CONFIG_SDIO_HCI
static int proc_get_sd_f0_reg_dump(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct dvobj_priv *d = adapter_to_dvobj(adapter);

	rtw_hal_sd_f0_reg_dump(m, GET_HAL_INFO(d));

	return 0;
}

static int proc_get_sdio_local_reg_dump(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct dvobj_priv *d = adapter_to_dvobj(adapter);

	rtw_hal_sdio_local_reg_dump(m, GET_HAL_INFO(d));

	return 0;
}
static int proc_get_sdio_card_info(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	dump_sdio_card_info(m, adapter_to_dvobj(adapter));

	return 0;
}

#ifdef DBG_SDIO
static int proc_get_sdio_dbg(struct seq_file *m, void *v)
{
	struct net_device *dev;
	_adapter *a;
	struct dvobj_priv *d;
	struct sdio_data *sdio;


	dev = m->private;
	a = (_adapter *)rtw_netdev_priv(dev);
	d = adapter_to_dvobj(a);
	sdio = dvobj_to_sdio(d);

	dump_sdio_card_info(m, d);

	RTW_PRINT_SEL(m, "CMD52 error cnt: %d\n", sdio->cmd52_err_cnt);
	RTW_PRINT_SEL(m, "CMD53 error cnt: %d\n", sdio->cmd53_err_cnt);

#if (DBG_SDIO >= 3)
	RTW_PRINT_SEL(m, "dbg: %s\n", sdio->dbg_enable?"enable":"disable");
	RTW_PRINT_SEL(m, "err_stop: %s\n", sdio->err_stop?"enable":"disable");
	RTW_PRINT_SEL(m, "err_test: %s\n", sdio->err_test?"enable":"disable");
	RTW_PRINT_SEL(m, "err_test_triggered: %s\n",
		      sdio->err_test_triggered?"yes":"no");
#endif /* DBG_SDIO >= 3 */

#if (DBG_SDIO >= 2)
	RTW_PRINT_SEL(m, "I/O error dump mark: %d\n", sdio->reg_dump_mark);
	if (sdio->reg_dump_mark) {
		if (sdio->dbg_msg)
			RTW_PRINT_SEL(m, "debug messages: %s\n", sdio->dbg_msg);
		if (sdio->reg_mac)
			RTW_BUF_DUMP_SEL(_DRV_ALWAYS_, m, "MAC register:",
					 _TRUE, sdio->reg_mac, 0x800);
		if (sdio->reg_mac_ext)
			RTW_BUF_DUMP_SEL(_DRV_ALWAYS_, m, "MAC EXT register:",
					 _TRUE, sdio->reg_mac_ext, 0x800);
		if (sdio->reg_local)
			RTW_BUF_DUMP_SEL(_DRV_ALWAYS_, m, "SDIO Local register:",
					 _TRUE, sdio->reg_local, 0x100);
		if (sdio->reg_cia)
			RTW_BUF_DUMP_SEL(_DRV_ALWAYS_, m, "SDIO CIA register:",
					 _TRUE, sdio->reg_cia, 0x200);
	}
#endif /* DBG_SDIO >= 2 */

	return 0;
}

#if (DBG_SDIO >= 2)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
#define strnicmp	strncasecmp
#endif /* Linux kernel >= 4.0.0 */
void rtw_sdio_dbg_reg_free(struct dvobj_priv *d);
#endif /* DBG_SDIO >= 2 */

ssize_t proc_set_sdio_dbg(struct file *file, const char __user *buffer,
			  size_t count, loff_t *pos, void *data)
{
#if (DBG_SDIO >= 2)
	struct net_device *dev = data;
	struct dvobj_priv *d;
	_adapter *a;
	struct sdio_data *sdio;
	char tmp[32], cmd[32] = {0};
	int num;


	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	a = (_adapter *)rtw_netdev_priv(dev);
	d = adapter_to_dvobj(a);
	sdio = dvobj_to_sdio(d);

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		num = sscanf(tmp, "%s", cmd);

		if (num >= 1) {
			if (strnicmp(cmd, "reg_reset", 10) == 0) {
				sdio->reg_dump_mark = 0;
				goto exit;
			}
			if (strnicmp(cmd, "reg_free", 9) == 0) {
				rtw_sdio_dbg_reg_free(d);
				sdio->reg_dump_mark = 0;
				goto exit;
			}
#if (DBG_SDIO >= 3)
			if (strnicmp(cmd, "dbg_enable", 11) == 0) {
				sdio->dbg_enable = 1;
				goto exit;
			}
			if (strnicmp(cmd, "dbg_disable", 12) == 0) {
				sdio->dbg_enable = 0;
				goto exit;
			}
			if (strnicmp(cmd, "err_stop", 9) == 0) {
				sdio->err_stop = 1;
				goto exit;
			}
			if (strnicmp(cmd, "err_stop_disable", 16) == 0) {
				sdio->err_stop = 0;
				goto exit;
			}
			if (strnicmp(cmd, "err_test", 9) == 0) {
				sdio->err_test_triggered = 0;
				sdio->err_test = 1;
				goto exit;
			}
#endif /* DBG_SDIO >= 3 */
		}

		return -EINVAL;
	}

exit:
#endif /* DBG_SDIO >= 2 */
	return count;
}
#endif /* DBG_SDIO */
#endif /* CONFIG_SDIO_HCI */

static int proc_get_mac_reg_dump(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);

	rtw_phl_mac_reg_dump(m, GET_HAL_INFO(dvobj));

	return 0;
}

static int proc_get_bb_reg_dump(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);

	rtw_phl_bb_reg_dump(m, GET_HAL_INFO(dvobj));

	return 0;
}

static int proc_get_bb_reg_dump_ex(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);

	rtw_phl_bb_reg_dump_ex(m, GET_HAL_INFO(dvobj));

	return 0;
}

static int proc_get_rf_reg_dump(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);

	rtw_phl_rf_reg_dump(m, GET_HAL_INFO(dvobj));

	return 0;
}

#ifdef CONFIG_RTW_LED
int proc_get_led_config(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct led_priv *ledpriv = adapter_to_led(adapter);

	RTW_PRINT_SEL(m, "mode = %d\n", ledpriv->mode);
#ifdef CONFIG_WIFI_LED_SHARE_WITH_WPS
	RTW_PRINT_SEL(m, "last_mode = %d\n", ledpriv->last_mode);
#endif
	RTW_PRINT_SEL(m, "manual_ctrl = %d\n", ledpriv->manual_ctrl);
	RTW_PRINT_SEL(m, "manual_opt = %d\n", ledpriv->manual_opt);

	return 0;
}

ssize_t proc_set_led_config(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct led_priv *ledpriv = adapter_to_led(adapter);
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
	LED_CTL_MODE mode;
	enum rtw_led_event event;

	char tmp[32];
	u8 manual, onoff;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		/* [auto/manual] [led off/on] */
		int num = sscanf(tmp, "%hhu %hhu", &manual, &onoff);

		manual = !!manual ? _TRUE : _FALSE;

		if (manual == _TRUE) {
			if (num == 2) {
				ledpriv->manual_opt = !!onoff ? RTW_LED_OPT_LOW : RTW_LED_OPT_HIGH;
				ledpriv->manual_ctrl = _TRUE;
				rtw_led_control(adapter, LED_CTL_MANUAL);
			}
		} else {
			mode = (adapter->netif_up) ? LED_CTL_UP_IDLE : LED_CTL_DOWN;
			ledpriv->manual_ctrl = _FALSE;
			rtw_led_control(adapter, mode);
		}
	}

	return count;
}
#endif /* CONFIG_RTW_LED */

#ifdef CONFIG_AP_MODE
int proc_get_aid_status(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	dump_aid_status(m, adapter);

	return 0;
}

ssize_t proc_set_aid_status(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct sta_priv *stapriv = &adapter->stapriv;

	char tmp[32];
	u8 rr;
	u16 started_aid;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%hhu %hu", &rr, &started_aid);

		if (num >= 1)
			stapriv->rr_aid = rr ? 1 : 0;
		if (num >= 2) {
			started_aid = started_aid % (stapriv->max_aid + 1);
			stapriv->started_aid = started_aid ? started_aid : 1;
		}
	}

	return count;
}
#endif /* CONFIG_AP_MODE */

static int proc_get_dump_tx_rate_bmp(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	dump_tx_rate_bmp(m, adapter_to_dvobj(adapter));

	return 0;
}

static int proc_get_dump_adapters_status(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	dump_adapters_status(m, adapter_to_dvobj(adapter));

	return 0;
}

#ifdef CONFIG_RTW_CUSTOMER_STR
static int proc_get_customer_str(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	u8 cstr[RTW_CUSTOMER_STR_LEN];

	rtw_ps_deny(adapter, PS_DENY_IOCTL);
	if (rtw_pwr_wakeup(adapter) == _FAIL)
		goto exit;

	if (rtw_hal_customer_str_read(adapter, cstr) != _SUCCESS)
		goto exit;

	RTW_PRINT_SEL(m, RTW_CUSTOMER_STR_FMT"\n", RTW_CUSTOMER_STR_ARG(cstr));

exit:
	rtw_ps_deny_cancel(adapter, PS_DENY_IOCTL);
	return 0;
}
#endif /* CONFIG_RTW_CUSTOMER_STR */

#ifdef CONFIG_SCAN_BACKOP
static int proc_get_backop_flags_sta(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_ext_priv *mlmeext = &adapter->mlmeextpriv;

	RTW_PRINT_SEL(m, "0x%02x\n", mlmeext_scan_backop_flags_sta(mlmeext));

	return 0;
}

static ssize_t proc_set_backop_flags_sta(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_ext_priv *mlmeext = &adapter->mlmeextpriv;

	char tmp[32];
	u8 flags;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%hhx", &flags);

		if (num == 1)
			mlmeext_assign_scan_backop_flags_sta(mlmeext, flags);
	}

	return count;
}

#ifdef CONFIG_AP_MODE
static int proc_get_backop_flags_ap(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_ext_priv *mlmeext = &adapter->mlmeextpriv;

	RTW_PRINT_SEL(m, "0x%02x\n", mlmeext_scan_backop_flags_ap(mlmeext));

	return 0;
}

static ssize_t proc_set_backop_flags_ap(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_ext_priv *mlmeext = &adapter->mlmeextpriv;

	char tmp[32];
	u8 flags;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%hhx", &flags);

		if (num == 1)
			mlmeext_assign_scan_backop_flags_ap(mlmeext, flags);
	}

	return count;
}
#endif /* CONFIG_AP_MODE */

#ifdef CONFIG_RTW_MESH
static int proc_get_backop_flags_mesh(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_ext_priv *mlmeext = &adapter->mlmeextpriv;

	RTW_PRINT_SEL(m, "0x%02x\n", mlmeext_scan_backop_flags_mesh(mlmeext));

	return 0;
}

static ssize_t proc_set_backop_flags_mesh(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_ext_priv *mlmeext = &adapter->mlmeextpriv;

	char tmp[32];
	u8 flags;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%hhx", &flags);

		if (num == 1)
			mlmeext_assign_scan_backop_flags_mesh(mlmeext, flags);
	}

	return count;
}
#endif /* CONFIG_RTW_MESH */

#endif /* CONFIG_SCAN_BACKOP */

#if defined(CONFIG_LPS_PG) && defined(CONFIG_RTL8822C)
static int proc_get_lps_pg_debug(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = rtw_netdev_priv(dev);
	struct dm_struct *dm = adapter_to_phydm(adapter);

	rtw_run_in_thread_cmd(adapter, ((void *)(odm_lps_pg_debug_8822c)), dm);

	return 0;
}
#endif

/* gpio setting */
#ifdef CONFIG_GPIO_API
static ssize_t proc_set_config_gpio(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[32] = {0};
	int num = 0, gpio_pin = 0, gpio_mode = 0; /* gpio_mode:0 input  1:output; */

	if (count < 2)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		num	= sscanf(tmp, "%d %d", &gpio_pin, &gpio_mode);
		RTW_INFO("num=%d gpio_pin=%d mode=%d\n", num, gpio_pin, gpio_mode);
		padapter->pre_gpio_pin = gpio_pin;

		if (gpio_mode == 0 || gpio_mode == 1)
			rtw_hal_config_gpio(padapter, gpio_pin, gpio_mode);
	}
	return count;

}
static ssize_t proc_set_gpio_output_value(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[32] = {0};
	int num = 0, gpio_pin = 0, pin_mode = 0; /* pin_mode: 1 high         0:low */

	if (count < 2)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		num	= sscanf(tmp, "%d %d", &gpio_pin, &pin_mode);
		RTW_INFO("num=%d gpio_pin=%d pin_high=%d\n", num, gpio_pin, pin_mode);
		padapter->pre_gpio_pin = gpio_pin;

		if (pin_mode == 0 || pin_mode == 1)
			rtw_hal_set_gpio_output_value(padapter, gpio_pin, pin_mode);
	}
	return count;
}
static int proc_get_gpio(struct seq_file *m, void *v)
{
	u8 gpioreturnvalue = 0;
	struct net_device *dev = m->private;

	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	if (!padapter)
		return -EFAULT;
	gpioreturnvalue = rtw_hal_get_gpio(padapter, padapter->pre_gpio_pin);
	RTW_PRINT_SEL(m, "get_gpio %d:%d\n", padapter->pre_gpio_pin, gpioreturnvalue);

	return 0;

}
static ssize_t proc_set_gpio(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[32] = {0};
	int num = 0, gpio_pin = 0;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		num	= sscanf(tmp, "%d", &gpio_pin);
		RTW_INFO("num=%d gpio_pin=%d\n", num, gpio_pin);
		padapter->pre_gpio_pin = gpio_pin;

	}
	return count;
}
#endif

static ssize_t proc_set_rx_info_msg(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{

	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct recv_priv *precvpriv = &(padapter->recvpriv);
	char tmp[32] = {0};
	int phy_info_flag = 0;

	if (!padapter)
		return -EFAULT;

	if (count < 1) {
		RTW_INFO("argument size is less than 1\n");
		return -EFAULT;
	}

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		int num = sscanf(tmp, "%d", &phy_info_flag);

		if (num == 1)
			precvpriv->store_law_data_flag = (BOOLEAN) phy_info_flag;

		/*RTW_INFO("precvpriv->store_law_data_flag = %d\n",( BOOLEAN )(precvpriv->store_law_data_flag));*/
	}
	return count;
}
static int proc_get_rx_info_msg(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

	rtw_hal_set_phydm_var(padapter, HAL_PHYDM_RX_DATA_INFO, m, _FALSE);
	return 0;
}
static int proc_get_tx_info_msg(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct sta_info *psta;
	u8 bc_addr[ETH_ALEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	u8 null_addr[ETH_ALEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct sta_priv *pstapriv = &padapter->stapriv;
	int i;
	_list	*plist, *phead;
	u8 current_rate_id = 0, current_sgi = 0;

	char *BW, *status;

	_rtw_spinlock_bh(&pstapriv->sta_hash_lock);

	if (MLME_IS_STA(padapter))
		status = "station mode";
	else if (MLME_IS_AP(padapter))
		status = "AP mode";
	else if (MLME_IS_MESH(padapter))
		status = "mesh mode";
	else
		status = " ";
	_RTW_PRINT_SEL(m, "status=%s\n", status);
	for (i = 0; i < NUM_STA; i++) {
		phead = &(pstapriv->sta_hash[i]);
		plist = get_next(phead);

		while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {

			psta = LIST_CONTAINOR(plist, struct sta_info, hash_list);

			plist = get_next(plist);

			if ((_rtw_memcmp(psta->phl_sta->mac_addr, bc_addr, ETH_ALEN)  !=  _TRUE)
				&& (_rtw_memcmp(psta->phl_sta->mac_addr, null_addr, ETH_ALEN) != _TRUE)
				&& (_rtw_memcmp(psta->phl_sta->mac_addr, adapter_mac_addr(padapter), ETH_ALEN) != _TRUE)) {

				switch (psta->phl_sta->chandef.bw) {

				case CHANNEL_WIDTH_20:
					BW = "20M";
					break;

				case CHANNEL_WIDTH_40:
					BW = "40M";
					break;

				case CHANNEL_WIDTH_80:
					BW = "80M";
					break;

				case CHANNEL_WIDTH_160:
					BW = "160M";
					break;

				default:
					BW = "";
					break;
				}
				current_rate_id = rtw_hal_get_current_tx_rate(adapter, psta);
				current_sgi = rtw_get_current_tx_sgi(adapter, psta);

				RTW_PRINT_SEL(m, "==============================\n");
				_RTW_PRINT_SEL(m, "macaddr=" MAC_FMT"\n", MAC_ARG(psta->phl_sta->mac_addr));
				_RTW_PRINT_SEL(m, "Tx_Data_Rate=%s\n", HDATA_RATE(current_rate_id));
				_RTW_PRINT_SEL(m, "BW=%s,sgi=%u\n", BW, current_sgi);

			}
		}
	}

	_rtw_spinunlock_bh(&pstapriv->sta_hash_lock);

	return 0;

}


static int proc_get_linked_info_dump(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	if (padapter)
		RTW_PRINT_SEL(m, "rtw_hal_linked_info_dump :%s\n", (padapter->bLinkInfoDump) ? "enable" : "disable");

	return 0;
}


static ssize_t proc_set_linked_info_dump(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

	char tmp[32] = {0};
	int mode = 0, pre_mode = 0;
	int num = 0;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	pre_mode = padapter->bLinkInfoDump;
	RTW_INFO("pre_mode=%d\n", pre_mode);

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		num	= sscanf(tmp, "%d ", &mode);
		RTW_INFO("num=%d mode=%d\n", num, mode);

		if (num != 1) {
			RTW_INFO("argument number is wrong\n");
			return -EFAULT;
		}

		if (mode == 1 || (mode == 0 && pre_mode == 1)) /* not consider pwr_saving 0: */
			padapter->bLinkInfoDump = mode;

		else if ((mode == 2) || (mode == 0 && pre_mode == 2)) { /* consider power_saving */
			/* RTW_INFO("rtw_hal_linked_info_dump =%s\n", (padapter->bLinkInfoDump)?"enable":"disable") */
			rtw_hal_linked_info_dump(padapter, mode);
		}
	}
	return count;
}


static int proc_get_sta_tp_dump(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

	if (padapter)
		RTW_PRINT_SEL(m, "sta_tp_dump :%s\n", (padapter->bsta_tp_dump) ? "enable" : "disable");

	return 0;
}

static ssize_t proc_set_sta_tp_dump(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

	char tmp[32] = {0};
	int mode = 0;
	int num = 0;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		num	= sscanf(tmp, "%d ", &mode);

		if (num != 1) {
			RTW_INFO("argument number is wrong\n");
			return -EFAULT;
		}
		if (padapter)
			padapter->bsta_tp_dump = mode;
	}
	return count;
}

static int proc_get_sta_tp_info(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

	if (padapter)
		rtw_sta_traffic_info(m, padapter);

	return 0;
}

static int proc_get_turboedca_ctrl(struct seq_file *m, void *v)
{
#if 0

	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	HAL_DATA_TYPE *hal_data = GET_HAL_DATA(adapter_to_dvobj(padapter));

	if (hal_data) {

		u32 edca_param;

		if (hal_data->dis_turboedca == 0)
			RTW_PRINT_SEL(m, "Turbo-EDCA : %s\n", "Enable");
		else
			RTW_PRINT_SEL(m, "Turbo-EDCA : %s, mode=%d, edca_param_mode=0x%x\n", "Disable", hal_data->dis_turboedca, hal_data->edca_param_mode);


		rtw_hal_get_hwreg(padapter, HW_VAR_AC_PARAM_BE, (u8 *)(&edca_param));

		_RTW_PRINT_SEL(m, "PARAM_BE:0x%x\n", edca_param);

	}
#endif
	return 0;
}

static ssize_t proc_set_turboedca_ctrl(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
#if 0
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	HAL_DATA_TYPE *hal_data = GET_HAL_DATA(adapter_to_dvobj(padapter));
	char tmp[32] = {0};
	int mode = 0, num = 0;
	u32 param_mode = 0;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp))
		return -EFAULT;

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		num = sscanf(tmp, "%d %x", &mode, &param_mode);

		if (num < 1 || num > 2) {
			RTW_INFO("argument number is wrong\n");
			return -EFAULT;
		}

		/*  0: enable turboedca,
			1: disable turboedca,
			2: disable turboedca and setting EDCA parameter based on the input parameter
			> 2 : currently reset to 0 */

		if (mode > 2)
			mode = 0;

		hal_data->dis_turboedca = mode;

		hal_data->edca_param_mode = 0; /* init. value */

		RTW_INFO("dis_turboedca mode = 0x%x\n", hal_data->dis_turboedca);

		if (num == 2) {

			hal_data->edca_param_mode = param_mode;

			RTW_INFO("param_mode = 0x%x\n", param_mode);
		}

	}
#endif
	return count;

}

static int proc_get_mac_qinfo(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	rtw_hal_get_hwreg(adapter, HW_VAR_DUMP_MAC_QUEUE_INFO, (u8 *)m);

	return 0;
}

int proc_get_wifi_spec(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv	*pregpriv = &padapter->registrypriv;

	RTW_PRINT_SEL(m, "wifi_spec=%d\n", pregpriv->wifi_spec);
	return 0;
}

static int proc_get_chan_plan(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	dump_cur_chset(m, adapter_to_rfctl(adapter));

	return 0;
}

static ssize_t proc_set_chan_plan(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[32];
	u16 chplan = RTW_CHPLAN_UNSPECIFIED;
	u16 chplan_6g = RTW_CHPLAN_6G_UNSPECIFIED;

	if (!padapter)
		return -EFAULT;

	if (count < 1) {
		RTW_INFO("argument size is less than 1\n");
		return -EFAULT;
	}

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		int num = sscanf(tmp, "%hx %hx", &chplan, &chplan_6g);
		if (num < 1)
			return count;

		rtw_chplan_ioctl_input_mapping(&chplan, &chplan_6g);
	}

	rtw_set_channel_plan(padapter, chplan, chplan_6g);

	return count;
}

static int proc_get_country_code(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct rf_ctl_t *rfctl = adapter_to_rfctl(adapter);

	if (rfctl->country_ent)
		dump_country_chplan(m, rfctl->country_ent);
	else
		RTW_PRINT_SEL(m, "unspecified\n");

	return 0;
}

static ssize_t proc_set_country_code(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[32];
	char alpha2[2];
	int num;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (!buffer || copy_from_user(tmp, buffer, count))
		goto exit;

	num = sscanf(tmp, "%c%c", &alpha2[0], &alpha2[1]);
	if (num !=	2)
		return count;

	rtw_set_country(padapter, alpha2);

exit:
	return count;
}

static int cap_spt_op_class_ch_detail = 0;

static int proc_get_cap_spt_op_class_ch(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	dump_cap_spt_op_class_ch(m , adapter_to_rfctl(adapter), cap_spt_op_class_ch_detail);
	return 0;
}

static ssize_t proc_set_cap_spt_op_class_ch(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[32];
	int num;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (!buffer || copy_from_user(tmp, buffer, count))
		goto exit;

	num = sscanf(tmp, "%d", &cap_spt_op_class_ch_detail);

exit:
	return count;
}

static int reg_spt_op_class_ch_detail = 0;

static int proc_get_reg_spt_op_class_ch(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	dump_reg_spt_op_class_ch(m , adapter_to_rfctl(adapter), reg_spt_op_class_ch_detail);
	return 0;
}

static ssize_t proc_set_reg_spt_op_class_ch(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[32];
	int num;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (!buffer || copy_from_user(tmp, buffer, count))
		goto exit;

	num = sscanf(tmp, "%d", &reg_spt_op_class_ch_detail);

exit:
	return count;
}

static int cur_spt_op_class_ch_detail = 0;

static int proc_get_cur_spt_op_class_ch(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	dump_cur_spt_op_class_ch(m , adapter_to_rfctl(adapter), cur_spt_op_class_ch_detail);
	return 0;
}

static ssize_t proc_set_cur_spt_op_class_ch(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[32];
	int num;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (!buffer || copy_from_user(tmp, buffer, count))
		goto exit;

	num = sscanf(tmp, "%d", &cur_spt_op_class_ch_detail);

exit:
	return count;
}

#if CONFIG_RTW_MACADDR_ACL
static int proc_get_macaddr_acl(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	dump_macaddr_acl(m, adapter);
	return 0;
}

ssize_t proc_set_macaddr_acl(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	int tmp_size = 18 * NUM_ACL + 32; /* (macaddr + whitespace) * NUM_ACL + command */
	char *tmp;
	u8 period;
	char cmd[32];
	u8 mode;
	u8 addr[ETH_ALEN];

#define MAC_ACL_CMD_MODE	0
#define MAC_ACL_CMD_ADD		1
#define MAC_ACL_CMD_DEL		2
#define MAC_ACL_CMD_CLR		3
#define MAC_ACL_CMD_NUM		4

	static const char * const mac_acl_cmd_str[] = {
		"mode",
		"add",
		"del",
		"clr",
	};
	u8 cmd_id = MAC_ACL_CMD_NUM;

	if (count < 1)
		return -EFAULT;

	if (count > tmp_size) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	tmp = kmalloc(tmp_size, GFP_KERNEL);
	if (!tmp)
		return -ENOMEM;


	if (buffer && !copy_from_user(tmp, buffer, count)) {
		/*
		* <period> mode <mode> <macaddr> [<macaddr>]
		* <period> mode <mode>
		* <period> add <macaddr> [<macaddr>]
		* <period> del <macaddr> [<macaddr>]
		* <period> clr
		*/
		char *c, *next;
		int i;
		u8 is_bcast;

		next = tmp;
		c = strsep(&next, " \t");
		if (!c || sscanf(c, "%hhu", &period) != 1)
			goto exit;

		if (period >= RTW_ACL_PERIOD_NUM) {
			RTW_WARN(FUNC_ADPT_FMT" invalid period:%u", FUNC_ADPT_ARG(adapter), period);
			goto exit;
		}

		c = strsep(&next, " \t");
		if (!c || sscanf(c, "%s", cmd) != 1)
			goto exit;

		for (i = 0; i < MAC_ACL_CMD_NUM; i++)
			if (strcmp(mac_acl_cmd_str[i], cmd) == 0)
				cmd_id = i;

		switch (cmd_id) {
		case MAC_ACL_CMD_MODE:
			c = strsep(&next, " \t");
			if (!c || sscanf(c, "%hhu", &mode) != 1)
				goto exit;

			if (mode >= RTW_ACL_MODE_MAX) {
				RTW_WARN(FUNC_ADPT_FMT" invalid mode:%u", FUNC_ADPT_ARG(adapter), mode);
				goto exit;
			}
			break;

		case MAC_ACL_CMD_ADD:
		case MAC_ACL_CMD_DEL:
			break;

		case MAC_ACL_CMD_CLR:
			/* clear settings */
			rtw_macaddr_acl_clear(adapter, period);
			goto exit;

		default:
			RTW_WARN(FUNC_ADPT_FMT" invalid cmd:\"%s\"", FUNC_ADPT_ARG(adapter), cmd);
			goto exit;
		}

		/* check for macaddr list */
		c = strsep(&next, " \t");
		if (!c && cmd_id == MAC_ACL_CMD_MODE) {
			/* set mode only  */
			rtw_set_macaddr_acl(adapter, period, mode);
			goto exit;
		}

		if (cmd_id == MAC_ACL_CMD_MODE) {
			/* set mode and entire macaddr list */
			rtw_macaddr_acl_clear(adapter, period);
			rtw_set_macaddr_acl(adapter, period, mode);
		}

		while (c != NULL) {
			if (sscanf(c, MAC_SFMT, MAC_SARG(addr)) != 6)
				break;

			is_bcast = is_broadcast_mac_addr(addr);
			if (is_bcast
				|| rtw_check_invalid_mac_address(addr, 0) == _FALSE
			) {
				if (cmd_id == MAC_ACL_CMD_DEL) {
					rtw_acl_remove_sta(adapter, period, addr);
					if (is_bcast)
						break;
				 } else if (!is_bcast)
					rtw_acl_add_sta(adapter, period, addr, 0);
			}

			c = strsep(&next, " \t");
		}
	}

exit:
	kfree(tmp);
	return count;
}
#endif /* CONFIG_RTW_MACADDR_ACL */

#if CONFIG_RTW_PRE_LINK_STA
static int proc_get_pre_link_sta(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	dump_pre_link_sta_ctl(m, &adapter->stapriv);
	return 0;
}

ssize_t proc_set_pre_link_sta(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *mlme = &adapter->mlmepriv;
	struct mlme_ext_priv *mlmeext = &adapter->mlmeextpriv;
	char tmp[17 * RTW_PRE_LINK_STA_NUM + 32] = {0};
	char arg0[16] = {0};
	u8 addr[ETH_ALEN];

#define PRE_LINK_STA_CMD_RESET	0
#define PRE_LINK_STA_CMD_ADD	1
#define PRE_LINK_STA_CMD_DEL	2
#define PRE_LINK_STA_CMD_NUM	3

	static const char * const pre_link_sta_cmd_str[] = {
		"reset",
		"add",
		"del"
	};
	u8 cmd_id = PRE_LINK_STA_CMD_NUM;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		/* cmd [<macaddr>] */
		char *c, *next;
		int i;

		next = tmp;
		c = strsep(&next, " \t");

		if (sscanf(c, "%s", arg0) != 1)
			goto exit;

		for (i = 0; i < PRE_LINK_STA_CMD_NUM; i++)
			if (strcmp(pre_link_sta_cmd_str[i], arg0) == 0)
				cmd_id = i;

		switch (cmd_id) {
		case PRE_LINK_STA_CMD_RESET:
			rtw_pre_link_sta_ctl_reset(&adapter->stapriv);
			goto exit;
		case PRE_LINK_STA_CMD_ADD:
		case PRE_LINK_STA_CMD_DEL:
			break;
		default:
			goto exit;
		}

		/* macaddr list */
		c = strsep(&next, " \t");
		while (c != NULL) {
			if (sscanf(c, MAC_SFMT, MAC_SARG(addr)) != 6)
				break;

			if (rtw_check_invalid_mac_address(addr, 0) == _FALSE) {
				if (cmd_id == PRE_LINK_STA_CMD_ADD)
					rtw_pre_link_sta_add(&adapter->stapriv, addr);
				else
					rtw_pre_link_sta_del(&adapter->stapriv, addr);
			}

			c = strsep(&next, " \t");
		}
	}

exit:
	return count;
}
#endif /* CONFIG_RTW_PRE_LINK_STA */

static int proc_get_ch_sel_policy(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct rf_ctl_t *rfctl = adapter_to_rfctl(adapter);

	RTW_PRINT_SEL(m, "%-16s\n", "within_same_band");

	RTW_PRINT_SEL(m, "%16d\n", rfctl->ch_sel_within_same_band);

	return 0;
}

static ssize_t proc_set_ch_sel_policy(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct rf_ctl_t *rfctl = adapter_to_rfctl(adapter);
	char tmp[32];
	u8 within_sb;
	int num;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (!buffer || copy_from_user(tmp, buffer, count))
		goto exit;

	num = sscanf(tmp, "%hhu", &within_sb);
	if (num >=	1)
		rfctl->ch_sel_within_same_band = within_sb ? 1 : 0;

exit:
	return count;
}

#if CONFIG_DFS
ssize_t proc_set_csa_trigger(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct rf_ctl_t *rfctl = adapter_to_rfctl(adapter);
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
	char tmp[32];
	u8 ch, cntdown, i;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%hhu %hhu", &ch, &cntdown);

		if (num < 2)
			goto exit;
#if CONFIG_DFS
		rfctl->csa_ch = ch;
		rfctl->csa_cntdown = cntdown;
		rfctl->csa_set_ie = 1;
#endif

		rtw_mi_tx_beacon_hdl(adapter);
	}

exit:
	return count;
}

#ifdef CONFIG_DFS_MASTER
static int proc_get_dfs_test_case(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct rf_ctl_t *rfctl = adapter_to_rfctl(adapter);

	RTW_PRINT_SEL(m, "%-24s %-19s\n", "radar_detect_trigger_non", "choose_dfs_ch_first");
	RTW_PRINT_SEL(m, "%24hhu %19hhu\n"
		, rfctl->dbg_dfs_radar_detect_trigger_non
		, rfctl->dbg_dfs_choose_dfs_ch_first
	);

	return 0;
}

static ssize_t proc_set_dfs_test_case(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct rf_ctl_t *rfctl = adapter_to_rfctl(adapter);
	char tmp[32];
	u8 radar_detect_trigger_non;
	u8 choose_dfs_ch_first;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		int num = sscanf(tmp, "%hhu %hhu", &radar_detect_trigger_non, &choose_dfs_ch_first);

		if (num >= 1)
			rfctl->dbg_dfs_radar_detect_trigger_non = radar_detect_trigger_non;
		if (num >= 2)
			rfctl->dbg_dfs_choose_dfs_ch_first = choose_dfs_ch_first;
	}

	return count;
}

ssize_t proc_set_update_non_ocp(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct rf_ctl_t *rfctl = adapter_to_rfctl(adapter);
	char tmp[32];
	u8 ch, bw = CHANNEL_WIDTH_20, offset = CHAN_OFFSET_NO_EXT;
	int ms = -1;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%hhu %hhu %hhu %d", &ch, &bw, &offset, &ms);

		if (num < 1 || (bw != CHANNEL_WIDTH_20 && num < 3))
			goto exit;

		if (bw == CHANNEL_WIDTH_20)
			rtw_chset_update_non_ocp_ms(rfctl->channel_set
				, ch, bw, CHAN_OFFSET_NO_EXT, ms);
		else
			rtw_chset_update_non_ocp_ms(rfctl->channel_set
				, ch, bw, offset, ms);
	}

exit:
	return count;
}

ssize_t proc_set_radar_detect(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct rf_ctl_t *rfctl = adapter_to_rfctl(adapter);
	char tmp[32];
	u8 fake_radar_detect_cnt = 0;
	struct acs_priv *acspriv = adapter_to_acs(adapter);

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%hhu", &fake_radar_detect_cnt);

		if (num < 1)
			goto exit;

#if defined (CONFIG_DFS_CHAN_SEL_G_RANDOM) || defined (CONFIG_DFS_CHAN_SEL_R_SHORTEST_WAIT)
		rfctl->dbg_dfs_fake_radar_detect_cnt = fake_radar_detect_cnt;
#endif

	}

exit:
	return count;
}

static int proc_get_dfs_ch_sel_e_flags(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct rf_ctl_t *rfctl = adapter_to_rfctl(adapter);

	RTW_PRINT_SEL(m, "0x%02x\n", rfctl->dfs_ch_sel_e_flags);

	return 0;
}

static ssize_t proc_set_dfs_ch_sel_e_flags(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct rf_ctl_t *rfctl = adapter_to_rfctl(adapter);
	char tmp[32];
	u8 e_flags;
	int num;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (!buffer || copy_from_user(tmp, buffer, count))
		goto exit;

	num = sscanf(tmp, "%hhx", &e_flags);
	if (num != 1)
		goto exit;

	rfctl->dfs_ch_sel_e_flags = e_flags;

exit:
	return count;
}

static int proc_get_dfs_ch_sel_d_flags(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct rf_ctl_t *rfctl = adapter_to_rfctl(adapter);

	RTW_PRINT_SEL(m, "0x%02x\n", rfctl->dfs_ch_sel_d_flags);

	return 0;
}

static ssize_t proc_set_dfs_ch_sel_d_flags(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct rf_ctl_t *rfctl = adapter_to_rfctl(adapter);
	char tmp[32];
	u8 d_flags;
	int num;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (!buffer || copy_from_user(tmp, buffer, count))
		goto exit;

	num = sscanf(tmp, "%hhx", &d_flags);
	if (num != 1)
		goto exit;

	rfctl->dfs_ch_sel_d_flags = d_flags;

exit:
	return count;
}

#if CONFIG_DFS_SLAVE_WITH_RADAR_DETECT
static int proc_get_dfs_slave_with_rd(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct rf_ctl_t *rfctl = adapter_to_rfctl(adapter);

	RTW_PRINT_SEL(m, "%u\n", rfctl->dfs_slave_with_rd);

	return 0;
}

static ssize_t proc_set_dfs_slave_with_rd(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct rf_ctl_t *rfctl = adapter_to_rfctl(adapter);
	char tmp[32];
	u8 rd;
	int num;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (!buffer || copy_from_user(tmp, buffer, count))
		goto exit;

	num = sscanf(tmp, "%hhu", &rd);
	if (num != 1)
		goto exit;

	rd = rd ? 1 : 0;

	if (rfctl->dfs_slave_with_rd != rd) {
		rfctl->dfs_slave_with_rd = rd;
		rtw_dfs_rd_en_decision_cmd(adapter);
	}

exit:
	return count;
}
#endif /* CONFIG_DFS_SLAVE_WITH_RADAR_DETECT */

static int proc_get_mib_dfs(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct rf_ctl_t *rfctl = adapter_to_rfctl(adapter);

	RTW_PRINT_SEL(m, "csa_ch: %u\n", rfctl->csa_ch);
	RTW_PRINT_SEL(m, "csa_countdown: %u\n", rfctl->csa_cntdown);
	RTW_PRINT_SEL(m, "csa_set_ie: %u\n", rfctl->csa_set_ie);
	RTW_PRINT_SEL(m, "radar_detect_enabled: %u\n", rfctl->radar_detect_enabled);
	RTW_PRINT_SEL(m, "radar_detected: %u\n", rfctl->radar_detected);
	RTW_PRINT_SEL(m, "radar_detect_ch: %u\n", rfctl->radar_detect_ch);
	RTW_PRINT_SEL(m, "radar_detect_bw: %u\n", rfctl->radar_detect_bw);
	RTW_PRINT_SEL(m, "radar_detect_offset: %u\n", rfctl->radar_detect_offset);
	RTW_PRINT_SEL(m, "radar_detect_by_others: %u\n", rfctl->radar_detect_by_others);

	return 0;
}

static ssize_t proc_set_reset_non_ocp_time(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct rf_ctl_t *rfctl = adapter_to_rfctl(adapter);
	char tmp[32];
	u8 d_flags;
	int num;
	int i,chan_num;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (!buffer || copy_from_user(tmp, buffer, count))
		goto exit;

	num = sscanf(tmp, "%hhx", &d_flags);
	if (num != 1)
		goto exit;

	if(d_flags != 1){
		printk("value = %d, it need to be set to 1 \n",d_flags);
		goto exit;
	}

	for(i = 0; i < MAX_CHANNEL_NUM; i++){
		chan_num = rfctl->channel_set[i].ChannelNum;
		if(chan_num != 0){
			rtw_chset_update_non_ocp_ms(rfctl->channel_set
				, chan_num, CHANNEL_WIDTH_20, CHAN_OFFSET_NO_EXT, 0);

			rfctl->cac_start_time = rfctl->cac_end_time = RTW_CAC_STOPPED;
			rtw_reset_cac(rfctl, chan_num, CHANNEL_WIDTH_20, CHAN_OFFSET_NO_EXT);
#ifdef CONFIG_DFS_CHAN_SEL_G_RANDOM
			rfctl->channel_set[i].is_bw_selected = 0;
#endif
		}
	}

	printk("reset non-ocp time in chan plan");
exit:
	return count;
}

static ssize_t proc_set_dfs_cac_time(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct rf_ctl_t *rfctl = adapter_to_rfctl(adapter);
	char tmp[32];
	int time;
	int num;
	int i,chan_num;
	struct dvobj_priv *dvobj = NULL;
	dvobj = adapter_to_dvobj(adapter);

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (!buffer || copy_from_user(tmp, buffer, count))
		goto exit;

	num = sscanf(tmp, "%d", &time);
	if (num != 1)
		goto exit;

	printk("set dfs cac time = %d (s) \n",time);
	time*=1000;

	for(i = 0; i < MAX_CHANNEL_NUM; i++){
		chan_num = rfctl->channel_set[i].ChannelNum;
		if(chan_num != 0){
			rtw_chset_update_non_ocp_ms(rfctl->channel_set
				, chan_num, CHANNEL_WIDTH_20, CHAN_OFFSET_NO_EXT, 0);

			rfctl->cac_start_time = rtw_get_current_time();
			rfctl->cac_end_time = rfctl->cac_start_time + rtw_ms_to_systime(time);

		}
	}

	_set_timer(&rfctl->radar_detect_timer
			, rtw_dfs_hal_radar_detect_polling_int_ms(dvobj));

		if (rtw_rfctl_overlap_radar_detect_ch(rfctl)) {
			if (IS_CH_WAITING(rfctl)) {
				rtw_phl_cmd_dfs_rd_set_cac_status(dvobj->phl,
								HW_BAND_0,
								true,
								PHL_CMD_DIRECTLY,
								0);
			}

		}

exit:
	return count;
}


#endif /* CONFIG_DFS_MASTER */

static int
proc_get_dfs_regions(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(adapter);
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(pdvobjpriv);
	struct rtw_dfs_t *dfs_info = &phl_com->dfs_info;

	printk("dfs regions = %d (1=FCC, 2=JAP, 3=ETSI, 4=KCC)\n",dfs_info->region_domain);

	return 0;
}

static ssize_t
proc_set_dfs_regions(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct rf_ctl_t *rfctl = adapter_to_rfctl(adapter);
	char tmp[32];
	u8 reg_num;
	int num;
	enum phl_cmd_type cmd_type = PHL_CMD_DIRECTLY;
	u32 cmd_timeout = 0;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (!buffer || copy_from_user(tmp, buffer, count))
		goto exit;

	num = sscanf(tmp, "%hhu", &reg_num);
	if (num != 1)
		goto exit;

	rtw_phl_cmd_dfs_change_domain(adapter->dvobj->phl,
					HW_BAND_0,
					reg_num,
					cmd_type,cmd_timeout);
	printk("set dfs domain = %d (1=FCC, 2=JAP, 3=ETSI, 4=KCC)\n",reg_num);
exit:
	return count;
}
#endif

#ifdef CONFIG_80211N_HT
int proc_get_rx_ampdu_size_limit(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	dump_regsty_rx_ampdu_size_limit(m, adapter);

	return 0;
}

ssize_t proc_set_rx_ampdu_size_limit(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv *regsty = adapter_to_regsty(adapter);
	char tmp[32];
	u8 nss;
	u8 limit_by_bw[4] = {0xFF};

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		int i;
		int num = sscanf(tmp, "%hhu %hhu %hhu %hhu %hhu"
			, &nss, &limit_by_bw[0], &limit_by_bw[1], &limit_by_bw[2], &limit_by_bw[3]);

		if (num < 2)
			goto exit;
		if (nss == 0 || nss > 4)
			goto exit;

		for (i = 0; i < num - 1; i++)
			regsty->rx_ampdu_sz_limit_by_nss_bw[nss - 1][i] = limit_by_bw[i];

		rtw_rx_ampdu_apply(adapter);
	}

exit:
	return count;
}
#endif /* CONFIG_80211N_HT */

static int proc_get_rx_chk_limit(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	int rx_chk_limit = 0;

	if (padapter->dvobj->oper_channel >= 52 && padapter->dvobj->oper_channel <= 64)
		rx_chk_limit = rtw_get_rx_chk_limit(padapter, 1);
	else if (padapter->dvobj->oper_channel >= 100 && padapter->dvobj->oper_channel <= 144)
		rx_chk_limit = rtw_get_rx_chk_limit(padapter, 1);
	else
		rx_chk_limit = rtw_get_rx_chk_limit(padapter, 0);

	RTW_PRINT_SEL(m, "Current Rx Chk Limit : %d\n", rx_chk_limit);

	return 0;
}

static ssize_t proc_set_rx_chk_limit(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	char tmp[32];
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	int rx_chk_limit, rx_chk_limit_w53w56;

	if (count < 1) {
		RTW_INFO("argument size is less than 1\n");
		return -EFAULT;
	}

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		int num = sscanf(tmp, "%d,%d", &rx_chk_limit, &rx_chk_limit_w53w56);

		rtw_set_rx_chk_limit(padapter, rx_chk_limit, rx_chk_limit_w53w56);
	}

	return count;
}

static int proc_get_udpport(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct recv_priv *precvpriv = &(padapter->recvpriv);

	RTW_PRINT_SEL(m, "%d\n", precvpriv->sink_udpport);
	return 0;
}
static ssize_t proc_set_udpport(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct recv_priv *precvpriv = &(padapter->recvpriv);
	int sink_udpport = 0;
	char tmp[32];


	if (!padapter)
		return -EFAULT;

	if (count < 1) {
		RTW_INFO("argument size is less than 1\n");
		return -EFAULT;
	}

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%d", &sink_udpport);

		if (num !=  1) {
			RTW_INFO("invalid input parameter number!\n");
			return count;
		}

	}
	precvpriv->sink_udpport = sink_udpport;

	return count;

}

#if 0
static int proc_get_mi_ap_bc_info(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
	struct macid_ctl_t *macid_ctl = dvobj_to_macidctl(dvobj);
	u8 i;

	for (i = 0; i < dvobj->iface_nums; i++)
		RTW_PRINT_SEL(m, "iface_id:%d, mac_id && sec_cam_id = %d\n", i, macid_ctl->iface_bmc[i]);

	return 0;
}
#endif
#if 0
static int proc_get_macid_info(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
	struct macid_ctl_t *macid_ctl = dvobj_to_macidctl(dvobj);
	u8 i;
	u8 null_addr[ETH_ALEN] = {0};
	u8 *macaddr;

	RTW_PRINT_SEL(m, "max_num:%u\n", macid_ctl->num);
	RTW_PRINT_SEL(m, "\n");

	RTW_PRINT_SEL(m, "used:\n");
	dump_macid_map(m, &macid_ctl->used, macid_ctl->num);
	RTW_PRINT_SEL(m, "\n");

	RTW_PRINT_SEL(m, "%-3s %-3s %-5s %-4s %-17s %-6s %-3s"
		, "id", "bmc", "ifbmp", "ch_g", "macaddr", "bw", "vht");

	if (GET_HAL_TX_NSS(dvobj) > 2)
		_RTW_PRINT_SEL(m, " %-10s", "rate_bmp1");

	_RTW_PRINT_SEL(m, " %-10s %s\n", "rate_bmp0", "status");

	for (i = 0; i < macid_ctl->num; i++) {
		if (rtw_macid_is_used(macid_ctl, i)
			|| macid_ctl->h2c_msr[i]
		) {
			if (macid_ctl->sta[i])
				macaddr = macid_ctl->sta[i]->phl_sta->mac_addr;
			else
				macaddr = null_addr;

			RTW_PRINT_SEL(m, "%3u %3u  0x%02x %4d "MAC_FMT" %6s %3u"
				, i
				, rtw_macid_is_bmc(macid_ctl, i)
				, rtw_macid_get_iface_bmp(macid_ctl, i)
				, rtw_macid_get_ch_g(macid_ctl, i)
				, MAC_ARG(macaddr)
				, ch_width_str(macid_ctl->bw[i])
				, macid_ctl->vht_en[i]
			);

			if (GET_HAL_TX_NSS(dvobj) > 2)
				_RTW_PRINT_SEL(m, " 0x%08X", macid_ctl->rate_bmp1[i]);
#ifdef DBG_MACID_MSR_INFO
			_RTW_PRINT_SEL(m, " 0x%08X "H2C_MSR_FMT" %s\n"
				, macid_ctl->rate_bmp0[i]
				, H2C_MSR_ARG(&macid_ctl->h2c_msr[i])
				, rtw_macid_is_used(macid_ctl, i) ? "" : "[unused]"
			);
#endif
		}
	}

	return 0;
}
#endif
static int proc_get_sec_cam(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
	struct cam_ctl_t *cam_ctl = &dvobj->cam_ctl;

	RTW_PRINT_SEL(m, "sec_cap:0x%02x\n", cam_ctl->sec_cap);
	RTW_PRINT_SEL(m, "flags:0x%08x\n", cam_ctl->flags);
	RTW_PRINT_SEL(m, "\n");

	RTW_PRINT_SEL(m, "max_num:%u\n", cam_ctl->num);
	RTW_PRINT_SEL(m, "used:\n");
	dump_sec_cam_map(m, &cam_ctl->used, cam_ctl->num);
	RTW_PRINT_SEL(m, "\n");

#if 0 /*GEORGIA_TODO_REDEFINE_IO*/
	RTW_PRINT_SEL(m, "reg_scr:0x%04x\n", rtw_read16(adapter, 0x680));
#endif
	RTW_PRINT_SEL(m, "\n");

	dump_sec_cam(m, adapter);

	return 0;
}

static ssize_t proc_set_sec_cam(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
	struct cam_ctl_t *cam_ctl = &dvobj->cam_ctl;
	char tmp[32] = {0};
	char cmd[4];
	u8 id_1 = 0, id_2 = 0;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		/* c <id_1>: clear specific cam entry */
		/* wfc <id_1>: write specific cam entry from cam cache */
		/* sw <id_1> <id_2>: sec_cam 1/2 swap */

		int num = sscanf(tmp, "%s %hhu %hhu", cmd, &id_1, &id_2);

		if (num < 2)
			return count;

		if ((id_1 >= cam_ctl->num) || (id_2 >= cam_ctl->num)) {
			RTW_ERR(FUNC_ADPT_FMT" invalid id_1:%u id_2:%u\n", FUNC_ADPT_ARG(adapter), id_1, id_2);
			return count;
		}

		if (strcmp("c", cmd) == 0) {
			_clear_cam_entry(adapter, id_1);
			adapter->securitypriv.hw_decrypted = _FALSE; /* temporarily set this for TX path to use SW enc */
		} else if (strcmp("wfc", cmd) == 0)
			write_cam_from_cache(adapter, id_1);
		else if (strcmp("sw", cmd) == 0)
			rtw_sec_cam_swap(adapter, id_1, id_2);
		else if (strcmp("cdk", cmd) == 0)
			rtw_clean_dk_section(adapter);
#ifdef DBG_SEC_CAM_MOVE
		else if (strcmp("sgd", cmd) == 0)
			rtw_hal_move_sta_gk_to_dk(adapter);
		else if (strcmp("rsd", cmd) == 0)
			rtw_hal_read_sta_dk_key(adapter, id_1);
#endif
	}

	return count;
}

static int proc_get_sec_cam_cache(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	dump_sec_cam_cache(m, adapter);
	return 0;
}

#ifdef CONFIG_DBG_AX_CAM

static int proc_get_ax_address_cam(struct seq_file *m, void *v) {
	struct net_device *dev = m->private;
	struct _ADAPTER *adapter = (struct _ADAPTER *)rtw_netdev_priv(dev);

	get_ax_address_cam(m, adapter);
	return 0;
}

static int proc_get_ax_sec_cam(struct seq_file *m, void *v) {
	struct net_device *dev = m->private;
	struct _ADAPTER *adapter = (struct _ADAPTER *)rtw_netdev_priv(dev);

	get_ax_sec_cam(m, adapter);
	return 0;
}

static int proc_get_ax_valid_key(struct seq_file *m, void *v) {
	struct net_device *dev = m->private;
	struct _ADAPTER *adapter = (struct _ADAPTER *)rtw_netdev_priv(dev);

	get_ax_valid_key(m, adapter);
	return 0;
}

#endif

static ssize_t proc_set_change_bss_chbw(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
	int i;
	char tmp[32];
	s16 ch;
	s8 bw = REQ_BW_NONE, offset = REQ_OFFSET_NONE;
	u16 ifbmp = 0;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%hd %hhd %hhd %hx", &ch, &bw, &offset, &ifbmp);

		if (num < 1 || (bw != CHANNEL_WIDTH_20 && num < 3))
			goto exit;

		if (num < 4)
			ifbmp = BIT(adapter->iface_id);
		else
			ifbmp &= (1 << dvobj->iface_nums) - 1;

		for (i = 0; i < dvobj->iface_nums; i++) {
			if (!dvobj->padapters[i])
				continue;

			if (CHK_MLME_STATE(dvobj->padapters[i], WIFI_AP_STATE) && dvobj->padapters[i]->netif_up)
				ifbmp |= BIT(i);
		}

		if (ifbmp) {
			if(bw > REQ_BW_NONE && bw < CHANNEL_WIDTH_MAX){
				rtw_change_bss_chbw_cmd(adapter, RTW_CMDF_WAIT_ACK, ifbmp, 0, ch, bw, offset);
			}
		}
	}

exit:
	return count;
}

static int proc_get_tx_bw_mode(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	RTW_PRINT_SEL(m, "0x%02x\n", adapter->driver_tx_bw_mode);
	RTW_PRINT_SEL(m, "2.4G:%s\n", ch_width_str(ADAPTER_TX_BW_2G(adapter)));
	RTW_PRINT_SEL(m, "5G:%s\n", ch_width_str(ADAPTER_TX_BW_5G(adapter)));

	return 0;
}

static void rtw_set_tx_bw_mode(_adapter *adapter, u8 bw_mode)
{
	struct mlme_ext_priv *mlmeext = &(adapter->mlmeextpriv);
	/* macid_ctl move to phl */
	/* struct macid_ctl_t *macid_ctl = &adapter->dvobj->macid_ctl; */
	u8 update = _FALSE;

	if ((MLME_STATE(adapter) & WIFI_ASOC_STATE)
		&& ((mlmeext->cur_channel <= 14 && BW_MODE_2G(bw_mode) != ADAPTER_TX_BW_2G(adapter))
			|| (mlmeext->cur_channel >= 36 && BW_MODE_5G(bw_mode) != ADAPTER_TX_BW_5G(adapter)))
	) {
		/* RA mask update needed */
		update = _TRUE;
	}
	adapter->driver_tx_bw_mode = bw_mode;

	/* ToDo */
#if 0
	if (update == _TRUE) {
		struct sta_info *sta;
		int i;

		for (i = 0; i < MACID_NUM_SW_LIMIT; i++) {
			sta = macid_ctl->sta[i];
			if (sta && !is_broadcast_mac_addr(sta->phl_sta->mac_addr))
				rtw_dm_ra_mask_wk_cmd(adapter, (u8 *)sta);
		}
	}
#endif
}

static ssize_t proc_set_tx_bw_mode(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[32];
	u8 bw_mode;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%hhx", &bw_mode);

		if (num < 1 || bw_mode == adapter->driver_tx_bw_mode)
			goto exit;

		rtw_set_tx_bw_mode(adapter, bw_mode);
	}

exit:
	return count;
}

static int proc_get_hal_txpwr_info(struct seq_file *m, void *v)
{
#ifdef CONFIG_TXPWR_PG_WITH_PWR_IDX
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct dvobj_priv *devob = adapter_to_dvobj(adapter);
	HAL_DATA_TYPE *hal_data = GET_HAL_DATA(devob);
	struct hal_spec_t *hal_spec = GET_HAL_SPEC(devob);

	if (hal_data->txpwr_pg_mode == TXPWR_PG_WITH_PWR_IDX) {
		if (rtw_hw_is_band_support(devob, BAND_ON_24G))
			dump_hal_txpwr_info_2g(m, adapter, hal_spec->rfpath_num_2g, hal_data->max_tx_cnt);

		#if CONFIG_IEEE80211_BAND_5GHZ
		if (rtw_hw_is_band_support(devob, BAND_ON_5G))
			dump_hal_txpwr_info_5g(m, adapter, hal_spec->rfpath_num_5g, hal_data->max_tx_cnt);
		#endif
	}
#endif

	return 0;
}

static int proc_get_target_tx_power(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	rtw_hal_dump_target_tx_power(m, adapter);

	return 0;
}

static int proc_get_tx_power_by_rate(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	rtw_hal_dump_tx_power_by_rate(m, adapter);

	return 0;
}

#if CONFIG_TXPWR_LIMIT
static int proc_get_tx_power_limit(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	rtw_hal_dump_txpwr_lmt(m, adapter);

	return 0;
}
#endif /* CONFIG_TXPWR_LIMIT */

#ifdef GEORGIA_TODO_TX_PWR
static int proc_get_tx_power_ext_info(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	rtw_hal_dump_tx_power_ext_info(m, adapter);

	return 0;
}

static ssize_t proc_set_tx_power_ext_info(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	char tmp[32] = {0};
	char cmd[16] = {0};

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%s", cmd);

		if (num < 1)
			return count;

		#ifdef CONFIG_LOAD_PHY_PARA_FROM_FILE
		phy_free_filebuf_mask(adapter, LOAD_BB_PG_PARA_FILE | LOAD_RF_TXPWR_LMT_PARA_FILE);
		#endif

		rtw_ps_deny(adapter, PS_DENY_IOCTL);
		if (rtw_pwr_wakeup(adapter) == _FALSE)
			goto clear_ps_deny;

		if (strcmp("default", cmd) == 0)
			rtw_run_in_thread_cmd(adapter, ((void *)(phy_reload_default_tx_power_ext_info)), adapter);
		else
			rtw_run_in_thread_cmd(adapter, ((void *)(phy_reload_tx_power_ext_info)), adapter);

		rtw_run_in_thread_cmd_wait(adapter, ((void *)(rtw_hal_update_txpwr_level)), adapter, 2000);

clear_ps_deny:
		rtw_ps_deny_cancel(adapter, PS_DENY_IOCTL);
	}

	return count;
}


static void *proc_start_tx_power_idx(struct seq_file *m, loff_t *pos)
{
	u8 path = ((*pos) & 0xFF00) >> 8;

	if (path >= RF_PATH_MAX)
		return NULL;

	return pos;
}
static void proc_stop_tx_power_idx(struct seq_file *m, void *v)
{
}

static void *proc_next_tx_power_idx(struct seq_file *m, void *v, loff_t *pos)
{
	u8 path = ((*pos) & 0xFF00) >> 8;
	u8 rs = *pos & 0xFF;

	rs++;
	if (rs >= RATE_SECTION_NUM) {
		rs = 0;
		path++;
	}

	if (path >= RF_PATH_MAX)
		return NULL;

	*pos = (path << 8) | rs;

	return pos;
}

static int proc_get_tx_power_idx(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	u32 pos = *((loff_t *)(v));
	u8 path = (pos & 0xFF00) >> 8;
	u8 rs = pos & 0xFF;

	if (0)
		RTW_INFO("%s path=%u, rs=%u\n", __func__, path, rs);

	if (path == RF_PATH_A && rs == CCK)
		dump_tx_power_idx_title(m, adapter);
	dump_tx_power_idx_by_path_rs(m, adapter, path, rs);

	return 0;
}

static struct seq_operations seq_ops_tx_power_idx = {
	.start = proc_start_tx_power_idx,
	.stop  = proc_stop_tx_power_idx,
	.next  = proc_next_tx_power_idx,
	.show  = proc_get_tx_power_idx,
};

static void *proc_start_txpwr_total_dbm(struct seq_file *m, loff_t *pos)
{
	u8 rs = *pos;

	if (rs >= RATE_SECTION_NUM)
		return NULL;

	return pos;
}

static void proc_stop_txpwr_total_dbm(struct seq_file *m, void *v)
{
}

static void *proc_next_txpwr_total_dbm(struct seq_file *m, void *v, loff_t *pos)
{
	u8 rs = *pos;

	rs++;
	if (rs >= RATE_SECTION_NUM)
		return NULL;

	*pos = rs;

	return pos;
}

static int proc_get_txpwr_total_dbm(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	u32 pos = *((loff_t *)(v));
	u8 rs = pos;

	if (rs == CCK)
		dump_txpwr_total_dbm_title(m, adapter);
	dump_txpwr_total_dbm_by_rs(m, adapter, rs);

	return 0;
}

static struct seq_operations seq_ops_txpwr_total_dbm = {
	.start = proc_start_txpwr_total_dbm,
	.stop  = proc_stop_txpwr_total_dbm,
	.next  = proc_next_txpwr_total_dbm,
	.show  = proc_get_txpwr_total_dbm,
};
#endif

#ifdef CONFIG_BTC
static u8 btreg_read_type = 0;
static u16 btreg_read_addr = 0;
static int btreg_read_error = 0;
static u8 btreg_write_type = 0;
static u16 btreg_write_addr = 0;
static int btreg_write_error = 0;

static u8 *btreg_type[] = {
	"rf",
	"modem",
	"bluewize",
	"vendor",
	"le"
};

static int btreg_parse_str(char const *input, u8 *type, u16 *addr, u16 *val)
{
	u32 num;
	u8 str[80] = {0};
	u8 t = 0;
	u32 a, v;
	u8 i, n;


	num = sscanf(input, "%s %x %x", str, &a, &v);
	if (num < 2) {
		RTW_INFO("%s: INVALID input!(%s)\n", __FUNCTION__, input);
		return -EINVAL;
	}
	if ((num < 3) && val) {
		RTW_INFO("%s: INVALID input!(%s)\n", __FUNCTION__, input);
		return -EINVAL;
	}

	n = sizeof(btreg_type) / sizeof(btreg_type[0]);
	for (i = 0; i < n; i++) {
		if (!strcasecmp(str, btreg_type[i])) {
			t = i;
			break;
		}
	}
	if (i == n) {
		RTW_INFO("%s: unknown type(%s)!\n", __FUNCTION__, str);
		return -EINVAL;
	}

	switch (t) {
	case 0:
		/* RF */
		if (a & 0xFFFFFF80) {
			RTW_INFO("%s: INVALID address(0x%X) for type %s(%d)!\n",
				 __FUNCTION__, a, btreg_type[t], t);
			return -EINVAL;
		}
		break;
	case 1:
		/* Modem */
		if (a & 0xFFFFFE00) {
			RTW_INFO("%s: INVALID address(0x%X) for type %s(%d)!\n",
				 __FUNCTION__, a, btreg_type[t], t);
			return -EINVAL;
		}
		break;
	default:
		/* Others(Bluewize, Vendor, LE) */
		if (a & 0xFFFFF000) {
			RTW_INFO("%s: INVALID address(0x%X) for type %s(%d)!\n",
				 __FUNCTION__, a, btreg_type[t], t);
			return -EINVAL;
		}
		break;
	}

	if (val) {
		if (v & 0xFFFF0000) {
			RTW_INFO("%s: INVALID value(0x%x)!\n", __FUNCTION__, v);
			return -EINVAL;
		}
		*val = (u16)v;
	}

	*type = (u8)t;
	*addr = (u16)a;

	return 0;
}

int proc_get_btreg_read(struct seq_file *m, void *v)
{
#if 0
	struct net_device *dev;
	_adapter *padapter;
	u16 ret;
	u32 data;


	if (btreg_read_error)
		return btreg_read_error;

	dev = m->private;
	padapter = (_adapter *)rtw_netdev_priv(dev);

	ret = rtw_btc_btreg_read(padapter, btreg_read_type, btreg_read_addr, &data);
	if (CHECK_STATUS_CODE_FROM_BT_MP_OPER_RET(ret, BT_STATUS_BT_OP_SUCCESS))
		RTW_PRINT_SEL(m, "BTREG read: (%s)0x%04X = 0x%08x\n", btreg_type[btreg_read_type], btreg_read_addr, data);
	else
		RTW_PRINT_SEL(m, "BTREG read: (%s)0x%04X read fail. error code = 0x%04x.\n", btreg_type[btreg_read_type], btreg_read_addr, ret);
#endif

	return 0;
}

ssize_t proc_set_btreg_read(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
#if 0
	struct net_device *dev = data;
	_adapter *padapter;
	u8 tmp[80] = {0};
	u32 num;
	int err;


	padapter = (_adapter *)rtw_netdev_priv(dev);

	if (NULL == buffer) {
		RTW_INFO(FUNC_ADPT_FMT ": input buffer is NULL!\n",
			 FUNC_ADPT_ARG(padapter));
		err = -EFAULT;
		goto exit;
	}

	if (count < 1) {
		RTW_INFO(FUNC_ADPT_FMT ": input length is 0!\n",
			 FUNC_ADPT_ARG(padapter));
		err = -EFAULT;
		goto exit;
	}

	num = count;
	if (num > (sizeof(tmp) - 1))
		num = (sizeof(tmp) - 1);

	if (copy_from_user(tmp, buffer, num)) {
		RTW_INFO(FUNC_ADPT_FMT ": copy buffer from user space FAIL!\n",
			 FUNC_ADPT_ARG(padapter));
		err = -EFAULT;
		goto exit;
	}
	/* [Coverity] sure tmp end with '\0'(string terminal) */
	tmp[sizeof(tmp) - 1] = 0;

	err = btreg_parse_str(tmp, &btreg_read_type, &btreg_read_addr, NULL);
	if (err)
		goto exit;

	RTW_INFO(FUNC_ADPT_FMT ": addr=(%s)0x%X\n",
		FUNC_ADPT_ARG(padapter), btreg_type[btreg_read_type], btreg_read_addr);

exit:
	btreg_read_error = err;
#endif

	return count;
}

int proc_get_btreg_write(struct seq_file *m, void *v)
{
#if 0
	struct net_device *dev;
	_adapter *padapter;
	u16 ret;
	u32 data;


	if (btreg_write_error < 0)
		return btreg_write_error;
	else if (btreg_write_error > 0) {
		RTW_PRINT_SEL(m, "BTREG write: (%s)0x%04X write fail. error code = 0x%04x.\n", btreg_type[btreg_write_type], btreg_write_addr, btreg_write_error);
		return 0;
	}

	dev = m->private;
	padapter = (_adapter *)rtw_netdev_priv(dev);

	ret = rtw_btc_btreg_read(padapter, btreg_write_type, btreg_write_addr, &data);
	if (CHECK_STATUS_CODE_FROM_BT_MP_OPER_RET(ret, BT_STATUS_BT_OP_SUCCESS))
		RTW_PRINT_SEL(m, "BTREG read: (%s)0x%04X = 0x%08x\n", btreg_type[btreg_write_type], btreg_write_addr, data);
	else
		RTW_PRINT_SEL(m, "BTREG read: (%s)0x%04X read fail. error code = 0x%04x.\n", btreg_type[btreg_write_type], btreg_write_addr, ret);
#endif

	return 0;
}

ssize_t proc_set_btreg_write(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
#if 0
	struct net_device *dev = data;
	_adapter *padapter;
	u8 tmp[80] = {0};
	u32 num;
	u16 val;
	u16 ret;
	int err;


	padapter = (_adapter *)rtw_netdev_priv(dev);

	if (NULL == buffer) {
		RTW_INFO(FUNC_ADPT_FMT ": input buffer is NULL!\n",
			 FUNC_ADPT_ARG(padapter));
		err = -EFAULT;
		goto exit;
	}

	if (count < 1) {
		RTW_INFO(FUNC_ADPT_FMT ": input length is 0!\n",
			 FUNC_ADPT_ARG(padapter));
		err = -EFAULT;
		goto exit;
	}

	num = count;
	if (num > (sizeof(tmp) - 1))
		num = (sizeof(tmp) - 1);

	if (copy_from_user(tmp, buffer, num)) {
		RTW_INFO(FUNC_ADPT_FMT ": copy buffer from user space FAIL!\n",
			 FUNC_ADPT_ARG(padapter));
		err = -EFAULT;
		goto exit;
	}

	err = btreg_parse_str(tmp, &btreg_write_type, &btreg_write_addr, &val);
	if (err)
		goto exit;

	RTW_INFO(FUNC_ADPT_FMT ": Set (%s)0x%X = 0x%x\n",
		FUNC_ADPT_ARG(padapter), btreg_type[btreg_write_type], btreg_write_addr, val);

	ret = rtw_btc_btreg_write(padapter, btreg_write_type, btreg_write_addr, val);
	if (!CHECK_STATUS_CODE_FROM_BT_MP_OPER_RET(ret, BT_STATUS_BT_OP_SUCCESS))
		err = ret;

exit:
	btreg_write_error = err;
#endif

	return count;
}

#endif /* CONFIG_BTC */


int proc_get_mac_addr(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	rtw_hal_dump_macaddr(m, adapter);
	return 0;
}

static int proc_get_skip_band(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	int bandskip;

	bandskip = RTW_GET_SCAN_BAND_SKIP(adapter);
	RTW_PRINT_SEL(m, "bandskip:0x%02x\n", bandskip);
	return 0;
}

static ssize_t proc_set_skip_band(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[6];
	u8 skip_band;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}
	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%hhu", &skip_band);

		if (num < 1)
			return -EINVAL;

		if (1 == skip_band)
			RTW_SET_SCAN_BAND_SKIP(padapter, BAND_24G);
		else if (2 == skip_band)
			RTW_SET_SCAN_BAND_SKIP(padapter, BAND_5G);
		else if (3 == skip_band)
			RTW_CLR_SCAN_BAND_SKIP(padapter, BAND_24G);
		else if (4 == skip_band)
			RTW_CLR_SCAN_BAND_SKIP(padapter, BAND_5G);
	}
	return count;

}

#ifdef CONFIG_RTW_ACS
#ifdef WKARD_ACS
/*****************************************************************
* NOTE:
* The user space will parse the content of the following files,
* please DO NOT change the format of the output!
******************************************************************/
static void rtw_acs_chan_info_dump(struct seq_file *m, _adapter *a)
{
	struct acs_priv *acs = adapter_to_acs(a);
	struct acs_result *acs_r = &(acs->result[0]);
	u8 idx = 0;

	for (idx = 0; idx < MAX_ACS_INFO; idx++) {
		if (!acs_r[idx].channel)
			break;

		if (idx != 0)
			RTW_PRINT_SEL(m, ",");
		RTW_PRINT_SEL(m, "%d:%d", acs_r[idx].channel,
			(acs_r[idx].score > 0) ? acs_r[idx].score : 0);
	}

	RTW_PRINT_SEL(m, "\n");
}

static void rtw_acs_info_dump(struct seq_file *m, _adapter *a)
{
	struct mlme_priv *pmlmepriv = &(a->mlmepriv);
	struct acs_priv *acs = adapter_to_acs(a);
	struct mlme_ext_priv *pmlmeext = &a->mlmeextpriv;
	struct acs_result *acs_r = &(acs->result[0]);
#ifdef RTW_MI_SHARE_BSS_LIST
	_queue *queue = &a->dvobj->scanned_queue;
#else
	_queue *queue = &pmlmepriv->scanned_queue;
#endif
	_list *plist, *phead;
	struct wlan_network *pnetwork;
	u32 idx = 0;

	/* dump env ap */
	_rtw_spinlock_bh(&(queue->lock));

	phead = get_list_head(queue);
	plist = get_next(phead);

	RTW_PRINT_SEL(m, "1. Environment APs\n");
	RTW_PRINT_SEL(m, "%5s  %-17s %-3s  %-4s %-8s  %-6s\n", "index", "bssid", "ch", "rssi", "bw", "offset");

	while (rtw_end_of_queue_search(phead, plist) == _FALSE) {
		pnetwork = LIST_CONTAINOR(plist, struct wlan_network, list);

		RTW_PRINT_SEL(m, "%5d  "MAC_FMT" %-3d  %-4d %-8s  %-6d\n",
			++idx,
			MAC_ARG(pnetwork->network.MacAddress),
			pnetwork->network.Configuration.DSConfig,
			pnetwork->network.PhyInfo.SignalStrength,
			ch_width_str(pnetwork->network.Configuration.Bw),
			pnetwork->network.Configuration.Offset);

		plist = get_next(plist);
	}
	_rtw_spinunlock_bh(&(queue->lock));

	/* dump acs result */
	RTW_PRINT_SEL(m, "2. ENV result\n");
	RTW_PRINT_SEL(m, "%7s %3s %5s %5s %7s %5s %5s\n", "channel", "clm", "noise", "count", "overlap", "ban", "score");

	for (idx = 0; idx < MAX_ACS_INFO; idx++) {
		if (!acs_r[idx].channel)
			break;

		RTW_PRINT_SEL(m, "%7d %3d %5d %5d %7s %5s %5d\n",
			acs_r[idx].channel,
			acs_r[idx].clm_ratio,
			acs_r[idx].noise,
			acs_r[idx].rx_count,
			(acs_r[idx].overlap) ? "TRUE" : "FALSE",
			is_acs_ban_channel(a, acs_r[idx].channel, pmlmeext->cur_bwmode) ? "TRUE" : "FALSE",
			acs_r[idx].score);
	}

	/* dump nhm result */
	RTW_PRINT_SEL(m, "3. NHM result\n");
	RTW_PRINT_SEL(m, "%7s %8s\n", "channel", "sum");

	for (idx = 0; idx < MAX_ACS_INFO; idx++) {
		if (!acs_r[idx].channel)
			break;

		RTW_PRINT_SEL(m, "%7d %8d [%.2d  %.2d  %.2d  %.2d  %.2d  %.2d  %.2d  %.2d  %.2d  %.2d  %.2d  %.2d]\n",
			acs_r[idx].channel, acs_r[idx].nhm_score,
			acs_r[idx].nhm_rpt[11], acs_r[idx].nhm_rpt[10],
			acs_r[idx].nhm_rpt[9], acs_r[idx].nhm_rpt[8],
			acs_r[idx].nhm_rpt[7], acs_r[idx].nhm_rpt[6],
			acs_r[idx].nhm_rpt[5], acs_r[idx].nhm_rpt[4],
			acs_r[idx].nhm_rpt[3], acs_r[idx].nhm_rpt[2],
			acs_r[idx].nhm_rpt[1], acs_r[idx].nhm_rpt[0]);
	}
}

#ifdef CONFIG_RTW_DACS
static void rtw_dacs_info_dump(struct seq_file *m, _adapter *a)
{
	struct acs_priv *acs = adapter_to_acs(a);
	struct mlme_ext_priv *pmlmeext = &a->mlmeextpriv;
	struct acs_result *acs_r = &(acs->dacs_result[0]);
	u32 idx = 0;

	/* dump acs result */
	RTW_PRINT_SEL(m, "1. ENV result\n");
	RTW_PRINT_SEL(m, "%7s %3s %5s %5s %7s %5s %5s\n", "channel", "clm", "noise", "count", "overlap", "ban", "score");

	for (idx = 0; idx < MAX_ACS_INFO; idx++) {
		if (!acs_r[idx].channel)
			break;

		RTW_PRINT_SEL(m, "%7d %3d %5d %5d %7s %5s %5d\n",
			acs_r[idx].channel,
			acs_r[idx].clm_ratio,
			acs_r[idx].noise,
			acs_r[idx].rx_count,
			(acs_r[idx].overlap) ? "TRUE" : "FALSE",
			is_acs_ban_channel(a, acs_r[idx].channel, pmlmeext->cur_bwmode) ? "TRUE" : "FALSE",
			acs_r[idx].score);
	}

	/* dump nhm result */
	RTW_PRINT_SEL(m, "2. NHM result\n");
	RTW_PRINT_SEL(m, "%7s %8s\n", "channel", "sum");

	for (idx = 0; idx < MAX_ACS_INFO; idx++) {
		if (!acs_r[idx].channel)
			break;

		RTW_PRINT_SEL(m, "%7d %8d [%.2d  %.2d  %.2d  %.2d  %.2d  %.2d  %.2d  %.2d  %.2d  %.2d  %.2d  %.2d]\n",
			acs_r[idx].channel, acs_r[idx].nhm_score,
			acs_r[idx].nhm_rpt[11], acs_r[idx].nhm_rpt[10],
			acs_r[idx].nhm_rpt[9], acs_r[idx].nhm_rpt[8],
			acs_r[idx].nhm_rpt[7], acs_r[idx].nhm_rpt[6],
			acs_r[idx].nhm_rpt[5], acs_r[idx].nhm_rpt[4],
			acs_r[idx].nhm_rpt[3], acs_r[idx].nhm_rpt[2],
			acs_r[idx].nhm_rpt[1], acs_r[idx].nhm_rpt[0]);
	}
}
#endif /* CONFIG_RTW_DACS */
#endif /* WKARD_ACS */

static int proc_get_chan_info(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	if (rtw_sitesurvey_condition_check(adapter, _FALSE) != SS_ALLOW)
		return -EPERM;

	rtw_acs_chan_info_dump(m, adapter);
	return 0;
}

static int proc_get_best_chan(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	if (rtw_sitesurvey_condition_check(adapter, _FALSE) != SS_ALLOW)
		return -EPERM;

	rtw_acs_info_dump(m, adapter);
	return 0;
}
#ifdef CONFIG_RTW_DACS
static int proc_get_dacs_chan_sts(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	if (rtw_sitesurvey_condition_check(adapter, _FALSE) != SS_ALLOW)
		return -EPERM;

	rtw_dacs_info_dump(m, adapter);
	return 0;
}
#endif /* CONFIG_RTW_DACS */

#if 1
static ssize_t proc_set_acs(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	char tmp[32], cmd[8];
	u32 val;
	struct acs_priv *acs = adapter_to_acs(padapter);

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		int num = sscanf(tmp, "%s %u", cmd, &val);

		if (num < 1)
			return count;

		if (num == 2) {
			if (strcmp("nhm", cmd) == 0)
				acs->use_nhm = val;
		} else if (num == 1) {
			if (strcmp("switch", cmd) == 0)
				acs_change_bss_chbw(padapter, _TRUE, 5);
		}
	}

	return count;
}
#else
static ssize_t proc_set_acs(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
#ifdef CONFIG_RTW_ACS_DBG
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[32];
	u8 acs_state = 0;
	u16 scan_ch_ms= 0, acs_scan_ch_ms = 0;
	u8 scan_type = SCAN_ACTIVE, igi= 0, bw = 0;
	u8 acs_scan_type = SCAN_ACTIVE, acs_igi= 0, acs_bw = 0;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}
	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%hhu %hhu %hu %hhx %hhu",
			&acs_state, &scan_type, &scan_ch_ms, &igi, &bw);

		if (num < 1)
			return -EINVAL;

		if (acs_state)
			rtw_acs_start(padapter);
		else
			rtw_acs_stop(padapter);
		num = num -1;

		if(num) {
			if (num-- > 0)
				acs_scan_type = scan_type;
			if (num-- > 0)
				acs_scan_ch_ms = scan_ch_ms;
			if (num-- > 0)
				acs_igi = igi;
			if (num-- > 0)
				acs_bw = bw;
			rtw_acs_adv_setting(padapter, acs_scan_type, acs_scan_ch_ms, acs_igi, acs_bw);
		}
	}
#endif /*CONFIG_RTW_ACS_DBG*/
	return count;
}
#endif
#endif /*CONFIG_RTW_ACS*/


static int proc_get_hal_spec(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	rtw_hw_dump_hal_spec(m, adapter_to_dvobj(adapter));
	return 0;
}

static int proc_get_hal_trx_mode(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = rtw_netdev_priv(dev);

	rtw_hal_dump_trx_mode(m, adapter);
	return 0;
}

static int proc_get_phy_cap(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

#ifdef CONFIG_80211N_HT
	rtw_dump_drv_phy_cap(m, adapter);
	rtw_get_dft_phy_cap(m, adapter);
#endif /* CONFIG_80211N_HT */
	return 0;
}

#ifdef CONFIG_SUPPORT_TRX_SHARED
#include "../../hal/hal_halmac.h"
static int proc_get_trx_share_mode(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	dump_trx_share_mode(m, adapter);
	return 0;
}
#endif

static int proc_dump_rsvd_page(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	rtw_hal_dump_rsvd_page(m, adapter, adapter->rsvd_page_offset, adapter->rsvd_page_num);
	return 0;
}
static ssize_t proc_set_rsvd_page_info(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[32];
	u8 page_offset, page_num;

	if (count < 2)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}
	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%hhu %hhu", &page_offset, &page_num);

		if (num < 2)
			return -EINVAL;
		padapter->rsvd_page_offset = page_offset;
		padapter->rsvd_page_num = page_num;
	}
	return count;
}

#ifdef CONFIG_SUPPORT_FIFO_DUMP
static int proc_dump_fifo(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	rtw_dump_fifo(m, adapter, adapter->fifo_sel, adapter->fifo_addr, adapter->fifo_size);
	return 0;
}
static ssize_t proc_set_fifo_info(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[32];
	u8 fifo_sel = 0;
	u32 fifo_addr = 0;
	u32 fifo_size = 0;

	if (count < 3)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}
	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%hhu %x %d", &fifo_sel, &fifo_addr, &fifo_size);

		if (num < 3)
			return -EINVAL;

		padapter->fifo_sel = fifo_sel;
		padapter->fifo_addr = fifo_addr;
		padapter->fifo_size = fifo_size;
	}
	return count;
}
#endif

#ifdef CONFIG_WOWLAN
int proc_dump_pattern_cam(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);
	int i;
	struct  rtl_wow_pattern context;

	for (i = 0 ; i < pwrpriv->wowlan_pattern_idx; i++) {
		rtw_wow_pattern_read_cam_ent(padapter, i, &context);
		rtw_dump_wow_pattern(m, &context, i);
	}

	return 0;
}
#endif

static int proc_get_napi_info(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv *pregistrypriv = &adapter->registrypriv;
	u8 napi = 0, gro = 0;
	u32 weight = 0;
	struct dvobj_priv *d;
	d = adapter_to_dvobj(adapter);


#ifdef CONFIG_RTW_NAPI
	if (pregistrypriv->en_napi) {
		napi = 1;
		weight = RTL_NAPI_WEIGHT;
	}

#ifdef CONFIG_RTW_GRO
	if (pregistrypriv->en_gro)
		gro = 1;
#endif /* CONFIG_RTW_GRO */
#endif /* CONFIG_RTW_NAPI */

	if (napi) {
		RTW_PRINT_SEL(m, "NAPI enable, weight=%d\n", weight);
#ifdef CONFIG_RTW_NAPI_DYNAMIC
		RTW_PRINT_SEL(m, "Dynamaic NAPI mechanism is on, current NAPI %s\n",
			      d->en_napi_dynamic ? "enable" : "disable");
		RTW_PRINT_SEL(m, "Dynamaic NAPI info:\n"
				 "\ttcp_rx_threshold = %d Mbps\n"
				 "\tcur_rx_tp = %d Mbps\n",
			      pregistrypriv->napi_threshold,
			      d->traffic_stat.cur_rx_tp);
#endif /* CONFIG_RTW_NAPI_DYNAMIC */
	} else {
		RTW_PRINT_SEL(m, "NAPI disable\n");
	}
	RTW_PRINT_SEL(m, "GRO %s\n", gro?"enable":"disable");

	return 0;

}

#ifdef CONFIG_RTW_NAPI_DYNAMIC
static ssize_t proc_set_napi_th(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv *registry = &adapter->registrypriv;
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
	_adapter *iface = NULL;
	char tmp[32] = {0};
	int thrshld = 0;
	int num = 0, i = 0;


	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	RTW_INFO("%s: Last threshold = %d Mbps\n", __FUNCTION__, registry->napi_threshold);


	for (i = 0; i < dvobj->iface_nums; i++) {
		iface = dvobj->padapters[i];
		if (iface) {
			if (buffer && !copy_from_user(tmp, buffer, count)) {
				registry = &iface->registrypriv;
				num = sscanf(tmp, "%d", &thrshld);
				if (num > 0) {
					if (thrshld > 0)
						registry->napi_threshold = thrshld;
				}
			}
		}
	}
	RTW_INFO("%s: New threshold = %d Mbps\n", __FUNCTION__, registry->napi_threshold);
	RTW_INFO("%s: Current RX throughput = %d Mbps\n",
		 __FUNCTION__, adapter_to_dvobj(adapter)->traffic_stat.cur_rx_tp);

	return count;
}
#endif /* CONFIG_RTW_NAPI_DYNAMIC */


ssize_t proc_set_dynamic_agg_enable(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[32];
	int enable = 0, i = 0;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
		_adapter *iface = NULL;
		int num = sscanf(tmp, "%d", &enable);

		if (num !=  1) {
			RTW_INFO("invalid parameter!\n");
			return count;
		}

		RTW_INFO("dynamic_agg_enable:%d\n", enable);

		for (i = 0; i < dvobj->iface_nums; i++) {
			iface = dvobj->padapters[i];
			if (iface)
				iface->registrypriv.dynamic_agg_enable = enable;
		}

	}

	return count;

}

static int proc_get_dynamic_agg_enable(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv *pregistrypriv = &adapter->registrypriv;

	RTW_PRINT_SEL(m, "dynamic_agg_enable:%d\n", pregistrypriv->dynamic_agg_enable);

	return 0;
}

#ifdef CONFIG_RTW_A4_STA
static int proc_get_wds_gptr(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = rtw_netdev_priv(dev);

	if (MLME_IS_STA(adapter) && MLME_IS_ASOC(adapter))
		dump_wgptr(m, adapter);

	return 0;
}
#endif

#ifdef CONFIG_RTW_MESH
static int proc_get_mesh_peer_sel_policy(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	dump_mesh_peer_sel_policy(m, adapter);

	return 0;
}

#if CONFIG_RTW_MESH_ACNODE_PREVENT
static int proc_get_mesh_acnode_prevent(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	if (MLME_IS_MESH(adapter))
		dump_mesh_acnode_prevent_settings(m, adapter);

	return 0;
}

static ssize_t proc_set_mesh_acnode_prevent(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[32];

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		struct mesh_peer_sel_policy *peer_sel_policy = &adapter->mesh_cfg.peer_sel_policy;
		u8 enable;
		u32 conf_timeout_ms;
		u32 notify_timeout_ms;
		int num = sscanf(tmp, "%hhu %u %u", &enable, &conf_timeout_ms, &notify_timeout_ms);

		if (num >= 1)
			peer_sel_policy->acnode_prevent = enable;
		if (num >= 2)
			peer_sel_policy->acnode_conf_timeout_ms = conf_timeout_ms;
		if (num >= 3)
			peer_sel_policy->acnode_notify_timeout_ms = notify_timeout_ms;
	}

	return count;
}
#endif /* CONFIG_RTW_MESH_ACNODE_PREVENT */

#if CONFIG_RTW_MESH_OFFCH_CAND
static int proc_get_mesh_offch_cand(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	if (MLME_IS_MESH(adapter))
		dump_mesh_offch_cand_settings(m, adapter);

	return 0;
}

static ssize_t proc_set_mesh_offch_cand(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[32];

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		struct mesh_peer_sel_policy *peer_sel_policy = &adapter->mesh_cfg.peer_sel_policy;
		u8 enable;
		u32 find_int_ms;
		int num = sscanf(tmp, "%hhu %u", &enable, &find_int_ms);

		if (num >= 1)
			peer_sel_policy->offch_cand = enable;
		if (num >= 2)
			peer_sel_policy->offch_find_int_ms = find_int_ms;
	}

	return count;
}
#endif /* CONFIG_RTW_MESH_OFFCH_CAND */

#if CONFIG_RTW_MESH_PEER_BLACKLIST
static int proc_get_mesh_peer_blacklist(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	if (MLME_IS_MESH(adapter)) {
		dump_mesh_peer_blacklist_settings(m, adapter);
		if (MLME_IS_ASOC(adapter))
			dump_mesh_peer_blacklist(m, adapter);
	}

	return 0;
}

static ssize_t proc_set_mesh_peer_blacklist(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[32];

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		struct mesh_peer_sel_policy *peer_sel_policy = &adapter->mesh_cfg.peer_sel_policy;
		u32 conf_timeout_ms;
		u32 blacklist_timeout_ms;
		int num = sscanf(tmp, "%u %u", &conf_timeout_ms, &blacklist_timeout_ms);

		if (num >= 1)
			peer_sel_policy->peer_conf_timeout_ms = conf_timeout_ms;
		if (num >= 2)
			peer_sel_policy->peer_blacklist_timeout_ms = blacklist_timeout_ms;
	}

	return count;
}
#endif /* CONFIG_RTW_MESH_PEER_BLACKLIST */

#if CONFIG_RTW_MESH_CTO_MGATE_BLACKLIST
static int proc_get_mesh_cto_mgate_require(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	if (MLME_IS_MESH(adapter))
		RTW_PRINT_SEL(m, "%u\n", adapter->mesh_cfg.peer_sel_policy.cto_mgate_require);

	return 0;
}

static ssize_t proc_set_mesh_cto_mgate_require(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[32];

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		struct mesh_peer_sel_policy *peer_sel_policy = &adapter->mesh_cfg.peer_sel_policy;
		u8 require;
		int num = sscanf(tmp, "%hhu", &require);

		if (num >= 1) {
			peer_sel_policy->cto_mgate_require = require;
			#if CONFIG_RTW_MESH_CTO_MGATE_CARRIER
			if (rtw_mesh_cto_mgate_required(adapter))
				rtw_netif_carrier_off(adapter->pnetdev);
			else
				rtw_netif_carrier_on(adapter->pnetdev);
			#endif
		}
	}

	return count;
}

static int proc_get_mesh_cto_mgate_blacklist(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	if (MLME_IS_MESH(adapter)) {
		dump_mesh_cto_mgate_blacklist_settings(m, adapter);
		if (MLME_IS_ASOC(adapter))
			dump_mesh_cto_mgate_blacklist(m, adapter);
	}

	return 0;
}

static ssize_t proc_set_mesh_cto_mgate_blacklist(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[32];

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		struct mesh_peer_sel_policy *peer_sel_policy = &adapter->mesh_cfg.peer_sel_policy;
		u32 conf_timeout_ms;
		u32 blacklist_timeout_ms;
		int num = sscanf(tmp, "%u %u", &conf_timeout_ms, &blacklist_timeout_ms);

		if (num >= 1)
			peer_sel_policy->cto_mgate_conf_timeout_ms = conf_timeout_ms;
		if (num >= 2)
			peer_sel_policy->cto_mgate_blacklist_timeout_ms = blacklist_timeout_ms;
	}

	return count;
}
#endif /* CONFIG_RTW_MESH_CTO_MGATE_BLACKLIST */

static int proc_get_mesh_networks(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	dump_mesh_networks(m, adapter);

	return 0;
}

static int proc_get_mesh_plink_ctl(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	if (MLME_IS_MESH(adapter))
		dump_mesh_plink_ctl(m, adapter);

	return 0;
}

static int proc_get_mesh_mpath(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	if (MLME_IS_MESH(adapter) && MLME_IS_ASOC(adapter))
		dump_mpath(m, adapter);

	return 0;
}

static int proc_get_mesh_mpp(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	if (MLME_IS_MESH(adapter) && MLME_IS_ASOC(adapter))
		dump_mpp(m, adapter);

	return 0;
}

static int proc_get_mesh_known_gates(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	if (MLME_IS_MESH(adapter))
		dump_known_gates(m, adapter);

	return 0;
}

#if CONFIG_RTW_MESH_DATA_BMC_TO_UC
static int proc_get_mesh_b2u_flags(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	if (MLME_IS_MESH(adapter))
		dump_mesh_b2u_flags(m, adapter);

	return 0;
}

static ssize_t proc_set_mesh_b2u_flags(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[32];

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		struct rtw_mesh_cfg *mcfg = &adapter->mesh_cfg;
		u8 msrc, mfwd;
		int num = sscanf(tmp, "%hhx %hhx", &msrc, &mfwd);

		if (num >= 1)
			mcfg->b2u_flags_msrc = msrc;
		if (num >= 2)
			mcfg->b2u_flags_mfwd = mfwd;
	}

	return count;
}
#endif /* CONFIG_RTW_MESH_DATA_BMC_TO_UC */

static int proc_get_mesh_stats(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	if (MLME_IS_MESH(adapter))
		dump_mesh_stats(m, adapter);

	return 0;
}

static int proc_get_mesh_gate_timeout(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	if (MLME_IS_MESH(adapter))
		RTW_PRINT_SEL(m, "%u factor\n",
			       adapter->mesh_cfg.path_gate_timeout_factor);

	return 0;
}

static ssize_t proc_set_mesh_gate_timeout(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[32];

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		struct rtw_mesh_cfg *mcfg = &adapter->mesh_cfg;
		u32 timeout;
		int num = sscanf(tmp, "%u", &timeout);

		if (num < 1)
			goto exit;

		mcfg->path_gate_timeout_factor = timeout;
	}

exit:
	return count;
}

static int proc_get_mesh_gate_state(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct rtw_mesh_cfg *mcfg = &adapter->mesh_cfg;
	u8 cto_mgate = 0;

	if (MLME_IS_MESH(adapter)) {
		if (rtw_mesh_is_primary_gate(adapter))
			RTW_PRINT_SEL(m, "PG\n");
		else if (mcfg->dot11MeshGateAnnouncementProtocol)
			RTW_PRINT_SEL(m, "G\n");
		else if (rtw_mesh_gate_num(adapter))
			RTW_PRINT_SEL(m, "C\n");
		else
			RTW_PRINT_SEL(m, "N\n");
	}

	return 0;
}

static int proc_get_peer_alive_based_preq(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter= (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv  *rp = &adapter->registrypriv;

	RTW_PRINT_SEL(m, "peer_alive_based_preq = %u\n",
		      rp->peer_alive_based_preq);

	return 0;
}

static ssize_t
proc_set_peer_alive_based_preq(struct file *file, const char __user *buffer,
			       size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv  *rp = &adapter->registrypriv;
	char tmp[8];
	int num = 0;
	u8 enable = 0;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (!buffer || copy_from_user(tmp, buffer, count))
		goto exit;

	num = sscanf(tmp, "%hhu", &enable);
	if (num !=  1) {
		RTW_ERR("%s: invalid parameter!\n", __FUNCTION__);
		goto exit;
	}

	if (enable > 1) {
		RTW_ERR("%s: invalid value!\n", __FUNCTION__);
		goto exit;
	}
	rp->peer_alive_based_preq = enable;

exit:
	return count;
}

#endif /* CONFIG_RTW_MESH */

#ifdef RTW_BUSY_DENY_SCAN
static int proc_get_scan_interval_thr(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter= (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv *rp = &adapter->registrypriv;


	RTW_PRINT_SEL(m, "scan interval threshold = %u ms\n",
		      rp->scan_interval_thr);

	return 0;
}

static ssize_t proc_set_scan_interval_thr(struct file *file,
				          const char __user *buffer,
				          size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter= (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv *rp = &adapter->registrypriv;
	char tmp[12];
	int num = 0;
	u32 thr = 0;


	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (!buffer || copy_from_user(tmp, buffer, count))
		goto exit;

	num = sscanf(tmp, "%u", &thr);
	if (num != 1) {
		RTW_ERR("%s: invalid parameter!\n", __FUNCTION__);
		goto exit;
	}

	rp->scan_interval_thr = thr;

	RTW_PRINT("%s: scan interval threshold = %u ms\n",
		  __FUNCTION__, rp->scan_interval_thr);

exit:
	return count;
}

#endif /* RTW_BUSY_DENY_SCAN */

static int proc_get_scan_deny(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter= (_adapter *)rtw_netdev_priv(dev);
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);

	RTW_PRINT_SEL(m, "scan_deny is %s\n", (dvobj->scan_deny == _TRUE) ? "enable":"disable");

	return 0;
}

static ssize_t proc_set_scan_deny(struct file *file, const char __user *buffer,
				size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
	char tmp[8];
	int num = 0;
	int enable = 0;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (!buffer || copy_from_user(tmp, buffer, count))
		goto exit;

	num = sscanf(tmp, "%d", &enable);
	if (num !=  1) {
		RTW_ERR("%s: invalid parameter!\n", __FUNCTION__);
		goto exit;
	}

	dvobj->scan_deny = enable ? _TRUE : _FALSE;

	RTW_PRINT("%s: scan_deny is %s\n",
		  __FUNCTION__, (dvobj->scan_deny == _TRUE) ? "enable":"disable");

exit:
	return count;
}

extern uint rtw_wmm_dm;
static int proc_get_wmm_dm(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter= (_adapter *)rtw_netdev_priv(dev);
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);

	RTW_PRINT_SEL(m, "WMM DM is %s (%u).\n",
				  (rtw_wmm_dm ? "enabled":"disabled"),
				  dvobj->wmm_mode);

	return 0;
}

static ssize_t proc_set_wmm_dm(struct file *file, const char __user *buffer,
							   size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
	char tmp[8];
	int num = 0;
	int enable = 0;

	if (count > sizeof(tmp)) {
		RTW_ERR("%s: invalid parameter,the parameter should be set 0(Disabled) or 1(Enabled)!\n", __FUNCTION__);
		return -EFAULT;
	}

	if (!buffer || copy_from_user(tmp, buffer, count))
		goto exit;

	num = sscanf(tmp, "%d", &enable);
	if (num !=	1) {
		RTW_ERR("%s: invalid parameter!\n", __FUNCTION__);
		goto exit;
	}

	rtw_wmm_dm = enable ? _TRUE : _FALSE;

	rtw_switch_wmm_mode(adapter);

	RTW_PRINT("%s: %s WMM DM (%u).\n",
		  __FUNCTION__, rtw_wmm_dm ? "Enabled":"Disabled",
		  dvobj->wmm_mode);

exit:
	return count;
}

int proc_get_cur_beacon_keys(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = rtw_netdev_priv(dev);
	struct mlme_priv *mlme = &adapter->mlmepriv;

	rtw_dump_bcn_keys(m, &mlme->cur_beacon_keys);

	return 0;
}

static int proc_get_dump_debug(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);


	RTW_PRINT_SEL(m, "TX path registers: \n");

	SHOW_REG32_MSG(adapter, R_AX_RXQ_RXBD_IDX, "RX_BD_IDX", m);
	SHOW_REG32_MSG(adapter, R_AX_RPQ_RXBD_IDX, "RP_BD_IDX", m);
	SHOW_REG32_MSG(adapter, R_AX_ACH0_TXBD_IDX, "ACH0 IDX", m);
	SHOW_REG32_MSG(adapter, R_AX_ACH0_PAGE_INFO, "ACH0 PG INFO", m);
	SHOW_REG16_MSG(adapter, R_AX_ACH0_BDRAM_RWPTR, "ACH0 BD RWPTR", m);
	SHOW_REG32_MSG(adapter, R_AX_ACH1_TXBD_IDX, "ACH1 IDX", m);
	SHOW_REG32_MSG(adapter, R_AX_ACH1_PAGE_INFO, "ACH1 PG INFO", m);
	SHOW_REG16_MSG(adapter, R_AX_ACH1_BDRAM_RWPTR, "ACH1 BD RWPTR", m);
	SHOW_REG32_MSG(adapter, R_AX_ACH2_TXBD_IDX, "ACH2 IDX", m);
	SHOW_REG32_MSG(adapter, R_AX_ACH2_PAGE_INFO, "ACH2 PG INFO", m);
	SHOW_REG16_MSG(adapter, R_AX_ACH2_BDRAM_RWPTR, "ACH2 BD RWPTR", m);
	SHOW_REG32_MSG(adapter, R_AX_ACH3_TXBD_IDX, "ACH3 IDX", m);
	SHOW_REG32_MSG(adapter, R_AX_ACH3_PAGE_INFO, "ACH3 PG INFO", m);
	SHOW_REG16_MSG(adapter, R_AX_ACH3_BDRAM_RWPTR, "ACH3 BD RWPTR", m);
	SHOW_REG32_MSG(adapter, R_AX_ACH4_TXBD_IDX, "ACH4 IDX", m);
	SHOW_REG32_MSG(adapter, R_AX_ACH4_PAGE_INFO, "ACH4 PG INFO", m);
	SHOW_REG16_MSG(adapter, R_AX_ACH4_BDRAM_RWPTR, "ACH4 BD RWPTR", m);
	SHOW_REG32_MSG(adapter, R_AX_ACH5_TXBD_IDX, "ACH5 IDX", m);
	SHOW_REG32_MSG(adapter, R_AX_ACH5_PAGE_INFO, "ACH5 PG INFO", m);
	SHOW_REG16_MSG(adapter, R_AX_ACH5_BDRAM_RWPTR, "ACH5 BD RWPTR", m);
	SHOW_REG32_MSG(adapter, R_AX_ACH6_TXBD_IDX, "ACH6 IDX", m);
	SHOW_REG32_MSG(adapter, R_AX_ACH6_PAGE_INFO, "ACH6 PG INFO", m);
	SHOW_REG16_MSG(adapter, R_AX_ACH6_BDRAM_RWPTR, "ACH6 BD RWPTR", m);
	SHOW_REG32_MSG(adapter, R_AX_ACH7_TXBD_IDX, "ACH7 IDX", m);
	SHOW_REG32_MSG(adapter, R_AX_ACH7_PAGE_INFO, "ACH7 PG INFO", m);
	SHOW_REG16_MSG(adapter, R_AX_ACH7_BDRAM_RWPTR, "ACH7 BD RWPTR", m);
	SHOW_REG32_MSG(adapter, R_AX_CH8_TXBD_IDX, "CH8 IDX", m);
	SHOW_REG32_MSG(adapter, R_AX_CH8_PAGE_INFO, "CH8 PG INFO", m);
	SHOW_REG16_MSG(adapter, R_AX_CH8_BDRAM_RWPTR, "CH8 BD RWPTR", m);
	SHOW_REG32_MSG(adapter, R_AX_CH9_TXBD_IDX, "CH9 IDX", m);
	SHOW_REG32_MSG(adapter, R_AX_CH9_PAGE_INFO, "CH9 PG INFO", m);
	SHOW_REG16_MSG(adapter, R_AX_CH9_BDRAM_RWPTR, "CH9 BD RWPTR", m);
	SHOW_REG32_MSG(adapter, R_AX_CH10_TXBD_IDX, "CH10 IDX", m);
	SHOW_REG32_MSG(adapter, R_AX_CH10_PAGE_INFO, "CH10 PG INFO", m);
	SHOW_REG16_MSG(adapter, R_AX_CH10_BDRAM_RWPTR, "CH10 BD RWPTR", m);
	SHOW_REG32_MSG(adapter, R_AX_CH11_TXBD_IDX, "CH11 IDX", m);
	SHOW_REG32_MSG(adapter, R_AX_CH11_PAGE_INFO, "CH11 PG INFO", m);
	SHOW_REG16_MSG(adapter, R_AX_CH11_BDRAM_RWPTR, "CH11 BD RWPTR", m);
	SHOW_REG32_MSG(adapter, R_AX_CH12_TXBD_IDX, "CH12 IDX", m);
	SHOW_REG32_MSG(adapter, R_AX_CH12_PAGE_INFO, "CH12 PG INFO", m);
	SHOW_REG16_MSG(adapter, R_AX_CH12_BDRAM_RWPTR, "CH12 BD RWPTR", m);
#ifdef R_AX_PCIE_DBG_CTRL
	SHOW_REG32_MSG(adapter, R_AX_PCIE_DBG_CTRL, "DBG_CTRL", m);
#else
	SHOW_REG32_MSG(adapter, 0x11C0, "DBG_CTRL", m);
#endif
	SHOW_REG32_MSG(adapter, R_AX_DBG_ERR_FLAG, "DBG_ERR", m);
	SHOW_REG32_MSG(adapter, R_AX_PCIE_HIMR00, "IMR0", m);
	SHOW_REG32_MSG(adapter, R_AX_PCIE_HISR00, "ISR0", m);
	SHOW_REG32_MSG(adapter, R_AX_PCIE_HIMR10, "IMR1", m);
	SHOW_REG32_MSG(adapter, R_AX_PCIE_HISR10, "IMR1", m);
	SHOW_REG32_MSG(adapter, R_AX_PCIE_DMA_STOP1, "DMA_STOP1", m);
	SHOW_REG32_MSG(adapter, R_AX_PCIE_DMA_BUSY1, "DMA_BUSY1", m);
	SHOW_REG32_MSG(adapter, R_AX_PCIE_DMA_STOP2, "DMA_STOP2", m);
	SHOW_REG32_MSG(adapter, R_AX_PCIE_DMA_BUSY2, "DMA_BUSY2", m);
	SHOW_REG32_MSG(adapter, R_AX_CTN_TXEN, "CTN_TXEN", m);
	SHOW_REG32_MSG(adapter, R_AX_WCPU_FW_CTRL, "R_AX_WCPU_FW_CTRL", m);

	SHOW_REG32_MSG(adapter, R_AX_CMAC_ERR_ISR, "CMAC_ERR", m);
	SHOW_REG32_MSG(adapter, R_AX_PTCL_COMMON_SETTING_0, "PTCL_COMMON_SETTING_0", m);
	SHOW_REG32_MSG(adapter, 0x11a24, "BB_CCA", m);

	SHOW_REG32_MSG(adapter, 0x11a64, "BB_LEGACY", m);
	SHOW_REG32_MSG(adapter, 0x11a60, "BB_HT", m);
	SHOW_REG32_MSG(adapter, 0x11a5C, "BB_VHT", m);
	SHOW_REG32_MSG(adapter, 0x11a58, "BB_HE", m);

	SHOW_REG32(adapter, 0x8840, m);
	SHOW_REG32(adapter, 0x8844, m);
	SHOW_REG32(adapter, 0x8854, m);
	SHOW_REG16(adapter, 0xCA22, m);
	SHOW_REG32(adapter, 0x8AA8, m);

	/* Show TX PPDU counters */
	do {
		int i;
		u32 reg32 = rtw_phl_read32(adapter->dvobj->phl, R_AX_TX_PPDU_CNT);

		RTW_PRINT_SEL(m, "CMAC0 TX PPDU Counters @%04X:\n", R_AX_TX_PPDU_CNT);

		reg32 &= ~(B_AX_PPDU_CNT_IDX_MSK << B_AX_PPDU_CNT_IDX_SH);
		for (i = 0; i < 11; i++) {
			rtw_phl_write32(adapter->dvobj->phl, R_AX_TX_PPDU_CNT,
						reg32 | (i << B_AX_PPDU_CNT_IDX_SH));
			RTW_PRINT_SEL(m, "	%02X: %d\n", i,
					 (
						(	rtw_phl_read32(adapter->dvobj->phl, R_AX_TX_PPDU_CNT)
						 >> B_AX_TX_PPDU_CNT_SH)
					  & B_AX_TX_PPDU_CNT_MSK));
		}
		// SCC for now
	} while (0);

	/* Show RX PPDU counters */
	_show_RX_counter(adapter, m);

	_show_TX_dbg_status(adapter, m);

	_show_BCN_dbg_status(adapter, m);


	return 0;
}

static int proc_get_dump_cnt(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	core_cmd_dump_cnt(adapter, m, 0);
	return 0;
}

#ifdef CONFIG_RTW_A4_STA
static int proc_get_a4(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	u32 idx = 0;

	RTW_PRINT_SEL(m, "a4_enable = %d \n", adapter->a4_enable);
	RTW_PRINT_SEL(m, "cnt_a4: tx = %d, txsc = %d, txsc_amsdu = %d \n", adapter->cnt_a4_tx,
			adapter->cnt_a4_txsc, adapter->cnt_a4_txsc_amsdu);
	RTW_PRINT_SEL(m, "cnt_a4: rx = %d, rxsc = %d, rxsc_amsdu = %d \n", adapter->cnt_a4_rx,
			adapter->cnt_a4_rxsc, adapter->cnt_a4_rxsc_amsdu);
	if (adapter->a4_enable) {
		cmd_a4_dump_sta(adapter, m);
		cmd_a4_dump_db(adapter, m);
	}

	return 0;
}
#endif

static int proc_get_record_trx(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	u32 dump_type = 0;

	core_dump_record(adapter, (u8)dump_type, m);

	return 0;
}

ssize_t proc_set_record_trx(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
	{
		char tmp[32] = {0};
		char cmd[16] = {0};

		if (count < 1)
			return -EINVAL;

		if (count > sizeof(tmp)) {
			rtw_warn_on(1);
			return -EFAULT;
		}

		if (buffer && !copy_from_user(tmp, buffer, count))
		{
			struct net_device *dev = data;
			_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

			int num = sscanf(tmp, "%s", cmd);

			if (num < 1)
				return count;

			//printk("count=%d, tmp=%s, cmd=%s\n", count, tmp, cmd);

			/* FS/LS debug */
			if(!strcmp(cmd, "start"))
			{
				u8 *log = NULL;
				log = (u8*)&adapter->core_logs;
				memset(log, 0, sizeof(struct core_logs));
				log = (u8*)&adapter->phl_logs;
				memset(log, 0, sizeof(struct phl_logs));
				adapter->record_enable = 1;
			}
			else if(!strcmp(cmd, "stop"))
			{
				adapter->record_enable = 0;
			}
		}
		else
			return -EFAULT;
		return count;
}

static int proc_get_rx_debug(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct dvobj_priv *dvobj = adapter->dvobj;

#ifdef DEBUG_PHL_RX
	rtw_phl_cmd_phl_rx_dump(adapter, m);
#endif
#ifdef RTW_RX_CPU_BALANCE
	if (dvobj->handlers[RTW_HANDLER_PHL_EVT])
		RTW_PRINT_SEL(m, "core_rx_cpuid=%u\n",
			dvobj->handlers[RTW_HANDLER_PHL_EVT]->cpu_id);
#endif
	RTW_PRINT_SEL(m, "-- core ----------\n");
#ifdef DEBUG_CORE_RX
	RTW_PRINT_SEL(m, "cnt_rx_pktsz =%u\n", dvobj->cnt_rx_pktsz);
	RTW_PRINT_SEL(m, "rx_pktsz_os  =%u\n", dvobj->num_rx_pktsz_os);
	RTW_PRINT_SEL(m, "rx_pkt_os    =%u\n", dvobj->total_rx_pkt_os);
#endif
#ifdef CONFIG_SMP_NETIF_RX
	RTW_PRINT_SEL(m, "netif_rx_cpuid=%u\n", dvobj->netif_rx_task.cpu_id);
	RTW_PRINT_SEL(m, "netif_rx_ring: write_idx=%u,read_idx=%u,full=%u\n",
		dvobj->netif_rx_ring.write_idx, dvobj->netif_rx_ring.read_idx,
		dvobj->netif_rx_ring.full);
#endif
#ifdef CONFIG_SMP_PHL_RX_RECYCLE
	RTW_PRINT_SEL(m, "rx_recycle_cpuid=%u\n", dvobj->rx_recycle_task.cpu_id);
	RTW_PRINT_SEL(m, "rx_recycle_ring: write_idx=%u,read_idx=%u,full=%u,max_time=%u\n",
		dvobj->rx_recycle_ring.write_idx, dvobj->rx_recycle_ring.read_idx,
		dvobj->rx_recycle_ring.full, dvobj->rx_recycle_ring.max_time);
#endif
	return 0;
}

ssize_t proc_set_rx_debug(struct file *file, const char __user *buffer,
			size_t count, loff_t *pos, void *data)
{
	struct net_device *netdev = (struct net_device *)data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(netdev);
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	struct rtw_proc_cmd cmd;

	char tmpbuf[100] = {0};
	char *pstr;
	char *token;
	u32 argc = 0;
	char argv[MAX_PRC_CMD][PROC_CMD_LEN];
	int i, value;

	if (count < 1)
		return -EINVAL;

	if (count >= sizeof(tmpbuf)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (!buffer || copy_from_user(tmpbuf, buffer, count))
		return -EFAULT;

	/* strip a trailing newline */
	if (tmpbuf[count -1] == '\n')
		tmpbuf[count -1] = '\0';

	/* parses the command-line arguments */
	pstr = tmpbuf;
	do {
		token = strsep(&pstr, ", ");
		if (token) {
			strncpy(argv[argc], token, PROC_CMD_LEN-1);
			argv[argc][PROC_CMD_LEN-1] = '\0';
			argc++;
		} else {
			break;
		}
	} while (argc < MAX_PRC_CMD);

	if (!strcmp(argv[0], "clear")) {
#ifdef DEBUG_PHL_RX
		cmd.in_type = RTW_ARG_TYPE_ARRAY;
		cmd.in_cnt_len = 2;
		strcpy(cmd.in.vector[0], "phl_rx");
		strcpy(cmd.in.vector[1], "clear");
		rtw_phl_proc_cmd(GET_HAL_INFO(dvobj),
				RTW_PROC_CMD_PHL, &cmd, tmpbuf, sizeof(tmpbuf));
#endif
#ifdef DEBUG_CORE_RX
		dvobj->num_rx_pktsz_os = 0;
		dvobj->total_rx_pkt_os = 0;
#endif
#ifdef CONFIG_SMP_NETIF_RX
		dvobj->netif_rx_ring.full = 0;
#endif
#ifdef CONFIG_SMP_PHL_RX_RECYCLE
		dvobj->rx_recycle_ring.full = 0;
		dvobj->rx_recycle_ring.max_time = 0;
#endif
	}
#ifdef DEBUG_CORE_RX
	else if (!strcmp(argv[0], "cnt_rx_pktsz")) {
		if (argc > 1 && sscanf(argv[1], "%d", &value) == 1)
			dvobj->cnt_rx_pktsz = value;
	}
#endif
#ifdef RTW_RX_CPU_BALANCE
	else if (!strcmp(argv[0], "core_rx_cpuid")) {
		if (dvobj->handlers[RTW_HANDLER_PHL_EVT] &&
			argc > 1 && sscanf(argv[1], "%d", &value) == 1)
			dvobj->handlers[RTW_HANDLER_PHL_EVT]->cpu_id = value;
	}
#endif
#ifdef CONFIG_SMP_NETIF_RX
	else if (!strcmp(argv[0], "netif_rx_cpuid")) {
		if (argc > 1 && sscanf(argv[1], "%d", &value) == 1)
			dvobj->netif_rx_task.cpu_id = value;
	}
#endif
#ifdef CONFIG_SMP_PHL_RX_RECYCLE
	else if (!strcmp(argv[0], "rx_recycle_cpuid")) {
		if (argc > 1 && sscanf(argv[1], "%d", &value) == 1)
			dvobj->rx_recycle_task.cpu_id = value;
	}
#endif
#ifdef DEBUG_PHL_RX
	else {
		if (!strcmp(argv[0], "cnt_phl_rx_pktsz"))
			strcpy(argv[0], "cnt_rx_pktsz");

		cmd.in_type = RTW_ARG_TYPE_ARRAY;
		cmd.in_cnt_len = 1;
		strcpy(cmd.in.vector[0], "phl_rx");
		for (i = 0; i < argc; i++) {
			strncpy(cmd.in.vector[cmd.in_cnt_len], argv[i], MAX_ARGV-1);
			cmd.in.vector[cmd.in_cnt_len][MAX_ARGV-1] = '\0';
			cmd.in_cnt_len++;
		}
		rtw_phl_proc_cmd(GET_HAL_INFO(dvobj),
				RTW_PROC_CMD_PHL, &cmd, tmpbuf, sizeof(tmpbuf));
		printk("%s", tmpbuf);
	}
#endif

	return count;
}

#ifdef DEBUG_PHL_RX
static int proc_get_debug_phl(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	rtw_phl_cmd_debug(adapter->dvobj->phl, m);

	return 0;
}
#endif

#ifdef CONFIG_CORE_TXSC
static int proc_get_txsc(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	RTW_PRINT_SEL(m, "[txsc] debug info\n");
	txsc_dump(adapter, m);

	return 0;
}

ssize_t proc_set_txsc(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
	{
		char tmp[32] = {0};
		char cmd[16] = {0};

		if (count < 1)
			return -EINVAL;

		if (count > sizeof(tmp)) {
			rtw_warn_on(1);
			return -EFAULT;
		}

		if (buffer && !copy_from_user(tmp, buffer, count))
		{
			struct net_device *dev = data;
			_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
			struct	mlme_priv *pmlmepriv = &adapter->mlmepriv;
			struct xmit_priv *pxmitpriv = &adapter->xmitpriv;
			u32 value = 0;

			int num = sscanf(tmp, "%s %d", cmd, &value);

			//printk("count=%d, num=%d, cmd=%s, value=%d\n", count, num, cmd, value);

			if (num < 1)
				return count;

			if (!strcmp(cmd, "enable"))
			{
				u8 old_value = pxmitpriv->txsc_enable;

				RTW_PRINT("[txsc] enable:%d\n", value);
				if (1) {//(check_fwstate(pmlmepriv, WIFI_AP_STATE)) {
					pxmitpriv->txsc_enable = value;
				#ifdef CONFIG_TXSC_AMSDU
					if(pxmitpriv->txsc_enable == 0)
						pxmitpriv->txsc_amsdu_enable = 0;
				#endif
				} else
					RTW_PRINT("[TXSC][WARNING] only AP mode support tx shortcut now !!\n");

				if (value != old_value)
					txsc_clear(adapter, 1);
			}
			else if(!strcmp(cmd, "clear"))
			{
				RTW_PRINT("[TXSC] clear shortcut\n");
				txsc_clear(adapter, 1);
			}
		}
		else
			return -EFAULT;
		return count;
}

#ifdef CONFIG_TXSC_AMSDU
static int proc_get_amsdu(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	RTW_PRINT_SEL(m, "[amsdu] show info\n");
	txsc_amsdu_dump(adapter, m);

	return 0;
}

ssize_t proc_set_amsdu(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
	{
		char tmp[32] = {0};
		char cmd[16] = {0};

		if (count < 1)
			return -EINVAL;

		if (count > sizeof(tmp)) {
			rtw_warn_on(1);
			return -EFAULT;
		}

		if (buffer && !copy_from_user(tmp, buffer, count))
		{
			struct net_device *dev = data;
			_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
			struct	mlme_priv *pmlmepriv = &adapter->mlmepriv;
			struct xmit_priv *pxmitpriv = &adapter->xmitpriv;
			u32 value = 0;
			u8 *ra = NULL;
			struct sta_info *psta = NULL;

			int num = sscanf(tmp, "%s %d", cmd, &value);

			//printk("count=%d, num=%d, cmd=%s, value=%d\n", count, num, cmd, value);

			if (num < 1)
				return count;

			if (!strcmp(cmd, "enable"))
			{
				RTW_PRINT("[amsdu] set txsc_amsdu_enable:%d\n", value);

				if (value == 1) {
					pxmitpriv->txsc_enable = 0;
					txsc_clear(adapter, 1);
					txsc_amsdu_clear(adapter);
					pxmitpriv->txsc_enable = 1;
					RTW_PRINT("[txsc] set txsc_enable:%d\n", pxmitpriv->txsc_enable);
				} else if(value == 0) {
					txsc_clear(adapter, 1);
					txsc_amsdu_clear(adapter);
				}

				pxmitpriv->txsc_amsdu_enable = value;
				#if 0
				if (pxmitpriv->txsc_amsdu_enable)
					rtw_phl_write32(adapter->dvobj->phl, 0x9b00, 0);
				else
					rtw_phl_write32(adapter->dvobj->phl, 0x9b00, 0x7);
				#endif
			}
			else if(!strcmp(cmd, "num"))
			{
				if (value > MAX_TXSC_SKB_NUM) {
					value = MAX_TXSC_SKB_NUM;
					RTW_PRINT("[amsdu] error !!! amsdu num should not over %d\n", MAX_TXSC_SKB_NUM);
				}

				RTW_PRINT("[amsdu] set tx_amsdu num:%d\n", value);
				adapter->tx_amsdu = value;
			}
			else if (!strcmp(cmd, "tp"))
			{
				RTW_PRINT("[amsdu] set tx_amsdu_rate:%d\n", value);
				adapter->tx_amsdu_rate = value;
			}
			else if(!strcmp(cmd, "reset"))
			{
				RTW_PRINT("[amsdu] reset amsdu cnt\n");
				txsc_amsdu_reset(adapter);
			}
			else if (!strcmp(cmd, "force_num"))
			{
				if (value > MAX_TXSC_SKB_NUM)
					value = MAX_TXSC_SKB_NUM;

				RTW_PRINT("[amsdu] set txsc_amsdu_force_num:%d\n", value);

				pxmitpriv->txsc_amsdu_force_num = value;
				/* A4_TXSC */
				if (MLME_IS_STA(adapter)) {
					ra = get_bssid(&adapter->mlmepriv);
					psta = rtw_get_stainfo(&adapter->stapriv, ra);
					if (psta) {
						if (value)
							psta->txsc_amsdu_num = pxmitpriv->txsc_amsdu_force_num;
						else
							psta->txsc_amsdu_num = adapter->tx_amsdu;
					}
				}
			}
			else if(!strcmp(cmd, "mode"))
			{
				RTW_PRINT("[amsdu] set amsdu mode = %d\n", value);
				pxmitpriv->txsc_amsdu_mode = value;
			}
		}
		else
			return -EFAULT;
		return count;
}
#endif/* CONFIG_TXSC_AMSDU */
#endif	/* CONFIG_CORE_TXSC */

#ifdef CONFIG_RTW_CORE_RXSC
static int proc_get_rxsc(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct core_logs *log = &adapter->core_logs;

	RTW_PRINT_SEL(m, "dump");
	RTW_PRINT_SEL(m, "enable_rxsc: %d \n", adapter->enable_rxsc);
	RTW_PRINT_SEL(m, "rxCnt_data: orig=%d shortcut=%d(ratio=%d)\n",
		log->rxCnt_data_orig, log->rxCnt_data_shortcut,
		log->rxCnt_data_shortcut*100/((log->rxCnt_data_orig+log->rxCnt_data_shortcut)?:1));

	return 0;
}

ssize_t proc_set_rxsc(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
	{
		char tmp[32] = {0};
		char cmd[16] = {0};

		if (count < 1)
			return -EINVAL;

		if (count > sizeof(tmp)) {
			rtw_warn_on(1);
			return -EFAULT;
		}

		if (buffer && !copy_from_user(tmp, buffer, count))
		{
			struct net_device *dev = data;
			_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

			int num = sscanf(tmp, "%s", cmd);

			//printk("count=%d, num=%d, cmd=%s\n", count, num, cmd);

			if (num < 1)
				return count;

			if (!strcmp(cmd, "enable"))
			{
				DBGP("enable");
				adapter->enable_rxsc = 1;
			}
			else if(!strcmp(cmd, "disable"))
			{
				DBGP("disable");
				adapter->enable_rxsc = 0;
			}
		}
		else
			return -EFAULT;
		return count;
}

#endif	/*CONFIG_RTW_CORE_RXSC*/

static int proc_get_bufcap(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	rtw_core_dump_buf_cap(adapter, m);

	return 0;
}


#ifdef CONFIG_RTW_80211K

#define NEIGHBOR_REPORT_PROC_LEN 100

enum nb_report_cmd {
	NB_RPT_NONE = 0,
	NB_RPT_ADD = 1,
	NB_RPT_DEL = 2,
	NB_RPT_DELALL = 3,
	NB_RPT_TAG = 4,
	NB_RPT_MOD_PREFER = 5,
	NB_RPT_MAX
};

ssize_t proc_set_neighbor_rpt(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *mlme = &adapter->mlmepriv;
	struct wlan_network  *cur_network = &(mlme->cur_network);
	struct roam_nb_info *nb_info = &mlme->nb_info;
	struct registry_priv  *rp = &adapter->registrypriv;
	struct nb_rpt_hdr report;
	int empty_slot;
	u8 error_code = 1;
	char tmp[NEIGHBOR_REPORT_PROC_LEN];
	char *tmpptr;
	char *tokptr;
	int command = 0;
	int i;
	u8 is_tag = 0;
	u8 preference = 0;

	_rtw_memset(&report, 0, sizeof(report));

	if (count >= NEIGHBOR_REPORT_PROC_LEN) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (!buffer || copy_from_user(tmp, buffer, count))
		goto end;

	if (!MLME_IS_AP(adapter)) {
		goto end;
	}

	tmp[count] = 0;
    tmpptr = tmp;
    tmpptr = strsep((char **)&tmpptr, "\n");
    tokptr = strsep((char **)&tmpptr, " ");
    if (!memcmp(tokptr, "add", 3))
		command = NB_RPT_ADD;
    else if (!memcmp(tokptr, "delall", 6))
		command = NB_RPT_DELALL;
    else if (!memcmp(tokptr, "del", 3))
		command = NB_RPT_DEL;
	else if (!memcmp(tokptr, "tag", 3))
		command = NB_RPT_TAG;
	else if (!memcmp(tokptr, "mod", 3))
		command = NB_RPT_MOD_PREFER;



    if (command) {
        if (command == NB_RPT_ADD
           || command == NB_RPT_DEL
           || command == NB_RPT_TAG
           || command == NB_RPT_MOD_PREFER) {
        	tokptr = strsep((char **)&tmpptr, " ");
        	if (sscanf(tokptr, MAC_SFMT, MAC_SARG(report.bssid)) != 6)
				goto end;
        }

        /*add*/
        if (command == NB_RPT_ADD) {
            tokptr = strsep((char **)&tmpptr," ");
            if (tokptr) {
            	if (sscanf(tokptr, "%u", &report.bss_info) != 1)
	                goto end;
            } else {
                goto end;
            }

            tokptr = strsep((char **)&tmpptr," ");
            if (tokptr) {
            	if(sscanf(tokptr, "%hhu", &report.reg_class) != 1)
            		goto end;
            } else {
                goto end;
            }

            tokptr = strsep((char **)&tmpptr," ");
            if (tokptr) {
            	if(sscanf(tokptr, "%hhu", &report.ch_num) != 1)
            		goto end;
            } else {
                goto end;
            }

            tokptr = strsep((char **)&tmpptr," ");
            if (tokptr) {
            	if(sscanf(tokptr, "%hhu", &report.phy_type) != 1)
            		goto end;
            } else {
                goto end;
            }

            tokptr = strsep((char **)&tmpptr," ");
            if (tokptr) {
            	if(sscanf(tokptr, "%hhu", &report.preference) != 1)
            		goto end;
            } else {
                goto end;
            }

			i = rtw_rm_insert_nb_report(adapter, report);
			if(i == 0xff)
				goto end;
            RTW_INFO("Insert neighbor report into idx %d\n",i);
        } else if (command == NB_RPT_DEL) {
        	/*delete*/
        	if (_TRUE == _rtw_memcmp(report.bssid, cur_network->network.MacAddress, MAC_ADDR_LEN))
        		goto end;

            for (i = 0 ; i < RTW_MAX_NB_RPT_NUM; i++) {
                if(!nb_info->nb_rpt[i].enable)
                    continue;

                if (_TRUE == _rtw_memcmp(report.bssid, nb_info->nb_rpt[i].bssid, MAC_ADDR_LEN)) {
                    nb_info->nb_rpt[i].enable = 0;
                    nb_info->nb_rpt_num -= 1;
                    break;
                }
            }
        } else if (command == NB_RPT_DELALL) {
        	/*delete all*/
            for (i = 0 ; i < RTW_MAX_NB_RPT_NUM; i++) {
            	if (_TRUE == _rtw_memcmp(nb_info->nb_rpt[i].bssid, cur_network->network.MacAddress, MAC_ADDR_LEN))
            		continue;
            	nb_info->nb_rpt[i].enable = 0;
            }
            nb_info->nb_rpt_num = 0;
        } else if (command == NB_RPT_TAG) {
			/*tag specifi BSSID for nect BTM request use*/
			tokptr = strsep((char **)&tmpptr," ");
			if (tokptr) {
				if (sscanf(tokptr, "%hhu", &is_tag) != 1)
					goto end;
			} else {
				goto end;
			}

			for (i = 0 ; i < RTW_MAX_NB_RPT_NUM; i++) {
				if(!nb_info->nb_rpt[i].enable)
				continue;

				if (_TRUE == _rtw_memcmp(report.bssid, nb_info->nb_rpt[i].bssid, MAC_ADDR_LEN)) {
					if (is_tag && nb_info->nb_rpt[i].tag == 0) {
						nb_info->nb_rpt[i].tag = 1;
						nb_info->nb_rpt_tag += 1;
					} else if (!is_tag && nb_info->nb_rpt[i].tag == 1) {
						nb_info->nb_rpt[i].tag = 0;
						if(nb_info->nb_rpt_tag)
							nb_info->nb_rpt_tag -= 1;
					}
					break;
				}
			}
		} else if (command == NB_RPT_MOD_PREFER) {
			/*tag specifi BSSID for nect BTM request use*/
			tokptr = strsep((char **)&tmpptr," ");
			if (tokptr) {
				if (sscanf(tokptr, "%hhu", &preference) != 1)
					goto end;
			} else {
				goto end;
			}

			for (i = 0 ; i < RTW_MAX_NB_RPT_NUM; i++) {
				if(!nb_info->nb_rpt[i].enable)
				continue;

				if (_TRUE == _rtw_memcmp(report.bssid, nb_info->nb_rpt[i].bssid, MAC_ADDR_LEN)) {
					nb_info->nb_rpt[i].preference = preference;
					break;
				}
			}
        }
    } else {
        error_code = 1;
        goto end;
    }

    error_code = 0;

end:
    if (error_code == 1) {
        RTW_INFO("\nWarning: invalid command! Format:\n");
        RTW_INFO("add <bssid> <bss_info> <reg_class> <CH> <phy_type> <preference>\n");
        RTW_INFO("del <bssid>\n");
        RTW_INFO("delall\n");
		RTW_INFO("tag <bssid> <0/1>\n");
		RTW_INFO("mod <bssid> <preference>\n");
    } else if(error_code == 2) {
        RTW_INFO("\nwarning: neighbor report table full!\n");
    }
    return count;
}

int proc_get_neighbor_rpt(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *mlme = &adapter->mlmepriv;
	struct roam_nb_info *nb_info = &mlme->nb_info;
	int i;

	RTW_PRINT_SEL(m, "Neighbor report table:\n");
	RTW_PRINT_SEL(m, "[tag]\tIdx\tbssid\t\tbssinfo\topclass\tchannel\tphy\tpreference\n");
	for (i = 0; i < RTW_MAX_NB_RPT_NUM; i++) {
		if(nb_info->nb_rpt[i].enable) {
			RTW_PRINT_SEL(m, "[%s]%d\t"MAC_FMT"\t0x%x\t%u\t%u\t%u\t%u\n",
					nb_info->nb_rpt[i].tag?"V":" ",
					i,
					MAC_ARG(nb_info->nb_rpt[i].bssid),
					nb_info->nb_rpt[i].bss_info,
					nb_info->nb_rpt[i].reg_class,
					nb_info->nb_rpt[i].ch_num,
					nb_info->nb_rpt[i].phy_type,
					nb_info->nb_rpt[i].preference);
		}
	}

	return 0;

}
#endif /* CONFIG_RTW_80211K */

#ifdef CONFIG_ADPTVTY_CONTROL
static int proc_get_adptvty_info(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct sta_priv *pstapriv = &(padapter->stapriv);
	struct wifi_mib_priv *mib = &padapter->registrypriv.wifi_mib;
	struct adptvty_info *adpt = &(padapter->adpt_info);
	u32 scan_ap_num = 0;

	if (!MLME_IS_AP(padapter)) {
		RTW_PRINT_SEL(m, "Not a AP, return.\n");
		return 0;
	}

#ifdef RTW_MI_SHARE_BSS_LIST
	scan_ap_num = padapter->dvobj->num_of_scanned;
#else
	scan_ap_num = padapter->mlmepriv->num_of_scanned;
#endif /*RTW_MI_SHARE_BSS_LIST*/

	RTW_PRINT_SEL(m, "==== [%s] ====\n", dev->name);
	RTW_PRINT_SEL(m, "adptvty_en     : %u\n", mib->adptvty_en);
	RTW_PRINT_SEL(m, "adptvty_try    : %u\n", mib->adptvty_try);
	RTW_PRINT_SEL(m, "adptvty_th_t   : %u\n", mib->adptvty_th_t);
	RTW_PRINT_SEL(m, "adptvty_th_u   : %u\n", mib->adptvty_th_u);
	RTW_PRINT_SEL(m, "adptvty_ratio_u: %u\n", mib->adptvty_ratio_u);
	RTW_PRINT_SEL(m, "\n");

	RTW_PRINT_SEL(m, "asoc_sta_num    : %u\n", pstapriv->asoc_list_cnt);
	RTW_PRINT_SEL(m, "scan_ap_num     : %u\n", scan_ap_num);
	RTW_PRINT_SEL(m, "txop            : 0x%x\n", rtw_phl_read32(padapter->dvobj->phl, 0xC300));
	RTW_PRINT_SEL(m, "adpt_try        : %u\n", adpt->try);
	RTW_PRINT_SEL(m, "adpt_cnt_t      : %u\n", adpt->cnt_t);
	RTW_PRINT_SEL(m, "adpt_cnt_u      : %u\n", adpt->cnt_u);
	RTW_PRINT_SEL(m, "adpt_rto_u      : %u\n", adpt->rto_u);
	RTW_PRINT_SEL(m, "adptvty_test_cnt: %u\n", adpt->adptvty_test_cnt);
	RTW_PRINT_SEL(m, "adptvty_test    : %u\n", adpt->adptvty_test);
	RTW_PRINT_SEL(m, "tp_test_cnt     : %u\n", adpt->tp_test_cnt);
	RTW_PRINT_SEL(m, "tp_test         : %u\n", adpt->tp_test);

	return 0;
}
#endif /* CONFIG_ADPTVTY_CONTROL */

/*
* rtw_adapter_proc:
* init/deinit when register/unregister net_device
*/
const struct rtw_proc_hdl adapter_proc_hdls[] = {
#if RTW_SEQ_FILE_TEST
	RTW_PROC_HDL_SEQ("seq_file_test", &seq_file_test, NULL),
#endif
	RTW_PROC_HDL_SSEQ("write_reg", NULL, proc_set_write_reg),
	RTW_PROC_HDL_SSEQ("read_reg", proc_get_read_reg, proc_set_read_reg),

	RTW_PROC_HDL_SSEQ("mac_dbg_status_dump", NULL, proc_set_mac_dbg_status_dump),

	RTW_PROC_HDL_SSEQ("tx_rate_bmp", proc_get_dump_tx_rate_bmp, NULL),
	RTW_PROC_HDL_SSEQ("adapters_status", proc_get_dump_adapters_status, NULL),
#ifdef CONFIG_RTW_CUSTOMER_STR
	RTW_PROC_HDL_SSEQ("customer_str", proc_get_customer_str, NULL),
#endif
	RTW_PROC_HDL_SSEQ("fwstate", proc_get_fwstate, NULL),
	RTW_PROC_HDL_SSEQ("sec_info", proc_get_sec_info, NULL),
	RTW_PROC_HDL_SSEQ("mlmext_state", proc_get_mlmext_state, NULL),
	RTW_PROC_HDL_SSEQ("qos_option", proc_get_qos_option, NULL),
	RTW_PROC_HDL_SSEQ("ht_option", proc_get_ht_option, NULL),
	RTW_PROC_HDL_SSEQ("rf_info", proc_get_rf_info, NULL),
	RTW_PROC_HDL_SSEQ("curr_bw", proc_get_curr_bw, NULL),
	RTW_PROC_HDL_SSEQ("scan_param", proc_get_scan_param, proc_set_scan_param),
	RTW_PROC_HDL_SSEQ("scan_abort", proc_get_scan_abort, NULL),
#ifdef CONFIG_SCAN_BACKOP
	RTW_PROC_HDL_SSEQ("backop_flags_sta", proc_get_backop_flags_sta, proc_set_backop_flags_sta),
	#ifdef CONFIG_AP_MODE
	RTW_PROC_HDL_SSEQ("backop_flags_ap", proc_get_backop_flags_ap, proc_set_backop_flags_ap),
	#endif
	#ifdef CONFIG_RTW_MESH
	RTW_PROC_HDL_SSEQ("backop_flags_mesh", proc_get_backop_flags_mesh, proc_set_backop_flags_mesh),
	#endif
#endif
	RTW_PROC_HDL_SSEQ("survey_info", proc_get_survey_info, proc_set_survey_info),
#ifdef CONFIG_RTW_MULTI_AP
	RTW_PROC_HDL_SSEQ("map_block_list", proc_get_map_block_list, NULL),
#endif
#ifdef CONFIG_RTW_OPCLASS_CHANNEL_SCAN
	RTW_PROC_HDL_SSEQ("opclass_channel_scan", proc_get_opclass_channel_scan_info, NULL),
#endif
#ifdef CONFIG_BAND_STEERING
	RTW_PROC_HDL_SSEQ("band_steering_block_list", proc_get_band_steering_block_list, NULL),
#endif
	RTW_PROC_HDL_SSEQ("ap_info", proc_get_ap_info, NULL),
#ifdef ROKU_PRIVATE
	RTW_PROC_HDL_SSEQ("infra_ap", proc_get_infra_ap, NULL),
#endif /* ROKU_PRIVATE */
	RTW_PROC_HDL_SSEQ("trx_info", proc_get_trx_info, proc_reset_trx_info),
	RTW_PROC_HDL_SSEQ("tx_power_offset", proc_get_tx_power_offset, proc_set_tx_power_offset),
	RTW_PROC_HDL_SSEQ("rate_ctl", proc_get_rate_ctl, proc_set_rate_ctl),
	RTW_PROC_HDL_SSEQ("bw_ctl", proc_get_bw_ctl, proc_set_bw_ctl),
	RTW_PROC_HDL_SSEQ("mac_qinfo", proc_get_mac_qinfo, NULL),
	/*RTW_PROC_HDL_SSEQ("macid_info", proc_get_macid_info, NULL), */
	/* RTW_PROC_HDL_SSEQ("bcmc_info", proc_get_mi_ap_bc_info, NULL), */
	RTW_PROC_HDL_SSEQ("sec_cam", proc_get_sec_cam, proc_set_sec_cam),
	RTW_PROC_HDL_SSEQ("sec_cam_cache", proc_get_sec_cam_cache, NULL),
#ifdef CONFIG_DBG_AX_CAM
	RTW_PROC_HDL_SSEQ("dump_ax_valid_key", proc_get_ax_valid_key, NULL),
	RTW_PROC_HDL_SSEQ("dump_ax_address_cam", proc_get_ax_address_cam, NULL),
	RTW_PROC_HDL_SSEQ("dump_ax_security_cam", proc_get_ax_sec_cam, NULL),
#endif

	RTW_PROC_HDL_SSEQ("ps_dbg_info", proc_get_ps_dbg_info, proc_set_ps_dbg_info),
	RTW_PROC_HDL_SSEQ("wifi_spec", proc_get_wifi_spec, NULL),
#ifdef CONFIG_LAYER2_ROAMING
	RTW_PROC_HDL_SSEQ("roam_flags", proc_get_roam_flags, proc_set_roam_flags),
	RTW_PROC_HDL_SSEQ("roam_param", proc_get_roam_param, proc_set_roam_param),
	RTW_PROC_HDL_SSEQ("roam_tgt_addr", NULL, proc_set_roam_tgt_addr),
#endif /* CONFIG_LAYER2_ROAMING */

#ifdef CONFIG_RTW_80211R
	RTW_PROC_HDL_SSEQ("ft_flags", proc_get_ft_flags, proc_set_ft_flags),
#endif
	RTW_PROC_HDL_SSEQ("defs_param", proc_get_defs_param, proc_set_defs_param),
#ifdef CONFIG_SDIO_HCI
	RTW_PROC_HDL_SSEQ("sd_f0_reg_dump", proc_get_sd_f0_reg_dump, NULL),
	RTW_PROC_HDL_SSEQ("sdio_local_reg_dump", proc_get_sdio_local_reg_dump, NULL),
	RTW_PROC_HDL_SSEQ("sdio_card_info", proc_get_sdio_card_info, NULL),
#ifdef DBG_SDIO
	RTW_PROC_HDL_SSEQ("sdio_dbg", proc_get_sdio_dbg, proc_set_sdio_dbg),
#endif /* DBG_SDIO */
#endif /* CONFIG_SDIO_HCI */

	RTW_PROC_HDL_SSEQ("del_rx_ampdu_test_case", NULL, proc_set_del_rx_ampdu_test_case),
	RTW_PROC_HDL_SSEQ("wait_hiq_empty", NULL, proc_set_wait_hiq_empty),
	RTW_PROC_HDL_SSEQ("sta_linking_test", NULL, proc_set_sta_linking_test),
#ifdef CONFIG_AP_MODE
	RTW_PROC_HDL_SSEQ("ap_linking_test", NULL, proc_set_ap_linking_test),
#endif

	RTW_PROC_HDL_SSEQ("mac_reg_dump", proc_get_mac_reg_dump, NULL),
	RTW_PROC_HDL_SSEQ("bb_reg_dump", proc_get_bb_reg_dump, NULL),
	RTW_PROC_HDL_SSEQ("bb_reg_dump_ex", proc_get_bb_reg_dump_ex, NULL),
	RTW_PROC_HDL_SSEQ("rf_reg_dump", proc_get_rf_reg_dump, NULL),

#ifdef CONFIG_RTW_LED
	RTW_PROC_HDL_SSEQ("led_config", proc_get_led_config, proc_set_led_config),
#endif

#ifdef CONFIG_AP_MODE
	RTW_PROC_HDL_SSEQ("aid_status", proc_get_aid_status, proc_set_aid_status),
#ifdef CONFIG_RTW_AP_EXT_SUPPORT
	RTW_PROC_HDL_SSEQ("all_sta_info", proc_get_all_sta_info, NULL),
	RTW_PROC_HDL_SSEQ("sta_info", proc_get_sta_info, NULL),
	RTW_PROC_HDL_SSEQ("mib_staconfig", proc_get_mib_staconfig, NULL),
	RTW_PROC_HDL_SSEQ("mac_info", proc_get_mac_info, NULL),
	RTW_PROC_HDL_SSEQ("rf_para_info", proc_get_rf_para_info, NULL),
	RTW_PROC_HDL_SSEQ("temperature", proc_get_temperature, NULL),
#ifdef CONFIG_APPEND_VENDOR_IE_ENABLE
	RTW_PROC_HDL_SSEQ("ap_probereq_info", proc_get_ap_probereq_info, NULL),
	RTW_PROC_HDL_SSEQ("user_ie_info", proc_get_vsie_info, NULL),
#endif
#ifdef CONFIG_ONE_TXQ
	RTW_PROC_HDL_SSEQ("txq_info", proc_get_txq_info, NULL),
#endif
#ifdef MONITOR_UNASSOC_STA
	RTW_PROC_HDL_SSEQ("monitor_sta_info", proc_get_monitor_sta_info, NULL),
#endif
#ifdef RTW_BLOCK_STA_CONNECT
	RTW_PROC_HDL_SSEQ("block_sta_info", proc_get_block_sta_info, NULL),
#endif
	RTW_PROC_HDL_SSEQ("stats", proc_get_stats, NULL),
	RTW_PROC_HDL_SSEQ("mib_operation", proc_get_mib_operation, NULL),
	RTW_PROC_HDL_SSEQ("mib_wsc", proc_get_mib_wsc, NULL),
	RTW_PROC_HDL_SSEQ("led", proc_get_led, proc_set_led),
	RTW_PROC_HDL_SSEQ("led_interval", NULL, proc_set_led_interval),
#if defined(CONFIG_RTL_CFG80211_WAPI_SUPPORT)
	RTW_PROC_HDL_SSEQ("wapi_gcm_sm4_test",NULL,proc_set_wapi_gcm_sm4_test),
#endif
#ifdef CONFIG_RTW_MANUAL_EDCA
	RTW_PROC_HDL_SSEQ("mib_edca", proc_get_edca, NULL),
#endif
#ifdef SBWC
	RTW_PROC_HDL_SSEQ("mib_sbwc", proc_sbwc, NULL),
#endif
#ifdef GBWC
	RTW_PROC_HDL_SSEQ("mib_gbwc", proc_gbwc, NULL),
#endif
#ifdef CONFIG_CORE_TXSC
	RTW_PROC_HDL_SSEQ("txsc_ctl", proc_get_txsc_ctl, proc_set_txsc_ctl),
#endif
#endif
	RTW_PROC_HDL_SSEQ("bmc_tx_rate", proc_get_bmc_tx_rate, proc_set_bmc_tx_rate),
#endif /* CONFIG_AP_MODE */

#ifdef DBG_MEMORY_LEAK
	RTW_PROC_HDL_SSEQ("_malloc_cnt", proc_get_malloc_cnt, NULL),
#endif /* DBG_MEMORY_LEAK */

#ifdef CONFIG_FIND_BEST_CHANNEL
	RTW_PROC_HDL_SSEQ("best_channel", proc_get_best_channel, proc_set_best_channel),
#endif
#ifdef CONFIG_RTW_2G_40M_COEX
	RTW_PROC_HDL_SSEQ("coex_2g_40m", proc_get_coex_2g_40m, proc_set_coex_2g_40m),
#endif
	RTW_PROC_HDL_SSEQ("rx_signal", proc_get_rx_signal, proc_set_rx_signal),
	RTW_PROC_HDL_SSEQ("rx_chk_limit", proc_get_rx_chk_limit, proc_set_rx_chk_limit),
	RTW_PROC_HDL_SSEQ("hw_info", proc_get_hw_status, proc_set_hw_status),
	RTW_PROC_HDL_SSEQ("mac_rptbuf", proc_get_mac_rptbuf, NULL),
#ifdef CONFIG_80211N_HT
	RTW_PROC_HDL_SSEQ("ht_enable", proc_get_ht_enable, proc_set_ht_enable),
	RTW_PROC_HDL_SSEQ("bw_mode", proc_get_bw_mode, proc_set_bw_mode),
	RTW_PROC_HDL_SSEQ("ampdu_enable", proc_get_ampdu_enable, proc_set_ampdu_enable),
	RTW_PROC_HDL_SSEQ("rx_ampdu", proc_get_rx_ampdu, proc_set_rx_ampdu),
	RTW_PROC_HDL_SSEQ("rx_ampdu_size_limit", proc_get_rx_ampdu_size_limit, proc_set_rx_ampdu_size_limit),
	RTW_PROC_HDL_SSEQ("rx_ampdu_agg_num", proc_get_rx_ampdu_factor, proc_set_rx_ampdu_factor),
	RTW_PROC_HDL_SSEQ("rx_ampdu_density", proc_get_rx_ampdu_density, proc_set_rx_ampdu_density),
	RTW_PROC_HDL_SSEQ("rx_amsdu", proc_get_rx_amsdu, proc_set_rx_amsdu),
	RTW_PROC_HDL_SSEQ("rx_amsdu_size", proc_get_rx_amsdu_size, proc_set_rx_amsdu_size),
	RTW_PROC_HDL_SSEQ("tx_ampdu_density", proc_get_tx_ampdu_density, proc_set_tx_ampdu_density),
	RTW_PROC_HDL_SSEQ("tx_ampdu_agg_num", proc_get_tx_max_agg_num, proc_set_tx_max_agg_num),
	RTW_PROC_HDL_SSEQ("tx_quick_addba_req", proc_get_tx_quick_addba_req, proc_set_tx_quick_addba_req),
#ifdef CONFIG_TX_AMSDU
	RTW_PROC_HDL_SSEQ("tx_amsdu_num", proc_get_tx_amsdu, proc_set_tx_amsdu),
	RTW_PROC_HDL_SSEQ("tx_amsdu_rate", proc_get_tx_amsdu_rate, proc_set_tx_amsdu_rate),
#endif
#endif /* CONFIG_80211N_HT */
	RTW_PROC_HDL_SSEQ("dynamic_rrsr", proc_get_dyn_rrsr, proc_set_dyn_rrsr),
	RTW_PROC_HDL_SSEQ("en_fwps", proc_get_en_fwps, proc_set_en_fwps),

	/* RTW_PROC_HDL_SSEQ("path_rssi", proc_get_two_path_rssi, NULL),
	* 	RTW_PROC_HDL_SSEQ("rssi_disp",proc_get_rssi_disp, proc_set_rssi_disp), */

#ifdef CONFIG_BTC
	RTW_PROC_HDL_SSEQ("btc_dbg", proc_get_btc_dbg, proc_set_btc_dbg),
	RTW_PROC_HDL_SSEQ("btc", proc_get_btc_info, NULL),
	RTW_PROC_HDL_SSEQ("btreg_read", proc_get_btreg_read, proc_set_btreg_read),
	RTW_PROC_HDL_SSEQ("btreg_write", proc_get_btreg_write, proc_set_btreg_write),
#endif /* CONFIG_BTC */

#if defined(DBG_CONFIG_ERROR_DETECT)
	RTW_PROC_HDL_SSEQ("sreset", proc_get_sreset, proc_set_sreset),
#endif /* DBG_CONFIG_ERROR_DETECT */
	RTW_PROC_HDL_SSEQ("trx_info_debug", proc_get_trx_info_debug, NULL),

#ifdef CONFIG_HUAWEI_PROC
	RTW_PROC_HDL_SSEQ("huawei_trx_info", proc_get_huawei_trx_info, NULL),
#endif
	RTW_PROC_HDL_SSEQ("rtw_hal_linked_info_dump", proc_get_linked_info_dump, proc_set_linked_info_dump),
	RTW_PROC_HDL_SSEQ("sta_tp_dump", proc_get_sta_tp_dump, proc_set_sta_tp_dump),
	RTW_PROC_HDL_SSEQ("sta_tp_info", proc_get_sta_tp_info, NULL),
	RTW_PROC_HDL_SSEQ("tp_info", proc_get_whole_tp, NULL),
	RTW_PROC_HDL_SSEQ("dis_turboedca", proc_get_turboedca_ctrl, proc_set_turboedca_ctrl),
	RTW_PROC_HDL_SSEQ("tx_info_msg", proc_get_tx_info_msg, NULL),
	RTW_PROC_HDL_SSEQ("rx_info_msg", proc_get_rx_info_msg, proc_set_rx_info_msg),

#if defined(CONFIG_LPS_PG) && defined(CONFIG_RTL8822C)
	RTW_PROC_HDL_SSEQ("lps_pg_debug", proc_get_lps_pg_debug, NULL),
#endif

#ifdef CONFIG_GPIO_API
	RTW_PROC_HDL_SSEQ("gpio_info", proc_get_gpio, proc_set_gpio),
	RTW_PROC_HDL_SSEQ("gpio_set_output_value", NULL, proc_set_gpio_output_value),
	RTW_PROC_HDL_SSEQ("gpio_set_direction", NULL, proc_set_config_gpio),
#endif

#ifdef CONFIG_DBG_COUNTER
	RTW_PROC_HDL_SSEQ("rx_logs", proc_get_rx_logs, proc_reset_rx_logs),
	RTW_PROC_HDL_SSEQ("tx_logs", proc_get_tx_logs, proc_set_tx_logs),
	RTW_PROC_HDL_SSEQ("int_logs", proc_get_int_logs, NULL),
#ifdef CONFIG_RTW_DEBUG_BCN_STATS
	RTW_PROC_HDL_SSEQ("bcn_stats", proc_get_bcn_stats, proc_set_bcn_stats),
#endif /* CONFIG_RTW_DEBUG_BCN_STATS */
#endif

#ifdef CONFIG_DBG_RF_CAL
	RTW_PROC_HDL_SSEQ("iqk", proc_get_iqk_info, proc_set_iqk),
	RTW_PROC_HDL_SSEQ("lck", proc_get_lck_info, proc_set_lck),
#endif

#ifdef CONFIG_PCI_HCI
	RTW_PROC_HDL_SSEQ("rx_ring", proc_get_rx_ring, NULL),
	RTW_PROC_HDL_SSEQ("tx_ring", proc_get_tx_ring, NULL),
#ifdef DBG_TXBD_DESC_DUMP
	RTW_PROC_HDL_SSEQ("tx_ring_ext", proc_get_tx_ring_ext, proc_set_tx_ring_ext),
#endif
	RTW_PROC_HDL_SSEQ("pci_aspm", proc_get_pci_aspm, NULL),

	RTW_PROC_HDL_SSEQ("pci_conf_space", proc_get_pci_conf_space, proc_set_pci_conf_space),

	RTW_PROC_HDL_SSEQ("pci_bridge_conf_space", proc_get_pci_bridge_conf_space, proc_set_pci_bridge_conf_space),
#ifdef CONFIG_PCI_TRX_RES_DBG
	RTW_PROC_HDL_SSEQ("bd_info", proc_get_bd_info, NULL),
#endif
#endif

#ifdef CONFIG_WOWLAN
	RTW_PROC_HDL_SSEQ("wow_enable", proc_get_wow_enable, proc_set_wow_enable),
	RTW_PROC_HDL_SSEQ("wow_pattern_info", proc_get_pattern_info, proc_set_pattern_info),
	RTW_PROC_HDL_SSEQ("wow_wakeup_event", proc_get_wakeup_event,
			  proc_set_wakeup_event),
	RTW_PROC_HDL_SSEQ("wowlan_last_wake_reason", proc_get_wakeup_reason, NULL),
	RTW_PROC_HDL_SSEQ("wow_pattern_cam", proc_dump_pattern_cam, NULL),
#endif

#ifdef CONFIG_GPIO_WAKEUP
	RTW_PROC_HDL_SSEQ("wowlan_gpio_info", proc_get_wowlan_gpio_info, proc_set_wowlan_gpio_info),
#endif
#ifdef CONFIG_P2P_WOWLAN
	RTW_PROC_HDL_SSEQ("p2p_wowlan_info", proc_get_p2p_wowlan_info, NULL),
#endif
	RTW_PROC_HDL_SSEQ("country_code", proc_get_country_code, proc_set_country_code),
	RTW_PROC_HDL_SSEQ("chan_plan", proc_get_chan_plan, proc_set_chan_plan),
	RTW_PROC_HDL_SSEQ("chplan_ver", proc_get_chplan_ver, NULL),
	RTW_PROC_HDL_SSEQ("cap_spt_op_class_ch", proc_get_cap_spt_op_class_ch, proc_set_cap_spt_op_class_ch),
	RTW_PROC_HDL_SSEQ("reg_spt_op_class_ch", proc_get_reg_spt_op_class_ch, proc_set_reg_spt_op_class_ch),
	RTW_PROC_HDL_SSEQ("cur_spt_op_class_ch", proc_get_cur_spt_op_class_ch, proc_set_cur_spt_op_class_ch),
#if CONFIG_RTW_MACADDR_ACL
	RTW_PROC_HDL_SSEQ("macaddr_acl", proc_get_macaddr_acl, proc_set_macaddr_acl),
#endif
#if CONFIG_RTW_PRE_LINK_STA
	RTW_PROC_HDL_SSEQ("pre_link_sta", proc_get_pre_link_sta, proc_set_pre_link_sta),
#endif
	RTW_PROC_HDL_SSEQ("ch_sel_policy", proc_get_ch_sel_policy, proc_set_ch_sel_policy),
#if CONFIG_DFS
	RTW_PROC_HDL_SSEQ("csa_trigger", NULL, proc_set_csa_trigger),
#ifdef CONFIG_DFS_MASTER
	RTW_PROC_HDL_SSEQ("dfs_test_case", proc_get_dfs_test_case, proc_set_dfs_test_case),
	RTW_PROC_HDL_SSEQ("update_non_ocp", NULL, proc_set_update_non_ocp),
	RTW_PROC_HDL_SSEQ("radar_detect", NULL, proc_set_radar_detect),
	RTW_PROC_HDL_SSEQ("dfs_ch_sel_e_flags", proc_get_dfs_ch_sel_e_flags, proc_set_dfs_ch_sel_e_flags),
	RTW_PROC_HDL_SSEQ("dfs_ch_sel_d_flags", proc_get_dfs_ch_sel_d_flags, proc_set_dfs_ch_sel_d_flags),
	RTW_PROC_HDL_SSEQ("reset_non_ocp_time", NULL, proc_set_reset_non_ocp_time),
	RTW_PROC_HDL_SSEQ("dfs_regions", proc_get_dfs_regions, proc_set_dfs_regions),
	RTW_PROC_HDL_SSEQ("dfs_cac_time", NULL, proc_set_dfs_cac_time),
#if CONFIG_DFS_SLAVE_WITH_RADAR_DETECT
	RTW_PROC_HDL_SSEQ("dfs_slave_with_rd", proc_get_dfs_slave_with_rd, proc_set_dfs_slave_with_rd),
#endif
	RTW_PROC_HDL_SSEQ("mib_dfs", proc_get_mib_dfs, NULL),
#endif
#endif
	RTW_PROC_HDL_SSEQ("sink_udpport", proc_get_udpport, proc_set_udpport),
#ifdef DBG_RX_COUNTER_DUMP
	RTW_PROC_HDL_SSEQ("dump_rx_cnt_mode", proc_get_rx_cnt_dump, proc_set_rx_cnt_dump),
#endif
	RTW_PROC_HDL_SSEQ("change_bss_chbw", NULL, proc_set_change_bss_chbw),
	RTW_PROC_HDL_SSEQ("tx_bw_mode", proc_get_tx_bw_mode, proc_set_tx_bw_mode),

	RTW_PROC_HDL_SSEQ("target_tx_power", proc_get_target_tx_power, NULL),
	RTW_PROC_HDL_SSEQ("tx_power_by_rate", proc_get_tx_power_by_rate, NULL),
#if CONFIG_TXPWR_LIMIT
	RTW_PROC_HDL_SSEQ("tx_power_limit", proc_get_tx_power_limit, NULL),
#endif
#ifdef GEORGIA_TODO_TX_PWR
	RTW_PROC_HDL_SSEQ("tx_power_ext_info", proc_get_tx_power_ext_info, proc_set_tx_power_ext_info),
	RTW_PROC_HDL_SEQ("tx_power_idx", &seq_ops_tx_power_idx, NULL),
	RTW_PROC_HDL_SEQ("txpwr_total_dbm", &seq_ops_txpwr_total_dbm, NULL),
#endif
#ifdef CONFIG_POWER_SAVING
	RTW_PROC_HDL_SSEQ("ps_info", proc_get_ps_info, proc_set_ps_info),
#ifdef CONFIG_WMMPS_STA
	RTW_PROC_HDL_SSEQ("wmmps_info", proc_get_wmmps_info, proc_set_wmmps_info),
#endif /* CONFIG_WMMPS_STA */
#endif
#ifdef CONFIG_TDLS
	RTW_PROC_HDL_SSEQ("tdls_info", proc_get_tdls_info, NULL),
	RTW_PROC_HDL_SSEQ("tdls_enable", proc_get_tdls_enable, proc_set_tdls_enable),
#endif
	RTW_PROC_HDL_SSEQ("monitor", proc_get_monitor, proc_set_monitor),

#ifdef CONFIG_RTW_ACS
#ifdef CONFIG_RTW_DACS
	RTW_PROC_HDL_SSEQ("dacs", proc_get_dacs_chan_sts, NULL),
#endif
	RTW_PROC_HDL_SSEQ("acs", proc_get_best_chan, proc_set_acs),
	RTW_PROC_HDL_SSEQ("chan_info", proc_get_chan_info, NULL),
#endif

	RTW_PROC_HDL_SSEQ("hal_spec", proc_get_hal_spec, NULL),
	RTW_PROC_HDL_SSEQ("hal_trx_mode", proc_get_hal_trx_mode, NULL),
	RTW_PROC_HDL_SSEQ("hal_txpwr_info", proc_get_hal_txpwr_info, NULL),

#ifdef CONFIG_PREALLOC_RX_SKB_BUFFER
	RTW_PROC_HDL_SSEQ("rtkm_info", proc_get_rtkm_info, NULL),
#endif
	RTW_PROC_HDL_SSEQ("efuse_map", proc_get_efuse_map, NULL),
#if defined(CONFIG_IEEE80211W) && !defined(CONFIG_IOCTL_CFG80211)
	RTW_PROC_HDL_SSEQ("11w_tx_sa_query", proc_get_tx_sa_query, proc_set_tx_sa_query),
	RTW_PROC_HDL_SSEQ("11w_tx_deauth", proc_get_tx_deauth, proc_set_tx_deauth),
	RTW_PROC_HDL_SSEQ("11w_tx_auth", proc_get_tx_auth, proc_set_tx_auth),
#endif /* CONFIG_IEEE80211W */

	RTW_PROC_HDL_SSEQ("mac_addr", proc_get_mac_addr, NULL),
	RTW_PROC_HDL_SSEQ("skip_band", proc_get_skip_band, proc_set_skip_band),

	RTW_PROC_HDL_SSEQ("rx_stat", proc_get_rx_stat, NULL),

	RTW_PROC_HDL_SSEQ("tx_stat", proc_get_tx_stat, NULL),
	/**** PHY Capability ****/
	RTW_PROC_HDL_SSEQ("phy_cap", proc_get_phy_cap, NULL),
#ifdef CONFIG_80211N_HT
	RTW_PROC_HDL_SSEQ("rx_stbc", proc_get_rx_stbc, proc_set_rx_stbc),
	RTW_PROC_HDL_SSEQ("stbc_cap", proc_get_stbc_cap, proc_set_stbc_cap),
	RTW_PROC_HDL_SSEQ("ldpc_cap", proc_get_ldpc_cap, proc_set_ldpc_cap),
#endif /* CONFIG_80211N_HT */
#ifdef CONFIG_BEAMFORMING
	RTW_PROC_HDL_SSEQ("txbf_cap", proc_get_txbf_cap, proc_set_txbf_cap),
#endif

#ifdef CONFIG_SUPPORT_TRX_SHARED
	RTW_PROC_HDL_SSEQ("trx_share_mode", proc_get_trx_share_mode, NULL),
#endif
	RTW_PROC_HDL_SSEQ("napi_info", proc_get_napi_info, NULL),
#ifdef CONFIG_RTW_NAPI_DYNAMIC
	RTW_PROC_HDL_SSEQ("napi_th", proc_get_napi_info, proc_set_napi_th),
#endif /* CONFIG_RTW_NAPI_DYNAMIC */

	RTW_PROC_HDL_SSEQ("rsvd_page", proc_dump_rsvd_page, proc_set_rsvd_page_info),

#ifdef CONFIG_SUPPORT_FIFO_DUMP
	RTW_PROC_HDL_SSEQ("fifo_dump", proc_dump_fifo, proc_set_fifo_info),
#endif


#ifdef DBG_XMIT_BLOCK
	RTW_PROC_HDL_SSEQ("xmit_block", proc_get_xmit_block, proc_set_xmit_block),
#endif

	RTW_PROC_HDL_SSEQ("ack_timeout", proc_get_ack_timeout, proc_set_ack_timeout),

	RTW_PROC_HDL_SSEQ("dynamic_agg_enable", proc_get_dynamic_agg_enable, proc_set_dynamic_agg_enable),
	RTW_PROC_HDL_SSEQ("fw_offload", proc_get_fw_offload, proc_set_fw_offload),

#ifdef CONFIG_RTW_A4_STA
	RTW_PROC_HDL_SSEQ("a4_gptr", proc_get_wds_gptr, NULL),
#endif

#ifdef CONFIG_RTW_MESH
	#if CONFIG_RTW_MESH_ACNODE_PREVENT
	RTW_PROC_HDL_SSEQ("mesh_acnode_prevent", proc_get_mesh_acnode_prevent, proc_set_mesh_acnode_prevent),
	#endif
	#if CONFIG_RTW_MESH_OFFCH_CAND
	RTW_PROC_HDL_SSEQ("mesh_offch_cand", proc_get_mesh_offch_cand, proc_set_mesh_offch_cand),
	#endif
	#if CONFIG_RTW_MESH_PEER_BLACKLIST
	RTW_PROC_HDL_SSEQ("mesh_peer_blacklist", proc_get_mesh_peer_blacklist, proc_set_mesh_peer_blacklist),
	#endif
	#if CONFIG_RTW_MESH_CTO_MGATE_BLACKLIST
	RTW_PROC_HDL_SSEQ("mesh_cto_mgate_require", proc_get_mesh_cto_mgate_require, proc_set_mesh_cto_mgate_require),
	RTW_PROC_HDL_SSEQ("mesh_cto_mgate_blacklist", proc_get_mesh_cto_mgate_blacklist, proc_set_mesh_cto_mgate_blacklist),
	#endif
	RTW_PROC_HDL_SSEQ("mesh_peer_sel_policy", proc_get_mesh_peer_sel_policy, NULL),
	RTW_PROC_HDL_SSEQ("mesh_networks", proc_get_mesh_networks, NULL),
	RTW_PROC_HDL_SSEQ("mesh_plink_ctl", proc_get_mesh_plink_ctl, NULL),
	RTW_PROC_HDL_SSEQ("mesh_mpath", proc_get_mesh_mpath, NULL),
	RTW_PROC_HDL_SSEQ("mesh_mpp", proc_get_mesh_mpp, NULL),
	RTW_PROC_HDL_SSEQ("mesh_known_gates", proc_get_mesh_known_gates, NULL),
	#if CONFIG_RTW_MESH_DATA_BMC_TO_UC
	RTW_PROC_HDL_SSEQ("mesh_b2u_flags", proc_get_mesh_b2u_flags, proc_set_mesh_b2u_flags),
	#endif
	RTW_PROC_HDL_SSEQ("mesh_stats", proc_get_mesh_stats, NULL),
	RTW_PROC_HDL_SSEQ("mesh_gate_timeout_factor", proc_get_mesh_gate_timeout, proc_set_mesh_gate_timeout),
	RTW_PROC_HDL_SSEQ("mesh_gate_state", proc_get_mesh_gate_state, NULL),
	RTW_PROC_HDL_SSEQ("mesh_peer_alive_based_preq", proc_get_peer_alive_based_preq, proc_set_peer_alive_based_preq),
#endif

#ifdef CONFIG_LPS_CHK_BY_TP
	RTW_PROC_HDL_SSEQ("lps_chk_tp", proc_get_lps_chk_tp, proc_set_lps_chk_tp),
#endif
#ifdef CONFIG_SUPPORT_STATIC_SMPS
	RTW_PROC_HDL_SSEQ("smps", proc_get_smps, proc_set_smps),
#endif

#ifdef RTW_BUSY_DENY_SCAN
	RTW_PROC_HDL_SSEQ("scan_interval_thr", proc_get_scan_interval_thr, \
			  proc_set_scan_interval_thr),
#endif
	RTW_PROC_HDL_SSEQ("scan_deny", proc_get_scan_deny, proc_set_scan_deny),

#ifdef CONFIG_CTRL_TXSS_BY_TP
	RTW_PROC_HDL_SSEQ("txss_tp", proc_get_txss_tp, proc_set_txss_tp),
	#ifdef DBG_CTRL_TXSS
	RTW_PROC_HDL_SSEQ("txss_ctrl", proc_get_txss_ctrl, proc_set_txss_ctrl),
	#endif
#endif

	RTW_PROC_HDL_SSEQ("cur_beacon_keys", proc_get_cur_beacon_keys, NULL),

	RTW_PROC_HDL_SSEQ("chan", proc_get_chan, proc_set_chan),
#ifdef CONFIG_RTW_80211K
	RTW_PROC_HDL_SSEQ("nb_report", proc_get_neighbor_rpt, proc_set_neighbor_rpt),
#endif
	RTW_PROC_HDL_SSEQ("deny_legacy", proc_get_deny_legacy, proc_set_deny_legacy),
	RTW_PROC_HDL_SSEQ("log_level", proc_get_log_level, proc_set_log_level),
	RTW_PROC_HDL_SSEQ("wmm_dm", proc_get_wmm_dm, proc_set_wmm_dm),
	RTW_PROC_HDL_SSEQ("dump_debug", proc_get_dump_debug, NULL),
	RTW_PROC_HDL_SSEQ("dump_cnt", proc_get_dump_cnt, NULL),
#ifdef CONFIG_RTW_A4_STA
	RTW_PROC_HDL_SSEQ("a4_dump", proc_get_a4, NULL),
#endif
	RTW_PROC_HDL_SSEQ("record", proc_get_record_trx, proc_set_record_trx),
	RTW_PROC_HDL_SSEQ("rx_debug", proc_get_rx_debug, proc_set_rx_debug),
#ifdef CONFIG_CORE_TXSC
	RTW_PROC_HDL_SSEQ("txsc", proc_get_txsc, proc_set_txsc),
#ifdef CONFIG_TXSC_AMSDU
	RTW_PROC_HDL_SSEQ("amsdu", proc_get_amsdu, proc_set_amsdu),
#endif
#endif
#ifdef CONFIG_RTW_CORE_RXSC
	RTW_PROC_HDL_SSEQ("rxsc", proc_get_rxsc, proc_set_rxsc),
#endif
	RTW_PROC_HDL_SSEQ("buf_cap", proc_get_bufcap, NULL),
#ifdef DEBUG_PHL_RX
	RTW_PROC_HDL_SSEQ("debug_phl", proc_get_debug_phl, NULL),
#endif
	RTW_PROC_HDL_SSEQ("dscp_mapping", proc_get_dscp_mapping, proc_set_dscp_mapping),
#ifdef CONFIG_ONE_TXQ
	RTW_PROC_HDL_SSEQ("atm", proc_get_atm, proc_set_atm),
#endif
#ifdef CONFIG_ADPTVTY_CONTROL
	RTW_PROC_HDL_SSEQ("adptvty_info", proc_get_adptvty_info, NULL),
#endif /* CONFIG_ADPTVTY_CONTROL */
};

const int adapter_proc_hdls_num = sizeof(adapter_proc_hdls) / sizeof(struct rtw_proc_hdl);

#ifndef PLATFORM_ECOS
EXPORT_SYMBOL(adapter_proc_hdls);
EXPORT_SYMBOL(adapter_proc_hdls_num);

static int rtw_adapter_proc_open(struct inode *inode, struct file *file)
{
	ssize_t index = (ssize_t)PDE_DATA(inode);
	const struct rtw_proc_hdl *hdl = adapter_proc_hdls + index;
	void *private = proc_get_parent_data(inode);

	if (hdl->type == RTW_PROC_HDL_TYPE_SEQ) {
		int res = seq_open(file, hdl->u.seq_op);

		if (res == 0)
			((struct seq_file *)file->private_data)->private = private;

		return res;
	} else if (hdl->type == RTW_PROC_HDL_TYPE_SSEQ) {
		int (*show)(struct seq_file *, void *) = hdl->u.show ? hdl->u.show : proc_get_dummy;

		return single_open(file, show, private);
	} else if (hdl->type == RTW_PROC_HDL_TYPE_SZSEQ) {
		int (*show)(struct seq_file *, void *) = hdl->u.sz.show ? hdl->u.sz.show : proc_get_dummy;

		#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
		return single_open_size(file, show, private, hdl->u.sz.size);
		#else
		return single_open(file, show, private);
		#endif
	} else {
		return -EROFS;
	}
}

static ssize_t rtw_adapter_proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *pos)
{
	ssize_t index = (ssize_t)PDE_DATA(file_inode(file));
	const struct rtw_proc_hdl *hdl = adapter_proc_hdls + index;
	ssize_t (*write)(struct file *, const char __user *, size_t, loff_t *, void *) = hdl->write;

	if (write)
		return write(file, buffer, count, pos, ((struct seq_file *)file->private_data)->private);

	return -EROFS;
}

static const struct rtw_proc_ops rtw_adapter_proc_seq_fops = {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0))
	.proc_open = rtw_adapter_proc_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
	.proc_write = rtw_adapter_proc_write,
#else
	.owner = THIS_MODULE,
	.open = rtw_adapter_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
	.write = rtw_adapter_proc_write,
#endif
};

static const struct rtw_proc_ops rtw_adapter_proc_sseq_fops = {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0))
	.proc_open = rtw_adapter_proc_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
	.proc_write = rtw_adapter_proc_write,
#else
	.owner = THIS_MODULE,
	.open = rtw_adapter_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
	.write = rtw_adapter_proc_write,
#endif
};
#endif /* PLATFORM_ECOS */

int proc_get_phy_adaptivity(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

	rtw_hal_phy_adaptivity_parm_msg(m, padapter);

	return 0;
}

ssize_t proc_set_phy_adaptivity(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[32];
	u32 th_l2h_ini;
	s8 th_edcca_hl_diff;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%x %hhd", &th_l2h_ini, &th_edcca_hl_diff);

		if (num != 2)
			return count;

		rtw_hal_phy_adaptivity_parm_set(padapter, (s8)th_l2h_ini, th_edcca_hl_diff);
	}

	return count;
}

#define PHYDM_MSG_SIZE_MIN (128)
#define PHYDM_MSG_SIZE_MAX (80 * 2400 * 4)

static char *phydm_msg = NULL;
static size_t phydm_msg_size = PHYDM_MSG_SIZE_MIN;

static int proc_get_phydm_cmd(struct seq_file *m, void *v)
{
	if (phydm_msg) {
		_RTW_PRINT_SEL(m, "%s\n", phydm_msg);

		rtw_mfree(phydm_msg, phydm_msg_size);
		phydm_msg = NULL;
	} else {
		_RTW_PRINT_SEL(m, "(Nothing to output)\n");
	}

	return 0;
}


/*
 * PHL proc debug command of core:
 * command start with "core" to
 */
enum PHL_CMD_CORE_ID {
	PHL_CMD_CORE_HELP,
	PHL_CMD_CORE_GIT_INFO,
	#ifdef CONFIG_RTW_OS_HANDLER_EXT
	PHL_CMD_CORE_HANDLER_CPU,
	#endif /* CONFIG_RTW_OS_HANDLER_EXT */
};

struct _phl_cmd_name_id {
	const char *name;
	enum PHL_CMD_CORE_ID id;
};

#define	PHL_PROC_CMD_PRINTF(max_buff_len, used_len, buff_addr, remain_len, fmt, ...)\
	do {									\
		u32 *used_len_tmp = &(used_len);				\
		if (*used_len_tmp < max_buff_len)				\
			*used_len_tmp += snprintf(buff_addr, remain_len, fmt, ##__VA_ARGS__);\
	} while (0)

static const struct _phl_cmd_name_id _phl_core_cmds[] = {
	{"-h", PHL_CMD_CORE_HELP}, /*@do not move this element to other position*/
	{"git_info", PHL_CMD_CORE_GIT_INFO},
	#ifdef CONFIG_RTW_OS_HANDLER_EXT
	{"handler_cpu", PHL_CMD_CORE_HANDLER_CPU},
	#endif /* CONFIG_RTW_OS_HANDLER_EXT */
};

void _phl_cmd_git_info(char input[][MAX_ARGV],
		      u32 input_num, char *output, u32 out_len)
{
#if CONFIG_GEN_GIT_INFO
#include "../../phl/phl_git_info.h"

	u32 used = 0;

	PHL_PROC_CMD_PRINTF(out_len, used, output + used, out_len - used,
		"\ncore_ver   : %s\n", RTK_CORE_TAGINFO);
	PHL_PROC_CMD_PRINTF(out_len, used, output + used, out_len - used,
		"phl_ver    : %s\n", RTK_PHL_TAGINFO);
	PHL_PROC_CMD_PRINTF(out_len, used, output + used, out_len - used,
		"halmac_ver : %s\n", RTK_HALMAC_TAGINFO);
	PHL_PROC_CMD_PRINTF(out_len, used, output + used, out_len - used,
		"halbb_ver  : %s\n", RTK_HALBB_TAGINFO);
	PHL_PROC_CMD_PRINTF(out_len, used, output + used, out_len - used,
		"halrf_ver  : %s\n", RTK_HALRF_TAGINFO);
	PHL_PROC_CMD_PRINTF(out_len, used, output + used, out_len - used,
		"btc_ver    : %s\n", RTK_BTC_TAGINFO);

	PHL_PROC_CMD_PRINTF(out_len, used, output + used, out_len - used,
		"\ncore_sha1  : %s\n", RTK_CORE_SHA1);
	PHL_PROC_CMD_PRINTF(out_len, used, output + used, out_len - used,
		"phl_sha1   : %s\n", RTK_PHL_SHA1);
	PHL_PROC_CMD_PRINTF(out_len, used, output + used, out_len - used,
		"halmac_sha1: %s\n", RTK_HALMAC_SHA1);
	PHL_PROC_CMD_PRINTF(out_len, used, output + used, out_len - used,
		"halbb_sha1 : %s\n", RTK_HALBB_SHA1);
	PHL_PROC_CMD_PRINTF(out_len, used, output + used, out_len - used,
		"halrf_sha1 : %s\n", RTK_HALRF_SHA1);
	PHL_PROC_CMD_PRINTF(out_len, used, output + used, out_len - used,
		"btc_sha1   : %s\n", RTK_BTC_SHA1);
#endif /* CONFIG_GEN_GIT_INFO */
}

#if defined(CONFIG_RTW_OS_HANDLER_EXT)
struct handler_id_name {
	int	    id;
	const char *name;
};

static void _phl_cmd_handler_cpu(_adapter *padapter, char input[][MAX_ARGV],
                                 u32 input_num, char *output, u32 out_len)
{
	struct dvobj_priv *dvobj = padapter->dvobj;
	struct rtw_os_handler *os_handler;
	u32 handler_num = ARRAY_SIZE(dvobj->handlers);
	u32 handler_id;
	u32 cpu_id;
	u32 i;

	if (input_num < 3) {
		for (i = 0; i < handler_num; i++) {
			if (dvobj->handlers[i] == NULL)
				continue;
			os_handler = dvobj->handlers[i];

			RTW_PRINT("%d - %s: %c@(%u/%u)"
				  #ifdef CONFIG_RTW_OS_HANDLER_EXT_DBG
				  " R(%u/%u)"
				  #endif /* CONFIG_RTW_OS_HANDLER_EXT_DBG */
				  "\n",
				  i, os_handler->name,
			          (  (   os_handler->type
			              == RTW_OS_HANDLER_TASKLET)
			           ? 'T'
			           : 'W'),
			          os_handler->cpu_id, WORK_CPU_UNBOUND
				  #ifdef CONFIG_RTW_OS_HANDLER_EXT_DBG
				  , os_handler->run_on_cpu
				  , os_handler->run
				  #endif /* CONFIG_RTW_OS_HANDLER_EXT_DBG */
				  );
		}
		return;
	}

	sscanf(input[1], "%u", &handler_id);
	sscanf(input[2], "%u", &cpu_id);

	if (handler_id >= handler_num) {
		RTW_ERR(DEV_FMT": Invalid handler ID %u (%u)\n",
		        DEV_ARG(dvobj), handler_id, handler_num);
		return;
	}

	os_handler = dvobj->handlers[handler_id];
	if (os_handler == NULL) {
		RTW_ERR(DEV_FMT": No handler for ID %u\n",
		        DEV_ARG(dvobj), handler_id);
		return;
	}

	if (cpu_id >= WORK_CPU_UNBOUND) {
		RTW_ERR(DEV_FMT": CPU ID %u is not within range %u.\n",
		        DEV_ARG(dvobj), cpu_id, WORK_CPU_UNBOUND);
		return;
	}
	os_handler->cpu_id = cpu_id;
	RTW_PRINT(DEV_FMT": Set handler %u \"%s\" run on CPU %u.\n",
	          DEV_ARG(dvobj), handler_id, os_handler->name,
	          os_handler->cpu_id);
}
#endif /* CONFIG_RTW_OS_HANDLER_EXT */

void _parse_phl_core_cmd(_adapter *padapter, char input[][MAX_ARGV],
		        u32 input_num, char *output, u32 out_len)
{
	u8 id = 0;
	u32 i;
	u32 used = 0;
	u32 cmd_num = ARRAY_SIZE(_phl_core_cmds);

	if (cmd_num == 0)
		return;

	PHL_PROC_CMD_PRINTF(out_len, used, output + used, out_len - used,
	                    ADPT_FMT" PHL command: \"%s\"\n",
	                    ADPT_ARG(padapter), input[0]);
	/* Parsing Cmd ID */
	if (input_num) {
		for (i = 0; i < cmd_num; i++) {
			if (strcmp(_phl_core_cmds[i].name, input[0]) == 0) {
				id = _phl_core_cmds[i].id;
				RTW_INFO("[%s]===>\n", _phl_core_cmds[i].name);
				break;
			}
		}
		if (i == cmd_num) {
			PHL_PROC_CMD_PRINTF(out_len, used, output + used,
			                    out_len - used, "PHL CMD not found!\n");
			return;
		}
	}

	switch (id) {
	case PHL_CMD_CORE_HELP:
	{
		PHL_PROC_CMD_PRINTF(out_len, used, output + used, out_len - used,
				    "PHL core commands: \n");
		for (i = 0; i < cmd_num; i++)
			PHL_PROC_CMD_PRINTF(out_len, used, output + used,
			                    out_len - used, "%-5d: %s\n",
			                    (int)i, _phl_core_cmds[i].name);

	}
	break;
	case PHL_CMD_CORE_GIT_INFO:
	{
		_phl_cmd_git_info(input, input_num, output, out_len);
	}
	break;
#ifdef CONFIG_RTW_OS_HANDLER_EXT
	case PHL_CMD_CORE_HANDLER_CPU:
		_phl_cmd_handler_cpu(padapter, input, input_num, output, out_len);
		break;
#endif /* CONFIG_RTW_OS_HANDLER_EXT */
	default:
		PHL_PROC_CMD_PRINTF(out_len, used, output + used,
		                    out_len - used, "[DBG] Do not support this command\n");
		break;
	}
}

static int
_parse_phl_core_cmd_buf(_adapter *padapter, char *input, char *output, u32 out_len)
{
	char *token;
	u32 argc = 0;
	char argv[MAX_ARGC][MAX_ARGV];

	do {
		token = strsep(&input, ", ");
		if (token) {
			if (strlen((u8 *)token) <= MAX_ARGV)
				strcpy(argv[argc], token);

			argc++;
		} else {
			break;
		}
	} while (argc < MAX_ARGC);

	/* Strip out final new line */
	if (argc > 0) {
		char *last_arg = argv[argc - 1];
		int len = strlen(last_arg);
		if (last_arg[len - 1] == '\n')
			last_arg[len - 1] = '\0';
	}

	_parse_phl_core_cmd(padapter, argv, argc, output, out_len);

	return 0;
}

/* core xxx > DEV_PROC_PATH/odm/phl_cmd */
static void
_phl_proc_core_cmd(_adapter *padapter, struct rtw_proc_cmd *incmd,
                   char *output, u32 out_len)
{
	if (incmd->in_type == RTW_ARG_TYPE_BUF) {
		_parse_phl_core_cmd_buf(padapter, incmd->in.buf, output, out_len);
	} else if(incmd->in_type == RTW_ARG_TYPE_ARRAY){
		_parse_phl_core_cmd(padapter, incmd->in.vector,
		                    incmd->in_cnt_len, output, out_len);
	}
}

static int proc_get_phl_cmd(struct seq_file *m, void *v)
{
	return proc_get_phydm_cmd(m, v);
}

static ssize_t proc_set_phydm_cmd(struct file *file, char *buffer, size_t count,
				  loff_t *pos, void *data,
				  enum rtw_proc_cmd_type type)
{
	struct net_device *netdev = (struct net_device *)data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(netdev);
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	struct rtw_proc_cmd cmd;

	if (count < 1)
		return -EFAULT;

	if (buffer) {
		if (phydm_msg) {
			_rtw_memset(phydm_msg, 0, phydm_msg_size);
		} else {
			phydm_msg = rtw_zmalloc(phydm_msg_size);
			if (!phydm_msg)
				return -ENOMEM;
		}
		/* Parse buffer to argument array for submodules to
		   avoid from parameter parsing error in submodules */
#if 0
		cmd.in_type = RTW_ARG_TYPE_BUF;
		cmd.in_cnt_len = count;
		cmd.in.buf = buffer;
#else
		cmd.in_cnt_len = 0;
		do {
			char *token;
			token = strsep(&buffer, ", ");
			if (token == NULL)
				break;

			if (strlen((u8 *)token) < MAX_ARGV)
				strcpy(cmd.in.vector[cmd.in_cnt_len++], token);
		} while (cmd.in_cnt_len < MAX_ARGC);

		cmd.in_type = RTW_ARG_TYPE_ARRAY;
#endif
		/* Handle core commands in core */
		if (type == RTW_PROC_CMD_CORE)
			_phl_proc_core_cmd(padapter, &cmd, phydm_msg, phydm_msg_size);
		else
			rtw_phl_proc_cmd(GET_HAL_INFO(dvobj), type, &cmd, phydm_msg, phydm_msg_size);

		if (strlen(phydm_msg) == 0) {
			rtw_mfree(phydm_msg, phydm_msg_size);
			phydm_msg = NULL;
		}
	}

	return count;
}

static ssize_t proc_set_phl_cmd(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	char *buf, *p;
	enum rtw_proc_cmd_type type;
	ssize_t ret, len;
	int type_len;

	buf = _rtw_malloc(count + 1);
	if (!buf)
		return -ENOMEM;

	if (copy_from_user(buf, buffer, count)) {
		ret = -EFAULT;
		goto err;
	}

	if (strncmp(buf, "bb ", 3) == 0) {
		type = RTW_PROC_CMD_BB;
		type_len = 3;
	} else if (strncmp(buf, "rf ", 3) == 0) {
		type = RTW_PROC_CMD_RF;
		type_len = 3;
	} else if (strncmp(buf, "mac ", 4) == 0) {
		type = RTW_PROC_CMD_MAC;
		type_len = 4;
	} else if (strncmp(buf, "phl ", 4) == 0) {
		type = RTW_PROC_CMD_PHL;
		type_len = 4;
	} else if (strncmp(buf, "core ", 5) == 0) {
		type = RTW_PROC_CMD_CORE;
		type_len = 5;
	} else {
		ret = -EINVAL;
		goto err;
	}

	/* skip first type token, like 'bb ', 'rf ' */
	p = buf + type_len;
	len = count - type_len;

	/* remove trailing newline character, because BB/RF parser uses
	 * " ," as delimiter that leads the last token contains '\n' if
	 * we use 'echo' command without '-n' argument.
	 */
	if (p[len - 1] == '\n')
		len--;
	p[len] = 0;

	ret = proc_set_phydm_cmd(file, p, len, pos, data, type);

err:
	_rtw_mfree(buf, count + 1);

	if (ret >= 0)
		return count;

	return ret;
}

static int proc_get_phydm_msg_size(struct seq_file *m, void *v)
{
	RTW_PRINT_SEL(m, "phydm_msg_size = %u\n", phydm_msg_size);
	return 0;
}

static ssize_t proc_set_phydm_msg_size(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	char buf[32] = {0};
	u32 size = 0;

	if (count > sizeof(buf) || copy_from_user(buf, buffer, count))
		return -EFAULT;

	sscanf(buf, "%u", &size);
	if (size >= PHYDM_MSG_SIZE_MIN && size <= PHYDM_MSG_SIZE_MAX) {
		if (phydm_msg) {
			rtw_mfree(phydm_msg, phydm_msg_size);
			phydm_msg = NULL;
		}

		phydm_msg_size = size;
	}

	return count;
}

/*
* rtw_odm_proc:
* init/deinit when register/unregister net_device, along with rtw_adapter_proc
*/
const struct rtw_proc_hdl odm_proc_hdls[] = {
	RTW_PROC_HDL_SSEQ("adaptivity", proc_get_phy_adaptivity, proc_set_phy_adaptivity),
	RTW_PROC_HDL_SSEQ("phl_cmd", proc_get_phl_cmd, proc_set_phl_cmd),
	RTW_PROC_HDL_SSEQ("phydm_msg_size", proc_get_phydm_msg_size, proc_set_phydm_msg_size),
};

const int odm_proc_hdls_num = sizeof(odm_proc_hdls) / sizeof(struct rtw_proc_hdl);

#ifndef PLATFORM_ECOS
EXPORT_SYMBOL(odm_proc_hdls);
EXPORT_SYMBOL(odm_proc_hdls_num);

static int rtw_odm_proc_open(struct inode *inode, struct file *file)
{
	ssize_t index = (ssize_t)PDE_DATA(inode);
	const struct rtw_proc_hdl *hdl = odm_proc_hdls + index;
	void *private = proc_get_parent_data(inode);

	if (hdl->type == RTW_PROC_HDL_TYPE_SEQ) {
		int res = seq_open(file, hdl->u.seq_op);

		if (res == 0)
			((struct seq_file *)file->private_data)->private = private;

		return res;
	} else if (hdl->type == RTW_PROC_HDL_TYPE_SSEQ) {
		int (*show)(struct seq_file *, void *) = hdl->u.show ? hdl->u.show : proc_get_dummy;

		return single_open(file, show, private);
	} else if (hdl->type == RTW_PROC_HDL_TYPE_SZSEQ) {
		int (*show)(struct seq_file *, void *) = hdl->u.sz.show ? hdl->u.sz.show : proc_get_dummy;

		#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
		return single_open_size(file, show, private, hdl->u.sz.size);
		#else
		return single_open(file, show, private);
		#endif
	} else {
		return -EROFS;
	}
}

static ssize_t rtw_odm_proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *pos)
{
	ssize_t index = (ssize_t)PDE_DATA(file_inode(file));
	const struct rtw_proc_hdl *hdl = odm_proc_hdls + index;
	ssize_t (*write)(struct file *, const char __user *, size_t, loff_t *, void *) = hdl->write;

	if (write)
		return write(file, buffer, count, pos, ((struct seq_file *)file->private_data)->private);

	return -EROFS;
}

static const struct rtw_proc_ops rtw_odm_proc_seq_fops = {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0))
	.proc_open = rtw_odm_proc_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
	.proc_write = rtw_odm_proc_write,
#else
	.owner = THIS_MODULE,
	.open = rtw_odm_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
	.write = rtw_odm_proc_write,
#endif
};

static const struct rtw_proc_ops rtw_odm_proc_sseq_fops = {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0))
	.proc_open = rtw_odm_proc_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
	.proc_write = rtw_odm_proc_write,
#else
	.owner = THIS_MODULE,
	.open = rtw_odm_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
	.write = rtw_odm_proc_write,
#endif
};

struct proc_dir_entry *rtw_odm_proc_init(struct net_device *dev)
{
	struct proc_dir_entry *dir_odm = NULL;
	struct proc_dir_entry *entry = NULL;
	_adapter	*adapter = rtw_netdev_priv(dev);
	ssize_t i;

	if (adapter->dir_dev == NULL) {
		rtw_warn_on(1);
		goto exit;
	}

	if (adapter->dir_odm != NULL) {
		rtw_warn_on(1);
		goto exit;
	}

	dir_odm = rtw_proc_create_dir("odm", adapter->dir_dev, dev);
	if (dir_odm == NULL) {
		rtw_warn_on(1);
		goto exit;
	}

	adapter->dir_odm = dir_odm;

	for (i = 0; i < odm_proc_hdls_num; i++) {
		if (odm_proc_hdls[i].type == RTW_PROC_HDL_TYPE_SEQ)
			entry = rtw_proc_create_entry(odm_proc_hdls[i].name, dir_odm, &rtw_odm_proc_seq_fops, (void *)i);
		else if (odm_proc_hdls[i].type == RTW_PROC_HDL_TYPE_SSEQ ||
			 odm_proc_hdls[i].type == RTW_PROC_HDL_TYPE_SZSEQ)
			entry = rtw_proc_create_entry(odm_proc_hdls[i].name, dir_odm, &rtw_odm_proc_sseq_fops, (void *)i);
		else
			entry = NULL;

		if (!entry) {
			rtw_warn_on(1);
			goto exit;
		}
	}

exit:
	return dir_odm;
}

void rtw_odm_proc_deinit(_adapter  *adapter)
{
	struct proc_dir_entry *dir_odm = NULL;
	int i;

	dir_odm = adapter->dir_odm;

	if (dir_odm == NULL) {
		rtw_warn_on(1);
		return;
	}

	for (i = 0; i < odm_proc_hdls_num; i++)
		remove_proc_entry(odm_proc_hdls[i].name, dir_odm);

	remove_proc_entry("odm", adapter->dir_dev);

	adapter->dir_odm = NULL;

	if (phydm_msg) {
		rtw_mfree(phydm_msg, phydm_msg_size);
		phydm_msg = NULL;
	}
}

#ifdef CONFIG_MCC_MODE
/*
* rtw_mcc_proc:
* init/deinit when register/unregister net_device, along with rtw_adapter_proc
*/
const struct rtw_proc_hdl mcc_proc_hdls[] = {
	RTW_PROC_HDL_SSEQ("mcc_info", proc_get_mcc_info, NULL),
	RTW_PROC_HDL_SSEQ("mcc_enable", proc_get_mcc_info, proc_set_mcc_enable),
	RTW_PROC_HDL_SSEQ("mcc_duration", proc_get_mcc_info, proc_set_mcc_duration),
	#ifdef CONFIG_MCC_PHYDM_OFFLOAD
	RTW_PROC_HDL_SSEQ("mcc_phydm_offload", proc_get_mcc_info, proc_set_mcc_phydm_offload_enable),
	#endif
	RTW_PROC_HDL_SSEQ("mcc_single_tx_criteria", proc_get_mcc_info, proc_set_mcc_single_tx_criteria),
	RTW_PROC_HDL_SSEQ("mcc_ap_bw20_target_tp", proc_get_mcc_info, proc_set_mcc_ap_bw20_target_tp),
	RTW_PROC_HDL_SSEQ("mcc_ap_bw40_target_tp", proc_get_mcc_info, proc_set_mcc_ap_bw40_target_tp),
	RTW_PROC_HDL_SSEQ("mcc_ap_bw80_target_tp", proc_get_mcc_info, proc_set_mcc_ap_bw80_target_tp),
	RTW_PROC_HDL_SSEQ("mcc_sta_bw20_target_tp", proc_get_mcc_info, proc_set_mcc_sta_bw20_target_tp),
	RTW_PROC_HDL_SSEQ("mcc_sta_bw40_target_tp", proc_get_mcc_info, proc_set_mcc_sta_bw40_target_tp),
	RTW_PROC_HDL_SSEQ("mcc_sta_bw80_target_tp", proc_get_mcc_info, proc_set_mcc_sta_bw80_target_tp),
	RTW_PROC_HDL_SSEQ("mcc_policy_table", proc_get_mcc_policy_table, NULL),
};

const int mcc_proc_hdls_num = sizeof(mcc_proc_hdls) / sizeof(struct rtw_proc_hdl);

static int rtw_mcc_proc_open(struct inode *inode, struct file *file)
{
	ssize_t index = (ssize_t)PDE_DATA(inode);
	const struct rtw_proc_hdl *hdl = mcc_proc_hdls + index;
	void *private = proc_get_parent_data(inode);

	if (hdl->type == RTW_PROC_HDL_TYPE_SEQ) {
		int res = seq_open(file, hdl->u.seq_op);

		if (res == 0)
			((struct seq_file *)file->private_data)->private = private;

		return res;
	} else if (hdl->type == RTW_PROC_HDL_TYPE_SSEQ) {
		int (*show)(struct seq_file *, void *) = hdl->u.show ? hdl->u.show : proc_get_dummy;

		return single_open(file, show, private);
	} else if (hdl->type == RTW_PROC_HDL_TYPE_SZSEQ) {
		int (*show)(struct seq_file *, void *) = hdl->u.sz.show ? hdl->u.sz.show : proc_get_dummy;

		#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
		return single_open_size(file, show, private, hdl->u.sz.size);
		#else
		return single_open(file, show, private);
		#endif
	} else {
		return -EROFS;
	}
}

static ssize_t rtw_mcc_proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *pos)
{
	ssize_t index = (ssize_t)PDE_DATA(file_inode(file));
	const struct rtw_proc_hdl *hdl = mcc_proc_hdls + index;
	ssize_t (*write)(struct file *, const char __user *, size_t, loff_t *, void *) = hdl->write;

	if (write)
		return write(file, buffer, count, pos, ((struct seq_file *)file->private_data)->private);

	return -EROFS;
}

static const struct rtw_proc_ops rtw_mcc_proc_seq_fops = {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0))
	.proc_open = rtw_mcc_proc_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
	.proc_write = rtw_mcc_proc_write,
#else
	.owner = THIS_MODULE,
	.open = rtw_mcc_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
	.write = rtw_mcc_proc_write,
#endif
};

static const struct rtw_proc_ops rtw_mcc_proc_sseq_fops = {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0))
	.proc_open = rtw_mcc_proc_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
	.proc_write = rtw_mcc_proc_write,
#else
	.owner = THIS_MODULE,
	.open = rtw_mcc_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
	.write = rtw_mcc_proc_write,
#endif
};

struct proc_dir_entry *rtw_mcc_proc_init(struct net_device *dev)
{
	struct proc_dir_entry *dir_mcc = NULL;
	struct proc_dir_entry *entry = NULL;
	_adapter	*adapter = rtw_netdev_priv(dev);
	ssize_t i;

	if (adapter->dir_dev == NULL) {
		rtw_warn_on(1);
		goto exit;
	}

	if (adapter->dir_mcc != NULL) {
		rtw_warn_on(1);
		goto exit;
	}

	dir_mcc = rtw_proc_create_dir("mcc", adapter->dir_dev, dev);
	if (dir_mcc == NULL) {
		rtw_warn_on(1);
		goto exit;
	}

	adapter->dir_mcc = dir_mcc;

	for (i = 0; i < mcc_proc_hdls_num; i++) {
		if (mcc_proc_hdls[i].type == RTW_PROC_HDL_TYPE_SEQ)
			entry = rtw_proc_create_entry(mcc_proc_hdls[i].name, dir_mcc, &rtw_mcc_proc_seq_fops, (void *)i);
		else if (mcc_proc_hdls[i].type == RTW_PROC_HDL_TYPE_SSEQ ||
			 mcc_proc_hdls[i].type == RTW_PROC_HDL_TYPE_SZSEQ)
			entry = rtw_proc_create_entry(mcc_proc_hdls[i].name, dir_mcc, &rtw_mcc_proc_sseq_fops, (void *)i);
		else
			entry = NULL;

		if (!entry) {
			rtw_warn_on(1);
			goto exit;
		}
	}

exit:
	return dir_mcc;
}

void rtw_mcc_proc_deinit(_adapter *adapter)
{
	struct proc_dir_entry *dir_mcc = NULL;
	int i;

	dir_mcc = adapter->dir_mcc;

	if (dir_mcc == NULL) {
		rtw_warn_on(1);
		return;
	}

	for (i = 0; i < mcc_proc_hdls_num; i++)
		remove_proc_entry(mcc_proc_hdls[i].name, dir_mcc);

	remove_proc_entry("mcc", adapter->dir_dev);

	adapter->dir_mcc = NULL;
}
#endif /* CONFIG_MCC_MODE */

struct proc_dir_entry *rtw_adapter_proc_init(struct net_device *dev)
{
	struct proc_dir_entry *drv_proc = get_rtw_drv_proc();
	struct proc_dir_entry *dir_dev = NULL;
	struct proc_dir_entry *entry = NULL;
	_adapter *adapter = rtw_netdev_priv(dev);
	ssize_t i;

	if (drv_proc == NULL) {
		rtw_warn_on(1);
		goto exit;
	}

	if (adapter->dir_dev != NULL) {
		rtw_warn_on(1);
		goto exit;
	}

	dir_dev = rtw_proc_create_dir(dev->name, drv_proc, dev);
	if (dir_dev == NULL) {
		rtw_warn_on(1);
		goto exit;
	}

	adapter->dir_dev = dir_dev;

	for (i = 0; i < adapter_proc_hdls_num; i++) {
		if (adapter_proc_hdls[i].type == RTW_PROC_HDL_TYPE_SEQ)
			entry = rtw_proc_create_entry(adapter_proc_hdls[i].name, dir_dev, &rtw_adapter_proc_seq_fops, (void *)i);
		else if (adapter_proc_hdls[i].type == RTW_PROC_HDL_TYPE_SSEQ ||
			 adapter_proc_hdls[i].type == RTW_PROC_HDL_TYPE_SZSEQ)
			entry = rtw_proc_create_entry(adapter_proc_hdls[i].name, dir_dev, &rtw_adapter_proc_sseq_fops, (void *)i);
		else
			entry = NULL;

		if (!entry) {
			rtw_warn_on(1);
			goto exit;
		}
	}

	rtw_odm_proc_init(dev);

#ifdef CONFIG_MCC_MODE
	rtw_mcc_proc_init(dev);
#endif /* CONFIG_MCC_MODE */

exit:
	return dir_dev;
}

void rtw_adapter_proc_deinit(struct net_device *dev)
{
	struct proc_dir_entry *drv_proc = get_rtw_drv_proc();
	struct proc_dir_entry *dir_dev = NULL;
	_adapter *adapter = rtw_netdev_priv(dev);
	int i;

	dir_dev = adapter->dir_dev;

	if (dir_dev == NULL) {
		rtw_warn_on(1);
		return;
	}

	for (i = 0; i < adapter_proc_hdls_num; i++)
		remove_proc_entry(adapter_proc_hdls[i].name, dir_dev);

	rtw_odm_proc_deinit(adapter);

#ifdef CONFIG_MCC_MODE
	rtw_mcc_proc_deinit(adapter);
#endif /* CONFIG_MCC_MODE */

	remove_proc_entry(dev->name, drv_proc);

	adapter->dir_dev = NULL;
}

void rtw_adapter_proc_replace(struct net_device *dev)
{
	struct proc_dir_entry *drv_proc = get_rtw_drv_proc();
	struct proc_dir_entry *dir_dev = NULL;
	_adapter *adapter = rtw_netdev_priv(dev);
	int i;

	dir_dev = adapter->dir_dev;

	if (dir_dev == NULL) {
		rtw_warn_on(1);
		return;
	}

	for (i = 0; i < adapter_proc_hdls_num; i++)
		remove_proc_entry(adapter_proc_hdls[i].name, dir_dev);

	rtw_odm_proc_deinit(adapter);

#ifdef CONFIG_MCC_MODE
	rtw_mcc_proc_deinit(adapter);
#endif /* CONIG_MCC_MODE */

	remove_proc_entry(adapter->old_ifname, drv_proc);

	adapter->dir_dev = NULL;

	rtw_adapter_proc_init(dev);

}

#endif /* PLATFORM_ECOS */
#endif /* CONFIG_PROC_DEBUG */
