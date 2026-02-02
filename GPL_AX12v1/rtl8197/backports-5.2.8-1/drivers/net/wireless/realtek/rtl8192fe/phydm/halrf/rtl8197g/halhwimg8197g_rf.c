/******************************************************************************
 *
 * Copyright(c) 2007 - 2017 Realtek Corporation.
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
 * The full GNU General Public License is included in this distribution in the
 * file called LICENSE.
 *
 * Contact Information:
 * wlanfae <wlanfae@realtek.com>
 * Realtek Corporation, No. 2, Innovation Road II, Hsinchu Science Park,
 * Hsinchu 300, Taiwan.
 *
 * Larry Finger <Larry.Finger@lwfinger.net>
 *
 *****************************************************************************/

/*Image2HeaderVersion: R3 1.5.5*/
#include "mp_precomp.h"

#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
#if RT_PLATFORM == PLATFORM_MACOSX
#include "phydm_precomp.h"
#else
#include "../phydm_precomp.h"
#endif
#else
#include "../../phydm_precomp.h"
#endif

#define D_S_SIZE DELTA_SWINGIDX_SIZE
#define D_ST_SIZE DELTA_SWINTSSI_SIZE

#if (RTL8197G_SUPPORT == 1)
static boolean
check_positive(struct dm_struct *dm,
	       const u32	condition1,
	       const u32	condition2,
	       const u32	condition3,
	       const u32	condition4
)
{
	u32	cond1 = condition1, cond2 = condition2,
		cond3 = condition3, cond4 = condition4;

	u8	cut_version_for_para =
		(dm->cut_version ==  ODM_CUT_A) ? 15 : dm->cut_version;

	u8	pkg_type_for_para =
		(dm->package_type == 0) ? 15 : dm->package_type;

	u32	driver1 = cut_version_for_para << 24 |
			(dm->support_interface & 0xF0) << 16 |
			dm->support_platform << 16 |
			pkg_type_for_para << 12 |
			(dm->support_interface & 0x0F) << 8  |
			dm->rfe_type;

	u32	driver2 = (dm->type_glna & 0xFF) <<  0 |
			(dm->type_gpa & 0xFF)  <<  8 |
			(dm->type_alna & 0xFF) << 16 |
			(dm->type_apa & 0xFF)  << 24;

	u32	driver3 = 0;

	u32	driver4 = (dm->type_glna & 0xFF00) >>  8 |
			(dm->type_gpa & 0xFF00) |
			(dm->type_alna & 0xFF00) << 8 |
			(dm->type_apa & 0xFF00)  << 16;

	PHYDM_DBG(dm, ODM_COMP_INIT,
		  "===> %s (cond1, cond2, cond3, cond4) = (0x%X 0x%X 0x%X 0x%X)\n",
		  __func__, cond1, cond2, cond3, cond4);
	PHYDM_DBG(dm, ODM_COMP_INIT,
		  "===> %s (driver1, driver2, driver3, driver4) = (0x%X 0x%X 0x%X 0x%X)\n",
		  __func__, driver1, driver2, driver3, driver4);

	PHYDM_DBG(dm, ODM_COMP_INIT,
		  "	(Platform, Interface) = (0x%X, 0x%X)\n",
		  dm->support_platform, dm->support_interface);
	PHYDM_DBG(dm, ODM_COMP_INIT, "	(RFE, Package) = (0x%X, 0x%X)\n",
		  dm->rfe_type, dm->package_type);

	/*============== value Defined Check ===============*/
	/*cut version [27:24] need to do value check*/
	if (((cond1 & 0x0F000000) != 0) &&
	    ((cond1 & 0x0F000000) != (driver1 & 0x0F000000)))
		return false;

	/*pkg type [15:12] need to do value check*/
	if (((cond1 & 0x0000F000) != 0) &&
	    ((cond1 & 0x0000F000) != (driver1 & 0x0000F000)))
		return false;

	/*interface [11:8] need to do value check*/
	if (((cond1 & 0x00000F00) != 0) &&
	    ((cond1 & 0x00000F00) != (driver1 & 0x00000F00)))
		return false;
	/*=============== Bit Defined Check ================*/
	/* We don't care [31:28] */

	cond1 &= 0x000000FF;
	driver1 &= 0x000000FF;

	if (cond1 == driver1)
		return true;
	else
		return false;
}


/******************************************************************************
 *                           radioa.TXT
 ******************************************************************************/

const u32 array_mp_8197g_radioa[] = {
		0x000, 0x00030000,
		0x018, 0x00000C07,
		0x0CA, 0x00002000,
		0x088, 0x0004B0CC,
		0x085, 0x000910D5,
		0x086, 0x00033B1F,
		0x0EF, 0x00000002,
		0x033, 0x00000300,
		0x033, 0x00008393,
		0x033, 0x00010393,
		0x033, 0x00018300,
		0x033, 0x00020393,
		0x033, 0x00028393,
		0x033, 0x00030393,
		0x033, 0x00038393,
		0x033, 0x00040393,
		0x033, 0x00048393,
		0x033, 0x00050393,
		0x033, 0x00058393,
		0x033, 0x00060393,
		0x033, 0x000383D5,
		0x033, 0x000403D5,
		0x0EF, 0x00000000,
		0x0EF, 0x00002000,
		0x033, 0x00000003,
		0x034, 0x00000003,
		0x033, 0x00004403,
		0x034, 0x00000003,
		0x033, 0x00008803,
		0x034, 0x00000003,
		0x033, 0x0000CC03,
		0x034, 0x00000003,
		0x033, 0x00010103,
		0x034, 0x00000003,
		0x033, 0x00015103,
		0x034, 0x00000003,
		0x033, 0x0001A103,
		0x034, 0x00000003,
		0x033, 0x0001F10F,
		0x034, 0x00000003,
		0x033, 0x00020003,
		0x034, 0x00000003,
		0x033, 0x00024403,
		0x034, 0x00000003,
		0x033, 0x00028803,
		0x034, 0x00000003,
		0x033, 0x0002CC03,
		0x034, 0x00000003,
		0x033, 0x00030103,
		0x034, 0x00000003,
		0x033, 0x00035103,
		0x034, 0x00000003,
		0x033, 0x0003A103,
		0x034, 0x00000003,
		0x033, 0x0003F10F,
		0x034, 0x00000003,
		0x033, 0x00040003,
		0x034, 0x00000003,
		0x033, 0x00044403,
		0x034, 0x00000003,
		0x033, 0x00048803,
		0x034, 0x00000003,
		0x033, 0x0004CC03,
		0x034, 0x00000003,
		0x033, 0x00050103,
		0x034, 0x00000003,
		0x033, 0x00055103,
		0x034, 0x00000003,
		0x033, 0x0005A103,
		0x034, 0x00000003,
		0x033, 0x0005F10F,
		0x034, 0x00000003,
		0x0EF, 0x00000000,
		0x08E, 0x000645E0,
		0x08F, 0x000A0B3A,
		0x051, 0x0002A2A5,
		0x052, 0x00001347,
		0x053, 0x00000CC1,
		0x054, 0x00003437,
		0x056, 0x0009DDEB,
		0x057, 0x0004CE26,
		0x058, 0x0000EF80,
		0x05A, 0x00050000,
		0x05B, 0x0002A000,
		0x0EF, 0x00000400,
	0x81000002,	0x00000000,	0x40000000,	0x00000000,
		0x033, 0x0000067C,
		0x034, 0x00000126,
		0x033, 0x0000467C,
		0x034, 0x00000126,
		0x033, 0x0000867C,
		0x034, 0x00000126,
		0x033, 0x0000C67C,
		0x034, 0x00000126,
	0xA0000000,	0x00000000,
		0x033, 0x0000057C,
		0x034, 0x0000012A,
		0x033, 0x0000457C,
		0x034, 0x0000012A,
		0x033, 0x0000857C,
		0x034, 0x0000012A,
		0x033, 0x0000C57C,
		0x034, 0x0000012A,
	0xB0000000,	0x00000000,
	0x81000002,	0x00000000,	0x40000000,	0x00000000,
		0x033, 0x0001067C,
		0x034, 0x00000126,
	0xA0000000,	0x00000000,
		0x033, 0x0001057C,
		0x034, 0x0000012A,
	0xB0000000,	0x00000000,
	0x81000002,	0x00000000,	0x40000000,	0x00000000,
		0x033, 0x0001467C,
		0x034, 0x00000126,
	0xA0000000,	0x00000000,
		0x033, 0x0001457C,
		0x034, 0x0000012A,
	0xB0000000,	0x00000000,
	0x81000002,	0x00000000,	0x40000000,	0x00000000,
		0x033, 0x0001867C,
		0x034, 0x00000126,
	0xA0000000,	0x00000000,
		0x033, 0x0001857C,
		0x034, 0x0000012A,
	0xB0000000,	0x00000000,
	0x81000002,	0x00000000,	0x40000000,	0x00000000,
		0x033, 0x0001C67C,
		0x034, 0x00000126,
	0xA0000000,	0x00000000,
		0x033, 0x0001C57C,
		0x034, 0x0000012A,
	0xB0000000,	0x00000000,
	0x81000002,	0x00000000,	0x40000000,	0x00000000,
		0x033, 0x0002067C,
		0x034, 0x00000126,
	0xA0000000,	0x00000000,
		0x033, 0x0002057C,
		0x034, 0x0000012F,
	0xB0000000,	0x00000000,
	0x81000002,	0x00000000,	0x40000000,	0x00000000,
		0x033, 0x0002467C,
		0x034, 0x00000126,
	0xA0000000,	0x00000000,
		0x033, 0x0002457C,
		0x034, 0x0000012F,
	0xB0000000,	0x00000000,
	0x81000002,	0x00000000,	0x40000000,	0x00000000,
		0x033, 0x0002867C,
		0x034, 0x00000126,
	0xA0000000,	0x00000000,
		0x033, 0x0002857C,
		0x034, 0x0000012F,
	0xB0000000,	0x00000000,
	0x81000002,	0x00000000,	0x40000000,	0x00000000,
		0x033, 0x0002C67C,
		0x034, 0x00000126,
	0xA0000000,	0x00000000,
		0x033, 0x0002C57C,
		0x034, 0x0000012F,
	0xB0000000,	0x00000000,
	0x81000002,	0x00000000,	0x40000000,	0x00000000,
		0x033, 0x0003067C,
		0x034, 0x00000126,
	0xA0000000,	0x00000000,
		0x033, 0x0003057C,
		0x034, 0x0000012F,
	0xB0000000,	0x00000000,
	0x81000002,	0x00000000,	0x40000000,	0x00000000,
		0x033, 0x0003467C,
		0x034, 0x00000126,
	0xA0000000,	0x00000000,
		0x033, 0x0003457C,
		0x034, 0x0000012F,
	0xB0000000,	0x00000000,
	0x81000002,	0x00000000,	0x40000000,	0x00000000,
		0x033, 0x0003867C,
		0x034, 0x00000126,
	0xA0000000,	0x00000000,
		0x033, 0x0003857C,
		0x034, 0x0000012F,
	0xB0000000,	0x00000000,
	0x81000002,	0x00000000,	0x40000000,	0x00000000,
		0x033, 0x0003C67C,
		0x034, 0x00000126,
	0xA0000000,	0x00000000,
		0x033, 0x0003C57C,
		0x034, 0x0000012F,
	0xB0000000,	0x00000000,
	0x81000002,	0x00000000,	0x40000000,	0x00000000,
		0x033, 0x0004067C,
		0x034, 0x00000126,
	0xA0000000,	0x00000000,
		0x033, 0x0004057C,
		0x034, 0x0000012F,
	0xB0000000,	0x00000000,
	0x81000002,	0x00000000,	0x40000000,	0x00000000,
		0x033, 0x0004467C,
		0x034, 0x00000126,
	0xA0000000,	0x00000000,
		0x033, 0x0004457C,
		0x034, 0x0000012F,
	0xB0000000,	0x00000000,
	0x81000002,	0x00000000,	0x40000000,	0x00000000,
		0x033, 0x0004867C,
		0x034, 0x00000126,
	0xA0000000,	0x00000000,
		0x033, 0x0004857C,
		0x034, 0x0000012F,
	0xB0000000,	0x00000000,
	0x81000002,	0x00000000,	0x40000000,	0x00000000,
		0x033, 0x0004C67C,
		0x034, 0x00000126,
	0xA0000000,	0x00000000,
		0x033, 0x0004C57C,
		0x034, 0x0000012F,
	0xB0000000,	0x00000000,
	0x81000002,	0x00000000,	0x40000000,	0x00000000,
		0x033, 0x0005067C,
		0x034, 0x00000126,
	0xA0000000,	0x00000000,
		0x033, 0x0005057C,
		0x034, 0x0000012A,
	0xB0000000,	0x00000000,
	0x81000002,	0x00000000,	0x40000000,	0x00000000,
		0x033, 0x0005467C,
		0x034, 0x00000126,
	0xA0000000,	0x00000000,
		0x033, 0x0005457C,
		0x034, 0x0000012A,
	0xB0000000,	0x00000000,
	0x81000002,	0x00000000,	0x40000000,	0x00000000,
		0x033, 0x0005867C,
		0x034, 0x00000126,
	0xA0000000,	0x00000000,
		0x033, 0x0005857C,
		0x034, 0x0000012A,
	0xB0000000,	0x00000000,
	0x81000002,	0x00000000,	0x40000000,	0x00000000,
		0x033, 0x0005C67C,
		0x034, 0x00000126,
	0xA0000000,	0x00000000,
		0x033, 0x0005C57C,
		0x034, 0x0000012A,
	0xB0000000,	0x00000000,
	0x81000002,	0x00000000,	0x40000000,	0x00000000,
		0x033, 0x0006067C,
		0x034, 0x00000126,
	0xA0000000,	0x00000000,
		0x033, 0x0006057C,
		0x034, 0x0000012F,
	0xB0000000,	0x00000000,
	0x81000002,	0x00000000,	0x40000000,	0x00000000,
		0x033, 0x0006467C,
		0x034, 0x00000126,
	0xA0000000,	0x00000000,
		0x033, 0x0006457C,
		0x034, 0x0000012F,
	0xB0000000,	0x00000000,
	0x81000002,	0x00000000,	0x40000000,	0x00000000,
		0x033, 0x0006867C,
		0x034, 0x00000126,
	0xA0000000,	0x00000000,
		0x033, 0x0006857C,
		0x034, 0x0000012F,
	0xB0000000,	0x00000000,
	0x81000002,	0x00000000,	0x40000000,	0x00000000,
		0x033, 0x0006C67C,
		0x034, 0x00000126,
	0xA0000000,	0x00000000,
		0x033, 0x0006C57C,
		0x034, 0x0000012F,
	0xB0000000,	0x00000000,
	0x81000002,	0x00000000,	0x40000000,	0x00000000,
		0x033, 0x0007067C,
		0x034, 0x00000126,
	0xA0000000,	0x00000000,
		0x033, 0x0007057C,
		0x034, 0x0000012F,
	0xB0000000,	0x00000000,
	0x81000002,	0x00000000,	0x40000000,	0x00000000,
		0x033, 0x0007467C,
		0x034, 0x00000126,
	0xA0000000,	0x00000000,
		0x033, 0x0007457C,
		0x034, 0x0000012F,
	0xB0000000,	0x00000000,
	0x81000002,	0x00000000,	0x40000000,	0x00000000,
		0x033, 0x0007867C,
		0x034, 0x00000126,
	0xA0000000,	0x00000000,
		0x033, 0x0007857C,
		0x034, 0x0000012F,
	0xB0000000,	0x00000000,
	0x81000002,	0x00000000,	0x40000000,	0x00000000,
		0x033, 0x0007C67C,
		0x034, 0x00000126,
	0xA0000000,	0x00000000,
		0x033, 0x0007C57C,
		0x034, 0x0000012F,
	0xB0000000,	0x00000000,
	0x81000002,	0x00000000,	0x40000000,	0x00000000,
		0x033, 0x0008067C,
		0x034, 0x00000126,
	0xA0000000,	0x00000000,
		0x033, 0x0008057C,
		0x034, 0x0000012A,
	0xB0000000,	0x00000000,
	0x81000002,	0x00000000,	0x40000000,	0x00000000,
		0x033, 0x0008467C,
		0x034, 0x00000126,
	0xA0000000,	0x00000000,
		0x033, 0x0008457C,
		0x034, 0x0000012A,
	0xB0000000,	0x00000000,
	0x81000002,	0x00000000,	0x40000000,	0x00000000,
		0x033, 0x0008867C,
		0x034, 0x00000126,
	0xA0000000,	0x00000000,
		0x033, 0x0008857C,
		0x034, 0x0000012A,
	0xB0000000,	0x00000000,
	0x81000002,	0x00000000,	0x40000000,	0x00000000,
		0x033, 0x0008C67C,
		0x034, 0x00000126,
	0xA0000000,	0x00000000,
		0x033, 0x0008C57C,
		0x034, 0x0000012A,
	0xB0000000,	0x00000000,
	0x81000002,	0x00000000,	0x40000000,	0x00000000,
		0x033, 0x0009067C,
		0x034, 0x00000126,
	0xA0000000,	0x00000000,
		0x033, 0x0009057C,
		0x034, 0x0000012A,
	0xB0000000,	0x00000000,
	0x81000002,	0x00000000,	0x40000000,	0x00000000,
		0x033, 0x0009467C,
		0x034, 0x00000126,
	0xA0000000,	0x00000000,
		0x033, 0x0009457C,
		0x034, 0x0000012A,
	0xB0000000,	0x00000000,
	0x81000002,	0x00000000,	0x40000000,	0x00000000,
		0x033, 0x0009867C,
		0x034, 0x00000126,
	0xA0000000,	0x00000000,
		0x033, 0x0009857C,
		0x034, 0x0000012A,
	0xB0000000,	0x00000000,
	0x81000002,	0x00000000,	0x40000000,	0x00000000,
		0x033, 0x0009C67C,
		0x034, 0x00000126,
	0xA0000000,	0x00000000,
		0x033, 0x0009C57C,
		0x034, 0x0000012A,
	0xB0000000,	0x00000000,
	0x81000002,	0x00000000,	0x40000000,	0x00000000,
		0x033, 0x000A067C,
		0x034, 0x00000126,
	0xA0000000,	0x00000000,
		0x033, 0x000A057C,
		0x034, 0x0000012A,
	0xB0000000,	0x00000000,
	0x81000002,	0x00000000,	0x40000000,	0x00000000,
		0x033, 0x000A467C,
		0x034, 0x00000126,
	0xA0000000,	0x00000000,
		0x033, 0x000A457C,
		0x034, 0x0000012A,
	0xB0000000,	0x00000000,
	0x81000002,	0x00000000,	0x40000000,	0x00000000,
		0x033, 0x000A867C,
		0x034, 0x00000126,
	0xA0000000,	0x00000000,
		0x033, 0x000A857C,
		0x034, 0x0000012A,
	0xB0000000,	0x00000000,
	0x81000002,	0x00000000,	0x40000000,	0x00000000,
		0x033, 0x000AC67C,
		0x034, 0x00000126,
	0xA0000000,	0x00000000,
		0x033, 0x000AC57C,
		0x034, 0x0000012A,
	0xB0000000,	0x00000000,
	0x81000002,	0x00000000,	0x40000000,	0x00000000,
		0x033, 0x000B067C,
		0x034, 0x00000126,
	0xA0000000,	0x00000000,
		0x033, 0x000B057C,
		0x034, 0x0000012A,
	0xB0000000,	0x00000000,
		0x0EF, 0x00000000,
	0x81000001,	0x00000000,	0x40000000,	0x00000000,
		0x0EE, 0x00000008,
		0x033, 0x00028039,
		0x033, 0x00024036,
		0x033, 0x00020033,
		0x033, 0x0001C030,
		0x033, 0x0001802D,
		0x033, 0x0001402A,
		0x033, 0x00010027,
		0x033, 0x0000C024,
		0x033, 0x00008007,
		0x033, 0x00004004,
		0x033, 0x00000001,
		0x0EE, 0x00000000,
	0x91000005,	0x00000000,	0x40000000,	0x00000000,
		0x0EE, 0x00000008,
		0x033, 0x00028039,
		0x033, 0x00024036,
		0x033, 0x00020033,
		0x033, 0x0001C030,
		0x033, 0x0001802D,
		0x033, 0x0001402A,
		0x033, 0x00010027,
		0x033, 0x0000C024,
		0x033, 0x00008007,
		0x033, 0x00004004,
		0x033, 0x00000001,
		0x0EE, 0x00000000,
	0xA0000000,	0x00000000,
		0x0EE, 0x00000008,
		0x033, 0x000280F8,
		0x033, 0x000240F5,
		0x033, 0x000200F2,
		0x033, 0x0001C0EF,
		0x033, 0x000180EC,
		0x033, 0x000140E9,
		0x033, 0x000100E6,
		0x033, 0x0000C0E3,
		0x033, 0x00008066,
		0x033, 0x00004063,
		0x033, 0x00000043,
		0x0EE, 0x00000000,
	0xB0000000,	0x00000000,
	0x81000001,	0x00000000,	0x40000000,	0x00000000,
		0x0EE, 0x00000004,
		0x033, 0x000283E1,
		0x033, 0x000241E7,
		0x033, 0x000201E1,
		0x033, 0x0001C1DB,
		0x033, 0x000181D5,
		0x033, 0x000141CF,
		0x033, 0x000101C9,
		0x033, 0x0000C0CF,
		0x033, 0x000080C9,
		0x033, 0x0000404F,
		0x033, 0x00000049,
		0x0EE, 0x00000000,
	0x91000005,	0x00000000,	0x40000000,	0x00000000,
		0x0EE, 0x00000004,
		0x033, 0x000283E1,
		0x033, 0x000241E7,
		0x033, 0x000201E1,
		0x033, 0x0001C1DB,
		0x033, 0x000181D5,
		0x033, 0x000141CF,
		0x033, 0x000101C9,
		0x033, 0x0000C0CF,
		0x033, 0x000080C9,
		0x033, 0x0000404F,
		0x033, 0x00000049,
		0x0EE, 0x00000000,
	0x91000006,	0x00000000,	0x40000000,	0x00000000,
		0x0EE, 0x00000004,
		0x033, 0x000287EF,
		0x033, 0x000243EF,
		0x033, 0x000203E9,
		0x033, 0x0001C3E3,
		0x033, 0x000183DD,
		0x033, 0x000143D7,
		0x033, 0x000103D1,
		0x033, 0x0000C3CB,
		0x033, 0x000083C5,
		0x033, 0x000040D7,
		0x033, 0x000000D1,
		0x0EE, 0x00000000,
	0xA0000000,	0x00000000,
		0x0EE, 0x00000004,
		0x033, 0x000287EF,
		0x033, 0x000243EF,
		0x033, 0x000203E9,
		0x033, 0x0001C3E3,
		0x033, 0x000183DD,
		0x033, 0x000143D7,
		0x033, 0x000103D1,
		0x033, 0x0000C1D7,
		0x033, 0x000081D1,
		0x033, 0x000040D7,
		0x033, 0x000000D1,
		0x0EE, 0x00000000,
	0xB0000000,	0x00000000,
		0x0EF, 0x00020000,
		0x033, 0x00000000,
		0x034, 0x00002000,
		0x033, 0x00004000,
		0x034, 0x00003924,
		0x033, 0x00008000,
		0x034, 0x00006208,
		0x033, 0x0000C000,
		0x034, 0x00007208,
		0x033, 0x00010000,
		0x034, 0x00002208,
		0x033, 0x00014000,
		0x034, 0x00002410,
		0x033, 0x00018000,
		0x034, 0x00003208,
		0x033, 0x0001C000,
		0x034, 0x000027DF,
		0x033, 0x00020000,
		0x034, 0x000037DF,
		0x033, 0x00024000,
		0x034, 0x00002FFF,
		0x033, 0x00028000,
		0x034, 0x00003FFF,
		0x0EF, 0x00000000,
		0x0CA, 0x00002000,
		0x0C9, 0x00000600,
		0x0B0, 0x000FFBC0,
		0x0B1, 0x00033F8F,
		0x0B2, 0x00033762,
		0x0B3, 0x00030000,
		0x0B4, 0x000140F0,
		0x0B5, 0x00014000,
		0x0B6, 0x00012427,
		0x0B7, 0x00010DF0,
		0x0B8, 0x00070FF0,
		0x0B9, 0x000C0008,
		0x0BA, 0x00040005,
		0x0C2, 0x00002C01,
		0x0C3, 0x0000000B,
		0x0C4, 0x00081E2F,
		0x0C5, 0x0005C28F,
		0x0C6, 0x000000A0,
	0x81000002,	0x00000000,	0x40000000,	0x00000000,
		0x0C9, 0x00001F00,
	0xA0000000,	0x00000000,
		0x0C9, 0x00001600,
	0xB0000000,	0x00000000,
		0x0CA, 0x00002000,
		0x0EF, 0x00080000,
		0x030, 0x00020000,
		0x031, 0x000000EF,
		0x032, 0x00001FF7,
		0x0EF, 0x00000000,
		0x0EF, 0x00080000,
		0x030, 0x00030000,
		0x031, 0x000000EF,
		0x032, 0x000F1FF3,
		0x0EF, 0x00000000,
		0x0EF, 0x00080000,
		0x030, 0x00010000,
		0x031, 0x000000EF,
		0x032, 0x00001EFE,
		0x0EF, 0x00000000,
		0x0EF, 0x00080000,
		0x030, 0x00028000,
		0x031, 0x000000EF,
		0x032, 0x00039FFE,
		0x0EF, 0x00000000,
		0x01B, 0x00033A06,
		0x01B, 0x00033A46,
		0x01A, 0x00040004,
		0xFFE, 0x00000000,
		0x018, 0x00008C07,
		0xFFE, 0x00000000,
		0xFFE, 0x00000000,
		0xFFE, 0x00000000,
		0x000, 0x00035DF5,
		0xFFFF, 0x0000FFFF,

};

