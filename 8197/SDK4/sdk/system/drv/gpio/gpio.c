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
 * $Revision: 46572 $
 * $Date: 2014-02-21 13:41:41 +0800 (Fri, 21 Feb 2014) $
 *
 * Purpose : Definition those public GPIO routing APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *           (1) Internal GPIO
 *
 */
 
/*  
 * Include Files 
 */

#include <common/debug/rt_log.h>
#include <drv/gpio/gpio.h>
#include <dev_config.h>
#include <osal/thread.h>
#include <osal/atomic.h>
#include <osal/wait.h>
#include <soc/soc.h>
#include <soc/type.h>


/* 
 * Symbol Definition 
 */

/*
 * Data Declaration 
 */
static uint32 regGpioControl[] = 
{
    PABCDCNR,/* Port A */
    PABCDCNR,/* Port B */
    PABCDCNR,/* Port C */
    PABCDCNR,/* Port D */
#if defined(CONFIG_SDK_RTL8389) || defined(CONFIG_SDK_RTL8328)
    PEFGCNR, /* Port E */
    PEFGCNR, /* Port F */
    PEFGCNR, /* Port G */
#endif
};

static uint32 bitStartGpioControl[] =
{
    24,      /* Port A */
    16,      /* Port B */
    8,       /* Port C */
    0,       /* Port D */
#if defined(CONFIG_SDK_RTL8389) || defined(CONFIG_SDK_RTL8328)
    24,      /* Port E */
    16,      /* Port F */
    8,       /* Port G */
#endif
};

static uint32 regGpioDirection[] =
{
    PABCDDIR,/* Port A */
    PABCDDIR,/* Port B */
    PABCDDIR,/* Port C */
    PABCDDIR,/* Port D */
#if defined(CONFIG_SDK_RTL8389) || defined(CONFIG_SDK_RTL8328)
    PEFGDIR, /* Port E */
    PEFGDIR, /* Port F */
    PEFGDIR, /* Port G */
#endif
};

static uint32 bitStartGpioDirection[] =
{
    24,      /* Port A */
    16,      /* Port B */
    8,       /* Port C */
    0,       /* Port D */
#if defined(CONFIG_SDK_RTL8389) || defined(CONFIG_SDK_RTL8328)
    24,      /* Port E */
    16,      /* Port F */
    8,       /* Port G */
#endif
};

static uint32 regGpioData[] =
{
    PABCDDAT,/* Port A */
    PABCDDAT,/* Port B */
    PABCDDAT,/* Port C */
    PABCDDAT,/* Port D */
#if defined(CONFIG_SDK_RTL8389) || defined(CONFIG_SDK_RTL8328)
    PEFGDAT, /* Port E */
    PEFGDAT, /* Port F */
    PEFGDAT, /* Port G */
#endif
};

static uint32 bitStartGpioData[] =
{
    24,      /* Port A */
    16,      /* Port B */
    8,       /* Port C */
    0,       /* Port D */
#if defined(CONFIG_SDK_RTL8389) || defined(CONFIG_SDK_RTL8328)
    24,      /* Port E */
    16,      /* Port F */
    8,       /* Port G */
#endif
};

static uint32 regGpioInterruptStatus[] =
{
    PABCDISR,/* Port A */
    PABCDISR,/* Port B */
    PABCDISR,/* Port C */
    PABCDISR,/* Port D */
#if defined(CONFIG_SDK_RTL8389) || defined(CONFIG_SDK_RTL8328)
    PEFGISR, /* Port E */
    PEFGISR, /* Port F */
    PEFGISR, /* Port G */
#endif
};

static uint32 bitStartGpioInterruptStatus[] =
{
    24,      /* Port A */
    16,      /* Port B */
    8,       /* Port C */
    0,       /* Port D */
#if defined(CONFIG_SDK_RTL8389) || defined(CONFIG_SDK_RTL8328)
    24,      /* Port E */
    16,      /* Port F */
    8,       /* Port G */
#endif
};

