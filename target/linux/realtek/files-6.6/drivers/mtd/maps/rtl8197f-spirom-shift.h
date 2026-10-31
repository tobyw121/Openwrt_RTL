/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef _RTL8197F_SPIROM_SHIFT_H
#define _RTL8197F_SPIROM_SHIFT_H

#include <linux/errno.h>
#include <linux/types.h>

/* Each seven-byte window starts three bytes before its expected magic.
 * Require both independent signatures and exactly one candidate shift.
 */
static inline int rtl8197f_spirom_select_shift(const u8 windows[2][7],
					     const u8 magic[2][4])
{
	int shift, probe, byte, selected = -ENODEV;

	for (shift = 0; shift <= 3; shift++) {
		bool match = true;

		for (probe = 0; probe < 2; probe++)
			for (byte = 0; byte < 4; byte++)
				if (windows[probe][3 - shift + byte] != magic[probe][byte])
					match = false;
		if (!match)
			continue;
		if (selected >= 0)
			return -EINVAL;
		selected = shift;
	}
	return selected;
}
#endif
