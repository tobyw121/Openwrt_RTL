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
 * $Revision: 44561 $
 * $Date: 2013-11-19 11:43:12 +0800 (Tue, 19 Nov 2013) $
 *
 * Purpose : Implementation of the uart driver
 *
 * Feature : uart driver
 *
 */

/*  
 * Include Files 
 */
#include <soc/soc.h>
#include <soc/type.h>
#include <drv/uart/uart.h>
#include <drv/uart/probe.h>
#include <osal/time.h>
#include <osal/print.h>
/* 
 * Symbol Definition 
 */
//#define DEBUG
/* poe packet size is 15, buffer_size > 15 */
#define UART_TX_BUFFER_SIZE	(32)
#define UART_RX_BUFFER_SIZE	(32)

/*
 * Data Declaration 
 */

typedef struct UART_QUE_S
{
	unsigned int	size;
	unsigned int	consumerIndex;
	unsigned int	producerIndex;
	unsigned int	boundryIndex;
	unsigned char	*buf;
} UART_QUE_T;

UART_QUE_T rx_buf;
UART_QUE_T tx_buf;

static uint8 rx_pool[UART_RX_BUFFER_SIZE];
static uint8 tx_pool[UART_TX_BUFFER_SIZE];
/*
 * Macro Definition
 */

/*
 * Function Declaration
 */

/* Function Name:
 *      drv_uart_QueReset
 * Description:
 *      reset the queue info to default value
 * Input:
 *      que - queue pointer
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
static int drv_uart_QueReset(UART_QUE_T* que)
{
	/* check if it is a value queue */
	if(que == NULL)
		return RT_ERR_FAILED;

	/* reset the indices */
	que->consumerIndex = 1;
	que->producerIndex = 1;
	que->boundryIndex = 0;

	return RT_ERR_OK;	
}

/* Function Name:
 *      drv_uart_QueCreate
 * Description:
 *      allocate buffer to queue
 * Input:
 *      que - queue pointer
 *      buf - data buffer
 *      size - buffer size
 * Output:
 *      None
 * Return:
 *			queue pointer 			
 * Note:
 *      None
 */
static UART_QUE_T *drv_uart_QueCreate(UART_QUE_T *que,uint8 *buf,uint32 size)
{
	if(que != NULL)
	{
		/* allocating the buffer address right after the structure */
		que->buf = buf;
		que->size = size;
		
		/* reset the indeices */
		drv_uart_QueReset(que);
	}
	
	return que;
}

/* Function Name:
 *      drv_uart_IsQueEmpty
 * Description:
 *      check if queue empty
 * Input:
 *      que - queue pointer
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
static int drv_uart_IsQueEmpty(UART_QUE_T *que)
{
	/* check if the que is valid */
	if(que == NULL)
		return RT_ERR_FAILED;

    if (que->producerIndex == que->consumerIndex) 
        return RT_ERR_OK;
    else
        return RT_ERR_FAILED;
}

/* Function Name:
 *      drv_uart_IsQueFull
 * Description:
 *      check if queue full
 * Input:
 *      que - queue pointer
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
static int drv_uart_IsQueFull(UART_QUE_T *que)
{
	/* check if the que is valid */
	if(que == NULL)
		return RT_ERR_FAILED;

    if(que->producerIndex == que->boundryIndex)
        return RT_ERR_OK;
    else
        return RT_ERR_FAILED;
}

/* Function Name:
 *      drv_uart_QueConsume
 * Description:
 *      get char(s) from queue buffer
 * Input:
 *      que - queue pointer
 *      buf - char(s) buffer pointer
 *    	length - expect input char(s) size
 * Output:
 *      None
 * Return:
 *      the success insert char(s) count
 * Note:
 *      None
 */
static int drv_uart_QueConsume(UART_QUE_T *que, uint8 *buf, int32 length)
{
	int32 i;
	uint8 *source_ptr;
	unsigned long flags;
	/* check if the que is valid */
	if(que == NULL)
		return 0;

	source_ptr=que->buf;
	
	for(i = 0;i<length && drv_uart_IsQueEmpty(que) != RT_ERR_OK; i++)
	{
			local_irq_save(flags);
	    que->boundryIndex = que->consumerIndex;
	    *(buf+i)=*(source_ptr+que->consumerIndex);
	    que->consumerIndex++;
	    if(que->consumerIndex >= que->size)
    	{
    		/* circular wrap */
        	que->consumerIndex = 0;
    	}
   		local_irq_restore(flags);
	}
    return i;
}

