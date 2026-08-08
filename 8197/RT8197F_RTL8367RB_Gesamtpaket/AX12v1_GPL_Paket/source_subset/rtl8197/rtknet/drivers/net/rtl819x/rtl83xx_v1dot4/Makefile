#
# Makefile for the Tulip ethernet driver
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
# Note 2! The CFLAGS definitions are now in the main makefile...
#Add mips16 Support

DIR_RTLASIC = $(DIR_LINUX)/drivers/net/rtl819x
DIR_83XX = $(DIR_RTLASIC)/rtl83xx_v1dot4

# customer may need to support 8367d only for shrinking code size
#   please MARK "RTL8367C_SUPPORT = y" for supporting 8367d only
RTL8367C_SUPPORT = y

obj-y := \
    rtk_api.o \
    chip.o \
    cpu.o \
    l2.o \
    led.o \
    port.o \
    rtk_switch.o \
    acl.o \
    dot1x.o \
    eee.o \
    i2c.o \
    igmp.o \
    interrupt.o \
    leaky.o \
    mirror.o \
    oam.o \
    ptp.o \
    qos.o \
    rate.o \
    rldp.o \
    stat.o \
    storm.o \
    svlan.o \
    trap.o \
    trunk.o \
    vlan.o \
    dal/dal_mgmt.o \
    dal/rtl8367c/rtl8367c_asicdrv.o \
    dal/rtl8367c/rtl8367c_asicdrv_lut.o \
    dal/rtl8367c/rtl8367c_asicdrv_phy.o \
    dal/rtl8367d/rtl8367d_asicdrv.o \
    dal/rtl8367d/dal_rtl8367d_cpu.o \
    dal/rtl8367d/dal_rtl8367d_l2.o \
    dal/rtl8367d/dal_rtl8367d_led.o \
    dal/rtl8367d/dal_rtl8367d_mapper.o \
    dal/rtl8367d/dal_rtl8367d_port.o \
    dal/rtl8367d/dal_rtl8367d_switch.o \
    dal/rtl8367d/dal_rtl8367d_acl.o \
    dal/rtl8367d/dal_rtl8367d_dot1x.o \
    dal/rtl8367d/dal_rtl8367d_eee.o \
    dal/rtl8367d/dal_rtl8367d_igmp.o \
    dal/rtl8367d/dal_rtl8367d_interrupt.o \
    dal/rtl8367d/dal_rtl8367d_leaky.o \
    dal/rtl8367d/dal_rtl8367d_mirror.o \
    dal/rtl8367d/dal_rtl8367d_qos.o \
    dal/rtl8367d/dal_rtl8367d_rate.o \
    dal/rtl8367d/dal_rtl8367d_rldp.o \
    dal/rtl8367d/dal_rtl8367d_stat.o \
    dal/rtl8367d/dal_rtl8367d_storm.o \
    dal/rtl8367d/dal_rtl8367d_svlan.o \
    dal/rtl8367d/dal_rtl8367d_trap.o \
    dal/rtl8367d/dal_rtl8367d_trunk.o \
    dal/rtl8367d/dal_rtl8367d_vlan.o