static uint32 regGpioInterruptEnable[] =
{
    PABIMR,  /* Port A */
    PABIMR,  /* Port B */
    PCDIMR,  /* Port C */
    PCDIMR,  /* Port D */
#if defined(CONFIG_SDK_RTL8389) || defined(CONFIG_SDK_RTL8328)
    PEFIMR,  /* Port E */
    PEFIMR,  /* Port F */
    PGIMR,   /* Port G */
#endif
};

static uint32 bitStartGpioInterruptEnable[] =
{
    16,      /* Port A */
    0,       /* Port B */
    16,      /* Port C */
    0,       /* Port D */
#if defined(CONFIG_SDK_RTL8389) || defined(CONFIG_SDK_RTL8328)
    16,      /* Port E */
    0,       /* Port F */
    16,      /* Port G */
#endif
};



typedef struct gpio_database_s
{
    uint32  register_pin;
    uint32  init_pin;
    uint32  action_status;
    drv_gpioIsr_cb_f callback;
    
} gpio_database_t;


static gpio_database_t data[GPIO_INTERNAL_PIN_END];

static uint32 PABCDDAT_shadow = 0; /* Shadow of GPIO ABCD data */
static uint32 PABCDDAT_mask = 0;   /* Shadow of GPIO ABCD mask */
#if defined(CONFIG_SDK_RTL8389) || defined(CONFIG_SDK_RTL8328)
static uint32 PEFGDAT_shadow = 0;  /* Shadow of GPIO EFG data */
static uint32 PEFGDAT_mask = 0;    /* Shadow of GPIO EFG mask */
#endif
static osal_atomic_t gpio_wait_for_intr;
static osal_wait_queue_head_t gpio_intr_wait_queue;
rtk_enable_t threadEnabled = DISABLED;


static drv_gpioIsr_cb_f gpioInterruptCb[GPIO_INTERNAL_PIN_END];



/*
 * Macro Definition
 */

/*
 * Function Declaration
 */
 


/* Function Name:
 *      _gpio_interrupt_handler
 * Description:
 *      Pending interrupt thread of gpio interrupt for waiting interrupt event.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
static int _gpio_interrupt_handler(void)
{
    int32 port,pin = 0;
    int32 ret = RT_ERR_OK;      
    gpio_pin_data_t  gpio_data;

#if defined(CONFIG_SDK_RTL8389) || defined(CONFIG_SDK_RTL8328)
    for(port = GPIO_PORT_A ; port <= GPIO_PORT_G ; port++ ){
#endif
#if defined(CONFIG_SDK_RTL8380) || defined(CONFIG_SDK_RTL8390)
	for(port = GPIO_PORT_A ; port <= GPIO_PORT_D ; port++ ){
#endif

        for(pin = GPIO_PIN_MIN; pin <= GPIO_PIN_MAX ; pin++ ){
            if(data[PORT_AND_PIN_TO_PINID(port,pin)].action_status == ENABLED){
                gpio_data.pin_id = PORT_AND_PIN_TO_PINID(port,pin);                       
                ret = drv_gpio_dataBit_get(GPIO_ID(port,pin),&(gpio_data.gpio_bit));
                if (RT_ERR_OK != ret)
                {
                    return ret;
                }
                gpioInterruptCb[PORT_AND_PIN_TO_PINID(port,pin)](&gpio_data);
                data[PORT_AND_PIN_TO_PINID(port,pin)].action_status = DISABLED;
            }
        }

    }
    return RT_ERR_OK;
}

/* Function Name:
 *      _gpio_intr_thread
 * Description:
 *      GPIO interrupt thread to receive interrupt event.
 * Input:
 *      pArg - Parameter that is provided when thread create. 
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
static void *_gpio_intr_thread(void *pArg)
{   

    osal_init_waitqueue_head(&gpio_intr_wait_queue);

    while(1)
    {            
        if (gpio_wait_for_intr.counter <= 0)
        {
            osal_atomic_inc(&gpio_wait_for_intr);
        }      
        osal_wait_event_interruptible(gpio_intr_wait_queue, osal_atomic_read(&gpio_wait_for_intr) <= 0);
        /*                      
        * only run the interrupt handler once.
        */
        osal_atomic_set(&gpio_wait_for_intr, 0);
        
        _gpio_interrupt_handler();             
    }    
    return NULL;
} 