void
odm_read_and_config_mp_8197g_radioa(struct dm_struct *dm)
{
	u32	i = 0;
	u8	c_cond;
	boolean	is_matched = true, is_skipped = false;
	u32	array_len =
			sizeof(array_mp_8197g_radioa) / sizeof(u32);
	u32	*array = (u32 *)array_mp_8197g_radioa;

	u32	v1 = 0, v2 = 0, pre_v1 = 0, pre_v2 = 0;
	u32	a1 = 0, a2 = 0, a3 = 0, a4 = 0;

	PHYDM_DBG(dm, ODM_COMP_INIT, "===> %s\n", __func__);

	while ((i + 1) < array_len) {
		v1 = array[i];
		v2 = array[i + 1];

		if (v1 & (BIT(31) | BIT(30))) {/*positive & negative condition*/
			if (v1 & BIT(31)) {/* positive condition*/
				c_cond  =
					(u8)((v1 & (BIT(29) | BIT(28))) >> 28);
				if (c_cond == COND_ENDIF) {/*end*/
					is_matched = true;
					is_skipped = false;
					PHYDM_DBG(dm, ODM_COMP_INIT, "ENDIF\n");
				} else if (c_cond == COND_ELSE) { /*else*/
					is_matched = is_skipped ? false : true;
					PHYDM_DBG(dm, ODM_COMP_INIT, "ELSE\n");
				} else {/*if , else if*/
					pre_v1 = v1;
					pre_v2 = v2;
					PHYDM_DBG(dm, ODM_COMP_INIT,
						  "IF or ELSE IF\n");
				}
			} else if (v1 & BIT(30)) { /*negative condition*/
				if (!is_skipped) {
					a1 = pre_v1; a2 = pre_v2;
					a3 = v1; a4 = v2;
					if (check_positive(dm,
							   a1, a2, a3, a4)) {
						is_matched = true;
						is_skipped = true;
					} else {
						is_matched = false;
						is_skipped = false;
					}
				} else {
					is_matched = false;
				}
			}
		} else {
			if (is_matched)
				odm_config_rf_radio_a_8197g(dm, v1, v2);
		}
		i = i + 2;
	}
}

u32
odm_get_version_mp_8197g_radioa(void)
{
		return 19;
}

/******************************************************************************
 *                           radiob.TXT
 ******************************************************************************/

const u32 array_mp_8197g_radiob[] = {
		0x000, 0x00030000,
		0x088, 0x0004B0CC,
		0x085, 0x000910D5,
		0x086, 0x00033B1F,
		0x0EF, 0x00000002,
		0x033, 0x00000300,
		0x033, 0x00008393,
		0x033, 0x00010393,
		0x033, 0x00018300,
		0x033, 0x00020393,
		0x033, 0x00028393,
		0x033, 0x00030393,
		0x033, 0x00038393,
		0x033, 0x00040393,
		0x033, 0x00048393,
		0x033, 0x00050393,
		0x033, 0x00058393,
		0x033, 0x00060393,
		0x033, 0x000383D5,
		0x033, 0x000403D5,
		0x0EF, 0x00000000,
		0x0EF, 0x00002000,
		0x033, 0x00000003,
		0x034, 0x00000003,
		0x033, 0x00004403,
		0x034, 0x00000003,
		0x033, 0x00008803,
		0x034, 0x00000003,
		0x033, 0x0000CC03,
		0x034, 0x00000003,
		0x033, 0x00010103,
		0x034, 0x00000003,
		0x033, 0x00015103,
		0x034, 0x00000003,
		0x033, 0x0001A103,
		0x034, 0x00000003,
		0x033, 0x0001F10F,
		0x034, 0x00000003,
		0x033, 0x00020003,
		0x034, 0x00000003,
		0x033, 0x00024403,
		0x034, 0x00000003,
		0x033, 0x00028803,
		0x034, 0x00000003,
		0x033, 0x0002CC03,
		0x034, 0x00000003,
		0x033, 0x00030103,
		0x034, 0x00000003,
		0x033, 0x00035103,
		0x034, 0x00000003,
		0x033, 0x0003A103,
		0x034, 0x00000003,
		0x033, 0x0003F10F,
		0x034, 0x00000003,
		0x033, 0x00040003,
		0x034, 0x00000003,
		0x033, 0x00044403,
		0x034, 0x00000003,
		0x033, 0x00048803,
		0x034, 0x00000003,
		0x033, 0x0004CC03,
		0x034, 0x00000003,
		0x033, 0x00050103,
		0x034, 0x00000003,
		0x033, 0x00055103,
		0x034, 0x00000003,
		0x033, 0x0005A103,
		0x034, 0x00000003,
		0x033, 0x0005F10F,
		0x034, 0x00000003,
		0x0EF, 0x00000000,
		0x08E, 0x000645E0,
		0x08F, 0x000A0B3A,
		0x051, 0x0002A2A5,
		0x052, 0x00001347,
		0x053, 0x00000CC1,
		0x054, 0x00003437,
		0x056, 0x0009DDEB,
		0x057, 0x0004CE26,
		0x058, 0x0000EF80,
		0x05A, 0x00050000,
		0x05B, 0x0002A000,
		0x0EF, 0x00000400,
		0x033, 0x0000057C,
		0x034, 0x0000012A,
		0x033, 0x0000457C,
		0x034, 0x0000012A,
		0x033, 0x0000857C,
		0x034, 0x0000012A,
		0x033, 0x0000C57C,
		0x034, 0x0000012A,
		0x033, 0x0001057C,
		0x034, 0x0000012A,
		0x033, 0x0001457C,
		0x034, 0x0000012A,
		0x033, 0x0001857C,
		0x034, 0x0000012A,
		0x033, 0x0001C57C,
		0x034, 0x0000012A,
		0x033, 0x0002057C,
		0x034, 0x0000012F,
		0x033, 0x0002457C,
		0x034, 0x0000012F,
		0x033, 0x0002857C,
		0x034, 0x0000012F,
		0x033, 0x0002C57C,
		0x034, 0x0000012F,
		0x033, 0x0003057C,
		0x034, 0x0000012F,
		0x033, 0x0003457C,
		0x034, 0x0000012F,
		0x033, 0x0003857C,
		0x034, 0x0000012F,
		0x033, 0x0003C57C,
		0x034, 0x0000012F,
		0x033, 0x0004057C,
		0x034, 0x0000012F,
		0x033, 0x0004457C,
		0x034, 0x0000012F,
		0x033, 0x0004857C,
		0x034, 0x0000012F,
		0x033, 0x0004C57C,
		0x034, 0x0000012F,
		0x033, 0x0005057C,
		0x034, 0x0000012A,
		0x033, 0x0005457C,
		0x034, 0x0000012A,
		0x033, 0x0005857C,
		0x034, 0x0000012A,
		0x033, 0x0005C57C,
		0x034, 0x0000012A,
		0x033, 0x0006057C,
		0x034, 0x0000012F,
		0x033, 0x0006457C,
		0x034, 0x0000012F,
		0x033, 0x0006857C,
		0x034, 0x0000012F,
		0x033, 0x0006C57C,
		0x034, 0x0000012F,
		0x033, 0x0007057C,
		0x034, 0x0000012F,
		0x033, 0x0007457C,
		0x034, 0x0000012F,
		0x033, 0x0007857C,
		0x034, 0x0000012F,
		0x033, 0x0007C57C,
		0x034, 0x0000012F,
		0x033, 0x0008057C,
		0x034, 0x0000012A,
		0x033, 0x0008457C,
		0x034, 0x0000012A,
		0x033, 0x0008857C,
		0x034, 0x0000012A,
		0x033, 0x0008C57C,
		0x034, 0x0000012A,
		0x033, 0x0009057C,
		0x034, 0x0000012A,
		0x033, 0x0009457C,
		0x034, 0x0000012A,
		0x033, 0x0009857C,
		0x034, 0x0000012A,
		0x033, 0x0009C57C,
		0x034, 0x0000012A,
		0x033, 0x000A057C,
		0x034, 0x0000012A,
		0x033, 0x000A457C,
		0x034, 0x0000012A,
		0x033, 0x000A857C,
		0x034, 0x0000012A,
		0x033, 0x000AC57C,
		0x034, 0x0000012A,
		0x033, 0x000B057C,
		0x034, 0x0000012A,
		0x0EF, 0x00000000,
	0x81000001,	0x00000000,	0x40000000,	0x00000000,
		0x0EE, 0x00000008,
		0x033, 0x00028039,
		0x033, 0x00024036,
		0x033, 0x00020033,
		0x033, 0x0001C030,
		0x033, 0x0001802D,
		0x033, 0x0001402A,
		0x033, 0x00010027,
		0x033, 0x0000C024,
		0x033, 0x00008007,
		0x033, 0x00004004,
		0x033, 0x00000001,
		0x0EE, 0x00000000,
	0x91000005,	0x00000000,	0x40000000,	0x00000000,
		0x0EE, 0x00000008,
		0x033, 0x00028039,
		0x033, 0x00024036,
		0x033, 0x00020033,
		0x033, 0x0001C030,
		0x033, 0x0001802D,
		0x033, 0x0001402A,
		0x033, 0x00010027,
		0x033, 0x0000C024,
		0x033, 0x00008007,
		0x033, 0x00004004,
		0x033, 0x00000001,
		0x0EE, 0x00000000,
	0xA0000000,	0x00000000,
		0x0EE, 0x00000008,
		0x033, 0x000280F8,
		0x033, 0x000240F5,
		0x033, 0x000200F2,
		0x033, 0x0001C0EF,
		0x033, 0x000180EC,
		0x033, 0x000140E9,
		0x033, 0x000100E6,
		0x033, 0x0000C0E3,
		0x033, 0x00008066,
		0x033, 0x00004063,
		0x033, 0x00000043,
		0x0EE, 0x00000000,
	0xB0000000,	0x00000000,
	0x81000001,	0x00000000,	0x40000000,	0x00000000,
		0x0EE, 0x00000004,
		0x033, 0x000283E1,
		0x033, 0x000241E7,
		0x033, 0x000201E1,
		0x033, 0x0001C1DB,
		0x033, 0x000181D5,
		0x033, 0x000141CF,
		0x033, 0x000101C9,
		0x033, 0x0000C0CF,
		0x033, 0x000080C9,
		0x033, 0x0000404F,
		0x033, 0x00000049,
		0x0EE, 0x00000000,
	0x91000005,	0x00000000,	0x40000000,	0x00000000,
		0x0EE, 0x00000004,
		0x033, 0x000283E1,
		0x033, 0x000241E7,
		0x033, 0x000201E1,
		0x033, 0x0001C1DB,
		0x033, 0x000181D5,
		0x033, 0x000141CF,
		0x033, 0x000101C9,
		0x033, 0x0000C0CF,
		0x033, 0x000080C9,
		0x033, 0x0000404F,
		0x033, 0x00000049,
		0x0EE, 0x00000000,
	0x91000006,	0x00000000,	0x40000000,	0x00000000,
		0x0EE, 0x00000004,
		0x033, 0x000287EF,
		0x033, 0x000243EF,
		0x033, 0x000203E9,
		0x033, 0x0001C3E3,
		0x033, 0x000183DD,
		0x033, 0x000143D7,
		0x033, 0x000103D1,
		0x033, 0x0000C3CB,
		0x033, 0x000083C5,
		0x033, 0x000040D7,
		0x033, 0x000000D1,
		0x0EE, 0x00000000,
	0xA0000000,	0x00000000,
		0x0EE, 0x00000004,
		0x033, 0x000287EF,
		0x033, 0x000243EF,
		0x033, 0x000203E9,
		0x033, 0x0001C3E3,
		0x033, 0x000183DD,
		0x033, 0x000143D7,
		0x033, 0x000103D1,
		0x033, 0x0000C1D7,
		0x033, 0x000081D1,
		0x033, 0x000040D7,
		0x033, 0x000000D1,
		0x0EE, 0x00000000,
	0xB0000000,	0x00000000,
		0x0EF, 0x00020000,
		0x033, 0x00000000,
		0x034, 0x00002000,
		0x033, 0x00004000,
		0x034, 0x00003924,
		0x033, 0x00008000,
		0x034, 0x00006208,
		0x033, 0x0000C000,
		0x034, 0x00007208,
		0x033, 0x00010000,
		0x034, 0x00002208,
		0x033, 0x00014000,
		0x034, 0x00002410,
		0x033, 0x00018000,
		0x034, 0x00003208,
		0x033, 0x0001C000,
		0x034, 0x000027DF,
		0x033, 0x00020000,
		0x034, 0x000037DF,
		0x033, 0x00024000,
		0x034, 0x00002FFF,
		0x033, 0x00028000,
		0x034, 0x00003FFF,
		0x0EF, 0x00000000,
		0x0EF, 0x00080000,
		0x030, 0x00020000,
		0x031, 0x000000EF,
		0x032, 0x00001FF7,
		0x0EF, 0x00000000,
		0x0EF, 0x00080000,
		0x030, 0x00030000,
		0x031, 0x000000EF,
		0x032, 0x000F1FF3,
		0x0EF, 0x00000000,
		0x0EF, 0x00080000,
		0x030, 0x00010000,
		0x031, 0x000000EF,
		0x032, 0x00001EFE,
		0x0EF, 0x00000000,
		0x0EF, 0x00080000,
		0x030, 0x00028000,
		0x031, 0x000000EF,
		0x032, 0x00039FFE,
		0x0EF, 0x00000000,
		0x01B, 0x00033A06,
		0x01B, 0x00033A46,
		0x01A, 0x00040004,
		0x000, 0x00035DF5,
		0xFFFF, 0x0000FFFF,

};

void
odm_read_and_config_mp_8197g_radiob(struct dm_struct *dm)
{
	u32	i = 0;
	u8	c_cond;
	boolean	is_matched = true, is_skipped = false;
	u32	array_len =
			sizeof(array_mp_8197g_radiob) / sizeof(u32);
	u32	*array = (u32 *)array_mp_8197g_radiob;

	u32	v1 = 0, v2 = 0, pre_v1 = 0, pre_v2 = 0;
	u32	a1 = 0, a2 = 0, a3 = 0, a4 = 0;

	PHYDM_DBG(dm, ODM_COMP_INIT, "===> %s\n", __func__);

	while ((i + 1) < array_len) {
		v1 = array[i];
		v2 = array[i + 1];

		if (v1 & (BIT(31) | BIT(30))) {/*positive & negative condition*/
			if (v1 & BIT(31)) {/* positive condition*/
				c_cond  =
					(u8)((v1 & (BIT(29) | BIT(28))) >> 28);
				if (c_cond == COND_ENDIF) {/*end*/
					is_matched = true;
					is_skipped = false;
					PHYDM_DBG(dm, ODM_COMP_INIT, "ENDIF\n");
				} else if (c_cond == COND_ELSE) { /*else*/
					is_matched = is_skipped ? false : true;
					PHYDM_DBG(dm, ODM_COMP_INIT, "ELSE\n");
				} else {/*if , else if*/
					pre_v1 = v1;
					pre_v2 = v2;
					PHYDM_DBG(dm, ODM_COMP_INIT,
						  "IF or ELSE IF\n");
				}
			} else if (v1 & BIT(30)) { /*negative condition*/
				if (!is_skipped) {
					a1 = pre_v1; a2 = pre_v2;
					a3 = v1; a4 = v2;
					if (check_positive(dm,
							   a1, a2, a3, a4)) {
						is_matched = true;
						is_skipped = true;
					} else {
						is_matched = false;
						is_skipped = false;
					}
				} else {
					is_matched = false;
				}
			}
		} else {
			if (is_matched)
				odm_config_rf_radio_b_8197g(dm, v1, v2);
		}
		i = i + 2;
	}
}

u32
odm_get_version_mp_8197g_radiob(void)
{
		return 19;
}

/******************************************************************************
 *                           txpowertrack.TXT
 ******************************************************************************/

#ifdef CONFIG_8197G
const u8 delta_swingidx_mp_5gb_n_txpwrtrk_8197g[][D_S_SIZE] = {
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
};

const u8 delta_swingidx_mp_5gb_p_txpwrtrk_8197g[][D_S_SIZE] = {
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
};

const u8 delta_swingidx_mp_5ga_n_txpwrtrk_8197g[][D_S_SIZE] = {
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
};

const u8 delta_swingidx_mp_5ga_p_txpwrtrk_8197g[][D_S_SIZE] = {
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
};

const u8 delta_swingidx_mp_2gb_n_txpwrtrk_8197g[]    = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2gb_p_txpwrtrk_8197g[]    = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2ga_n_txpwrtrk_8197g[]    = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2ga_p_txpwrtrk_8197g[]    = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2g_cck_b_n_txpwrtrk_8197g[] = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2g_cck_b_p_txpwrtrk_8197g[] = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2g_cck_a_n_txpwrtrk_8197g[] = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2g_cck_a_p_txpwrtrk_8197g[] = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
#endif

void
odm_read_and_config_mp_8197g_txpowertrack(struct dm_struct *dm)
{
#ifdef CONFIG_8197G

struct dm_rf_calibration_struct  *cali_info = &dm->rf_calibrate_info;

PHYDM_DBG(dm, ODM_COMP_INIT, "===> ODM_ReadAndConfig_MP_mp_8197g\n");

odm_move_memory(dm, cali_info->delta_swing_table_idx_2ga_p,
		(void *)delta_swingidx_mp_2ga_p_txpwrtrk_8197g,
		DELTA_SWINGIDX_SIZE);
odm_move_memory(dm, cali_info->delta_swing_table_idx_2ga_n,
		(void *)delta_swingidx_mp_2ga_n_txpwrtrk_8197g,
		DELTA_SWINGIDX_SIZE);
odm_move_memory(dm, cali_info->delta_swing_table_idx_2gb_p,
		(void *)delta_swingidx_mp_2gb_p_txpwrtrk_8197g,
		DELTA_SWINGIDX_SIZE);
odm_move_memory(dm, cali_info->delta_swing_table_idx_2gb_n,
		(void *)delta_swingidx_mp_2gb_n_txpwrtrk_8197g,
		DELTA_SWINGIDX_SIZE);

odm_move_memory(dm, cali_info->delta_swing_table_idx_2g_cck_a_p,
		(void *)delta_swingidx_mp_2g_cck_a_p_txpwrtrk_8197g,
		DELTA_SWINGIDX_SIZE);
odm_move_memory(dm, cali_info->delta_swing_table_idx_2g_cck_a_n,
		(void *)delta_swingidx_mp_2g_cck_a_n_txpwrtrk_8197g,
		DELTA_SWINGIDX_SIZE);
odm_move_memory(dm, cali_info->delta_swing_table_idx_2g_cck_b_p,
		(void *)delta_swingidx_mp_2g_cck_b_p_txpwrtrk_8197g,
		DELTA_SWINGIDX_SIZE);
odm_move_memory(dm, cali_info->delta_swing_table_idx_2g_cck_b_n,
		(void *)delta_swingidx_mp_2g_cck_b_n_txpwrtrk_8197g,
		DELTA_SWINGIDX_SIZE);

odm_move_memory(dm, cali_info->delta_swing_table_idx_5ga_p,
		(void *)delta_swingidx_mp_5ga_p_txpwrtrk_8197g,
		DELTA_SWINGIDX_SIZE * 3);
odm_move_memory(dm, cali_info->delta_swing_table_idx_5ga_n,
		(void *)delta_swingidx_mp_5ga_n_txpwrtrk_8197g,
		DELTA_SWINGIDX_SIZE * 3);
odm_move_memory(dm, cali_info->delta_swing_table_idx_5gb_p,
		(void *)delta_swingidx_mp_5gb_p_txpwrtrk_8197g,
		DELTA_SWINGIDX_SIZE * 3);
odm_move_memory(dm, cali_info->delta_swing_table_idx_5gb_n,
		(void *)delta_swingidx_mp_5gb_n_txpwrtrk_8197g,
		DELTA_SWINGIDX_SIZE * 3);
#endif
}

