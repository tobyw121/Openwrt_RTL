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
 * $Revision: 30055 $
 * $Date: 2012-06-19 15:33:08 +0800 (Tue, 19 Jun 2012) $
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
#include <drv/gpio/ext_gpio.h>
#include <drv/smi/smi.h>
#include <drv/smi/ext_smi.h>

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
typedef struct drv_extSmi_devConfEntry_s
{
    uint32  gpioIdSCK;
    uint32  gpioIdSDA;
    uint32  phyAddrSCK;
    uint32  phyAddrSDA;
    drv_extGpio_conf_t extGpioConf;
    uint32  valid;
} drv_extSmi_devConfEntry_t;

static drv_extSmi_devConfEntry_t extSmiConfEntry[RTK_MAX_NUM_OF_UNIT][SMI_DEVICE_MAX];

static gpioID ext_smi_TYPE[RTK_MAX_NUM_OF_UNIT][SMI_DEVICE_MAX];        /* SMI waveform function */
static gpioID ext_smi_CHIPID[RTK_MAX_NUM_OF_UNIT][SMI_DEVICE_MAX];      /* CHIP ID for PD64012 type device */
static gpioID ext_smi_DELAY[RTK_MAX_NUM_OF_UNIT][SMI_DEVICE_MAX];       /* CHIP ID for PD64012 type device */
static osal_mutex_t ext_smi_SEM[SMI_DEVICE_MAX];   /* EXT-SMI semaphore */

/*
 * Macro Declaration
 */
/* loop 300,000 time for 10ms */
#define CLK_DURATION(clk)               do { int i; for (i = 0; i < clk; i++); } while(0)

#define EXT_SMI_SEM_INIT(dev)\
do {\
    if (0 == (ext_smi_SEM[dev] = osal_sem_mutex_create()))\
    {\
        RT_LOG(LOG_DEBUG, MOD_GENERAL, "SMI semaphore create failed");\
        return RT_ERR_FAILED;\
    }\
} while(0)

#define EXT_SMI_SEM_LOCK(dev)\
do {\
    if (osal_sem_mutex_take(ext_smi_SEM[dev], OSAL_SEM_WAIT_FOREVER) != RT_ERR_OK)\
    {\
        RT_LOG(LOG_DEBUG, MOD_GENERAL, "SMI semaphore lock failed");\
        return RT_ERR_FAILED;\
    }\
} while(0)

#define EXT_SMI_SEM_UNLOCK(dev)\
do {\
    if (osal_sem_mutex_give(ext_smi_SEM[dev]) != RT_ERR_OK)\
    {\
        RT_LOG(LOG_DEBUG, MOD_GENERAL, "SMI semaphore unlock failed");\
        return RT_ERR_FAILED;\
    }\
} while(0)

#define IS_EXTSMI_UNIT_PERIPH_INVALID(unit, periph)  ((extSmiConfEntry[unit][periph].valid) ? 0 : 1)

/*
 * Function Declaration
 */