/* Function Name:
 *      gpio_intr_attach
 * Description:
 *      Connect interrupt with handle thread
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32 gpio_intr_attach(void){
 
    int32 ret;    
    /* Check arguments */
    ret = osal_thread_create(GPIO_INTR_THREAD_NAME, 4096, 0, (void *)_gpio_intr_thread, NULL);
    if (0 == ret)
    {
      RT_ERR(ret, MOD_RTCORE, "GPIO interrupt thread create failed");
      return RT_ERR_FAILED;     
    }           
    return RT_ERR_OK;
}
 
/* Function Name:
*      drv_gpio_isr
* Description:
*      GPIO ABCD and EFG interrupt handler routine
* Input:
*      pParam - isr parameter
* Output:
*      None
* Return:
*      SYS_ERR_OK
*      SYS_ERR_FAILED
* Note:
*      None
*/
osal_isrret_t drv_gpio_isr(void *pParam)
{    
 int32 port,pin = 0;
 int32 gIsr = 0;
 int32 ret = RT_ERR_OK;      
#if defined(CONFIG_SDK_RTL8389) || defined(CONFIG_SDK_RTL8328)
 for(port = GPIO_PORT_A ; port <= GPIO_PORT_G ; port++ ){
#endif  
#if defined(CONFIG_SDK_RTL8380) || defined(CONFIG_SDK_RTL8390)
 for(port = GPIO_PORT_A ; port <= GPIO_PORT_D ; port++ ){
#endif 

     for(pin = GPIO_PIN_MIN; pin <= GPIO_PIN_MAX ; pin++ ){

         ret = drv_gpio_isr_get(GPIO_ID(port,pin),&gIsr);
         if (RT_ERR_OK != ret)
         {
             return ret;
         }
         
         if (gIsr != 0){                  
             if(data[PORT_AND_PIN_TO_PINID(port,pin)].init_pin == GPIO_FLAG_ON && data[PORT_AND_PIN_TO_PINID(port,pin)].register_pin == GPIO_FLAG_ON){
                 
                 if (RT_ERR_OK != ret)
                 {
                     return ret;
                 }
                 drv_gpio_isr_clear(GPIO_ID(port,pin));
                 
                 data[PORT_AND_PIN_TO_PINID(port,pin)].action_status = ENABLED;

                 if (osal_atomic_dec_return(&gpio_wait_for_intr) >= 0) 
                 {
                     osal_wake_up_interruptible(&gpio_intr_wait_queue);
                 }
             }
         }
     }
 }

 return RT_ERR_OK;
 
}


/* Function Name:
 *      drv_gpio_intrHandler_register
 * Description:
 *      register GPIO interrupt callback function for specified GPIO pin
 * Input:
 * Output:
 *      pin             - specified GPIO pin
 *      gpioIsrCallback - callback function
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32 drv_gpio_intrHandler_register(GPIO_INTERNAL_PIN_t pin, drv_gpioIsr_cb_f gpioIsrCallback)
{

    int i = 0;
    int32 ret = RT_ERR_FAILED;
    rtk_enable_t regIntrSrc = DISABLED;
   
     
    RT_PARAM_CHK((NULL == gpioIsrCallback), RT_ERR_NULL_POINTER);     
    RT_PARAM_CHK(IS_GPIO_PIN_INVALID(pin), RT_ERR_INPUT);

    if(threadEnabled == DISABLED){
        gpio_intr_attach();
        threadEnabled = ENABLED;
    }
    
    if(pin <= D7 && pin >= A0){
        
        for(i = A0 ; i <= D7 ; i++){
            if(data[i].register_pin == GPIO_FLAG_ON)
                regIntrSrc = ENABLED;
        }        

        if(regIntrSrc == DISABLED){
            /* Register GPIO_ABCD IRQ */
            RT_ERR_HDL(osal_isr_register(RTK_DEV_GPIO_ABCD, drv_gpio_isr, NULL), error, ret);

        }

    }else if(pin <= G7 && pin >= E0){

        for(i = E0 ; i <= G7 ; i++){
            if(data[i].register_pin == GPIO_FLAG_ON)
                regIntrSrc = ENABLED;
        }
        
#if defined(CONFIG_SDK_RTL8389) || defined(CONFIG_SDK_RTL8328)
        if(regIntrSrc == DISABLED){
            /* Register GPIO_EFG IRQ */
            RT_ERR_HDL(osal_isr_register(RTK_DEV_GPIO_EFG, drv_gpio_isr, NULL), error, ret);
        }
#endif
    }
    
    gpioInterruptCb[pin] = gpioIsrCallback;    
    data[pin].register_pin = GPIO_FLAG_ON;

    return RT_ERR_OK;

    error:
        printk("Error - Register GPIO IRQ and Interrupt Handler Failed!\n");

    return RT_ERR_FAILED;    
}

