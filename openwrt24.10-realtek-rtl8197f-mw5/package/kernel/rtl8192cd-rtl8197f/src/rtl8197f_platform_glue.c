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
MODULE_PARM_DESC(board_profile, "auto, mw5, rd05 or ac23 (diagnostic/profile selector)");

static bool external_radio = true;
module_param(external_radio, bool, 0644);
MODULE_PARM_DESC(external_radio, "Allow PCIe external-radio registration");

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

	pr_warn("rtl8197f-wlan: unknown board, defaulting bond option to 97FB\n");
	return BSP_BOND_97FB;
}

bool rtl8197f_wlan_external_enabled(void)
{
	return external_radio;
}

void rtl8197f_wlan_phase_set(int phase, const char *name, int rc)
{
	init_phase = phase;
	init_rc = rc;
	strscpy(last_phase, name ? name : "unnamed", sizeof(last_phase));
	pr_info("rtl8197f-wlan: phase=%d name=%s rc=%d profile=%s bond=%u external=%u\n",
		phase, last_phase, rc, rtl8197f_detect_profile(),
		rtl819x_bond_option(), external_radio);
}

MODULE_DESCRIPTION("RTL8197F RD05/MW5/AC23 rtl8192cd platform glue");
MODULE_LICENSE("GPL");

int (*RTLWIFINIC_GPIO_read_ptr)(unsigned int);
void (*RTLWIFINIC_GPIO_write_ptr)(unsigned int, unsigned int);