int32 _extSmi_start(uint32 unit, uint32 periferal)
{
    int32 ret = RT_ERR_FAILED;
    uint32  gpioIdSCK = extSmiConfEntry[unit][periferal].gpioIdSCK;
    uint32  gpioIdSDA = extSmiConfEntry[unit][periferal].gpioIdSDA;
    uint32  phyAddrSCK = extSmiConfEntry[unit][periferal].phyAddrSCK;
    uint32  phyAddrSDA = extSmiConfEntry[unit][periferal].phyAddrSDA;
    drv_extGpio_conf_t extGpioConf;

    extGpioConf.direction = GPIO_DIR_OUT;
    extGpioConf.debounce = 0;
    extGpioConf.inverter = 0;

    /* change GPIO pin to Output only */
    RT_ERR_CHK(drv_extGpio_pin_init(unit, phyAddrSCK, gpioIdSCK, &extGpioConf), ret);
    RT_ERR_CHK(drv_extGpio_pin_init(unit, phyAddrSDA, gpioIdSDA, &extGpioConf), ret);

    /* Initial state: SCK: 0, SDA: 1 */
    RT_ERR_CHK(drv_extGpio_dataBit_set(unit, phyAddrSCK, gpioIdSCK, 0), ret);
    RT_ERR_CHK(drv_extGpio_dataBit_set(unit, phyAddrSDA, gpioIdSDA, 1), ret);

    /* CLK 1: 0 -> 1, 1 -> 0 */
    CLK_DURATION(ext_smi_DELAY[unit][periferal]);
    RT_ERR_CHK(drv_extGpio_dataBit_set(unit, phyAddrSCK, gpioIdSCK, 1), ret);

    CLK_DURATION(ext_smi_DELAY[unit][periferal]);
    RT_ERR_CHK(drv_extGpio_dataBit_set(unit, phyAddrSCK, gpioIdSCK, 0), ret);

    /* CLK 2: */
    CLK_DURATION(ext_smi_DELAY[unit][periferal]);
    RT_ERR_CHK(drv_extGpio_dataBit_set(unit, phyAddrSCK, gpioIdSCK, 1), ret);

    CLK_DURATION(ext_smi_DELAY[unit][periferal]);
    RT_ERR_CHK(drv_extGpio_dataBit_set(unit, phyAddrSDA, gpioIdSDA, 0), ret);

    CLK_DURATION(ext_smi_DELAY[unit][periferal]);
    RT_ERR_CHK(drv_extGpio_dataBit_set(unit, phyAddrSCK, gpioIdSCK, 0), ret);

    CLK_DURATION(ext_smi_DELAY[unit][periferal]);
    RT_ERR_CHK(drv_extGpio_dataBit_set(unit, phyAddrSDA, gpioIdSDA, 1), ret);

    return ret;
}

static int32 _extSmi_readBit(uint32 unit, uint32 periferal, uint32 bitLen, uint32 *pRdata)
{
    uint32  data = 0;
    uint32  delay = ext_smi_DELAY[unit][periferal];
    int32   ret = RT_ERR_FAILED;
    uint32  gpioIdSCK = extSmiConfEntry[unit][periferal].gpioIdSCK;
    uint32  gpioIdSDA = extSmiConfEntry[unit][periferal].gpioIdSDA;
    uint32  phyAddrSCK = extSmiConfEntry[unit][periferal].phyAddrSCK;
    uint32  phyAddrSDA = extSmiConfEntry[unit][periferal].phyAddrSDA;
    drv_extGpio_conf_t extGpioConf;

    extGpioConf.direction = GPIO_DIR_IN;
    extGpioConf.debounce = 0;
    extGpioConf.inverter = 0;


    /* change GPIO pin to Input only */
    RT_ERR_CHK(drv_extGpio_pin_init(unit, phyAddrSDA, gpioIdSDA, &extGpioConf), ret);

    for (*pRdata = 0; bitLen > 0; bitLen--)
    {
        CLK_DURATION(delay);
        CLK_DURATION(delay);

        /* clocking */
        RT_ERR_CHK(drv_extGpio_dataBit_set(unit, phyAddrSCK, gpioIdSCK, 1), ret);

        CLK_DURATION(delay);

        RT_ERR_CHK(drv_extGpio_dataBit_get(unit, phyAddrSDA, gpioIdSDA, &data), ret);
        CLK_DURATION(delay);

        RT_ERR_CHK(drv_extGpio_dataBit_set(unit, phyAddrSCK, gpioIdSCK, 0), ret);

        *pRdata |= (data << (bitLen - 1));
    }

    /* change GPIO pin to Output only */
    extGpioConf.direction = GPIO_DIR_OUT;
    RT_ERR_CHK(drv_extGpio_pin_init(unit, phyAddrSDA, gpioIdSDA, &extGpioConf), ret);

    return ret;
}

