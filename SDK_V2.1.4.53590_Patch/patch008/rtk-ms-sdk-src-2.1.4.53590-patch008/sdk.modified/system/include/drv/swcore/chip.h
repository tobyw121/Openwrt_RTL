/*
 * Copyright (C) 2009 Realtek Semiconductor Corp. 
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated, 
 * modified or distributed under the authorized license from Realtek. 
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER 
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED. 
 *
 * $Revision: 57050 $
 * $Date: 2015-03-23 14:36:24 +0800 (Mon, 23 Mar 2015) $
 *
 * Purpose : chip symbol and data type definition in the SDK.
 *
 * Feature : chip symbol and data type definition
 *
 */

#ifndef __DRV_SWCORE_CHIP_H__
#define __DRV_SWCORE_CHIP_H__

/*
 * Include Files
 */
#include <common/type.h>

/*
 * Symbol Definition
 */

/* Definition family ID */
#define RTL8389_FAMILY_ID   (0x83890000)
#define RTL8328_FAMILY_ID   (0x83280000)
#define RTL8390_FAMILY_ID   (0x83900000)
#define RTL8350_FAMILY_ID   (0x83500000)
#define RTL8380_FAMILY_ID   (0x83800000)
#define RTL8330_FAMILY_ID   (0x83300000)

#define FAMILY_ID_MASK   	(0xFFF00000)

#define CHIP_FAMILY_IS_RTL8389(_chipId)         ((_chipId == RTL8389M_CHIP_ID) || \
                                                 (_chipId == RTL8389L_CHIP_ID) || \
                                                 (_chipId == RTL8329M_CHIP_ID) || \
                                                 (_chipId == RTL8377M_CHIP_ID)  )
#define CHIP_FAMILY_IS_RTL8328(_chipId)         ((_chipId == RTL8328M_CHIP_ID) || \
                                                 (_chipId == RTL8328S_CHIP_ID) || \
                                                 (_chipId == RTL8328L_CHIP_ID)  )
#define CHIP_FAMILY_IS_RTL8330(_chipId)         ((_chipId & FAMILY_ID_MASK) == RTL8330_FAMILY_ID)
#define CHIP_FAMILY_IS_RTL8380(_chipId)         ((_chipId != RTL8389M_CHIP_ID) &&   \
                                                 (_chipId != RTL8389L_CHIP_ID) &&   \
                                                 ((_chipId & FAMILY_ID_MASK) == RTL8380_FAMILY_ID))
#define CHIP_FAMILY_IS_RTL8350(_chipId)         ((_chipId & FAMILY_ID_MASK) == RTL8350_FAMILY_ID)
#define CHIP_FAMILY_IS_RTL8390(_chipId)         ((_chipId & FAMILY_ID_MASK) == RTL8390_FAMILY_ID)

/* Definition chip type */
#define CHIP_TYPE_NOTFOUND  (0x0)
#define CHIP_MES_TYPE        (0x1)
#define CHIP_MP_TYPE        (0x2)


/* Definition chip ID */
#define RTL8389M_CHIP_ID    (0x83896800)
#define RTL8389L_CHIP_ID    (0x83896000)
#define RTL8329M_CHIP_ID    (0x83296800)
#define RTL8329_CHIP_ID     (0x83290000)
#define RTL8377M_CHIP_ID    (0x83776800)
#define RTL8328M_CHIP_ID    (0x83284800)
#define RTL8328S_CHIP_ID    (0x83289800)
#define RTL8328L_CHIP_ID    (0x83286000)
#define RTL8352M_CHIP_ID    (0x83526800)
#define RTL8353M_CHIP_ID    (0x83536800)
#define RTL8390M_CHIP_ID    (0x83906800)
#define RTL8391M_CHIP_ID    (0x83916800)
#define RTL8392M_CHIP_ID    (0x83926800)
#define RTL8393M_CHIP_ID    (0x83936800)
#define RTL8396M_CHIP_ID    (0x83966800)
#define RTL8352MES_CHIP_ID  (0x83526966)
#define RTL8353MES_CHIP_ID  (0x83536966)
#define RTL8392MES_CHIP_ID  (0x83926966)
#define RTL8393MES_CHIP_ID  (0x83936966)
#define RTL8396MES_CHIP_ID  (0x83966966)
#define RTL8330M_CHIP_ID    (0x83306800)
#define RTL8332M_CHIP_ID    (0x83326800)
#define RTL8380M_CHIP_ID    (0x83806800)
#define RTL8382M_CHIP_ID    (0x83826800)
#define RTL8330MES_CHIP_ID  (0x83306966)
#define RTL8332MES_CHIP_ID  (0x83326966)
#define RTL8380MES_CHIP_ID  (0x83806966)
#define RTL8382MES_CHIP_ID  (0x83826966)

/* Definition chip revision ID */
#define CHIP_REV_ID_A       (0x0)
#define CHIP_REV_ID_B       (0x1)
#define CHIP_REV_ID_C       (0x2)
#define CHIP_REV_ID_D       (0x3)
#define CHIP_REV_ID_MAX     (255)   /* 0xFF */

