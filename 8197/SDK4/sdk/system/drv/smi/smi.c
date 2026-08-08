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

/*
 * Include Files
 */
#include <common/debug/rt_log.h>
#include <osal/sem.h>
#include <drv/gpio/gpio.h>
#include <drv/smi/smi.h>
#include <drv/gpio/generalCtrl_gpio.h>

/*
 * Symbol Definition
 */
#define ACK_TIMER                       5
#define DELAY_4000                      4000 /*100KHz*/
#define DELAY_1800                      1800 /*100KHz*/
#define SFP_CHIPID                      0x50
#define SFP_TYPE                        SMI_TYPE_8BITS_DEV

/*
 * Data Declaration
 */
static gpioID smi_TYPE[SMI_DEVICE_MAX];        /* SMI waveform function */
static gpioID smi_CHIPID[SMI_DEVICE_MAX];      /* CHIP ID for PD64012 type device */
static gpioID smi_DELAY[SMI_DEVICE_MAX];       /* CHIP ID for PD64012 type device */
static osal_mutex_t smi_SEM[SMI_DEVICE_MAX];   /* SMI semaphore */
static init_state_t smi_INIT_FLAG[SMI_DEVICE_MAX];   /* SMI device initial flag */


static drv_generalCtrlGpio_gpioId_t general_smi_SCK[SMI_DEVICE_MAX];
static drv_generalCtrlGpio_gpioId_t general_smi_SDA[SMI_DEVICE_MAX];

/*
 * Macro Declaration
 */
/* loop 300,000 time for 10ms */
#define CLK_DURATION(clk)               do { int i; for (i = 0; i < clk; i++); } while(0)

#define SMI_SEM_INIT(dev)\
do {\
    if (0 == (smi_SEM[dev] = osal_sem_mutex_create()))\
    {\
        RT_LOG(LOG_DEBUG, MOD_GENERAL, "SMI semaphore create failed");\
        return RT_ERR_FAILED;\
    }\
} while(0)

#define SMI_SEM_LOCK(dev)\
do {\
    if (osal_sem_mutex_take(smi_SEM[dev], OSAL_SEM_WAIT_FOREVER) != RT_ERR_OK)\
    {\
        RT_LOG(LOG_DEBUG, MOD_GENERAL, "SMI semaphore lock failed");\
        return RT_ERR_FAILED;\
    }\
} while(0)

#define SMI_SEM_UNLOCK(dev)\
do {\
    if (osal_sem_mutex_give(smi_SEM[dev]) != RT_ERR_OK)\
    {\
        RT_LOG(LOG_DEBUG, MOD_GENERAL, "SMI semaphore unlock failed");\
        return RT_ERR_FAILED;\
    }\
} while(0)

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
int32 drv_smi_type_set(uint32 smi_type, uint32 chipid, uint32 delay, uint32 dev)
{
    smi_TYPE[dev] = smi_type;
    smi_CHIPID[dev] = chipid;
    smi_DELAY[dev] = delay;

    return RT_ERR_OK;
} /* end of drv_smi_type_set */

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
int32 drv_smi_type_get(uint32 * ptype, uint32 * pchipid, uint32 * pdelay, uint32 dev)
{
    *ptype = smi_TYPE[dev];
    *pchipid = smi_CHIPID[dev];
    *pdelay = smi_DELAY[dev];

    return RT_ERR_OK;
} /* end of drv_smi_type_get */

/* Function Name:
 *      _general_smi_start
 * Description:
 *      SMI start function
 * Input:
 *      dev - dev id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      None
 */
static int32 _general_smi_start(uint32 dev)
{
    int32 ret = RT_ERR_FAILED;
	uint32 unit = 0;
	drv_generalCtrlGpio_pinConf_t gpioConfig;

	/* change GPIO pin to Output only */
	gpioConfig.direction = GPIO_DIR_OUT;
	gpioConfig.default_value = 1;
	gpioConfig.int_gpio.function = GPIO_CTRLFUNC_NORMAL;
	gpioConfig.int_gpio.interruptEnable = GPIO_INT_DISABLE;
	gpioConfig.ext_gpio.debounce = 0;
	gpioConfig.ext_gpio.inverter =0;
	gpioConfig.ext_gpio.direction = GPIO_DIR_OUT;
	
	RT_ERR_CHK(drv_generalCtrlGPIO_pin_init(unit, general_smi_SDA[dev].devId, general_smi_SDA[dev].pinId, &gpioConfig), ret);
	RT_ERR_CHK(drv_generalCtrlGPIO_pin_init(unit, general_smi_SCK[dev].devId, general_smi_SCK[dev].pinId, &gpioConfig), ret);

    /* Initial state: SCK: 0, SDA: 1 */
//    RT_ERR_CHK(drv_gpio_dataBit_set(smi_SCK[dev], 0), ret);
//    RT_ERR_CHK(drv_gpio_dataBit_set(smi_SDA[dev], 1), ret);
	RT_ERR_CHK(drv_generalCtrlGPIO_dataBit_set(unit, general_smi_SCK[dev].devId, general_smi_SCK[dev].pinId, 0), ret);
	RT_ERR_CHK(drv_generalCtrlGPIO_dataBit_set(unit, general_smi_SDA[dev].devId, general_smi_SDA[dev].pinId, 1), ret);
	
    /* CLK 1: 0 -> 1, 1 -> 0 */
    CLK_DURATION(smi_DELAY[dev]);
//    RT_ERR_CHK(drv_gpio_dataBit_set(smi_SCK[dev], 1), ret);
	RT_ERR_CHK(drv_generalCtrlGPIO_dataBit_set(unit, general_smi_SCK[dev].devId, general_smi_SCK[dev].pinId, 1), ret);


    CLK_DURATION(smi_DELAY[dev]);
//    RT_ERR_CHK(drv_gpio_dataBit_set(smi_SCK[dev], 0), ret);
	RT_ERR_CHK(drv_generalCtrlGPIO_dataBit_set(unit, general_smi_SCK[dev].devId, general_smi_SCK[dev].pinId, 0), ret);
	

    /* CLK 2: */
    CLK_DURATION(smi_DELAY[dev]);
//    RT_ERR_CHK(drv_gpio_dataBit_set(smi_SCK[dev], 1), ret);
	RT_ERR_CHK(drv_generalCtrlGPIO_dataBit_set(unit, general_smi_SCK[dev].devId, general_smi_SCK[dev].pinId, 1), ret);
	

    CLK_DURATION(smi_DELAY[dev]);
//    RT_ERR_CHK(drv_gpio_dataBit_set(smi_SDA[dev], 0), ret);
	RT_ERR_CHK(drv_generalCtrlGPIO_dataBit_set(unit, general_smi_SDA[dev].devId, general_smi_SDA[dev].pinId, 0), ret);
	

    CLK_DURATION(smi_DELAY[dev]);
//    RT_ERR_CHK(drv_gpio_dataBit_set(smi_SCK[dev], 0), ret);
	RT_ERR_CHK(drv_generalCtrlGPIO_dataBit_set(unit, general_smi_SCK[dev].devId, general_smi_SCK[dev].pinId, 0), ret);
	

    CLK_DURATION(smi_DELAY[dev]);
//    RT_ERR_CHK(drv_gpio_dataBit_set(smi_SDA[dev], 1), ret);
	RT_ERR_CHK(drv_generalCtrlGPIO_dataBit_set(unit, general_smi_SDA[dev].devId, general_smi_SDA[dev].pinId, 1), ret);

    return ret;
} /* end of _general_smi_start */

/* Function Name:
 *      _general_smi_stop
 * Description:
 *      SMI stop function
 * Input:
 *      dev  - dev id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      None
 */