/* Function Name:
 *      drv_gpio_intrHandler_unregister
 * Description:
 *      unregister GPIO interrupt callback function for specified GPIO pin
 * Input:
 * Output:
 *      pin - specified GPIO pin
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32 drv_gpio_intrHandler_unregister(GPIO_INTERNAL_PIN_t pin)
{
    int i = 0;
    int32 ret = RT_ERR_FAILED;
    rtk_enable_t unRegIntrSrc = DISABLED;

    RT_PARAM_CHK(IS_GPIO_PIN_INVALID(pin), RT_ERR_INPUT);


    data[pin].register_pin = GPIO_FLAG_OFF;

    if(pin <= D7 && pin >= A0){
        
        for(i = A0 ; i <= D7 ; i++){
            if(data[i].register_pin == GPIO_FLAG_ON)
                unRegIntrSrc = ENABLED;
        }

        if(unRegIntrSrc == DISABLED)
            RT_ERR_HDL(osal_isr_unregister(RTK_DEV_GPIO_ABCD), error, ret);


    }else if(pin <= G7 && pin >= E0){

        for(i = E0 ; i <= G7 ; i++){
            if(data[i].register_pin == GPIO_FLAG_ON)
                unRegIntrSrc = ENABLED;
        }
        
#if defined(CONFIG_SDK_RTL8389) || defined(CONFIG_SDK_RTL8328)
        if(unRegIntrSrc == DISABLED)
            RT_ERR_HDL(osal_isr_unregister(RTK_DEV_GPIO_EFG), error, ret);
#endif

    }
        
    return RT_ERR_OK;


    error:
        printk("Error - Unregister GPIO IRQ and Interrupt Handler Failed!\n");

    return RT_ERR_FAILED; 
    
}


/* Function Name:
 *      _getGpio
 * Description:
 *      Get gpio control/direction/data/status value
 *      Abstract GPIO registers
 *      This function is for internal use only. 
 *      You don't need to care what register address of GPIO is.
 *      This function abstracts these information.
 * Input:
 *      func - control/data/interrupt register
 *      poer - GPIO port
 *      pin  - pin number
 * Output:
 *      None
 * Return:
 *      RT_ERR_FAILED  
 *      0             - control/direction/data/status value
 *      1             - control/direction/data/status value
 *      0xFFFFFFFF    - unknow request
 * Note:
 *      None
 */