/* Function Name:
 *      drv_uart_QueProduce
 * Description:
 *      remove char(s) from queue buffer
 * Input:
 *      que - queue pointer
 *      buf - char(s) buffer pointer
 *    	length - expect input char(s) size
 * Output:
 *      None
 * Return:
 *      the success remove char(s) count
 * Note:
 *      None
 */
static int drv_uart_QueProduce(UART_QUE_T *que, uint8 *buf, int32 length)
{
   
	int32 i;
	uint8 *dest_ptr;
	unsigned long flags;

	/* check if the que is valid */
	
	if(que == NULL)
		return 0;
	
	dest_ptr = que->buf;
	for(i = 0;i<length && drv_uart_IsQueFull(que) != RT_ERR_OK; i++)
	{
	  	local_irq_save(flags);
    	*(dest_ptr+que->producerIndex) = *(buf+i);
    	que->producerIndex++;
    	
	    if(que->producerIndex == que->size)
    	{
    		/* circular wrap */
        	que->producerIndex = 0;
    	}
   		local_irq_restore(flags);
    }
    return i;
}

/* Function Name:
 *      drv_uart_CallbackInput
 * Description:
 *      the call back routine for serial isr char in
 * Input:
 *      que - queue pointer
 *      inchar - serial in data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
static int32 drv_uart_CallbackInput(uint32 unit, uint8 inchar)
{
	/* store received byte to rx queue */
	if(drv_uart_QueProduce(&rx_buf, &inchar, (int32)1) != 1)
		return RT_ERR_FAILED;
	else
	{
#if defined(DEBUG)
		printk("%.2x ",inchar);
#endif
		return RT_ERR_OK;
	}
}

/* Function Name:
 *      drv_uart_CallbackOutput
 * Description:
 *      the call back routine for serial isr char out
 * Input:
 *      que - queue pointer
 *      inch - serial out data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
static int32 drv_uart_CallbackOutput(uint32 unit, uint8 *inch)
{
	/* send a byte to caller (the interrupt routing) */
	if(drv_uart_QueConsume(&tx_buf, (uint8 *)inch, (int32)1) != 1)
		return RT_ERR_FAILED;
	else
	{
#if defined(DEBUG)
		printk("%.2x ",*inch);
#endif
		return RT_ERR_OK;
	}
}

/* Function Name:
 *      drv_uart_init
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
 *      None
 */
int32
drv_uart_init(uint32 unit)
{
	drv_uart_QueCreate(&rx_buf,rx_pool,UART_RX_BUFFER_SIZE);
  drv_uart_QueCreate(&tx_buf,tx_pool,UART_TX_BUFFER_SIZE);
  
  return UART_CTRL(unit).init(unit, 
    (drv_uart_rxcallback_t)drv_uart_CallbackInput,
    (drv_uart_txcallback_t)drv_uart_CallbackOutput);
    
  
} /* end of drv_uart_init */

/* Function Name:
 *      drv_uart_clearfifo
 * Description:
 *      empty hw rx/tx fifo and reset queue information to default value
 * Input:
 *      que - queue pointer
 *      inch - serial out data
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void
drv_uart_clearfifo(uint32 unit)
{
	unsigned long flags;
	local_irq_save(flags);
	UART_CTRL(unit).clearfifo(unit);
	drv_uart_QueReset(&rx_buf);
  drv_uart_QueReset(&tx_buf);
	local_irq_restore(flags);
	return;
	
} /* end of drv_uart_clearfifo */

/* Function Name:
 *      drv_uart_getc
 * Description:
 *      Get the character from uart interface with timeout value in the specified device
 * Input:
 *      unit    - unit id
 *      timeout - timeout value (unit: milli-second), 0 mean no timeout
 * Output:
 *      pData   - pointer buffer of character from uart interface
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_BUSYWAIT_TIMEOUT - timeout and no get the character
 * Note:
 *      None
 */