static int32 _general_smi_stop(uint32 dev)
{
    int32 ret = RT_ERR_FAILED;
	uint32 unit = 0;	
	drv_generalCtrlGpio_pinConf_t gpioConfig;

    CLK_DURATION(smi_DELAY[dev]);

//    RT_ERR_CHK(drv_gpio_dataBit_set(smi_SDA[dev], 0), ret);
	RT_ERR_CHK(drv_generalCtrlGPIO_dataBit_set(unit, general_smi_SDA[dev].devId, general_smi_SDA[dev].pinId, 0), ret);
	

    CLK_DURATION(smi_DELAY[dev]);
//    RT_ERR_CHK(drv_gpio_dataBit_set(smi_SCK[dev], 1), ret);
	RT_ERR_CHK(drv_generalCtrlGPIO_dataBit_set(unit, general_smi_SCK[dev].devId, general_smi_SCK[dev].pinId, 1), ret);

    CLK_DURATION(smi_DELAY[dev]);
//    RT_ERR_CHK(drv_gpio_dataBit_set(smi_SDA[dev], 1), ret);
	RT_ERR_CHK(drv_generalCtrlGPIO_dataBit_set(unit, general_smi_SDA[dev].devId, general_smi_SDA[dev].pinId, 1), ret);

    CLK_DURATION(smi_DELAY[dev]);
//    RT_ERR_CHK(drv_gpio_dataBit_set(smi_SCK[dev], 1), ret);
	RT_ERR_CHK(drv_generalCtrlGPIO_dataBit_set(unit, general_smi_SCK[dev].devId, general_smi_SCK[dev].pinId, 1), ret);

    CLK_DURATION(smi_DELAY[dev]);
    CLK_DURATION(smi_DELAY[dev]);
//    RT_ERR_CHK(drv_gpio_dataBit_set(smi_SCK[dev], 0), ret);
	RT_ERR_CHK(drv_generalCtrlGPIO_dataBit_set(unit, general_smi_SCK[dev].devId, general_smi_SCK[dev].pinId, 0), ret);
	

    CLK_DURATION(smi_DELAY[dev]);
    CLK_DURATION(smi_DELAY[dev]);
//    RT_ERR_CHK(drv_gpio_dataBit_set(smi_SCK[dev], 1), ret);
	RT_ERR_CHK(drv_generalCtrlGPIO_dataBit_set(unit, general_smi_SCK[dev].devId, general_smi_SCK[dev].pinId, 1), ret);
	
    /* add a click */
    CLK_DURATION(smi_DELAY[dev]);
    CLK_DURATION(smi_DELAY[dev]);
//    RT_ERR_CHK(drv_gpio_dataBit_set(smi_SCK[dev], 0), ret);
	RT_ERR_CHK(drv_generalCtrlGPIO_dataBit_set(unit, general_smi_SCK[dev].devId, general_smi_SCK[dev].pinId, 0), ret);
	

    CLK_DURATION(smi_DELAY[dev]);
    CLK_DURATION(smi_DELAY[dev]);
//    RT_ERR_CHK(drv_gpio_dataBit_set(smi_SCK[dev], 1), ret);
	RT_ERR_CHK(drv_generalCtrlGPIO_dataBit_set(unit, general_smi_SCK[dev].devId, general_smi_SCK[dev].pinId, 1), ret);	

    /* change GPIO pin to Output only */
//    RT_ERR_CHK(drv_gpio_init(smi_SDA[dev], GPIO_CTRLFUNC_NORMAL, GPIO_DIR_IN, GPIO_INT_DISABLE), ret);
//    RT_ERR_CHK(drv_gpio_init(smi_SCK[dev], GPIO_CTRLFUNC_NORMAL, GPIO_DIR_IN, GPIO_INT_DISABLE), ret);
	gpioConfig.direction = GPIO_DIR_OUT;
	gpioConfig.default_value = 1;
	gpioConfig.int_gpio.function = GPIO_CTRLFUNC_NORMAL;
	gpioConfig.int_gpio.interruptEnable = GPIO_INT_DISABLE;
	gpioConfig.ext_gpio.debounce = 0;
	gpioConfig.ext_gpio.inverter =0;
	gpioConfig.ext_gpio.direction = GPIO_DIR_OUT;
	RT_ERR_CHK(drv_generalCtrlGPIO_pin_init(unit, general_smi_SDA[dev].devId, general_smi_SDA[dev].pinId, &gpioConfig), ret);
	RT_ERR_CHK(drv_generalCtrlGPIO_pin_init(unit, general_smi_SCK[dev].devId, general_smi_SCK[dev].pinId, &gpioConfig), ret);

    return ret;
} /* end of _general_smi_stop */


/* Function Name:
 *      _general_smi_readBit
 * Description:
 *      SMI read bit function
 * Input:
 *      bitLen  - bit length
 *      dev     - dev id
 * Output:
 *      pRdata - data read
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      None
 */
static int32 _general_smi_readBit(uint32 bitLen, uint32 *pRdata, uint32 dev)
{
	uint32 unit = 0;
    uint32 data = 0;
    uint32 delay = smi_DELAY[dev];
    int32  ret = RT_ERR_FAILED;
	drv_generalCtrlGpio_pinConf_t gpioConfig;

/*  Remove from orignal static defined DELAY_1800, now it is user confiurable.
    if(SMI_DEVICE_SI3452 == dev)
    {
        delay = DELAY_1800;
    }
*/

    /* change GPIO pin to Input only */
//    RT_ERR_CHK(drv_gpio_init(smi_SDA[dev], GPIO_CTRLFUNC_NORMAL, GPIO_DIR_IN, GPIO_INT_DISABLE), ret);
	CLK_DURATION(delay);

	gpioConfig.direction = GPIO_DIR_IN;
	gpioConfig.default_value = 1;
	gpioConfig.int_gpio.function = GPIO_CTRLFUNC_NORMAL;
	gpioConfig.int_gpio.interruptEnable = GPIO_INT_DISABLE;
	gpioConfig.ext_gpio.debounce = 0;
	gpioConfig.ext_gpio.inverter =0;
	gpioConfig.ext_gpio.direction = GPIO_DIR_IN;	
	RT_ERR_CHK(drv_generalCtrlGPIO_pin_init(unit, general_smi_SDA[dev].devId, general_smi_SDA[dev].pinId, &gpioConfig), ret);

    for (*pRdata = 0; bitLen > 0; bitLen--)
    {
        CLK_DURATION(delay);
        CLK_DURATION(delay);

        /* clocking */
//        RT_ERR_CHK(drv_gpio_dataBit_set(smi_SCK[dev], 1), ret);
		RT_ERR_CHK(drv_generalCtrlGPIO_dataBit_set(unit, general_smi_SCK[dev].devId, general_smi_SCK[dev].pinId, 1), ret);
		

        CLK_DURATION(delay);

//        RT_ERR_CHK(drv_gpio_dataBit_get(smi_SDA[dev], &data), ret);
		RT_ERR_CHK(drv_generalCtrlGPIO_dataBit_get(unit, general_smi_SDA[dev].devId, general_smi_SDA[dev].pinId, &data), ret);
		
		
        CLK_DURATION(delay);

//        RT_ERR_CHK(drv_gpio_dataBit_set(smi_SCK[dev], 0), ret);
		RT_ERR_CHK(drv_generalCtrlGPIO_dataBit_set(unit, general_smi_SCK[dev].devId, general_smi_SCK[dev].pinId, 0), ret);

        *pRdata |= (data << (bitLen - 1));
    }

    /* change GPIO pin to Output only */
    //RT_ERR_CHK(drv_gpio_init(smi_SDA[dev], GPIO_CTRLFUNC_NORMAL, GPIO_DIR_OUT, GPIO_INT_DISABLE), ret);
	gpioConfig.direction = GPIO_DIR_OUT;
	gpioConfig.default_value = 1;
	gpioConfig.int_gpio.function = GPIO_CTRLFUNC_NORMAL;
	gpioConfig.int_gpio.interruptEnable = GPIO_INT_DISABLE;
	gpioConfig.ext_gpio.debounce = 0;
	gpioConfig.ext_gpio.inverter =0;
	gpioConfig.ext_gpio.direction = GPIO_DIR_OUT;	
	RT_ERR_CHK(drv_generalCtrlGPIO_pin_init(unit, general_smi_SDA[dev].devId, general_smi_SDA[dev].pinId, &gpioConfig), ret);

    return RT_ERR_OK;
} /* end of _general_smi_readBit */

/* Function Name:
 *      _general_smi_writeBit
 * Description:
 *      SMI write bit function
 * Input:
 *      signal - ctrl code
 *      bitLen - bit length
 *      dev    - dev id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OUT_OF_RANGE
 *      RT_ERR_OK
 * Note:
 *      None
 */