/******************************************************************************
 *                           txpowertrack_type0.TXT
 ******************************************************************************/

#ifdef CONFIG_8197G_TYPE0
const u8 delta_swingidx_mp_5gb_n_txpwrtrk_type0_8197g[][D_S_SIZE] = {
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
};

const u8 delta_swingidx_mp_5gb_p_txpwrtrk_type0_8197g[][D_S_SIZE] = {
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
};

const u8 delta_swingidx_mp_5ga_n_txpwrtrk_type0_8197g[][D_S_SIZE] = {
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
};

const u8 delta_swingidx_mp_5ga_p_txpwrtrk_type0_8197g[][D_S_SIZE] = {
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
};

const u8 delta_swingidx_mp_2gb_n_txpwrtrk_type0_8197g[]    = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2gb_p_txpwrtrk_type0_8197g[]    = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2ga_n_txpwrtrk_type0_8197g[]    = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2ga_p_txpwrtrk_type0_8197g[]    = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2g_cck_b_n_txpwrtrk_type0_8197g[] = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2g_cck_b_p_txpwrtrk_type0_8197g[] = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2g_cck_a_n_txpwrtrk_type0_8197g[] = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2g_cck_a_p_txpwrtrk_type0_8197g[] = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
#endif

void
odm_read_and_config_mp_8197g_txpowertrack_type0(struct dm_struct *dm)
{
#ifdef CONFIG_8197G_TYPE0

struct dm_rf_calibration_struct  *cali_info = &dm->rf_calibrate_info;

PHYDM_DBG(dm, ODM_COMP_INIT, "===> ODM_ReadAndConfig_MP_mp_8197g\n");

odm_move_memory(dm, cali_info->delta_swing_table_idx_2ga_p,
		(void *)delta_swingidx_mp_2ga_p_txpwrtrk_type0_8197g,
		DELTA_SWINGIDX_SIZE);
odm_move_memory(dm, cali_info->delta_swing_table_idx_2ga_n,
		(void *)delta_swingidx_mp_2ga_n_txpwrtrk_type0_8197g,
		DELTA_SWINGIDX_SIZE);
odm_move_memory(dm, cali_info->delta_swing_table_idx_2gb_p,
		(void *)delta_swingidx_mp_2gb_p_txpwrtrk_type0_8197g,
		DELTA_SWINGIDX_SIZE);
odm_move_memory(dm, cali_info->delta_swing_table_idx_2gb_n,
		(void *)delta_swingidx_mp_2gb_n_txpwrtrk_type0_8197g,
		DELTA_SWINGIDX_SIZE);

odm_move_memory(dm, cali_info->delta_swing_table_idx_2g_cck_a_p,
		(void *)delta_swingidx_mp_2g_cck_a_p_txpwrtrk_type0_8197g,
		DELTA_SWINGIDX_SIZE);
odm_move_memory(dm, cali_info->delta_swing_table_idx_2g_cck_a_n,
		(void *)delta_swingidx_mp_2g_cck_a_n_txpwrtrk_type0_8197g,
		DELTA_SWINGIDX_SIZE);
odm_move_memory(dm, cali_info->delta_swing_table_idx_2g_cck_b_p,
		(void *)delta_swingidx_mp_2g_cck_b_p_txpwrtrk_type0_8197g,
		DELTA_SWINGIDX_SIZE);
odm_move_memory(dm, cali_info->delta_swing_table_idx_2g_cck_b_n,
		(void *)delta_swingidx_mp_2g_cck_b_n_txpwrtrk_type0_8197g,
		DELTA_SWINGIDX_SIZE);

odm_move_memory(dm, cali_info->delta_swing_table_idx_5ga_p,
		(void *)delta_swingidx_mp_5ga_p_txpwrtrk_type0_8197g,
		DELTA_SWINGIDX_SIZE * 3);
odm_move_memory(dm, cali_info->delta_swing_table_idx_5ga_n,
		(void *)delta_swingidx_mp_5ga_n_txpwrtrk_type0_8197g,
		DELTA_SWINGIDX_SIZE * 3);
odm_move_memory(dm, cali_info->delta_swing_table_idx_5gb_p,
		(void *)delta_swingidx_mp_5gb_p_txpwrtrk_type0_8197g,
		DELTA_SWINGIDX_SIZE * 3);
odm_move_memory(dm, cali_info->delta_swing_table_idx_5gb_n,
		(void *)delta_swingidx_mp_5gb_n_txpwrtrk_type0_8197g,
		DELTA_SWINGIDX_SIZE * 3);
#endif
}

/******************************************************************************
 *                           txpowertrack_type1.TXT
 ******************************************************************************/

#ifdef CONFIG_8197G_TYPE1
const u8 delta_swingidx_mp_5gb_n_txpwrtrk_type1_8197g[][D_S_SIZE] = {
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
};

const u8 delta_swingidx_mp_5gb_p_txpwrtrk_type1_8197g[][D_S_SIZE] = {
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
};

const u8 delta_swingidx_mp_5ga_n_txpwrtrk_type1_8197g[][D_S_SIZE] = {
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
};

const u8 delta_swingidx_mp_5ga_p_txpwrtrk_type1_8197g[][D_S_SIZE] = {
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
};

const u8 delta_swingidx_mp_2gb_n_txpwrtrk_type1_8197g[]    = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2gb_p_txpwrtrk_type1_8197g[]    = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2ga_n_txpwrtrk_type1_8197g[]    = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2ga_p_txpwrtrk_type1_8197g[]    = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2g_cck_b_n_txpwrtrk_type1_8197g[] = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2g_cck_b_p_txpwrtrk_type1_8197g[] = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2g_cck_a_n_txpwrtrk_type1_8197g[] = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2g_cck_a_p_txpwrtrk_type1_8197g[] = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
#endif

void
odm_read_and_config_mp_8197g_txpowertrack_type1(struct dm_struct *dm)
{
#ifdef CONFIG_8197G_TYPE1

struct dm_rf_calibration_struct  *cali_info = &dm->rf_calibrate_info;

PHYDM_DBG(dm, ODM_COMP_INIT, "===> ODM_ReadAndConfig_MP_mp_8197g\n");

odm_move_memory(dm, cali_info->delta_swing_table_idx_2ga_p,
		(void *)delta_swingidx_mp_2ga_p_txpwrtrk_type1_8197g,
		DELTA_SWINGIDX_SIZE);
odm_move_memory(dm, cali_info->delta_swing_table_idx_2ga_n,
		(void *)delta_swingidx_mp_2ga_n_txpwrtrk_type1_8197g,
		DELTA_SWINGIDX_SIZE);
odm_move_memory(dm, cali_info->delta_swing_table_idx_2gb_p,
		(void *)delta_swingidx_mp_2gb_p_txpwrtrk_type1_8197g,
		DELTA_SWINGIDX_SIZE);
odm_move_memory(dm, cali_info->delta_swing_table_idx_2gb_n,
		(void *)delta_swingidx_mp_2gb_n_txpwrtrk_type1_8197g,
		DELTA_SWINGIDX_SIZE);

odm_move_memory(dm, cali_info->delta_swing_table_idx_2g_cck_a_p,
		(void *)delta_swingidx_mp_2g_cck_a_p_txpwrtrk_type1_8197g,
		DELTA_SWINGIDX_SIZE);
odm_move_memory(dm, cali_info->delta_swing_table_idx_2g_cck_a_n,
		(void *)delta_swingidx_mp_2g_cck_a_n_txpwrtrk_type1_8197g,
		DELTA_SWINGIDX_SIZE);
odm_move_memory(dm, cali_info->delta_swing_table_idx_2g_cck_b_p,
		(void *)delta_swingidx_mp_2g_cck_b_p_txpwrtrk_type1_8197g,
		DELTA_SWINGIDX_SIZE);
odm_move_memory(dm, cali_info->delta_swing_table_idx_2g_cck_b_n,
		(void *)delta_swingidx_mp_2g_cck_b_n_txpwrtrk_type1_8197g,
		DELTA_SWINGIDX_SIZE);

odm_move_memory(dm, cali_info->delta_swing_table_idx_5ga_p,
		(void *)delta_swingidx_mp_5ga_p_txpwrtrk_type1_8197g,
		DELTA_SWINGIDX_SIZE * 3);
odm_move_memory(dm, cali_info->delta_swing_table_idx_5ga_n,
		(void *)delta_swingidx_mp_5ga_n_txpwrtrk_type1_8197g,
		DELTA_SWINGIDX_SIZE * 3);
odm_move_memory(dm, cali_info->delta_swing_table_idx_5gb_p,
		(void *)delta_swingidx_mp_5gb_p_txpwrtrk_type1_8197g,
		DELTA_SWINGIDX_SIZE * 3);
odm_move_memory(dm, cali_info->delta_swing_table_idx_5gb_n,
		(void *)delta_swingidx_mp_5gb_n_txpwrtrk_type1_8197g,
		DELTA_SWINGIDX_SIZE * 3);
#endif
}

/******************************************************************************
 *                           txpowertrack_type2.TXT
 ******************************************************************************/

#ifdef CONFIG_8197G_TYPE2
const u8 delta_swingidx_mp_5gb_n_txpwrtrk_type2_8197g[][D_S_SIZE] = {
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
};

const u8 delta_swingidx_mp_5gb_p_txpwrtrk_type2_8197g[][D_S_SIZE] = {
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
};

const u8 delta_swingidx_mp_5ga_n_txpwrtrk_type2_8197g[][D_S_SIZE] = {
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
};

const u8 delta_swingidx_mp_5ga_p_txpwrtrk_type2_8197g[][D_S_SIZE] = {
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
};

const u8 delta_swingidx_mp_2gb_n_txpwrtrk_type2_8197g[]    = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2gb_p_txpwrtrk_type2_8197g[]    = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2ga_n_txpwrtrk_type2_8197g[]    = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2ga_p_txpwrtrk_type2_8197g[]    = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2g_cck_b_n_txpwrtrk_type2_8197g[] = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2g_cck_b_p_txpwrtrk_type2_8197g[] = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2g_cck_a_n_txpwrtrk_type2_8197g[] = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2g_cck_a_p_txpwrtrk_type2_8197g[] = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
#endif

void
odm_read_and_config_mp_8197g_txpowertrack_type2(struct dm_struct *dm)
{
#ifdef CONFIG_8197G_TYPE2

struct dm_rf_calibration_struct  *cali_info = &dm->rf_calibrate_info;

PHYDM_DBG(dm, ODM_COMP_INIT, "===> ODM_ReadAndConfig_MP_mp_8197g\n");

odm_move_memory(dm, cali_info->delta_swing_table_idx_2ga_p,
		(void *)delta_swingidx_mp_2ga_p_txpwrtrk_type2_8197g,
		DELTA_SWINGIDX_SIZE);
odm_move_memory(dm, cali_info->delta_swing_table_idx_2ga_n,
		(void *)delta_swingidx_mp_2ga_n_txpwrtrk_type2_8197g,
		DELTA_SWINGIDX_SIZE);
odm_move_memory(dm, cali_info->delta_swing_table_idx_2gb_p,
		(void *)delta_swingidx_mp_2gb_p_txpwrtrk_type2_8197g,
		DELTA_SWINGIDX_SIZE);
odm_move_memory(dm, cali_info->delta_swing_table_idx_2gb_n,
		(void *)delta_swingidx_mp_2gb_n_txpwrtrk_type2_8197g,
		DELTA_SWINGIDX_SIZE);

odm_move_memory(dm, cali_info->delta_swing_table_idx_2g_cck_a_p,
		(void *)delta_swingidx_mp_2g_cck_a_p_txpwrtrk_type2_8197g,
		DELTA_SWINGIDX_SIZE);
odm_move_memory(dm, cali_info->delta_swing_table_idx_2g_cck_a_n,
		(void *)delta_swingidx_mp_2g_cck_a_n_txpwrtrk_type2_8197g,
		DELTA_SWINGIDX_SIZE);
odm_move_memory(dm, cali_info->delta_swing_table_idx_2g_cck_b_p,
		(void *)delta_swingidx_mp_2g_cck_b_p_txpwrtrk_type2_8197g,
		DELTA_SWINGIDX_SIZE);
odm_move_memory(dm, cali_info->delta_swing_table_idx_2g_cck_b_n,
		(void *)delta_swingidx_mp_2g_cck_b_n_txpwrtrk_type2_8197g,
		DELTA_SWINGIDX_SIZE);

odm_move_memory(dm, cali_info->delta_swing_table_idx_5ga_p,
		(void *)delta_swingidx_mp_5ga_p_txpwrtrk_type2_8197g,
		DELTA_SWINGIDX_SIZE * 3);
odm_move_memory(dm, cali_info->delta_swing_table_idx_5ga_n,
		(void *)delta_swingidx_mp_5ga_n_txpwrtrk_type2_8197g,
		DELTA_SWINGIDX_SIZE * 3);
odm_move_memory(dm, cali_info->delta_swing_table_idx_5gb_p,
		(void *)delta_swingidx_mp_5gb_p_txpwrtrk_type2_8197g,
		DELTA_SWINGIDX_SIZE * 3);
odm_move_memory(dm, cali_info->delta_swing_table_idx_5gb_n,
		(void *)delta_swingidx_mp_5gb_n_txpwrtrk_type2_8197g,
		DELTA_SWINGIDX_SIZE * 3);
#endif
}

/******************************************************************************
 *                           txpowertrack_type3.TXT
 ******************************************************************************/

#ifdef CONFIG_8197G_TYPE3
const u8 delta_swingidx_mp_5gb_n_txpwrtrk_type3_8197g[][D_S_SIZE] = {
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
};

const u8 delta_swingidx_mp_5gb_p_txpwrtrk_type3_8197g[][D_S_SIZE] = {
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
};

const u8 delta_swingidx_mp_5ga_n_txpwrtrk_type3_8197g[][D_S_SIZE] = {
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
};

const u8 delta_swingidx_mp_5ga_p_txpwrtrk_type3_8197g[][D_S_SIZE] = {
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
};

const u8 delta_swingidx_mp_2gb_n_txpwrtrk_type3_8197g[]    = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2gb_p_txpwrtrk_type3_8197g[]    = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2ga_n_txpwrtrk_type3_8197g[]    = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2ga_p_txpwrtrk_type3_8197g[]    = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2g_cck_b_n_txpwrtrk_type3_8197g[] = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2g_cck_b_p_txpwrtrk_type3_8197g[] = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2g_cck_a_n_txpwrtrk_type3_8197g[] = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2g_cck_a_p_txpwrtrk_type3_8197g[] = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
#endif

void
odm_read_and_config_mp_8197g_txpowertrack_type3(struct dm_struct *dm)
{
#ifdef CONFIG_8197G_TYPE3

struct dm_rf_calibration_struct  *cali_info = &dm->rf_calibrate_info;

PHYDM_DBG(dm, ODM_COMP_INIT, "===> ODM_ReadAndConfig_MP_mp_8197g\n");

odm_move_memory(dm, cali_info->delta_swing_table_idx_2ga_p,
		(void *)delta_swingidx_mp_2ga_p_txpwrtrk_type3_8197g,
		DELTA_SWINGIDX_SIZE);
odm_move_memory(dm, cali_info->delta_swing_table_idx_2ga_n,
		(void *)delta_swingidx_mp_2ga_n_txpwrtrk_type3_8197g,
		DELTA_SWINGIDX_SIZE);
odm_move_memory(dm, cali_info->delta_swing_table_idx_2gb_p,
		(void *)delta_swingidx_mp_2gb_p_txpwrtrk_type3_8197g,
		DELTA_SWINGIDX_SIZE);
odm_move_memory(dm, cali_info->delta_swing_table_idx_2gb_n,
		(void *)delta_swingidx_mp_2gb_n_txpwrtrk_type3_8197g,
		DELTA_SWINGIDX_SIZE);

odm_move_memory(dm, cali_info->delta_swing_table_idx_2g_cck_a_p,
		(void *)delta_swingidx_mp_2g_cck_a_p_txpwrtrk_type3_8197g,
		DELTA_SWINGIDX_SIZE);
odm_move_memory(dm, cali_info->delta_swing_table_idx_2g_cck_a_n,
		(void *)delta_swingidx_mp_2g_cck_a_n_txpwrtrk_type3_8197g,
		DELTA_SWINGIDX_SIZE);
odm_move_memory(dm, cali_info->delta_swing_table_idx_2g_cck_b_p,
		(void *)delta_swingidx_mp_2g_cck_b_p_txpwrtrk_type3_8197g,
		DELTA_SWINGIDX_SIZE);
odm_move_memory(dm, cali_info->delta_swing_table_idx_2g_cck_b_n,
		(void *)delta_swingidx_mp_2g_cck_b_n_txpwrtrk_type3_8197g,
		DELTA_SWINGIDX_SIZE);

odm_move_memory(dm, cali_info->delta_swing_table_idx_5ga_p,
		(void *)delta_swingidx_mp_5ga_p_txpwrtrk_type3_8197g,
		DELTA_SWINGIDX_SIZE * 3);
odm_move_memory(dm, cali_info->delta_swing_table_idx_5ga_n,
		(void *)delta_swingidx_mp_5ga_n_txpwrtrk_type3_8197g,
		DELTA_SWINGIDX_SIZE * 3);
odm_move_memory(dm, cali_info->delta_swing_table_idx_5gb_p,
		(void *)delta_swingidx_mp_5gb_p_txpwrtrk_type3_8197g,
		DELTA_SWINGIDX_SIZE * 3);
odm_move_memory(dm, cali_info->delta_swing_table_idx_5gb_n,
		(void *)delta_swingidx_mp_5gb_n_txpwrtrk_type3_8197g,
		DELTA_SWINGIDX_SIZE * 3);
#endif
}

/******************************************************************************
 *                           txpowertrack_type4.TXT
 ******************************************************************************/

#ifdef CONFIG_8197G_TYPE4
const u8 delta_swingidx_mp_5gb_n_txpwrtrk_type4_8197g[][D_S_SIZE] = {
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
};

const u8 delta_swingidx_mp_5gb_p_txpwrtrk_type4_8197g[][D_S_SIZE] = {
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
};

const u8 delta_swingidx_mp_5ga_n_txpwrtrk_type4_8197g[][D_S_SIZE] = {
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
};

const u8 delta_swingidx_mp_5ga_p_txpwrtrk_type4_8197g[][D_S_SIZE] = {
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
};

const u8 delta_swingidx_mp_2gb_n_txpwrtrk_type4_8197g[]    = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2gb_p_txpwrtrk_type4_8197g[]    = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2ga_n_txpwrtrk_type4_8197g[]    = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2ga_p_txpwrtrk_type4_8197g[]    = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2g_cck_b_n_txpwrtrk_type4_8197g[] = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2g_cck_b_p_txpwrtrk_type4_8197g[] = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2g_cck_a_n_txpwrtrk_type4_8197g[] = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2g_cck_a_p_txpwrtrk_type4_8197g[] = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
#endif

void
odm_read_and_config_mp_8197g_txpowertrack_type4(struct dm_struct *dm)
{
#ifdef CONFIG_8197G_TYPE4

struct dm_rf_calibration_struct  *cali_info = &dm->rf_calibrate_info;

PHYDM_DBG(dm, ODM_COMP_INIT, "===> ODM_ReadAndConfig_MP_mp_8197g\n");

odm_move_memory(dm, cali_info->delta_swing_table_idx_2ga_p,
		(void *)delta_swingidx_mp_2ga_p_txpwrtrk_type4_8197g,
		DELTA_SWINGIDX_SIZE);
odm_move_memory(dm, cali_info->delta_swing_table_idx_2ga_n,
		(void *)delta_swingidx_mp_2ga_n_txpwrtrk_type4_8197g,
		DELTA_SWINGIDX_SIZE);
odm_move_memory(dm, cali_info->delta_swing_table_idx_2gb_p,
		(void *)delta_swingidx_mp_2gb_p_txpwrtrk_type4_8197g,
		DELTA_SWINGIDX_SIZE);
odm_move_memory(dm, cali_info->delta_swing_table_idx_2gb_n,
		(void *)delta_swingidx_mp_2gb_n_txpwrtrk_type4_8197g,
		DELTA_SWINGIDX_SIZE);

odm_move_memory(dm, cali_info->delta_swing_table_idx_2g_cck_a_p,
		(void *)delta_swingidx_mp_2g_cck_a_p_txpwrtrk_type4_8197g,
		DELTA_SWINGIDX_SIZE);
odm_move_memory(dm, cali_info->delta_swing_table_idx_2g_cck_a_n,
		(void *)delta_swingidx_mp_2g_cck_a_n_txpwrtrk_type4_8197g,
		DELTA_SWINGIDX_SIZE);
odm_move_memory(dm, cali_info->delta_swing_table_idx_2g_cck_b_p,
		(void *)delta_swingidx_mp_2g_cck_b_p_txpwrtrk_type4_8197g,
		DELTA_SWINGIDX_SIZE);
odm_move_memory(dm, cali_info->delta_swing_table_idx_2g_cck_b_n,
		(void *)delta_swingidx_mp_2g_cck_b_n_txpwrtrk_type4_8197g,
		DELTA_SWINGIDX_SIZE);

odm_move_memory(dm, cali_info->delta_swing_table_idx_5ga_p,
		(void *)delta_swingidx_mp_5ga_p_txpwrtrk_type4_8197g,
		DELTA_SWINGIDX_SIZE * 3);
odm_move_memory(dm, cali_info->delta_swing_table_idx_5ga_n,
		(void *)delta_swingidx_mp_5ga_n_txpwrtrk_type4_8197g,
		DELTA_SWINGIDX_SIZE * 3);
odm_move_memory(dm, cali_info->delta_swing_table_idx_5gb_p,
		(void *)delta_swingidx_mp_5gb_p_txpwrtrk_type4_8197g,
		DELTA_SWINGIDX_SIZE * 3);
odm_move_memory(dm, cali_info->delta_swing_table_idx_5gb_n,
		(void *)delta_swingidx_mp_5gb_n_txpwrtrk_type4_8197g,
		DELTA_SWINGIDX_SIZE * 3);
#endif
}