static int32 
_getGpio(drv_gpio_func_t func, drv_gpio_port_t port, uint32 pin)
{
    uint32  gpio_data = 0;

    /* parameter check */
    RT_INTERNAL_PARAM_CHK(!GPIO_FUNC_CHK(func), RT_ERR_FAILED);
    /* port and pin had been checked by extenal public API */

    RT_LOG(LOG_FUNC_ENTER, MOD_GENERAL, "[%s():%d] func=%d port=%d pin=%d\n",                   \
            __FUNCTION__, __LINE__, func, port, pin );

    switch (func)
    {
        case GPIO_FUNC_CONTROL:
            RT_LOG(LOG_DEBUG, MOD_GENERAL, "[%s():%d] regGpioControl[port]=0x%08x  "            \
                    "bitStartGpioControl[port]=%d\n", __FUNCTION__, __LINE__,                   \
                    regGpioControl[port], bitStartGpioControl[port]);

            /* Get GPIO control bit */
            if (REG32(regGpioControl[port]) & ((uint32)1 << (pin + bitStartGpioControl[port])))
            {
                return 1;
            }
            else
            {
                return 0;
            }
            break;            
            
        case GPIO_FUNC_DIRECTION:
            RT_LOG(LOG_DEBUG, MOD_GENERAL, "[%s():%d] regGpioDirection[port]=0x%08x  "          \
                    "bitStartGpioDirection[port]=%d\n", __FUNCTION__, __LINE__,                 \
                    regGpioDirection[port], bitStartGpioDirection[port] );

            /* Get GPIO direction bit */
            if (REG32(regGpioDirection[port]) &                                                 \
                 ((uint32)1 << (pin + bitStartGpioDirection[port])))
            {
                return 1;
            }
            else
            {
                return 0;
            }
            break;
            
        case GPIO_FUNC_DATA:
            RT_LOG(LOG_DEBUG, MOD_GENERAL, "[%s():%d] regGpioData[port]=0x%08x  "               \
                    "bitStartGpioData[port]=%d\n", __FUNCTION__, __LINE__,                      \
                    regGpioData[port], bitStartGpioData[port] );

            /* Get GPIO port[pin] bit */
            gpio_data = REG32(regGpioData[port]);
            if (port <= GPIO_PORT_D)
            {
                gpio_data &= ~(PABCDDAT_mask);
                gpio_data |= (PABCDDAT_shadow & PABCDDAT_mask);
            }
            else
            {
#if defined(CONFIG_SDK_RTL8389) || defined(CONFIG_SDK_RTL8328)
                gpio_data &= ~(PEFGDAT_mask);
                gpio_data |= (PEFGDAT_shadow & PEFGDAT_mask);
#endif
            }
            
            if (gpio_data & ((uint32)1 << (pin + bitStartGpioData[port])))
            {
                return 1;
            }
            else
            {
                return 0;
            }
            break;
            
        case GPIO_FUNC_INTERRUPT_ENABLE:
            RT_LOG(LOG_DEBUG, MOD_GENERAL, "[%s():%d] regGpioInterruptEnable[port]=0x%08x  "    \
                    "bitStartGpioInterruptEnable[port]=%d\n", __FUNCTION__, __LINE__,           \
                       regGpioInterruptEnable[port], bitStartGpioInterruptEnable[port] );

            /* Get GPIO interrupt enable bit */
            return (REG32(regGpioInterruptEnable[port]) >>                                      \
                     (pin*2 + bitStartGpioInterruptEnable[port])) & (uint32)0x3;
            break;

        case GPIO_FUNC_INTERRUPT_STATUS:
            RT_LOG(LOG_DEBUG, MOD_GENERAL, "[%s():%d] regGpioInterruptStatus[port]=0x%08x  "    \
                "bitStartGpioInterruptEnable[port]=%d\n", __FUNCTION__, __LINE__,               \
                regGpioInterruptStatus[port], bitStartGpioInterruptStatus[port] );

            /* Get GPIO interrupt status bit */
            if (REG32(regGpioInterruptStatus[port]) &                                           \
                 ((uint32)1 << (pin + bitStartGpioInterruptStatus[port])))
            {
                return 1;
            }
            else
            {
                return 0;
            }
            break;
            
        case GPIO_FUNC_END:
            return RT_ERR_FAILED;
    }
    
    return (0xFFFFFFFF);
} /* end of _getGpio */