static int32 _extSmi_writeBit(uint32 unit, uint32 periferal, uint16 signal, uint32 bitLen)
{
    int32   ret = RT_ERR_FAILED;
    uint32  delay = ext_smi_DELAY[unit][periferal];
    uint32  gpioIdSCK = extSmiConfEntry[unit][periferal].gpioIdSCK;
    uint32  gpioIdSDA = extSmiConfEntry[unit][periferal].gpioIdSDA;
    uint32  phyAddrSCK = extSmiConfEntry[unit][periferal].phyAddrSCK;
    uint32  phyAddrSDA = extSmiConfEntry[unit][periferal].phyAddrSDA;

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
            RT_ERR_CHK(drv_extGpio_dataBit_set(unit, phyAddrSDA, gpioIdSDA, 1), ret);
        }
        else
        {
            RT_ERR_CHK(drv_extGpio_dataBit_set(unit, phyAddrSDA, gpioIdSDA, 0), ret);
        }

        CLK_DURATION(delay);

        /* clocking */
        RT_ERR_CHK(drv_extGpio_dataBit_set(unit, phyAddrSCK, gpioIdSCK, 1), ret);
        CLK_DURATION(delay);
        CLK_DURATION(delay);
        RT_ERR_CHK(drv_extGpio_dataBit_set(unit, phyAddrSCK, gpioIdSCK, 0), ret);
    }

    return ret;
}

int32 _extSmi_write(uint32 unit, uint32 periferal, uint32 mAddrs, uint32 wData)
{
    uint32 ack = 0;
    uint32 slave = 0;
    int32  ret = RT_ERR_FAILED;
    int8   con = 0;

    slave = ext_smi_CHIPID[unit][periferal] & 0xff;
    RT_DBG(LOG_DEBUG, MOD_GENERAL, "[Read]devid 0x%x addr 0x%x ack 0x%x", slave, mAddrs, ack);

    RT_ERR_CHK(_extSmi_writeBit(unit, periferal, slave, 7), ret);           /* CTRL code: 7'bslave_ADDR */
    RT_ERR_CHK(_extSmi_writeBit(unit, periferal, 0x0, 1), ret);             /* 0: issue WRITE command */

    do
    {
        con++;
        RT_ERR_CHK(_extSmi_readBit(unit, periferal, 1, &ack), ret);         /* ack for issuing WRITE command*/
    } while ((0 != ack) && (con < ACK_TIMER));

    if (0 != ack)
    {
        return RT_ERR_FAILED;
    }

    RT_ERR_CHK(_extSmi_writeBit(unit, periferal, (mAddrs & 0xFF), 8), ret); /* Set reg_addr[7:0] */

    con = 0;
    do
    {
        con++;
        RT_ERR_CHK(_extSmi_readBit(unit, periferal, 1, &ack), ret);         /* ack for setting reg_addr[7:0] */
    } while ((0 != ack) && (con < ACK_TIMER));

    if (0 != ack)
    {
        return RT_ERR_FAILED;
    }

    RT_ERR_CHK(_extSmi_writeBit(unit, periferal, (mAddrs >> 8), 8), ret);   /* Set reg_addr[15:8] */

    con = 0;
    do
    {
        con++;
        RT_ERR_CHK(_extSmi_readBit(unit, periferal, 1, &ack), ret);         /* ack for setting reg_addr[15:8] */
    } while ((0 != ack) && (con < ACK_TIMER));

    if (0 != ack)
    {
        return RT_ERR_FAILED;
    }

    RT_ERR_CHK(_extSmi_writeBit(unit, periferal, (wData & 0xFF), 8), ret);  /* Write Data [7:0] out */

    con = 0;
    do
    {
        con++;
        RT_ERR_CHK(_extSmi_readBit(unit, periferal, 1, &ack), ret);         /* ack for writting data [7:0] */
    } while ((0 != ack) && (con < ACK_TIMER));

    if (0 != ack)
    {
        return RT_ERR_FAILED;
    }

    RT_ERR_CHK(_extSmi_writeBit(unit, periferal, (wData >> 8), 8), ret);    /* Write Data [15:8] out */

    con = 0;

    do
    {
        con++;
        RT_ERR_CHK(_extSmi_readBit(unit, periferal, 1, &ack), ret);          /* ack for writting data [15:8] */
    } while ((0 != ack) && (con < ACK_TIMER));

    if (0 != ack)
    {
        return RT_ERR_FAILED;
    }

    return ret;
}