/******************************************************************************
 *                           txpowertrack_type5.TXT
 ******************************************************************************/

#ifdef CONFIG_8197G_TYPE5
const u8 delta_swingidx_mp_5gb_n_txpwrtrk_type5_8197g[][D_S_SIZE] = {
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
};

const u8 delta_swingidx_mp_5gb_p_txpwrtrk_type5_8197g[][D_S_SIZE] = {
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
};

const u8 delta_swingidx_mp_5ga_n_txpwrtrk_type5_8197g[][D_S_SIZE] = {
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
};

const u8 delta_swingidx_mp_5ga_p_txpwrtrk_type5_8197g[][D_S_SIZE] = {
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
};

const u8 delta_swingidx_mp_2gb_n_txpwrtrk_type5_8197g[]    = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2gb_p_txpwrtrk_type5_8197g[]    = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2ga_n_txpwrtrk_type5_8197g[]    = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2ga_p_txpwrtrk_type5_8197g[]    = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2g_cck_b_n_txpwrtrk_type5_8197g[] = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2g_cck_b_p_txpwrtrk_type5_8197g[] = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2g_cck_a_n_txpwrtrk_type5_8197g[] = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2g_cck_a_p_txpwrtrk_type5_8197g[] = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
#endif

void
odm_read_and_config_mp_8197g_txpowertrack_type5(struct dm_struct *dm)
{
#ifdef CONFIG_8197G_TYPE5

struct dm_rf_calibration_struct  *cali_info = &dm->rf_calibrate_info;

PHYDM_DBG(dm, ODM_COMP_INIT, "===> ODM_ReadAndConfig_MP_mp_8197g\n");

odm_move_memory(dm, cali_info->delta_swing_table_idx_2ga_p,
		(void *)delta_swingidx_mp_2ga_p_txpwrtrk_type5_8197g,
		DELTA_SWINGIDX_SIZE);
odm_move_memory(dm, cali_info->delta_swing_table_idx_2ga_n,
		(void *)delta_swingidx_mp_2ga_n_txpwrtrk_type5_8197g,
		DELTA_SWINGIDX_SIZE);
odm_move_memory(dm, cali_info->delta_swing_table_idx_2gb_p,
		(void *)delta_swingidx_mp_2gb_p_txpwrtrk_type5_8197g,
		DELTA_SWINGIDX_SIZE);
odm_move_memory(dm, cali_info->delta_swing_table_idx_2gb_n,
		(void *)delta_swingidx_mp_2gb_n_txpwrtrk_type5_8197g,
		DELTA_SWINGIDX_SIZE);

odm_move_memory(dm, cali_info->delta_swing_table_idx_2g_cck_a_p,
		(void *)delta_swingidx_mp_2g_cck_a_p_txpwrtrk_type5_8197g,
		DELTA_SWINGIDX_SIZE);
odm_move_memory(dm, cali_info->delta_swing_table_idx_2g_cck_a_n,
		(void *)delta_swingidx_mp_2g_cck_a_n_txpwrtrk_type5_8197g,
		DELTA_SWINGIDX_SIZE);
odm_move_memory(dm, cali_info->delta_swing_table_idx_2g_cck_b_p,
		(void *)delta_swingidx_mp_2g_cck_b_p_txpwrtrk_type5_8197g,
		DELTA_SWINGIDX_SIZE);
odm_move_memory(dm, cali_info->delta_swing_table_idx_2g_cck_b_n,
		(void *)delta_swingidx_mp_2g_cck_b_n_txpwrtrk_type5_8197g,
		DELTA_SWINGIDX_SIZE);

odm_move_memory(dm, cali_info->delta_swing_table_idx_5ga_p,
		(void *)delta_swingidx_mp_5ga_p_txpwrtrk_type5_8197g,
		DELTA_SWINGIDX_SIZE * 3);
odm_move_memory(dm, cali_info->delta_swing_table_idx_5ga_n,
		(void *)delta_swingidx_mp_5ga_n_txpwrtrk_type5_8197g,
		DELTA_SWINGIDX_SIZE * 3);
odm_move_memory(dm, cali_info->delta_swing_table_idx_5gb_p,
		(void *)delta_swingidx_mp_5gb_p_txpwrtrk_type5_8197g,
		DELTA_SWINGIDX_SIZE * 3);
odm_move_memory(dm, cali_info->delta_swing_table_idx_5gb_n,
		(void *)delta_swingidx_mp_5gb_n_txpwrtrk_type5_8197g,
		DELTA_SWINGIDX_SIZE * 3);
#endif
}

/******************************************************************************
 *                           txpowertrack_type6.TXT
 ******************************************************************************/

#ifdef CONFIG_8197G_TYPE6
const u8 delta_swingidx_mp_5gb_n_txpwrtrk_type6_8197g[][D_S_SIZE] = {
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
};

const u8 delta_swingidx_mp_5gb_p_txpwrtrk_type6_8197g[][D_S_SIZE] = {
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
};

const u8 delta_swingidx_mp_5ga_n_txpwrtrk_type6_8197g[][D_S_SIZE] = {
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
};

const u8 delta_swingidx_mp_5ga_p_txpwrtrk_type6_8197g[][D_S_SIZE] = {
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
	{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12},
};

const u8 delta_swingidx_mp_2gb_n_txpwrtrk_type6_8197g[]    = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2gb_p_txpwrtrk_type6_8197g[]    = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2ga_n_txpwrtrk_type6_8197g[]    = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2ga_p_txpwrtrk_type6_8197g[]    = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2g_cck_b_n_txpwrtrk_type6_8197g[] = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2g_cck_b_p_txpwrtrk_type6_8197g[] = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2g_cck_a_n_txpwrtrk_type6_8197g[] = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
const u8 delta_swingidx_mp_2g_cck_a_p_txpwrtrk_type6_8197g[] = {
	0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3,
	 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5};
#endif

void
odm_read_and_config_mp_8197g_txpowertrack_type6(struct dm_struct *dm)
{
#ifdef CONFIG_8197G_TYPE6

struct dm_rf_calibration_struct  *cali_info = &dm->rf_calibrate_info;

PHYDM_DBG(dm, ODM_COMP_INIT, "===> ODM_ReadAndConfig_MP_mp_8197g\n");

odm_move_memory(dm, cali_info->delta_swing_table_idx_2ga_p,
		(void *)delta_swingidx_mp_2ga_p_txpwrtrk_type6_8197g,
		DELTA_SWINGIDX_SIZE);
odm_move_memory(dm, cali_info->delta_swing_table_idx_2ga_n,
		(void *)delta_swingidx_mp_2ga_n_txpwrtrk_type6_8197g,
		DELTA_SWINGIDX_SIZE);
odm_move_memory(dm, cali_info->delta_swing_table_idx_2gb_p,
		(void *)delta_swingidx_mp_2gb_p_txpwrtrk_type6_8197g,
		DELTA_SWINGIDX_SIZE);
odm_move_memory(dm, cali_info->delta_swing_table_idx_2gb_n,
		(void *)delta_swingidx_mp_2gb_n_txpwrtrk_type6_8197g,
		DELTA_SWINGIDX_SIZE);

odm_move_memory(dm, cali_info->delta_swing_table_idx_2g_cck_a_p,
		(void *)delta_swingidx_mp_2g_cck_a_p_txpwrtrk_type6_8197g,
		DELTA_SWINGIDX_SIZE);
odm_move_memory(dm, cali_info->delta_swing_table_idx_2g_cck_a_n,
		(void *)delta_swingidx_mp_2g_cck_a_n_txpwrtrk_type6_8197g,
		DELTA_SWINGIDX_SIZE);
odm_move_memory(dm, cali_info->delta_swing_table_idx_2g_cck_b_p,
		(void *)delta_swingidx_mp_2g_cck_b_p_txpwrtrk_type6_8197g,
		DELTA_SWINGIDX_SIZE);
odm_move_memory(dm, cali_info->delta_swing_table_idx_2g_cck_b_n,
		(void *)delta_swingidx_mp_2g_cck_b_n_txpwrtrk_type6_8197g,
		DELTA_SWINGIDX_SIZE);

odm_move_memory(dm, cali_info->delta_swing_table_idx_5ga_p,
		(void *)delta_swingidx_mp_5ga_p_txpwrtrk_type6_8197g,
		DELTA_SWINGIDX_SIZE * 3);
odm_move_memory(dm, cali_info->delta_swing_table_idx_5ga_n,
		(void *)delta_swingidx_mp_5ga_n_txpwrtrk_type6_8197g,
		DELTA_SWINGIDX_SIZE * 3);
odm_move_memory(dm, cali_info->delta_swing_table_idx_5gb_p,
		(void *)delta_swingidx_mp_5gb_p_txpwrtrk_type6_8197g,
		DELTA_SWINGIDX_SIZE * 3);
odm_move_memory(dm, cali_info->delta_swing_table_idx_5gb_n,
		(void *)delta_swingidx_mp_5gb_n_txpwrtrk_type6_8197g,
		DELTA_SWINGIDX_SIZE * 3);
#endif
}

/******************************************************************************
 *                           txpwr_lmt.TXT
 ******************************************************************************/

#ifdef CONFIG_8197G
const char *array_mp_8197g_txpwr_lmt[] = {
	"FCC", "2.4G", "20M", "CCK", "1T", "01", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "01", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "01", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "02", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "02", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "02", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "03", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "03", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "03", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "04", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "04", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "04", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "05", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "05", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "05", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "06", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "06", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "06", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "07", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "07", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "07", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "08", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "08", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "08", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "09", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "09", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "09", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "10", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "10", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "10", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "11", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "11", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "11", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "12", "63",
	"ETSI", "2.4G", "20M", "CCK", "1T", "12", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "12", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "13", "63",
	"ETSI", "2.4G", "20M", "CCK", "1T", "13", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "13", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "14", "63",
	"ETSI", "2.4G", "20M", "CCK", "1T", "14", "63",
	"MKK", "2.4G", "20M", "CCK", "1T", "14", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "01", "34",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "01", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "01", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "02", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "02", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "02", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "03", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "03", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "03", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "04", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "04", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "04", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "05", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "05", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "05", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "06", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "06", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "06", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "07", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "07", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "07", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "08", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "08", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "08", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "09", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "09", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "09", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "10", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "10", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "10", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "11", "32",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "11", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "11", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "12", "63",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "12", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "12", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "13", "63",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "13", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "13", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "14", "63",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "14", "63",
	"MKK", "2.4G", "20M", "OFDM", "1T", "14", "63",
	"FCC", "2.4G", "20M", "HT", "1T", "01", "34",
	"ETSI", "2.4G", "20M", "HT", "1T", "01", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "01", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "02", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "02", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "02", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "03", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "03", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "03", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "04", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "04", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "04", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "05", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "05", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "05", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "06", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "06", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "06", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "07", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "07", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "07", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "08", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "08", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "08", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "09", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "09", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "09", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "10", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "10", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "10", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "11", "32",
	"ETSI", "2.4G", "20M", "HT", "1T", "11", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "11", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "12", "63",
	"ETSI", "2.4G", "20M", "HT", "1T", "12", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "12", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "13", "63",
	"ETSI", "2.4G", "20M", "HT", "1T", "13", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "13", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "14", "63",
	"ETSI", "2.4G", "20M", "HT", "1T", "14", "63",
	"MKK", "2.4G", "20M", "HT", "1T", "14", "63",
	"FCC", "2.4G", "20M", "HT", "2T", "01", "32",
	"ETSI", "2.4G", "20M", "HT", "2T", "01", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "01", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "02", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "02", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "02", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "03", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "03", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "03", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "04", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "04", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "04", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "05", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "05", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "05", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "06", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "06", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "06", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "07", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "07", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "07", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "08", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "08", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "08", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "09", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "09", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "09", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "10", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "10", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "10", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "11", "30",
	"ETSI", "2.4G", "20M", "HT", "2T", "11", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "11", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "12", "63",
	"ETSI", "2.4G", "20M", "HT", "2T", "12", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "12", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "13", "63",
	"ETSI", "2.4G", "20M", "HT", "2T", "13", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "13", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "14", "63",
	"ETSI", "2.4G", "20M", "HT", "2T", "14", "63",
	"MKK", "2.4G", "20M", "HT", "2T", "14", "63",
	"FCC", "2.4G", "20M", "HT", "3T", "01", "30",
	"ETSI", "2.4G", "20M", "HT", "3T", "01", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "01", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "02", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "02", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "02", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "03", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "03", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "03", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "04", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "04", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "04", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "05", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "05", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "05", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "06", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "06", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "06", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "07", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "07", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "07", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "08", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "08", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "08", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "09", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "09", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "09", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "10", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "10", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "10", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "11", "28",
	"ETSI", "2.4G", "20M", "HT", "3T", "11", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "11", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "12", "63",
	"ETSI", "2.4G", "20M", "HT", "3T", "12", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "12", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "13", "63",
	"ETSI", "2.4G", "20M", "HT", "3T", "13", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "13", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "14", "63",
	"ETSI", "2.4G", "20M", "HT", "3T", "14", "63",
	"MKK", "2.4G", "20M", "HT", "3T", "14", "63",
	"FCC", "2.4G", "20M", "HT", "4T", "01", "28",
	"ETSI", "2.4G", "20M", "HT", "4T", "01", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "01", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "02", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "02", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "02", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "03", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "03", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "03", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "04", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "04", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "04", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "05", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "05", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "05", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "06", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "06", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "06", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "07", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "07", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "07", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "08", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "08", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "08", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "09", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "09", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "09", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "10", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "10", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "10", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "11", "26",
	"ETSI", "2.4G", "20M", "HT", "4T", "11", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "11", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "12", "63",
	"ETSI", "2.4G", "20M", "HT", "4T", "12", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "12", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "13", "63",
	"ETSI", "2.4G", "20M", "HT", "4T", "13", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "13", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "14", "63",
	"ETSI", "2.4G", "20M", "HT", "4T", "14", "63",
	"MKK", "2.4G", "20M", "HT", "4T", "14", "63",
	"FCC", "2.4G", "40M", "HT", "1T", "01", "63",
	"ETSI", "2.4G", "40M", "HT", "1T", "01", "63",
	"MKK", "2.4G", "40M", "HT", "1T", "01", "63",
	"FCC", "2.4G", "40M", "HT", "1T", "02", "63",
	"ETSI", "2.4G", "40M", "HT", "1T", "02", "63",
	"MKK", "2.4G", "40M", "HT", "1T", "02", "63",
	"FCC", "2.4G", "40M", "HT", "1T", "03", "32",
	"ETSI", "2.4G", "40M", "HT", "1T", "03", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "03", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "04", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "04", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "04", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "05", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "05", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "05", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "06", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "06", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "06", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "07", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "07", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "07", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "08", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "08", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "08", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "09", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "09", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "09", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "10", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "10", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "10", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "11", "32",
	"ETSI", "2.4G", "40M", "HT", "1T", "11", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "11", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "12", "63",
	"ETSI", "2.4G", "40M", "HT", "1T", "12", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "12", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "13", "63",
	"ETSI", "2.4G", "40M", "HT", "1T", "13", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "13", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "14", "63",
	"ETSI", "2.4G", "40M", "HT", "1T", "14", "63",
	"MKK", "2.4G", "40M", "HT", "1T", "14", "63",
	"FCC", "2.4G", "40M", "HT", "2T", "01", "63",
	"ETSI", "2.4G", "40M", "HT", "2T", "01", "63",
	"MKK", "2.4G", "40M", "HT", "2T", "01", "63",
	"FCC", "2.4G", "40M", "HT", "2T", "02", "63",
	"ETSI", "2.4G", "40M", "HT", "2T", "02", "63",
	"MKK", "2.4G", "40M", "HT", "2T", "02", "63",
	"FCC", "2.4G", "40M", "HT", "2T", "03", "30",
	"ETSI", "2.4G", "40M", "HT", "2T", "03", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "03", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "04", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "04", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "04", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "05", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "05", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "05", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "06", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "06", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "06", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "07", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "07", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "07", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "08", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "08", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "08", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "09", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "09", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "09", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "10", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "10", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "10", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "11", "30",
	"ETSI", "2.4G", "40M", "HT", "2T", "11", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "11", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "12", "63",
	"ETSI", "2.4G", "40M", "HT", "2T", "12", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "12", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "13", "63",
	"ETSI", "2.4G", "40M", "HT", "2T", "13", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "13", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "14", "63",
	"ETSI", "2.4G", "40M", "HT", "2T", "14", "63",
	"MKK", "2.4G", "40M", "HT", "2T", "14", "63",
	"FCC", "2.4G", "40M", "HT", "3T", "01", "63",
	"ETSI", "2.4G", "40M", "HT", "3T", "01", "63",
	"MKK", "2.4G", "40M", "HT", "3T", "01", "63",
	"FCC", "2.4G", "40M", "HT", "3T", "02", "63",
	"ETSI", "2.4G", "40M", "HT", "3T", "02", "63",
	"MKK", "2.4G", "40M", "HT", "3T", "02", "63",
	"FCC", "2.4G", "40M", "HT", "3T", "03", "28",
	"ETSI", "2.4G", "40M", "HT", "3T", "03", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "03", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "04", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "04", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "04", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "05", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "05", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "05", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "06", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "06", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "06", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "07", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "07", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "07", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "08", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "08", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "08", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "09", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "09", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "09", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "10", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "10", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "10", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "11", "28",
	"ETSI", "2.4G", "40M", "HT", "3T", "11", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "11", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "12", "63",
	"ETSI", "2.4G", "40M", "HT", "3T", "12", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "12", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "13", "63",
	"ETSI", "2.4G", "40M", "HT", "3T", "13", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "13", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "14", "63",
	"ETSI", "2.4G", "40M", "HT", "3T", "14", "63",
	"MKK", "2.4G", "40M", "HT", "3T", "14", "63",
	"FCC", "2.4G", "40M", "HT", "4T", "01", "63",
	"ETSI", "2.4G", "40M", "HT", "4T", "01", "63",
	"MKK", "2.4G", "40M", "HT", "4T", "01", "63",
	"FCC", "2.4G", "40M", "HT", "4T", "02", "63",
	"ETSI", "2.4G", "40M", "HT", "4T", "02", "63",
	"MKK", "2.4G", "40M", "HT", "4T", "02", "63",
	"FCC", "2.4G", "40M", "HT", "4T", "03", "26",
	"ETSI", "2.4G", "40M", "HT", "4T", "03", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "03", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "04", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "04", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "04", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "05", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "05", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "05", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "06", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "06", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "06", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "07", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "07", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "07", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "08", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "08", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "08", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "09", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "09", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "09", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "10", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "10", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "10", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "11", "26",
	"ETSI", "2.4G", "40M", "HT", "4T", "11", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "11", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "12", "63",
	"ETSI", "2.4G", "40M", "HT", "4T", "12", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "12", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "13", "63",
	"ETSI", "2.4G", "40M", "HT", "4T", "13", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "13", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "14", "63",
	"ETSI", "2.4G", "40M", "HT", "4T", "14", "63",
	"MKK", "2.4G", "40M", "HT", "4T", "14", "63"
};
#endif

void
odm_read_and_config_mp_8197g_txpwr_lmt(struct dm_struct *dm)
{
#ifdef CONFIG_8197G

	u32	i = 0;
#if (DM_ODM_SUPPORT_TYPE == ODM_IOT)
	u32	array_len =
			sizeof(array_mp_8197g_txpwr_lmt) / sizeof(u8);
	u8	*array = (u8 *)array_mp_8197g_txpwr_lmt;
#else
	u32	array_len =
			sizeof(array_mp_8197g_txpwr_lmt) / sizeof(u8 *);
	u8	**array = (u8 **)array_mp_8197g_txpwr_lmt;
#endif

#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	void	*adapter = dm->adapter;
	HAL_DATA_TYPE	*hal_data = GET_HAL_DATA(((PADAPTER)adapter));

	odm_memory_set(dm, hal_data->BufOfLinesPwrLmt, 0,
		       MAX_LINES_HWCONFIG_TXT *
		       MAX_BYTES_LINE_HWCONFIG_TXT);
	hal_data->nLinesReadPwrLmt = array_len / 7;
#endif

	PHYDM_DBG(dm, ODM_COMP_INIT, "===> %s\n", __func__);

	for (i = 0; i < array_len; i += 7) {
#if (DM_ODM_SUPPORT_TYPE == ODM_IOT)
		u8	regulation = array[i];
		u8	band = array[i + 1];
		u8	bandwidth = array[i + 2];
		u8	rate = array[i + 3];
		u8	rf_path = array[i + 4];
		u8	chnl = array[i + 5];
		u8	val = array[i + 6];
#else
		u8	*regulation = array[i];
		u8	*band = array[i + 1];
		u8	*bandwidth = array[i + 2];
		u8	*rate = array[i + 3];
		u8	*rf_path = array[i + 4];
		u8	*chnl = array[i + 5];
		u8	*val = array[i + 6];
#endif

		odm_config_bb_txpwr_lmt_8197g(dm, regulation, band, bandwidth,
					      rate, rf_path, chnl, val);
#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
		rsprintf((char *)hal_data->BufOfLinesPwrLmt[i / 7], 100, "\"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\",",
			 regulation, band, bandwidth, rate, rf_path, chnl, val);
#endif
	}

#endif
}

/******************************************************************************
 *                           txpwr_lmt_8197gmp_type0.TXT
 ******************************************************************************/