/* Function Name:
 *      _setGpio
 * Description:
 *      Set gpio control/direction/data/status value
 *      Abstract GPIO registers
 *      This function is for internal use only. 
 *      You don't need to care what register address of GPIO is.
 *      This function abstracts these information.
 * Input:
 *      func - control/data/interrupt register
 *      port - GPIO port
 *      pin  - pin number
 *      data - write value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
static int32 
_setGpio(drv_gpio_func_t func, drv_gpio_port_t port, uint32 pin, uint32 data)
{
    uint32  gpio_data = 0;

    /* parameter check */     
    RT_INTERNAL_PARAM_CHK(!GPIO_FUNC_CHK(func), RT_ERR_FAILED);
    /* port and pin had been checked by extenal public API */
    
    RT_LOG(LOG_FUNC_ENTER, MOD_GENERAL, "[%s():%d] func=%d port=%d pin=%d data=%d\n",           \
            __FUNCTION__, __LINE__, func, port, pin, data );

    switch (func)
    {
        case GPIO_FUNC_CONTROL:
            RT_LOG(LOG_DEBUG, MOD_GENERAL, "[%s():%d] regGpioControl[port]=0x%08x  "            \
                    "bitStartGpioControl[port]=%d\n", __FUNCTION__, __LINE__,                   \
                    regGpioControl[port], bitStartGpioControl[port] );

            /* Set GPIO control */
            if (data)
            {
                REG32(regGpioControl[port]) |= (uint32)1 << (pin + bitStartGpioControl[port]);
            }
            else
            {
                REG32(regGpioControl[port]) &= ~((uint32)1 << (pin + bitStartGpioControl[port]));
            }
            break;
            
        case GPIO_FUNC_DIRECTION:
            RT_LOG(LOG_DEBUG, MOD_GENERAL, "[%s():%d] regGpioDirection[port]=0x%08x  "          \
                    "bitStartGpioDirection[port]=%d\n", __FUNCTION__, __LINE__,                 \
                    regGpioDirection[port], bitStartGpioDirection[port] );

            /* Set GPIO direction */
            if (data)
            {
                REG32(regGpioDirection[port]) |= (uint32)1 << (pin + bitStartGpioDirection[port]);
            }
            else
            {
                REG32(regGpioDirection[port]) &= ~((uint32)1 << (pin + bitStartGpioDirection[port]));
            }
            break;

        case GPIO_FUNC_DATA:
            RT_LOG(LOG_DEBUG, MOD_GENERAL, "[%s():%d] regGpioData[port]=0x%08x  "               \
                    "bitStartGpioData[port]=%d\n", __FUNCTION__, __LINE__,                      \
                    regGpioData[port], bitStartGpioData[port] );

            gpio_data = REG32(regGpioData[port]);
            /* If it is GPO pin, update the shadow data first */
            if (port <= GPIO_PORT_D)
            {
                if ( PABCDDAT_mask & ((uint32)1 << (pin + bitStartGpioData[port])))
                {
                    PABCDDAT_shadow &= ~((uint32)1 << (pin + bitStartGpioData[port]));
                    PABCDDAT_shadow |= ((uint32)data << (pin + bitStartGpioData[port]));
                }
                gpio_data &= ~(PABCDDAT_mask);
                gpio_data |= (PABCDDAT_shadow & PABCDDAT_mask);
            }
            else
            {
#if defined(CONFIG_SDK_RTL8389) || defined(CONFIG_SDK_RTL8328)
                if ( PEFGDAT_mask & ((uint32)1 << (pin + bitStartGpioData[port])))
                {
                    PEFGDAT_shadow &= ~((uint32)1 << (pin + bitStartGpioData[port]));
                    PEFGDAT_shadow |= ((uint32)data << (pin + bitStartGpioData[port]));
                }
                gpio_data &= ~(PEFGDAT_mask);
                gpio_data |= (PEFGDAT_shadow & PEFGDAT_mask);
#endif
            }

            /* Set GPIO port[pin] bit */
            if (data)
            {
                gpio_data |= (uint32)1 << (pin + bitStartGpioData[port]);
            }
            else
            {
                gpio_data &= ~((uint32)1 << (pin + bitStartGpioData[port]));
            }
            REG32(regGpioData[port]) = gpio_data;
            break;
            
        case GPIO_FUNC_INTERRUPT_ENABLE:
            RT_LOG(LOG_DEBUG, MOD_GENERAL, "[%s():%d] regGpioInterruptEnable[port]=0x%08x  "    \
                    "bitStartGpioInterruptEnable[port]=%d\n", __FUNCTION__, __LINE__,           \
                    regGpioInterruptEnable[port], bitStartGpioInterruptEnable[port] );

            /* Set GPIO interrupt enable bit */
            REG32(regGpioInterruptEnable[port]) &= ~((uint32)0x3 <<                             \
                    (pin*2 + bitStartGpioInterruptEnable[port]));
            REG32(regGpioInterruptEnable[port]) |= (uint32)data <<                              \
                    (pin*2 + bitStartGpioInterruptEnable[port]);
            break;

        case GPIO_FUNC_INTERRUPT_STATUS:
            RT_LOG(LOG_DEBUG, MOD_GENERAL, "[%s():%d] regGpioInterruptStatus[port]=0x%08x  "    \
                    "bitStartGpioInterruptStatus[port]=%d\n", __FUNCTION__, __LINE__,           \
                    regGpioInterruptStatus[port], bitStartGpioInterruptStatus[port] );

            /* Set GPIO interrupt status bit */
            if (data)
            {
                REG32(regGpioInterruptStatus[port]) |= (uint32)1 <<                             \
                    (pin + bitStartGpioInterruptStatus[port]);
            }
            else
            {
                REG32(regGpioInterruptStatus[port]) &= ~((uint32)1 <<                           \
                    (pin + bitStartGpioInterruptStatus[port]));
            }
            break;
        
        default:    
            break;
    }

    return RT_ERR_OK;
} /* end of _setGpio */