static int32 _si3452_ext_start(uint32 unit, uint32 periferal)
{
    int32   ret = RT_ERR_FAILED;
    drv_extGpio_conf_t extGpioConf;
    uint32  gpioIdSCK = extSmiConfEntry[unit][periferal].gpioIdSCK;
    uint32  gpioIdSDA = extSmiConfEntry[unit][periferal].gpioIdSDA;
    uint32  phyAddrSCK = extSmiConfEntry[unit][periferal].phyAddrSCK;
    uint32  phyAddrSDA = extSmiConfEntry[unit][periferal].phyAddrSDA;


    extGpioConf.direction = GPIO_DIR_OUT;
    extGpioConf.debounce = 0;
    extGpioConf.inverter = 0;

    /* change GPIO pin to Output only */
    RT_ERR_CHK(drv_extGpio_pin_init(unit, phyAddrSCK, gpioIdSCK, &extGpioConf), ret);
    RT_ERR_CHK(drv_extGpio_pin_init(unit, phyAddrSDA, gpioIdSDA, &extGpioConf), ret);
    RT_ERR_CHK(drv_extGpio_dataBit_set(unit, phyAddrSCK, gpioIdSCK, 0), ret);

    CLK_DURATION(ext_smi_DELAY[unit][periferal]);
    RT_ERR_CHK(drv_extGpio_dataBit_set(unit, phyAddrSCK, gpioIdSCK, 1), ret);

    CLK_DURATION(ext_smi_DELAY[unit][periferal]);
    RT_ERR_CHK(drv_extGpio_dataBit_set(unit, phyAddrSDA, gpioIdSDA, 0), ret);

    CLK_DURATION(ext_smi_DELAY[unit][periferal]);
    RT_ERR_CHK(drv_extGpio_dataBit_set(unit, phyAddrSCK, gpioIdSCK, 0), ret);

    return ret;
}

