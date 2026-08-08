#ifndef __SWCTRL_LED_H__
#define __SWCTRL_LED_H__



/*for LED*/
#define MAX_PHY_PORT 	           (56)    /* number of physical ports */
#define MAX_PORT_ID                (MAX_PHY_PORT)
#define MIN_PORT_ID                (0)

#define REG_WIDTH 			        32

#define SWLED_REFLASH_FREQ 			CONFIG_SDK_SOFTWARE_CONTROL_LED_RATE      /* Reflash time is (10mS * SWLED_REFLASH_FREQ)*/

#define PORT_COLLISION_GET(port) (_port_collision[port])
#define PORT_COLLISION_YES(port) (_port_collision[port] = 1)
#define PORT_COLLISION_NO(port) (_port_collision[port] = 0)

#define PORT_BLOCK_GET(port) (_led_rlpp_block[port])
#define PORT_BLOCK_YES(port) (_led_rlpp_block[port] = 1)
#define PORT_BLOCK_NO(port) (_led_rlpp_block[port] = 0)
#define PORT_ACT_GET(port) (_led_act[port])
#define PORT_ACT_YES(port) (_led_act[port] = 1)
#define PORT_ACT_NO(port) (_led_act[port] = 0)

#define PORT_LED_GET(port, ledId) (_led_state[port][ledId])
#define PORT_LED_ONE(port, ledId) (_led_state[port][ledId] = 1)
#define PORT_LED_ZERO(port, ledId) (_led_state[port][ledId] = 0)
#define PORT_LED_LIGHT_ON(port, ledId) PORT_LED_ZERO(port, ledId)
#define PORT_LED_LIGHT_OFF(port, ledId) PORT_LED_ONE(port, ledId)

typedef enum {
    LED_MODE_LINK_ACT,
	LED_MODE_LINK,
	LED_MODE_ACT,
	LED_MODE_NOT_USED,
	LED_MODE_100M_10M_LINK,
	LED_MODE_COL_FULL_DUPLEX,
	LED_MODE_FULL_DUPLEX,
	LED_MODE_1000M_LINK,
	LED_MODE_100M_LINK,
	LED_MODE_10M_LINK,
	LED_MODE_1000M_LINK_ACT,
	LED_MODE_100M_LINK_ACT,
	LED_MODE_10M_LINK_ACT,
	LED_MODE_1000M_100M_LINK_ACT,
	LED_MODE_1000M_10M_LINK_ACT,
	LED_MODE_100M_10M_LINK_ACT,
	LED_MODE_NOT_USED2
} sal_led_mode;


enum RTL8389_MIB_PORT_COUNTER{
    ifOutOctets  = 0,
    ifInOctets,
    dot3StatsFCSErrors,
    etherStatsCollisions, /* 15 */
    etherStatsUndersizePkts,
    MAX_MIB_PORT_COUNTER,
};

typedef struct rtk_MIBCounterInfo_s
{
    uint32 regAddr;
    uint32 regMsk;
    uint32 regOffset;
    uint8 bitNum;   /*The bit number this counter used*/
} rtk_MIBCounterInfo_t;


typedef enum swCtrl_led_controller_e
{
    SWCTRL_LED_R8390,
    SWCTRL_LED_CHIP_END            
} swCtrl_led_controller_t;

int32 led_state_refresh(void);
int32 led_refresh(void);
int32 swCtrl_led_init(void);
int32 swCtrl_led_refresh_handler(void);
int32 swCtrl_led_error_handler(uint8 index);
int32 swCtrl_led_getAsicReg(uint32 reg, uint32 *val);
int32 swCtrl_led_setAsicRegBits(uint32 reg, uint32 bits, uint32 value);
int32 swCtrl_led_getPortSpeedDuplex(uint8 chipID, int32 port, uint32 *pSpeed, uint32 *pDuplex);
int32 swCtrl_led_getPortLink(uint8 chipID, int32 port, uint32 *linkSts);
int32 swCtrl_led_getMIBPortCounter(uint8 chipID, int32 port, uint32 mibIdx, uint64* counter);

int32 rtl8390_getAsicMIBPortCounter(int32 port, uint32 mibIdx, uint64* counter);
int32 rtl8390_getAsicPortSpeedDuplex(int32 port, uint32 *pSpeed, uint32 *pDuplex);
int32 rtl8390_getAsicPortLinkStat(int32 port, uint32 *pLinkStat);

typedef struct swCtrl_led_mapper_operation_s
{
    int32   (*getPortSpeedDuplex)(int32, uint32 *, uint32 *);	
    int32   (*getPortLink)(int32, uint32 *);
    int32   (*getMIBPortCounter)(int32, uint32, uint64 *);

} swCtrl_led_mapper_operation_t;

#define PORT_DUPLEX_FULL           (1)
#define PORT_DUPLEX_HALF           (0)    
#define PORT_SPEED_10			   (0)	
#define PORT_SPEED_100             (1)
#define PORT_SPEED_1000            (2)

#define PORT_LINK_DOWN             (0)

#endif