#ifdef CONFIG_8197G_8197GMP_TYPE0
const char *array_mp_8197g_txpwr_lmt_8197gmp_type0[] = {
	"FCC", "2.4G", "20M", "CCK", "1T", "01", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "01", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "01", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "02", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "02", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "02", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "03", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "03", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "03", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "04", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "04", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "04", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "05", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "05", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "05", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "06", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "06", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "06", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "07", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "07", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "07", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "08", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "08", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "08", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "09", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "09", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "09", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "10", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "10", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "10", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "11", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "11", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "11", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "12", "63",
	"ETSI", "2.4G", "20M", "CCK", "1T", "12", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "12", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "13", "63",
	"ETSI", "2.4G", "20M", "CCK", "1T", "13", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "13", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "14", "63",
	"ETSI", "2.4G", "20M", "CCK", "1T", "14", "63",
	"MKK", "2.4G", "20M", "CCK", "1T", "14", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "01", "34",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "01", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "01", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "02", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "02", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "02", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "03", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "03", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "03", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "04", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "04", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "04", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "05", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "05", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "05", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "06", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "06", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "06", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "07", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "07", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "07", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "08", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "08", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "08", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "09", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "09", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "09", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "10", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "10", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "10", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "11", "32",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "11", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "11", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "12", "63",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "12", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "12", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "13", "63",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "13", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "13", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "14", "63",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "14", "63",
	"MKK", "2.4G", "20M", "OFDM", "1T", "14", "63",
	"FCC", "2.4G", "20M", "HT", "1T", "01", "34",
	"ETSI", "2.4G", "20M", "HT", "1T", "01", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "01", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "02", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "02", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "02", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "03", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "03", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "03", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "04", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "04", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "04", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "05", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "05", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "05", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "06", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "06", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "06", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "07", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "07", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "07", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "08", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "08", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "08", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "09", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "09", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "09", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "10", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "10", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "10", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "11", "32",
	"ETSI", "2.4G", "20M", "HT", "1T", "11", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "11", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "12", "63",
	"ETSI", "2.4G", "20M", "HT", "1T", "12", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "12", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "13", "63",
	"ETSI", "2.4G", "20M", "HT", "1T", "13", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "13", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "14", "63",
	"ETSI", "2.4G", "20M", "HT", "1T", "14", "63",
	"MKK", "2.4G", "20M", "HT", "1T", "14", "63",
	"FCC", "2.4G", "20M", "HT", "2T", "01", "32",
	"ETSI", "2.4G", "20M", "HT", "2T", "01", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "01", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "02", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "02", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "02", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "03", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "03", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "03", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "04", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "04", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "04", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "05", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "05", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "05", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "06", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "06", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "06", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "07", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "07", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "07", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "08", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "08", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "08", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "09", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "09", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "09", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "10", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "10", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "10", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "11", "30",
	"ETSI", "2.4G", "20M", "HT", "2T", "11", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "11", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "12", "63",
	"ETSI", "2.4G", "20M", "HT", "2T", "12", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "12", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "13", "63",
	"ETSI", "2.4G", "20M", "HT", "2T", "13", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "13", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "14", "63",
	"ETSI", "2.4G", "20M", "HT", "2T", "14", "63",
	"MKK", "2.4G", "20M", "HT", "2T", "14", "63",
	"FCC", "2.4G", "20M", "HT", "3T", "01", "30",
	"ETSI", "2.4G", "20M", "HT", "3T", "01", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "01", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "02", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "02", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "02", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "03", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "03", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "03", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "04", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "04", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "04", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "05", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "05", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "05", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "06", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "06", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "06", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "07", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "07", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "07", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "08", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "08", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "08", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "09", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "09", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "09", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "10", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "10", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "10", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "11", "28",
	"ETSI", "2.4G", "20M", "HT", "3T", "11", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "11", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "12", "63",
	"ETSI", "2.4G", "20M", "HT", "3T", "12", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "12", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "13", "63",
	"ETSI", "2.4G", "20M", "HT", "3T", "13", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "13", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "14", "63",
	"ETSI", "2.4G", "20M", "HT", "3T", "14", "63",
	"MKK", "2.4G", "20M", "HT", "3T", "14", "63",
	"FCC", "2.4G", "20M", "HT", "4T", "01", "28",
	"ETSI", "2.4G", "20M", "HT", "4T", "01", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "01", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "02", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "02", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "02", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "03", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "03", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "03", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "04", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "04", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "04", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "05", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "05", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "05", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "06", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "06", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "06", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "07", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "07", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "07", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "08", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "08", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "08", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "09", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "09", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "09", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "10", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "10", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "10", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "11", "26",
	"ETSI", "2.4G", "20M", "HT", "4T", "11", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "11", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "12", "63",
	"ETSI", "2.4G", "20M", "HT", "4T", "12", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "12", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "13", "63",
	"ETSI", "2.4G", "20M", "HT", "4T", "13", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "13", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "14", "63",
	"ETSI", "2.4G", "20M", "HT", "4T", "14", "63",
	"MKK", "2.4G", "20M", "HT", "4T", "14", "63",
	"FCC", "2.4G", "40M", "HT", "1T", "01", "63",
	"ETSI", "2.4G", "40M", "HT", "1T", "01", "63",
	"MKK", "2.4G", "40M", "HT", "1T", "01", "63",
	"FCC", "2.4G", "40M", "HT", "1T", "02", "63",
	"ETSI", "2.4G", "40M", "HT", "1T", "02", "63",
	"MKK", "2.4G", "40M", "HT", "1T", "02", "63",
	"FCC", "2.4G", "40M", "HT", "1T", "03", "32",
	"ETSI", "2.4G", "40M", "HT", "1T", "03", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "03", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "04", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "04", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "04", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "05", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "05", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "05", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "06", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "06", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "06", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "07", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "07", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "07", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "08", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "08", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "08", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "09", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "09", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "09", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "10", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "10", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "10", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "11", "32",
	"ETSI", "2.4G", "40M", "HT", "1T", "11", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "11", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "12", "63",
	"ETSI", "2.4G", "40M", "HT", "1T", "12", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "12", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "13", "63",
	"ETSI", "2.4G", "40M", "HT", "1T", "13", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "13", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "14", "63",
	"ETSI", "2.4G", "40M", "HT", "1T", "14", "63",
	"MKK", "2.4G", "40M", "HT", "1T", "14", "63",
	"FCC", "2.4G", "40M", "HT", "2T", "01", "63",
	"ETSI", "2.4G", "40M", "HT", "2T", "01", "63",
	"MKK", "2.4G", "40M", "HT", "2T", "01", "63",
	"FCC", "2.4G", "40M", "HT", "2T", "02", "63",
	"ETSI", "2.4G", "40M", "HT", "2T", "02", "63",
	"MKK", "2.4G", "40M", "HT", "2T", "02", "63",
	"FCC", "2.4G", "40M", "HT", "2T", "03", "30",
	"ETSI", "2.4G", "40M", "HT", "2T", "03", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "03", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "04", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "04", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "04", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "05", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "05", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "05", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "06", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "06", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "06", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "07", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "07", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "07", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "08", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "08", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "08", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "09", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "09", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "09", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "10", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "10", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "10", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "11", "30",
	"ETSI", "2.4G", "40M", "HT", "2T", "11", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "11", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "12", "63",
	"ETSI", "2.4G", "40M", "HT", "2T", "12", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "12", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "13", "63",
	"ETSI", "2.4G", "40M", "HT", "2T", "13", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "13", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "14", "63",
	"ETSI", "2.4G", "40M", "HT", "2T", "14", "63",
	"MKK", "2.4G", "40M", "HT", "2T", "14", "63",
	"FCC", "2.4G", "40M", "HT", "3T", "01", "63",
	"ETSI", "2.4G", "40M", "HT", "3T", "01", "63",
	"MKK", "2.4G", "40M", "HT", "3T", "01", "63",
	"FCC", "2.4G", "40M", "HT", "3T", "02", "63",
	"ETSI", "2.4G", "40M", "HT", "3T", "02", "63",
	"MKK", "2.4G", "40M", "HT", "3T", "02", "63",
	"FCC", "2.4G", "40M", "HT", "3T", "03", "28",
	"ETSI", "2.4G", "40M", "HT", "3T", "03", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "03", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "04", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "04", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "04", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "05", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "05", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "05", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "06", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "06", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "06", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "07", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "07", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "07", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "08", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "08", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "08", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "09", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "09", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "09", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "10", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "10", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "10", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "11", "28",
	"ETSI", "2.4G", "40M", "HT", "3T", "11", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "11", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "12", "63",
	"ETSI", "2.4G", "40M", "HT", "3T", "12", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "12", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "13", "63",
	"ETSI", "2.4G", "40M", "HT", "3T", "13", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "13", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "14", "63",
	"ETSI", "2.4G", "40M", "HT", "3T", "14", "63",
	"MKK", "2.4G", "40M", "HT", "3T", "14", "63",
	"FCC", "2.4G", "40M", "HT", "4T", "01", "63",
	"ETSI", "2.4G", "40M", "HT", "4T", "01", "63",
	"MKK", "2.4G", "40M", "HT", "4T", "01", "63",
	"FCC", "2.4G", "40M", "HT", "4T", "02", "63",
	"ETSI", "2.4G", "40M", "HT", "4T", "02", "63",
	"MKK", "2.4G", "40M", "HT", "4T", "02", "63",
	"FCC", "2.4G", "40M", "HT", "4T", "03", "26",
	"ETSI", "2.4G", "40M", "HT", "4T", "03", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "03", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "04", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "04", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "04", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "05", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "05", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "05", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "06", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "06", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "06", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "07", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "07", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "07", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "08", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "08", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "08", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "09", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "09", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "09", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "10", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "10", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "10", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "11", "26",
	"ETSI", "2.4G", "40M", "HT", "4T", "11", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "11", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "12", "63",
	"ETSI", "2.4G", "40M", "HT", "4T", "12", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "12", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "13", "63",
	"ETSI", "2.4G", "40M", "HT", "4T", "13", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "13", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "14", "63",
	"ETSI", "2.4G", "40M", "HT", "4T", "14", "63",
	"MKK", "2.4G", "40M", "HT", "4T", "14", "63"
};
#endif

void
odm_read_and_config_mp_8197g_txpwr_lmt_8197gmp_type0(struct dm_struct *dm)
{
#ifdef CONFIG_8197G_8197GMP_TYPE0

	u32	i = 0;
#if (DM_ODM_SUPPORT_TYPE == ODM_IOT)
	u32	array_len =
			sizeof(array_mp_8197g_txpwr_lmt_8197gmp_type0) / sizeof(u8);
	u8	*array = (u8 *)array_mp_8197g_txpwr_lmt_8197gmp_type0;
#else
	u32	array_len =
			sizeof(array_mp_8197g_txpwr_lmt_8197gmp_type0) / sizeof(u8 *);
	u8	**array = (u8 **)array_mp_8197g_txpwr_lmt_8197gmp_type0;
#endif

#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	void	*adapter = dm->adapter;
	HAL_DATA_TYPE	*hal_data = GET_HAL_DATA(((PADAPTER)adapter));

	odm_memory_set(dm, hal_data->BufOfLinesPwrLmt, 0,
		       MAX_LINES_HWCONFIG_TXT *
		       MAX_BYTES_LINE_HWCONFIG_TXT);
	hal_data->nLinesReadPwrLmt = array_len / 7;
#endif

	PHYDM_DBG(dm, ODM_COMP_INIT, "===> %s\n", __func__);

	for (i = 0; i < array_len; i += 7) {
#if (DM_ODM_SUPPORT_TYPE == ODM_IOT)
		u8	regulation = array[i];
		u8	band = array[i + 1];
		u8	bandwidth = array[i + 2];
		u8	rate = array[i + 3];
		u8	rf_path = array[i + 4];
		u8	chnl = array[i + 5];
		u8	val = array[i + 6];
#else
		u8	*regulation = array[i];
		u8	*band = array[i + 1];
		u8	*bandwidth = array[i + 2];
		u8	*rate = array[i + 3];
		u8	*rf_path = array[i + 4];
		u8	*chnl = array[i + 5];
		u8	*val = array[i + 6];
#endif

		odm_config_bb_txpwr_lmt_8197g(dm, regulation, band, bandwidth,
					      rate, rf_path, chnl, val);
#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
		rsprintf((char *)hal_data->BufOfLinesPwrLmt[i / 7], 100, "\"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\",",
			 regulation, band, bandwidth, rate, rf_path, chnl, val);
#endif
	}

#endif
}

/******************************************************************************
 *                           txpwr_lmt_8197gmp_type1.TXT
 ******************************************************************************/

#ifdef CONFIG_8197G_8197GMP_TYPE1
const char *array_mp_8197g_txpwr_lmt_8197gmp_type1[] = {
	"FCC", "2.4G", "20M", "CCK", "1T", "01", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "01", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "01", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "02", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "02", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "02", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "03", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "03", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "03", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "04", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "04", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "04", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "05", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "05", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "05", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "06", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "06", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "06", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "07", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "07", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "07", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "08", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "08", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "08", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "09", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "09", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "09", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "10", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "10", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "10", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "11", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "11", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "11", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "12", "63",
	"ETSI", "2.4G", "20M", "CCK", "1T", "12", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "12", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "13", "63",
	"ETSI", "2.4G", "20M", "CCK", "1T", "13", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "13", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "14", "63",
	"ETSI", "2.4G", "20M", "CCK", "1T", "14", "63",
	"MKK", "2.4G", "20M", "CCK", "1T", "14", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "01", "34",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "01", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "01", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "02", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "02", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "02", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "03", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "03", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "03", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "04", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "04", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "04", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "05", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "05", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "05", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "06", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "06", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "06", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "07", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "07", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "07", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "08", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "08", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "08", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "09", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "09", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "09", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "10", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "10", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "10", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "11", "32",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "11", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "11", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "12", "63",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "12", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "12", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "13", "63",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "13", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "13", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "14", "63",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "14", "63",
	"MKK", "2.4G", "20M", "OFDM", "1T", "14", "63",
	"FCC", "2.4G", "20M", "HT", "1T", "01", "34",
	"ETSI", "2.4G", "20M", "HT", "1T", "01", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "01", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "02", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "02", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "02", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "03", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "03", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "03", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "04", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "04", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "04", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "05", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "05", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "05", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "06", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "06", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "06", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "07", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "07", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "07", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "08", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "08", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "08", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "09", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "09", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "09", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "10", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "10", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "10", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "11", "32",
	"ETSI", "2.4G", "20M", "HT", "1T", "11", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "11", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "12", "63",
	"ETSI", "2.4G", "20M", "HT", "1T", "12", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "12", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "13", "63",
	"ETSI", "2.4G", "20M", "HT", "1T", "13", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "13", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "14", "63",
	"ETSI", "2.4G", "20M", "HT", "1T", "14", "63",
	"MKK", "2.4G", "20M", "HT", "1T", "14", "63",
	"FCC", "2.4G", "20M", "HT", "2T", "01", "32",
	"ETSI", "2.4G", "20M", "HT", "2T", "01", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "01", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "02", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "02", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "02", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "03", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "03", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "03", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "04", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "04", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "04", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "05", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "05", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "05", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "06", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "06", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "06", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "07", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "07", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "07", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "08", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "08", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "08", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "09", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "09", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "09", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "10", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "10", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "10", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "11", "30",
	"ETSI", "2.4G", "20M", "HT", "2T", "11", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "11", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "12", "63",
	"ETSI", "2.4G", "20M", "HT", "2T", "12", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "12", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "13", "63",
	"ETSI", "2.4G", "20M", "HT", "2T", "13", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "13", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "14", "63",
	"ETSI", "2.4G", "20M", "HT", "2T", "14", "63",
	"MKK", "2.4G", "20M", "HT", "2T", "14", "63",
	"FCC", "2.4G", "20M", "HT", "3T", "01", "30",
	"ETSI", "2.4G", "20M", "HT", "3T", "01", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "01", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "02", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "02", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "02", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "03", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "03", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "03", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "04", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "04", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "04", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "05", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "05", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "05", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "06", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "06", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "06", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "07", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "07", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "07", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "08", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "08", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "08", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "09", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "09", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "09", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "10", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "10", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "10", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "11", "28",
	"ETSI", "2.4G", "20M", "HT", "3T", "11", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "11", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "12", "63",
	"ETSI", "2.4G", "20M", "HT", "3T", "12", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "12", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "13", "63",
	"ETSI", "2.4G", "20M", "HT", "3T", "13", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "13", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "14", "63",
	"ETSI", "2.4G", "20M", "HT", "3T", "14", "63",
	"MKK", "2.4G", "20M", "HT", "3T", "14", "63",
	"FCC", "2.4G", "20M", "HT", "4T", "01", "28",
	"ETSI", "2.4G", "20M", "HT", "4T", "01", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "01", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "02", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "02", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "02", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "03", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "03", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "03", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "04", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "04", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "04", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "05", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "05", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "05", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "06", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "06", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "06", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "07", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "07", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "07", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "08", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "08", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "08", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "09", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "09", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "09", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "10", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "10", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "10", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "11", "26",
	"ETSI", "2.4G", "20M", "HT", "4T", "11", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "11", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "12", "63",
	"ETSI", "2.4G", "20M", "HT", "4T", "12", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "12", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "13", "63",
	"ETSI", "2.4G", "20M", "HT", "4T", "13", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "13", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "14", "63",
	"ETSI", "2.4G", "20M", "HT", "4T", "14", "63",
	"MKK", "2.4G", "20M", "HT", "4T", "14", "63",
	"FCC", "2.4G", "40M", "HT", "1T", "01", "63",
	"ETSI", "2.4G", "40M", "HT", "1T", "01", "63",
	"MKK", "2.4G", "40M", "HT", "1T", "01", "63",
	"FCC", "2.4G", "40M", "HT", "1T", "02", "63",
	"ETSI", "2.4G", "40M", "HT", "1T", "02", "63",
	"MKK", "2.4G", "40M", "HT", "1T", "02", "63",
	"FCC", "2.4G", "40M", "HT", "1T", "03", "32",
	"ETSI", "2.4G", "40M", "HT", "1T", "03", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "03", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "04", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "04", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "04", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "05", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "05", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "05", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "06", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "06", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "06", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "07", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "07", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "07", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "08", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "08", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "08", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "09", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "09", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "09", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "10", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "10", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "10", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "11", "32",
	"ETSI", "2.4G", "40M", "HT", "1T", "11", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "11", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "12", "63",
	"ETSI", "2.4G", "40M", "HT", "1T", "12", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "12", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "13", "63",
	"ETSI", "2.4G", "40M", "HT", "1T", "13", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "13", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "14", "63",
	"ETSI", "2.4G", "40M", "HT", "1T", "14", "63",
	"MKK", "2.4G", "40M", "HT", "1T", "14", "63",
	"FCC", "2.4G", "40M", "HT", "2T", "01", "63",
	"ETSI", "2.4G", "40M", "HT", "2T", "01", "63",
	"MKK", "2.4G", "40M", "HT", "2T", "01", "63",
	"FCC", "2.4G", "40M", "HT", "2T", "02", "63",
	"ETSI", "2.4G", "40M", "HT", "2T", "02", "63",
	"MKK", "2.4G", "40M", "HT", "2T", "02", "63",
	"FCC", "2.4G", "40M", "HT", "2T", "03", "30",
	"ETSI", "2.4G", "40M", "HT", "2T", "03", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "03", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "04", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "04", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "04", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "05", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "05", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "05", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "06", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "06", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "06", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "07", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "07", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "07", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "08", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "08", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "08", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "09", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "09", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "09", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "10", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "10", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "10", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "11", "30",
	"ETSI", "2.4G", "40M", "HT", "2T", "11", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "11", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "12", "63",
	"ETSI", "2.4G", "40M", "HT", "2T", "12", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "12", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "13", "63",
	"ETSI", "2.4G", "40M", "HT", "2T", "13", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "13", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "14", "63",
	"ETSI", "2.4G", "40M", "HT", "2T", "14", "63",
	"MKK", "2.4G", "40M", "HT", "2T", "14", "63",
	"FCC", "2.4G", "40M", "HT", "3T", "01", "63",
	"ETSI", "2.4G", "40M", "HT", "3T", "01", "63",
	"MKK", "2.4G", "40M", "HT", "3T", "01", "63",
	"FCC", "2.4G", "40M", "HT", "3T", "02", "63",
	"ETSI", "2.4G", "40M", "HT", "3T", "02", "63",
	"MKK", "2.4G", "40M", "HT", "3T", "02", "63",
	"FCC", "2.4G", "40M", "HT", "3T", "03", "28",
	"ETSI", "2.4G", "40M", "HT", "3T", "03", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "03", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "04", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "04", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "04", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "05", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "05", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "05", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "06", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "06", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "06", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "07", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "07", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "07", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "08", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "08", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "08", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "09", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "09", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "09", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "10", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "10", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "10", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "11", "28",
	"ETSI", "2.4G", "40M", "HT", "3T", "11", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "11", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "12", "63",
	"ETSI", "2.4G", "40M", "HT", "3T", "12", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "12", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "13", "63",
	"ETSI", "2.4G", "40M", "HT", "3T", "13", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "13", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "14", "63",
	"ETSI", "2.4G", "40M", "HT", "3T", "14", "63",
	"MKK", "2.4G", "40M", "HT", "3T", "14", "63",
	"FCC", "2.4G", "40M", "HT", "4T", "01", "63",
	"ETSI", "2.4G", "40M", "HT", "4T", "01", "63",
	"MKK", "2.4G", "40M", "HT", "4T", "01", "63",
	"FCC", "2.4G", "40M", "HT", "4T", "02", "63",
	"ETSI", "2.4G", "40M", "HT", "4T", "02", "63",
	"MKK", "2.4G", "40M", "HT", "4T", "02", "63",
	"FCC", "2.4G", "40M", "HT", "4T", "03", "26",
	"ETSI", "2.4G", "40M", "HT", "4T", "03", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "03", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "04", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "04", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "04", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "05", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "05", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "05", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "06", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "06", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "06", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "07", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "07", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "07", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "08", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "08", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "08", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "09", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "09", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "09", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "10", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "10", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "10", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "11", "26",
	"ETSI", "2.4G", "40M", "HT", "4T", "11", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "11", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "12", "63",
	"ETSI", "2.4G", "40M", "HT", "4T", "12", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "12", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "13", "63",
	"ETSI", "2.4G", "40M", "HT", "4T", "13", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "13", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "14", "63",
	"ETSI", "2.4G", "40M", "HT", "4T", "14", "63",
	"MKK", "2.4G", "40M", "HT", "4T", "14", "63"
};
#endif

