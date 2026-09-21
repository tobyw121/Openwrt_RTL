// SPDX-License-Identifier: GPL-2.0-only
/*
 * Realtek RTL8197F cr6c/cs6c firmware parser.
 *
 * The 16-byte IMG_HEADER_T contains a big-endian length at offset 12.  On
 * Tenda RTL8197F images that length includes the optional bytes between the
 * logical 16-byte header and the executable payload, plus the final checksum.
 * The SquashFS image starts exactly at 0x10 + len.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/byteorder/generic.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/of.h>
#include <linux/slab.h>

#include "mtdsplit.h"

#define RTL8197F_NR_PARTS 2

struct rtl8197f_img_header {
	u8 signature[4];
	__be32 start_addr;
	__be32 burn_addr;
	__be32 length;
} __packed;

static bool rtl8197f_valid_signature(const u8 signature[4])
{
	return !memcmp(signature, "cr6c", 4) ||
	       !memcmp(signature, "cs6c", 4) ||
	       !memcmp(signature, "csys", 4);
}

static int mtdsplit_parse_rtl8197f(struct mtd_info *master,
				  const struct mtd_partition **pparts,
				  struct mtd_part_parser_data *data)
{
	struct rtl8197f_img_header hdr;
	struct mtd_partition *parts;
	size_t retlen;
	u64 rootfs_offset;
	u32 image_len;
	int err;

	err = mtd_read(master, 0, sizeof(hdr), &retlen, (void *)&hdr);
	if (err)
		return err;
	if (retlen != sizeof(hdr))
		return -EIO;
	if (!rtl8197f_valid_signature(hdr.signature))
		return -EINVAL;

	image_len = be32_to_cpu(hdr.length);
	rootfs_offset = sizeof(hdr) + (u64)image_len;
	if (image_len < 2 || rootfs_offset >= master->size)
		return -EINVAL;

	err = mtd_check_rootfs_magic(master, rootfs_offset, NULL);
	if (err)
		return err;

	parts = kcalloc(RTL8197F_NR_PARTS, sizeof(*parts), GFP_KERNEL);
	if (!parts)
		return -ENOMEM;

	parts[0].name = KERNEL_PART_NAME;
	parts[0].offset = 0;
	parts[0].size = rootfs_offset;

	parts[1].name = ROOTFS_PART_NAME;
	parts[1].offset = rootfs_offset;
	parts[1].size = master->size - rootfs_offset;

	pr_info("rtl8197f-fw: %s split kernel=0x%llx rootfs=0x%llx\n",
		master->name, rootfs_offset, rootfs_offset);

	*pparts = parts;
	return RTL8197F_NR_PARTS;
}

static const struct of_device_id mtdsplit_rtl8197f_of_match_table[] = {
	{ .compatible = "realtek,rtl8197f-firmware" },
	{},
};
MODULE_DEVICE_TABLE(of, mtdsplit_rtl8197f_of_match_table);

static struct mtd_part_parser mtdsplit_rtl8197f_parser = {
	.owner = THIS_MODULE,
	.name = "rtl8197f-fw",
	.of_match_table = mtdsplit_rtl8197f_of_match_table,
	.parse_fn = mtdsplit_parse_rtl8197f,
	.type = MTD_PARSER_TYPE_FIRMWARE,
};

static int __init mtdsplit_rtl8197f_init(void)
{
	return register_mtd_parser(&mtdsplit_rtl8197f_parser);
}
subsys_initcall(mtdsplit_rtl8197f_init);

MODULE_DESCRIPTION("Realtek RTL8197F cr6c/cs6c firmware parser");
MODULE_LICENSE("GPL");