/* Function Name:
 *      drv_gpio_init
 * Description:
 *      Init GPIO port
 * Input:
 *      gpioId          - The GPIO port that will be configured
 *      function        - Pin control function
 *      direction       - Data direction, in or out
 *      interruptEnable - Interrupt mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      None
 */
int32 
drv_gpio_init( 
    gpioID gpioId, 
    drv_gpio_control_t function,
    drv_gpio_direction_t direction,
    drv_gpio_interruptType_t interruptEnable)
{
    uint32 port = GPIO_PORT(gpioId);
    uint32 pin = GPIO_PIN(gpioId);
    
    /* parameter check */    
    RT_PARAM_CHK(!GPIO_PORT_CHK(port), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(!GPIO_PIN_CHK(pin), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(!GPIO_CTRLFUNC_CHK(function), RT_ERR_FAILED);
    RT_PARAM_CHK(!GPIO_DIR_CHK(direction), RT_ERR_FAILED);
    RT_PARAM_CHK(!GPIO_INT_CHK(interruptEnable), RT_ERR_FAILED);

    /* set gpio function */
    _setGpio(GPIO_FUNC_CONTROL, port, pin, function); 
    _setGpio(GPIO_FUNC_DIRECTION, port, pin, direction);
    _setGpio(GPIO_FUNC_INTERRUPT_ENABLE, port, pin, interruptEnable);

    data[PORT_AND_PIN_TO_PINID(port,pin)].init_pin = GPIO_FLAG_ON;

    return RT_ERR_OK;
} /* end of drv_gpio_init */

/* Function Name:
 *      drv_gpio_dataBit_init
 * Description:
 *      Initialize the bit value of a specified GPIO ID
 * Input:
 *      gpioId - GPIO ID
 * Output:
 *      data   - Data to write
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      Only the GPO pin need to call the API to init default value.
 */
int32 drv_gpio_dataBit_init(gpioID gpioId, uint32 data)
{
    uint32 port = GPIO_PORT(gpioId);
    uint32 pin = GPIO_PIN(gpioId);

    /* parameter check */    
    RT_PARAM_CHK(!GPIO_PORT_CHK(port), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(!GPIO_PIN_CHK(pin), RT_ERR_OUT_OF_RANGE);

    RT_LOG(LOG_FUNC_ENTER, MOD_GENERAL, "[%s():%d] (port=%d,pin=%d)=%d\n", \
            __FUNCTION__, __LINE__, port, pin, data );

    switch (port)
    {
        case GPIO_PORT_A:
        case GPIO_PORT_B:
        case GPIO_PORT_C:
        case GPIO_PORT_D:
            PABCDDAT_shadow |= (uint32)data << (pin + bitStartGpioData[port]);
            PABCDDAT_mask |= (uint32)1 << (pin + bitStartGpioData[port]);
            break;
#if defined(CONFIG_SDK_RTL8389) || defined(CONFIG_SDK_RTL8328)
        case GPIO_PORT_E:
        case GPIO_PORT_F:
        case GPIO_PORT_G:
            PEFGDAT_shadow |= (uint32)data << (pin + bitStartGpioData[port]);
            PEFGDAT_mask |= (uint32)1 << (pin + bitStartGpioData[port]);
            break;
#endif
        default:
            return RT_ERR_OUT_OF_RANGE;
    } /* end of switch (port) */

    return RT_ERR_OK;
} /* end of drv_gpio_dataBit_init */

/* Function Name:
 *      drv_gpio_dataBit_get
 * Description:
 *      Get the bit value of a specified GPIO ID
 * Input:
 *      gpioId - GPIO ID
 * Output:
 *      pData   - Pointer to store return value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      None
 */
int32 
drv_gpio_dataBit_get(gpioID gpioId, uint32 *pData)
{
    uint32 port = GPIO_PORT(gpioId);
    uint32 pin = GPIO_PIN(gpioId);

    /* parameter check */    
    RT_PARAM_CHK(!GPIO_PORT_CHK(port), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(!GPIO_PIN_CHK(pin), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((NULL == pData), RT_ERR_NULL_POINTER); 

    *pData = _getGpio(GPIO_FUNC_DATA, port, pin);

    RT_LOG(LOG_FUNC_ENTER, MOD_GENERAL, "[%s():%d] (port=%d,pin=%d)=%d\n", \
            __FUNCTION__, __LINE__, port, pin, *pData );

    return RT_ERR_OK;
} /* end of drv_gpio_dataBit_get */

/* Function Name:
 *      drv_gpio_dataBit_set
 * Description:
 *      Set GPIO data
 * Input:
 *      gpioId - GPIO ID
 *      data   - Data to write
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      None
 */
int32 
drv_gpio_dataBit_set(gpioID gpioId, uint32 data)
{
    uint32 port = GPIO_PORT(gpioId);
    uint32 pin = GPIO_PIN(gpioId);

    /* parameter check */    
    RT_PARAM_CHK(!GPIO_PORT_CHK(port), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(!GPIO_PIN_CHK(pin), RT_ERR_OUT_OF_RANGE);

    RT_LOG(LOG_FUNC_ENTER, MOD_GENERAL, "[%s():%d] (port=%d,pin=%d)=%d\n", \
            __FUNCTION__, __LINE__, port, pin, data );

    _setGpio(GPIO_FUNC_DATA, port, pin, data);

    return RT_ERR_OK;
} /* end of drv_gpio_dataBit_set */

/* Function Name:
 *      drv_gpio_isr_get
 * Description:
 *      Get the interrupt status register value of a specified GPIO ID
 * Input:
 *      gpioId  - GPIO ID
 * Output:
 *      pIsr    - Pointer to store return value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_OUT_OF_RANGE
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 
drv_gpio_isr_get(gpioID gpioId, uint32 *pIsr)
{
    uint32 port = GPIO_PORT(gpioId);
    uint32 pin = GPIO_PIN(gpioId);

    /* parameter check */    
    RT_PARAM_CHK(!GPIO_PORT_CHK(port), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(!GPIO_PIN_CHK(pin), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((NULL == pIsr), RT_ERR_NULL_POINTER); 

    *pIsr = _getGpio(GPIO_FUNC_INTERRUPT_STATUS, port, pin);

    return RT_ERR_OK;
} /* end of drv_gpio_isr_get */

/* Function Name:
 *      drv_gpio_isr_clear
 * Description:
 *      Clear the interrupt status register value of a specified GPIO ID.
 * Input:
 *      gpioId - GPIO ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      None
 */
int32 
drv_gpio_isr_clear(gpioID gpioId)
{
    uint32 port = GPIO_PORT(gpioId);
    uint32 pin = GPIO_PIN(gpioId);

    /* parameter check */    
    RT_PARAM_CHK(!GPIO_PORT_CHK(port), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(!GPIO_PIN_CHK(pin), RT_ERR_OUT_OF_RANGE);
    
    _setGpio(GPIO_FUNC_INTERRUPT_STATUS, port, pin, 1);   /* write `1` to clear */

    return RT_ERR_OK;
} /* end of drv_gpio_isr_clear */