static int32 _general_smi_writeBit(uint16 signal, uint32 bitLen, uint32 dev)
{
	uint32 unit = 0;
    int32 ret = RT_ERR_FAILED;
    uint32 delay = smi_DELAY[dev];

/*  Remove from orignal static defined DELAY_1800, now it is user confiurable.
    if(SMI_DEVICE_SI3452 == dev)
    {
        delay = DELAY_1800;
    }
*/

    for ( ; bitLen > 0; bitLen--)
    {
        CLK_DURATION(delay);

        /* prepare data */
        if (signal & (1 << (bitLen - 1)))
        {
//            RT_ERR_CHK(drv_gpio_dataBit_set(smi_SDA[dev], 1), ret);
			RT_ERR_CHK(drv_generalCtrlGPIO_dataBit_set(unit, general_smi_SDA[dev].devId, general_smi_SDA[dev].pinId, 1), ret);
			
        }
        else
        {
//            RT_ERR_CHK(drv_gpio_dataBit_set(smi_SDA[dev], 0), ret);
			RT_ERR_CHK(drv_generalCtrlGPIO_dataBit_set(unit, general_smi_SDA[dev].devId, general_smi_SDA[dev].pinId, 0), ret);
			
        }

        CLK_DURATION(delay);

        /* clocking */
//        RT_ERR_CHK(drv_gpio_dataBit_set(smi_SCK[dev], 1), ret);
		RT_ERR_CHK(drv_generalCtrlGPIO_dataBit_set(unit, general_smi_SCK[dev].devId, general_smi_SCK[dev].pinId, 1), ret);
		
        CLK_DURATION(delay);
        CLK_DURATION(delay);
//        RT_ERR_CHK(drv_gpio_dataBit_set(smi_SCK[dev], 0), ret);
		RT_ERR_CHK(drv_generalCtrlGPIO_dataBit_set(unit, general_smi_SCK[dev].devId, general_smi_SCK[dev].pinId, 0), ret);
		
    }

    return RT_ERR_OK;
} /* end of _general_smi_writeBit */

/* Function Name:
 *      _general_8bits_dev_start
 * Description:
 *      8bits device start function
 * Input:
 *      dev - dev id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      None
 */
static int32 _general_8bits_dev_start(uint32 dev)
{
    int32 ret = RT_ERR_FAILED;
	uint32 unit = 0;	
	drv_generalCtrlGpio_pinConf_t gpioConfig;

    /* change GPIO pin to Output only */
	gpioConfig.direction = GPIO_DIR_OUT;
	gpioConfig.default_value = 1;
	gpioConfig.int_gpio.function = GPIO_CTRLFUNC_NORMAL;
	gpioConfig.int_gpio.interruptEnable = GPIO_INT_DISABLE;	
	gpioConfig.ext_gpio.debounce = 0;
	gpioConfig.ext_gpio.inverter =0;
	gpioConfig.ext_gpio.direction = GPIO_DIR_OUT;
	
	RT_ERR_CHK(drv_generalCtrlGPIO_pin_init(unit, general_smi_SDA[dev].devId, general_smi_SDA[dev].pinId, &gpioConfig), ret);
	RT_ERR_CHK(drv_generalCtrlGPIO_pin_init(unit, general_smi_SCK[dev].devId, general_smi_SCK[dev].pinId, &gpioConfig), ret);

	RT_ERR_CHK(drv_generalCtrlGPIO_dataBit_set(unit, general_smi_SDA[dev].devId, general_smi_SDA[dev].pinId, 1), ret);

	RT_ERR_CHK(drv_generalCtrlGPIO_dataBit_set(unit, general_smi_SCK[dev].devId, general_smi_SCK[dev].pinId, 0), ret);
	
    CLK_DURATION(smi_DELAY[dev]);
	RT_ERR_CHK(drv_generalCtrlGPIO_dataBit_set(unit, general_smi_SCK[dev].devId, general_smi_SCK[dev].pinId, 1), ret);

    CLK_DURATION(smi_DELAY[dev]);
	RT_ERR_CHK(drv_generalCtrlGPIO_dataBit_set(unit, general_smi_SDA[dev].devId, general_smi_SDA[dev].pinId, 0), ret);
	

    CLK_DURATION(smi_DELAY[dev]);
	RT_ERR_CHK(drv_generalCtrlGPIO_dataBit_set(unit, general_smi_SCK[dev].devId, general_smi_SCK[dev].pinId, 0), ret);

    return ret;
} /* end of _general_8bits_dev2_start */

/* Function Name:
 *      _general_8bits_dev_smi_read
 * Description:
 *      8bits device read function
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
static int32 _general_8bits_dev_read(uint32 mAddrs, uint8 *pRdata, uint32 dev)
{
    uint32 rawData = 0;
    uint32 ack = 0;
    uint32 slave = 0;
    int32  ret = RT_ERR_FAILED;
	uint32 unit = 0;
	uint16 regAddr;

    /*Initial data memory*/
    slave = smi_CHIPID[dev] & 0xff;
    RT_DBG(LOG_DEBUG, MOD_GENERAL, "[Read]devid 0x%x addr 0x%x ack 0x%x", slave, mAddrs, ack);
    mAddrs = mAddrs & 0xff;
    *pRdata = 0;

    /* CTRL code: 7'slave adress */
    RT_ERR_CHK(_general_smi_writeBit(slave, 7, dev), ret);

    /* 0: issue WRITE command */
    RT_ERR_CHK(_general_smi_writeBit(0x0, 1, dev), ret);
//	RT_ERR_CHK(_general_smi_writeBit(0x1, 1, dev), ret);

    /* Read ack for issuing WRITE command*/
    RT_ERR_CHK(_general_smi_readBit(1, &ack, dev), ret);

    /* Check ack value, it should be zero. */
    if (0 != ack)
    {
        RT_LOG(LOG_DEBUG, MOD_GENERAL, "[Read]devid 0x%x addr 0x%x ack 0x%x\n", slave, mAddrs, ack);
        RT_LOG(LOG_DEBUG, MOD_GENERAL, "[si3452_smi_read] Read first ack faild\n");
        RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error 1:,Send:Dev[6:0]_0, no Ack");	
        return RT_ERR_FAILED;
    }

    /* Set data pin to high */
//    RT_ERR_CHK(drv_gpio_dataBit_set(smi_SDA[dev], 1), ret);
	RT_ERR_CHK(drv_generalCtrlGPIO_dataBit_set(unit, general_smi_SDA[dev].devId, general_smi_SDA[dev].pinId, 1), ret);
	

    /* Note: 8-bit Reg Address */
	regAddr = (uint16)(mAddrs & 0xFF);
//    RT_ERR_CHK(_general_smi_writeBit((mAddrs & 0xFF), 8, dev), ret);
	RT_ERR_CHK(_general_smi_writeBit(regAddr, 8, dev), ret);


    /* Read ack for issuing WRITE command*/
    RT_ERR_CHK(_general_smi_readBit(1, &ack, dev), ret);

    /* Check ack value, it should be zero. */
    if (0 != ack)
    {
        RT_LOG(LOG_DEBUG, MOD_GENERAL, "[Read]devid 0x%x addr 0x%x ack 0x%x\n", slave, mAddrs, ack);
        RT_LOG(LOG_DEBUG, MOD_GENERAL, "[si3452_smi_read] Read second ack faild\n");
        RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error 2:Dev[6:0]_0|A|Addr[7:0], no Ack");	
        return RT_ERR_FAILED;
    }

    /* Set data pin to high */
//    RT_ERR_CHK(drv_gpio_dataBit_set(smi_SDA[dev], 1), ret);
	RT_ERR_CHK(drv_generalCtrlGPIO_dataBit_set(unit, general_smi_SDA[dev].devId, general_smi_SDA[dev].pinId, 1), ret);
	
    /* Start signal for Read */
    RT_ERR_CHK(_general_smi_start(dev), ret);

    /* Device Address & Read */
    RT_ERR_CHK(_general_smi_writeBit(slave, 7, dev), ret);

     /* 1: issue READ command */
    RT_ERR_CHK(_general_smi_writeBit(0x1, 1, dev), ret);

    /* Read ack for issuing READ command*/
    RT_ERR_CHK(_general_smi_readBit(1, &ack, dev), ret);

    /* Check ack value, it should be zero. */
    if (ack != 0)
    {
        RT_LOG(LOG_DEBUG, MOD_GENERAL, "[Read]devid 0x%x addr 0x%x ack 0x%x\n", slave, mAddrs, ack);
        RT_LOG(LOG_DEBUG, MOD_GENERAL, "[si3452_smi_read] Read third ack faild\n");
        RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error 3:Dev[6:0]_0|A|Addr[7:0]|A|Dev[6:0], no Ack");
        return RT_ERR_FAILED;
    }

    /* Read 1 bytes data, Read DATA [7:0] */
    RT_ERR_CHK(_general_smi_readBit(8, &rawData, dev), ret);

    *pRdata |= rawData & 0xFF;

    /* Return Nack = 1*/
    RT_ERR_CHK(_general_smi_writeBit(0x01, 1, dev), ret);


    return ret;
} /* end of _general_8bits_dev_smi_read */