obj-$(RTL8367C_SUPPORT) += \
    dal/rtl8367c/dal_rtl8367c_cpu.o \
    dal/rtl8367c/dal_rtl8367c_l2.o \
    dal/rtl8367c/dal_rtl8367c_led.o \
    dal/rtl8367c/dal_rtl8367c_mapper.o \
    dal/rtl8367c/dal_rtl8367c_port.o \
    dal/rtl8367c/dal_rtl8367c_switch.o \
    dal/rtl8367c/dal_rtl8367c_acl.o \
    dal/rtl8367c/dal_rtl8367c_dot1x.o \
    dal/rtl8367c/dal_rtl8367c_eee.o \
    dal/rtl8367c/dal_rtl8367c_i2c.o \
    dal/rtl8367c/dal_rtl8367c_igmp.o \
    dal/rtl8367c/dal_rtl8367c_interrupt.o \
    dal/rtl8367c/dal_rtl8367c_leaky.o \
    dal/rtl8367c/dal_rtl8367c_mirror.o \
    dal/rtl8367c/dal_rtl8367c_oam.o \
    dal/rtl8367c/dal_rtl8367c_ptp.o \
    dal/rtl8367c/dal_rtl8367c_qos.o \
    dal/rtl8367c/dal_rtl8367c_rate.o \
    dal/rtl8367c/dal_rtl8367c_rldp.o \
    dal/rtl8367c/dal_rtl8367c_stat.o \
    dal/rtl8367c/dal_rtl8367c_storm.o \
    dal/rtl8367c/dal_rtl8367c_svlan.o \
    dal/rtl8367c/dal_rtl8367c_trap.o \
    dal/rtl8367c/dal_rtl8367c_trunk.o \
    dal/rtl8367c/dal_rtl8367c_vlan.o \
    dal/rtl8367c/rtl8367c_asicdrv_cputag.o \
    dal/rtl8367c/rtl8367c_asicdrv_inbwctrl.o \
    dal/rtl8367c/rtl8367c_asicdrv_led.o \
    dal/rtl8367c/rtl8367c_asicdrv_mirror.o \
    dal/rtl8367c/rtl8367c_asicdrv_misc.o \
    dal/rtl8367c/rtl8367c_asicdrv_port.o \
    dal/rtl8367c/rtl8367c_asicdrv_rma.o \
    dal/rtl8367c/rtl8367c_asicdrv_scheduling.o \
    dal/rtl8367c/rtl8367c_asicdrv_portIsolation.o \
    dal/rtl8367c/rtl8367c_asicdrv_acl.o \
    dal/rtl8367c/rtl8367c_asicdrv_dot1x.o \
    dal/rtl8367c/rtl8367c_asicdrv_eav.o \
    dal/rtl8367c/rtl8367c_asicdrv_eee.o \
    dal/rtl8367c/rtl8367c_asicdrv_fc.o \
    dal/rtl8367c/rtl8367c_asicdrv_green.o \
    dal/rtl8367c/rtl8367c_asicdrv_hsb.o \
    dal/rtl8367c/rtl8367c_asicdrv_i2c.o \
    dal/rtl8367c/rtl8367c_asicdrv_igmp.o \
    dal/rtl8367c/rtl8367c_asicdrv_interrupt.o \
    dal/rtl8367c/rtl8367c_asicdrv_meter.o \
    dal/rtl8367c/rtl8367c_asicdrv_mib.o \
    dal/rtl8367c/rtl8367c_asicdrv_oam.o \
    dal/rtl8367c/rtl8367c_asicdrv_qos.o \
    dal/rtl8367c/rtl8367c_asicdrv_rldp.o \
    dal/rtl8367c/rtl8367c_asicdrv_storm.o \
    dal/rtl8367c/rtl8367c_asicdrv_svlan.o \
    dal/rtl8367c/rtl8367c_asicdrv_trunking.o \
    dal/rtl8367c/rtl8367c_asicdrv_unknownMulticast.o \
    dal/rtl8367c/rtl8367c_asicdrv_vlan.o

#EXTRA_CFLAGS += -O1 -DRTL_TBLDRV -D__linux__  -mno-memcpy -DRTL865X_OVER_KERNEL -DRTL865X_OVER_LINUX -Werror
EXTRA_CFLAGS += -O1 -DRTL_TBLDRV -D__linux__  -mno-memcpy -DRTL865X_OVER_KERNEL -DRTL865X_OVER_LINUX -Wno-implicit
EXTRA_CFLAGS += -I$(DIR_83XX) -I$(DIR_83XX)/dal/rtl8367c -I$(DIR_83XX)/dal/rtl8367d

ifeq ($(RTL8367C_SUPPORT),y)
else
EXTRA_CFLAGS += -DCONFIG_DAL_RTL8367D
endif

EXTRA_AFLAGS += $(EXTRA_CFLAGS)

