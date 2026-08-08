#
# Copyright (C) 2011 OpenWrt.org
#

PART_NAME=firmware
REQUIRE_IMAGE_METADATA=1

redboot_fis_do_upgrade() {
	local append
	local sysup_file="$1"
	local kern_part="$2"
	local magic=$(get_magic_word "$sysup_file")

	if [ "$magic" = "4349" ]; then
		local kern_length=0x$(dd if="$sysup_file" bs=2 skip=1 count=4 2>/dev/null)

		[ -f "$UPGRADE_BACKUP" ] && append="-j $UPGRADE_BACKUP"
		dd if="$sysup_file" bs=64k skip=1 2>/dev/null | \
			mtd -r $append -F$kern_part:$kern_length:0x80060000,rootfs write - $kern_part:rootfs

	elif [ "$magic" = "7379" ]; then
		local board_dir=$(tar tf $sysup_file | grep -m 1 '^sysupgrade-.*/$')
		local kern_length=$(tar xf $sysup_file ${board_dir}kernel -O | wc -c)

		[ -f "$UPGRADE_BACKUP" ] && append="-j $UPGRADE_BACKUP"
		tar xf $sysup_file ${board_dir}kernel ${board_dir}root -O | \
			mtd -r $append -F$kern_part:$kern_length:0x80060000,rootfs write - $kern_part:rootfs

	else
		echo "Unknown image, aborting!"
		return 1
	fi
}

tplink_restore_partition(){
	local _sysfile="$1"
	local _sizesf=$(wc -c "$_sysfile" | awk '{print $1}')
	local _partsize=$(printf %d 0x$(cat /proc/mtd | grep firmware| awk '{print $2}'))
	local _cmd

	if [ $_sizesf -gt $_partsize ]
	then
		dd if="$_sysfile" of="/tmp/tpl_file" bs=1 skip=$_partsize 2>/dev/null
		_cmd="dd bs=1k count=$(( _partsize / 1024 ))"
		mtd write /tmp/tpl_file tplink
	fi
	default_do_upgrade "$_sysfile" "$_cmd"
}

platform_check_image() {
	return 0
}

platform_do_upgrade() {
	local board=$(board_name)

	case "$board" in
	jjplus,ja76pf2)
		redboot_fis_do_upgrade "$1" linux
		;;
	ubnt,routerstation|\
	ubnt,routerstation-pro)
		redboot_fis_do_upgrade "$1" kernel
		;;
	tplink,archer-c6-v2-us)
		tplink_restore_partition "$1"
		;;
	*)
		default_do_upgrade "$1"
		;;
	esac
}