/* Function Name:
 *      _general_8bits_dev_smi_write
 * Description:
 *      8bits device write function
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
static int32 _general_8bits_dev_write(uint32 mAddrs, uint32 wData, uint32 dev)
{
    uint32 ack = 0;
    uint32 slave = 0;
    int32  ret = RT_ERR_FAILED;
	uint32 unit = 0;

    /*Initial data memory*/
    slave = smi_CHIPID[dev] & 0xff;
    RT_DBG(LOG_DEBUG, MOD_GENERAL, "Write add: %02X ", slave);
    mAddrs = mAddrs & 0xff;

    /* CTRL code: 7'slave adress */
    RT_ERR_CHK(_general_smi_writeBit(slave, 7, dev), ret);

    /* 0: issue WRITE command */
    RT_ERR_CHK(_general_smi_writeBit(0x0, 1, dev), ret);

    /* Set data pin to high */
//    RT_ERR_CHK(drv_gpio_dataBit_set(smi_SDA[dev], 1), ret);
	RT_ERR_CHK(drv_generalCtrlGPIO_dataBit_set(unit, general_smi_SDA[dev].devId, general_smi_SDA[dev].pinId, 1), ret);


    /* Read ack for issuing WRITE command*/
    RT_ERR_CHK(_general_smi_readBit(1, &ack, dev), ret);

    /* Check ack value, it should be zero. */
    if (0 != ack)
    {
        RT_LOG(LOG_DEBUG, MOD_GENERAL, "[Write]devid 0x%x addr 0x%x ack 0x%x\n", slave, mAddrs, ack);
        RT_LOG(LOG_DEBUG, MOD_GENERAL, "[si3452_smi_write] Read first ack faild\n");
        return RT_ERR_FAILED;
    }

//    RT_ERR_CHK(drv_gpio_dataBit_set(smi_SDA[dev], 1), ret);
	RT_ERR_CHK(drv_generalCtrlGPIO_dataBit_set(unit, general_smi_SDA[dev].devId, general_smi_SDA[dev].pinId, 1), ret);
	

    /* Set reg_addr[7:0] */
    RT_ERR_CHK(_general_smi_writeBit((mAddrs & 0xFF), 8, dev), ret);

    /* Read ack for issuing WRITE command*/
    RT_ERR_CHK(_general_smi_readBit(1, &ack, dev), ret);

    /* Check ack value, it should be zero. */
    if (0 != ack)
    {
        RT_LOG(LOG_DEBUG, MOD_GENERAL, "[Write]devid 0x%x addr 0x%x ack 0x%x\n", slave, mAddrs, ack);
        RT_LOG(LOG_DEBUG, MOD_GENERAL, "[si3452_smi_write] Read second ack faild\n");
        return RT_ERR_FAILED;
    }


//    RT_ERR_CHK(drv_gpio_dataBit_set(smi_SDA[dev], 1), ret);
	RT_ERR_CHK(drv_generalCtrlGPIO_dataBit_set(unit, general_smi_SDA[dev].devId, general_smi_SDA[dev].pinId, 1), ret);
	
    /* Write Data [7:0] out */
    RT_ERR_CHK(_general_smi_writeBit((wData & 0xFF), 8, dev), ret);

    /* Read ack for issuing WRITE command*/
    RT_ERR_CHK(_general_smi_readBit(1, &ack, dev), ret);

    /* Check ack value, it should be zero. */
    if (0 != ack)
    {
        RT_LOG(LOG_DEBUG, MOD_GENERAL, "[Write]devid 0x%x addr 0x%x ack 0x%x\n", slave, mAddrs, ack);
        RT_LOG(LOG_DEBUG, MOD_GENERAL, "[si3452_smi_write] Read third ack faild\n");
        return RT_ERR_FAILED;
    }

//    RT_ERR_CHK(drv_gpio_dataBit_set(smi_SDA[dev], 1), ret);
	RT_ERR_CHK(drv_generalCtrlGPIO_dataBit_set(unit, general_smi_SDA[dev].devId, general_smi_SDA[dev].pinId, 1), ret);
	
    return ret;
} /* end of _general_8bits_dev_smi_write */


/* Function Name:
 *      _general_8bits_dev_stop
 * Description:
 *      8bits device stop function
 * Input:
 *      dev - dev id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      None
 */
static int32 _general_8bits_dev_stop(uint32 dev)
{
    int32 ret = RT_ERR_FAILED;
	uint32 unit = 0;	
	drv_generalCtrlGpio_pinConf_t gpioConfig;

    CLK_DURATION(smi_DELAY[dev]);
//    RT_ERR_CHK(drv_gpio_dataBit_set(smi_SDA[dev], 0), ret);
//    RT_ERR_CHK(drv_gpio_dataBit_set(smi_SCK[dev], 1), ret);
	RT_ERR_CHK(drv_generalCtrlGPIO_dataBit_set(unit, general_smi_SDA[dev].devId, general_smi_SDA[dev].pinId, 0), ret);
	RT_ERR_CHK(drv_generalCtrlGPIO_dataBit_set(unit, general_smi_SCK[dev].devId, general_smi_SCK[dev].pinId, 1), ret);
	

    CLK_DURATION(smi_DELAY[dev]);
//    RT_ERR_CHK(drv_gpio_dataBit_set(smi_SDA[dev], 1), ret);
	RT_ERR_CHK(drv_generalCtrlGPIO_dataBit_set(unit, general_smi_SDA[dev].devId, general_smi_SDA[dev].pinId, 1), ret);
	

    CLK_DURATION(smi_DELAY[dev]);
//    RT_ERR_CHK(drv_gpio_dataBit_set(smi_SCK[dev], 0), ret);
	RT_ERR_CHK(drv_generalCtrlGPIO_dataBit_set(unit, general_smi_SCK[dev].devId, general_smi_SCK[dev].pinId, 0), ret);

    /* change GPIO pin to Output only */
//    RT_ERR_CHK(drv_gpio_init(smi_SDA[dev], GPIO_CTRLFUNC_NORMAL, GPIO_DIR_IN, GPIO_INT_DISABLE), ret);
//    RT_ERR_CHK(drv_gpio_init(smi_SCK[dev], GPIO_CTRLFUNC_NORMAL, GPIO_DIR_IN, GPIO_INT_DISABLE), ret);
	gpioConfig.direction = GPIO_DIR_OUT;
	gpioConfig.default_value = 1;
	gpioConfig.int_gpio.function = GPIO_CTRLFUNC_NORMAL;
	gpioConfig.int_gpio.interruptEnable = GPIO_INT_DISABLE;
	gpioConfig.ext_gpio.debounce = 0;
	gpioConfig.ext_gpio.inverter =0;
	gpioConfig.ext_gpio.direction = GPIO_DIR_OUT;	
	RT_ERR_CHK(drv_generalCtrlGPIO_pin_init(unit, general_smi_SDA[dev].devId, general_smi_SDA[dev].pinId, &gpioConfig), ret);
	RT_ERR_CHK(drv_generalCtrlGPIO_pin_init(unit, general_smi_SCK[dev].devId, general_smi_SCK[dev].pinId, &gpioConfig), ret);

    return ret;
} /* end of _general_8bits_dev_stop */


/* Function Name:
 *      _general_16bits_start
 * Description:
 *      SMI start function
 * Input:
 *      dev - dev id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      PD64012
 */
