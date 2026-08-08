
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
 * $Revision: $
 * $Date: $
 *
 * Purpose : Software Control LED
 *
 * Feature : For RTL8231 Serial Mode
 *
 * Variable:
 *   gLed0Mode : Decide the LED0's meaning
 *   gLed1ode : Decide the LED1's meaning
 *   gLed2ode : Decide the LED2's meaning
 *   led01dual : 1 = LED0&1 is dual color LED
 *   led12dual : 1 = LED1&2 is dual color LED
 *   gLedSetNum : How many LEDs are controlled by RTL8231 serial mode,
 *                this value should be right, then the LED behavior will XXX
 */

#include <common/rt_autoconf.h>
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/debug/rt_log.h>
#include <common/util/rt_bitop.h>
#include <common/util/rt_util.h>
#include <osal/sem.h>
#include <osal/lib.h>
#include <osal/memory.h>
#include <osal/thread.h>
#include <ioal/mem32.h>
#include <osal/isr.h>
#include <osal/time.h>
#include <drv/swled/swctrl_led_main.h>
#include <drv/swcore/chip.h>

/*
 * Symbol Definition
 */
/* LED definition for customization */
#define LED_NUM_PER_PORT            (2)
#define LED_PORT_NUM_IN_SYSTEM      (52)
//#define LED_NUM_PER_PORT            (3)
//#define LED_PORT_NUM_IN_SYSTEM      (56)
#define LED_GPIO_DIRECTION_REG      (0xB8003508)
#define LED_GPIO_DATA_REG           (0xB800350C)
#define LED_GPIO_CLK_PIN_OFFSET     (11) /* Use the GPIO C3 as CLOCK pin */
#define LED_GPIO_DATA_PIN_OFFSET    (10) /* Use the GPIO C2 as DATA pin */
//#define LED_GPIO_CLK_PIN_OFFSET     (25) /* Use the GPIO A1 as CLOCK pin */
//#define LED_GPIO_DATA_PIN_OFFSET    (26) /* Use the GPIO A2 as DATA pin */
#define IS_LED_0_1_BICOLOR          (0)  /* 1: bi-color */
#define IS_LED_1_2_BICOLOR          (0)  /* 1: bi-color */
#define LED_0_MODE                  LED_MODE_100M_10M_LINK_ACT
#define LED_1_MODE                  LED_MODE_1000M_LINK_ACT
#define LED_2_MODE                  LED_MODE_NOT_USED
//#define LED_0_MODE                  LED_MODE_LINK_ACT
//#define LED_1_MODE                  LED_MODE_1000M_LINK
//#define LED_2_MODE                  LED_MODE_100M_10M_LINK

/* LED definition for code usage */
#define LED_SET_NUM             gLedSetNum
#define LED_MODE(ledId)         (*gpLedMode[ledId])
#define SWCTRL_LED_CTRL(chip)   swCtrl_led_ops[chip]

typedef struct rtk_switch_model_s {
     struct {
        uint8  offset;
        uint8  count;
        uint8  num;
        uint16 sel_p0_p23_led_mod;
        uint16 sel_p24_p27_led_mod;		
 		uint16 sel_led_group_3_0;
    } led; 
}rtk_swCtrl_led_model_t;


/*
 * Data Declaration
 */
extern swCtrl_led_mapper_operation_t swCtrl_led_ops[];

uint32 gLedSetNum;
uint32 gLed0Mode;
uint32 gLed1Mode;
uint32 gLed2Mode;
uint32 gPortLedNum;
uint32 *gpLedMode[] = {&gLed0Mode, &gLed1Mode, &gLed2Mode};

uint8  _port_collision[MAX_PHY_PORT];
uint64 _port_collision_num[MAX_PHY_PORT];

uint8  _led_act_blink = 0;
uint16 _led_block_blink_timer;
uint8  _port_link[MAX_PHY_PORT];
uint8  _led_rlpp_block[MAX_PHY_PORT];
uint8  _led_act[MAX_PHY_PORT];
uint8  _led_state[MAX_PHY_PORT][LED_NUM_PER_PORT];
uint64 _port_ifin[MAX_PHY_PORT];
uint64 _port_inUnderSize[MAX_PHY_PORT];
uint64 _port_inBadCRC[MAX_PHY_PORT];
uint64 _port_ifout[MAX_PHY_PORT];

