/*
 * Copyright (C) 2012 Realtek Semiconductor Corp. 
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated, 
 * modified or distributed under the authorized license from Realtek. 
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER 
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED. 
 *
 * $Revision: 44378 $
 * $Date: 2013-11-12 11:07:06 +0800 (Tue, 12 Nov 2013) $
 *
 * Purpose : Definition those public uart APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *            1) uart init
 *            2) character set & get
 */

/*
 * Include Files
 */
#include <linux/version.h> 
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,19))
#include <bspchip.h>
#else
#include <soc/soc.h>
#endif
#include <soc/type.h>
#include <ioal/mem32.h>
#include <osal/print.h>
#include <osal/time.h>
#include <drv/uart/r8380.h>
#include <drv/swcore/rtl8380.h>
#include <drv/swcore/chip.h>
#include <linux/serial_reg.h>
#include <osal/isr.h>
#include <dev_config.h>

/*
 * Symbol Definition
 */
#define LCR_BKSE 0x80                /* Bank select enable */
#define LCRVAL   LCR_8N1             /* 8 data, 1 stop, no parity */
#define MCRVAL   (MCR_DTR | MCR_RTS) /* RTS/DTR */
#define FCRVAL   0xC1                /* Clear & enable FIFOs */
#define LCR_8N1  0x03
#define MCR_DTR  0x01
#define MCR_RTS  0x02

/* Divisor Latch Value for different baudrate when system clock is 200MHz */
#define BAUDRTAE_9600_DIVISOR_LATCH_VALUE_IN_SYS_200MHZ     0x0515
#define BAUDRTAE_19200_DIVISOR_LATCH_VALUE_IN_SYS_200MHZ    0x028A
#define BAUDRTAE_38400_DIVISOR_LATCH_VALUE_IN_SYS_200MHZ    0x0145
#define BAUDRTAE_57600_DIVISOR_LATCH_VALUE_IN_SYS_200MHZ    0x00D8
#define BAUDRTAE_115200_DIVISOR_LATCH_VALUE_IN_SYS_200MHZ   0x006C

/*
 * Data Type Definition
 */

struct serial_info_s {
	drv_uart_baudrate_t baudrate;
	drv_uart_txcallback_t  getTxChar;
	drv_uart_rxcallback_t  putRxChar;
};

static struct serial_info_s serial_info[RTK_MAX_NUM_OF_UNIT];
/*
 * Data Declaration
 */
extern uint32 uart_chipId[RTK_MAX_NUM_OF_UNIT];

/*
 * Macro Definition
 */

/*
 * Function Declaration
 */

static int32 r8380_serial_intr(void *isr_param);

/* Function Name:
 *      r8380_serial_out
 * Description:
 *      out serial data
 * Input:
 *      unit  - unit id 
 *      value - out data
 * Output:
 *      None
 * Return:
 * Note:
 */
static void
r8380_serial_out(int reg_index, int value)
{
	int offset = reg_index << 2;
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,19))
    REG32(BSP_UART1_BASE+offset) = (uint32)(((uint32)(value)) << 24);
#else
	REG32(UART1_BASE+offset) = (uint32)(((uint32)(value)) << 24);
#endif
	return;
}

/* Function Name:
 *      r8380_serial_in
 * Description:
 *      out serial data
 * Input:
 *      unit  - unit id 
 * Output:
 * Return:
  *      serial input data
 * Note:
 */
static uint8
r8380_serial_in(int reg_index)
{
	uint8 value;
	int offset = (reg_index << 2);
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,19))
    value = (uint8)(REG8(BSP_UART1_BASE+offset));
#else
	value = (uint8)(REG8(UART1_BASE+offset));
#endif
	return value;
}
/* Function Name:
 *      r8380_uart_init
 * Description:
 *      Init the uart module of the specified device.
 * Input:
 *      unit - unit id 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      Initialize the uart1 interface.
 */