static int32 _general_16bits_start(uint32 dev)
{
    int32 ret = RT_ERR_FAILED;
	uint32 unit = 0;
	drv_generalCtrlGpio_pinConf_t gpioConfig;

    /* change GPIO pin to Output only */
	gpioConfig.direction = GPIO_DIR_OUT;
	gpioConfig.default_value = 1;
	gpioConfig.int_gpio.function = GPIO_CTRLFUNC_NORMAL;
	gpioConfig.int_gpio.interruptEnable = GPIO_INT_DISABLE;
	gpioConfig.ext_gpio.debounce = 0;
	gpioConfig.ext_gpio.inverter =0;
	gpioConfig.ext_gpio.direction = GPIO_DIR_OUT;
	RT_ERR_CHK(drv_generalCtrlGPIO_pin_init(unit, general_smi_SDA[dev].devId, general_smi_SDA[dev].pinId, &gpioConfig), ret);
	RT_ERR_CHK(drv_generalCtrlGPIO_pin_init(unit, general_smi_SCK[dev].devId, general_smi_SCK[dev].pinId, &gpioConfig), ret);	
    //RT_ERR_CHK(drv_gpio_init(smi_SCK[dev], GPIO_CTRLFUNC_NORMAL, GPIO_DIR_OUT, GPIO_INT_DISABLE), ret);
    //RT_ERR_CHK(drv_gpio_init(smi_SDA[dev], GPIO_CTRLFUNC_NORMAL, GPIO_DIR_OUT, GPIO_INT_DISABLE), ret);

	RT_ERR_CHK(drv_generalCtrlGPIO_dataBit_set(unit, general_smi_SDA[dev].devId, general_smi_SDA[dev].pinId, 1), ret);
	RT_ERR_CHK(drv_generalCtrlGPIO_dataBit_set(unit, general_smi_SCK[dev].devId, general_smi_SCK[dev].pinId, 1), ret);	
    //RT_ERR_CHK(drv_gpio_dataBit_set(smi_SDA[dev], 1), ret);
    //RT_ERR_CHK(drv_gpio_dataBit_set(smi_SCK[dev], 1), ret);

    CLK_DURATION(smi_DELAY[dev]);
    //RT_ERR_CHK(drv_gpio_dataBit_set(smi_SDA[dev], 0), ret);
	RT_ERR_CHK(drv_generalCtrlGPIO_dataBit_set(unit, general_smi_SDA[dev].devId, general_smi_SDA[dev].pinId, 0), ret);

    /* Initial state: SCK: 0, SDA: 1 */
    CLK_DURATION(smi_DELAY[dev]);
    //RT_ERR_CHK(drv_gpio_dataBit_set(smi_SCK[dev], 0), ret);
	RT_ERR_CHK(drv_generalCtrlGPIO_dataBit_set(unit, general_smi_SCK[dev].devId, general_smi_SCK[dev].pinId, 0), ret);	

    return ret;
} /* end of _general_16bits_start */

/* Function Name:
 *      _general_16bits_read
 * Description:
 *      ina209 read function
 * Input:
 *      mAddrs  - address
 *      dev     - dev id
 * Output:
 *      pRdata - data read
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      PD64012.
 */
