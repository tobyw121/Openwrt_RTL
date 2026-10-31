PART_NAME=firmware
REQUIRE_IMAGE_METADATA=1

RAMFS_COPY_BIN='fw_printenv fw_setenv'
RAMFS_COPY_DATA='/etc/fw_env.config /var/lock/fw_printenv.lock'

# The TP-Link bootloader gets its flash layout from "bootargs".
# Use this to our advantage, and:
#  1. Disable the second rootfs (usrimg2)
#  2. Extend the first rootfs (usrimg1) to include more space
#  3. Increase the baudrate to the expected 115200
tplink_sg2xxx_fix_mtdparts() {
	local args
	args="bootargs mtdparts=spi_flash:896K(boot),128K(env),6144K(sys)"
	args="$args,20480K(usrimg1@main),0K(usrimg2)"
	args="$args,4096K(usrappfs),1024K(para)\n"
	args="$args baudrate 115200"

	echo -e "$args" | fw_setenv --script -
}

platform_check_image() {
	case "$(board_name)" in
	cudy,wr1500|cudy,re1500|cudy,m1500|cudy,ax1500-r76)
		echo 'Cudy AX1500: RAM bring-up only; flash layout is not validated.' >&2
		return 1
		;;
	esac
	return 0
}

platform_do_upgrade() {
        local board=$(board_name)

        case "$board" in
        cudy,wr1500|cudy,re1500|cudy,m1500|cudy,ax1500-r76)
                echo 'Cudy AX1500: flash upgrades are not implemented.' >&2
                return 1
                ;;
        xiaomi,r4-rd05|\
        tenda,ac23|\
	tenda,lynx-rtl8197f)
                PART_NAME="kernel:rootfs"
                ;;
	tenda,nova-mw5)
                PART_NAME="firmware"
                ;;
	iball,wrd12gn)
		PART_NAME="firmware"
		;;
        esac

        case "$board" in
        tplink,sg2008p-v1|\
        tplink,sg2210p-v3)
                tplink_sg2xxx_fix_mtdparts
		default_do_upgrade "$1"
		;;
	*)
		default_do_upgrade "$1"
		;;
	esac
}