static int32 _si3452_ext_read(uint32 unit, uint32 periferal, uint32 mAddrs, uint32 *pRdata)
{
    uint32  rawData = 0;
    uint32  ack = 0;
    uint32  slave = 0;
    int32   ret = RT_ERR_FAILED;
    uint32  gpioIdSDA = extSmiConfEntry[unit][periferal].gpioIdSDA;
    uint32  phyAddrSDA = extSmiConfEntry[unit][periferal].phyAddrSDA;


    /*Initial data memory*/
    slave = ext_smi_CHIPID[unit][periferal] & 0xff;
    RT_DBG(LOG_DEBUG, MOD_GENERAL, "[Read]devid 0x%x addr 0x%x ack 0x%x", slave, mAddrs, ack);
    mAddrs = mAddrs & 0xffff;
    *pRdata = 0;

    /* CTRL code: 7'slave adress */
    RT_ERR_CHK(_extSmi_writeBit(unit, periferal, slave, 7), ret);

    /* 0: issue WRITE command */
    RT_ERR_CHK(_extSmi_writeBit(unit, periferal, 0x0, 1), ret);

    RT_ERR_CHK(drv_extGpio_dataBit_set(unit, phyAddrSDA, gpioIdSDA, 1), ret);
    /* Read ack for issuing WRITE command*/
    RT_ERR_CHK(_extSmi_readBit(unit, periferal, 1, &ack), ret);

    /* Check ack value, it should be zero. */
    if (0 != ack)
    {
        RT_LOG(LOG_DEBUG, MOD_GENERAL, "[Read]devid 0x%x addr 0x%x ack 0x%x\n", slave, mAddrs, ack);
        RT_LOG(LOG_DEBUG, MOD_GENERAL, "[si3452_smi_read] Read first ack faild\n");
        RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error 1:,Send:Dev[6:0]_0, no Ack");
        return RT_ERR_FAILED;
    }

    /* Set data pin to high */
    //RT_ERR_CHK(drv_extGpio_dataBit_set(unit, phyAddrSDA, gpioIdSDA, 1), ret);

    /* Note: 8-bit Reg Address */
    RT_ERR_CHK(_extSmi_writeBit(unit, periferal, (mAddrs & 0xFF), 8), ret);

    RT_ERR_CHK(drv_extGpio_dataBit_set(unit, phyAddrSDA, gpioIdSDA, 1), ret);
    /* Read ack */
    RT_ERR_CHK(_extSmi_readBit(unit, periferal, 1, &ack), ret);

    /* Check ack value, it should be zero. */
    if (0 != ack)
    {
        RT_LOG(LOG_DEBUG, MOD_GENERAL, "[Read]devid 0x%x addr 0x%x ack 0x%x\n", slave, mAddrs, ack);
        RT_LOG(LOG_DEBUG, MOD_GENERAL, "[si3452_smi_read] Read first ack faild\n");
        RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error 1:,Send:Dev[6:0]_0, no Ack");
        return RT_ERR_FAILED;
    }

#if 0
    RT_ERR_CHK(_extSmi_writeBit(unit, periferal, (mAddrs >> 8), 8), ret);
    
    RT_ERR_CHK(drv_extGpio_dataBit_set(unit, phyAddrSDA, gpioIdSDA, 1), ret);
    /* Read ack */
    RT_ERR_CHK(_extSmi_readBit(unit, periferal, 1, &ack), ret);

    /* Check ack value, it should be zero. */
    if (0 != ack)
    {
        RT_LOG(LOG_DEBUG, MOD_GENERAL, "[Read]devid 0x%x addr 0x%x ack 0x%x\n", slave, mAddrs, ack);
        RT_LOG(LOG_DEBUG, MOD_GENERAL, "[si3452_smi_read] Read first ack faild\n");
        RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error 1:,Send:Dev[6:0]_0, no Ack");
        return RT_ERR_FAILED;
    }
#endif

    RT_ERR_CHK(drv_extGpio_dataBit_set(unit, phyAddrSDA, gpioIdSDA, 1), ret);
    _si3452_ext_start(unit, periferal);
    
    /* Device Address & Read */
    RT_ERR_CHK(_extSmi_writeBit(unit, periferal, slave, 7), ret);

    /* 1: issue READ command */
    RT_ERR_CHK(_extSmi_writeBit(unit, periferal, 0x1, 1), ret);

    /* Read ack for issuing READ command*/
    RT_ERR_CHK(_extSmi_readBit(unit, periferal, 1, &ack), ret);

    /* Check ack value, it should be zero. */
    if (ack != 0)
    {
        RT_LOG(LOG_DEBUG, MOD_GENERAL, "[Read]devid 0x%x addr 0x%x ack 0x%x\n", slave, mAddrs, ack);
        RT_LOG(LOG_DEBUG, MOD_GENERAL, "[si3452_smi_read] Read third ack faild\n");
        RT_DBG(LOG_DEBUG, MOD_GENERAL, "PoE Read Error 3:Dev[6:0]_0|A|Addr[7:0]|A|Dev[6:0], no Ack");
        return RT_ERR_FAILED;
    }

    /* Read 1 bytes data, Read DATA [7:0] */
    RT_ERR_CHK(_extSmi_readBit(unit, periferal, 8, &rawData), ret);

    /* nack by CPU */
    RT_ERR_CHK(_extSmi_writeBit(unit, periferal, 0x01, 1), ret);

    *pRdata = rawData & 0xFF;
    

    return ret;
} /* end of _si3452_smi_read */