uint8 _blocked_port_num_change;
uint8 _blocked_port_num;

uint8 led01dual, led12dual; /* decide which LED is dual color*/
uint32 ledSignal[LED_NUM_PER_PORT]; /* decide the output value for each LED, this is low active.*/
uint8 swCtrl_chipID;

const rtk_swCtrl_led_model_t *pBorad_Model;
const rtk_swCtrl_led_model_t *gSwitchModel = NULL;

static const rtk_swCtrl_led_model_t rtk_device_led_info = { 
	.led.offset = 0,
	.led.count = LED_PORT_NUM_IN_SYSTEM,
	.led.num = LED_NUM_PER_PORT,
	.led.sel_p0_p23_led_mod = 0x5470,
	.led.sel_p24_p27_led_mod = 0x5470,
	.led.sel_led_group_3_0 = 0x0021,
};

/* Function Name:
 *      board_led_info
 * Description:
 *      Get LED configuration code for device
 * Input:
 *      None
 * Output:
 *      Return LED Const Structure
 * Return:
 *      None
 * Note:
 *      None
 */
const rtk_swCtrl_led_model_t * board_led_info(void)
{ 
	gSwitchModel = &rtk_device_led_info;

    return (const rtk_swCtrl_led_model_t *)gSwitchModel;
} /* end of board_led_info */

/* Function Name: 
 *      swCtrl_led_allOff
 * Description: 
 *      Turn Off all LEDs
 * Input:  
 *      None
 * Output: 
 *      None 
 * Return: 
 *      RT_ERR_OK - initialize success
 * Note: 
 *      
 */
int32 swCtrl_led_allOff(void)
{	
    uint8 port, ledId;
    uint32 regData;
	
	for (port = 0; port < LED_SET_NUM; port ++)
	{			
		for(ledId = 0; ledId < LED_NUM_PER_PORT; ledId ++)
		{	PORT_LED_ONE(port, ledId);
			ledSignal[ledId] = PORT_LED_GET(port, ledId);
		}
			
		if(1 == led01dual && 1 == ledSignal[0] && 1 == ledSignal[1])
		{
			ledSignal[0] = ledSignal[1] = 0;
		}
		if(1 == led12dual && 1 == ledSignal[1] && 1 == ledSignal[2])
		{
			ledSignal[1] = ledSignal[2] = 0;
		}
		for(ledId = 0; ledId < LED_NUM_PER_PORT; ledId ++)
		{
			regData = READ_MEM32(LED_GPIO_DATA_REG);
			regData &= ~(1<<LED_GPIO_CLK_PIN_OFFSET);
			regData &= ~(1<<LED_GPIO_DATA_PIN_OFFSET);
        	WRITE_MEM32(LED_GPIO_DATA_REG, regData);
			regData |= (1<<LED_GPIO_CLK_PIN_OFFSET);
			regData |= (1<<LED_GPIO_DATA_PIN_OFFSET);
        	WRITE_MEM32(LED_GPIO_DATA_REG, regData);
			regData &= ~(1<<LED_GPIO_CLK_PIN_OFFSET);
			regData &= ~(1<<LED_GPIO_DATA_PIN_OFFSET);
        	WRITE_MEM32(LED_GPIO_DATA_REG, regData);
		}
	}
		
	return RT_ERR_OK;
    
} /* end of swCtrl_led_allOff()*/

/* Function Name: 
 *      swCtrl_led_init
 * Description: 
 *      [1] Check the Chip ID is supported or NOT?
 *      [2] Configure the LED mode.
 *      [3] Get LED Configuration from U-Boot
 * Input:  
 *      None
 * Output: 
 *      None 
 * Return: 
 *      RT_ERR_OK       - initialize success
 *      RT_ERR_CHIP_NOT_SUPPORTED - chip desn't support this feature
 * Note: 
 *      
 */ 
