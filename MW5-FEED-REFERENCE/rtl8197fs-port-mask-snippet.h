#define GPIO_RESET_97FN			BSP_GPIO_PIN_F1		// GPIO_F1
#endif
#define PORT0_RGMII_PHYID		6
#endif
#endif

#define GPIO_RESET_8197FS_83XX		26		// GPIO_H2
#define GPIO_RESET_8197FH_83XX		9		// GPIO_F1

#if defined(CONFIG_RTL_8197F)
int32 rtl8651_enableCPUdtxHP(int enable);
#endif

#define RTL8197FS_FE_PORT_MASK                  (PM_PORT_4)
#define RTL8197FS_VG_FE_PORT_MASK               (PM_PORT_1)

#define RTL8197FS_NOT_EXIST_PORTS_MASK          (PM_PORT_1|PM_PORT_2|PM_PORT_3)
#define RTL8197FS_VG_NOT_EXIST_PORTS_MASK       (PM_PORT_2|PM_PORT_3|PM_PORT_4)

/* pure 8197FS, no 8211F or 83xx */
#define RTL8197FS_LAN_PORT_MASK                 (RTL8197FS_FE_PORT_MASK|PM_PORT_8)
#define RTL8197FS_VG_LAN_PORT_MASK              (RTL8197FS_VG_FE_PORT_MASK|PM_PORT_8)

#endif
