unsigned int rtl819x_bond_option(void)
{
	unsigned int type = 0, ret = 0;

	type = __raw_readl((void __iomem*)BSP_BOND_OPTION) & 0xf;

	switch(type) {
	case 0x0:	/* 97FB */
		ret = BSP_BOND_97FB;
		break;
	case 0x4:	/* 97FN */
	case 0x5:
	case 0x6:
		 ret = BSP_BOND_97FN;
		break;
	case 0xa:	/* 97FS */
	case 0xb:
	case 0xc:
		ret = BSP_BOND_97FS;
	}

	pr_debug("[%s][%d]: 97F type %d\n", __FUNCTION__, __LINE__, ret);
	return ret;
}
EXPORT_SYMBOL(rtl819x_bond_option);

static int __init rtl8197f_setup(void)
{
	mips_machine_setup();

	return 0;