int32
drv_uart_getc(uint32 unit, uint8 *pData, uint32 timeout)
{
	  uint32  timeout_us;
    osal_usecs_t begin;
    osal_usecs_t end;

    timeout_us = timeout * 1000;
    osal_time_usecs_get(&begin);
    *pData = 0;
    while (UART_CTRL(unit).tstc(unit)== RT_ERR_FAILED)
    {
        if (timeout != 0)
        {
            osal_time_usecs_get(&end);
            if (end - begin > timeout_us)
                return RT_ERR_BUSYWAIT_TIMEOUT;
            osal_time_udelay(10);
        }
    }
    return UART_CTRL(unit).poll_getc(unit,pData);
} /* end of drv_uart_getc */

/* Function Name:
 *      drv_uart_gets
 * Description:
 *      Get the character(s) from uart queue buffer with timeout value in the specified device
 * Input:
 *      unit    - unit id
 *      timeout - timeout value (unit: milli-second), 0 mean no timeout
 * Output:
 *      pData   - pointer buffer of character from uart interface
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_BUSYWAIT_TIMEOUT - timeout and no get the character
 * Note:
 *      None
 */
int32
drv_uart_gets(uint32 unit, uint8 *pData, uint32 expect_size,uint32 *rev_cnt,uint32 timeout)
{
	  uint32  timeout_us;
    osal_usecs_t begin;
    osal_usecs_t end;
    uint32 count;
		uint32 offset,remainder;
		
		*rev_cnt=count=offset=0;
		remainder = expect_size;
    while (remainder != 0)
    {
	    timeout_us = timeout * 1000;
	    osal_time_usecs_get(&begin);
	    count=0;
    	while (count==0)
    	{
    		  count = drv_uart_QueConsume(&rx_buf, pData+offset, remainder);
    	    if (!count && timeout != 0)
    	    {
    	        osal_time_usecs_get(&end);
    	        if (end - begin > timeout_us)
    	        {
    	        	printk("\n timeout,expect size %d,remainder %d",expect_size,remainder);
    	          return RT_ERR_BUSYWAIT_TIMEOUT;
    	        }
    	        osal_time_usleep(10*1000);
    	    }
    	    if (count)
    	    {
    	    	#if defined(DEBUG)
    	    	int i;
    	    	for (i=0;i<count;i++)
    	    		printk("%.2x ",*(pData+offset+i));
    	    	#endif
    	    	offset=offset+count;
    	    	remainder=remainder-count;
    	    }
    	}
  	}
  	
  	*rev_cnt = (expect_size-remainder);
  	if (remainder)
  		return RT_ERR_FAILED;
  	else
  		return RT_ERR_OK;
} /* end of drv_uart_gets */

/* Function Name:
 *      drv_uart_putc
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
drv_uart_putc(uint32 unit, uint8 data)
{
	return UART_CTRL(unit).poll_putc(unit,data);
} /* end of drv_uart_putc */

/* Function Name:
 *      drv_uart_puts
 * Description:
 *      Output the character(s) to uart queue buffer in the specified device
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
drv_uart_puts(uint32 unit, uint8 *data,uint32 size)
{
	int32 retval = RT_ERR_OK;
	uint32 out_size = 0;
	uint32   offset,remainder;
	remainder=size;
	offset=0;
	while (remainder != 0)
	{
		out_size = drv_uart_QueProduce(&tx_buf,data+offset, remainder);
		if (out_size != 0)
		{
			UART_CTRL(unit).starttx(unit);
   		osal_time_usleep(100);
		}
		remainder=remainder-out_size;
		offset=offset+out_size;
		#if defined(DEBUG)
		if (remainder != 0)
			printk("\n drv_uart_puts, remainder %d offset %d\n",remainder,offset);
		#endif
  }
  return retval;
} /* end of drv_uart_puts */

/* Function Name:
 *      drv_uart_baudrate_get
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
drv_uart_baudrate_get(uint32 unit, drv_uart_baudrate_t *pBaudrate)
{
    return UART_CTRL(unit).baudrate_get(unit, pBaudrate);
} /* end of drv_uart_baudrate_get */

/* Function Name:
 *      drv_uart_baudrate_set
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
drv_uart_baudrate_set(uint32 unit, drv_uart_baudrate_t baudrate)
{
    return UART_CTRL(unit).baudrate_set(unit, baudrate);
} /* end of drv_uart_baudrate_set */