int32
r8380_uart_init(uint32 unit,drv_uart_rxcallback_t rxcallback,drv_uart_txcallback_t txcallback)
{
    /* GMII interface select to uart-1, default is SPI slave */
    ioal_mem32_write(unit, RTL8380_GMII_INTF_SEL_ADDR, 0x10);

    /* uart-1 initialize code */
    r8380_uart_baudrate_set(unit, UART_BAUDRATE_19200);

    serial_info[unit].putRxChar = rxcallback;
    serial_info[unit].getTxChar = txcallback;

		while ((r8380_serial_in(UART_LSR) & UART_LSR_DR) != 0)
		{
			uint8 Data;
			Data = r8380_serial_in(UART_RX);
		}
		
 	  osal_isr_register(RTK_DEV_UART1, r8380_serial_intr, (void *)unit);
 	  //r8380_serial_out(UART_IER, UART_IER_RDI);
    return RT_ERR_OK;
} /* end of r8380_uart_init */

/* Function Name:
 *      r8380_uart_tstc
 * Description:
 *      test if serial data in
 * Input:
 *      unit  - unit id 
 * Output:
 *      
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 */
int32
r8380_uart_tstc(uint32 unit)
{
	if (r8380_serial_in(UART_LSR) & UART_LSR_DR)
		return RT_ERR_OK;
	else
		return RT_ERR_FAILED;
}

/* Function Name:
 *      r8380_uart_getc
 * Description:
 *      Get the character from uart interface 
 * Input:
 *      unit    - unit id
 * Output:
 *      pData   - pointer buffer of character from uart interface
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32
r8380_uart_getc(uint32 unit, uint8 *pData)
{
		if (r8380_serial_in(UART_LSR) & UART_LSR_DR)
    {
    	(*pData) = r8380_serial_in(UART_RX);
            return RT_ERR_OK;
        }
		else
		{
			return RT_ERR_FAILED;
    }
} /* end of r8380_uart_getc */

/* Function Name:
 *      r8380_uart_putc
 * Description:
 *      Output the character to uart interface in the specified device
 * Input:
 *      unit - unit id
 * Output:
 *      data - output the character to uart interface
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32
r8380_uart_putc(uint32 unit, uint8 data)
{
  while ((r8380_serial_in(UART_LSR) & UART_LSR_THRE) == 0);

    r8380_serial_out(UART_TX,data);
    return RT_ERR_OK;
} /* end of r8380_uart_putc */

/* Function Name:
 *      r8380_uart_baudrate_get
 * Description:
 *      Get the baudrate of the uart interface in the specified device
 * Input:
 *      unit      - unit id
 * Output:
 *      pBaudrate - pointer buffer of baudrate value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32
r8380_uart_baudrate_get(uint32 unit, drv_uart_baudrate_t *pBaudrate)
{
    (*pBaudrate) = serial_info[unit].baudrate;
    return RT_ERR_OK;
} /* end of r8380_uart_baudrate_get */

/* Function Name:
 *      r8380_uart_baudrate_set
 * Description:
 *      Configure the baudrate of the uart interface in the specified device
 * Input:
 *      unit     - unit id
 *      baudrate - baudrate value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32
r8380_uart_baudrate_set(uint32 unit, drv_uart_baudrate_t baudrate)
{
    uint32  divisor;

    RT_PARAM_CHK(baudrate >= UART_BAUDRATE_END, RT_ERR_INPUT);

    switch (baudrate)
    {
        case UART_BAUDRATE_9600:
            divisor = BAUDRTAE_9600_DIVISOR_LATCH_VALUE_IN_SYS_200MHZ;
            break;
        case UART_BAUDRATE_19200:
            divisor = BAUDRTAE_19200_DIVISOR_LATCH_VALUE_IN_SYS_200MHZ;
            break;
        case UART_BAUDRATE_38400:
            divisor = BAUDRTAE_38400_DIVISOR_LATCH_VALUE_IN_SYS_200MHZ;
            break;
        case UART_BAUDRATE_57600:
            divisor = BAUDRTAE_57600_DIVISOR_LATCH_VALUE_IN_SYS_200MHZ;
            break;
        case UART_BAUDRATE_115200:
            divisor = BAUDRTAE_115200_DIVISOR_LATCH_VALUE_IN_SYS_200MHZ;
            break;
        case UART_BAUDRATE_END:
        default:
            return RT_ERR_FAILED;
    }
    
  r8380_serial_out(UART_IER,0);
  r8380_serial_out(UART_LCR,(UART_LCR_DLAB | UART_LCR_WLEN8));
  r8380_serial_out(UART_DLL, 0);
	r8380_serial_out(UART_DLM, 0);
	r8380_serial_out(UART_DLL, (divisor& 0xff));
	r8380_serial_out(UART_DLM, (divisor >> 8));
	r8380_serial_out(UART_LCR,UART_LCR_WLEN8);
	r8380_serial_out(UART_FCR,FCRVAL);
	r8380_serial_out(UART_MCR,(MCRVAL|UART_MCR_TCRTLR));
  serial_info[unit].baudrate = baudrate;
  osal_time_sleep(1);
    return RT_ERR_OK;
} /* end of r8380_uart_baudrate_set */

