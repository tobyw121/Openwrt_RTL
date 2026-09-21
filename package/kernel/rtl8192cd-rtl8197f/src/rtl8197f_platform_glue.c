// SPDX-License-Identifier: GPL-2.0
/* RTL8197F family compatibility and bring-up telemetry for rtl8192cd. */
#include <linux/module.h>
#include <linux/of.h>
#include <linux/string.h>
#include <linux/types.h>

static int bond_option = -1;
module_param(bond_option, int, 0644);
MODULE_PARM_DESC(bond_option, "Override Realtek bond option (-1=derive from DT)");

static char board_profile[24] = "auto";
module_param_string(board_profile, board_profile, sizeof(board_profile), 0644);
MODULE_PARM_DESC(board_profile, "auto, mw5, rd05, ac23 or wrd12gn (diagnostic/profile selector)");

static bool external_radio = true;
module_param(external_radio, bool, 0644);
MODULE_PARM_DESC(external_radio, "Allow PCIe external-radio registration");

/* Legacy diagnostic option from v43.38.  The OEM attachments show that
 * MW5 normally uses wlan0=external RTL8822B/5GHz and wlan1=integrated
 * RTL8197FS/2.4GHz, so the new default 2G path does NOT use this.
 */
static bool integrated_first = false;
module_param(integrated_first, bool, 0644);
MODULE_PARM_DESC(integrated_first, "Legacy diagnostic: initialize embedded RTL8197FS as wlan_device[0]");

/* v43.43: follow MW5 OEM order for the 2.4 GHz bring-up.
 * The OEM snapshots show wlan0=RTL8822B/5GHz/IRQ5/RFE6 and
 * wlan1=RTL8197FS/2.4GHz/IRQ6/RFE5.  This option keeps the forced table in
 * that order, skips every non-2G entry, and expects the embedded radio to
 * register as wlan1.
 */
static bool oem_2g_only = false;
module_param(oem_2g_only, bool, 0644);
MODULE_PARM_DESC(oem_2g_only, "MW5 OEM-order diagnostic: only initialize wlan_device[1] as wlan1 2.4GHz");

/* Avoid creating MBSSID/VXD helper interfaces during the first low-level
 * hardware test.  Root netdev first, then add extras only after stable proof.
 */
static bool root_only = false;
module_param(root_only, bool, 0644);
MODULE_PARM_DESC(root_only, "Initialize only the root WLAN interface, skip MBSSID/VXD");

/* v43.43: hard-hang isolation.  Earlier MW5 tests hung before modprobe returned.
 * probe_only lets us verify that module insertion and parameter plumbing work
 * without touching the RTL8197FS/RTL8822B hardware init path.
 * skip_init_one lets the next diagnostic stage scan the device table but avoid
 * rtl8192cd_init_one(), which is currently the first suspected hard-hang point.
 */
static bool probe_only = false;
module_param(probe_only, bool, 0644);
MODULE_PARM_DESC(probe_only, "Diagnostic: return from module init before hardware bring-up");

static bool skip_init_one = false;
module_param(skip_init_one, bool, 0644);
MODULE_PARM_DESC(skip_init_one, "Diagnostic: scan device table but skip rtl8192cd_init_one hardware init");

static int stop_after_phase = -1;
module_param(stop_after_phase, int, 0644);
MODULE_PARM_DESC(stop_after_phase, "Diagnostic: stop successfully after the requested init phase (-1=disabled)");

static int init_phase;
module_param(init_phase, int, 0444);
MODULE_PARM_DESC(init_phase, "Last completed RTL8197F WLAN initialization phase");

static int init_rc;
module_param(init_rc, int, 0444);
MODULE_PARM_DESC(init_rc, "Return code recorded for the last WLAN phase");

static char last_phase[64] = "not-started";
module_param_string(last_phase, last_phase, sizeof(last_phase), 0444);
MODULE_PARM_DESC(last_phase, "Human-readable last WLAN initialization phase");

static const char *rtl8197f_detect_profile(void)
{
	if (strcmp(board_profile, "auto"))
		return board_profile;
	if (of_machine_is_compatible("tenda,nova-mw5"))
		return "mw5";
	if (of_machine_is_compatible("xiaomi,r4-rd05"))
		return "rd05";
	if (of_machine_is_compatible("tenda,ac23") ||
	    of_machine_is_compatible("tenda,lynx-rtl8197f"))
		return "ac23";
	if (of_machine_is_compatible("iball,wrd12gn"))
		return "wrd12gn";
	return "unknown";
}

unsigned int rtl819x_bond_option(void)
{
	const char *profile;

	if (bond_option >= 0)
		return bond_option;

	profile = rtl8197f_detect_profile();
	if (!strcmp(profile, "mw5"))
		return BSP_BOND_97FS;

	/* RD05 is RTL8197FH-VG and AC23 is listed as RTL8197FH.  The GPL BSP
	 * handles those non-FS packages through the 97FB/FH bond path.
	 */
	if (!strcmp(profile, "rd05") || !strcmp(profile, "ac23"))
		return BSP_BOND_97FB;
	if (!strcmp(profile, "wrd12gn"))
		return BSP_BOND_97FN;

	pr_warn("rtl8197f-wlan: unknown board, defaulting bond option to 97FB\n");
	return BSP_BOND_97FB;
}

bool rtl8197f_wlan_external_enabled(void)
{
	return external_radio;
}

bool rtl8197f_wlan_integrated_first(void)
{
	return integrated_first;
}

bool rtl8197f_wlan_oem_2g_only(void)
{
	return oem_2g_only;
}

bool rtl8197f_wlan_root_only(void)
{
	return root_only;
}

bool rtl8197f_wlan_probe_only(void)
{
	return probe_only;
}

bool rtl8197f_wlan_skip_init_one(void)
{
	return skip_init_one;
}

bool rtl8197f_wlan_stop_after(int phase)
{
	return stop_after_phase >= 0 && phase >= stop_after_phase;
}

void rtl8197f_wlan_phase_set(int phase, const char *name, int rc)
{
	init_phase = phase;
	init_rc = rc;
	strscpy(last_phase, name ? name : "unnamed", sizeof(last_phase));
	/* KERN_EMERG is intentional during bring-up: the box can hard-hang before
	 * userspace can read dmesg, so the phase must hit the serial console.
	 */
	printk(KERN_EMERG "rtl8197f-wlan: phase=%d name=%s rc=%d profile=%s bond=%u external=%u integrated_first=%u oem_2g_only=%u root_only=%u probe_only=%u skip_init_one=%u stop_after=%d\n",
		phase, last_phase, rc, rtl8197f_detect_profile(),
		rtl819x_bond_option(), external_radio, integrated_first, oem_2g_only,
		root_only, probe_only, skip_init_one, stop_after_phase);
}

MODULE_DESCRIPTION("RTL8197F RD05/MW5/AC23/WRD12GN rtl8192cd platform glue");
MODULE_LICENSE("GPL");

int (*RTLWIFINIC_GPIO_read_ptr)(unsigned int);
void (*RTLWIFINIC_GPIO_write_ptr)(unsigned int, unsigned int);