int32 _general_16bits_read(uint32 chipid, uint32 mAddrs, uint32 *pRdata, uint32 dev)
{
    uint32 rawData = 0;
    uint32 ack = 0;
    int32  ret = RT_ERR_FAILED;
    uint16 devid = 0;
    uint8  con = 0;
	uint32 unit = 0;
	drv_generalCtrlGpio_pinConf_t gpioConfig;

    /*Initial data memory*/
    *pRdata = 0;

    devid = chipid & 0xff;
    RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read:device:%04d, addr:%04d", devid, mAddrs);

    /* Start SMI */
    ret = _general_16bits_start(dev);
    if (RT_ERR_OK != ret)
    {
        RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error 1:Start");
        return ret;
    }

    /* Device Address & Write */

    ret = _general_smi_writeBit(devid, 7, dev);              /* CTRL code: 7'PD64012_MasterAddress */
    if (RT_ERR_OK != ret)
    {
        RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error 2:write devid");
        return ret;
    }

    ret = _general_smi_writeBit(0x0, 1, dev);               /* 1: issue READ command */
    if (RT_ERR_OK != ret)
    {
        RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error 3:write read_bit 0");
        return ret;
    }

	gpioConfig.direction = GPIO_DIR_IN;
	gpioConfig.default_value = 1;
	gpioConfig.int_gpio.function = GPIO_CTRLFUNC_NORMAL;
	gpioConfig.int_gpio.interruptEnable = GPIO_INT_DISABLE;
	gpioConfig.ext_gpio.debounce = 0;
	gpioConfig.ext_gpio.inverter =0;
	gpioConfig.ext_gpio.direction = GPIO_DIR_IN;
	ret = drv_generalCtrlGPIO_pin_init(unit, general_smi_SDA[dev].devId, general_smi_SDA[dev].pinId, &gpioConfig);
	
    if (RT_ERR_OK != ret)
    {
        RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error 4:init change dir to in");
        return ret;
    }

    do
    {
        con++;
        ret = _general_smi_readBit(1, &ack, dev);           /* ack for issuing READ command*/
        if (RT_ERR_OK != ret)
        {
            RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error 5:read bit");
            return ret;
        }
    } while ((0 != ack) && (con < ACK_TIMER));

	gpioConfig.direction = GPIO_DIR_OUT;
	gpioConfig.default_value = 1;
	gpioConfig.int_gpio.function = GPIO_CTRLFUNC_NORMAL;
	gpioConfig.int_gpio.interruptEnable = GPIO_INT_DISABLE;
	gpioConfig.ext_gpio.debounce = 0;
	gpioConfig.ext_gpio.inverter =0;
	gpioConfig.ext_gpio.direction = GPIO_DIR_OUT;
	ret = drv_generalCtrlGPIO_pin_init(unit, general_smi_SDA[dev].devId, general_smi_SDA[dev].pinId, &gpioConfig);

    if (RT_ERR_OK != ret)
    {
        RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error 8:Change direction");
        return ret;
    }


    if (0 != ack)
    {
        RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error 9: Dev[6:0]_0, no Ack");
        return RT_ERR_FAILED;
    }

    /* Note: 8-bit Reg Address */
    ret = _general_smi_writeBit((mAddrs & 0xFF), 8, dev);   /* Set reg_addr[7:0] */

    if (RT_ERR_OK != ret)
    {
        RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error A: Dev[6:0]_0|A|Addr[7:0]");
        return ret;
    }
	
	gpioConfig.direction = GPIO_DIR_IN;
	gpioConfig.default_value = 1;
	gpioConfig.int_gpio.function = GPIO_CTRLFUNC_NORMAL;
	gpioConfig.int_gpio.interruptEnable = GPIO_INT_DISABLE;
	gpioConfig.ext_gpio.debounce = 0;
	gpioConfig.ext_gpio.inverter =0;
	gpioConfig.ext_gpio.direction = GPIO_DIR_IN;
	ret = drv_generalCtrlGPIO_pin_init(unit, general_smi_SDA[dev].devId, general_smi_SDA[dev].pinId, &gpioConfig);

    if (RT_ERR_OK != ret)
    {
        RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error B: Dev[6:0]_0|A|Addr[7:0], change direction to in");
        return ret;
    }

    con = 0;
    do
    {
        con++;
        ret = _general_smi_readBit(1, &ack, dev);           /* ack for setting reg_addr[7:0] */
        if (RT_ERR_OK != ret)
        {
            RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error C");
            return ret;
        }
    } while ((0 != ack) && (con < ACK_TIMER));

	gpioConfig.direction = GPIO_DIR_OUT;
	gpioConfig.default_value = 1;
	gpioConfig.int_gpio.function = GPIO_CTRLFUNC_NORMAL;
	gpioConfig.int_gpio.interruptEnable = GPIO_INT_DISABLE;
	gpioConfig.ext_gpio.debounce = 0;
	gpioConfig.ext_gpio.inverter =0;
	gpioConfig.ext_gpio.direction = GPIO_DIR_OUT;
	ret = drv_generalCtrlGPIO_pin_init(unit, general_smi_SDA[dev].devId, general_smi_SDA[dev].pinId, &gpioConfig);

    if (RT_ERR_OK != ret)
    {
        return ret;
    }

    if (0 != ack)
    {
        RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error G: Dev[6:0]_0|A|Addr[7:0], no Ack");
        return RT_ERR_FAILED;
    }

    ret = _general_smi_writeBit((mAddrs >> 8), 8, dev);      /* Set reg_addr[15:8] */
    if (RT_ERR_OK != ret)
    {
        RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error H: Dev[6:0]_0|A|Addr[7:0]|A|Addr[15:8]");
        return ret;
    }
	
	gpioConfig.direction = GPIO_DIR_IN;
	gpioConfig.default_value = 1;
	gpioConfig.int_gpio.function = GPIO_CTRLFUNC_NORMAL;
	gpioConfig.int_gpio.interruptEnable = GPIO_INT_DISABLE;
	gpioConfig.ext_gpio.debounce = 0;
	gpioConfig.ext_gpio.inverter =0;
	gpioConfig.ext_gpio.direction = GPIO_DIR_IN;	
	ret = drv_generalCtrlGPIO_pin_init(unit, general_smi_SDA[dev].devId, general_smi_SDA[dev].pinId, &gpioConfig);

    if (RT_ERR_OK != ret)
    {
        RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error I: Dev[6:0]_0|A|Addr[7:0]|A|Addr[15:8], change direction to in");
        return ret;
    }

    con = 0;
    do
    {
        con++;
        ret = _general_smi_readBit(1, &ack, dev);           /* ack for setting reg_addr[15:8] */
        if (RT_ERR_OK != ret)
        {
            RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error 11");
            return ret;
        }
    } while ((0 != ack) && (con < ACK_TIMER));

	gpioConfig.direction = GPIO_DIR_OUT;
	gpioConfig.default_value = 1;
	gpioConfig.int_gpio.function = GPIO_CTRLFUNC_NORMAL;
	gpioConfig.int_gpio.interruptEnable = GPIO_INT_DISABLE;
	gpioConfig.ext_gpio.debounce = 0;
	gpioConfig.ext_gpio.inverter =0;
	gpioConfig.ext_gpio.direction = GPIO_DIR_OUT;
	ret = drv_generalCtrlGPIO_pin_init(unit, general_smi_SDA[dev].devId, general_smi_SDA[dev].pinId, &gpioConfig);

    if (RT_ERR_OK != ret)
    {
        RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error M: Dev[6:0]_0|A|Addr[7:0]|A|Addr[15:8], change direction to out");
        return ret;
    }

    if (0 != ack)
    {
        RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error 12: Dev[6:0]_0|A|Addr[7:0]|A|Addr[15:8], no Ack");
        return RT_ERR_FAILED;
    }

    /* Start signal for Read */
    ret = _general_smi_start(dev);                          /* Start SMI */
    if (RT_ERR_OK != ret)
    {
        RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error 13: Dev[6:0]_0|A|Addr[7:0]|A|Addr[15:8]|A, start");
        return ret;
    }

    /* Device Address & Read */
    ret = _general_smi_writeBit(devid, 7, dev);              /* CTRL code: 7'PD64012_MasterAddress */
    if (RT_ERR_OK != ret)
    {
        RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error 14: Dev[6:0]_0|A|Addr[7:0]|A|Addr[15:8]|A|Dev[6:0]");
        return ret;
    }

    ret = _general_smi_writeBit(0x1, 1, dev);               /* 1: issue READ command */
    if (RT_ERR_OK != ret)
    {
        RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error 17: Dev[6:0]_0|A|Addr[7:0]|A|Addr[15:8]|A|Dev[6:0]_1");
        return ret;
    }

	gpioConfig.direction = GPIO_DIR_IN;
	gpioConfig.default_value = 1;
	gpioConfig.int_gpio.function = GPIO_CTRLFUNC_NORMAL;
	gpioConfig.int_gpio.interruptEnable = GPIO_INT_DISABLE;
	gpioConfig.ext_gpio.debounce = 0;
	gpioConfig.ext_gpio.inverter =0;
	gpioConfig.ext_gpio.direction = GPIO_DIR_IN;	
	ret = drv_generalCtrlGPIO_pin_init(unit, general_smi_SDA[dev].devId, general_smi_SDA[dev].pinId, &gpioConfig);

    if (RT_ERR_OK != ret)
    {
        RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error N: Dev[6:0]_0|A|Addr[7:0]|A|Addr[15:8]|A|Dev[6:0]_1 change direction to in");
        return ret;
    }

    con = 0;
    do
    {
        con++;
        ret = _general_smi_readBit(1, &ack, dev);           /* ack for issuing READ command*/
        if (RT_ERR_OK != ret)
        {
            RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error 18");
            return ret;
        }
    } while ((0 != ack) && (con < ACK_TIMER));

    if (ack != 0)
    {
        RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error 19: Dev[6:0]_0|A|Addr[7:0]|A|Addr[15:8]|A|Dev[6:0]_1, no Ack");
        return RT_ERR_FAILED;
    }

    /* Read 2 bytes data */
    ret = _general_smi_readBit(8, &rawData, dev);           /* Read DATA [7:0] */
    if (RT_ERR_OK != ret)
    {
        RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error 20: Dev[6:0]_0|A|Addr[7:0]|A|Addr[15:8]|A|Dev[6:0]_1|A, no datat read");
        return ret;
    }

	gpioConfig.direction = GPIO_DIR_OUT;
	gpioConfig.default_value = 1;
	gpioConfig.int_gpio.function = GPIO_CTRLFUNC_NORMAL;
	gpioConfig.int_gpio.interruptEnable = GPIO_INT_DISABLE;
	gpioConfig.ext_gpio.debounce = 0;
	gpioConfig.ext_gpio.inverter =0;
	gpioConfig.ext_gpio.direction = GPIO_DIR_OUT;	
	ret = drv_generalCtrlGPIO_pin_init(unit, general_smi_SDA[dev].devId, general_smi_SDA[dev].pinId, &gpioConfig);

    if (RT_ERR_OK != ret)
    {
        RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error R");
        return ret;
    }

    *pRdata |= rawData & 0xFF;

    ret = _general_smi_writeBit(0x00, 1, dev);              /* ack by Master(CPU) */
    if (RT_ERR_OK != ret)
    {
        RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error 21: Dev[6:0]_0|A|Addr[7:0]|A|Addr[15:8]|A|Dev[6:0]_1|A|DATA[7:0], send Ack fail");
        return ret;
    }

	gpioConfig.direction = GPIO_DIR_IN;
	gpioConfig.default_value = 1;
	gpioConfig.int_gpio.function = GPIO_CTRLFUNC_NORMAL;
	gpioConfig.int_gpio.interruptEnable = GPIO_INT_DISABLE;
	gpioConfig.ext_gpio.debounce = 0;
	gpioConfig.ext_gpio.inverter =0;
	gpioConfig.ext_gpio.direction = GPIO_DIR_IN;	
	ret = drv_generalCtrlGPIO_pin_init(unit, general_smi_SDA[dev].devId, general_smi_SDA[dev].pinId, &gpioConfig);

    if (RT_ERR_OK != ret)
    {
        RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error S");
        return ret;
    }
    ret = _general_smi_readBit(8, &rawData, dev);           /* Read DATA [15: 8] */
    if (RT_ERR_OK != ret)
    {
        RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error 22: Dev[6:0]_0|A|Addr[7:0]|A|Addr[15:8]|A|Dev[6:0]_1|A|DATA[7:0]|A no data read");
        return ret;
    }
        CLK_DURATION(smi_DELAY[dev]);

	gpioConfig.direction = GPIO_DIR_OUT;
	gpioConfig.default_value = 1;
	gpioConfig.int_gpio.function = GPIO_CTRLFUNC_NORMAL;
	gpioConfig.int_gpio.interruptEnable = GPIO_INT_DISABLE;
	gpioConfig.ext_gpio.debounce = 0;
	gpioConfig.ext_gpio.inverter =0;
	gpioConfig.ext_gpio.direction = GPIO_DIR_OUT;	
	ret = drv_generalCtrlGPIO_pin_init(unit, general_smi_SDA[dev].devId, general_smi_SDA[dev].pinId, &gpioConfig);

    if (RT_ERR_OK != ret)
    {
        RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error T");
        return ret;
    }

    *pRdata |= rawData << 8;

    /* Stop SMI */
    ret = _general_smi_stop(dev);
    if (RT_ERR_OK != ret)
    {
        RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error 23: Dev[6:0]_0|A|Addr[7:0]|A|Addr[15:8]|A|Dev[6:0]_1|A|DATA[7:0]|A|DATA[15:8], no stop");
        return RT_ERR_FAILED;
    }

    return ret;
} /* end of _general_16bits_read */


/* Function Name:
 *      _general_16bits_write
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
 *      PD64012
 */