int32 swCtrl_led_init(void)
{
	uint32 regData; 
	int32 ret;	
	uint32 chip_id, chip_rev;

	ret = drv_swcore_cid_get(0, &chip_id, &chip_rev);
	if(ret == RT_ERR_OK)
	{
        if (CHIP_FAMILY_IS_RTL8350(chip_id) || 
            CHIP_FAMILY_IS_RTL8390(chip_id))
        {
            swCtrl_chipID = SWCTRL_LED_R8390;
        }
        else
        {
            osal_printf("Chip is NOT RTL8389 or 8390 series!!!\n");
            return RT_ERR_CHIP_NOT_SUPPORTED;
        }
	}	

	/* Port LED setting */
	gLedSetNum = LED_PORT_NUM_IN_SYSTEM;
	gLed0Mode = LED_0_MODE;	
	gLed1Mode = LED_1_MODE;
	gLed2Mode = LED_2_MODE;
	/* If led01dual = 1, LED 0&1 are a Dual Color LED
	 * If led01dual = 0, LED 0&1 are NOT a Dual Color LED
	 */
	led01dual = IS_LED_0_1_BICOLOR; 
	/* If led12dual = 1, LED 1&2 are a Dual Color LED
	 * If led12dual = 0, LED 1&2 are NOT a Dual Color LED
	 */
	led12dual = IS_LED_1_2_BICOLOR;
	/* How many LED for a port*/
	gPortLedNum = LED_NUM_PER_PORT;

	osal_time_mdelay(2500); /* delay 0.25 sec */

    /* Configure the 8390 use GPIO A1/A2 to output pin */
	regData = READ_MEM32(LED_GPIO_DIRECTION_REG); 
	regData |= (1<<LED_GPIO_CLK_PIN_OFFSET); /* GPIO C3 (CLK) set to output direction */
	regData |= (1<<LED_GPIO_DATA_PIN_OFFSET); /* GPIO C4 (DATA) set to output direction */
	WRITE_MEM32(LED_GPIO_DIRECTION_REG, regData); 

	ret = swCtrl_led_allOff();
	
    return RT_ERR_OK;
} /* end of swCtrl_led_init */ 

/* Function Name: 
 *      swCtrl_led_refresh_handler
 * Description: 
 *      Software Control LED function's entry point.
 *	  This API is called by Timer 1 interrupt call back function.
 * Input:  
 *      None
 * Output: 
 *      None 
 * Return: 
 *      RT_ERR_OK     - function is successed
 *      RT_ERR_FAILED - function is failed
 * Note: 
 *      
 */
int32 swCtrl_led_refresh_handler(void)
{
	int32 ret;
//	static uint32 counter=0;
//    uint32 time_before=0, time_after=0;

//    time_before = READ_MEM32(0xB8003108); /* get timer-0 */
	ret = led_refresh();
//    counter++;
//    time_after = READ_MEM32(0xB8003108); /* get timer-0 */
//    if ((counter % 19) == 0)
//        osal_printf("[%s:%d] time_before=0x%x, time_after=0x%x, diff=0x%x\n\r", __FUNCTION__, __LINE__, (time_before>>8), (time_after>>8), ((time_after-time_before)>>8));

    return ret;
} /* end of swCtrl_led_refresh_handler */ 

/* Function Name: 
 *      swCtrl_led_error_handler
 * Description: 
 *	  This API is called by Timer 1 interrupt call back function.
 *      This API is used to check interrupt bit status.
 * Input:  
 *      None
 * Output: 
 *      None 
 * Return: 
 *      RT_ERR_OK         - function is successed
 * Note: 
 *      
 */
int32 swCtrl_led_error_handler(uint8 index)
{
	osal_printf("\nswCtrl_led_error_handler() %u\n", index);
    return RT_ERR_OK;
} /* end of swCtrl_led_error_handler */ 

/* Function Name: 
 *      led_state_refresh
 * Description: 
 *	  This API will be called during Timer 1 interrupt.
 *      This API will updated all ports' status.
 * Input:  
 *      None
 * Output: 
 *      None 
 * Return: 
 *      RT_ERR_OK         - function is successed
 * Note: 
 *      
 */