static int32 _si3452_ext_write(uint32 unit, uint32 periferal, uint32 mAddrs, uint32 wData)
{
    uint32 ack = 0;
    uint32 slave = 0;
    int32  ret = RT_ERR_FAILED;
    uint32  gpioIdSDA = extSmiConfEntry[unit][periferal].gpioIdSDA;
    uint32  phyAddrSDA = extSmiConfEntry[unit][periferal].phyAddrSDA;

    /*Initial data memory*/
    slave = ext_smi_CHIPID[unit][periferal] & 0xff;
    RT_DBG(LOG_DEBUG, MOD_GENERAL, "Write add: %02X ", slave);
    mAddrs = mAddrs & 0xff;

    /* CTRL code: 7'slave adress */
    RT_ERR_CHK(_extSmi_writeBit(unit, periferal, slave, 7), ret);

    /* 0: issue WRITE command */
    RT_ERR_CHK(_extSmi_writeBit(unit, periferal, 0x0, 1), ret);

    /* Set data pin to high */
    RT_ERR_CHK(drv_extGpio_dataBit_set(unit, phyAddrSDA, gpioIdSDA, 1), ret);

    /* Read ack for issuing WRITE command*/
    RT_ERR_CHK(_extSmi_readBit(unit, periferal, 1, &ack), ret);

    /* Check ack value, it should be zero. */
    if (0 != ack)
    {
        RT_LOG(LOG_DEBUG, MOD_GENERAL, "[Write]devid 0x%x addr 0x%x ack 0x%x\n", slave, mAddrs, ack);
        RT_LOG(LOG_DEBUG, MOD_GENERAL, "[si3452_smi_write] Read first ack faild\n");
        return RT_ERR_FAILED;
    }

    RT_ERR_CHK(drv_extGpio_dataBit_set(unit, phyAddrSDA, gpioIdSDA, 1), ret);

    /* Set reg_addr[7:0] */
    RT_ERR_CHK(_extSmi_writeBit(unit, periferal, (mAddrs & 0xFF), 8), ret);

    /* Read ack for issuing WRITE command*/
    RT_ERR_CHK(_extSmi_readBit(unit, periferal, 1, &ack), ret);

    /* Check ack value, it should be zero. */
    if (0 != ack)
    {
        RT_LOG(LOG_DEBUG, MOD_GENERAL, "[Write]devid 0x%x addr 0x%x ack 0x%x\n", slave, mAddrs, ack);
        RT_LOG(LOG_DEBUG, MOD_GENERAL, "[si3452_smi_write] Read second ack faild\n");
        return RT_ERR_FAILED;
    }


    RT_ERR_CHK(drv_extGpio_dataBit_set(unit, phyAddrSDA, gpioIdSDA, 1), ret);

    /* Write Data [7:0] out */
    RT_ERR_CHK(_extSmi_writeBit(unit, periferal, (wData & 0xFF), 8), ret);

    /* Read ack for issuing WRITE command*/
    RT_ERR_CHK(_extSmi_readBit(unit, periferal, 1, &ack), ret);

    /* Check ack value, it should be zero. */
    if (0 != ack)
    {
        RT_LOG(LOG_DEBUG, MOD_GENERAL, "[Write]devid 0x%x addr 0x%x ack 0x%x\n", slave, mAddrs, ack);
        RT_LOG(LOG_DEBUG, MOD_GENERAL, "[si3452_smi_write] Read third ack faild\n");
        return RT_ERR_FAILED;
    }

    RT_ERR_CHK(drv_extGpio_dataBit_set(unit, phyAddrSDA, gpioIdSDA, 1), ret);
    return ret;
} /* end of si3425_smi_write */