/* Definition chip CPU Freq reg (0xBB00 switch part)*/
#define RTL839x5x_CPU_PLL0			(0x0028)
#define RTL839x5x_CPU_PLL1			(0x002C)

#define RTL838x3x_CPU_READ_CTL		(0x0058)
#define RTL838x3x_CPU_CTL0			(0x0FC4)
#define RTL838x3x_CPU_CTL1			(0x0FC8)
#define RTL838x3x_CPU_MISC			(0x0FCC)


/* Definition chip CPU Freq */
#define RTL839x5x_CPU_750M_PLL0     (0x000041AC)
#define RTL839x5x_CPU_750M_PLL1		(0x00000004)
#define RTL839x5x_CPU_700M_PLL0     (0x0000418C)
#define RTL839x5x_CPU_700M_PLL1		(0x00000004)
#define RTL839x5x_CPU_650M_PLL0     (0x0000416C)
#define RTL839x5x_CPU_650M_PLL1		(0x00000004)
#define RTL839x5x_CPU_500M_PLL0     (0x000041AC)
#define RTL839x5x_CPU_500M_PLL1		(0x00000005)

#define RTL838x3x_CPU_500M_CTL0		(0x00004748)
#define RTL838x3x_CPU_500M_CTL1     (0x0C14530E)
#define RTL838x3x_CPU_500M_MISC		(0x2DA0CFB8)
#define RTL838x3x_CPU_300M_CTL0		(0x000045C8)
#define RTL838x3x_CPU_300M_CTL1     (0x1414530E)
#define RTL838x3x_CPU_300M_MISC		(0x2D20CFB8)


typedef struct cid_group_s
{
    uint32 cid;     /* Chip ID  */
    uint32 gid;     /* Group ID */        
} cid_group_t;

extern uint32 drv_chip_family_id;


/*
 * Data Type Definition
 */

/*
 * Function Declaration
 */

/* Function Name:
 *      drv_swcore_cid_get
 * Description:
 *      Get chip id and chip revision id.
 * Input:
 *      unit           - unit id
 * Output:
 *      pChip_id       - pointer buffer of chip id
 *      pChip_rev_id   - pointer buffer of chip revision id
 * Return:
 *      RT_ERR_OK      - OK
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      None
 */
extern int32
drv_swcore_cid_get(uint32 unit, uint32 *pChip_id, uint32 *pChip_rev_id);

/* Function Name:
 *      drv_swcore_cid_cmp
 * Description:
 *      Compare cmp_id with the chip id of unit
 * Input:
 *      unit           - unit id
 *      cmp_id         - cmp id
 * Output:
 *      None
 * Return:
 *      0              - identical
 *      RT_ERR_FAILED  - not identical
 * Note:
 *      None
 */
extern int32
drv_swcore_cid_cmp(uint32 unit, uint32 cmp_id);

/* Function Name:
 *      drv_swcore_family_cid_get
 * Description:
 *      Get Chip Family ID
 * Input:
 *      unit           - unit id
 * Output:
 *      pFamily_id       - Family_id
 * Return:
 *      0              - identical
 *      RT_ERR_FAILED  - not identical
 * Note:
 *      None
 */
extern int32
drv_swcore_family_cid_get(uint32 unit, uint32 * pFamily_id);

/* Function Name:
 *      drv_swcore_CPU_freq_get
 * Description:
 *      Get Chip CPU Frequency
 * Input:
 *      unit           - unit id
 * Output:
 *      pCPU_freq       - CPU_freq
 * Return:
 *      RT_ERR_OK        - OK
 *      RT_ERR_FAILED   - Failed 
 *      RT_ERR_CHIP_NOT_SUPPORTED - Not support by this API
 * Note:
 *      None
 */
extern int32
drv_swcore_CPU_freq_get(uint32 unit, uint32 * pCPU_freq);

/* Function Name:
 *      drv_swcore_register_dump
 * Description:
 *      Dump Chip peripher registers
 * Input:
 *      unit           - unit id
 * Output:
 *      N/A
 * Return:
 *      RT_ERR_OK        - OK
 *      RT_ERR_FAILED   - Failed
 * Note:
 *      None
 */
extern int32
drv_swcore_register_dump(uint32 unit);


/* Function Name:
 *      drv_swcore_sysMac_get
 * Description:
 *      Get mac address of the system
 * Input:
 *      None
 * Output:
 *      pMacAddr - mac address
 * Return:
 *      RT_ERR_OK  
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
extern int32
drv_swcore_sysMac_get(uint8 *pMacAddr);

/* Function Name:
 *      drv_swcore_sysMac_set
 * Description:
 *      Set mac address of the system
 * Input:
 *      pMacAddr - mac address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK  
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
extern int32
drv_swcore_sysMac_set(uint8 *pMacAddr);

#endif  /* __DRV_SWCORE_CHIP_H__ */