int32 led_state_refresh(void)
{
    uint64 port_ifout_current = 0, port_ifin_current = 0, port_collision_current = 0;
    uint64 port_inUnderSize_current = 0, port_inBadCRC_current = 0;
    uint32 link, speed, duplex; 
    uint8 port, ledId;
    
    for (port = 0; port < LED_SET_NUM; port ++)
    {
		swCtrl_led_getPortLink(swCtrl_chipID, port, &link);
			
        if(_port_link[port] != link)
        {
            _port_link[port] = link;
            if(!link) /*link down*/
            {
				swCtrl_led_getMIBPortCounter(swCtrl_chipID, (int32)port, (uint32)ifOutOctets, &port_ifout_current);
                swCtrl_led_getMIBPortCounter(swCtrl_chipID, (int32)port, (uint32)ifInOctets, &port_ifin_current);
                swCtrl_led_getMIBPortCounter(swCtrl_chipID, (int32)port, (uint32)dot3StatsFCSErrors, &port_inBadCRC_current);
                swCtrl_led_getMIBPortCounter(swCtrl_chipID, (int32)port, (uint32)etherStatsUndersizePkts, &port_inUnderSize_current);
                swCtrl_led_getMIBPortCounter(swCtrl_chipID, (int32)port, (uint32)etherStatsCollisions, &port_collision_current);				
                _port_ifout[port] = port_ifout_current;
                _port_ifin[port] = port_ifin_current;
                _port_inUnderSize[port] = port_inUnderSize_current;
                _port_inBadCRC[port] = port_inBadCRC_current;
                _port_collision_num[port] = port_collision_current;

                for(ledId = 0; ledId < LED_NUM_PER_PORT; ledId ++)
                {
                    if(LED_MODE(ledId) != LED_MODE_NOT_USED)
                        PORT_LED_LIGHT_OFF(port, ledId);
                    else
                        PORT_LED_LIGHT_ON(port, ledId);
                }
                
                PORT_ACT_NO(port);
                PORT_COLLISION_NO(port);
            }
        }
        if(link)
        {     	
			swCtrl_led_getPortSpeedDuplex(swCtrl_chipID, port, &speed, &duplex);

            if(PORT_DUPLEX_FULL== duplex)
            {
                for(ledId = 0; ledId < LED_NUM_PER_PORT; ledId ++)
                {
                    switch(LED_MODE(ledId))
                    {
                        case LED_MODE_COL_FULL_DUPLEX:
                        case LED_MODE_FULL_DUPLEX:
                            PORT_LED_LIGHT_ON(port, ledId);
                            break;
                        default:
                            break;
                    }
                }
            }
            else
            {
                for(ledId = 0; ledId < LED_NUM_PER_PORT; ledId ++)
                {
                    switch(LED_MODE(ledId))
                    {
                        case LED_MODE_COL_FULL_DUPLEX:
                        case LED_MODE_FULL_DUPLEX:
                            PORT_LED_LIGHT_OFF(port, ledId);
                            break;
                        default:
                            break;
                    }
                }
            }
            
            if(PORT_SPEED_10 == speed)
            {
                for(ledId = 0; ledId < LED_NUM_PER_PORT; ledId ++)
                {
                    switch(LED_MODE(ledId))
                    {
                        case LED_MODE_LINK:
                        case LED_MODE_LINK_ACT:
                        case LED_MODE_100M_10M_LINK_ACT:
                        case LED_MODE_NOT_USED:
						case LED_MODE_100M_10M_LINK:							
                            PORT_LED_LIGHT_ON(port, ledId);
                            break;
                        case LED_MODE_100M_LINK:
                        case LED_MODE_1000M_LINK:
                        case LED_MODE_100M_LINK_ACT:
                        case LED_MODE_1000M_LINK_ACT:
                            PORT_LED_LIGHT_OFF(port, ledId);
                            break;
                        default:
                            break;
                    }
                }
            }
            else if(PORT_SPEED_100 == speed)
            {
                for(ledId = 0; ledId < LED_NUM_PER_PORT; ledId ++)
                {
                    switch(LED_MODE(ledId))
                    {
                        case LED_MODE_LINK:
                        case LED_MODE_LINK_ACT:
                        case LED_MODE_100M_10M_LINK_ACT:
                        case LED_MODE_100M_LINK:
                        case LED_MODE_100M_LINK_ACT:
                        case LED_MODE_NOT_USED:
						case LED_MODE_100M_10M_LINK:	
                            PORT_LED_LIGHT_ON(port, ledId);
                            break;
                        case LED_MODE_1000M_LINK:
                        case LED_MODE_1000M_LINK_ACT:
                            PORT_LED_LIGHT_OFF(port, ledId);
                            break;
                        default:
                            break;
                    }
                }
            }
            else 
            {
                for(ledId = 0; ledId < LED_NUM_PER_PORT; ledId ++)
                {
                    switch(LED_MODE(ledId))
                    {
                        case LED_MODE_LINK:
                        case LED_MODE_LINK_ACT:
                        case LED_MODE_1000M_LINK:
                        case LED_MODE_1000M_LINK_ACT:
                        case LED_MODE_NOT_USED:                            
                            PORT_LED_LIGHT_ON(port, ledId);
                            break;
                        case LED_MODE_100M_10M_LINK_ACT:
                        case LED_MODE_100M_LINK:
                        case LED_MODE_100M_LINK_ACT:
						case LED_MODE_100M_10M_LINK:							
                            PORT_LED_LIGHT_OFF(port, ledId);
                            break;
                        default:
                            break;
                    }
                }
            }

            swCtrl_led_getMIBPortCounter(swCtrl_chipID, port, (uint32)ifOutOctets, &port_ifout_current);
            swCtrl_led_getMIBPortCounter(swCtrl_chipID, port, (uint32)ifInOctets, &port_ifin_current);
            swCtrl_led_getMIBPortCounter(swCtrl_chipID, port, (uint32)dot3StatsFCSErrors, &port_inBadCRC_current);
            swCtrl_led_getMIBPortCounter(swCtrl_chipID, port, (uint32)etherStatsUndersizePkts, &port_inUnderSize_current);      
            if (_port_ifout[port] != port_ifout_current || _port_ifin[port] != port_ifin_current 
                || _port_inUnderSize[port] != port_inUnderSize_current || _port_inBadCRC[port] != port_inBadCRC_current)
            {
                PORT_ACT_YES(port);
                for(ledId = 0; ledId < LED_NUM_PER_PORT; ledId ++)
                {
                    switch(LED_MODE(ledId))
                    {
                        case LED_MODE_ACT:
                            PORT_LED_LIGHT_ON(port, ledId);
                            break;
                        default:
                            break;
                    }
                }                
            }
            else
            {
                PORT_ACT_NO(port);

                for(ledId = 0; ledId < LED_NUM_PER_PORT; ledId ++)
                {
                    switch(LED_MODE(ledId))
                    {
                        case LED_MODE_ACT:
                            PORT_LED_LIGHT_OFF(port, ledId);
                            break;
                        default:
                            break;
                    }
                }
            }

            if(duplex == PORT_DUPLEX_HALF)
            {
                swCtrl_led_getMIBPortCounter(swCtrl_chipID, port, (uint32)etherStatsCollisions, &port_collision_current);
	
                if (_port_collision_num[port] != port_collision_current)
                {
                    PORT_COLLISION_YES(port);
                }
                else
                    PORT_COLLISION_NO(port);

                _port_collision_num[port] = port_collision_current;
            }
            else
                PORT_COLLISION_NO(port);

            _port_ifout[port] = port_ifout_current;
            _port_ifin[port] = port_ifin_current;
            _port_inUnderSize[port] = port_inUnderSize_current;
            _port_inBadCRC[port] = port_inBadCRC_current;
        }
    }
    return RT_ERR_OK;
} /* end of led_state_refresh */

