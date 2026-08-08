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
 * $Revision: 6401 $
 * $Date: 2009-10-14 16:03:12 +0800 (Wed, 14 Oct 2009) $
 *
 * Purpose : A Linux Ethernet driver for the Realtek Switch SOC.
 *
 * Feature : NIC module
 *
 */

#ifndef __RTNIC_DRV_H__
#define __RTNIC_DRV_H__

/*
 * Include Files
 */

/*
 * Symbol Definition
 */
#define RTNIC_UNIT_ID       (0)
#define RTNIC_TX_TIMEOUT    (10*HZ)
#define RTNIC_MAX_PKTLEN    (1600)
#define RTNIC_PKTLEN_RSVD   (2)     /* Reserve 2 bytes for IP alignment */

/*
 * Data Declaration
 */

/*
 * Macro Declaration
 */

/*
 * Function Declaration
 */

#endif  /*__RTNIC_DRV_H__*/

