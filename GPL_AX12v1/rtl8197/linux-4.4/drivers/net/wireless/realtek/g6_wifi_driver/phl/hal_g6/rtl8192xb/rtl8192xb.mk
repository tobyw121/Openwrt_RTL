# EXTRA_CFLAGS += -DCONFIG_RTL8192XB
IC_NAME_92XB := rtl8192xb

ifeq ($(CONFIG_PHL_ARCH), y)
HAL = phl/hal_g6
else
HAL = hal_g6
endif

#ifeq ($(CONFIG_USB_HCI), y)
#	FILE_NAME_92XB = rtl8192xbu
#endif
ifeq ($(CONFIG_PCI_HCI), y)
	FILE_NAME_92XB = rtl8192xbe
endif
#ifeq ($(CONFIG_SDIO_HCI), y)
#	FILE_NAME_92XB = rtl8192xbs
#endif


_HAL_IC_FILES +=	$(HAL)/$(IC_NAME_92XB)/$(IC_NAME_92XB)_halinit.o \
			$(HAL)/$(IC_NAME_92XB)/$(IC_NAME_92XB)_mac.o \
			$(HAL)/$(IC_NAME_92XB)/$(IC_NAME_92XB)_cmd.o \
			$(HAL)/$(IC_NAME_92XB)/$(IC_NAME_92XB)_phy.o \
			$(HAL)/$(IC_NAME_92XB)/$(IC_NAME_92XB)_ops.o \
			$(HAL)/$(IC_NAME_92XB)/hal_trx_8192xb.o

_HAL_IC_FILES +=	$(HAL)/$(IC_NAME_92XB)/$(HCI_NAME)/$(FILE_NAME_92XB)_halinit.o \
			$(HAL)/$(IC_NAME_92XB)/$(HCI_NAME)/$(FILE_NAME_92XB)_io.o \
			$(HAL)/$(IC_NAME_92XB)/$(HCI_NAME)/$(FILE_NAME_92XB)_led.o \
			$(HAL)/$(IC_NAME_92XB)/$(HCI_NAME)/$(FILE_NAME_92XB)_ops.o

#ifeq ($(CONFIG_SDIO_HCI), y)
#_HAL_IC_FILES += $(HAL)/$(IC_NAME_92XB)/$(HCI_NAME)/hal_trx_8192xbs.o
#endif

#ifeq ($(CONFIG_USB_HCI), y)
#_HAL_IC_FILES += $(HAL)/$(IC_NAME_92XB)/$(HCI_NAME)/hal_trx_8192xbu.o
#endif

ifeq ($(CONFIG_PCI_HCI), y)
_HAL_IC_FILES += $(HAL)/$(IC_NAME_92XB)/$(HCI_NAME)/hal_trx_8192xbe.o
endif