/* Function Name: 
 *      led_refresh
 * Description: 
 *	  This API will be called during Timer 1 interrupt.
 *      This API is used to output the LED control signals.
 * Input:  
 *      None
 * Output: 
 *      None 
 * Return: 
 *      RT_ERR_OK         - function is successed
 * Note: 
 *      
 */
int32 led_refresh(void)
{
    uint8 port, ledId;
    uint32 regData;
   
    _led_act_blink = ~_led_act_blink;

    if (_led_act_blink)
        led_state_refresh();
	
    for (port = 0; port < LED_SET_NUM; port ++)
    {
		if(PORT_ACT_GET(port) && _led_act_blink) /*Light off active LED*/
        {
            regData = READ_MEM32(LED_GPIO_DATA_REG);

            if(PORT_COLLISION_GET(port)) /*blink if collision*/
            {
                for(ledId = 0; ledId < LED_NUM_PER_PORT; ledId ++)
                {
                    switch(LED_MODE(ledId))
                    {
                        case LED_MODE_COL_FULL_DUPLEX:
                            ledSignal[ledId] = 1;
                            break;
                        default:
                            break;
                    }
                }
            }
            
            for(ledId = 0; ledId < LED_NUM_PER_PORT; ledId ++)
            {
                switch(LED_MODE(ledId))
                {
                    case LED_MODE_ACT:
                    case LED_MODE_LINK_ACT:
                    case LED_MODE_100M_LINK_ACT:
                    case LED_MODE_1000M_LINK_ACT:
                    case LED_MODE_100M_10M_LINK_ACT:
                        ledSignal[ledId] = 1;
                        break;
                    default:
                        ledSignal[ledId] = PORT_LED_GET(port, ledId);
                        break;
                }
            }

            if(1 == led01dual && 1 == ledSignal[0] && 1 == ledSignal[1])
            {
                ledSignal[0] = ledSignal[1] = 0;
            }
            if(1 == led12dual && 1 == ledSignal[1] && 1 == ledSignal[2])
            {
                ledSignal[1] = ledSignal[2] = 0;
            }

            for(ledId = 0; ledId < LED_NUM_PER_PORT; ledId ++)
            {
    			regData = READ_MEM32(LED_GPIO_DATA_REG);
    			regData &= ~(1<<LED_GPIO_CLK_PIN_OFFSET);
    			regData &= ~(1<<LED_GPIO_DATA_PIN_OFFSET);
            	WRITE_MEM32(LED_GPIO_DATA_REG, regData);
    			regData |= (1<<LED_GPIO_CLK_PIN_OFFSET);
    			regData |= (ledSignal[ledId]<<LED_GPIO_DATA_PIN_OFFSET);
            	WRITE_MEM32(LED_GPIO_DATA_REG, regData);
    			regData &= ~(1<<LED_GPIO_CLK_PIN_OFFSET);
    			regData &= ~(1<<LED_GPIO_DATA_PIN_OFFSET);
            	WRITE_MEM32(LED_GPIO_DATA_REG, regData);
            }
        }
        else /*Light on*/
        {
            for(ledId = 0; ledId < LED_NUM_PER_PORT; ledId ++)
                ledSignal[ledId] = PORT_LED_GET(port, ledId);
          
            if(1 == led01dual && 1 == ledSignal[0] && 1 == ledSignal[1])
            {
                ledSignal[0] = ledSignal[1] = 0;
            }
            if(1 == led12dual && 1 == ledSignal[1] && 1 == ledSignal[2])
            {
                ledSignal[1] = ledSignal[2] = 0;
            }

            for(ledId = 0; ledId < LED_NUM_PER_PORT; ledId ++)
            {
    			regData = READ_MEM32(LED_GPIO_DATA_REG);
    			regData &= ~(1<<LED_GPIO_CLK_PIN_OFFSET);
    			regData &= ~(1<<LED_GPIO_DATA_PIN_OFFSET);
            	WRITE_MEM32(LED_GPIO_DATA_REG, regData);
    			regData |= (1<<LED_GPIO_CLK_PIN_OFFSET);
    			regData |= (ledSignal[ledId]<<LED_GPIO_DATA_PIN_OFFSET);
            	WRITE_MEM32(LED_GPIO_DATA_REG, regData);
    			regData &= ~(1<<LED_GPIO_CLK_PIN_OFFSET);
    			regData &= ~(1<<LED_GPIO_DATA_PIN_OFFSET);
            	WRITE_MEM32(LED_GPIO_DATA_REG, regData);
            }            
        }
    }

	regData = READ_MEM32(LED_GPIO_DATA_REG);
	regData &= ~(1<<LED_GPIO_CLK_PIN_OFFSET);
	regData &= ~(1<<LED_GPIO_DATA_PIN_OFFSET);
	WRITE_MEM32(LED_GPIO_DATA_REG, regData);

    return RT_ERR_OK;
} /* end of led_refresh */