void
odm_read_and_config_mp_8197g_txpwr_lmt_8197gmp_type1(struct dm_struct *dm)
{
#ifdef CONFIG_8197G_8197GMP_TYPE1

	u32	i = 0;
#if (DM_ODM_SUPPORT_TYPE == ODM_IOT)
	u32	array_len =
			sizeof(array_mp_8197g_txpwr_lmt_8197gmp_type1) / sizeof(u8);
	u8	*array = (u8 *)array_mp_8197g_txpwr_lmt_8197gmp_type1;
#else
	u32	array_len =
			sizeof(array_mp_8197g_txpwr_lmt_8197gmp_type1) / sizeof(u8 *);
	u8	**array = (u8 **)array_mp_8197g_txpwr_lmt_8197gmp_type1;
#endif

#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	void	*adapter = dm->adapter;
	HAL_DATA_TYPE	*hal_data = GET_HAL_DATA(((PADAPTER)adapter));

	odm_memory_set(dm, hal_data->BufOfLinesPwrLmt, 0,
		       MAX_LINES_HWCONFIG_TXT *
		       MAX_BYTES_LINE_HWCONFIG_TXT);
	hal_data->nLinesReadPwrLmt = array_len / 7;
#endif

	PHYDM_DBG(dm, ODM_COMP_INIT, "===> %s\n", __func__);

	for (i = 0; i < array_len; i += 7) {
#if (DM_ODM_SUPPORT_TYPE == ODM_IOT)
		u8	regulation = array[i];
		u8	band = array[i + 1];
		u8	bandwidth = array[i + 2];
		u8	rate = array[i + 3];
		u8	rf_path = array[i + 4];
		u8	chnl = array[i + 5];
		u8	val = array[i + 6];
#else
		u8	*regulation = array[i];
		u8	*band = array[i + 1];
		u8	*bandwidth = array[i + 2];
		u8	*rate = array[i + 3];
		u8	*rf_path = array[i + 4];
		u8	*chnl = array[i + 5];
		u8	*val = array[i + 6];
#endif

		odm_config_bb_txpwr_lmt_8197g(dm, regulation, band, bandwidth,
					      rate, rf_path, chnl, val);
#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
		rsprintf((char *)hal_data->BufOfLinesPwrLmt[i / 7], 100, "\"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\",",
			 regulation, band, bandwidth, rate, rf_path, chnl, val);
#endif
	}

#endif
}

/******************************************************************************
 *                           txpwr_lmt_8197gmp_type2.TXT
 ******************************************************************************/

#ifdef CONFIG_8197G_8197GMP_TYPE2
const char *array_mp_8197g_txpwr_lmt_8197gmp_type2[] = {
	"FCC", "2.4G", "20M", "CCK", "1T", "01", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "01", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "01", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "02", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "02", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "02", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "03", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "03", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "03", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "04", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "04", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "04", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "05", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "05", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "05", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "06", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "06", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "06", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "07", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "07", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "07", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "08", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "08", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "08", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "09", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "09", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "09", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "10", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "10", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "10", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "11", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "11", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "11", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "12", "63",
	"ETSI", "2.4G", "20M", "CCK", "1T", "12", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "12", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "13", "63",
	"ETSI", "2.4G", "20M", "CCK", "1T", "13", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "13", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "14", "63",
	"ETSI", "2.4G", "20M", "CCK", "1T", "14", "63",
	"MKK", "2.4G", "20M", "CCK", "1T", "14", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "01", "34",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "01", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "01", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "02", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "02", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "02", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "03", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "03", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "03", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "04", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "04", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "04", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "05", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "05", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "05", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "06", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "06", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "06", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "07", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "07", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "07", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "08", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "08", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "08", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "09", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "09", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "09", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "10", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "10", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "10", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "11", "32",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "11", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "11", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "12", "63",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "12", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "12", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "13", "63",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "13", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "13", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "14", "63",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "14", "63",
	"MKK", "2.4G", "20M", "OFDM", "1T", "14", "63",
	"FCC", "2.4G", "20M", "HT", "1T", "01", "34",
	"ETSI", "2.4G", "20M", "HT", "1T", "01", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "01", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "02", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "02", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "02", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "03", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "03", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "03", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "04", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "04", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "04", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "05", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "05", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "05", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "06", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "06", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "06", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "07", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "07", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "07", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "08", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "08", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "08", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "09", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "09", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "09", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "10", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "10", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "10", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "11", "32",
	"ETSI", "2.4G", "20M", "HT", "1T", "11", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "11", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "12", "63",
	"ETSI", "2.4G", "20M", "HT", "1T", "12", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "12", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "13", "63",
	"ETSI", "2.4G", "20M", "HT", "1T", "13", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "13", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "14", "63",
	"ETSI", "2.4G", "20M", "HT", "1T", "14", "63",
	"MKK", "2.4G", "20M", "HT", "1T", "14", "63",
	"FCC", "2.4G", "20M", "HT", "2T", "01", "32",
	"ETSI", "2.4G", "20M", "HT", "2T", "01", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "01", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "02", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "02", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "02", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "03", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "03", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "03", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "04", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "04", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "04", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "05", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "05", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "05", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "06", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "06", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "06", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "07", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "07", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "07", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "08", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "08", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "08", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "09", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "09", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "09", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "10", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "10", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "10", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "11", "30",
	"ETSI", "2.4G", "20M", "HT", "2T", "11", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "11", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "12", "63",
	"ETSI", "2.4G", "20M", "HT", "2T", "12", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "12", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "13", "63",
	"ETSI", "2.4G", "20M", "HT", "2T", "13", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "13", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "14", "63",
	"ETSI", "2.4G", "20M", "HT", "2T", "14", "63",
	"MKK", "2.4G", "20M", "HT", "2T", "14", "63",
	"FCC", "2.4G", "20M", "HT", "3T", "01", "30",
	"ETSI", "2.4G", "20M", "HT", "3T", "01", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "01", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "02", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "02", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "02", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "03", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "03", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "03", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "04", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "04", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "04", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "05", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "05", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "05", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "06", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "06", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "06", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "07", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "07", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "07", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "08", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "08", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "08", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "09", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "09", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "09", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "10", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "10", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "10", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "11", "28",
	"ETSI", "2.4G", "20M", "HT", "3T", "11", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "11", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "12", "63",
	"ETSI", "2.4G", "20M", "HT", "3T", "12", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "12", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "13", "63",
	"ETSI", "2.4G", "20M", "HT", "3T", "13", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "13", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "14", "63",
	"ETSI", "2.4G", "20M", "HT", "3T", "14", "63",
	"MKK", "2.4G", "20M", "HT", "3T", "14", "63",
	"FCC", "2.4G", "20M", "HT", "4T", "01", "28",
	"ETSI", "2.4G", "20M", "HT", "4T", "01", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "01", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "02", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "02", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "02", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "03", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "03", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "03", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "04", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "04", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "04", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "05", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "05", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "05", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "06", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "06", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "06", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "07", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "07", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "07", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "08", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "08", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "08", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "09", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "09", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "09", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "10", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "10", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "10", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "11", "26",
	"ETSI", "2.4G", "20M", "HT", "4T", "11", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "11", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "12", "63",
	"ETSI", "2.4G", "20M", "HT", "4T", "12", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "12", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "13", "63",
	"ETSI", "2.4G", "20M", "HT", "4T", "13", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "13", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "14", "63",
	"ETSI", "2.4G", "20M", "HT", "4T", "14", "63",
	"MKK", "2.4G", "20M", "HT", "4T", "14", "63",
	"FCC", "2.4G", "40M", "HT", "1T", "01", "63",
	"ETSI", "2.4G", "40M", "HT", "1T", "01", "63",
	"MKK", "2.4G", "40M", "HT", "1T", "01", "63",
	"FCC", "2.4G", "40M", "HT", "1T", "02", "63",
	"ETSI", "2.4G", "40M", "HT", "1T", "02", "63",
	"MKK", "2.4G", "40M", "HT", "1T", "02", "63",
	"FCC", "2.4G", "40M", "HT", "1T", "03", "32",
	"ETSI", "2.4G", "40M", "HT", "1T", "03", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "03", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "04", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "04", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "04", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "05", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "05", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "05", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "06", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "06", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "06", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "07", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "07", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "07", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "08", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "08", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "08", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "09", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "09", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "09", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "10", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "10", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "10", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "11", "32",
	"ETSI", "2.4G", "40M", "HT", "1T", "11", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "11", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "12", "63",
	"ETSI", "2.4G", "40M", "HT", "1T", "12", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "12", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "13", "63",
	"ETSI", "2.4G", "40M", "HT", "1T", "13", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "13", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "14", "63",
	"ETSI", "2.4G", "40M", "HT", "1T", "14", "63",
	"MKK", "2.4G", "40M", "HT", "1T", "14", "63",
	"FCC", "2.4G", "40M", "HT", "2T", "01", "63",
	"ETSI", "2.4G", "40M", "HT", "2T", "01", "63",
	"MKK", "2.4G", "40M", "HT", "2T", "01", "63",
	"FCC", "2.4G", "40M", "HT", "2T", "02", "63",
	"ETSI", "2.4G", "40M", "HT", "2T", "02", "63",
	"MKK", "2.4G", "40M", "HT", "2T", "02", "63",
	"FCC", "2.4G", "40M", "HT", "2T", "03", "30",
	"ETSI", "2.4G", "40M", "HT", "2T", "03", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "03", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "04", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "04", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "04", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "05", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "05", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "05", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "06", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "06", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "06", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "07", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "07", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "07", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "08", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "08", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "08", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "09", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "09", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "09", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "10", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "10", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "10", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "11", "30",
	"ETSI", "2.4G", "40M", "HT", "2T", "11", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "11", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "12", "63",
	"ETSI", "2.4G", "40M", "HT", "2T", "12", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "12", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "13", "63",
	"ETSI", "2.4G", "40M", "HT", "2T", "13", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "13", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "14", "63",
	"ETSI", "2.4G", "40M", "HT", "2T", "14", "63",
	"MKK", "2.4G", "40M", "HT", "2T", "14", "63",
	"FCC", "2.4G", "40M", "HT", "3T", "01", "63",
	"ETSI", "2.4G", "40M", "HT", "3T", "01", "63",
	"MKK", "2.4G", "40M", "HT", "3T", "01", "63",
	"FCC", "2.4G", "40M", "HT", "3T", "02", "63",
	"ETSI", "2.4G", "40M", "HT", "3T", "02", "63",
	"MKK", "2.4G", "40M", "HT", "3T", "02", "63",
	"FCC", "2.4G", "40M", "HT", "3T", "03", "28",
	"ETSI", "2.4G", "40M", "HT", "3T", "03", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "03", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "04", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "04", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "04", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "05", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "05", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "05", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "06", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "06", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "06", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "07", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "07", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "07", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "08", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "08", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "08", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "09", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "09", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "09", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "10", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "10", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "10", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "11", "28",
	"ETSI", "2.4G", "40M", "HT", "3T", "11", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "11", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "12", "63",
	"ETSI", "2.4G", "40M", "HT", "3T", "12", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "12", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "13", "63",
	"ETSI", "2.4G", "40M", "HT", "3T", "13", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "13", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "14", "63",
	"ETSI", "2.4G", "40M", "HT", "3T", "14", "63",
	"MKK", "2.4G", "40M", "HT", "3T", "14", "63",
	"FCC", "2.4G", "40M", "HT", "4T", "01", "63",
	"ETSI", "2.4G", "40M", "HT", "4T", "01", "63",
	"MKK", "2.4G", "40M", "HT", "4T", "01", "63",
	"FCC", "2.4G", "40M", "HT", "4T", "02", "63",
	"ETSI", "2.4G", "40M", "HT", "4T", "02", "63",
	"MKK", "2.4G", "40M", "HT", "4T", "02", "63",
	"FCC", "2.4G", "40M", "HT", "4T", "03", "26",
	"ETSI", "2.4G", "40M", "HT", "4T", "03", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "03", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "04", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "04", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "04", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "05", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "05", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "05", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "06", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "06", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "06", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "07", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "07", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "07", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "08", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "08", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "08", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "09", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "09", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "09", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "10", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "10", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "10", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "11", "26",
	"ETSI", "2.4G", "40M", "HT", "4T", "11", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "11", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "12", "63",
	"ETSI", "2.4G", "40M", "HT", "4T", "12", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "12", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "13", "63",
	"ETSI", "2.4G", "40M", "HT", "4T", "13", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "13", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "14", "63",
	"ETSI", "2.4G", "40M", "HT", "4T", "14", "63",
	"MKK", "2.4G", "40M", "HT", "4T", "14", "63"
};
#endif

void
odm_read_and_config_mp_8197g_txpwr_lmt_8197gmp_type2(struct dm_struct *dm)
{
#ifdef CONFIG_8197G_8197GMP_TYPE2

	u32	i = 0;
#if (DM_ODM_SUPPORT_TYPE == ODM_IOT)
	u32	array_len =
			sizeof(array_mp_8197g_txpwr_lmt_8197gmp_type2) / sizeof(u8);
	u8	*array = (u8 *)array_mp_8197g_txpwr_lmt_8197gmp_type2;
#else
	u32	array_len =
			sizeof(array_mp_8197g_txpwr_lmt_8197gmp_type2) / sizeof(u8 *);
	u8	**array = (u8 **)array_mp_8197g_txpwr_lmt_8197gmp_type2;
#endif

#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	void	*adapter = dm->adapter;
	HAL_DATA_TYPE	*hal_data = GET_HAL_DATA(((PADAPTER)adapter));

	odm_memory_set(dm, hal_data->BufOfLinesPwrLmt, 0,
		       MAX_LINES_HWCONFIG_TXT *
		       MAX_BYTES_LINE_HWCONFIG_TXT);
	hal_data->nLinesReadPwrLmt = array_len / 7;
#endif

	PHYDM_DBG(dm, ODM_COMP_INIT, "===> %s\n", __func__);

	for (i = 0; i < array_len; i += 7) {
#if (DM_ODM_SUPPORT_TYPE == ODM_IOT)
		u8	regulation = array[i];
		u8	band = array[i + 1];
		u8	bandwidth = array[i + 2];
		u8	rate = array[i + 3];
		u8	rf_path = array[i + 4];
		u8	chnl = array[i + 5];
		u8	val = array[i + 6];
#else
		u8	*regulation = array[i];
		u8	*band = array[i + 1];
		u8	*bandwidth = array[i + 2];
		u8	*rate = array[i + 3];
		u8	*rf_path = array[i + 4];
		u8	*chnl = array[i + 5];
		u8	*val = array[i + 6];
#endif

		odm_config_bb_txpwr_lmt_8197g(dm, regulation, band, bandwidth,
					      rate, rf_path, chnl, val);
#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
		rsprintf((char *)hal_data->BufOfLinesPwrLmt[i / 7], 100, "\"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\",",
			 regulation, band, bandwidth, rate, rf_path, chnl, val);
#endif
	}

#endif
}

/******************************************************************************
 *                           txpwr_lmt_8197gmp_type3.TXT
 ******************************************************************************/

#ifdef CONFIG_8197G_8197GMP_TYPE3
const char *array_mp_8197g_txpwr_lmt_8197gmp_type3[] = {
	"FCC", "2.4G", "20M", "CCK", "1T", "01", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "01", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "01", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "02", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "02", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "02", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "03", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "03", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "03", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "04", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "04", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "04", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "05", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "05", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "05", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "06", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "06", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "06", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "07", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "07", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "07", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "08", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "08", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "08", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "09", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "09", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "09", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "10", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "10", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "10", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "11", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "11", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "11", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "12", "63",
	"ETSI", "2.4G", "20M", "CCK", "1T", "12", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "12", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "13", "63",
	"ETSI", "2.4G", "20M", "CCK", "1T", "13", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "13", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "14", "63",
	"ETSI", "2.4G", "20M", "CCK", "1T", "14", "63",
	"MKK", "2.4G", "20M", "CCK", "1T", "14", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "01", "34",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "01", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "01", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "02", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "02", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "02", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "03", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "03", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "03", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "04", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "04", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "04", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "05", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "05", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "05", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "06", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "06", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "06", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "07", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "07", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "07", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "08", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "08", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "08", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "09", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "09", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "09", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "10", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "10", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "10", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "11", "32",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "11", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "11", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "12", "63",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "12", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "12", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "13", "63",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "13", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "13", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "14", "63",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "14", "63",
	"MKK", "2.4G", "20M", "OFDM", "1T", "14", "63",
	"FCC", "2.4G", "20M", "HT", "1T", "01", "34",
	"ETSI", "2.4G", "20M", "HT", "1T", "01", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "01", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "02", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "02", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "02", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "03", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "03", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "03", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "04", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "04", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "04", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "05", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "05", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "05", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "06", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "06", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "06", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "07", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "07", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "07", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "08", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "08", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "08", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "09", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "09", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "09", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "10", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "10", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "10", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "11", "32",
	"ETSI", "2.4G", "20M", "HT", "1T", "11", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "11", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "12", "63",
	"ETSI", "2.4G", "20M", "HT", "1T", "12", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "12", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "13", "63",
	"ETSI", "2.4G", "20M", "HT", "1T", "13", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "13", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "14", "63",
	"ETSI", "2.4G", "20M", "HT", "1T", "14", "63",
	"MKK", "2.4G", "20M", "HT", "1T", "14", "63",
	"FCC", "2.4G", "20M", "HT", "2T", "01", "32",
	"ETSI", "2.4G", "20M", "HT", "2T", "01", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "01", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "02", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "02", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "02", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "03", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "03", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "03", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "04", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "04", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "04", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "05", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "05", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "05", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "06", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "06", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "06", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "07", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "07", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "07", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "08", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "08", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "08", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "09", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "09", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "09", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "10", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "10", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "10", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "11", "30",
	"ETSI", "2.4G", "20M", "HT", "2T", "11", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "11", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "12", "63",
	"ETSI", "2.4G", "20M", "HT", "2T", "12", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "12", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "13", "63",
	"ETSI", "2.4G", "20M", "HT", "2T", "13", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "13", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "14", "63",
	"ETSI", "2.4G", "20M", "HT", "2T", "14", "63",
	"MKK", "2.4G", "20M", "HT", "2T", "14", "63",
	"FCC", "2.4G", "20M", "HT", "3T", "01", "30",
	"ETSI", "2.4G", "20M", "HT", "3T", "01", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "01", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "02", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "02", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "02", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "03", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "03", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "03", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "04", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "04", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "04", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "05", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "05", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "05", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "06", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "06", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "06", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "07", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "07", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "07", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "08", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "08", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "08", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "09", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "09", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "09", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "10", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "10", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "10", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "11", "28",
	"ETSI", "2.4G", "20M", "HT", "3T", "11", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "11", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "12", "63",
	"ETSI", "2.4G", "20M", "HT", "3T", "12", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "12", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "13", "63",
	"ETSI", "2.4G", "20M", "HT", "3T", "13", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "13", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "14", "63",
	"ETSI", "2.4G", "20M", "HT", "3T", "14", "63",
	"MKK", "2.4G", "20M", "HT", "3T", "14", "63",
	"FCC", "2.4G", "20M", "HT", "4T", "01", "28",
	"ETSI", "2.4G", "20M", "HT", "4T", "01", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "01", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "02", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "02", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "02", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "03", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "03", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "03", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "04", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "04", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "04", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "05", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "05", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "05", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "06", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "06", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "06", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "07", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "07", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "07", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "08", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "08", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "08", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "09", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "09", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "09", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "10", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "10", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "10", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "11", "26",
	"ETSI", "2.4G", "20M", "HT", "4T", "11", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "11", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "12", "63",
	"ETSI", "2.4G", "20M", "HT", "4T", "12", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "12", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "13", "63",
	"ETSI", "2.4G", "20M", "HT", "4T", "13", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "13", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "14", "63",
	"ETSI", "2.4G", "20M", "HT", "4T", "14", "63",
	"MKK", "2.4G", "20M", "HT", "4T", "14", "63",
	"FCC", "2.4G", "40M", "HT", "1T", "01", "63",
	"ETSI", "2.4G", "40M", "HT", "1T", "01", "63",
	"MKK", "2.4G", "40M", "HT", "1T", "01", "63",
	"FCC", "2.4G", "40M", "HT", "1T", "02", "63",
	"ETSI", "2.4G", "40M", "HT", "1T", "02", "63",
	"MKK", "2.4G", "40M", "HT", "1T", "02", "63",
	"FCC", "2.4G", "40M", "HT", "1T", "03", "32",
	"ETSI", "2.4G", "40M", "HT", "1T", "03", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "03", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "04", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "04", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "04", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "05", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "05", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "05", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "06", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "06", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "06", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "07", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "07", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "07", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "08", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "08", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "08", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "09", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "09", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "09", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "10", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "10", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "10", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "11", "32",
	"ETSI", "2.4G", "40M", "HT", "1T", "11", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "11", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "12", "63",
	"ETSI", "2.4G", "40M", "HT", "1T", "12", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "12", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "13", "63",
	"ETSI", "2.4G", "40M", "HT", "1T", "13", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "13", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "14", "63",
	"ETSI", "2.4G", "40M", "HT", "1T", "14", "63",
	"MKK", "2.4G", "40M", "HT", "1T", "14", "63",
	"FCC", "2.4G", "40M", "HT", "2T", "01", "63",
	"ETSI", "2.4G", "40M", "HT", "2T", "01", "63",
	"MKK", "2.4G", "40M", "HT", "2T", "01", "63",
	"FCC", "2.4G", "40M", "HT", "2T", "02", "63",
	"ETSI", "2.4G", "40M", "HT", "2T", "02", "63",
	"MKK", "2.4G", "40M", "HT", "2T", "02", "63",
	"FCC", "2.4G", "40M", "HT", "2T", "03", "30",
	"ETSI", "2.4G", "40M", "HT", "2T", "03", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "03", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "04", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "04", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "04", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "05", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "05", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "05", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "06", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "06", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "06", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "07", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "07", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "07", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "08", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "08", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "08", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "09", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "09", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "09", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "10", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "10", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "10", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "11", "30",
	"ETSI", "2.4G", "40M", "HT", "2T", "11", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "11", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "12", "63",
	"ETSI", "2.4G", "40M", "HT", "2T", "12", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "12", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "13", "63",
	"ETSI", "2.4G", "40M", "HT", "2T", "13", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "13", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "14", "63",
	"ETSI", "2.4G", "40M", "HT", "2T", "14", "63",
	"MKK", "2.4G", "40M", "HT", "2T", "14", "63",
	"FCC", "2.4G", "40M", "HT", "3T", "01", "63",
	"ETSI", "2.4G", "40M", "HT", "3T", "01", "63",
	"MKK", "2.4G", "40M", "HT", "3T", "01", "63",
	"FCC", "2.4G", "40M", "HT", "3T", "02", "63",
	"ETSI", "2.4G", "40M", "HT", "3T", "02", "63",
	"MKK", "2.4G", "40M", "HT", "3T", "02", "63",
	"FCC", "2.4G", "40M", "HT", "3T", "03", "28",
	"ETSI", "2.4G", "40M", "HT", "3T", "03", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "03", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "04", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "04", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "04", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "05", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "05", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "05", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "06", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "06", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "06", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "07", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "07", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "07", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "08", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "08", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "08", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "09", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "09", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "09", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "10", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "10", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "10", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "11", "28",
	"ETSI", "2.4G", "40M", "HT", "3T", "11", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "11", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "12", "63",
	"ETSI", "2.4G", "40M", "HT", "3T", "12", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "12", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "13", "63",
	"ETSI", "2.4G", "40M", "HT", "3T", "13", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "13", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "14", "63",
	"ETSI", "2.4G", "40M", "HT", "3T", "14", "63",
	"MKK", "2.4G", "40M", "HT", "3T", "14", "63",
	"FCC", "2.4G", "40M", "HT", "4T", "01", "63",
	"ETSI", "2.4G", "40M", "HT", "4T", "01", "63",
	"MKK", "2.4G", "40M", "HT", "4T", "01", "63",
	"FCC", "2.4G", "40M", "HT", "4T", "02", "63",
	"ETSI", "2.4G", "40M", "HT", "4T", "02", "63",
	"MKK", "2.4G", "40M", "HT", "4T", "02", "63",
	"FCC", "2.4G", "40M", "HT", "4T", "03", "26",
	"ETSI", "2.4G", "40M", "HT", "4T", "03", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "03", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "04", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "04", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "04", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "05", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "05", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "05", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "06", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "06", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "06", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "07", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "07", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "07", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "08", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "08", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "08", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "09", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "09", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "09", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "10", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "10", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "10", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "11", "26",
	"ETSI", "2.4G", "40M", "HT", "4T", "11", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "11", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "12", "63",
	"ETSI", "2.4G", "40M", "HT", "4T", "12", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "12", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "13", "63",
	"ETSI", "2.4G", "40M", "HT", "4T", "13", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "13", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "14", "63",
	"ETSI", "2.4G", "40M", "HT", "4T", "14", "63",
	"MKK", "2.4G", "40M", "HT", "4T", "14", "63"
};
#endif

