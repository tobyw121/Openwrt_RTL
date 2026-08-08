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
 * $Revision: 50569 $
 * $Date: 2014-08-22 15:20:56 +0800 (Fri, 22 Aug 2014) $
 *
 * Purpose : DRV APIs definition.
 *
 * Feature : SMI relative API
 *
 */
#ifndef __DRV_SMI_H__
#define __DRV_SMI_H__

/*
* Include Files
*/ 
#include <common/rt_error.h>
     
/*
* Symbol Definition
*/
/*Power over Ethernet*/
#define SI3452 TRUE
#define PD64012 TRUE

#ifdef PD64012
#define PD64012_NUM_CHIPS 2
#define PD64012_Master_DeviceID    0x24 /*0x48 Address #9  0b010,010,00 ->inverse 0b000,1001 0x9 */
#define PD64012_Slave_DeviceID     0x29 /*0x50 Address #10 0b010,100,10 ->inverse 0b000,0101 0x5*/
#endif
     
#ifdef PD69012
#define POE_NUM_CHIPS 1
#define POE_Master_DeviceID    0x30
#define POE_Slave_DeviceID NULL
#endif
      
#ifdef SI3452
#define POE_NUM_CHIPS 3
#define POE_Master_DeviceID    0x20 /**/
#define POE_Slave_DeviceID     0x24 /**/
#define POE_Slave2_DeviceID     0x28 /**/
#endif
     
/* SMI devices which are accessed through 8389's GPIO */
enum SMI_DEVICE
{
    SMI_DEVICE_SFP1 = 0,    /*normal waveform*/
    SMI_DEVICE_SFP2,
    SMI_DEVICE_SFP3,
    SMI_DEVICE_SFP4,
    SMI_DEVICE_PD64012,  /*informal waveform*/ 
    SMI_DEVICE_SI3452,
    SMI_DEVICE_NONE6,
    SMI_DEVICE_NONE7,
    SMI_DEVICE_NONE8,
    SMI_DEVICE_NONE9,
    SMI_DEVICE_NONE10,
    SMI_DEVICE_NONE11,
    SMI_DEVICE_NONE12,
    SMI_DEVICE_NONE13,
    SMI_DEVICE_NONE14,
    SMI_DEVICE_NONE15,
    SMI_DEVICE_MAX
};

enum SMI_TYPE
{
	SMI_TYPE_8BITS_DEV =0,
	SMI_TYPE_16BITS_DEV,
	SMI_TYPE_MAX
};
/*
 * Data Declaration
 */
 
/*
 * Macro Declaration
 */
#define SMI_DEVICE_CHK(dev)     ((dev >= 0) && (dev < SMI_DEVICE_MAX))

/*
 * Function Declaration
 */ 


/* Function Name:
 *      drv_smi_type_set
 * Description:
 *      SMI init function
 * Input:
 *      portSCK - SCK port id
 *      pinSCK  - SCK pin
 *      portSDA - SDA port id
 *      pinSDA  - SDA pin 
 *      dev     - dev id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_OUT_OF_RANGE
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 drv_smi_type_set(uint32 smi_type, uint32 chipid, uint32 delay, uint32 dev);
/* Function Name:
 *      drv_smi_type_get
 * Description:
 *      SMI init function
 * Input:
 *      portSCK - SCK port id
 *      pinSCK  - SCK pin
 *      portSDA - SDA port id
 *      pinSDA  - SDA pin 
 *      dev     - dev id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_OUT_OF_RANGE
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 drv_smi_type_get(uint32 * ptype, uint32 * pchipid, uint32 * pdelay, uint32 dev);


/* Function Name:
 *      pd64012_write
 * Description:
 *      pd64012 read function
 * Input:
 *      mAddrs  - address
 *      dev     - dev id
 * Output:
 *      pRdata - data read 
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32 pd64012_smi_read(uint32 chipid, uint32 mAddrs, uint32 *pRdata, uint32 dev);

/* Function Name:
 *      pd64012_write
 * Description:
 *      pd64012 write function
 * Input:
 *      mAddrs  - address
 *      wData   - data write 
 *      dev     - dev id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      None
 */
int32 pd64012_smi_write(uint32 chipid, uint32 mAddrs, uint32 wData, uint32 dev);

/* Function Name:
 *      drv_smi_slavePresent
 * Description:
 *      Detect if slave is exist
 * Input:
 *      slave  - slave address
 *      dev     - dev id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK : slave exist
 *      RT_ERR_FAILED: slave not exist
 * Note:
 *      None
 */
int32 drv_smi_slavePresent(uint32 devid, uint32 dev);

/* Function Name:
 *      drv_smi_group_get
 * Description:
 *      SMI init function
 * Input:
 *      pDevSCK - SCK device id
 *      pinSCK  - SCK pin
 *      pDevDA - SDA device id
 *      pinSDA  - SDA pin
 *      dev     - dev id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_OUT_OF_RANGE
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 drv_smi_group_get(uint32 * pDevSCK, uint32 * pPinSCK, uint32 * pDevSDA, uint32 * pPinSDA, uint32 dev);

/* Function Name:
 *      drv_smi_init
 * Description:
 *      SMI init function
 * Input:
 *      devSCK - SCK GPIO dev id
 *      pinSCK  - SCK pin
 *      devSDA - SDA GPIO dev id
 *      pinSDA  - SDA pin
 *      dev     - dev id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_OUT_OF_RANGE
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 drv_smi_init(uint32 devSCK, uint32 pinSCK, uint32 devSDA, uint32 pinSDA, uint32 dev);

/* Function Name:
 *      drv_smi_write
 * Description:
 *      SMI write wrapper function
 * Input:
 *      mAddrs  - address
 *      wData   - data write
 *      dev     - dev id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      None
 */
int32 drv_smi_write(uint32 mAddrs, uint32 wData, uint32 dev);

/* Function Name:
 *      drv_smi_read
 * Description:
 *      SMI read wrapper function
 * Input:
 *      mAddrs  - address
 *      dev     - dev id
 * Output:
 *      pRdata  - data read
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      None
 */
int32 drv_smi_read(uint32 mAddrs, uint32 *pRdata, uint32 dev);

/* Function Name:
 *      drv_smi_module_init
 * Description:
 *      SMI init function
 * Input:
 *      unit	- unit ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 * Note:
 *      None
 */
int32 drv_smi_module_init(uint32 unit);


#endif /* __DRV_SMI_H__ */