/* Function Name: 
 *		swCtrl_led_getPortSpeedDuplex
 * Description: 
 *		Get the specified port's speed and duplex status.
 * Input:  
 *		chipID	- the convert Chip ID.
 *		port		- the specified port
 * Output: 
 *		
 *		  pSpeed	   - the link status of specified poty. 
 *                              10M = 0; 100M = 1; 1000M =2 
 *		  pDuplex	   - the link status of specified poty.
 *					FULL = 1; HALF = 0
 * Return: 
 *		RT_ERR_OK		  - function is successed
 *		RT_ERR_FAILED	  - function is failed
 * Note: 
 *		  
 */
int32 swCtrl_led_getPortSpeedDuplex(uint8 chipID, int32 port, uint32 *pSpeed, uint32 *pDuplex)
{
	
	if(RT_ERR_OK != swCtrl_led_ops[chipID].getPortSpeedDuplex(port, pSpeed, pDuplex))
		return RT_ERR_FAILED;

    return RT_ERR_OK;
} /* end of swCtrl_led_getPortSpeedDuplex */

/* Function Name: 
 *		swCtrl_led_getPortLink
 * Description: 
 *		Get the specified port's link status.
 * Input:  
 *		chipID	- the convert Chip ID.
 *		port		- the specified port
 * Output: 
 *		
 *	      linkSts 	   - the link status of specified poty.
 * Return: 
 *		RT_ERR_OK		  - function is successed
 *		RT_ERR_FAILED	  - function is failed
 * Note: 
 *		  
 */