void
odm_read_and_config_mp_8197g_txpwr_lmt_8197gmp_type3(struct dm_struct *dm)
{
#ifdef CONFIG_8197G_8197GMP_TYPE3

	u32	i = 0;
#if (DM_ODM_SUPPORT_TYPE == ODM_IOT)
	u32	array_len =
			sizeof(array_mp_8197g_txpwr_lmt_8197gmp_type3) / sizeof(u8);
	u8	*array = (u8 *)array_mp_8197g_txpwr_lmt_8197gmp_type3;
#else
	u32	array_len =
			sizeof(array_mp_8197g_txpwr_lmt_8197gmp_type3) / sizeof(u8 *);
	u8	**array = (u8 **)array_mp_8197g_txpwr_lmt_8197gmp_type3;
#endif

#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	void	*adapter = dm->adapter;
	HAL_DATA_TYPE	*hal_data = GET_HAL_DATA(((PADAPTER)adapter));

	odm_memory_set(dm, hal_data->BufOfLinesPwrLmt, 0,
		       MAX_LINES_HWCONFIG_TXT *
		       MAX_BYTES_LINE_HWCONFIG_TXT);
	hal_data->nLinesReadPwrLmt = array_len / 7;
#endif

	PHYDM_DBG(dm, ODM_COMP_INIT, "===> %s\n", __func__);

	for (i = 0; i < array_len; i += 7) {
#if (DM_ODM_SUPPORT_TYPE == ODM_IOT)
		u8	regulation = array[i];
		u8	band = array[i + 1];
		u8	bandwidth = array[i + 2];
		u8	rate = array[i + 3];
		u8	rf_path = array[i + 4];
		u8	chnl = array[i + 5];
		u8	val = array[i + 6];
#else
		u8	*regulation = array[i];
		u8	*band = array[i + 1];
		u8	*bandwidth = array[i + 2];
		u8	*rate = array[i + 3];
		u8	*rf_path = array[i + 4];
		u8	*chnl = array[i + 5];
		u8	*val = array[i + 6];
#endif

		odm_config_bb_txpwr_lmt_8197g(dm, regulation, band, bandwidth,
					      rate, rf_path, chnl, val);
#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
		rsprintf((char *)hal_data->BufOfLinesPwrLmt[i / 7], 100, "\"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\",",
			 regulation, band, bandwidth, rate, rf_path, chnl, val);
#endif
	}

#endif
}

/******************************************************************************
 *                           txpwr_lmt_8197gmp_type4.TXT
 ******************************************************************************/

#ifdef CONFIG_8197G_8197GMP_TYPE4
const char *array_mp_8197g_txpwr_lmt_8197gmp_type4[] = {
	"FCC", "2.4G", "20M", "CCK", "1T", "01", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "01", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "01", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "02", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "02", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "02", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "03", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "03", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "03", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "04", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "04", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "04", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "05", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "05", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "05", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "06", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "06", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "06", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "07", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "07", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "07", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "08", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "08", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "08", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "09", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "09", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "09", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "10", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "10", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "10", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "11", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "11", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "11", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "12", "63",
	"ETSI", "2.4G", "20M", "CCK", "1T", "12", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "12", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "13", "63",
	"ETSI", "2.4G", "20M", "CCK", "1T", "13", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "13", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "14", "63",
	"ETSI", "2.4G", "20M", "CCK", "1T", "14", "63",
	"MKK", "2.4G", "20M", "CCK", "1T", "14", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "01", "34",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "01", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "01", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "02", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "02", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "02", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "03", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "03", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "03", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "04", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "04", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "04", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "05", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "05", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "05", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "06", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "06", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "06", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "07", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "07", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "07", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "08", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "08", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "08", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "09", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "09", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "09", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "10", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "10", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "10", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "11", "32",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "11", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "11", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "12", "63",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "12", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "12", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "13", "63",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "13", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "13", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "14", "63",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "14", "63",
	"MKK", "2.4G", "20M", "OFDM", "1T", "14", "63",
	"FCC", "2.4G", "20M", "HT", "1T", "01", "34",
	"ETSI", "2.4G", "20M", "HT", "1T", "01", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "01", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "02", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "02", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "02", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "03", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "03", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "03", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "04", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "04", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "04", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "05", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "05", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "05", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "06", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "06", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "06", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "07", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "07", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "07", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "08", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "08", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "08", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "09", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "09", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "09", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "10", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "10", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "10", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "11", "32",
	"ETSI", "2.4G", "20M", "HT", "1T", "11", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "11", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "12", "63",
	"ETSI", "2.4G", "20M", "HT", "1T", "12", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "12", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "13", "63",
	"ETSI", "2.4G", "20M", "HT", "1T", "13", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "13", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "14", "63",
	"ETSI", "2.4G", "20M", "HT", "1T", "14", "63",
	"MKK", "2.4G", "20M", "HT", "1T", "14", "63",
	"FCC", "2.4G", "20M", "HT", "2T", "01", "32",
	"ETSI", "2.4G", "20M", "HT", "2T", "01", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "01", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "02", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "02", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "02", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "03", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "03", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "03", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "04", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "04", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "04", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "05", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "05", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "05", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "06", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "06", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "06", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "07", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "07", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "07", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "08", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "08", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "08", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "09", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "09", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "09", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "10", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "10", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "10", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "11", "30",
	"ETSI", "2.4G", "20M", "HT", "2T", "11", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "11", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "12", "63",
	"ETSI", "2.4G", "20M", "HT", "2T", "12", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "12", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "13", "63",
	"ETSI", "2.4G", "20M", "HT", "2T", "13", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "13", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "14", "63",
	"ETSI", "2.4G", "20M", "HT", "2T", "14", "63",
	"MKK", "2.4G", "20M", "HT", "2T", "14", "63",
	"FCC", "2.4G", "20M", "HT", "3T", "01", "30",
	"ETSI", "2.4G", "20M", "HT", "3T", "01", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "01", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "02", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "02", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "02", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "03", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "03", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "03", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "04", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "04", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "04", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "05", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "05", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "05", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "06", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "06", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "06", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "07", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "07", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "07", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "08", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "08", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "08", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "09", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "09", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "09", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "10", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "10", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "10", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "11", "28",
	"ETSI", "2.4G", "20M", "HT", "3T", "11", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "11", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "12", "63",
	"ETSI", "2.4G", "20M", "HT", "3T", "12", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "12", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "13", "63",
	"ETSI", "2.4G", "20M", "HT", "3T", "13", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "13", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "14", "63",
	"ETSI", "2.4G", "20M", "HT", "3T", "14", "63",
	"MKK", "2.4G", "20M", "HT", "3T", "14", "63",
	"FCC", "2.4G", "20M", "HT", "4T", "01", "28",
	"ETSI", "2.4G", "20M", "HT", "4T", "01", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "01", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "02", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "02", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "02", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "03", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "03", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "03", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "04", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "04", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "04", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "05", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "05", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "05", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "06", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "06", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "06", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "07", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "07", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "07", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "08", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "08", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "08", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "09", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "09", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "09", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "10", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "10", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "10", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "11", "26",
	"ETSI", "2.4G", "20M", "HT", "4T", "11", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "11", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "12", "63",
	"ETSI", "2.4G", "20M", "HT", "4T", "12", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "12", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "13", "63",
	"ETSI", "2.4G", "20M", "HT", "4T", "13", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "13", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "14", "63",
	"ETSI", "2.4G", "20M", "HT", "4T", "14", "63",
	"MKK", "2.4G", "20M", "HT", "4T", "14", "63",
	"FCC", "2.4G", "40M", "HT", "1T", "01", "63",
	"ETSI", "2.4G", "40M", "HT", "1T", "01", "63",
	"MKK", "2.4G", "40M", "HT", "1T", "01", "63",
	"FCC", "2.4G", "40M", "HT", "1T", "02", "63",
	"ETSI", "2.4G", "40M", "HT", "1T", "02", "63",
	"MKK", "2.4G", "40M", "HT", "1T", "02", "63",
	"FCC", "2.4G", "40M", "HT", "1T", "03", "32",
	"ETSI", "2.4G", "40M", "HT", "1T", "03", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "03", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "04", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "04", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "04", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "05", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "05", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "05", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "06", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "06", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "06", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "07", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "07", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "07", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "08", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "08", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "08", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "09", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "09", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "09", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "10", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "10", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "10", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "11", "32",
	"ETSI", "2.4G", "40M", "HT", "1T", "11", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "11", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "12", "63",
	"ETSI", "2.4G", "40M", "HT", "1T", "12", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "12", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "13", "63",
	"ETSI", "2.4G", "40M", "HT", "1T", "13", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "13", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "14", "63",
	"ETSI", "2.4G", "40M", "HT", "1T", "14", "63",
	"MKK", "2.4G", "40M", "HT", "1T", "14", "63",
	"FCC", "2.4G", "40M", "HT", "2T", "01", "63",
	"ETSI", "2.4G", "40M", "HT", "2T", "01", "63",
	"MKK", "2.4G", "40M", "HT", "2T", "01", "63",
	"FCC", "2.4G", "40M", "HT", "2T", "02", "63",
	"ETSI", "2.4G", "40M", "HT", "2T", "02", "63",
	"MKK", "2.4G", "40M", "HT", "2T", "02", "63",
	"FCC", "2.4G", "40M", "HT", "2T", "03", "30",
	"ETSI", "2.4G", "40M", "HT", "2T", "03", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "03", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "04", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "04", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "04", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "05", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "05", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "05", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "06", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "06", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "06", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "07", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "07", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "07", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "08", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "08", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "08", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "09", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "09", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "09", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "10", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "10", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "10", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "11", "30",
	"ETSI", "2.4G", "40M", "HT", "2T", "11", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "11", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "12", "63",
	"ETSI", "2.4G", "40M", "HT", "2T", "12", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "12", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "13", "63",
	"ETSI", "2.4G", "40M", "HT", "2T", "13", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "13", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "14", "63",
	"ETSI", "2.4G", "40M", "HT", "2T", "14", "63",
	"MKK", "2.4G", "40M", "HT", "2T", "14", "63",
	"FCC", "2.4G", "40M", "HT", "3T", "01", "63",
	"ETSI", "2.4G", "40M", "HT", "3T", "01", "63",
	"MKK", "2.4G", "40M", "HT", "3T", "01", "63",
	"FCC", "2.4G", "40M", "HT", "3T", "02", "63",
	"ETSI", "2.4G", "40M", "HT", "3T", "02", "63",
	"MKK", "2.4G", "40M", "HT", "3T", "02", "63",
	"FCC", "2.4G", "40M", "HT", "3T", "03", "28",
	"ETSI", "2.4G", "40M", "HT", "3T", "03", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "03", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "04", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "04", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "04", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "05", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "05", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "05", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "06", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "06", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "06", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "07", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "07", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "07", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "08", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "08", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "08", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "09", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "09", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "09", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "10", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "10", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "10", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "11", "28",
	"ETSI", "2.4G", "40M", "HT", "3T", "11", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "11", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "12", "63",
	"ETSI", "2.4G", "40M", "HT", "3T", "12", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "12", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "13", "63",
	"ETSI", "2.4G", "40M", "HT", "3T", "13", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "13", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "14", "63",
	"ETSI", "2.4G", "40M", "HT", "3T", "14", "63",
	"MKK", "2.4G", "40M", "HT", "3T", "14", "63",
	"FCC", "2.4G", "40M", "HT", "4T", "01", "63",
	"ETSI", "2.4G", "40M", "HT", "4T", "01", "63",
	"MKK", "2.4G", "40M", "HT", "4T", "01", "63",
	"FCC", "2.4G", "40M", "HT", "4T", "02", "63",
	"ETSI", "2.4G", "40M", "HT", "4T", "02", "63",
	"MKK", "2.4G", "40M", "HT", "4T", "02", "63",
	"FCC", "2.4G", "40M", "HT", "4T", "03", "26",
	"ETSI", "2.4G", "40M", "HT", "4T", "03", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "03", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "04", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "04", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "04", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "05", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "05", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "05", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "06", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "06", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "06", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "07", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "07", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "07", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "08", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "08", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "08", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "09", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "09", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "09", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "10", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "10", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "10", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "11", "26",
	"ETSI", "2.4G", "40M", "HT", "4T", "11", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "11", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "12", "63",
	"ETSI", "2.4G", "40M", "HT", "4T", "12", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "12", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "13", "63",
	"ETSI", "2.4G", "40M", "HT", "4T", "13", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "13", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "14", "63",
	"ETSI", "2.4G", "40M", "HT", "4T", "14", "63",
	"MKK", "2.4G", "40M", "HT", "4T", "14", "63"
};
#endif

void
odm_read_and_config_mp_8197g_txpwr_lmt_8197gmp_type4(struct dm_struct *dm)
{
#ifdef CONFIG_8197G_8197GMP_TYPE4

	u32	i = 0;
#if (DM_ODM_SUPPORT_TYPE == ODM_IOT)
	u32	array_len =
			sizeof(array_mp_8197g_txpwr_lmt_8197gmp_type4) / sizeof(u8);
	u8	*array = (u8 *)array_mp_8197g_txpwr_lmt_8197gmp_type4;
#else
	u32	array_len =
			sizeof(array_mp_8197g_txpwr_lmt_8197gmp_type4) / sizeof(u8 *);
	u8	**array = (u8 **)array_mp_8197g_txpwr_lmt_8197gmp_type4;
#endif

#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	void	*adapter = dm->adapter;
	HAL_DATA_TYPE	*hal_data = GET_HAL_DATA(((PADAPTER)adapter));

	odm_memory_set(dm, hal_data->BufOfLinesPwrLmt, 0,
		       MAX_LINES_HWCONFIG_TXT *
		       MAX_BYTES_LINE_HWCONFIG_TXT);
	hal_data->nLinesReadPwrLmt = array_len / 7;
#endif

	PHYDM_DBG(dm, ODM_COMP_INIT, "===> %s\n", __func__);

	for (i = 0; i < array_len; i += 7) {
#if (DM_ODM_SUPPORT_TYPE == ODM_IOT)
		u8	regulation = array[i];
		u8	band = array[i + 1];
		u8	bandwidth = array[i + 2];
		u8	rate = array[i + 3];
		u8	rf_path = array[i + 4];
		u8	chnl = array[i + 5];
		u8	val = array[i + 6];
#else
		u8	*regulation = array[i];
		u8	*band = array[i + 1];
		u8	*bandwidth = array[i + 2];
		u8	*rate = array[i + 3];
		u8	*rf_path = array[i + 4];
		u8	*chnl = array[i + 5];
		u8	*val = array[i + 6];
#endif

		odm_config_bb_txpwr_lmt_8197g(dm, regulation, band, bandwidth,
					      rate, rf_path, chnl, val);
#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
		rsprintf((char *)hal_data->BufOfLinesPwrLmt[i / 7], 100, "\"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\",",
			 regulation, band, bandwidth, rate, rf_path, chnl, val);
#endif
	}

#endif
}

/******************************************************************************
 *                           txpwr_lmt_8197gmp_type5.TXT
 ******************************************************************************/