/* Function Name:
 *      r8380_serial_starttx
 * Description:
 *      trigger hw to start tx.
 * Input:
 *      unit  - unit id 
 * Output:
 *      
 * Return:
 * Note:
 */
void   r8380_serial_starttx(uint32 unit)
{
	#if 0
  uint8 ier;
	ier = r8380_serial_in(UART_IER);
	ier |= UART_IER_THRI;
	r8380_serial_out(UART_IER, ier);
	#else
	r8380_serial_out(UART_IER, (UART_IER_RDI|UART_IER_THRI) );
	#endif
	return;
}

/* Function Name:
 *      r8380_serial_clearfifo
 * Description:
 *      empty hw rx/tx fifo.
 * Input:
 *      unit  - unit id 
 * Output:
 *      
 * Return:
 * Note:
 *      for sync io, poe should call this function before exchange message
 */
void r8380_serial_clearfifo(uint32 unit)
{
	uint8 data;

	r8380_serial_out(UART_IER, 0);
	while ((r8380_serial_in(UART_LSR) & UART_LSR_DR) != 0)
	{
		data = r8380_serial_in(UART_RX);
		//printk("%.2x ",data);
	}
	r8380_serial_out(UART_FCR,FCRVAL);
  //r8380_serial_out(UART_IER, UART_IER_RDI);
	return;
}

/* Function Name:
 *      r8380_serial_intr
 * Description:
 *      serial isr
 * Input:
 *      isr_param  - unit id 
 * Output:
 *      
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      POE uart message packet size is 15, for speed up the process,
 *      isr read/write 15 chars.
 */
static int32 r8380_serial_intr(void *isr_param)
{
	uint32 unit,cnt;
	uint8 lsr, iir,ier,data;
	
	ier = r8380_serial_in(UART_IER);
	r8380_serial_out(UART_IER, 0);
	
	unit = (uint32)isr_param;
	iir = r8380_serial_in(UART_IIR);
  //printk("\n&&&&***** UART ISR LSR %.2x IIR %.2x*******&&&&&&&&&&&&&\n",lsr,iir); 
	cnt = 0;
  while (!(iir & UART_IIR_NO_INT) && cnt < 15) 
  {  
  	lsr = r8380_serial_in(UART_LSR);
	  if (lsr & UART_LSR_DR)
	  {
	  	data = r8380_serial_in(UART_RX);
			serial_info[unit].putRxChar(unit,data);
		}
		if (lsr & UART_LSR_THRE)
		{
  		if (serial_info[unit].getTxChar(unit,&data) != RT_ERR_OK)
  		{
  			ier &= ~UART_IER_THRI;
  		}
  		else
  			r8380_serial_out(UART_TX, data);
		}
		iir = r8380_serial_in(UART_IIR);
		cnt++;
	}
	r8380_serial_out(UART_IER, ier);	
	return RT_ERR_OK;
}