int32 _general_16bits_write(uint32 chipid, uint32 mAddrs, uint32 wData, uint32 dev)
{
    uint32 ack = 0;
    int32  ret = RT_ERR_FAILED;
    uint16 devid = PD64012_Master_DeviceID;
    int8   con = 0;
	uint32 unit = 0;
	drv_generalCtrlGpio_pinConf_t gpioConfig;

    devid = chipid & 0xff;
    RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read:device:%04d, addr:%04d",devid,mAddrs);

    /* Start SMI */
    RT_ERR_CHK(_general_16bits_start(dev), ret);

    /* Start signal for Read */
    RT_ERR_CHK(_general_smi_writeBit(devid, 7, dev), ret);              /* CTRL code: 7'PD64012_MasterAddress */
    RT_ERR_CHK(_general_smi_writeBit(0x0, 1, dev), ret);                /* 0: issue WRITE command */

	gpioConfig.direction = GPIO_DIR_IN;
	gpioConfig.default_value = 1;
	gpioConfig.int_gpio.function = GPIO_CTRLFUNC_NORMAL;
	gpioConfig.int_gpio.interruptEnable = GPIO_INT_DISABLE;
	gpioConfig.ext_gpio.debounce = 0;
	gpioConfig.ext_gpio.inverter =0;
	gpioConfig.ext_gpio.direction = GPIO_DIR_IN;	
    RT_ERR_CHK(drv_generalCtrlGPIO_pin_init(unit, general_smi_SDA[dev].devId, general_smi_SDA[dev].pinId, &gpioConfig), ret);

    do
    {
        con++;
        RT_ERR_CHK(_general_smi_readBit(1, &ack, dev), ret);           /* ack for issuing WRITE command*/
    } while ((0 != ack) && (con < ACK_TIMER));

	gpioConfig.direction = GPIO_DIR_OUT;
	gpioConfig.default_value = 1;
	gpioConfig.int_gpio.function = GPIO_CTRLFUNC_NORMAL;
	gpioConfig.int_gpio.interruptEnable = GPIO_INT_DISABLE;
	gpioConfig.ext_gpio.debounce = 0;
	gpioConfig.ext_gpio.inverter =0;
	gpioConfig.ext_gpio.direction = GPIO_DIR_OUT;
    RT_ERR_CHK(drv_generalCtrlGPIO_pin_init(unit, general_smi_SDA[dev].devId, general_smi_SDA[dev].pinId, &gpioConfig), ret);	

    if (0 != ack)
    {
        return RT_ERR_FAILED;
    }

    RT_ERR_CHK(_general_smi_writeBit((mAddrs & 0xFF), 8, dev), ret);   /* Set reg_addr[7:0] */

	gpioConfig.direction = GPIO_DIR_IN;
	gpioConfig.default_value = 1;
	gpioConfig.int_gpio.function = GPIO_CTRLFUNC_NORMAL;
	gpioConfig.int_gpio.interruptEnable = GPIO_INT_DISABLE;
	gpioConfig.ext_gpio.debounce = 0;
	gpioConfig.ext_gpio.inverter =0;
	gpioConfig.ext_gpio.direction = GPIO_DIR_IN;	
    RT_ERR_CHK(drv_generalCtrlGPIO_pin_init(unit, general_smi_SDA[dev].devId, general_smi_SDA[dev].pinId, &gpioConfig), ret);

    con = 0;
    do
    {
        con++;
        RT_ERR_CHK(_general_smi_readBit(1, &ack, dev), ret);           /* ack for setting reg_addr[7:0] */
    } while ((0 != ack) && (con < ACK_TIMER));
	
	gpioConfig.direction = GPIO_DIR_OUT;
	gpioConfig.default_value = 1;
	gpioConfig.int_gpio.function = GPIO_CTRLFUNC_NORMAL;
	gpioConfig.int_gpio.interruptEnable = GPIO_INT_DISABLE;
	gpioConfig.ext_gpio.debounce = 0;
	gpioConfig.ext_gpio.inverter =0;
	gpioConfig.ext_gpio.direction = GPIO_DIR_OUT;	
    RT_ERR_CHK(drv_generalCtrlGPIO_pin_init(unit, general_smi_SDA[dev].devId, general_smi_SDA[dev].pinId, &gpioConfig), ret);	

    if (0 != ack)
    {
        return RT_ERR_FAILED;
    }

    RT_ERR_CHK(_general_smi_writeBit((mAddrs >> 8), 8, dev), ret);      /* Set reg_addr[15:8] */

	gpioConfig.direction = GPIO_DIR_IN;
	gpioConfig.default_value = 1;
	gpioConfig.int_gpio.function = GPIO_CTRLFUNC_NORMAL;
	gpioConfig.int_gpio.interruptEnable = GPIO_INT_DISABLE;
	gpioConfig.ext_gpio.debounce = 0;
	gpioConfig.ext_gpio.inverter =0;
	gpioConfig.ext_gpio.direction = GPIO_DIR_IN;	
    RT_ERR_CHK(drv_generalCtrlGPIO_pin_init(unit, general_smi_SDA[dev].devId, general_smi_SDA[dev].pinId, &gpioConfig), ret);	

    con = 0;
    do
    {
        con++;
        RT_ERR_CHK(_general_smi_readBit(1, &ack, dev), ret);           /* ack for setting reg_addr[15:8] */
    } while ((0 != ack) && (con < ACK_TIMER));

	gpioConfig.direction = GPIO_DIR_OUT;
	gpioConfig.default_value = 1;
	gpioConfig.int_gpio.function = GPIO_CTRLFUNC_NORMAL;
	gpioConfig.int_gpio.interruptEnable = GPIO_INT_DISABLE;
	gpioConfig.ext_gpio.debounce = 0;
	gpioConfig.ext_gpio.inverter =0;
	gpioConfig.ext_gpio.direction = GPIO_DIR_OUT;	
    RT_ERR_CHK(drv_generalCtrlGPIO_pin_init(unit, general_smi_SDA[dev].devId, general_smi_SDA[dev].pinId, &gpioConfig), ret);

    if (0 != ack)
    {
        return RT_ERR_FAILED;
    }

    RT_ERR_CHK(_general_smi_writeBit((wData & 0xFF), 8, dev), ret);    /* Write Data [7:0] out */
	
	gpioConfig.direction = GPIO_DIR_IN;
	gpioConfig.default_value = 1;
	gpioConfig.int_gpio.function = GPIO_CTRLFUNC_NORMAL;
	gpioConfig.int_gpio.interruptEnable = GPIO_INT_DISABLE;
	gpioConfig.ext_gpio.debounce = 0;
	gpioConfig.ext_gpio.inverter =0;
	gpioConfig.ext_gpio.direction = GPIO_DIR_IN;	
    RT_ERR_CHK(drv_generalCtrlGPIO_pin_init(unit, general_smi_SDA[dev].devId, general_smi_SDA[dev].pinId, &gpioConfig), ret);	

    con = 0;
    do
    {
        con++;
        RT_ERR_CHK(_general_smi_readBit(1, &ack, dev), ret);           /* ack for writting data [7:0] */
    } while ((0 != ack) && (con < ACK_TIMER));

	gpioConfig.direction = GPIO_DIR_OUT;
	gpioConfig.default_value = 1;
	gpioConfig.int_gpio.function = GPIO_CTRLFUNC_NORMAL;
	gpioConfig.int_gpio.interruptEnable = GPIO_INT_DISABLE;
	gpioConfig.ext_gpio.debounce = 0;
	gpioConfig.ext_gpio.inverter =0;
	gpioConfig.ext_gpio.direction = GPIO_DIR_OUT;	
    RT_ERR_CHK(drv_generalCtrlGPIO_pin_init(unit, general_smi_SDA[dev].devId, general_smi_SDA[dev].pinId, &gpioConfig), ret);	

    if (0 != ack)
    {
        return RT_ERR_FAILED;
    }

    RT_ERR_CHK(_general_smi_writeBit((wData >> 8), 8, dev), ret);      /*Write Data [15:8] out */
	gpioConfig.direction = GPIO_DIR_IN;
	gpioConfig.default_value = 1;
	gpioConfig.int_gpio.function = GPIO_CTRLFUNC_NORMAL;
	gpioConfig.int_gpio.interruptEnable = GPIO_INT_DISABLE;
	gpioConfig.ext_gpio.debounce = 0;
	gpioConfig.ext_gpio.inverter =0;
	gpioConfig.ext_gpio.direction = GPIO_DIR_IN;	
    RT_ERR_CHK(drv_generalCtrlGPIO_pin_init(unit, general_smi_SDA[dev].devId, general_smi_SDA[dev].pinId, &gpioConfig), ret);	

    con = 0;
    do
    {
        con++;
        RT_ERR_CHK(_general_smi_readBit(1, &ack, dev), ret);           /* ack for writting data [15:8] */
    } while ((0 != ack) && (con < ACK_TIMER));
	
	gpioConfig.direction = GPIO_DIR_OUT;
	gpioConfig.default_value = 1;
	gpioConfig.int_gpio.function = GPIO_CTRLFUNC_NORMAL;
	gpioConfig.int_gpio.interruptEnable = GPIO_INT_DISABLE;
	gpioConfig.ext_gpio.debounce = 0;
	gpioConfig.ext_gpio.inverter =0;
	gpioConfig.ext_gpio.direction = GPIO_DIR_OUT;	
    RT_ERR_CHK(drv_generalCtrlGPIO_pin_init(unit, general_smi_SDA[dev].devId, general_smi_SDA[dev].pinId, &gpioConfig), ret);

    if (0 != ack)
    {
        return RT_ERR_FAILED;
    }

    /* Stop SMI */
    ret = _general_smi_stop(dev);
    if (RT_ERR_OK != ret)
    {
        return RT_ERR_FAILED;
    }
    return ret;
} /* end of _general_16bits_write */


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
int32 drv_smi_slavePresent(uint32 slave, uint32 dev)
{
    uint32 ack = 0;
    int32  ret = RT_ERR_FAILED;
	uint32 unit = 0;

    /* Start SMI */
    RT_ERR_CHK(_general_8bits_dev_start(dev), ret);
    RT_ERR_CHK(_general_smi_writeBit(slave, 7, dev), ret);
    RT_ERR_CHK(_general_smi_writeBit(0x0, 1, dev), ret);
    RT_ERR_CHK(drv_generalCtrlGPIO_dataBit_set(unit, general_smi_SDA[dev].devId, general_smi_SDA[dev].pinId, 1), ret);

    /*Read ack for issuing WRITE command*/
    RT_ERR_CHK(_general_smi_readBit(1, &ack, dev), ret);

    /* Check ack value, it should be zero. */
    if (0 != ack)
    {
        return RT_ERR_FAILED;
    }

    RT_ERR_CHK(drv_generalCtrlGPIO_dataBit_set(unit, general_smi_SDA[dev].devId, general_smi_SDA[dev].pinId, 1), ret);

    /* Stop SMI */
    RT_ERR_CHK(_general_8bits_dev_stop(dev), ret);

    return ((0 == ack) ? RT_ERR_OK : RT_ERR_FAILED);
} /* end of drv_smi_slavePresent */


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
int32 drv_smi_group_get(uint32 * pDevSCK, uint32 * pPinSCK, uint32 * pDevSDA, uint32 * pPinSDA, uint32 dev)
{
	RT_INIT_CHK(smi_INIT_FLAG[dev]);

    *pDevSCK = GPIO_PORT(general_smi_SCK[dev].devId);
    *pPinSCK = GPIO_PIN(general_smi_SCK[dev].pinId);
    *pDevSDA = GPIO_PORT(general_smi_SDA[dev].devId);
    *pPinSDA = GPIO_PIN(general_smi_SDA[dev].pinId);

    return RT_ERR_OK;
} /* end of drv_smi_group_get */


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
int32 drv_smi_init(uint32 devSCK, uint32 pinSCK, uint32 devSDA, uint32 pinSDA, uint32 dev)
{
	uint32 unit = 0;
//    gpioID gpioId = 0;
    int32 ret = RT_ERR_FAILED;
	drv_generalCtrlGpio_pinConf_t gpioConfig;

    /* parameter check */
//    RT_PARAM_CHK(!GPIO_PORT_CHK(portSCK), RT_ERR_OUT_OF_RANGE);
//    RT_PARAM_CHK((pinSCK & (~GPIO_ID_PIN_MASK)), RT_ERR_INPUT);
//    RT_PARAM_CHK(!GPIO_PORT_CHK(portSDA), RT_ERR_OUT_OF_RANGE);
//    RT_PARAM_CHK((pinSDA & (~GPIO_ID_PIN_MASK)), RT_ERR_INPUT);
    RT_PARAM_CHK(!SMI_DEVICE_CHK(dev), RT_ERR_OUT_OF_RANGE);

    /* Initialize GPIO port 'portSDA', pin 'pinSDA' as SMI SDA */
    smi_TYPE[dev] = SFP_TYPE;
    smi_CHIPID[dev] = SFP_CHIPID;
    smi_DELAY[dev] = DELAY_4000;
	
	gpioConfig.direction = GPIO_DIR_OUT;
	gpioConfig.default_value = 1;
	gpioConfig.int_gpio.function = GPIO_CTRLFUNC_NORMAL;
	gpioConfig.int_gpio.interruptEnable = GPIO_INT_DISABLE;
	gpioConfig.ext_gpio.debounce = 0;
	gpioConfig.ext_gpio.inverter = 0;	
    RT_ERR_CHK(drv_generalCtrlGPIO_pin_init(unit, devSDA, pinSDA, &gpioConfig), ret);	

	general_smi_SDA[dev].devId = devSDA;
	general_smi_SDA[dev].pinId= pinSDA;

    /* Initialize GPIO port 'portSCK', pin 'pinSCK' as SMI SCK */

	gpioConfig.direction = GPIO_DIR_OUT;
	gpioConfig.default_value = 1;
	gpioConfig.int_gpio.function = GPIO_CTRLFUNC_NORMAL;
	gpioConfig.int_gpio.interruptEnable = GPIO_INT_DISABLE;
	gpioConfig.ext_gpio.debounce = 0;
	gpioConfig.ext_gpio.inverter = 0;
	RT_ERR_CHK(drv_generalCtrlGPIO_pin_init(unit, devSCK, pinSCK, &gpioConfig), ret);	

	general_smi_SCK[dev].devId = devSCK;
	general_smi_SCK[dev].pinId= pinSCK;
	
    RT_ERR_CHK(drv_generalCtrlGPIO_dataBit_set(unit, general_smi_SDA[dev].devId, general_smi_SDA[dev].pinId, 1), ret);
	RT_ERR_CHK(drv_generalCtrlGPIO_dataBit_set(unit, general_smi_SCK[dev].devId, general_smi_SCK[dev].pinId, 1), ret);

    /* Initialize SMI SEM */
    SMI_SEM_INIT(dev);
	smi_INIT_FLAG[dev] = INIT_COMPLETED;
    return ret;
} /* end of drv_smi_init */

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
int32 drv_smi_write(uint32 mAddrs, uint32 wData, uint32 dev)
{
    int32 ret = RT_ERR_FAILED;

    /* parameter check */
    RT_PARAM_CHK(!SMI_DEVICE_CHK(dev), RT_ERR_OUT_OF_RANGE);
	RT_INIT_CHK(smi_INIT_FLAG[dev]);

    SMI_SEM_LOCK(dev);

    switch (smi_TYPE[dev])
    {
        case SMI_TYPE_8BITS_DEV:
            /* Start SMI */
            ret = _general_8bits_dev_start(dev);
            if (RT_ERR_OK != ret)
            {
                goto smi_general_write_end;
            }

            ret = _general_8bits_dev_write(mAddrs, wData, dev);

            /* Stop SMI */
            ret = _general_8bits_dev_stop(dev);
            break;
        case SMI_TYPE_16BITS_DEV:
            ret = _general_16bits_write(smi_CHIPID[dev], mAddrs, wData, dev);
            break;			

        default:
            break;
    }

smi_general_write_end:

    SMI_SEM_UNLOCK(dev);

    return ret;
} /* end of drv_smi_write */

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
int32 drv_smi_read(uint32 mAddrs, uint32 *pRdata, uint32 dev)
{
    uint8 value;
    int32 ret = RT_ERR_FAILED;

    /* parameter check */
    RT_PARAM_CHK((NULL == pRdata), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!SMI_DEVICE_CHK(dev), RT_ERR_OUT_OF_RANGE);
	RT_INIT_CHK(smi_INIT_FLAG[dev]);
	
    *pRdata = 0;
    SMI_SEM_LOCK(dev);

    switch (smi_TYPE[dev])
    {
        case SMI_TYPE_8BITS_DEV:
            /* Start SMI */
            ret = _general_8bits_dev_start(dev);
            if (RT_ERR_OK != ret)
            {
                goto smi_general_read_end;
            }

            ret = _general_8bits_dev_read(mAddrs, &value, dev);
            *pRdata = value;
            if (RT_ERR_OK != ret)
            {
                goto smi_general_read_end;
            }

            /* Stop SMI */
            ret = _general_8bits_dev_stop(dev);
            break;
			
		case SMI_TYPE_16BITS_DEV:
				ret = _general_16bits_read(smi_CHIPID[dev], mAddrs, pRdata, dev);
				break;

        default:
            break;
    }

smi_general_read_end:
    SMI_SEM_UNLOCK(dev);

    return ret;
} /* end of drv_smi_read */


/* Function Name:
 *      drv_smi_module_init
 * Description:
 *      SMI init function
 * Input:
 *		unit - unit ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 * Note:
 *      None
 */
int32 drv_smi_module_init(uint32 unit)
{
	int32 i;
	
    for(i = 0; i < SMI_DEVICE_MAX; i++)
		smi_INIT_FLAG[i] = INIT_NOT_COMPLETED;

    return RT_ERR_OK;
} /* end of drv_smi_init */