#ifdef CONFIG_8197G_8197GMP_TYPE5
const char *array_mp_8197g_txpwr_lmt_8197gmp_type5[] = {
	"FCC", "2.4G", "20M", "CCK", "1T", "01", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "01", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "01", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "02", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "02", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "02", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "03", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "03", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "03", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "04", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "04", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "04", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "05", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "05", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "05", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "06", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "06", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "06", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "07", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "07", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "07", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "08", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "08", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "08", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "09", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "09", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "09", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "10", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "10", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "10", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "11", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "11", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "11", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "12", "63",
	"ETSI", "2.4G", "20M", "CCK", "1T", "12", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "12", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "13", "63",
	"ETSI", "2.4G", "20M", "CCK", "1T", "13", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "13", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "14", "63",
	"ETSI", "2.4G", "20M", "CCK", "1T", "14", "63",
	"MKK", "2.4G", "20M", "CCK", "1T", "14", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "01", "34",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "01", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "01", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "02", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "02", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "02", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "03", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "03", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "03", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "04", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "04", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "04", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "05", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "05", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "05", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "06", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "06", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "06", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "07", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "07", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "07", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "08", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "08", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "08", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "09", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "09", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "09", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "10", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "10", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "10", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "11", "32",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "11", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "11", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "12", "63",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "12", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "12", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "13", "63",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "13", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "13", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "14", "63",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "14", "63",
	"MKK", "2.4G", "20M", "OFDM", "1T", "14", "63",
	"FCC", "2.4G", "20M", "HT", "1T", "01", "34",
	"ETSI", "2.4G", "20M", "HT", "1T", "01", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "01", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "02", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "02", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "02", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "03", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "03", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "03", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "04", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "04", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "04", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "05", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "05", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "05", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "06", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "06", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "06", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "07", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "07", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "07", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "08", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "08", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "08", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "09", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "09", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "09", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "10", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "10", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "10", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "11", "32",
	"ETSI", "2.4G", "20M", "HT", "1T", "11", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "11", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "12", "63",
	"ETSI", "2.4G", "20M", "HT", "1T", "12", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "12", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "13", "63",
	"ETSI", "2.4G", "20M", "HT", "1T", "13", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "13", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "14", "63",
	"ETSI", "2.4G", "20M", "HT", "1T", "14", "63",
	"MKK", "2.4G", "20M", "HT", "1T", "14", "63",
	"FCC", "2.4G", "20M", "HT", "2T", "01", "32",
	"ETSI", "2.4G", "20M", "HT", "2T", "01", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "01", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "02", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "02", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "02", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "03", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "03", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "03", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "04", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "04", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "04", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "05", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "05", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "05", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "06", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "06", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "06", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "07", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "07", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "07", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "08", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "08", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "08", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "09", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "09", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "09", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "10", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "10", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "10", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "11", "30",
	"ETSI", "2.4G", "20M", "HT", "2T", "11", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "11", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "12", "63",
	"ETSI", "2.4G", "20M", "HT", "2T", "12", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "12", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "13", "63",
	"ETSI", "2.4G", "20M", "HT", "2T", "13", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "13", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "14", "63",
	"ETSI", "2.4G", "20M", "HT", "2T", "14", "63",
	"MKK", "2.4G", "20M", "HT", "2T", "14", "63",
	"FCC", "2.4G", "20M", "HT", "3T", "01", "30",
	"ETSI", "2.4G", "20M", "HT", "3T", "01", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "01", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "02", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "02", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "02", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "03", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "03", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "03", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "04", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "04", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "04", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "05", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "05", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "05", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "06", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "06", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "06", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "07", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "07", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "07", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "08", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "08", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "08", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "09", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "09", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "09", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "10", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "10", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "10", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "11", "28",
	"ETSI", "2.4G", "20M", "HT", "3T", "11", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "11", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "12", "63",
	"ETSI", "2.4G", "20M", "HT", "3T", "12", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "12", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "13", "63",
	"ETSI", "2.4G", "20M", "HT", "3T", "13", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "13", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "14", "63",
	"ETSI", "2.4G", "20M", "HT", "3T", "14", "63",
	"MKK", "2.4G", "20M", "HT", "3T", "14", "63",
	"FCC", "2.4G", "20M", "HT", "4T", "01", "28",
	"ETSI", "2.4G", "20M", "HT", "4T", "01", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "01", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "02", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "02", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "02", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "03", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "03", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "03", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "04", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "04", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "04", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "05", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "05", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "05", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "06", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "06", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "06", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "07", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "07", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "07", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "08", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "08", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "08", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "09", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "09", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "09", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "10", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "10", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "10", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "11", "26",
	"ETSI", "2.4G", "20M", "HT", "4T", "11", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "11", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "12", "63",
	"ETSI", "2.4G", "20M", "HT", "4T", "12", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "12", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "13", "63",
	"ETSI", "2.4G", "20M", "HT", "4T", "13", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "13", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "14", "63",
	"ETSI", "2.4G", "20M", "HT", "4T", "14", "63",
	"MKK", "2.4G", "20M", "HT", "4T", "14", "63",
	"FCC", "2.4G", "40M", "HT", "1T", "01", "63",
	"ETSI", "2.4G", "40M", "HT", "1T", "01", "63",
	"MKK", "2.4G", "40M", "HT", "1T", "01", "63",
	"FCC", "2.4G", "40M", "HT", "1T", "02", "63",
	"ETSI", "2.4G", "40M", "HT", "1T", "02", "63",
	"MKK", "2.4G", "40M", "HT", "1T", "02", "63",
	"FCC", "2.4G", "40M", "HT", "1T", "03", "32",
	"ETSI", "2.4G", "40M", "HT", "1T", "03", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "03", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "04", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "04", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "04", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "05", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "05", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "05", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "06", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "06", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "06", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "07", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "07", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "07", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "08", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "08", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "08", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "09", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "09", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "09", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "10", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "10", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "10", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "11", "32",
	"ETSI", "2.4G", "40M", "HT", "1T", "11", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "11", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "12", "63",
	"ETSI", "2.4G", "40M", "HT", "1T", "12", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "12", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "13", "63",
	"ETSI", "2.4G", "40M", "HT", "1T", "13", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "13", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "14", "63",
	"ETSI", "2.4G", "40M", "HT", "1T", "14", "63",
	"MKK", "2.4G", "40M", "HT", "1T", "14", "63",
	"FCC", "2.4G", "40M", "HT", "2T", "01", "63",
	"ETSI", "2.4G", "40M", "HT", "2T", "01", "63",
	"MKK", "2.4G", "40M", "HT", "2T", "01", "63",
	"FCC", "2.4G", "40M", "HT", "2T", "02", "63",
	"ETSI", "2.4G", "40M", "HT", "2T", "02", "63",
	"MKK", "2.4G", "40M", "HT", "2T", "02", "63",
	"FCC", "2.4G", "40M", "HT", "2T", "03", "30",
	"ETSI", "2.4G", "40M", "HT", "2T", "03", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "03", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "04", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "04", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "04", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "05", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "05", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "05", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "06", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "06", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "06", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "07", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "07", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "07", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "08", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "08", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "08", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "09", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "09", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "09", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "10", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "10", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "10", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "11", "30",
	"ETSI", "2.4G", "40M", "HT", "2T", "11", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "11", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "12", "63",
	"ETSI", "2.4G", "40M", "HT", "2T", "12", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "12", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "13", "63",
	"ETSI", "2.4G", "40M", "HT", "2T", "13", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "13", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "14", "63",
	"ETSI", "2.4G", "40M", "HT", "2T", "14", "63",
	"MKK", "2.4G", "40M", "HT", "2T", "14", "63",
	"FCC", "2.4G", "40M", "HT", "3T", "01", "63",
	"ETSI", "2.4G", "40M", "HT", "3T", "01", "63",
	"MKK", "2.4G", "40M", "HT", "3T", "01", "63",
	"FCC", "2.4G", "40M", "HT", "3T", "02", "63",
	"ETSI", "2.4G", "40M", "HT", "3T", "02", "63",
	"MKK", "2.4G", "40M", "HT", "3T", "02", "63",
	"FCC", "2.4G", "40M", "HT", "3T", "03", "28",
	"ETSI", "2.4G", "40M", "HT", "3T", "03", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "03", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "04", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "04", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "04", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "05", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "05", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "05", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "06", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "06", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "06", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "07", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "07", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "07", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "08", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "08", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "08", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "09", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "09", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "09", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "10", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "10", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "10", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "11", "28",
	"ETSI", "2.4G", "40M", "HT", "3T", "11", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "11", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "12", "63",
	"ETSI", "2.4G", "40M", "HT", "3T", "12", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "12", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "13", "63",
	"ETSI", "2.4G", "40M", "HT", "3T", "13", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "13", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "14", "63",
	"ETSI", "2.4G", "40M", "HT", "3T", "14", "63",
	"MKK", "2.4G", "40M", "HT", "3T", "14", "63",
	"FCC", "2.4G", "40M", "HT", "4T", "01", "63",
	"ETSI", "2.4G", "40M", "HT", "4T", "01", "63",
	"MKK", "2.4G", "40M", "HT", "4T", "01", "63",
	"FCC", "2.4G", "40M", "HT", "4T", "02", "63",
	"ETSI", "2.4G", "40M", "HT", "4T", "02", "63",
	"MKK", "2.4G", "40M", "HT", "4T", "02", "63",
	"FCC", "2.4G", "40M", "HT", "4T", "03", "26",
	"ETSI", "2.4G", "40M", "HT", "4T", "03", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "03", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "04", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "04", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "04", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "05", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "05", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "05", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "06", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "06", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "06", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "07", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "07", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "07", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "08", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "08", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "08", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "09", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "09", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "09", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "10", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "10", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "10", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "11", "26",
	"ETSI", "2.4G", "40M", "HT", "4T", "11", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "11", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "12", "63",
	"ETSI", "2.4G", "40M", "HT", "4T", "12", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "12", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "13", "63",
	"ETSI", "2.4G", "40M", "HT", "4T", "13", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "13", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "14", "63",
	"ETSI", "2.4G", "40M", "HT", "4T", "14", "63",
	"MKK", "2.4G", "40M", "HT", "4T", "14", "63"
};
#endif

void
odm_read_and_config_mp_8197g_txpwr_lmt_8197gmp_type5(struct dm_struct *dm)
{
#ifdef CONFIG_8197G_8197GMP_TYPE5

	u32	i = 0;
#if (DM_ODM_SUPPORT_TYPE == ODM_IOT)
	u32	array_len =
			sizeof(array_mp_8197g_txpwr_lmt_8197gmp_type5) / sizeof(u8);
	u8	*array = (u8 *)array_mp_8197g_txpwr_lmt_8197gmp_type5;
#else
	u32	array_len =
			sizeof(array_mp_8197g_txpwr_lmt_8197gmp_type5) / sizeof(u8 *);
	u8	**array = (u8 **)array_mp_8197g_txpwr_lmt_8197gmp_type5;
#endif

#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	void	*adapter = dm->adapter;
	HAL_DATA_TYPE	*hal_data = GET_HAL_DATA(((PADAPTER)adapter));

	odm_memory_set(dm, hal_data->BufOfLinesPwrLmt, 0,
		       MAX_LINES_HWCONFIG_TXT *
		       MAX_BYTES_LINE_HWCONFIG_TXT);
	hal_data->nLinesReadPwrLmt = array_len / 7;
#endif

	PHYDM_DBG(dm, ODM_COMP_INIT, "===> %s\n", __func__);

	for (i = 0; i < array_len; i += 7) {
#if (DM_ODM_SUPPORT_TYPE == ODM_IOT)
		u8	regulation = array[i];
		u8	band = array[i + 1];
		u8	bandwidth = array[i + 2];
		u8	rate = array[i + 3];
		u8	rf_path = array[i + 4];
		u8	chnl = array[i + 5];
		u8	val = array[i + 6];
#else
		u8	*regulation = array[i];
		u8	*band = array[i + 1];
		u8	*bandwidth = array[i + 2];
		u8	*rate = array[i + 3];
		u8	*rf_path = array[i + 4];
		u8	*chnl = array[i + 5];
		u8	*val = array[i + 6];
#endif

		odm_config_bb_txpwr_lmt_8197g(dm, regulation, band, bandwidth,
					      rate, rf_path, chnl, val);
#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
		rsprintf((char *)hal_data->BufOfLinesPwrLmt[i / 7], 100, "\"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\",",
			 regulation, band, bandwidth, rate, rf_path, chnl, val);
#endif
	}

#endif
}

/******************************************************************************
 *                           txpwr_lmt_8197gmp_type6.TXT
 ******************************************************************************/

#ifdef CONFIG_8197G_8197GMP_TYPE6
const char *array_mp_8197g_txpwr_lmt_8197gmp_type6[] = {
	"FCC", "2.4G", "20M", "CCK", "1T", "01", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "01", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "01", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "02", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "02", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "02", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "03", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "03", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "03", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "04", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "04", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "04", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "05", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "05", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "05", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "06", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "06", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "06", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "07", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "07", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "07", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "08", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "08", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "08", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "09", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "09", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "09", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "10", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "10", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "10", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "11", "36",
	"ETSI", "2.4G", "20M", "CCK", "1T", "11", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "11", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "12", "63",
	"ETSI", "2.4G", "20M", "CCK", "1T", "12", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "12", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "13", "63",
	"ETSI", "2.4G", "20M", "CCK", "1T", "13", "32",
	"MKK", "2.4G", "20M", "CCK", "1T", "13", "32",
	"FCC", "2.4G", "20M", "CCK", "1T", "14", "63",
	"ETSI", "2.4G", "20M", "CCK", "1T", "14", "63",
	"MKK", "2.4G", "20M", "CCK", "1T", "14", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "01", "34",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "01", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "01", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "02", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "02", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "02", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "03", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "03", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "03", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "04", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "04", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "04", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "05", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "05", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "05", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "06", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "06", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "06", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "07", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "07", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "07", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "08", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "08", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "08", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "09", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "09", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "09", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "10", "36",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "10", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "10", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "11", "32",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "11", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "11", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "12", "63",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "12", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "12", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "13", "63",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "13", "32",
	"MKK", "2.4G", "20M", "OFDM", "1T", "13", "32",
	"FCC", "2.4G", "20M", "OFDM", "1T", "14", "63",
	"ETSI", "2.4G", "20M", "OFDM", "1T", "14", "63",
	"MKK", "2.4G", "20M", "OFDM", "1T", "14", "63",
	"FCC", "2.4G", "20M", "HT", "1T", "01", "34",
	"ETSI", "2.4G", "20M", "HT", "1T", "01", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "01", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "02", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "02", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "02", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "03", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "03", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "03", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "04", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "04", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "04", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "05", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "05", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "05", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "06", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "06", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "06", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "07", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "07", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "07", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "08", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "08", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "08", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "09", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "09", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "09", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "10", "36",
	"ETSI", "2.4G", "20M", "HT", "1T", "10", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "10", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "11", "32",
	"ETSI", "2.4G", "20M", "HT", "1T", "11", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "11", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "12", "63",
	"ETSI", "2.4G", "20M", "HT", "1T", "12", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "12", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "13", "63",
	"ETSI", "2.4G", "20M", "HT", "1T", "13", "32",
	"MKK", "2.4G", "20M", "HT", "1T", "13", "32",
	"FCC", "2.4G", "20M", "HT", "1T", "14", "63",
	"ETSI", "2.4G", "20M", "HT", "1T", "14", "63",
	"MKK", "2.4G", "20M", "HT", "1T", "14", "63",
	"FCC", "2.4G", "20M", "HT", "2T", "01", "32",
	"ETSI", "2.4G", "20M", "HT", "2T", "01", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "01", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "02", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "02", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "02", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "03", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "03", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "03", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "04", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "04", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "04", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "05", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "05", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "05", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "06", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "06", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "06", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "07", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "07", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "07", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "08", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "08", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "08", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "09", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "09", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "09", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "10", "34",
	"ETSI", "2.4G", "20M", "HT", "2T", "10", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "10", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "11", "30",
	"ETSI", "2.4G", "20M", "HT", "2T", "11", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "11", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "12", "63",
	"ETSI", "2.4G", "20M", "HT", "2T", "12", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "12", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "13", "63",
	"ETSI", "2.4G", "20M", "HT", "2T", "13", "30",
	"MKK", "2.4G", "20M", "HT", "2T", "13", "30",
	"FCC", "2.4G", "20M", "HT", "2T", "14", "63",
	"ETSI", "2.4G", "20M", "HT", "2T", "14", "63",
	"MKK", "2.4G", "20M", "HT", "2T", "14", "63",
	"FCC", "2.4G", "20M", "HT", "3T", "01", "30",
	"ETSI", "2.4G", "20M", "HT", "3T", "01", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "01", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "02", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "02", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "02", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "03", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "03", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "03", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "04", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "04", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "04", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "05", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "05", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "05", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "06", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "06", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "06", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "07", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "07", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "07", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "08", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "08", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "08", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "09", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "09", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "09", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "10", "32",
	"ETSI", "2.4G", "20M", "HT", "3T", "10", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "10", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "11", "28",
	"ETSI", "2.4G", "20M", "HT", "3T", "11", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "11", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "12", "63",
	"ETSI", "2.4G", "20M", "HT", "3T", "12", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "12", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "13", "63",
	"ETSI", "2.4G", "20M", "HT", "3T", "13", "28",
	"MKK", "2.4G", "20M", "HT", "3T", "13", "28",
	"FCC", "2.4G", "20M", "HT", "3T", "14", "63",
	"ETSI", "2.4G", "20M", "HT", "3T", "14", "63",
	"MKK", "2.4G", "20M", "HT", "3T", "14", "63",
	"FCC", "2.4G", "20M", "HT", "4T", "01", "28",
	"ETSI", "2.4G", "20M", "HT", "4T", "01", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "01", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "02", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "02", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "02", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "03", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "03", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "03", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "04", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "04", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "04", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "05", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "05", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "05", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "06", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "06", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "06", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "07", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "07", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "07", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "08", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "08", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "08", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "09", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "09", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "09", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "10", "30",
	"ETSI", "2.4G", "20M", "HT", "4T", "10", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "10", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "11", "26",
	"ETSI", "2.4G", "20M", "HT", "4T", "11", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "11", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "12", "63",
	"ETSI", "2.4G", "20M", "HT", "4T", "12", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "12", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "13", "63",
	"ETSI", "2.4G", "20M", "HT", "4T", "13", "26",
	"MKK", "2.4G", "20M", "HT", "4T", "13", "26",
	"FCC", "2.4G", "20M", "HT", "4T", "14", "63",
	"ETSI", "2.4G", "20M", "HT", "4T", "14", "63",
	"MKK", "2.4G", "20M", "HT", "4T", "14", "63",
	"FCC", "2.4G", "40M", "HT", "1T", "01", "63",
	"ETSI", "2.4G", "40M", "HT", "1T", "01", "63",
	"MKK", "2.4G", "40M", "HT", "1T", "01", "63",
	"FCC", "2.4G", "40M", "HT", "1T", "02", "63",
	"ETSI", "2.4G", "40M", "HT", "1T", "02", "63",
	"MKK", "2.4G", "40M", "HT", "1T", "02", "63",
	"FCC", "2.4G", "40M", "HT", "1T", "03", "32",
	"ETSI", "2.4G", "40M", "HT", "1T", "03", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "03", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "04", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "04", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "04", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "05", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "05", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "05", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "06", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "06", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "06", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "07", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "07", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "07", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "08", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "08", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "08", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "09", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "09", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "09", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "10", "36",
	"ETSI", "2.4G", "40M", "HT", "1T", "10", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "10", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "11", "32",
	"ETSI", "2.4G", "40M", "HT", "1T", "11", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "11", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "12", "63",
	"ETSI", "2.4G", "40M", "HT", "1T", "12", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "12", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "13", "63",
	"ETSI", "2.4G", "40M", "HT", "1T", "13", "32",
	"MKK", "2.4G", "40M", "HT", "1T", "13", "32",
	"FCC", "2.4G", "40M", "HT", "1T", "14", "63",
	"ETSI", "2.4G", "40M", "HT", "1T", "14", "63",
	"MKK", "2.4G", "40M", "HT", "1T", "14", "63",
	"FCC", "2.4G", "40M", "HT", "2T", "01", "63",
	"ETSI", "2.4G", "40M", "HT", "2T", "01", "63",
	"MKK", "2.4G", "40M", "HT", "2T", "01", "63",
	"FCC", "2.4G", "40M", "HT", "2T", "02", "63",
	"ETSI", "2.4G", "40M", "HT", "2T", "02", "63",
	"MKK", "2.4G", "40M", "HT", "2T", "02", "63",
	"FCC", "2.4G", "40M", "HT", "2T", "03", "30",
	"ETSI", "2.4G", "40M", "HT", "2T", "03", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "03", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "04", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "04", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "04", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "05", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "05", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "05", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "06", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "06", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "06", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "07", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "07", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "07", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "08", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "08", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "08", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "09", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "09", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "09", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "10", "34",
	"ETSI", "2.4G", "40M", "HT", "2T", "10", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "10", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "11", "30",
	"ETSI", "2.4G", "40M", "HT", "2T", "11", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "11", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "12", "63",
	"ETSI", "2.4G", "40M", "HT", "2T", "12", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "12", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "13", "63",
	"ETSI", "2.4G", "40M", "HT", "2T", "13", "30",
	"MKK", "2.4G", "40M", "HT", "2T", "13", "30",
	"FCC", "2.4G", "40M", "HT", "2T", "14", "63",
	"ETSI", "2.4G", "40M", "HT", "2T", "14", "63",
	"MKK", "2.4G", "40M", "HT", "2T", "14", "63",
	"FCC", "2.4G", "40M", "HT", "3T", "01", "63",
	"ETSI", "2.4G", "40M", "HT", "3T", "01", "63",
	"MKK", "2.4G", "40M", "HT", "3T", "01", "63",
	"FCC", "2.4G", "40M", "HT", "3T", "02", "63",
	"ETSI", "2.4G", "40M", "HT", "3T", "02", "63",
	"MKK", "2.4G", "40M", "HT", "3T", "02", "63",
	"FCC", "2.4G", "40M", "HT", "3T", "03", "28",
	"ETSI", "2.4G", "40M", "HT", "3T", "03", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "03", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "04", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "04", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "04", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "05", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "05", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "05", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "06", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "06", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "06", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "07", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "07", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "07", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "08", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "08", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "08", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "09", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "09", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "09", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "10", "32",
	"ETSI", "2.4G", "40M", "HT", "3T", "10", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "10", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "11", "28",
	"ETSI", "2.4G", "40M", "HT", "3T", "11", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "11", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "12", "63",
	"ETSI", "2.4G", "40M", "HT", "3T", "12", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "12", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "13", "63",
	"ETSI", "2.4G", "40M", "HT", "3T", "13", "28",
	"MKK", "2.4G", "40M", "HT", "3T", "13", "28",
	"FCC", "2.4G", "40M", "HT", "3T", "14", "63",
	"ETSI", "2.4G", "40M", "HT", "3T", "14", "63",
	"MKK", "2.4G", "40M", "HT", "3T", "14", "63",
	"FCC", "2.4G", "40M", "HT", "4T", "01", "63",
	"ETSI", "2.4G", "40M", "HT", "4T", "01", "63",
	"MKK", "2.4G", "40M", "HT", "4T", "01", "63",
	"FCC", "2.4G", "40M", "HT", "4T", "02", "63",
	"ETSI", "2.4G", "40M", "HT", "4T", "02", "63",
	"MKK", "2.4G", "40M", "HT", "4T", "02", "63",
	"FCC", "2.4G", "40M", "HT", "4T", "03", "26",
	"ETSI", "2.4G", "40M", "HT", "4T", "03", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "03", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "04", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "04", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "04", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "05", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "05", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "05", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "06", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "06", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "06", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "07", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "07", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "07", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "08", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "08", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "08", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "09", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "09", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "09", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "10", "30",
	"ETSI", "2.4G", "40M", "HT", "4T", "10", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "10", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "11", "26",
	"ETSI", "2.4G", "40M", "HT", "4T", "11", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "11", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "12", "63",
	"ETSI", "2.4G", "40M", "HT", "4T", "12", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "12", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "13", "63",
	"ETSI", "2.4G", "40M", "HT", "4T", "13", "26",
	"MKK", "2.4G", "40M", "HT", "4T", "13", "26",
	"FCC", "2.4G", "40M", "HT", "4T", "14", "63",
	"ETSI", "2.4G", "40M", "HT", "4T", "14", "63",
	"MKK", "2.4G", "40M", "HT", "4T", "14", "63"
};
#endif

void
odm_read_and_config_mp_8197g_txpwr_lmt_8197gmp_type6(struct dm_struct *dm)
{
#ifdef CONFIG_8197G_8197GMP_TYPE6

	u32	i = 0;
#if (DM_ODM_SUPPORT_TYPE == ODM_IOT)
	u32	array_len =
			sizeof(array_mp_8197g_txpwr_lmt_8197gmp_type6) / sizeof(u8);
	u8	*array = (u8 *)array_mp_8197g_txpwr_lmt_8197gmp_type6;
#else
	u32	array_len =
			sizeof(array_mp_8197g_txpwr_lmt_8197gmp_type6) / sizeof(u8 *);
	u8	**array = (u8 **)array_mp_8197g_txpwr_lmt_8197gmp_type6;
#endif

#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	void	*adapter = dm->adapter;
	HAL_DATA_TYPE	*hal_data = GET_HAL_DATA(((PADAPTER)adapter));

	odm_memory_set(dm, hal_data->BufOfLinesPwrLmt, 0,
		       MAX_LINES_HWCONFIG_TXT *
		       MAX_BYTES_LINE_HWCONFIG_TXT);
	hal_data->nLinesReadPwrLmt = array_len / 7;
#endif

	PHYDM_DBG(dm, ODM_COMP_INIT, "===> %s\n", __func__);

	for (i = 0; i < array_len; i += 7) {
#if (DM_ODM_SUPPORT_TYPE == ODM_IOT)
		u8	regulation = array[i];
		u8	band = array[i + 1];
		u8	bandwidth = array[i + 2];
		u8	rate = array[i + 3];
		u8	rf_path = array[i + 4];
		u8	chnl = array[i + 5];
		u8	val = array[i + 6];
#else
		u8	*regulation = array[i];
		u8	*band = array[i + 1];
		u8	*bandwidth = array[i + 2];
		u8	*rate = array[i + 3];
		u8	*rf_path = array[i + 4];
		u8	*chnl = array[i + 5];
		u8	*val = array[i + 6];
#endif

		odm_config_bb_txpwr_lmt_8197g(dm, regulation, band, bandwidth,
					      rate, rf_path, chnl, val);
#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
		rsprintf((char *)hal_data->BufOfLinesPwrLmt[i / 7], 100, "\"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\",",
			 regulation, band, bandwidth, rate, rf_path, chnl, val);
#endif
	}

#endif
}

#endif /* end of HWIMG_SUPPORT*/