int32 swCtrl_led_getPortLink(uint8 chipID, int32 port, uint32 *linkSts)
{	

	if(RT_ERR_OK != swCtrl_led_ops[chipID].getPortLink(port, linkSts))
		return RT_ERR_FAILED;

    return RT_ERR_OK;
} /* end of  swCtrl_led_getPortLink */ 

/* Function Name: 
 *      swCtrl_led_getAsicReg
 * Description: 
 *      Set bits value of a specified register.
 * Input:  
 *      reg 		- Register's address.
 * Output: 
 *      
 *	  value	       - Register's value.
 * Return: 
 *      RT_ERR_OK         - function is successed
 * Note: 
 *        
 */
int32 swCtrl_led_getAsicReg(uint32 reg, uint32 *val)
{
    *val = READ_MEM32(reg);
    return RT_ERR_OK;
} /* end of swCtrl_led_getAsicReg */

/* Function Name: 
 *      swCtrl_ledsetAsicRegBits
 * Description: 
 *      Set bits value of a specified register.
 * Input:  
 *      reg 		- Register's address.
 *	  bits		- Bits mask for setting.
 *  	  value		- Bits value for setting. Value of bits will be set with mapping mask bit is 1.
 * Output: 
 *      None 
 * Return: 
 *      RT_ERR_OK         - function is successed
 * Note: 
 *      Set bits of a specified register to value. Both bits and value are be treated as bit-mask.
 *        
 */
int32 swCtrl_led_setAsicRegBits(uint32 reg, uint32 bits, uint32 value)
{
    uint32 regData;

    regData = READ_MEM32(reg);

    regData = regData & (~bits);
    regData = regData | (value & bits);

    WRITE_MEM32(reg, regData);
    return RT_ERR_OK;
} /* end of swCtrl_led_setAsicRegBits */

/* Function Name: 
 *      swCtrl_led_getMIBPortCounter
 * Description: 
 *      Set bits value of a specified register.
 * Input:  
 *      chipID  - the convert Chip ID.
 *	    port    - the specified port.
 *	    mibIdx  - the specified MIB counter index.
 * Output: 
 *      counter - MIB counter value. 
 * Return: 
 *      RT_ERR_OK - function is successed
 * Note: 
 *        
 */
int32 swCtrl_led_getMIBPortCounter(uint8 chipID, int32 port, uint32 mibIdx, uint64* counter)
{
    if ((RT_ERR_OK !=  swCtrl_led_ops[chipID].getMIBPortCounter(port, mibIdx, counter)) != RT_ERR_OK)
        return RT_ERR_FAILED;

    return RT_ERR_OK;
} /* end of swCtrl_led_getMIBPortCounter */