static int32 _si3452_ext_stop(uint32 unit, uint32 periferal)
{
    int32   ret = RT_ERR_FAILED;
    uint32  gpioIdSCK = extSmiConfEntry[unit][periferal].gpioIdSCK;
    uint32  gpioIdSDA = extSmiConfEntry[unit][periferal].gpioIdSDA;
    uint32  phyAddrSCK = extSmiConfEntry[unit][periferal].phyAddrSCK;
    uint32  phyAddrSDA = extSmiConfEntry[unit][periferal].phyAddrSDA;
    drv_extGpio_conf_t extGpioConf;


    CLK_DURATION(ext_smi_DELAY[unit][periferal]);
    RT_ERR_CHK(drv_extGpio_dataBit_set(unit, phyAddrSDA, gpioIdSDA, 0), ret);
    RT_ERR_CHK(drv_extGpio_dataBit_set(unit, phyAddrSCK, gpioIdSCK, 1), ret);

    CLK_DURATION(ext_smi_DELAY[unit][periferal]);
    RT_ERR_CHK(drv_extGpio_dataBit_set(unit, phyAddrSDA, gpioIdSDA, 1), ret);

    CLK_DURATION(ext_smi_DELAY[unit][periferal]);
    RT_ERR_CHK(drv_extGpio_dataBit_set(unit, phyAddrSCK, gpioIdSCK, 0), ret);

    /* change GPIO pin to Output only */
    extGpioConf.direction = GPIO_DIR_IN;
    extGpioConf.debounce = 0;
    extGpioConf.inverter = 0;
    RT_ERR_CHK(drv_extGpio_pin_init(unit, phyAddrSDA, gpioIdSDA, &extGpioConf), ret);
    RT_ERR_CHK(drv_extGpio_pin_init(unit, phyAddrSCK, gpioIdSCK, &extGpioConf), ret);

    return ret;
}

/* Function Name:
 *      drv_extSmi_read
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
int32 drv_extSmi_read(uint32 unit, uint32 periferal, uint32 mAddrs, uint32 *pRdata)
{
    int32 ret = RT_ERR_OK; 
  
    return ret;

    /* parameter check */
    RT_PARAM_CHK((NULL == pRdata), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(IS_EXTSMI_UNIT_PERIPH_INVALID(unit, periferal), RT_ERR_NOT_INIT);
    RT_PARAM_CHK(!SMI_DEVICE_CHK(periferal), RT_ERR_OUT_OF_RANGE);
    *pRdata = 0;
    
    EXT_SMI_SEM_LOCK(periferal);


    switch (ext_smi_TYPE[unit][periferal])
    {
        case SMI_TYPE_8BITS_DEV:
            /* Start SMI */
            ret = _si3452_ext_start(unit, periferal);
            if (RT_ERR_OK != ret)
            {
                goto extSmi_read_end;
            }

            ret = _si3452_ext_read(unit, periferal, mAddrs, pRdata);
            if (RT_ERR_OK != ret)
            {
                goto extSmi_read_end;
            }

            /* Stop SMI */
            ret = _si3452_ext_stop(unit, periferal);
            break;

        default:
            break;
    }

extSmi_read_end:
    EXT_SMI_SEM_UNLOCK(periferal);

}

int32 drv_extSmi_write(uint32 unit, uint32 periferal, uint32 mAddrs, uint32 wData)
{
    int32 ret = RT_ERR_OK; 
  
    return ret;
    /* parameter check */    
    RT_PARAM_CHK(!SMI_DEVICE_CHK(periferal), RT_ERR_OUT_OF_RANGE);

    EXT_SMI_SEM_LOCK(periferal);
    
    switch (ext_smi_TYPE[unit][periferal])
    {
        case SMI_TYPE_8BITS_DEV:
            /* Start SMI */
            ret = _si3452_ext_start(unit, periferal);
            if (RT_ERR_OK != ret)
            {
                goto extSmi_write_end;
            }

            ret = _si3452_ext_write(unit, periferal, mAddrs, wData);

            /* Stop SMI */
            ret = _si3452_ext_stop(unit, periferal);
            break;

        default:
            break;
    }

extSmi_write_end:

    EXT_SMI_SEM_UNLOCK(periferal);
}

int32 drv_extSmi_dev_init(uint32 unit, uint32 periferal, uint32 phyAddrSCK, uint32 phyAddrSDA, uint32 gpioIdSCK, uint32 gpioIdSDA)
{
    int32 ret = RT_ERR_OK; 
  
    return ret;

}

