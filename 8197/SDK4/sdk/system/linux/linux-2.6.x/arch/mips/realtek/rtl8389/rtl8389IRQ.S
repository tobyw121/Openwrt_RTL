/*
 * Copyright (C) 2009 Realtek Semiconductor Corp.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * $Revision: 6226 $
 * $Date: 2009-10-01 18:07:30 +0800 (Thu, 01 Oct 2009) $
 *
 */
 
#include <linux/config.h>
#include <asm/asm.h>
#include <asm/mipsregs.h>
#include <asm/regdef.h>
#include <asm/stackframe.h>

/*
 *   RTL8389 IRQ        Source
 *   -----------        ------
 *             0        Software (ignored)
 *             1        Software (ignored)
 *             2        Hardware
 *             3        Hardware
 *             4        Hardware
 *             5        Hardware
 *             6        Hardware
 *             7        Hardware
 */

   .text
   .set    noreorder
   .set    noat
   .align  5
   NESTED(rtl8389IRQ, PT_SIZE, sp)
   SAVE_ALL
   CLI
   .set    at
   move a0, sp
   jal  irq_dispatch
   nop                          # delay slot
   j    ret_from_irq
   nop                          # delay slot
irq_bad:
   j    irq_bad
   nop
   END(rtl8389IRQ)

