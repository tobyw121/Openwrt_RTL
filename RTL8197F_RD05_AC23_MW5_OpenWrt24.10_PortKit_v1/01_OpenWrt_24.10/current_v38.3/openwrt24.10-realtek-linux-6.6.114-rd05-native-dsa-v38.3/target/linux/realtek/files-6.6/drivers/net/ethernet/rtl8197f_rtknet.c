// SPDX-License-Identifier: GPL-2.0-only
/*
 * Native Linux 6.6 Ethernet CPU-interface driver for Realtek RTL8197F/RTL8197FH rtl865x.
 *
 * This is a clean net_device/NAPI port built from the RTL8197F/rtl865x SDK
 * register and descriptor definitions.  It intentionally does not import the
 * old Realtek rtknet forwarding/NAT/fastpath stack as private kernel ABI.
 * VLAN and external RTL8367 switching are handled by DSA/bridge, software
 * fastpath by nftables flowtables and future hardware ACL/NAT by tc/flowtable
 * callbacks rather than Realtek private ioctls.
 */

#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/capability.h>
#include <linux/dma-mapping.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/interrupt.h>
#include <linux/atomic.h>
#include <linux/if_vlan.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/rcupdate.h>
#include <linux/iopoll.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/sockios.h>
#include <linux/string.h>
#include <linux/of.h>
#include <linux/of_net.h>
#include <linux/platform_device.h>
#include <linux/proc_fs.h>
#include <linux/skbuff.h>
#include <linux/uaccess.h>
#include <linux/workqueue.h>
#include <linux/if_ether.h>
#include <linux/timer.h>
#include <linux/netlink.h>
#include <net/flow_offload.h>
#include <net/pkt_cls.h>

#define DRV_NAME			"rtl8197f-rtknet"
#define DRV_VERSION			"1.4.2-sdk-native-rd05-dsa-v38.2"

/* RTL865x/8197F CPU interface registers, relative to CPU_IFACE_BASE. */
#define RTL_RTK_CPUICR			0x000
#define RTL_RTK_CPURPDCR0		0x004
#define RTL_RTK_CPURMDCR0		0x01c
#define RTL_RTK_CPUTPDCR0		0x020
#define RTL_RTK_CPUIIMR			0x028
#define RTL_RTK_CPUIISR			0x02c
#define RTL_RTK_DMA_CR0			0x03c
#define RTL_RTK_DMA_CR1			0x040
#define RTL_RTK_DMA_CR2			0x044
#define RTL_RTK_CPUTPDCR2		0x060
#define RTL_RTK_CPUTPDCR3		0x064
#define RTL_RTK_DMA_CR3			0x068
#define RTL_RTK_TXRINGCR		0x078
#define RTL_RTK_CPUIMCR			0x080
#define RTL_RTK_DMA_CR4			0x0a0
#define RTL_RTK_CPUICR1			0x0a4

#define RTL_RTK_CPUQDM0			0x030
#define RTL_RTK_CPUQDM2			0x034
#define RTL_RTK_CPUQDM4			0x038
#define RTL_RTK_CPUQDM_CPU_RX_DESC_SHIFT	12
#define RTL_RTK_CPUQDM_EXT1_RX_DESC_SHIFT	8
#define RTL_RTK_CPUQDM_EXT2_RX_DESC_SHIFT	4
#define RTL_RTK_CPUQDM_EXT3_RX_DESC_SHIFT	0
#define RTL_RTK_CPUQDM_ALL_TO_RING0	0x00000000

/* CPUICR */
#define RTL_RTK_CPUICR_TXCMD		BIT(31)
#define RTL_RTK_CPUICR_RXCMD		BIT(30)
#define RTL_RTK_CPUICR_BUSBURST_32	0
#define RTL_RTK_CPUICR_BUSBURST_64	BIT(28)
#define RTL_RTK_CPUICR_BUSBURST_128	(2 << 28)
#define RTL_RTK_CPUICR_MBUF_2048		(4 << 24)
#define RTL_RTK_CPUICR_TXFD		BIT(23)
#define RTL_RTK_CPUICR_SOFTRST		BIT(22)
#define RTL_RTK_CPUICR_STOPTX		BIT(21)
#define RTL_RTK_CPUICR_EXCLUDE_CRC	BIT(16)

/* CPUICR1 */
#define RTL_RTK_CPUICR1_TXRX_DIV_LX	BIT(0)
#define RTL_RTK_CPUICR1_LE		BIT(1)
#define RTL_RTK_CPUICR1_TSO_ID_SEL	BIT(4)
#define RTL_RTK_CPUICR1_RX_GATHER	BIT(5)
#define RTL_RTK_CPUICR1_TX_GATHER	BIT(6)
#define RTL_RTK_CPUICR1_PKT_HDR_TYPE_MASK	(3 << 8)
#define RTL_RTK_CPUICR1_PKT_HDR_8198C_DEF	(0 << 8)
#define RTL_RTK_CPUICR1_PKT_HDR_SHORTCUT_LSO	(1 << 8)
#define RTL_RTK_CPUICR1_TXDESC_OFFSET	12
#define RTL_RTK_CPUICR1_TXDESC_MASK	(0x3f << RTL_RTK_CPUICR1_TXDESC_OFFSET)
#define RTL_RTK_CPUICR1_RXDESC_OFFSET	18
#define RTL_RTK_CPUICR1_RXDESC_MASK	(0x3f << RTL_RTK_CPUICR1_RXDESC_OFFSET)

/* CPUIIMR / CPUIISR */
#define RTL_RTK_INT_LINK_CHANGE		BIT(31)
#define RTL_RTK_INT_RX_ERR0		BIT(25)
#define RTL_RTK_INT_TX_ERR0		BIT(23)
#define RTL_RTK_INT_RX_RUNOUT0		BIT(17)
#define RTL_RTK_INT_MBUF_RUNOUT0		BIT(16)
#define RTL_RTK_INT_TX_DONE0		BIT(9)
#define RTL_RTK_INT_RX_DONE0		BIT(3)
#define RTL_RTK_INT_TX_ALL_DONE0		BIT(1)
#define RTL_RTK_INT_RX_DONE_ALL		(0x3f << 3)
#define RTL_RTK_INT_TX_ALL_DONE_ALL	(0x03 << 1)
#define RTL_RTK_INT_PKTHDR_RUNOUT_ALL	(0x3f << 17)
#define RTL_RTK_INT_MBUF_RUNOUT_ALL	BIT(16)

#define RTL_RTK_INT_RX_MASK		(RTL_RTK_INT_RX_DONE0 | \
					 RTL_RTK_INT_RX_ERR0 | \
					 RTL_RTK_INT_RX_RUNOUT0 | \
					 RTL_RTK_INT_MBUF_RUNOUT0)
#define RTL_RTK_INT_TX_MASK		(RTL_RTK_INT_TX_DONE0 | \
					 RTL_RTK_INT_TX_ALL_DONE0 | \
					 RTL_RTK_INT_TX_ERR0)
#define RTL_RTK_INT_MASK		(RTL_RTK_INT_RX_MASK | \
					 RTL_RTK_INT_TX_MASK | \
					 RTL_RTK_INT_LINK_CHANGE)

/* DMA_CR0 / DMA_CR4 */
#define RTL_RTK_DMA_CR0_LOW_FIFO_MARK_MASK	(0xff << 8)
#define RTL_RTK_DMA_CR0_HIGH_FIFO_MARK_MASK	0xff
#define RTL_RTK_DMA_CR0_LOW_FIFO_MARK(v)	(((v) & 0xff) << 8)
#define RTL_RTK_DMA_CR0_HIGH_FIFO_MARK(v)	((v) & 0xff)
#define RTL_RTK_DMA_CR4_TX_RING0_TAIL_AWARE BIT(0)

/* SWCORE/SYSTEM offsets from the RTL8197F SDK.
 * CPU_IFACE lives in the normal system window (0x18010000), but SIRR/TRXRDY
 * and MACCTRL1 live in the rtl865x switch-core window (SDK SWCORE_BASE,
 * physical 0x1b800000 on RTL8197F).  Mapping SIRR at 0x18004200 looks valid
 * but writes the wrong block, leaving the DMA core unable to consume rings.
 */
#define RTL_RTK_SWCORE_SWMISC_BASE	0x4200
#define RTL_RTK_SWMISC_SSIR		0x004
#define RTL_RTK_SWMISC_TRXRDY		BIT(0)
#define RTL_RTK_SWCORE_MACCR		0x4000
#define RTL_RTK_SWCORE_MACCR1		0x4058
#define RTL_RTK_SWCORE_PITCR		0x4100
#define RTL_RTK_SWCORE_P0GMIICR		0x414c
#define RTL_RTK_SWCORE_MACCTRL1		0x5100
#define RTL_RTK_SWCORE_EXTPCR0		0x5108
#define RTL_RTK_MACCR_P0_GIGA_LINK	BIT(12)
#define RTL_RTK_MACCR1_P0_ROUTER_MODE	BIT(0)
#define RTL_RTK_MACCR1_RMD_TAG_MASK	GENMASK(7, 6)
#define RTL_RTK_PITCR_P0_EXT_INTERFACE	BIT(0)
#define RTL_RTK_PITCR_FIX_IPG_MASK	GENMASK(17, 12)
#define RTL_RTK_PITCR_FIX_IPG_8BYTE	BIT(12)
#define RTL_RTK_EXTPCR0_TX_IPG_MASK	GENMASK(19, 16)
#define RTL_RTK_EXTPCR0_TX_IPG_8BYTE	(8U << 16)
#define RTL_RTK_P0GMIICR_CPU_TAG_RX	BIT(25)
#define RTL_RTK_P0GMIICR_CPU_TAG_TX	BIT(26)
#define RTL_RTK_P0GMIICR_GMAC_MASK	GENMASK(24, 23)
#define RTL_RTK_P0GMIICR_RGTXC_MASK	GENMASK(19, 18)
#define RTL_RTK_P0GMIICR_RGTXC_3	(3U << 18)
#define RTL_RTK_P0GMIICR_TX_DELAY_MASK	BIT(4)
#define RTL_RTK_P0GMIICR_RX_DELAY_MASK	GENMASK(2, 0)
#define RTL_RTK_P0GMIICR_RX_DELAY_VG	6U
#define RTL_RTK_P0GMIICR_CONF_DONE	BIT(6)
#define RTL_RTK_MACCTRL1_CMAC_CLK_SEL	BIT(0)
#define RTL_RTK_MACCTRL1_CMAC_LATPKT_EN	BIT(5)
#define RTL_RTK_MACCTRL1_CMAC_LATPKT_TYPE	BIT(6)
#define RTL_RTK_SYSTEM_SYS_CLK_MAG	0x010
#define RTL_RTK_SYSTEM_PAD_CTRL_1	0x850
#define RTL_RTK_SYSTEM_GIMR		0x3000
#define RTL_RTK_PAD_P0_RGMII_DN_SHIFT	29
#define RTL_RTK_PAD_P0_RGMII_DN_MASK	GENMASK(31, 29)
#define RTL_RTK_PAD_P0_RGMII_DP_SHIFT	26
#define RTL_RTK_PAD_P0_RGMII_DP_MASK	GENMASK(28, 26)
#define RTL_RTK_PAD_P0_RGMII_MODE	BIT(25)
#define RTL_RTK_PAD_P0_TX_E2		BIT(22)
#define RTL_RTK_PAD_P0_VG_BIT21		BIT(21)
#define RTL_RTK_PAD_P0_DRIVE_VG		6U
#define RTL_RTK_SYSTEM_GISR		0x3004
#define RTL_RTK_GIMR_BSP_SW_IE		BIT(15)
#define RTL_RTK_RD05_PRIV_RESEED		(SIOCDEVPRIVATE + 0)
#define RTL_RTK_RD05_PRIV_RXSTART		(SIOCDEVPRIVATE + 1)
#define RTL_RTK_RD05_PRIV_VENDOR_SIDE	(SIOCDEVPRIVATE + 2)
#define RTL_RTK_SYS_SW_CLK_ENABLE	BIT(9)
#define RTL_RTK_SYS_CLK_ACTIVE_SWCORE	BIT(11)
#define RTL_RTK_SYS_CLK_ACTIVE_LX1_ARB	BIT(13)
#define RTL_RTK_SYS_CLK_ACTIVE_LX1_CLK	BIT(12)
#define RTL_RTK_SYS_CLK_ACTIVE_LX2_ARB	BIT(20)
#define RTL_RTK_SYS_CLK_ACTIVE_LX2_CLK	BIT(19)
#define RTL_RTK_ACL_TABLE_ENTRY0_PHYS	0x1b0c0000

/* Additional RTL8197F SDK sideband registers used by rtl865x_start()/
 * rtl8651_clearAsicAllTable().  Earlier v3-v8 traces proved the external
 * RTL8367D forwards broadcasts out MAC7/EXT1 while the RTL8197F CPU DMA ring
 * receives nothing.  The legacy SDK path also enables every internal/ext port
 * PCR, per-port match action, flow-control scheduler, remark/ALE defaults and
 * IPv6 extension-header-to-CPU mode before asserting TRXRDY.  v9 intentionally
 * mirrors those writes for RD05 because the project now allows SDK/private ABI
 * behaviour where needed for hardware bring-up.
 */
#define RTL_RTK_SWCORE_TMRCR		0x6300
#define RTL_RTK_SWCORE_PPMAR		0x4010
#define RTL_RTK_SWCORE_PATP(_p)		(0x4014 + ((_p) * 4))
#define RTL_RTK_SWCORE_PCRP(_p)		(0x4104 + ((_p) * 4))
#define RTL_RTK_SWCORE_RMACR		0x4408
#define RTL_RTK_SWCORE_ALECR		0x440c
#define RTL_RTK_SWCORE_SBFCR0		0x4500
#define RTL_RTK_SWCORE_SBFCR1		0x4504
#define RTL_RTK_SWCORE_SBFCR2		0x4508
#define RTL_RTK_SWCORE_PBFCR(_p)	(0x450c + ((_p) * 4))
#define RTL_RTK_SWCORE_PQPLGR		0x45d8
#define RTL_RTK_SWCORE_QRR		0x45dc
#define RTL_RTK_SWCORE_QIDDPCR		0x4750
#define RTL_RTK_SWCORE_RMCR1P		0x476c
#define RTL_RTK_SWCORE_DSCPRM0		0x4770
#define RTL_RTK_SWCORE_DSCPRM1		0x4774
#define RTL_RTK_SWCORE_RLRC		0x4778
#define RTL_RTK_SWCORE_P0Q0RGCR(_n)	(0x4800 + ((_n) * 4))
#define RTL_RTK_SWCORE_WFQRCRP0(_n)	(0x48b0 + ((_n) * 12))
#define RTL_RTK_SWCORE_ELBPCR		0x4904
#define RTL_RTK_SWCORE_ELBTTCR		0x4908
#define RTL_RTK_SWCORE_ILBPCR1		0x490c
#define RTL_RTK_SWCORE_ILBPCR2		0x4910
#define RTL_RTK_SWCORE_IGPRIVCR0	0x5168
#define RTL_RTK_SWCORE_IGPRIVCR1	0x516c
#define RTL_RTK_SWCORE_IGPRIVCR2	0x5170
#define RTL_RTK_SWCORE_IPV6CR1		0x5204
#define RTL_RTK_TMRCR_ENHSBTESTMODE	BIT(1)
#define RTL_RTK_PCRP_MAC_SW_RESET	BIT(3)
#define RTL_RTK_PCRP_ENABLE_PHY_IF	BIT(0)
#define RTL_RTK_PCRP_EN_FORCE_MODE	BIT(25)
#define RTL_RTK_PCRP_POLL_LINK_STATUS	BIT(24)
#define RTL_RTK_PCRP_EXT_PHYID_SHIFT	26
#define RTL_RTK_PCRP_EXT_PHYID_MASK	(0x1fU << RTL_RTK_PCRP_EXT_PHYID_SHIFT)
#define RTL_RTK_PCRP_FORCE_LINK		BIT(23)
#define RTL_RTK_PCRP_FORCE_STATUS_MASK	GENMASK(22, 18)
#define RTL_RTK_PCRP_FORCE_SPEED_1000M	(2U << 19)
#define RTL_RTK_PCRP_FORCE_DUPLEX	BIT(18)
#define RTL_RTK_PCRP_PAUSE_MASK		GENMASK(17, 16)
#define RTL_RTK_PCRP_PAUSE_TXRX		(3U << 16)
#define RTL_RTK_PCRP_MII_RXER		BIT(13)
#define RTL_RTK_PCRP_STP_MASK		GENMASK(5, 4)
#define RTL_RTK_PCRP_STP_FORWARDING	(3U << 4)
#define RTL_RTK_RD05_P0_EXT_PHY_ID	5U
#define RTL_RTK_QIDDPCR_SDK_INIT		0x00011111
#define RTL_RTK_SBFCR0_SDK_INIT		0x000001e0
#define RTL_RTK_SBFCR1_SDK_INIT		0x019001cc
#define RTL_RTK_SBFCR2_SDK_INIT		0x0050006c
#define RTL_RTK_PBFCR_SDK_INIT		0x003c005a
#define RTL_RTK_ELBPCR_SDK_INIT		0x00003326
#define RTL_RTK_ELBTTCR_SDK_INIT		0x00000400
#define RTL_RTK_ILBPCR1_SDK_INIT	0x00000000
#define RTL_RTK_ILBPCR2_SDK_INIT	0x00003326
#define RTL_RTK_P0QRGCR_SDK_INIT	0x7ff03fff
#define RTL_RTK_WFQRCRP0_SDK_INIT	0x000fffff
#define RTL_RTK_PQPLGR_SDK_INIT		0x00181818
#define RTL_RTK_TXRINGCR_TX_RING0_EN	BIT(0)
#define RTL_RTK_TXRINGCR_TX_RING1_EN	BIT(1)
#define RTL_RTK_TXRINGCR_TX_RING2_EN	BIT(2)
#define RTL_RTK_TXRINGCR_TX_RING3_EN	BIT(3)
#define RTL_RTK_TXRINGCR_TX_RING_ROUND	BIT(4)
#define RTL_RTK_TXRINGCR_TXDCP_BP_EN(v)	(((v) & 0xf) << 8)
#define RTL_RTK_TXRINGCR_TXRING2_TO_FIFO(v)	(((v) & 0x3) << 16)
#define RTL_RTK_TXRINGCR_TXRING3_TO_FIFO(v)	(((v) & 0x3) << 18)
#define RTL_RTK_TXRINGCR_BLEN_ADJ_EN	BIT(31)
#define RTL_RTK_TXRINGCR_8197F_VG_INIT	( RTL_RTK_TXRINGCR_BLEN_ADJ_EN | \
					  RTL_RTK_TXRINGCR_TXRING3_TO_FIFO(0x2) | \
					  RTL_RTK_TXRINGCR_TXRING2_TO_FIFO(0x1) | \
					  RTL_RTK_TXRINGCR_TXDCP_BP_EN(0xf) | \
					  RTL_RTK_TXRINGCR_TX_RING_ROUND | \
					  RTL_RTK_TXRINGCR_TX_RING1_EN | \
					  RTL_RTK_TXRINGCR_TX_RING0_EN )


/*
 * RTL865x ASIC table access controller (TACI), relative to SWCORE_BASE.
 * The OEM RD05 firmware keeps LAN in VID9 and programs static L2 entries:
 *   ff:ff:ff:ff:ff:ff FID0 mbr(0 1 2 3 4 7) CPU STA NH AUTH
 *   00:00:0a:00:00:0f FID0 CPU STA NH AUTH
 *   00:00:0a:00:00:0f FID1 CPU STA NH AUTH
 *   ff:ff:ff:ff:ff:ff FID1 mbr(0 1 2 3 4 7) CPU STA NH AUTH
 * v5r seeds those entries directly through TACI before CPU RX starts.
 * v5t keeps that seed and additionally reconstructs the OEM SWCORE
 * ingress pipeline before L2 lookup: PVID, port->netif, netif VID9 and
 * an explicit ingress ACL test block. v5s proved the guessed PTRAPCR
 * offset is not writable/effective on this RTL8197F path.
 */
#define RTL_RTK_SWCORE_ALE_BASE		0x4400
#define RTL_RTK_SWCORE_TACI_BASE	0x4d00
#define RTL_RTK_SWTACR			(RTL_RTK_SWCORE_TACI_BASE + 0x000)
#define RTL_RTK_SWTASR			(RTL_RTK_SWCORE_TACI_BASE + 0x004)
#define RTL_RTK_SWTAA			(RTL_RTK_SWCORE_TACI_BASE + 0x008)
#define RTL_RTK_TCR0			(RTL_RTK_SWCORE_TACI_BASE + 0x020)
#define RTL_RTK_TCR1			(RTL_RTK_SWCORE_TACI_BASE + 0x024)
#define RTL_RTK_TCR2			(RTL_RTK_SWCORE_TACI_BASE + 0x028)
#define RTL_RTK_TCR3			(RTL_RTK_SWCORE_TACI_BASE + 0x02c)
#define RTL_RTK_TCR4			(RTL_RTK_SWCORE_TACI_BASE + 0x030)
#define RTL_RTK_TCR5			(RTL_RTK_SWCORE_TACI_BASE + 0x034)
#define RTL_RTK_TCR6			(RTL_RTK_SWCORE_TACI_BASE + 0x038)
#define RTL_RTK_TCR7			(RTL_RTK_SWCORE_TACI_BASE + 0x03c)
#define RTL_RTK_TCR8			(RTL_RTK_SWCORE_TACI_BASE + 0x040)
#define RTL_RTK_TCR9			(RTL_RTK_SWCORE_TACI_BASE + 0x044)
#define RTL_RTK_TCR10			(RTL_RTK_SWCORE_TACI_BASE + 0x048)
#define RTL_RTK_TACI_ACTION_START	BIT(0)
#define RTL_RTK_TACI_ACTION_MASK		BIT(0)
#define RTL_RTK_TACI_CMD_FORCE		BIT(3)
#define RTL_RTK_TACI_TABSTS_FAIL		BIT(0)
#define RTL_RTK_ASICTBL_BASE_KSEG1	0xbb000000
#define RTL_RTK_ASICTBL_ENTRY_LEN	32
#define RTL_RTK_ASICTBL_TYPE_L2		0
#define RTL_RTK_ASICTBL_TYPE_NETIF	4
#define RTL_RTK_ASICTBL_TYPE_VLAN	6
#define RTL_RTK_ASICTBL_TYPE_ACL		12
/*
 * Linux DSA owns the external RTL8367D port namespace.  The RTL8197F internal
 * rtl865x switch therefore sees exactly one physical cascade link: P0/RGMII.
 * Do not mirror RTL8367D UTP0..4 or MAC7 into rtl865x extension-port bits; that
 * representation belongs to the OEM private multi-netdev stack and caused the
 * v30 DP=0x40/DP_EXT=2 self-loop.  The internal VLAN/L2 host-link member is P0.
 */
#define RTL_RTK_RD05_HOST_PORT		0
#define RTL_RTK_RD05_HOST_MBR		BIT(RTL_RTK_RD05_HOST_PORT)
#define RTL_RTK_RD05_OEM_WAN_VID	8

/* rtl865x SWCORE ingress/classifier registers, relative to SWCORE_BASE. */
#define RTL_RTK_SWCORE_MSCR		0x4410
#define RTL_RTK_SWCORE_SWTCR0		0x4418
#define RTL_RTK_SWCORE_SWTCR1		0x441c
#define RTL_RTK_SWCORE_PLITIMR		0x4420
#define RTL_RTK_SWCORE_DACLRCR		0x4424
#define RTL_RTK_SWCORE_FFCR		0x4428
#define RTL_RTK_SWCORE_PVCR0		0x4a08
#define RTL_RTK_SWCORE_PTRAPCR		0x7024
#define RTL_RTK_DACLRCR_ACLI_STA(v)	((v) & 0xff)
#define RTL_RTK_DACLRCR_ACLI_END(v)	(((v) & 0xff) << 8)
#define RTL_RTK_DACLRCR_ACLO_STA(v)	(((v) & 0xff) << 16)
#define RTL_RTK_DACLRCR_ACLO_END(v)	(((v) & 0xff) << 24)
#define RTL_RTK_DACLRCR_ING_0_4_EG_253 \
	(RTL_RTK_DACLRCR_ACLI_STA(0) | RTL_RTK_DACLRCR_ACLI_END(4) | \
	 RTL_RTK_DACLRCR_ACLO_STA(253) | RTL_RTK_DACLRCR_ACLO_END(253))
#define RTL_RTK_PTRAPCR_EN_ARP_TRAP	BIT(24)
#define RTL_RTK_PTRAPCR_EN_PPPOE_TRAP	BIT(26)
#define RTL_RTK_PTRAPCR_EN_DHCP67_TRAP	BIT(28)
#define RTL_RTK_PTRAPCR_EN_DHCP68_TRAP	BIT(29)
#define RTL_RTK_PTRAPCR_RD05_TRAPS \
	(RTL_RTK_PTRAPCR_EN_ARP_TRAP | RTL_RTK_PTRAPCR_EN_PPPOE_TRAP | \
	 RTL_RTK_PTRAPCR_EN_DHCP67_TRAP | RTL_RTK_PTRAPCR_EN_DHCP68_TRAP)
#define RTL_RTK_MSCR_EN_L2		BIT(0)
#define RTL_RTK_MSCR_EN_L3		BIT(1)
#define RTL_RTK_MSCR_EN_IN_ACL		BIT(4)
#define RTL_RTK_SWTCR0_LIMDBC_MASK	GENMASK(17, 16)
#define RTL_RTK_SWTCR0_LIMDBC_VLAN	0
#define RTL_RTK_SWTCR0_EN_UKVID_TO_CPU	BIT(15)
#define RTL_RTK_SWTCR1_SEL_CPU_REASON	BIT(8)
#define RTL_RTK_SWTCR1_EN_NATT2LOG	BIT(10)
#define RTL_RTK_SWTCR1_EN_FRAG_TO_ACLPT	BIT(11)
#define RTL_RTK_FFCR_EN_UNK_MCAST_TOCPU	BIT(0)
#define RTL_RTK_FFCR_EN_UNK_UCAST_TOCPU	BIT(1)
#define RTL_RTK_ACL_ACTION_PERMIT	0x0
#define RTL_RTK_ACL_ACTION_TOCPU	0x3
#define RTL_RTK_ACL_TYPE_ETHERNET	0x0
#define RTL_RTK_ACL_TYPE_UDP		0x7
#define RTL_RTK_ACL_TYPE_SRCFILTER	0x8
#define RTL_RTK_ACL_PKT_OP_ALL		0x7
#define RTL_RTK_RD05_LAN_VID		9
#define RTL_RTK_RD05_LAN_FID		0


/* Descriptor fields shared with the vendor RTL8197F rtknet driver. */
#define RTL_RTK_DESC_OWN		BIT(0)
#define RTL_RTK_DESC_WRAP		BIT(1)
#define RTL_RTK_DESC_LAST		BIT(2)
#define RTL_RTK_DESC_FIRST		BIT(3)
#define RTL_RTK_TX_HWLKUP		BIT(4)
#define RTL_RTK_TX_BRIDGE		BIT(5)
#define RTL_RTK_RX_EXTSIZE_SHIFT		16
#define RTL_RTK_RX_LEN_MASK		0x3fff
#define RTL_RTK_TX_PH_LEN_SHIFT		6
#define RTL_RTK_TX_M_LEN_SHIFT		15
#define RTL_RTK_TX_DP_EXT_SHIFT		12
#define RTL_RTK_TX_DP_EXT_MASK		0x7
#define RTL_RTK_TX_DP_SHIFT		24
#define RTL_RTK_TX_DP_MASK		0x7f
#define RTL_RTK_TX_EXTSPA_SHIFT		30
#define RTL_RTK_TX_EXTSPA_MASK		0x3

#define RTL_RTK_DEFAULT_RX_RING		64
#define RTL_RTK_DEFAULT_TX_RING		64
#define RTL_RTK_MIN_RING		16
#define RTL_RTK_MAX_RING		512
#define RTL_RTK_DEFAULT_RX_BUFSZ		2048
#define RTL_RTK_DEFAULT_TX_PORT_MASK	0x7f
#define RTL_RTK_DEFAULT_TX_DP_EXT	0
#define RTL_RTK_DEFAULT_TX_EXTSPA	0
#define RTL_RTK_DEFAULT_RX_POLL_MS	100
#define RTL_RTK_REG_DUMP_LEN		(RTL_RTK_CPUICR1 + sizeof(u32))


/*
 * RD05 v31 retains explicit diagnostic module-parameter axes for
 * rd05-netdiag magic-RX autotune/hunt across DSA, TXDESC, portmap and
 * EXTCPU modes.
 *
 * v20-v22b proved that the Linux DSA tagger can create multiple RTL8367D
 * CPU-tag formats, but the PC still never sees router-originated ARP frames.
 * Keep DSA on the standard header tag and vary the RTL8197F TX descriptor
 * metadata that steers frames from the SoC DMA engine into the internal
 * rtl865x/SWCORE extension path toward the external RTL8367D RGMII link.
 */
static int rd05_txdesc_mode;
module_param_named(rd05_txdesc_mode, rd05_txdesc_mode, int, 0644);
MODULE_PARM_DESC(rd05_txdesc_mode, "RD05 RTL8197F TX mode: 0 direct P0/RGMII, 1 legacy DP40/EXT2 loop, 2..12 diagnostics");

static unsigned int rd05_txdesc_trace_limit = 128;
module_param_named(rd05_txdesc_trace_limit, rd05_txdesc_trace_limit, uint, 0644);
MODULE_PARM_DESC(rd05_txdesc_trace_limit, "RD05 TX descriptor trace limit for ARP/IPv4/RTL8367D-tagged frames");

/* RD05 v36 exposes both SoC-side RGMII delay controls.  The Linux SDK
 * defaults are TX0/RX6 on RTL8197F-VG; hardware calibration can change
 * them at runtime and replay P0 setup through rd05-ioctl eth0 reseed.
 */
static unsigned int rd05_p0_tx_delay;
module_param_named(rd05_p0_tx_delay, rd05_p0_tx_delay, uint, 0644);
MODULE_PARM_DESC(rd05_p0_tx_delay, "RD05 RTL8197F P0 RGMII TX delay: 0 or 1");

static unsigned int rd05_p0_rx_delay = RTL_RTK_P0GMIICR_RX_DELAY_VG;
module_param_named(rd05_p0_rx_delay, rd05_p0_rx_delay, uint, 0644);
MODULE_PARM_DESC(rd05_p0_rx_delay, "RD05 RTL8197F P0 RGMII RX delay tap: 0..7");

static atomic_t rd05_txdesc_trace_count = ATOMIC_INIT(0);


/*
 * v31 stable raw-RX validation: the PC may continuously transmit a broadcast/raw
 * Ethernet or UDP test frame containing one of these ASCII tokens.  The
 * RTL8197F RX path scans raw DMA payload bytes before eth_type_trans()/DSA/
 * bridge/IP handling, so rd05-netdiag can identify the first hardware init
 * candidate that really makes a PC-originated frame reach the CPU descriptor.
 */
static bool rd05_magic_enable = true;
module_param_named(rd05_magic_enable, rd05_magic_enable, bool, 0644);
MODULE_PARM_DESC(rd05_magic_enable, "RD05 scan raw RX DMA frames for v31/v30 PC magic test tokens");

static int rd05_magic_candidate = -1;
module_param_named(rd05_magic_candidate, rd05_magic_candidate, int, 0644);
MODULE_PARM_DESC(rd05_magic_candidate, "RD05 current magic-rx autotune candidate id for hit correlation");

static unsigned int rd05_magic_log_limit = 64;
module_param_named(rd05_magic_log_limit, rd05_magic_log_limit, uint, 0644);
MODULE_PARM_DESC(rd05_magic_log_limit, "RD05 raw-RX magic hit printk limit");

static atomic_t rd05_magic_log_count = ATOMIC_INIT(0);

#define RD05_MAGIC_TOKEN_V31       "RD05MAGIC-V31"
#define RD05_MAGIC_TOKEN_V30       "RD05MAGIC-V30"
#define RD05_MAGIC_TOKEN_V29       "RD05MAGIC-V29"
#define RD05_MAGIC_TOKEN_AUTOTUNE  "RD05_AUTOTUNE"

/*
 * RTL8197F-VG SDK builds the new CPU DMA descriptor with
 * CONFIG_RTL_TX_CACHE_ALIGN/CONFIG_RTL_RX_CACHE_ALIGN, which appends two
 * reserved dwords to the 6-dword RTL8198C-style descriptor.  The hardware ring
 * walker therefore advances in 32-byte steps.  Using the older 24-byte stride
 * lets Linux populate descriptors, but the rtl865x CPU DMA never consumes them
 * reliably: TXFD kicks accumulate, no TX-done IRQ arrives and eth0 eventually
 * hits NETDEV WATCHDOG while p07 counters remain zero.
 */
struct rtl8197f_rtk_desc {
	__le32 opts1;
	__le32 addr;
	__le32 opts2;
	__le32 opts3;
	__le32 opts4;
	__le32 opts5;
	__le32 opts6;
	__le32 opts7;
};

/*
 * RD05 v12: legacy RTL865x RX packet-header/mbuf mode, physical DMA addresses.
 * The GPL SDK does not feed RX buffers through the 8198C-style descriptor used
 * by our tagged TX path.  It programs CPURPDCR0 with a ring of 32-bit pointers
 * to 32-byte rtl_pktHdr objects and CPURMDCR0 with a ring of pointers to
 * 32-byte rtl_mBuf objects.  Previous traces showed CPURPDCR0 stayed zero and
 * no RX descriptor ownership ever flipped; mirror the SDK RX-side format while
 * keeping the Linux netdev/NAPI handoff.
 */
#define RTL_RTK_RD05_PKTHDR_USED_INCOMING	0x9000
#define RTL_RTK_RD05_MBUF_FLAGS_RX		0x9c

struct rtl8197f_rtk_rd05_pkthdr {
	__le32 ph_mbuf;
	__le32 ph_word1;
	__le32 ph_word2;
	__le32 ph_word3;
	__le32 ph_word4;
	__le32 ph_word5;
	__le32 ph_pending0;
	__le32 ph_pending1;
};

struct rtl8197f_rtk_rd05_mbuf {
	__le32 m_next;
	__le32 m_pkthdr;
	__le32 m_word2;
	__le32 m_data;
	__le32 m_extbuf;
	__le32 m_word5;
	__le32 skb_cookie;
	__le32 pending0;
};

struct rtl8197f_rtknet_priv {
	struct device *dev;
	struct net_device *ndev;
	void __iomem *base;
	void __iomem *swcore;
	void __iomem *system;
	void __iomem *acl0;
	int irq;
	struct napi_struct napi;
	spinlock_t tx_lock;

	struct rtl8197f_rtk_desc *rx_desc;
	dma_addr_t rx_desc_dma;
	struct sk_buff **rx_skb;
	dma_addr_t *rx_dma;
	u32 rx_ring_size;
	u32 rx_buf_size;
	u32 rx_tail;

	struct rtl8197f_rtk_desc *tx_desc;
	dma_addr_t tx_desc_dma;
	struct sk_buff **tx_skb;
	dma_addr_t *tx_dma;
	u32 *tx_len;
	u32 tx_ring_size;
	u32 tx_head;
	u32 tx_tail;

	u32 tx_port_mask;
	u32 tx_dp_ext;
	u32 tx_extspa;
	bool tx_hwlookup;
	bool tx_bridge;
	u32 msg_enable;
	bool desc_addr_kseg1;
	bool desc_addr_physical;
	bool rx_poll_fallback;
	bool sdk_trxrdy;
	bool sdk_cpuicr1_init;
	bool sdk_swcore_init;
	bool legacy_rd05_pipeline;
	bool sdk_crc_lengths;
	bool p0_cpu_tag_passthrough;
	u32 rx_poll_ms;
	struct timer_list rx_poll_timer;
	struct delayed_work rd05_reseed_work;
	struct proc_dir_entry *rd05_proc;

	u64 irq_rx_done;
	u64 irq_tx_done;
	u64 irq_rx_errors;
	u64 irq_tx_errors;
	u64 irq_rx_runout;
	u64 irq_mbuf_runout;
	u64 irq_link_change;
	u64 napi_polls;
	u64 rx_bad_desc;
	u64 rx_missing_skb;
	u64 rx_alloc_fail;
	u64 rx_dma_errors;
	u64 rx_cdp_reads;
	u64 rx_cdp_advanced;
	u64 rx_cdp_owned_advanced;
	u64 rx_cdp_invalid;
	u64 rx_cdp_last;
	u64 rx_cdp_last_idx;
	u64 tx_mtu_drops;
	u64 tx_dma_errors;
	u64 tx_ring_full;
	u64 tx_busy_desc;
	u64 tx_timeouts;
	u64 hw_rx_restarts;
	u64 rx_poll_timer_runs;
	u64 sw_trxrdy_writes;
	u64 cpuicr1_sdk_writes;
	u64 swcore_sdk_writes;
	u64 txringcr_sdk_writes;
	u64 sirr_direct_writes;
	u64 gimr_sdk_writes;
	u64 acl0_sdk_reads;
	u64 rd05_vendor_sideband_runs;
	u64 rd05_private_ioctl_runs;
	u64 tx_kicks;
	u64 tx_crc_len_adjusts;
	u64 rx_crc_strips;
	u64 rd05_rx_untagged;
	u64 rd05_rx_external;
	u64 rd05_rx_self_loop;
	u64 rd05_rx_arp;
	u64 rd05_rx_ipv4;
	u64 rd05_rx_ipv6;
	u64 rd05_rx_other;
	u64 rd05_rx_bcast;
	u64 rd05_rx_mcast;
	u64 rd05_rx_ucast;
	u64 rd05_rx_direct_lan2;
	u64 rd05_rxtrace_frames;
	u64 rd05_rxtrace_logs;
	u64 rd05_rxtrace_bcast0;
	u64 rd05_rxtrace_bcast4;
	u64 rd05_rxtrace_bcast8;
	u64 rd05_rxtrace_arp0;
	u64 rd05_rxtrace_arp4;
	u64 rd05_rxtrace_arp8;
	u64 rd05_rxtrace_rtk0;
	u64 rd05_rxtrace_rtk4;
	u64 rd05_rxtrace_suspect;
	u64 rd05_rxtrace_last_idx;
	u64 rd05_rxtrace_last_len;
	u64 rd05_rxtrace_last_opts1;
	u64 rd05_rxtrace_last_opts2;
	u64 rd05_rxtrace_last_opts3;
	u64 rd05_rxtrace_last_opts4;
	u64 rd05_rxtrace_last_opts5;
	u64 rd05_rxdesc_last_ext3;
	u64 rd05_rxdesc_last_src13;
	u64 rd05_rxdesc_last_src24;
	u64 rd05_rxdesc_last_spa5;
	u64 rd05_rxdesc_ext3_0;
	u64 rd05_rxdesc_ext3_2;
	u64 rd05_rxdesc_src13_p6;
	u64 rd05_rxdesc_src13_p7;
	u64 rd05_rxdesc_src13_other;
	u64 rd05_rxdesc_spa5_0;
	u64 rd05_rxdesc_spa5_2;
	u64 rd05_magic_hits;
	u64 rd05_magic_last_idx;
	u64 rd05_magic_last_len;
	u64 rd05_magic_last_candidate;
	u64 rd05_magic_last_proto0;
	u64 rd05_magic_last_proto4;
	u64 rd05_magic_last_proto8;
	u64 rd05_magic_last_src_lo;
	u64 rd05_magic_last_dst_lo;
	u64 rd05_l2cpu_runs;
	u64 rd05_l2cpu_ok;
	u64 rd05_l2cpu_fail;
	u64 rd05_l2cpu_last_status;
	u64 rd05_l2cpu_last_eidx;
	u64 rd05_l2cpu_last_w0;
	u64 rd05_l2cpu_last_w1;
	u64 rd05_l2cpu_last_swtacr;
	u64 rd05_pipe_runs;
	u64 rd05_pipe_ok;
	u64 rd05_pipe_fail;
	u64 rd05_pvid_ok;
	u64 rd05_netif_ok;
	u64 rd05_trap_ok;
	u64 rd05_pipe_last_reg;
	u64 rd05_pipe_last_val;
	u64 rd05_pipe_last_read;
	u64 rd05_acl_runs;
	u64 rd05_acl_ok;
	u64 rd05_acl_fail;
	u64 rd05_acl_last_idx;
	u64 rd05_acl_last_w7;
	u64 rd05_acl_last_status;
	u64 rd05_ffcr_read;
	u64 rd05_daclrcr_read;
	u64 rd05_ptrapcr_read;

	/* Linux 6.6 mainstream replacement for SDK private IOCTL/FastPath hooks.
	 * These counters show that tc/nft flowtable requests reached the driver.
	 * Hardware programming is intentionally conservative: unsupported rules return
	 * -EOPNOTSUPP so Linux can fall back to software instead of silently using
	 * Realtek private ABI state.
	 */
	u64 tc_setup_block_calls;
	u64 tc_setup_ft_calls;
	u64 tc_cls_flower_replace;
	u64 tc_cls_flower_destroy;
	u64 tc_cls_flower_stats;
	u64 tc_unsupported;
	u64 tc_ft_bind;
	u64 tc_ft_unbind;
	u64 tc_acl_drop_seen;
	u64 tc_acl_trap_seen;

	/* RD05 SDK descriptor mode.  RTL8197F SDK defines
	 * CONFIG_RTL_SWITCH_NEW_DESCRIPTOR, so normal RD05 operation uses the
	 * 8-dword CPURPDCR0/CPUTPDCR0 descriptor rings.  The legacy rxmbuf
	 * allocator is kept only as a disabled fallback/debug path.
	 */
	bool rd05_sdk_newdesc_rx;
	bool rd05_legacy_rx_enabled;
	__le32 *rd05_rx_ph_ring;
	dma_addr_t rd05_rx_ph_ring_dma;
	__le32 *rd05_rx_mbuf_ring;
	dma_addr_t rd05_rx_mbuf_ring_dma;
	struct rtl8197f_rtk_rd05_pkthdr *rd05_rx_ph;
	dma_addr_t rd05_rx_ph_dma;
	struct rtl8197f_rtk_rd05_mbuf *rd05_rx_mbuf;
	dma_addr_t rd05_rx_mbuf_dma;
	u32 rd05_legacy_rx_tail;
	u64 rd05_legacy_rx_init_ok;
	u64 rd05_legacy_rx_pkts;
	u64 rd05_legacy_rx_recycles;
	u64 rd05_legacy_rx_bad;
	u64 rd05_legacy_rx_empty;
	u64 rd05_legacy_rx_last_ent;
	u64 rd05_legacy_rx_last_len;
	u64 rd05_legacy_rx_ring_base;
	u64 rd05_legacy_mbuf_ring_base;

	bool rx_restart_pending;
};

enum rtl8197f_rtk_stat {
	RTL_RTK_STAT_IRQ_RX_DONE,
	RTL_RTK_STAT_IRQ_TX_DONE,
	RTL_RTK_STAT_IRQ_RX_ERRORS,
	RTL_RTK_STAT_IRQ_TX_ERRORS,
	RTL_RTK_STAT_IRQ_RX_RUNOUT,
	RTL_RTK_STAT_IRQ_MBUF_RUNOUT,
	RTL_RTK_STAT_IRQ_LINK_CHANGE,
	RTL_RTK_STAT_NAPI_POLLS,
	RTL_RTK_STAT_RX_BAD_DESC,
	RTL_RTK_STAT_RX_MISSING_SKB,
	RTL_RTK_STAT_RX_ALLOC_FAIL,
	RTL_RTK_STAT_RX_DMA_ERRORS,
	RTL_RTK_STAT_RX_CDP_READS,
	RTL_RTK_STAT_RX_CDP_ADVANCED,
	RTL_RTK_STAT_RX_CDP_OWNED_ADVANCED,
	RTL_RTK_STAT_RX_CDP_INVALID,
	RTL_RTK_STAT_RX_CDP_LAST,
	RTL_RTK_STAT_RX_CDP_LAST_IDX,
	RTL_RTK_STAT_TX_MTU_DROPS,
	RTL_RTK_STAT_TX_DMA_ERRORS,
	RTL_RTK_STAT_TX_RING_FULL,
	RTL_RTK_STAT_TX_BUSY_DESC,
	RTL_RTK_STAT_TX_TIMEOUTS,
	RTL_RTK_STAT_HW_RX_RESTARTS,
	RTL_RTK_STAT_RX_POLL_TIMER_RUNS,
	RTL_RTK_STAT_SW_TRXRDY_WRITES,
	RTL_RTK_STAT_CPUICR1_SDK_WRITES,
	RTL_RTK_STAT_SWCORE_SDK_WRITES,
	RTL_RTK_STAT_TXRINGCR_SDK_WRITES,
	RTL_RTK_STAT_SIRR_DIRECT_WRITES,
	RTL_RTK_STAT_GIMR_SDK_WRITES,
	RTL_RTK_STAT_ACL0_SDK_READS,
	RTL_RTK_STAT_TX_KICKS,
	RTL_RTK_STAT_TX_CRC_LEN_ADJUSTS,
	RTL_RTK_STAT_RX_CRC_STRIPS,
	RTL_RTK_STAT_RD05_RX_UNTAGGED,
	RTL_RTK_STAT_RD05_RX_EXTERNAL,
	RTL_RTK_STAT_RD05_RX_SELF_LOOP,
	RTL_RTK_STAT_RD05_RX_ARP,
	RTL_RTK_STAT_RD05_RX_IPV4,
	RTL_RTK_STAT_RD05_RX_IPV6,
	RTL_RTK_STAT_RD05_RX_OTHER,
	RTL_RTK_STAT_RD05_RX_BCAST,
	RTL_RTK_STAT_RD05_RX_MCAST,
	RTL_RTK_STAT_RD05_RX_UCAST,
	RTL_RTK_STAT_RD05_RX_DIRECT_LAN2,
	RTL_RTK_STAT_RD05_RXTRACE_FRAMES,
	RTL_RTK_STAT_RD05_RXTRACE_LOGS,
	RTL_RTK_STAT_RD05_RXTRACE_BCAST0,
	RTL_RTK_STAT_RD05_RXTRACE_BCAST4,
	RTL_RTK_STAT_RD05_RXTRACE_BCAST8,
	RTL_RTK_STAT_RD05_RXTRACE_ARP0,
	RTL_RTK_STAT_RD05_RXTRACE_ARP4,
	RTL_RTK_STAT_RD05_RXTRACE_ARP8,
	RTL_RTK_STAT_RD05_RXTRACE_RTK0,
	RTL_RTK_STAT_RD05_RXTRACE_RTK4,
	RTL_RTK_STAT_RD05_RXTRACE_SUSPECT,
	RTL_RTK_STAT_RD05_RXTRACE_LAST_IDX,
	RTL_RTK_STAT_RD05_RXTRACE_LAST_LEN,
	RTL_RTK_STAT_RD05_RXTRACE_LAST_OPTS1,
	RTL_RTK_STAT_RD05_RXTRACE_LAST_OPTS2,
	RTL_RTK_STAT_RD05_RXTRACE_LAST_OPTS3,
	RTL_RTK_STAT_RD05_RXTRACE_LAST_OPTS4,
	RTL_RTK_STAT_RD05_RXTRACE_LAST_OPTS5,
	RTL_RTK_STAT_RD05_RXDESC_LAST_EXT3,
	RTL_RTK_STAT_RD05_RXDESC_LAST_SRC13,
	RTL_RTK_STAT_RD05_RXDESC_LAST_SRC24,
	RTL_RTK_STAT_RD05_RXDESC_LAST_SPA5,
	RTL_RTK_STAT_RD05_RXDESC_EXT3_0,
	RTL_RTK_STAT_RD05_RXDESC_EXT3_2,
	RTL_RTK_STAT_RD05_RXDESC_SRC13_P6,
	RTL_RTK_STAT_RD05_RXDESC_SRC13_P7,
	RTL_RTK_STAT_RD05_RXDESC_SRC13_OTHER,
	RTL_RTK_STAT_RD05_RXDESC_SPA5_0,
	RTL_RTK_STAT_RD05_RXDESC_SPA5_2,
	RTL_RTK_STAT_RD05_MAGIC_HITS,
	RTL_RTK_STAT_RD05_MAGIC_LAST_IDX,
	RTL_RTK_STAT_RD05_MAGIC_LAST_LEN,
	RTL_RTK_STAT_RD05_MAGIC_LAST_CANDIDATE,
	RTL_RTK_STAT_RD05_MAGIC_LAST_PROTO0,
	RTL_RTK_STAT_RD05_MAGIC_LAST_PROTO4,
	RTL_RTK_STAT_RD05_MAGIC_LAST_PROTO8,
	RTL_RTK_STAT_RD05_MAGIC_LAST_SRC_LO,
	RTL_RTK_STAT_RD05_MAGIC_LAST_DST_LO,
	RTL_RTK_STAT_RD05_L2CPU_RUNS,
	RTL_RTK_STAT_RD05_L2CPU_OK,
	RTL_RTK_STAT_RD05_L2CPU_FAIL,
	RTL_RTK_STAT_RD05_L2CPU_LAST_STATUS,
	RTL_RTK_STAT_RD05_L2CPU_LAST_EIDX,
	RTL_RTK_STAT_RD05_L2CPU_LAST_W0,
	RTL_RTK_STAT_RD05_L2CPU_LAST_W1,
	RTL_RTK_STAT_RD05_L2CPU_LAST_SWTACR,
	RTL_RTK_STAT_RD05_PIPE_RUNS,
	RTL_RTK_STAT_RD05_PIPE_OK,
	RTL_RTK_STAT_RD05_PIPE_FAIL,
	RTL_RTK_STAT_RD05_PVID_OK,
	RTL_RTK_STAT_RD05_NETIF_OK,
	RTL_RTK_STAT_RD05_TRAP_OK,
	RTL_RTK_STAT_RD05_PIPE_LAST_REG,
	RTL_RTK_STAT_RD05_PIPE_LAST_VAL,
	RTL_RTK_STAT_RD05_PIPE_LAST_READ,
	RTL_RTK_STAT_RD05_ACL_RUNS,
	RTL_RTK_STAT_RD05_ACL_OK,
	RTL_RTK_STAT_RD05_ACL_FAIL,
	RTL_RTK_STAT_RD05_ACL_LAST_IDX,
	RTL_RTK_STAT_RD05_ACL_LAST_W7,
	RTL_RTK_STAT_RD05_ACL_LAST_STATUS,
	RTL_RTK_STAT_RD05_FFCR_READ,
	RTL_RTK_STAT_RD05_DACLRCR_READ,
	RTL_RTK_STAT_RD05_PTRAPCR_READ,
	RTL_RTK_STAT_TC_SETUP_BLOCK_CALLS,
	RTL_RTK_STAT_TC_SETUP_FT_CALLS,
	RTL_RTK_STAT_TC_CLS_FLOWER_REPLACE,
	RTL_RTK_STAT_TC_CLS_FLOWER_DESTROY,
	RTL_RTK_STAT_TC_CLS_FLOWER_STATS,
	RTL_RTK_STAT_TC_UNSUPPORTED,
	RTL_RTK_STAT_TC_FT_BIND,
	RTL_RTK_STAT_TC_FT_UNBIND,
	RTL_RTK_STAT_TC_ACL_DROP_SEEN,
	RTL_RTK_STAT_TC_ACL_TRAP_SEEN,
	RTL_RTK_STAT_DESC_STRIDE,
	RTL_RTK_STAT_TX_DESC0_OPTS1,
	RTL_RTK_STAT_TX_DESC0_OPTS2,
	RTL_RTK_STAT_TX_DESC0_OPTS4,
	RTL_RTK_STAT_RX_DESC0_OPTS1,
	RTL_RTK_STAT_RX_DESC0_OPTS2,
	RTL_RTK_STAT_RX_DESC0_OPTS3,
	RTL_RTK_STAT_RX_DESC0_OPTS4,
	RTL_RTK_STAT_RX_DESC0_OPTS5,
	RTL_RTK_STAT_LAST_CPUICR,
	RTL_RTK_STAT_LAST_CPUICR1,
	RTL_RTK_STAT_LAST_TXRINGCR,
	RTL_RTK_STAT_HW_RX_DESC_BASE,
	RTL_RTK_STAT_HW_TX_DESC_BASE,
	RTL_RTK_STAT_LAST_DMA_CR0,
	RTL_RTK_STAT_LAST_CPUQDM0,
	RTL_RTK_STAT_LAST_CPUQDM2,
	RTL_RTK_STAT_LAST_CPUQDM4,
	RTL_RTK_STAT_LAST_SIRR,
	RTL_RTK_STAT_LAST_MACCTRL1,
	RTL_RTK_STAT_LAST_SYS_CLK_MAG,
	RTL_RTK_STAT_REG_CPURPDCR0,
	RTL_RTK_STAT_REG_CPURMDCR0,
	RTL_RTK_STAT_REG_CPUTPDCR0,
	RTL_RTK_STAT_RD05_SDK_NEWDESC_RX,
	RTL_RTK_STAT_RD05_LEGACY_RX_ENABLED,
	RTL_RTK_STAT_RD05_LEGACY_RX_INIT_OK,
	RTL_RTK_STAT_RD05_LEGACY_RX_PKTS,
	RTL_RTK_STAT_RD05_LEGACY_RX_RECYCLES,
	RTL_RTK_STAT_RD05_LEGACY_RX_BAD,
	RTL_RTK_STAT_RD05_LEGACY_RX_EMPTY,
	RTL_RTK_STAT_RD05_LEGACY_RX_LAST_ENT,
	RTL_RTK_STAT_RD05_LEGACY_RX_LAST_LEN,
	RTL_RTK_STAT_RD05_LEGACY_RX_RING_BASE,
	RTL_RTK_STAT_RD05_LEGACY_MBUF_RING_BASE,
	RTL_RTK_STAT_REG_DMA_CR1,
	RTL_RTK_STAT_REG_DMA_CR4,
	RTL_RTK_STAT_REG_CPUIIMR,
	RTL_RTK_STAT_REG_CPUIISR,
};

static const char rtl8197f_rtk_gstrings_stats[][ETH_GSTRING_LEN] = {
	[RTL_RTK_STAT_IRQ_RX_DONE]		= "irq_rx_done",
	[RTL_RTK_STAT_IRQ_TX_DONE]		= "irq_tx_done",
	[RTL_RTK_STAT_IRQ_RX_ERRORS]		= "irq_rx_errors",
	[RTL_RTK_STAT_IRQ_TX_ERRORS]		= "irq_tx_errors",
	[RTL_RTK_STAT_IRQ_RX_RUNOUT]		= "irq_rx_runout",
	[RTL_RTK_STAT_IRQ_MBUF_RUNOUT]	= "irq_mbuf_runout",
	[RTL_RTK_STAT_IRQ_LINK_CHANGE]	= "irq_link_change",
	[RTL_RTK_STAT_NAPI_POLLS]		= "napi_polls",
	[RTL_RTK_STAT_RX_BAD_DESC]		= "rx_bad_desc",
	[RTL_RTK_STAT_RX_MISSING_SKB]	= "rx_missing_skb",
	[RTL_RTK_STAT_RX_ALLOC_FAIL]		= "rx_alloc_fail",
	[RTL_RTK_STAT_RX_DMA_ERRORS]	= "rx_dma_errors",
	[RTL_RTK_STAT_RX_CDP_READS]	= "rx_cdp_reads",
	[RTL_RTK_STAT_RX_CDP_ADVANCED]	= "rx_cdp_advanced",
	[RTL_RTK_STAT_RX_CDP_OWNED_ADVANCED] = "rx_cdp_owned_advanced",
	[RTL_RTK_STAT_RX_CDP_INVALID]	= "rx_cdp_invalid",
	[RTL_RTK_STAT_RX_CDP_LAST]	= "rx_cdp_last",
	[RTL_RTK_STAT_RX_CDP_LAST_IDX]	= "rx_cdp_last_idx",
	[RTL_RTK_STAT_TX_MTU_DROPS]		= "tx_mtu_drops",
	[RTL_RTK_STAT_TX_DMA_ERRORS]		= "tx_dma_errors",
	[RTL_RTK_STAT_TX_RING_FULL]		= "tx_ring_full",
	[RTL_RTK_STAT_TX_BUSY_DESC]		= "tx_busy_desc",
	[RTL_RTK_STAT_TX_TIMEOUTS]		= "tx_timeouts",
	[RTL_RTK_STAT_HW_RX_RESTARTS]	= "hw_rx_restarts",
	[RTL_RTK_STAT_RX_POLL_TIMER_RUNS]	= "rx_poll_timer_runs",
	[RTL_RTK_STAT_SW_TRXRDY_WRITES]	= "sw_trxrdy_writes",
	[RTL_RTK_STAT_CPUICR1_SDK_WRITES]	= "cpuicr1_sdk_writes",
	[RTL_RTK_STAT_SWCORE_SDK_WRITES]	= "swcore_sdk_writes",
	[RTL_RTK_STAT_TXRINGCR_SDK_WRITES]	= "txringcr_sdk_writes",
	[RTL_RTK_STAT_SIRR_DIRECT_WRITES]	= "sirr_direct_writes",
	[RTL_RTK_STAT_GIMR_SDK_WRITES]	= "gimr_sdk_writes",
	[RTL_RTK_STAT_ACL0_SDK_READS]	= "acl0_sdk_reads",
	[RTL_RTK_STAT_TX_KICKS]		= "tx_kicks",
	[RTL_RTK_STAT_TX_CRC_LEN_ADJUSTS]	= "tx_crc_len_adjusts",
	[RTL_RTK_STAT_RX_CRC_STRIPS]	= "rx_crc_strips",
	[RTL_RTK_STAT_RD05_RX_UNTAGGED]	= "rd05_rx_untagged",
	[RTL_RTK_STAT_RD05_RX_EXTERNAL]	= "rd05_rx_external",
	[RTL_RTK_STAT_RD05_RX_SELF_LOOP]	= "rd05_rx_self_loop",
	[RTL_RTK_STAT_RD05_RX_ARP]	= "rd05_rx_arp",
	[RTL_RTK_STAT_RD05_RX_IPV4]	= "rd05_rx_ipv4",
	[RTL_RTK_STAT_RD05_RX_IPV6]	= "rd05_rx_ipv6",
	[RTL_RTK_STAT_RD05_RX_OTHER]	= "rd05_rx_other",
	[RTL_RTK_STAT_RD05_RX_BCAST]	= "rd05_rx_bcast",
	[RTL_RTK_STAT_RD05_RX_MCAST]	= "rd05_rx_mcast",
	[RTL_RTK_STAT_RD05_RX_UCAST]	= "rd05_rx_ucast",
	[RTL_RTK_STAT_RD05_RX_DIRECT_LAN2]	= "rd05_rx_direct_lan2",
	[RTL_RTK_STAT_RD05_RXTRACE_FRAMES]	= "rd05_rxtrace_frames",
	[RTL_RTK_STAT_RD05_RXTRACE_LOGS]	= "rd05_rxtrace_logs",
	[RTL_RTK_STAT_RD05_RXTRACE_BCAST0]	= "rd05_rxtrace_bcast0",
	[RTL_RTK_STAT_RD05_RXTRACE_BCAST4]	= "rd05_rxtrace_bcast4",
	[RTL_RTK_STAT_RD05_RXTRACE_BCAST8]	= "rd05_rxtrace_bcast8",
	[RTL_RTK_STAT_RD05_RXTRACE_ARP0]	= "rd05_rxtrace_arp0",
	[RTL_RTK_STAT_RD05_RXTRACE_ARP4]	= "rd05_rxtrace_arp4",
	[RTL_RTK_STAT_RD05_RXTRACE_ARP8]	= "rd05_rxtrace_arp8",
	[RTL_RTK_STAT_RD05_RXTRACE_RTK0]	= "rd05_rxtrace_rtk0",
	[RTL_RTK_STAT_RD05_RXTRACE_RTK4]	= "rd05_rxtrace_rtk4",
	[RTL_RTK_STAT_RD05_RXTRACE_SUSPECT]	= "rd05_rxtrace_suspect",
	[RTL_RTK_STAT_RD05_RXTRACE_LAST_IDX]	= "rd05_rxtrace_last_idx",
	[RTL_RTK_STAT_RD05_RXTRACE_LAST_LEN]	= "rd05_rxtrace_last_len",
	[RTL_RTK_STAT_RD05_RXTRACE_LAST_OPTS1]	= "rd05_rxtrace_last_opts1",
	[RTL_RTK_STAT_RD05_RXTRACE_LAST_OPTS2]	= "rd05_rxtrace_last_opts2",
	[RTL_RTK_STAT_RD05_RXTRACE_LAST_OPTS3]	= "rd05_rxtrace_last_opts3",
	[RTL_RTK_STAT_RD05_RXTRACE_LAST_OPTS4]	= "rd05_rxtrace_last_opts4",
	[RTL_RTK_STAT_RD05_RXTRACE_LAST_OPTS5]	= "rd05_rxtrace_last_opts5",
	[RTL_RTK_STAT_RD05_RXDESC_LAST_EXT3]	= "rd05_rxdesc_last_ext3",
	[RTL_RTK_STAT_RD05_RXDESC_LAST_SRC13]	= "rd05_rxdesc_last_src13",
	[RTL_RTK_STAT_RD05_RXDESC_LAST_SRC24]	= "rd05_rxdesc_last_src24",
	[RTL_RTK_STAT_RD05_RXDESC_LAST_SPA5]	= "rd05_rxdesc_last_spa5",
	[RTL_RTK_STAT_RD05_RXDESC_EXT3_0]	= "rd05_rxdesc_ext3_0",
	[RTL_RTK_STAT_RD05_RXDESC_EXT3_2]	= "rd05_rxdesc_ext3_2",
	[RTL_RTK_STAT_RD05_RXDESC_SRC13_P6]	= "rd05_rxdesc_src13_p6",
	[RTL_RTK_STAT_RD05_RXDESC_SRC13_P7]	= "rd05_rxdesc_src13_p7",
	[RTL_RTK_STAT_RD05_RXDESC_SRC13_OTHER]	= "rd05_rxdesc_src13_other",
	[RTL_RTK_STAT_RD05_RXDESC_SPA5_0]	= "rd05_rxdesc_spa5_0",
	[RTL_RTK_STAT_RD05_RXDESC_SPA5_2]	= "rd05_rxdesc_spa5_2",
	[RTL_RTK_STAT_RD05_MAGIC_HITS]	= "rd05_magic_hits",
	[RTL_RTK_STAT_RD05_MAGIC_LAST_IDX]	= "rd05_magic_last_idx",
	[RTL_RTK_STAT_RD05_MAGIC_LAST_LEN]	= "rd05_magic_last_len",
	[RTL_RTK_STAT_RD05_MAGIC_LAST_CANDIDATE]	= "rd05_magic_last_candidate",
	[RTL_RTK_STAT_RD05_MAGIC_LAST_PROTO0]	= "rd05_magic_last_proto0",
	[RTL_RTK_STAT_RD05_MAGIC_LAST_PROTO4]	= "rd05_magic_last_proto4",
	[RTL_RTK_STAT_RD05_MAGIC_LAST_PROTO8]	= "rd05_magic_last_proto8",
	[RTL_RTK_STAT_RD05_MAGIC_LAST_SRC_LO]	= "rd05_magic_last_src_lo",
	[RTL_RTK_STAT_RD05_MAGIC_LAST_DST_LO]	= "rd05_magic_last_dst_lo",
	[RTL_RTK_STAT_RD05_L2CPU_RUNS]	= "rd05_l2cpu_runs",
	[RTL_RTK_STAT_RD05_L2CPU_OK]	= "rd05_l2cpu_ok",
	[RTL_RTK_STAT_RD05_L2CPU_FAIL]	= "rd05_l2cpu_fail",
	[RTL_RTK_STAT_RD05_L2CPU_LAST_STATUS]	= "rd05_l2cpu_last_status",
	[RTL_RTK_STAT_RD05_L2CPU_LAST_EIDX]	= "rd05_l2cpu_last_eidx",
	[RTL_RTK_STAT_RD05_L2CPU_LAST_W0]	= "rd05_l2cpu_last_w0",
	[RTL_RTK_STAT_RD05_L2CPU_LAST_W1]	= "rd05_l2cpu_last_w1",
	[RTL_RTK_STAT_RD05_L2CPU_LAST_SWTACR]	= "rd05_l2cpu_last_swtacr",
	[RTL_RTK_STAT_RD05_PIPE_RUNS]	= "rd05_pipe_runs",
	[RTL_RTK_STAT_RD05_PIPE_OK]	= "rd05_pipe_ok",
	[RTL_RTK_STAT_RD05_PIPE_FAIL]	= "rd05_pipe_fail",
	[RTL_RTK_STAT_RD05_PVID_OK]	= "rd05_pvid_ok",
	[RTL_RTK_STAT_RD05_NETIF_OK]	= "rd05_netif_ok",
	[RTL_RTK_STAT_RD05_TRAP_OK]	= "rd05_trap_ok",
	[RTL_RTK_STAT_RD05_PIPE_LAST_REG]	= "rd05_pipe_last_reg",
	[RTL_RTK_STAT_RD05_PIPE_LAST_VAL]	= "rd05_pipe_last_val",
	[RTL_RTK_STAT_RD05_PIPE_LAST_READ]	= "rd05_pipe_last_read",
	[RTL_RTK_STAT_RD05_ACL_RUNS]	= "rd05_acl_runs",
	[RTL_RTK_STAT_RD05_ACL_OK]	= "rd05_acl_ok",
	[RTL_RTK_STAT_RD05_ACL_FAIL]	= "rd05_acl_fail",
	[RTL_RTK_STAT_RD05_ACL_LAST_IDX]	= "rd05_acl_last_idx",
	[RTL_RTK_STAT_RD05_ACL_LAST_W7]	= "rd05_acl_last_w7",
	[RTL_RTK_STAT_RD05_ACL_LAST_STATUS]	= "rd05_acl_last_status",
	[RTL_RTK_STAT_RD05_FFCR_READ]	= "rd05_ffcr_read",
	[RTL_RTK_STAT_RD05_DACLRCR_READ]	= "rd05_daclrcr_read",
	[RTL_RTK_STAT_RD05_PTRAPCR_READ]	= "rd05_ptrapcr_read",
	[RTL_RTK_STAT_TC_SETUP_BLOCK_CALLS]	= "tc_setup_block_calls",
	[RTL_RTK_STAT_TC_SETUP_FT_CALLS]	= "tc_setup_ft_calls",
	[RTL_RTK_STAT_TC_CLS_FLOWER_REPLACE]	= "tc_cls_flower_replace",
	[RTL_RTK_STAT_TC_CLS_FLOWER_DESTROY]	= "tc_cls_flower_destroy",
	[RTL_RTK_STAT_TC_CLS_FLOWER_STATS]	= "tc_cls_flower_stats",
	[RTL_RTK_STAT_TC_UNSUPPORTED]	= "tc_unsupported",
	[RTL_RTK_STAT_TC_FT_BIND]	= "tc_ft_bind",
	[RTL_RTK_STAT_TC_FT_UNBIND]	= "tc_ft_unbind",
	[RTL_RTK_STAT_TC_ACL_DROP_SEEN]	= "tc_acl_drop_seen",
	[RTL_RTK_STAT_TC_ACL_TRAP_SEEN]	= "tc_acl_trap_seen",
	[RTL_RTK_STAT_DESC_STRIDE]	= "desc_stride",
	[RTL_RTK_STAT_TX_DESC0_OPTS1]	= "tx_desc0_opts1",
	[RTL_RTK_STAT_TX_DESC0_OPTS2]	= "tx_desc0_opts2",
	[RTL_RTK_STAT_TX_DESC0_OPTS4]	= "tx_desc0_opts4",
	[RTL_RTK_STAT_RX_DESC0_OPTS1]	= "rx_desc0_opts1",
	[RTL_RTK_STAT_RX_DESC0_OPTS2]	= "rx_desc0_opts2",
	[RTL_RTK_STAT_RX_DESC0_OPTS3]	= "rx_desc0_opts3",
	[RTL_RTK_STAT_RX_DESC0_OPTS4]	= "rx_desc0_opts4",
	[RTL_RTK_STAT_RX_DESC0_OPTS5]	= "rx_desc0_opts5",
	[RTL_RTK_STAT_LAST_CPUICR]	= "last_cpuicr",
	[RTL_RTK_STAT_LAST_CPUICR1]	= "last_cpuicr1",
	[RTL_RTK_STAT_LAST_TXRINGCR]	= "last_txringcr",
	[RTL_RTK_STAT_HW_RX_DESC_BASE]	= "hw_rx_desc_base",
	[RTL_RTK_STAT_HW_TX_DESC_BASE]	= "hw_tx_desc_base",
	[RTL_RTK_STAT_LAST_DMA_CR0]	= "last_dma_cr0",
	[RTL_RTK_STAT_LAST_CPUQDM0]	= "last_cpuqdm0",
	[RTL_RTK_STAT_LAST_CPUQDM2]	= "last_cpuqdm2",
	[RTL_RTK_STAT_LAST_CPUQDM4]	= "last_cpuqdm4",
	[RTL_RTK_STAT_LAST_SIRR]	= "last_sirr",
	[RTL_RTK_STAT_LAST_MACCTRL1]	= "last_macctrl1",
	[RTL_RTK_STAT_LAST_SYS_CLK_MAG]	= "last_sys_clk_mag",
	[RTL_RTK_STAT_REG_CPURPDCR0]	= "reg_cpurpdcr0",
	[RTL_RTK_STAT_REG_CPURMDCR0]	= "reg_cpurmdcr0",
	[RTL_RTK_STAT_REG_CPUTPDCR0]	= "reg_cputpdcr0",
	[RTL_RTK_STAT_RD05_SDK_NEWDESC_RX]	= "rd05_sdk_newdesc_rx",
	[RTL_RTK_STAT_RD05_LEGACY_RX_ENABLED]	= "rd05_legacy_rx_enabled",
	[RTL_RTK_STAT_RD05_LEGACY_RX_INIT_OK]	= "rd05_legacy_rx_init_ok",
	[RTL_RTK_STAT_RD05_LEGACY_RX_PKTS]	= "rd05_legacy_rx_pkts",
	[RTL_RTK_STAT_RD05_LEGACY_RX_RECYCLES]	= "rd05_legacy_rx_recycles",
	[RTL_RTK_STAT_RD05_LEGACY_RX_BAD]	= "rd05_legacy_rx_bad",
	[RTL_RTK_STAT_RD05_LEGACY_RX_EMPTY]	= "rd05_legacy_rx_empty",
	[RTL_RTK_STAT_RD05_LEGACY_RX_LAST_ENT]	= "rd05_legacy_rx_last_ent",
	[RTL_RTK_STAT_RD05_LEGACY_RX_LAST_LEN]	= "rd05_legacy_rx_last_len",
	[RTL_RTK_STAT_RD05_LEGACY_RX_RING_BASE]	= "rd05_legacy_rx_ring_base",
	[RTL_RTK_STAT_RD05_LEGACY_MBUF_RING_BASE]	= "rd05_legacy_mbuf_ring_base",
	[RTL_RTK_STAT_REG_DMA_CR1]	= "reg_dma_cr1",
	[RTL_RTK_STAT_REG_DMA_CR4]	= "reg_dma_cr4",
	[RTL_RTK_STAT_REG_CPUIIMR]	= "reg_cpuiimr",
	[RTL_RTK_STAT_REG_CPUIISR]	= "reg_cpuiisr",
};

#define RTL_RTK_NUM_STATS	ARRAY_SIZE(rtl8197f_rtk_gstrings_stats)

static inline u32 rtl8197f_rtk_read(struct rtl8197f_rtknet_priv *priv,
					    u32 reg)
{
	return readl(priv->base + reg);
}

static inline void rtl8197f_rtk_write(struct rtl8197f_rtknet_priv *priv,
					     u32 reg, u32 val)
{
	writel(val, priv->base + reg);
}

static void __iomem *rtl8197f_rtk_ioremap_norequest(struct platform_device *pdev,
							   const char *name)
{
	struct resource *res;

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, name);
	if (!res)
		return NULL;

	/* The RTL8197F system window can already be reserved by early platform code. */
	return devm_ioremap(&pdev->dev, res->start, resource_size(res));
}

static u32 rtl8197f_rtk_hw_dma_addr(struct rtl8197f_rtknet_priv *priv,
					    dma_addr_t dma)
{
	u32 addr = lower_32_bits(dma);

	/*
	 * Realtek's 4.4 SDK programs KSEG1-looking descriptor/buffer pointers
	 * into the CPU-interface registers.  Linux DMA API users usually program
	 * physical bus addresses.  Keep the default native, but allow board DTS to
	 * request the SDK-compatible address form while bring-up is ongoing.
	 */
	if (priv->desc_addr_physical)
		return addr;

	if (priv->desc_addr_kseg1)
		addr = (addr & 0x1fffffff) | 0xa0000000;

	return addr;
}

static u32 rtl8197f_rtk_rd05_legacy_dma_addr(dma_addr_t dma)
{
	/*
	 * RD05 v11 proved that CPURPDCR0/CPURMDCR0 can be programmed, but the
	 * rtl865x RX engine never consumed KSEG1-form 0xa....... ring pointers:
	 * rd05_legacy_rx_empty rose while rd05_legacy_rx_pkts stayed zero.  The
	 * packet-header/mbuf rings are DMA-coherent Linux allocations, so expose
	 * their physical bus addresses to the legacy RTL865x ring walker even when
	 * the tagged TX/RX descriptor path still uses SDK-style KSEG1 addresses.
	 */
	return lower_32_bits(dma) & 0x1fffffff;
}

static u32 rtl8197f_rtk_next(u32 idx, u32 size)
{
	return (idx + 1 == size) ? 0 : idx + 1;
}

/*
 * The RTL8197F SDK uses CPURPDCR0 as the authoritative receive current
 * descriptor pointer (USE_SWITCH_RX_CDP).  A descriptor can still show OWN
 * when the hardware pointer has already advanced beyond it, especially with a
 * cached/non-coherent MIPS observer.  Validate the pointer strictly before
 * using it; an invalid value falls back to the ownership bit.
 */
static bool rtl8197f_rtk_rx_cdp_index(struct rtl8197f_rtknet_priv *priv,
				     u32 *hw_idx)
{
	u32 base = rtl8197f_rtk_hw_dma_addr(priv, priv->rx_desc_dma);
	u32 span = priv->rx_ring_size * sizeof(struct rtl8197f_rtk_desc);
	u32 cdp = rtl8197f_rtk_read(priv, RTL_RTK_CPURPDCR0);
	u32 offset;

	priv->rx_cdp_reads++;
	priv->rx_cdp_last = cdp;

	if (cdp < base || cdp >= base + span) {
		priv->rx_cdp_invalid++;
		return false;
	}

	offset = cdp - base;
	if (offset % sizeof(struct rtl8197f_rtk_desc)) {
		priv->rx_cdp_invalid++;
		return false;
	}

	*hw_idx = offset / sizeof(struct rtl8197f_rtk_desc);
	priv->rx_cdp_last_idx = *hw_idx;
	return true;
}

static bool rtl8197f_rtk_tx_full(struct rtl8197f_rtknet_priv *priv)
{
	return rtl8197f_rtk_next(priv->tx_head, priv->tx_ring_size) ==
		priv->tx_tail;
}

static void rtl8197f_rtk_enable_irq(struct rtl8197f_rtknet_priv *priv)
{
	rtl8197f_rtk_write(priv, RTL_RTK_CPUIIMR, RTL_RTK_INT_MASK);
}

static void rtl8197f_rtk_disable_irq(struct rtl8197f_rtknet_priv *priv)
{
	rtl8197f_rtk_write(priv, RTL_RTK_CPUIIMR, 0);
}

static void rtl8197f_rtk_kick_rx(struct rtl8197f_rtknet_priv *priv)
{
	u32 val;

	val = rtl8197f_rtk_read(priv, RTL_RTK_CPUICR);
	val |= RTL_RTK_CPUICR_RXCMD;
	rtl8197f_rtk_write(priv, RTL_RTK_CPUICR, val);
	priv->hw_rx_restarts++;
}

static void rtl8197f_rtk_assert_trxrdy(struct rtl8197f_rtknet_priv *priv)
{
	if (!priv->swcore || !priv->sdk_trxrdy)
		return;

	/* SDK uses REG32(SIRR) = TRXRDY, not read/modify/write.
	 * Use a direct write so stale reset/initialisation bits are not preserved.
	 */
	writel(RTL_RTK_SWMISC_TRXRDY,
	       priv->swcore + RTL_RTK_SWCORE_SWMISC_BASE + RTL_RTK_SWMISC_SSIR);
	priv->sw_trxrdy_writes++;
	priv->sirr_direct_writes++;
}

static void rtl8197f_rtk_deassert_trxrdy(struct rtl8197f_rtknet_priv *priv)
{
	if (!priv->swcore || !priv->sdk_trxrdy)
		return;

	/* Match the vendor reinit path: stop the rtl865x TX/RX state machine before
	 * rewriting descriptor bases and CPUIF mode registers, then assert TRXRDY
	 * only after CPUICR/RXCMD and FIFO watermarks are programmed.
	 */
	writel(0, priv->swcore + RTL_RTK_SWCORE_SWMISC_BASE + RTL_RTK_SWMISC_SSIR);
	readl(priv->swcore + RTL_RTK_SWCORE_SWMISC_BASE + RTL_RTK_SWMISC_SSIR);
	priv->sirr_direct_writes++;
}

static void rtl8197f_rtk_apply_sdk_swcore_init(struct rtl8197f_rtknet_priv *priv)
{
	u32 val;

	if (!priv->sdk_swcore_init)
		return;

	/* RTL8197F SDK: REG32(MACCTRL1) |= CF_CMAC_CLK_SEL and, on 8197F,
	 * latch packet content from the first byte.  This matters while proving
	 * EXT1/RGMII ingress because frames can leave RTL8367D port7 yet never
	 * advance the CPU DMA ring if the CMAC side is not fully clocked/latching.
	 */
	if (priv->swcore) {
		val = readl(priv->swcore + RTL_RTK_SWCORE_MACCTRL1);
		val |= RTL_RTK_MACCTRL1_CMAC_CLK_SEL |
		       RTL_RTK_MACCTRL1_CMAC_LATPKT_EN;
		val &= ~RTL_RTK_MACCTRL1_CMAC_LATPKT_TYPE;
		writel(val, priv->swcore + RTL_RTK_SWCORE_MACCTRL1);
		readl(priv->swcore + RTL_RTK_SWCORE_MACCTRL1);
		priv->swcore_sdk_writes++;
	}

	/* RTL8197F-VG SDK: keep the switch-core and LX1 clock/arbiter active
	 * before programming the CPU-interface DMA rings.  The vendor BSP also
	 * toggles SYS_SW_CLK_ENABLE around switch-core reinit.  Without these
	 * gates the CPU_IFACE registers can accept posted writes but read back as
	 * zero and the TXFD kick never produces TX-done.
	 */
	if (priv->system) {
		val = readl(priv->system + RTL_RTK_SYSTEM_SYS_CLK_MAG);
		val |= RTL_RTK_SYS_SW_CLK_ENABLE |
		       RTL_RTK_SYS_CLK_ACTIVE_SWCORE |
		       RTL_RTK_SYS_CLK_ACTIVE_LX1_CLK |
		       RTL_RTK_SYS_CLK_ACTIVE_LX1_ARB |
		       RTL_RTK_SYS_CLK_ACTIVE_LX2_CLK |
		       RTL_RTK_SYS_CLK_ACTIVE_LX2_ARB;
		writel(val, priv->system + RTL_RTK_SYSTEM_SYS_CLK_MAG);
		/* Read back once to flush the posted write before CPU_IFACE access. */
		readl(priv->system + RTL_RTK_SYSTEM_SYS_CLK_MAG);
		priv->swcore_sdk_writes++;
	}
}

static void rtl8197f_rtk_apply_sdk_start_sidebands(struct rtl8197f_rtknet_priv *priv)
{
	u32 val;

	/* RTL8197F SDK rtl865x_start():
	 *   REG32(0xbb0c0000) = REG32(0xbb0c0000);
	 * This touches ACL table entry 0 and fixes a documented receive stall after
	 * link changes.  Keep it RD05-only by mapping the exact SDK physical address.
	 */
	if (priv->acl0) {
		val = readl(priv->acl0);
		writel(val, priv->acl0);
		priv->acl0_sdk_reads++;
	}

	/* SDK enables BSP_SW_IE in the global interrupt mask after CPUIIMR/SIRR.
	 * Linux intc normally owns this register, but OR'ing bit 15 is equivalent to
	 * the vendor switch-core IRQ gate and is useful while the CPU interface is
	 * brought up.
	 */
	if (priv->system) {
		val = readl(priv->system + RTL_RTK_SYSTEM_GIMR);
		val |= RTL_RTK_GIMR_BSP_SW_IE;
		writel(val, priv->system + RTL_RTK_SYSTEM_GIMR);
		priv->gimr_sdk_writes++;
	}
}


static void rtl8197f_rtk_rd05_init_p0_rgmii(struct rtl8197f_rtknet_priv *priv,
					       const char *reason)
{
	u32 pcr, gmii, val, pad = 0;

	if (!priv->swcore)
		return;

	/* GPL SDK init_8197f_p0(): RTL8197F/RTL8197FH connects an external
	 * RTL8367/RTL83xx switch through physical P0/RGMII, not through the
	 * internal extension/CPU descriptor path.  Release P0 from MAC reset,
	 * force the MAC-to-MAC link to 1G/full and put the port in forwarding.
	 */
	pcr = readl(priv->swcore + RTL_RTK_SWCORE_PCRP(0));
	pcr &= ~(RTL_RTK_PCRP_EXT_PHYID_MASK |
		 RTL_RTK_PCRP_EN_FORCE_MODE |
		 RTL_RTK_PCRP_POLL_LINK_STATUS |
		 RTL_RTK_PCRP_FORCE_LINK |
		 RTL_RTK_PCRP_FORCE_STATUS_MASK |
		 RTL_RTK_PCRP_PAUSE_MASK |
		 RTL_RTK_PCRP_STP_MASK |
		 RTL_RTK_PCRP_MAC_SW_RESET |
		 RTL_RTK_PCRP_ENABLE_PHY_IF);
	pcr |= (RTL_RTK_RD05_P0_EXT_PHY_ID << RTL_RTK_PCRP_EXT_PHYID_SHIFT) |
	       RTL_RTK_PCRP_EN_FORCE_MODE |
	       RTL_RTK_PCRP_FORCE_LINK |
	       RTL_RTK_PCRP_FORCE_SPEED_1000M |
	       RTL_RTK_PCRP_FORCE_DUPLEX |
	       RTL_RTK_PCRP_PAUSE_TXRX |
	       RTL_RTK_PCRP_MII_RXER |
	       RTL_RTK_PCRP_STP_FORWARDING |
	       RTL_RTK_PCRP_ENABLE_PHY_IF;
	writel(pcr, priv->swcore + RTL_RTK_SWCORE_PCRP(0));
	readl(priv->swcore + RTL_RTK_SWCORE_PCRP(0));
	udelay(10);
	pcr |= RTL_RTK_PCRP_MAC_SW_RESET;
	writel(pcr, priv->swcore + RTL_RTK_SWCORE_PCRP(0));
	readl(priv->swcore + RTL_RTK_SWCORE_PCRP(0));

	/* The RTL8367D emits its 8-byte protocol-4 CPU tag after the source MAC.
	 * Linux's rtl8_4 tagger must receive those bytes unchanged.  All supplied
	 * RTL8197F + RTL83xx SDK generations enable both CFG_CPUC_TAG and
	 * CFG_TX_CPUC_TAG on physical P0 together with PORT0_ROUTER_MODE; the latter
	 * is a P0 CPU-tag transmit/parser gate, not an instruction to synthesize a
	 * second tag in the DMA descriptor.  Keep MACCR1 tag removal disabled so DSA
	 * sees the complete tag.  v36 keeps the RTL8197FH-VG SDK defaults TX0/RX6
	 * but permits a bounded runtime sweep because the hardware trace proves that
	 * neither direction currently has a valid RGMII sampling window.
	 */
	{
		u32 tx_delay = min_t(u32, READ_ONCE(rd05_p0_tx_delay), 1);
		u32 rx_delay = min_t(u32, READ_ONCE(rd05_p0_rx_delay), 7);

		gmii = readl(priv->swcore + RTL_RTK_SWCORE_P0GMIICR);
		gmii &= ~(RTL_RTK_P0GMIICR_CPU_TAG_RX |
			  RTL_RTK_P0GMIICR_CPU_TAG_TX |
			  RTL_RTK_P0GMIICR_GMAC_MASK |
			  RTL_RTK_P0GMIICR_RGTXC_MASK |
			  RTL_RTK_P0GMIICR_TX_DELAY_MASK |
			  RTL_RTK_P0GMIICR_RX_DELAY_MASK |
			  RTL_RTK_P0GMIICR_CONF_DONE);
		gmii |= RTL_RTK_P0GMIICR_RGTXC_3 | rx_delay;
		if (tx_delay)
			gmii |= RTL_RTK_P0GMIICR_TX_DELAY_MASK;
	}
	if (priv->p0_cpu_tag_passthrough)
		gmii |= RTL_RTK_P0GMIICR_CPU_TAG_RX |
			RTL_RTK_P0GMIICR_CPU_TAG_TX;
	writel(gmii, priv->swcore + RTL_RTK_SWCORE_P0GMIICR);

	val = readl(priv->swcore + RTL_RTK_SWCORE_MACCR1);
	val &= ~(RTL_RTK_MACCR1_P0_ROUTER_MODE |
		 RTL_RTK_MACCR1_RMD_TAG_MASK);
	if (priv->p0_cpu_tag_passthrough)
		val |= RTL_RTK_MACCR1_P0_ROUTER_MODE;
	writel(val, priv->swcore + RTL_RTK_SWCORE_MACCR1);

	/* RTL8197F-VG init_8197f_p0() fixes the transmit inter-packet gap
	 * at eight bytes on both PITCR and EXTPCR0.  Without these fields the
	 * external RTL8367 can count each otherwise valid SoC frame as an FCS/drop
	 * event even when the RGMII delay taps themselves are in range.
	 */
	val = readl(priv->swcore + RTL_RTK_SWCORE_PITCR);
	val &= ~RTL_RTK_PITCR_FIX_IPG_MASK;
	val |= RTL_RTK_PITCR_P0_EXT_INTERFACE |
	       RTL_RTK_PITCR_FIX_IPG_8BYTE;
	writel(val, priv->swcore + RTL_RTK_SWCORE_PITCR);

	val = readl(priv->swcore + RTL_RTK_SWCORE_EXTPCR0);
	val &= ~RTL_RTK_EXTPCR0_TX_IPG_MASK;
	val |= RTL_RTK_EXTPCR0_TX_IPG_8BYTE;
	writel(val, priv->swcore + RTL_RTK_SWCORE_EXTPCR0);

	val = readl(priv->swcore + RTL_RTK_SWCORE_MACCR);
	val |= RTL_RTK_MACCR_P0_GIGA_LINK;
	writel(val, priv->swcore + RTL_RTK_SWCORE_MACCR);

	if (priv->system) {
		pad = readl(priv->system + RTL_RTK_SYSTEM_PAD_CTRL_1);
		pad &= ~(RTL_RTK_PAD_P0_RGMII_DN_MASK |
			 RTL_RTK_PAD_P0_RGMII_DP_MASK |
			 RTL_RTK_PAD_P0_RGMII_MODE |
			 RTL_RTK_PAD_P0_TX_E2 |
			 RTL_RTK_PAD_P0_VG_BIT21);
		pad |= (RTL_RTK_PAD_P0_DRIVE_VG << RTL_RTK_PAD_P0_RGMII_DN_SHIFT) |
		       (RTL_RTK_PAD_P0_DRIVE_VG << RTL_RTK_PAD_P0_RGMII_DP_SHIFT) |
		       RTL_RTK_PAD_P0_RGMII_MODE |
		       RTL_RTK_PAD_P0_TX_E2 |
		       RTL_RTK_PAD_P0_VG_BIT21;
		writel(pad, priv->system + RTL_RTK_SYSTEM_PAD_CTRL_1);
		readl(priv->system + RTL_RTK_SYSTEM_PAD_CTRL_1);
	}

	/* Conf_done must be the final P0 interface write. */
	gmii |= RTL_RTK_P0GMIICR_CONF_DONE;
	writel(gmii, priv->swcore + RTL_RTK_SWCORE_P0GMIICR);
	readl(priv->swcore + RTL_RTK_SWCORE_P0GMIICR);

	dev_info(priv->dev,
		 "rd05 p0 rgmii v38.2: %s pcrp0=0x%08x p0gmii=0x%08x pitcr=0x%08x extpcr0=0x%08x maccr=0x%08x maccr1=0x%08x pad=0x%08x txdelay=%u rxdelay=%u cputag-rx=%u cputag-tx=%u preserve-tag=1 txdp=0x1/dpext=0\n",
		 reason,
		 readl(priv->swcore + RTL_RTK_SWCORE_PCRP(0)),
		 readl(priv->swcore + RTL_RTK_SWCORE_P0GMIICR),
		 readl(priv->swcore + RTL_RTK_SWCORE_PITCR),
		 readl(priv->swcore + RTL_RTK_SWCORE_EXTPCR0),
		 readl(priv->swcore + RTL_RTK_SWCORE_MACCR),
		 readl(priv->swcore + RTL_RTK_SWCORE_MACCR1),
		 priv->system ? readl(priv->system + RTL_RTK_SYSTEM_PAD_CTRL_1) : pad,
		 !!(readl(priv->swcore + RTL_RTK_SWCORE_P0GMIICR) &
		    RTL_RTK_P0GMIICR_TX_DELAY_MASK),
		 (u32)(readl(priv->swcore + RTL_RTK_SWCORE_P0GMIICR) &
		       RTL_RTK_P0GMIICR_RX_DELAY_MASK),
		 !!(readl(priv->swcore + RTL_RTK_SWCORE_P0GMIICR) &
		    RTL_RTK_P0GMIICR_CPU_TAG_RX),
		 !!(readl(priv->swcore + RTL_RTK_SWCORE_P0GMIICR) &
		    RTL_RTK_P0GMIICR_CPU_TAG_TX));
}

static void rtl8197f_rtk_rd05_vendor_sidebands(struct rtl8197f_rtknet_priv *priv,
						 const char *reason)
{
	u32 val;
	int i;

	if (!of_machine_is_compatible("xiaomi,r4-rd05") || !priv->swcore)
		return;

	priv->rd05_vendor_sideband_runs++;

	/* Match rtl8651_clearAsicAllTable(): reset all physical/extension MACs,
	 * then release only the actual RD05 cascade interface.  The GPL SDK selects
	 * RTL8197F P0 for CONFIG_RTL_83XX_SUPPORT; leaving P6/P7/P8 in reset avoids
	 * the v30 CPU/extension self-loop.
	 */
	for (i = 0; i <= 8; i++)
		writel(1, priv->swcore + RTL_RTK_SWCORE_PCRP(i));
	writel(1, priv->swcore + RTL_RTK_SWCORE_PPMAR);
	rtl8197f_rtk_rd05_init_p0_rgmii(priv, reason);

	/* Match SDK QoS/remark/ALE neutral defaults. */
	writel(RTL_RTK_QIDDPCR_SDK_INIT, priv->swcore + RTL_RTK_SWCORE_QIDDPCR);
	writel(0, priv->swcore + RTL_RTK_SWCORE_RMACR);
	writel(0, priv->swcore + RTL_RTK_SWCORE_ALECR);
	writel(0, priv->swcore + RTL_RTK_SWCORE_RMCR1P);
	writel(0, priv->swcore + RTL_RTK_SWCORE_DSCPRM0);
	writel(0, priv->swcore + RTL_RTK_SWCORE_DSCPRM1);
	writel(0, priv->swcore + RTL_RTK_SWCORE_RLRC);

	/* Enable normal switch-core mode and IPv6 extension-header CPU visibility. */
	val = readl(priv->swcore + RTL_RTK_SWCORE_TMRCR);
	val &= ~RTL_RTK_TMRCR_ENHSBTESTMODE;
	writel(val, priv->swcore + RTL_RTK_SWCORE_TMRCR);
	val = readl(priv->swcore + RTL_RTK_SWCORE_IPV6CR1);
	val = (val & ~0x3) | 0x3;
	writel(val, priv->swcore + RTL_RTK_SWCORE_IPV6CR1);

	/* Vendor flow-control/scheduler initialisation.  These are not Linux policy
	 * knobs; they unblock the rtl865x internal ingress/egress FIFOs during bring-up.
	 */
	writel(RTL_RTK_SBFCR0_SDK_INIT, priv->swcore + RTL_RTK_SWCORE_SBFCR0);
	writel(RTL_RTK_SBFCR1_SDK_INIT, priv->swcore + RTL_RTK_SWCORE_SBFCR1);
	writel(RTL_RTK_SBFCR2_SDK_INIT, priv->swcore + RTL_RTK_SWCORE_SBFCR2);
	for (i = 0; i < 6; i++)
		writel(RTL_RTK_PBFCR_SDK_INIT, priv->swcore + RTL_RTK_SWCORE_PBFCR(i));
	writel(RTL_RTK_ELBPCR_SDK_INIT, priv->swcore + RTL_RTK_SWCORE_ELBPCR);
	writel(RTL_RTK_ELBTTCR_SDK_INIT, priv->swcore + RTL_RTK_SWCORE_ELBTTCR);
	writel(RTL_RTK_ILBPCR1_SDK_INIT, priv->swcore + RTL_RTK_SWCORE_ILBPCR1);
	writel(RTL_RTK_ILBPCR2_SDK_INIT, priv->swcore + RTL_RTK_SWCORE_ILBPCR2);
	for (i = 0; i < 42; i++)
		writel(RTL_RTK_P0QRGCR_SDK_INIT, priv->swcore + RTL_RTK_SWCORE_P0Q0RGCR(i));
	for (i = 0; i < 7; i++) {
		writel(RTL_RTK_WFQRCRP0_SDK_INIT, priv->swcore + RTL_RTK_SWCORE_WFQRCRP0(i));
		writel(0, priv->swcore + RTL_RTK_SWCORE_WFQRCRP0(i) + 4);
		writel(0, priv->swcore + RTL_RTK_SWCORE_WFQRCRP0(i) + 8);
	}
	for (i = 0; i < 12; i++)
		writel(0xfe12, priv->swcore + RTL_RTK_SWCORE_PATP(i));
	writel(RTL_RTK_PQPLGR_SDK_INIT, priv->swcore + RTL_RTK_SWCORE_PQPLGR);
	writel(0, priv->swcore + RTL_RTK_SWCORE_QRR);
	writel(0, priv->swcore + RTL_RTK_SWCORE_IGPRIVCR0);
	writel(0, priv->swcore + RTL_RTK_SWCORE_IGPRIVCR1);
	writel(0, priv->swcore + RTL_RTK_SWCORE_IGPRIVCR2);

	if (priv->system) {
		val = readl(priv->system + RTL_RTK_SYSTEM_GIMR);
		val |= RTL_RTK_GIMR_BSP_SW_IE;
		writel(val, priv->system + RTL_RTK_SYSTEM_GIMR);
	}

	dev_info(priv->dev,
		 "rd05 vendor datapath v31: %s runs=%llu pcrp0=0x%08x p0gmii=0x%08x pcrp6=0x%08x pcrp7=0x%08x ppmar=0x%08x tmcr=0x%08x ipv6cr1=0x%08x pqplgr=0x%08x qrr=0x%08x macctrl1=0x%08x sirr=0x%08x gimr=0x%08x\n",
		 reason,
		 (unsigned long long)priv->rd05_vendor_sideband_runs,
		 readl(priv->swcore + RTL_RTK_SWCORE_PCRP(0)),
		 readl(priv->swcore + RTL_RTK_SWCORE_P0GMIICR),
		 readl(priv->swcore + RTL_RTK_SWCORE_PCRP(6)),
		 readl(priv->swcore + RTL_RTK_SWCORE_PCRP(7)),
		 readl(priv->swcore + RTL_RTK_SWCORE_PPMAR),
		 readl(priv->swcore + RTL_RTK_SWCORE_TMRCR),
		 readl(priv->swcore + RTL_RTK_SWCORE_IPV6CR1),
		 readl(priv->swcore + RTL_RTK_SWCORE_PQPLGR),
		 readl(priv->swcore + RTL_RTK_SWCORE_QRR),
		 readl(priv->swcore + RTL_RTK_SWCORE_MACCTRL1),
		 readl(priv->swcore + RTL_RTK_SWCORE_SWMISC_BASE + RTL_RTK_SWMISC_SSIR),
		 priv->system ? readl(priv->system + RTL_RTK_SYSTEM_GIMR) : 0);
}


static int rtl8197f_rtk_rd05_taci_wait(struct rtl8197f_rtknet_priv *priv,
				       const char *what)
{
	u32 val = 0;
	int ret;

	if (!priv->swcore)
		return -ENODEV;

	ret = readl_poll_timeout_atomic(priv->swcore + RTL_RTK_SWTACR, val,
					  !(val & RTL_RTK_TACI_ACTION_MASK),
					  1, 10000);
	if (ret) {
		priv->rd05_l2cpu_last_swtacr = val;
		dev_warn(priv->dev,
			 "rd05 rtl865x l2cpu v5ag: TACI wait timeout at %s swtacr=0x%08x\n",
			 what, val);
	}

	return ret;
}

static u32 rtl8197f_rtk_rd05_l2_w0(const u8 *mac)
{
	return ((u32)mac[3] << 8) | mac[4] |
	       ((((u32)mac[1] << 8) | mac[2]) << 16);
}

static u32 rtl8197f_rtk_rd05_l2_w1(const u8 *mac, u16 member, u8 fid,
					   bool cpu, bool is_static, bool nh,
					   bool auth)
{
	u32 w1;

	w1 = mac[0];
	w1 |= (member & 0x3f) << 8;
	w1 |= ((member >> 6) & 0x7) << 14;
	if (cpu)
		w1 |= BIT(17);
	if (is_static)
		w1 |= BIT(18);
	if (nh)
		w1 |= BIT(22);
	w1 |= (fid & 0x3) << 23;
	if (auth)
		w1 |= BIT(25);

	return w1;
}

static u8 rtl8197f_rtk_rd05_l2_row(const u8 *mac, u8 fid)
{
	static const u8 fid_hash[4] = { 0x00, 0x0f, 0xf0, 0xff };

	return (mac[0] ^ mac[1] ^ mac[2] ^ mac[3] ^ mac[4] ^ mac[5] ^
		fid_hash[fid & 0x3]) & 0xff;
}

static int rtl8197f_rtk_rd05_force_table2(struct rtl8197f_rtknet_priv *priv,
						 u32 type, u32 eidx, u32 w0,
						 u32 w1, const char *name)
{
	u32 status, addr;
	int ret;

	priv->rd05_l2cpu_runs++;
	priv->rd05_l2cpu_last_eidx = eidx;
	priv->rd05_l2cpu_last_w0 = w0;
	priv->rd05_l2cpu_last_w1 = w1;

	ret = rtl8197f_rtk_rd05_taci_wait(priv, "pre");
	if (ret)
		goto fail;

	writel(w0, priv->swcore + RTL_RTK_TCR0);
	writel(w1, priv->swcore + RTL_RTK_TCR1);
	writel(0, priv->swcore + RTL_RTK_TCR2);

	addr = RTL_RTK_ASICTBL_BASE_KSEG1 +
	       (type << 16) + eidx * RTL_RTK_ASICTBL_ENTRY_LEN;
	writel(addr, priv->swcore + RTL_RTK_SWTAA);
	wmb();
	writel(RTL_RTK_TACI_ACTION_START | RTL_RTK_TACI_CMD_FORCE,
	       priv->swcore + RTL_RTK_SWTACR);

	ret = rtl8197f_rtk_rd05_taci_wait(priv, "post");
	status = readl(priv->swcore + RTL_RTK_SWTASR);
	priv->rd05_l2cpu_last_status = status;
	priv->rd05_l2cpu_last_swtacr = readl(priv->swcore + RTL_RTK_SWTACR);
	if (ret || (status & RTL_RTK_TACI_TABSTS_FAIL))
		goto fail;

	priv->rd05_l2cpu_ok++;
	dev_info(priv->dev,
		 "rd05 rtl865x l2cpu v5ag: force %s type=%u eidx=%u w0=0x%08x w1=0x%08x status=0x%08x ok=%llu fail=%llu\n",
		 name, type, eidx, w0, w1, status,
		 (unsigned long long)priv->rd05_l2cpu_ok,
		 (unsigned long long)priv->rd05_l2cpu_fail);
	return 0;

fail:
	priv->rd05_l2cpu_fail++;
	dev_warn(priv->dev,
		 "rd05 rtl865x l2cpu v5ag: force %s type=%u eidx=%u w0=0x%08x w1=0x%08x status=0x%08x swtacr=0x%08llx ret=%d ok=%llu fail=%llu\n",
		 name, type, eidx, w0, w1,
		 (u32)priv->rd05_l2cpu_last_status,
		 (unsigned long long)priv->rd05_l2cpu_last_swtacr, ret,
		 (unsigned long long)priv->rd05_l2cpu_ok,
		 (unsigned long long)priv->rd05_l2cpu_fail);
	return ret ?: -EIO;
}

static int rtl8197f_rtk_rd05_force_l2(struct rtl8197f_rtknet_priv *priv,
					      const u8 *mac, u8 fid, u16 member,
					      bool cpu, bool is_static, bool nh,
					      bool auth, const char *name)
{
	u8 row = rtl8197f_rtk_rd05_l2_row(mac, fid);
	u32 eidx = row << 2;
	u32 w0 = rtl8197f_rtk_rd05_l2_w0(mac);
	u32 w1 = rtl8197f_rtk_rd05_l2_w1(mac, member, fid, cpu,
					      is_static, nh, auth);

	return rtl8197f_rtk_rd05_force_table2(priv, RTL_RTK_ASICTBL_TYPE_L2,
					      eidx, w0, w1, name);
}

static u32 rtl8197f_rtk_rd05_vlan_w0(u16 member, u8 fid)
{
	/* member/untag port mask uses p0..p5 in bits 0..5 and p6..p8 in ext bits. */
	u16 untag = member;
	u32 w0;

	w0 = member & 0x3f;
	w0 |= ((member >> 6) & 0x7) << 6;
	w0 |= (untag & 0x3f) << 9;
	w0 |= ((untag >> 6) & 0x7) << 15;
	w0 |= (fid & 0x3) << 18;

	return w0;
}

static int rtl8197f_rtk_rd05_force_table5(struct rtl8197f_rtknet_priv *priv,
						 u32 type, u32 eidx, const u32 *w,
						 const char *name)
{
	u32 status, addr;
	int ret;

	priv->rd05_l2cpu_runs++;
	priv->rd05_l2cpu_last_eidx = eidx;
	priv->rd05_l2cpu_last_w0 = w[0];
	priv->rd05_l2cpu_last_w1 = w[1];

	ret = rtl8197f_rtk_rd05_taci_wait(priv, "pre5");
	if (ret)
		goto fail;

	writel(w[0], priv->swcore + RTL_RTK_TCR0);
	writel(w[1], priv->swcore + RTL_RTK_TCR1);
	writel(w[2], priv->swcore + RTL_RTK_TCR2);
	writel(w[3], priv->swcore + RTL_RTK_TCR3);
	writel(w[4], priv->swcore + RTL_RTK_TCR4);

	addr = RTL_RTK_ASICTBL_BASE_KSEG1 +
	       (type << 16) + eidx * RTL_RTK_ASICTBL_ENTRY_LEN;
	writel(addr, priv->swcore + RTL_RTK_SWTAA);
	wmb();
	writel(RTL_RTK_TACI_ACTION_START | RTL_RTK_TACI_CMD_FORCE,
	       priv->swcore + RTL_RTK_SWTACR);

	ret = rtl8197f_rtk_rd05_taci_wait(priv, "post5");
	status = readl(priv->swcore + RTL_RTK_SWTASR);
	priv->rd05_l2cpu_last_status = status;
	priv->rd05_l2cpu_last_swtacr = readl(priv->swcore + RTL_RTK_SWTACR);
	if (ret || (status & RTL_RTK_TACI_TABSTS_FAIL))
		goto fail;

	priv->rd05_l2cpu_ok++;
	dev_info(priv->dev,
		 "rd05 rtl865x pipe v5ag: force %s type=%u eidx=%u w0=0x%08x w1=0x%08x w2=0x%08x w3=0x%08x w4=0x%08x status=0x%08x ok=%llu fail=%llu\n",
		 name, type, eidx, w[0], w[1], w[2], w[3], w[4], status,
		 (unsigned long long)priv->rd05_l2cpu_ok,
		 (unsigned long long)priv->rd05_l2cpu_fail);
	return 0;

fail:
	priv->rd05_l2cpu_fail++;
	dev_warn(priv->dev,
		 "rd05 rtl865x pipe v5ag: force %s type=%u eidx=%u status=0x%08x swtacr=0x%08llx ret=%d ok=%llu fail=%llu\n",
		 name, type, eidx, (u32)priv->rd05_l2cpu_last_status,
		 (unsigned long long)priv->rd05_l2cpu_last_swtacr, ret,
		 (unsigned long long)priv->rd05_l2cpu_ok,
		 (unsigned long long)priv->rd05_l2cpu_fail);
	return ret ?: -EIO;
}


static int rtl8197f_rtk_rd05_force_table11(struct rtl8197f_rtknet_priv *priv,
						  u32 type, u32 eidx, const u32 *w,
						  const char *name)
{
	u32 status, addr;
	int ret, i;

	priv->rd05_acl_runs++;
	priv->rd05_acl_last_idx = eidx;
	priv->rd05_acl_last_w7 = w[7];

	ret = rtl8197f_rtk_rd05_taci_wait(priv, "pre11");
	if (ret)
		goto fail;

	for (i = 0; i < 11; i++)
		writel(w[i], priv->swcore + RTL_RTK_TCR0 + i * 4);

	addr = RTL_RTK_ASICTBL_BASE_KSEG1 +
	       (type << 16) + eidx * RTL_RTK_ASICTBL_ENTRY_LEN;
	writel(addr, priv->swcore + RTL_RTK_SWTAA);
	wmb();
	writel(RTL_RTK_TACI_ACTION_START | RTL_RTK_TACI_CMD_FORCE,
	       priv->swcore + RTL_RTK_SWTACR);

	ret = rtl8197f_rtk_rd05_taci_wait(priv, "post11");
	status = readl(priv->swcore + RTL_RTK_SWTASR);
	priv->rd05_acl_last_status = status;
	priv->rd05_l2cpu_last_swtacr = readl(priv->swcore + RTL_RTK_SWTACR);
	if (ret || (status & RTL_RTK_TACI_TABSTS_FAIL))
		goto fail;

	priv->rd05_acl_ok++;
	dev_info(priv->dev,
		 "rd05 rtl865x acl v5ag: force %s type=%u eidx=%u w6=0x%08x w7=0x%08x status=0x%08x ok=%llu fail=%llu\n",
		 name, type, eidx, w[6], w[7], status,
		 (unsigned long long)priv->rd05_acl_ok,
		 (unsigned long long)priv->rd05_acl_fail);
	return 0;

fail:
	priv->rd05_acl_fail++;
	dev_warn(priv->dev,
		 "rd05 rtl865x acl v5ag: force %s type=%u eidx=%u w6=0x%08x w7=0x%08x status=0x%08llx swtacr=0x%08llx ret=%d ok=%llu fail=%llu\n",
		 name, type, eidx, w[6], w[7],
		 (unsigned long long)priv->rd05_acl_last_status,
		 (unsigned long long)priv->rd05_l2cpu_last_swtacr, ret,
		 (unsigned long long)priv->rd05_acl_ok,
		 (unsigned long long)priv->rd05_acl_fail);
	return ret ?: -EIO;
}

static u32 rtl8197f_rtk_rd05_acl_w7(u8 rule_type, u8 action)
{
	return (RTL_RTK_ACL_PKT_OP_ALL << 24) |
	       ((action & 0xf) << 4) |
	       (rule_type & 0xf);
}

static void rtl8197f_rtk_rd05_seed_acl(struct rtl8197f_rtknet_priv *priv)
{
	u32 w[11] = { 0 };

	if (!priv->swcore) {
		priv->rd05_acl_fail++;
		dev_warn(priv->dev,
			 "rd05 rtl865x acl v5ag: no swcore mapping, skip ACL seed\n");
		return;
	}

	dev_info(priv->dev,
		 "rd05 rtl865x acl v5ag: seed OEM ingress ACL 0-4 without ARP trap, egress ACL253 permit\n");

	/* OEM rule 0: source-filter permit all/authorized source path. */
	memset(w, 0, sizeof(w));
	w[1] = 0x10000000; /* spaP/sport 256 in the LE source-filter layout. */
	w[3] = 0x00000040; /* ProtoType: 1, matching OEM /proc/rtl865x/acl dump. */
	w[7] = rtl8197f_rtk_rd05_acl_w7(RTL_RTK_ACL_TYPE_SRCFILTER,
						 RTL_RTK_ACL_ACTION_PERMIT);
	rtl8197f_rtk_rd05_force_table11(priv, RTL_RTK_ASICTBL_TYPE_ACL,
					     0, w, "acl0-oem-srcfilter-permit");

	/* OEM rule 1: DHCPv4 UDP 67..68 -> CPU. */
	memset(w, 0, sizeof(w));
	w[5] = (68 << 16) | 67;
	w[6] = (68 << 16) | 67;
	w[7] = rtl8197f_rtk_rd05_acl_w7(RTL_RTK_ACL_TYPE_UDP,
						 RTL_RTK_ACL_ACTION_TOCPU);
	rtl8197f_rtk_rd05_force_table11(priv, RTL_RTK_ASICTBL_TYPE_ACL,
					     1, w, "acl1-oem-dhcpv4-to-cpu");

	/* OEM rule 2: DHCPv6 UDP 546..547 -> CPU. */
	memset(w, 0, sizeof(w));
	w[5] = (547 << 16) | 546;
	w[6] = (547 << 16) | 546;
	w[7] = rtl8197f_rtk_rd05_acl_w7(RTL_RTK_ACL_TYPE_UDP,
						 RTL_RTK_ACL_ACTION_TOCPU);
	rtl8197f_rtk_rd05_force_table11(priv, RTL_RTK_ASICTBL_TYPE_ACL,
					     2, w, "acl2-oem-dhcpv6-to-cpu");

	/* OEM rule 3: PPPoE discovery 0x8863 -> CPU. */
	memset(w, 0, sizeof(w));
	w[6] = (0xffffu << 16) | ETH_P_PPP_DISC;
	w[7] = rtl8197f_rtk_rd05_acl_w7(RTL_RTK_ACL_TYPE_ETHERNET,
						 RTL_RTK_ACL_ACTION_TOCPU);
	rtl8197f_rtk_rd05_force_table11(priv, RTL_RTK_ASICTBL_TYPE_ACL,
					     3, w, "acl3-oem-pppoe-disc-to-cpu");

	/* OEM rule 4: final ingress permit. */
	memset(w, 0, sizeof(w));
	w[7] = rtl8197f_rtk_rd05_acl_w7(RTL_RTK_ACL_TYPE_ETHERNET,
						 RTL_RTK_ACL_ACTION_PERMIT);
	rtl8197f_rtk_rd05_force_table11(priv, RTL_RTK_ASICTBL_TYPE_ACL,
					     4, w, "acl4-oem-permit-all");

	/* OEM egress ACL 253 permit all. */
	memset(w, 0, sizeof(w));
	w[7] = rtl8197f_rtk_rd05_acl_w7(RTL_RTK_ACL_TYPE_ETHERNET,
						 RTL_RTK_ACL_ACTION_PERMIT);
	rtl8197f_rtk_rd05_force_table11(priv, RTL_RTK_ASICTBL_TYPE_ACL,
					     253, w, "acl253-oem-egress-permit");

	dev_info(priv->dev,
		 "rd05 rtl865x acl v5ag: seed done acl_runs=%llu acl_ok=%llu acl_fail=%llu last_idx=%llu last_w7=0x%llx status=0x%llx\n",
		 (unsigned long long)priv->rd05_acl_runs,
		 (unsigned long long)priv->rd05_acl_ok,
		 (unsigned long long)priv->rd05_acl_fail,
		 (unsigned long long)priv->rd05_acl_last_idx,
		 (unsigned long long)priv->rd05_acl_last_w7,
		 (unsigned long long)priv->rd05_acl_last_status);
}

static bool rtl8197f_rtk_rd05_reg_update(struct rtl8197f_rtknet_priv *priv,
						 u32 reg, u32 mask, u32 set,
						 const char *name)
{
	u32 old, val, rd;

	if (!priv->swcore) {
		priv->rd05_pipe_fail++;
		return false;
	}

	priv->rd05_pipe_runs++;
	old = readl(priv->swcore + reg);
	val = (old & ~mask) | (set & mask);
	writel(val, priv->swcore + reg);
	readl(priv->swcore + reg);
	rd = readl(priv->swcore + reg);

	priv->rd05_pipe_last_reg = reg;
	priv->rd05_pipe_last_val = val;
	priv->rd05_pipe_last_read = rd;

	if ((rd & mask) == (val & mask)) {
		priv->rd05_pipe_ok++;
		dev_info(priv->dev,
			 "rd05 rtl865x pipe v5ag: reg %s off=0x%04x old=0x%08x val=0x%08x read=0x%08x ok=%llu fail=%llu\n",
			 name, reg, old, val, rd,
			 (unsigned long long)priv->rd05_pipe_ok,
			 (unsigned long long)priv->rd05_pipe_fail);
		return true;
	}

	priv->rd05_pipe_fail++;
	dev_warn(priv->dev,
		 "rd05 rtl865x pipe v5ag: reg %s off=0x%04x old=0x%08x val=0x%08x read=0x%08x ok=%llu fail=%llu\n",
		 name, reg, old, val, rd,
		 (unsigned long long)priv->rd05_pipe_ok,
		 (unsigned long long)priv->rd05_pipe_fail);
	return false;
}

static bool rtl8197f_rtk_rd05_set_pvid(struct rtl8197f_rtknet_priv *priv,
					      unsigned int port, u16 pvid)
{
	u32 reg = RTL_RTK_SWCORE_PVCR0 + ((port * 2) & ~0x3);
	u32 mask, set;
	bool ok;

	if (port & 1) {
		mask = 0x0fff0000;
		set = (pvid & 0xfff) << 16;
	} else {
		mask = 0x00000fff;
		set = pvid & 0xfff;
	}

	ok = rtl8197f_rtk_rd05_reg_update(priv, reg, mask, set,
						  "pvid");
	if (ok)
		priv->rd05_pvid_ok++;
	return ok;
}

static u32 rtl8197f_rtk_rd05_netif_w0(const u8 *mac, u16 vid)
{
	u32 mac18_0 = (((u32)mac[3] << 16) | ((u32)mac[4] << 8) | mac[5]) & 0x7ffff;

	return BIT(0) | ((vid & 0xfff) << 1) | (mac18_0 << 13);
}

static u32 rtl8197f_rtk_rd05_netif_w1(const u8 *mac, bool route, bool route6,
					     u16 in_acl_start)
{
	u32 mac47_19 = ((u32)mac[0] << 21) | ((u32)mac[1] << 13) |
			 ((u32)mac[2] << 5) | (mac[3] >> 3);
	u32 w1 = mac47_19;

	if (route)
		w1 |= BIT(29);
	if (route6)
		w1 |= BIT(30);
	if (in_acl_start & 1)
		w1 |= BIT(31);

	return w1;
}

static u32 rtl8197f_rtk_rd05_netif_w2(u16 in_start, u16 in_end,
					     u16 out_start, u16 out_end,
					     u8 mac_count)
{
	u32 mac_mask_l = mac_count <= 1 ? 1 : 0;

	return (((in_start >> 1) & 0x7f) << 0) |
	       ((in_end & 0xff) << 7) |
	       ((out_start & 0xff) << 15) |
	       ((out_end & 0xff) << 23) |
	       (mac_mask_l << 31);
}

static u32 rtl8197f_rtk_rd05_netif_w3(u16 mtu, u16 mtu6, u8 mac_count)
{
	u32 mac_mask_h;

	switch (mac_count) {
	case 0:
	case 1:
		mac_mask_h = 3;
		break;
	case 2:
		mac_mask_h = 3;
		break;
	case 4:
		mac_mask_h = 2;
		break;
	case 8:
		mac_mask_h = 0;
		break;
	default:
		mac_mask_h = 3;
		break;
	}

	return (mac_mask_h & 0x3) |
	       ((mtu & 0x7fff) << 2) |
	       ((mtu6 & 0x7fff) << 17);
}

static bool rtl8197f_rtk_rd05_wait_swcore_ready(struct rtl8197f_rtknet_priv *priv)
{
	u32 pvcr0 = 0;
	int ret;

	if (!priv->swcore)
		return false;

	/* The v36 hardware log showed the first VID9/netif seed racing the
	 * rtl865x/SWCORE clock domain: PVCR0 read as zero and six writes failed,
	 * while the delayed replay succeeded.  The SDK initializes these tables
	 * only after the core exposes its reset defaults (PVCR0=0x00010001 on
	 * RD05).  Wait a bounded 500 ms instead of recording transient failures.
	 */
	ret = readl_poll_timeout(priv->swcore + RTL_RTK_SWCORE_PVCR0, pvcr0,
				 pvcr0 != 0, 1000, 500000);
	if (ret) {
		dev_warn(priv->dev,
			 "rd05 rtl865x v38: SWCORE not ready for table seed, pvcr0=0x%08x; delayed reseed will retry\n",
			 pvcr0);
		return false;
	}

	return true;
}

static void rtl8197f_rtk_rd05_seed_pipeline(struct rtl8197f_rtknet_priv *priv)
{
	static const u8 lan_mac[ETH_ALEN] = { 0xa4, 0xba, 0x70, 0x1b, 0xd4, 0x10 };
	u32 netif[5];

	if (!of_machine_is_compatible("xiaomi,r4-rd05"))
		return;

	if (!priv->swcore) {
		priv->rd05_pipe_fail++;
		dev_warn(priv->dev,
			 "rd05 rtl865x pipe v5ag: no swcore mapping, skip PVID/netif/ACL seed\n");
		return;
	}

	dev_info(priv->dev,
		 "rd05 rtl865x pipe v31: seed P0/RGMII host-link ingress PVID->netif->ACL->CPU lan_vid=%u host_port=%u host_mbr=0x%x\n",
		 RTL_RTK_RD05_LAN_VID, RTL_RTK_RD05_HOST_PORT,
		 (u32)RTL_RTK_RD05_HOST_MBR);

	/* DSA exposes RTL8367D UTP ports above the CPU master.  The SoC-internal
	 * switch must classify only its physical P0 cascade link, not synthetic
	 * extension ports copied from the OEM private driver.
	 */
	rtl8197f_rtk_rd05_set_pvid(priv, RTL_RTK_RD05_HOST_PORT,
				 RTL_RTK_RD05_LAN_VID);

	/* OEM netif decision is VLAN based; port->netif is kept at index 0. */
	rtl8197f_rtk_rd05_reg_update(priv, RTL_RTK_SWCORE_SWTCR0,
					 RTL_RTK_SWTCR0_LIMDBC_MASK |
					 RTL_RTK_SWTCR0_EN_UKVID_TO_CPU,
					 RTL_RTK_SWTCR0_LIMDBC_VLAN |
					 RTL_RTK_SWTCR0_EN_UKVID_TO_CPU,
					 "swtcr0-vlan-netdec");
	rtl8197f_rtk_rd05_reg_update(priv, RTL_RTK_SWCORE_PLITIMR,
					 GENMASK(26, 0), 0,
					 "port-to-netif0");

	/* Keep the host-link datapath minimal: L2 only, with L3/ingress ACL
	 * disabled for the first DSA ARP proof.  The L2 broadcast/unknown-to-CPU
	 * decisions below deliver P0 ingress to the CPU DMA ring.
	 */
	rtl8197f_rtk_rd05_reg_update(priv, RTL_RTK_SWCORE_MSCR,
					 RTL_RTK_MSCR_EN_L2 |
					 RTL_RTK_MSCR_EN_L3 |
					 RTL_RTK_MSCR_EN_IN_ACL,
					 RTL_RTK_MSCR_EN_L2,
					 "mscr-l2-only-no-acl");
	rtl8197f_rtk_rd05_reg_update(priv, RTL_RTK_SWCORE_SWTCR1,
					 RTL_RTK_SWTCR1_SEL_CPU_REASON |
					 RTL_RTK_SWTCR1_EN_NATT2LOG |
					 RTL_RTK_SWTCR1_EN_FRAG_TO_ACLPT,
					 RTL_RTK_SWTCR1_SEL_CPU_REASON |
					 RTL_RTK_SWTCR1_EN_NATT2LOG |
					 RTL_RTK_SWTCR1_EN_FRAG_TO_ACLPT,
					 "swtcr1-cpursn-aclpt");

	/* During DSA bring-up, send unknown L2 multicast and unknown unicast to the
	 * CPU as well.  This is a diagnostic-safe superset and avoids hiding the
	 * first ARP/neighbor proof behind a stale learning entry.
	 */
	rtl8197f_rtk_rd05_reg_update(priv, RTL_RTK_SWCORE_FFCR,
					 RTL_RTK_FFCR_EN_UNK_MCAST_TOCPU |
					 RTL_RTK_FFCR_EN_UNK_UCAST_TOCPU,
					 RTL_RTK_FFCR_EN_UNK_MCAST_TOCPU |
					 RTL_RTK_FFCR_EN_UNK_UCAST_TOCPU,
					 "ffcr-unkucast-unkmcast-tocpu");

	/* Preserve the OEM netif ACL bounds.  ARP reaches the CPU through the FID0
	 * broadcast entry and unknown-to-CPU policy; no synthetic extension member
	 * is required when the physical cascade is correctly configured as P0.
	 */
	rtl8197f_rtk_rd05_reg_update(priv, RTL_RTK_SWCORE_DACLRCR,
					 0xffffffff, RTL_RTK_DACLRCR_ING_0_4_EG_253,
					 "daclrcr-acl0-4-eg253");

	priv->rd05_ffcr_read = readl(priv->swcore + RTL_RTK_SWCORE_FFCR);
	priv->rd05_daclrcr_read = readl(priv->swcore + RTL_RTK_SWCORE_DACLRCR);
	priv->rd05_ptrapcr_read = readl(priv->swcore + RTL_RTK_SWCORE_PTRAPCR);
	dev_info(priv->dev,
		 "rd05 rtl865x pipe v5ag: readback ffcr=0x%08llx daclrcr=0x%08llx ptrapcr=0x%08llx trap_ok=%llu\n",
		 (unsigned long long)priv->rd05_ffcr_read,
		 (unsigned long long)priv->rd05_daclrcr_read,
		 (unsigned long long)priv->rd05_ptrapcr_read,
		 (unsigned long long)priv->rd05_trap_ok);

	/* OEM netif0: ingress ACL 0..4, egress ACL 253..253. */
	netif[0] = rtl8197f_rtk_rd05_netif_w0(lan_mac, RTL_RTK_RD05_LAN_VID);
	netif[1] = rtl8197f_rtk_rd05_netif_w1(lan_mac, true, false, 0);
	netif[2] = rtl8197f_rtk_rd05_netif_w2(0, 4, 253, 253, 1);
	netif[3] = rtl8197f_rtk_rd05_netif_w3(1500, 1500, 1);
	netif[4] = 0;
	if (!rtl8197f_rtk_rd05_force_table5(priv, RTL_RTK_ASICTBL_TYPE_NETIF,
						   0, netif, "netif0-vid9-lanmac-acl0-4"))
		priv->rd05_netif_ok++;

	rtl8197f_rtk_rd05_seed_acl(priv);

	dev_info(priv->dev,
		 "rd05 rtl865x pipe v31: seed done pipe_runs=%llu pipe_ok=%llu pipe_fail=%llu pvid_ok=%llu netif_ok=%llu acl_ok=%llu acl_fail=%llu ffcr=0x%llx\n",
		 (unsigned long long)priv->rd05_pipe_runs,
		 (unsigned long long)priv->rd05_pipe_ok,
		 (unsigned long long)priv->rd05_pipe_fail,
		 (unsigned long long)priv->rd05_pvid_ok,
		 (unsigned long long)priv->rd05_netif_ok,
		 (unsigned long long)priv->rd05_acl_ok,
		 (unsigned long long)priv->rd05_acl_fail,
		 (unsigned long long)priv->rd05_ffcr_read);
}

static void rtl8197f_rtk_rd05_seed_l2cpu(struct rtl8197f_rtknet_priv *priv)
{
	static const u8 bcast[ETH_ALEN] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
	static const u8 cpu_mac[ETH_ALEN] = { 0x00, 0x00, 0x0a, 0x00, 0x00, 0x0f };
	u32 vlan9 = rtl8197f_rtk_rd05_vlan_w0(RTL_RTK_RD05_HOST_MBR, 0);
	u32 vlan8 = rtl8197f_rtk_rd05_vlan_w0(RTL_RTK_RD05_HOST_MBR, 0);

	if (!of_machine_is_compatible("xiaomi,r4-rd05"))
		return;

	if (!priv->swcore) {
		priv->rd05_l2cpu_fail++;
		dev_warn(priv->dev,
			 "rd05 rtl865x l2cpu v5ag: no swcore mapping, skip OEM L2/VID seed\n");
		return;
	}

	dev_info(priv->dev,
		 "rd05 rtl865x l2cpu v31: seed VID9/L2 CPU entries on physical P0 host link mbr=0x%x\n",
		 (u32)RTL_RTK_RD05_HOST_MBR);

	/* Both retained bootstrap VIDs point only at P0.  Linux DSA, not the
	 * rtl865x extension-port namespace, separates RTL8367D user ports.
	 */
	rtl8197f_rtk_rd05_force_table2(priv, RTL_RTK_ASICTBL_TYPE_VLAN,
					  9, vlan9, 0, "vlan-vid9-fid0");
	rtl8197f_rtk_rd05_force_table2(priv, RTL_RTK_ASICTBL_TYPE_VLAN,
					  8, vlan8, 0, "vlan-vid8-fid1");

	/* OEM L2 seed: broadcast + CPU/NH/STA markers in FID0 and FID1. */
	rtl8197f_rtk_rd05_force_l2(priv, bcast, 0, RTL_RTK_RD05_HOST_MBR,
				       true, true, true, true, "l2-bcast-fid0");
	rtl8197f_rtk_rd05_force_l2(priv, cpu_mac, 0, 0,
				       true, true, true, true, "l2-cpumac-fid0");
	rtl8197f_rtk_rd05_force_l2(priv, cpu_mac, 1, 0,
				       true, true, true, true, "l2-cpumac-fid1");
	rtl8197f_rtk_rd05_force_l2(priv, bcast, 1, RTL_RTK_RD05_HOST_MBR,
				       true, true, true, true, "l2-bcast-fid1");

	dev_info(priv->dev,
		 "rd05 rtl865x l2cpu v31: seed done runs=%llu ok=%llu fail=%llu last_status=0x%llx\n",
		 (unsigned long long)priv->rd05_l2cpu_runs,
		 (unsigned long long)priv->rd05_l2cpu_ok,
		 (unsigned long long)priv->rd05_l2cpu_fail,
		 (unsigned long long)priv->rd05_l2cpu_last_status);
}

static void rtl8197f_rtk_apply_sdk_cpuif_init(struct rtl8197f_rtknet_priv *priv)
{
	u32 val;

	if (!priv->sdk_cpuicr1_init)
		return;

	/*
	 * The RTL8197F SDK does more than set descriptor base registers: for this
	 * rtl865x CPU interface it enables the split TX/RX bus path, little-endian
	 * NIC master mode, TSO ID selection and TX gather support before asserting
	 * TRXRDY.  Without these bits the ring can poll forever while descriptors are
	 * never consumed by the switch-core DMA engine.
	 */
	val = rtl8197f_rtk_read(priv, RTL_RTK_CPUICR1);
	val &= ~(RTL_RTK_CPUICR1_PKT_HDR_TYPE_MASK |
		 RTL_RTK_CPUICR1_TXDESC_MASK |
		 RTL_RTK_CPUICR1_RXDESC_MASK);
	/*
	 * RTL8197F SDK builds with CONFIG_RTL_SWITCH_NEW_DESCRIPTOR, which redirects
	 * the swNic hooks to New_swNic_* and programs 8-dword descriptor rings instead
	 * of rtl_pktHdr/rtl_mBuf pointer rings.  Keep CPUICR1 in that shortcut+LSO
	 * descriptor mode and keep CPURPDCR0/CPUTPDCR0 consistent with it; mixing this
	 * bit with legacy rxmbuf rings was the v11-v13 mismatch that left RX OWN idle.
	 */
	val |= RTL_RTK_CPUICR1_TXRX_DIV_LX | RTL_RTK_CPUICR1_LE |
	       RTL_RTK_CPUICR1_TSO_ID_SEL | RTL_RTK_CPUICR1_TX_GATHER |
	       RTL_RTK_CPUICR1_PKT_HDR_SHORTCUT_LSO |
	       ((sizeof(struct rtl8197f_rtk_desc) / sizeof(u32)) <<
		RTL_RTK_CPUICR1_TXDESC_OFFSET) |
	       ((sizeof(struct rtl8197f_rtk_desc) / sizeof(u32)) <<
		RTL_RTK_CPUICR1_RXDESC_OFFSET);
	rtl8197f_rtk_write(priv, RTL_RTK_CPUICR1, val);
	priv->cpuicr1_sdk_writes++;

	/* Queue IDs and external-port RX descriptor mappings all go to RX ring 0 in
	 * this single-ring native driver.  RTL8197F exposes separate CPU/EXT1/EXT2/
	 * EXT3 fields in CPUQDM0; write them explicitly and log readback because the
	 * current failure mode is: RTL8367D port7 TX grows, but CPUIF RX ring0 never
	 * advances.
	 */
	rtl8197f_rtk_write(priv, RTL_RTK_CPUQDM0, RTL_RTK_CPUQDM_ALL_TO_RING0);
	rtl8197f_rtk_write(priv, RTL_RTK_CPUQDM2, RTL_RTK_CPUQDM_ALL_TO_RING0);
	rtl8197f_rtk_write(priv, RTL_RTK_CPUQDM4, RTL_RTK_CPUQDM_ALL_TO_RING0);
	dev_info(priv->dev,
		 "rd05 cpuidma v31: cpuqdm0=0x%08x cpuqdm2=0x%08x cpuqdm4=0x%08x ext1->newdesc-rxring0\n",
		 rtl8197f_rtk_read(priv, RTL_RTK_CPUQDM0),
		 rtl8197f_rtk_read(priv, RTL_RTK_CPUQDM2),
		 rtl8197f_rtk_read(priv, RTL_RTK_CPUQDM4));
}

static void rtl8197f_rtk_rd05_program_newdesc_rx(struct rtl8197f_rtknet_priv *priv,
						   const char *reason);
static void rtl8197f_rtk_rd05_program_legacy_rx(struct rtl8197f_rtknet_priv *priv,
						  const char *reason);

static u32 rtl8197f_rtk_expected_cpuicr1(struct rtl8197f_rtknet_priv *priv)
{
	u32 val = rtl8197f_rtk_read(priv, RTL_RTK_CPUICR1);

	val &= ~(RTL_RTK_CPUICR1_PKT_HDR_TYPE_MASK |
		 RTL_RTK_CPUICR1_TXDESC_MASK | RTL_RTK_CPUICR1_RXDESC_MASK);
	val |= RTL_RTK_CPUICR1_TXRX_DIV_LX | RTL_RTK_CPUICR1_LE |
	       RTL_RTK_CPUICR1_TSO_ID_SEL | RTL_RTK_CPUICR1_TX_GATHER |
	       RTL_RTK_CPUICR1_PKT_HDR_SHORTCUT_LSO |
	       ((sizeof(struct rtl8197f_rtk_desc) / sizeof(u32)) <<
		RTL_RTK_CPUICR1_TXDESC_OFFSET) |
	       ((sizeof(struct rtl8197f_rtk_desc) / sizeof(u32)) <<
		RTL_RTK_CPUICR1_RXDESC_OFFSET);
	return val;
}

static void rtl8197f_rtk_rd05_poststart_rearm(struct rtl8197f_rtknet_priv *priv,
					       const char *reason)
{
	u32 expected, actual, tx_base;

	if (!of_machine_is_compatible("xiaomi,r4-rd05"))
		return;

	/*
	 * The RD05 hardware trace proved that writes made while CPUICR is stopped
	 * are discarded: CPURPDCR0 read back as zero and CPUICR1 retained the
	 * bootloader's 6-dword stride (0x00186180).  Reapply the SDK 8-dword
	 * format only after TXCMD/RXCMD and TRXRDY are live, then reset both ring
	 * current pointers.
	 */
	expected = rtl8197f_rtk_expected_cpuicr1(priv);
	rtl8197f_rtk_write(priv, RTL_RTK_CPUICR1, expected);
	readl(priv->base + RTL_RTK_CPUICR1);

	if (priv->rd05_sdk_newdesc_rx)
		rtl8197f_rtk_rd05_program_newdesc_rx(priv, reason);
	else if (priv->rd05_legacy_rx_enabled)
		rtl8197f_rtk_rd05_program_legacy_rx(priv, reason);

	tx_base = rtl8197f_rtk_hw_dma_addr(priv, priv->tx_desc_dma);
	rtl8197f_rtk_write(priv, RTL_RTK_CPUTPDCR0, tx_base);
	wmb();

	rtl8197f_rtk_kick_rx(priv);
	rtl8197f_rtk_assert_trxrdy(priv);

	actual = rtl8197f_rtk_read(priv, RTL_RTK_CPUICR1);
	if ((actual & (RTL_RTK_CPUICR1_TXDESC_MASK |
		       RTL_RTK_CPUICR1_RXDESC_MASK)) !=
	    (expected & (RTL_RTK_CPUICR1_TXDESC_MASK |
		         RTL_RTK_CPUICR1_RXDESC_MASK)))
		dev_warn(priv->dev,
			 "rd05 poststart v38.2: CPUICR1 stride write rejected: expected=0x%08x actual=0x%08x\n",
			 expected, actual);
	else
		dev_info(priv->dev,
			 "rd05 poststart v38.2: %s cpuicr1=0x%08x rxbase=0x%08x txbase=0x%08x rxdesc0=0x%08x\n",
			 reason, actual,
			 rtl8197f_rtk_read(priv, RTL_RTK_CPURPDCR0),
			 rtl8197f_rtk_read(priv, RTL_RTK_CPUTPDCR0),
			 priv->rx_desc ? le32_to_cpu(priv->rx_desc[0].opts1) : 0);
}

static int rtl8197f_rtk_alloc_rx_buf(struct rtl8197f_rtknet_priv *priv,
					     unsigned int idx)
{
	struct net_device *ndev = priv->ndev;
	struct sk_buff *skb;
	dma_addr_t dma;

	skb = netdev_alloc_skb_ip_align(ndev, priv->rx_buf_size);
	if (!skb)
		return -ENOMEM;

	dma = dma_map_single(priv->dev, skb->data, priv->rx_buf_size,
			     DMA_FROM_DEVICE);
	if (dma_mapping_error(priv->dev, dma)) {
		dev_kfree_skb_any(skb);
		return -ENOMEM;
	}

	priv->rx_skb[idx] = skb;
	priv->rx_dma[idx] = dma;
	priv->rx_desc[idx].addr = cpu_to_le32(rtl8197f_rtk_hw_dma_addr(priv, dma));
	priv->rx_desc[idx].opts2 = 0;
	priv->rx_desc[idx].opts3 = 0;
	priv->rx_desc[idx].opts4 = 0;
	priv->rx_desc[idx].opts5 = 0;
	priv->rx_desc[idx].opts6 = 0;
	priv->rx_desc[idx].opts7 = 0;
	priv->rx_desc[idx].opts1 = cpu_to_le32(RTL_RTK_DESC_OWN |
						(priv->rx_buf_size << RTL_RTK_RX_EXTSIZE_SHIFT) |
						(idx == priv->rx_ring_size - 1 ?
						 RTL_RTK_DESC_WRAP : 0));
	return 0;
}


static void rtl8197f_rtk_rd05_legacy_rx_init_slot(struct rtl8197f_rtknet_priv *priv,
							 unsigned int idx)
{
	u32 ph = rtl8197f_rtk_rd05_legacy_dma_addr(
			priv->rd05_rx_ph_dma + idx * sizeof(*priv->rd05_rx_ph));
	u32 mb = rtl8197f_rtk_rd05_legacy_dma_addr(
			priv->rd05_rx_mbuf_dma + idx * sizeof(*priv->rd05_rx_mbuf));
	u32 data = rtl8197f_rtk_rd05_legacy_dma_addr(priv->rx_dma[idx]);
	u32 wrap = idx == priv->rx_ring_size - 1 ? RTL_RTK_DESC_WRAP : 0;

	memset(&priv->rd05_rx_ph[idx], 0, sizeof(priv->rd05_rx_ph[idx]));
	memset(&priv->rd05_rx_mbuf[idx], 0, sizeof(priv->rd05_rx_mbuf[idx]));

	/* rtl_pktHdr, little-endian layout: ph_mbuf, misc/len, reason/type,
	 * portlist/orgtos/ph_flags, vlan/flags2, 8197F extras, padding.
	 */
	priv->rd05_rx_ph[idx].ph_mbuf = cpu_to_le32(mb);
	priv->rd05_rx_ph[idx].ph_word1 = 0;
	priv->rd05_rx_ph[idx].ph_word2 = 0;
	priv->rd05_rx_ph[idx].ph_word3 = cpu_to_le32(RTL_RTK_RD05_PKTHDR_USED_INCOMING << 16);

	/* rtl_mBuf, little-endian layout: next, pkthdr, rsv/flags/len, data,
	 * extbuf, rsv/extsize, skb-private, padding.
	 */
	priv->rd05_rx_mbuf[idx].m_pkthdr = cpu_to_le32(ph);
	priv->rd05_rx_mbuf[idx].m_word2 = cpu_to_le32(RTL_RTK_RD05_MBUF_FLAGS_RX << 8);
	priv->rd05_rx_mbuf[idx].m_data = cpu_to_le32(data);
	priv->rd05_rx_mbuf[idx].m_extbuf = cpu_to_le32(data);
	priv->rd05_rx_mbuf[idx].m_word5 = cpu_to_le32(priv->rx_buf_size << 16);

	/* Ring entries are pointers with OWN/WRAP bits, exactly as the SDK does. */
	priv->rd05_rx_ph_ring[idx] = cpu_to_le32(ph | RTL_RTK_DESC_OWN | wrap);
	priv->rd05_rx_mbuf_ring[idx] = cpu_to_le32(mb | RTL_RTK_DESC_OWN | wrap);
}

static int rtl8197f_rtk_rd05_alloc_legacy_rx(struct rtl8197f_rtknet_priv *priv)
{
	unsigned int i;
	size_t ring_sz, ph_sz, mb_sz;

	if (!of_machine_is_compatible("xiaomi,r4-rd05"))
		return 0;

	ring_sz = priv->rx_ring_size * sizeof(*priv->rd05_rx_ph_ring);
	ph_sz = priv->rx_ring_size * sizeof(*priv->rd05_rx_ph);
	mb_sz = priv->rx_ring_size * sizeof(*priv->rd05_rx_mbuf);

	priv->rd05_rx_ph_ring = dma_alloc_coherent(priv->dev, ring_sz,
			&priv->rd05_rx_ph_ring_dma, GFP_KERNEL);
	priv->rd05_rx_mbuf_ring = dma_alloc_coherent(priv->dev, ring_sz,
			&priv->rd05_rx_mbuf_ring_dma, GFP_KERNEL);
	priv->rd05_rx_ph = dma_alloc_coherent(priv->dev, ph_sz,
			&priv->rd05_rx_ph_dma, GFP_KERNEL);
	priv->rd05_rx_mbuf = dma_alloc_coherent(priv->dev, mb_sz,
			&priv->rd05_rx_mbuf_dma, GFP_KERNEL);
	if (!priv->rd05_rx_ph_ring || !priv->rd05_rx_mbuf_ring ||
	    !priv->rd05_rx_ph || !priv->rd05_rx_mbuf)
		return -ENOMEM;

	for (i = 0; i < priv->rx_ring_size; i++)
		rtl8197f_rtk_rd05_legacy_rx_init_slot(priv, i);

	priv->rd05_legacy_rx_enabled = true;
	priv->rd05_legacy_rx_tail = 0;
	priv->rd05_legacy_rx_init_ok++;
	priv->rd05_legacy_rx_ring_base = rtl8197f_rtk_rd05_legacy_dma_addr(
			priv->rd05_rx_ph_ring_dma);
	priv->rd05_legacy_mbuf_ring_base = rtl8197f_rtk_rd05_legacy_dma_addr(
			priv->rd05_rx_mbuf_ring_dma);

	dev_info(priv->dev,
		 "rd05 legacy rxmbuf v13: init ring=%u ph_ring=0x%08llx mbuf_ring=0x%08llx ph0=0x%08x mbuf0=0x%08x data0=0x%08x\n",
		 priv->rx_ring_size,
		 (unsigned long long)priv->rd05_legacy_rx_ring_base,
		 (unsigned long long)priv->rd05_legacy_mbuf_ring_base,
		 (u32)(le32_to_cpu(priv->rd05_rx_ph_ring[0]) &
		       ~(RTL_RTK_DESC_OWN | RTL_RTK_DESC_WRAP)),
		 (u32)(le32_to_cpu(priv->rd05_rx_mbuf_ring[0]) &
		       ~(RTL_RTK_DESC_OWN | RTL_RTK_DESC_WRAP)),
		 (u32)le32_to_cpu(priv->rd05_rx_mbuf[0].m_data));

	return 0;
}

static void rtl8197f_rtk_rd05_free_legacy_rx(struct rtl8197f_rtknet_priv *priv)
{
	size_t ring_sz = priv->rx_ring_size * sizeof(*priv->rd05_rx_ph_ring);
	size_t ph_sz = priv->rx_ring_size * sizeof(*priv->rd05_rx_ph);
	size_t mb_sz = priv->rx_ring_size * sizeof(*priv->rd05_rx_mbuf);

	if (priv->rd05_rx_ph_ring)
		dma_free_coherent(priv->dev, ring_sz, priv->rd05_rx_ph_ring,
				  priv->rd05_rx_ph_ring_dma);
	if (priv->rd05_rx_mbuf_ring)
		dma_free_coherent(priv->dev, ring_sz, priv->rd05_rx_mbuf_ring,
				  priv->rd05_rx_mbuf_ring_dma);
	if (priv->rd05_rx_ph)
		dma_free_coherent(priv->dev, ph_sz, priv->rd05_rx_ph,
				  priv->rd05_rx_ph_dma);
	if (priv->rd05_rx_mbuf)
		dma_free_coherent(priv->dev, mb_sz, priv->rd05_rx_mbuf,
				  priv->rd05_rx_mbuf_dma);

	priv->rd05_rx_ph_ring = NULL;
	priv->rd05_rx_mbuf_ring = NULL;
	priv->rd05_rx_ph = NULL;
	priv->rd05_rx_mbuf = NULL;
	priv->rd05_legacy_rx_enabled = false;
}

static void rtl8197f_rtk_rd05_program_legacy_rx(struct rtl8197f_rtknet_priv *priv,
						       const char *reason)
{
	if (!priv->rd05_legacy_rx_enabled)
		return;

	/* SDK RX programming: CPURPDCR0 is the packet-header pointer ring and
	 * CPURMDCR0 is the mbuf pointer ring.  CPURPDCR1..5 are intentionally zero
	 * because CPUQDM maps all CPU/EXT queues to ring0 in this driver.
	 */
	rtl8197f_rtk_write(priv, RTL_RTK_CPURPDCR0, priv->rd05_legacy_rx_ring_base);
	rtl8197f_rtk_write(priv, RTL_RTK_CPURMDCR0, priv->rd05_legacy_mbuf_ring_base);
	rtl8197f_rtk_write(priv, RTL_RTK_CPURPDCR0 + 4, 0);
	rtl8197f_rtk_write(priv, RTL_RTK_CPURPDCR0 + 8, 0);
	rtl8197f_rtk_write(priv, RTL_RTK_CPURPDCR0 + 12, 0);
	rtl8197f_rtk_write(priv, RTL_RTK_CPURPDCR0 + 16, 0);
	rtl8197f_rtk_write(priv, RTL_RTK_CPURPDCR0 + 20, 0);
	wmb();

	dev_info(priv->dev,
		 "rd05 legacy rxmbuf v13: program %s cpurpdcr0=0x%08x cpurmdcr0=0x%08x ring0=0x%08llx mbufring=0x%08llx ent0=0x%08x mbufent0=0x%08x\n",
		 reason,
		 rtl8197f_rtk_read(priv, RTL_RTK_CPURPDCR0),
		 rtl8197f_rtk_read(priv, RTL_RTK_CPURMDCR0),
		 (unsigned long long)priv->rd05_legacy_rx_ring_base,
		 (unsigned long long)priv->rd05_legacy_mbuf_ring_base,
		 le32_to_cpu(priv->rd05_rx_ph_ring[0]),
		 le32_to_cpu(priv->rd05_rx_mbuf_ring[0]));
}

static void rtl8197f_rtk_rd05_program_newdesc_rx(struct rtl8197f_rtknet_priv *priv,
                                                   const char *reason)
{
	u32 rx_base = rtl8197f_rtk_hw_dma_addr(priv, priv->rx_desc_dma);

	/* RTL8197F SDK CONFIG_RTL_SWITCH_NEW_DESCRIPTOR path: CPURPDCR0 is the
	 * native 8-dword RX descriptor ring.  CPURMDCR0 is only meaningful for the
	 * older pktHdr/mbuf path, so keep it zero in v17 to avoid mixed modes.
	 */
	rtl8197f_rtk_write(priv, RTL_RTK_CPURPDCR0, rx_base);
	rtl8197f_rtk_write(priv, RTL_RTK_CPURMDCR0, 0);
	rtl8197f_rtk_write(priv, RTL_RTK_CPURPDCR0 + 4, 0);
	rtl8197f_rtk_write(priv, RTL_RTK_CPURPDCR0 + 8, 0);
	rtl8197f_rtk_write(priv, RTL_RTK_CPURPDCR0 + 12, 0);
	rtl8197f_rtk_write(priv, RTL_RTK_CPURPDCR0 + 16, 0);
	rtl8197f_rtk_write(priv, RTL_RTK_CPURPDCR0 + 20, 0);
	wmb();

	dev_info(priv->dev,
		 "rd05 sdk newdesc v31: program %s cpurpdcr0=0x%08x cpurmdcr0=0x%08x rx_desc=0x%08x rx_desc0=0x%08x/%08x/%08x tx_desc=0x%08x\n",
		 reason,
		 rtl8197f_rtk_read(priv, RTL_RTK_CPURPDCR0),
		 rtl8197f_rtk_read(priv, RTL_RTK_CPURMDCR0),
		 rx_base,
		 priv->rx_desc ? le32_to_cpu(priv->rx_desc[0].opts1) : 0,
		 priv->rx_desc ? le32_to_cpu(priv->rx_desc[0].opts2) : 0,
		 priv->rx_desc ? le32_to_cpu(priv->rx_desc[0].opts3) : 0,
		 rtl8197f_rtk_hw_dma_addr(priv, priv->tx_desc_dma));
}

static void rtl8197f_rtk_free_rings(struct rtl8197f_rtknet_priv *priv)
{
	unsigned int i;

	rtl8197f_rtk_rd05_free_legacy_rx(priv);

	if (priv->rx_skb) {
		for (i = 0; i < priv->rx_ring_size; i++) {
			if (!priv->rx_skb[i])
				continue;
			dma_unmap_single(priv->dev, priv->rx_dma[i],
					 priv->rx_buf_size, DMA_FROM_DEVICE);
			dev_kfree_skb_any(priv->rx_skb[i]);
		}
	}

	if (priv->tx_skb) {
		for (i = 0; i < priv->tx_ring_size; i++) {
			if (!priv->tx_skb[i])
				continue;
			dma_unmap_single(priv->dev, priv->tx_dma[i],
					 priv->tx_len[i], DMA_TO_DEVICE);
			dev_kfree_skb_any(priv->tx_skb[i]);
		}
	}

	kfree(priv->rx_skb);
	kfree(priv->rx_dma);
	kfree(priv->tx_skb);
	kfree(priv->tx_dma);
	kfree(priv->tx_len);
	priv->rx_skb = NULL;
	priv->rx_dma = NULL;
	priv->tx_skb = NULL;
	priv->tx_dma = NULL;
	priv->tx_len = NULL;

	if (priv->rx_desc) {
		dma_free_coherent(priv->dev,
				  priv->rx_ring_size * sizeof(*priv->rx_desc),
				  priv->rx_desc, priv->rx_desc_dma);
		priv->rx_desc = NULL;
	}

	if (priv->tx_desc) {
		dma_free_coherent(priv->dev,
				  priv->tx_ring_size * sizeof(*priv->tx_desc),
				  priv->tx_desc, priv->tx_desc_dma);
		priv->tx_desc = NULL;
	}
}

static int rtl8197f_rtk_alloc_rings(struct rtl8197f_rtknet_priv *priv)
{
	unsigned int i;
	int ret;

	priv->rx_tail = 0;
	priv->tx_head = 0;
	priv->tx_tail = 0;

	priv->rx_desc = dma_alloc_coherent(priv->dev,
				priv->rx_ring_size * sizeof(*priv->rx_desc),
				&priv->rx_desc_dma, GFP_KERNEL);
	if (!priv->rx_desc)
		return -ENOMEM;

	priv->tx_desc = dma_alloc_coherent(priv->dev,
				priv->tx_ring_size * sizeof(*priv->tx_desc),
				&priv->tx_desc_dma, GFP_KERNEL);
	if (!priv->tx_desc) {
		ret = -ENOMEM;
		goto err_free;
	}

	priv->rx_skb = kcalloc(priv->rx_ring_size, sizeof(*priv->rx_skb), GFP_KERNEL);
	priv->rx_dma = kcalloc(priv->rx_ring_size, sizeof(*priv->rx_dma), GFP_KERNEL);
	priv->tx_skb = kcalloc(priv->tx_ring_size, sizeof(*priv->tx_skb), GFP_KERNEL);
	priv->tx_dma = kcalloc(priv->tx_ring_size, sizeof(*priv->tx_dma), GFP_KERNEL);
	priv->tx_len = kcalloc(priv->tx_ring_size, sizeof(*priv->tx_len), GFP_KERNEL);
	if (!priv->rx_skb || !priv->rx_dma || !priv->tx_skb ||
	    !priv->tx_dma || !priv->tx_len) {
		ret = -ENOMEM;
		goto err_free;
	}

	for (i = 0; i < priv->rx_ring_size; i++) {
		ret = rtl8197f_rtk_alloc_rx_buf(priv, i);
		if (ret)
			goto err_free;
	}

	if (!priv->rd05_sdk_newdesc_rx) {
		ret = rtl8197f_rtk_rd05_alloc_legacy_rx(priv);
		if (ret)
			goto err_free;
	}

	for (i = 0; i < priv->tx_ring_size; i++)
		priv->tx_desc[i].opts1 = cpu_to_le32(i == priv->tx_ring_size - 1 ?
							RTL_RTK_DESC_WRAP : 0);

	return 0;

err_free:
	rtl8197f_rtk_free_rings(priv);
	return ret;
}

static void rtl8197f_rtk_hw_stop(struct rtl8197f_rtknet_priv *priv)
{
	u32 val;

	rtl8197f_rtk_disable_irq(priv);
	val = rtl8197f_rtk_read(priv, RTL_RTK_CPUICR);
	val &= ~(RTL_RTK_CPUICR_TXCMD | RTL_RTK_CPUICR_RXCMD);
	val |= RTL_RTK_CPUICR_STOPTX;
	rtl8197f_rtk_write(priv, RTL_RTK_CPUICR, val);
	rtl8197f_rtk_write(priv, RTL_RTK_CPUIISR, 0xffffffff);
}

static void rtl8197f_rtk_hw_start(struct rtl8197f_rtknet_priv *priv)
{
	u32 val;

	rtl8197f_rtk_hw_stop(priv);
	rtl8197f_rtk_deassert_trxrdy(priv);

	/* Descriptor base registers.  On RTL8197F the SDK selects
	 * CONFIG_RTL_SWITCH_NEW_DESCRIPTOR, so RD05 v22b uses the native 8-dword RX
	 * descriptor ring instead of the legacy pktHdr/mbuf pointer rings.
	 */
	if (priv->rd05_sdk_newdesc_rx)
		rtl8197f_rtk_rd05_program_newdesc_rx(priv, "hw-start");
	else if (priv->rd05_legacy_rx_enabled)
		rtl8197f_rtk_rd05_program_legacy_rx(priv, "hw-start");
	else {
		rtl8197f_rtk_write(priv, RTL_RTK_CPURPDCR0,
				rtl8197f_rtk_hw_dma_addr(priv, priv->rx_desc_dma));
		rtl8197f_rtk_write(priv, RTL_RTK_CPURMDCR0, 0);
	}
	rtl8197f_rtk_write(priv, RTL_RTK_CPUTPDCR0,
			    rtl8197f_rtk_hw_dma_addr(priv, priv->tx_desc_dma));

	/* Vendor formula: (ring_entries - 1) * sizeof(struct dma_tx_desc). */
	rtl8197f_rtk_write(priv, RTL_RTK_DMA_CR1,
			    (priv->tx_ring_size - 1) * sizeof(*priv->tx_desc));
	rtl8197f_rtk_write(priv, RTL_RTK_DMA_CR4,
			    RTL_RTK_DMA_CR4_TX_RING0_TAIL_AWARE);
	/* RTL8197F-VG SDK enables ring0/ring1 and the TX-DCP backpressure/round-robin
	 * settings before traffic starts.  Without TX_RING0_EN the ring can accept
	 * software descriptors but the hardware never consumes them, producing
	 * NETDEV WATCHDOG with eth0 TX/RX counters stuck at zero.
	 */
	if (priv->sdk_swcore_init) {
		rtl8197f_rtk_write(priv, RTL_RTK_TXRINGCR,
				    RTL_RTK_TXRINGCR_8197F_VG_INIT);
		priv->txringcr_sdk_writes++;
	}

	rtl8197f_rtk_apply_sdk_cpuif_init(priv);
	rtl8197f_rtk_apply_sdk_swcore_init(priv);
	rtl8197f_rtk_apply_sdk_start_sidebands(priv);
	rtl8197f_rtk_rd05_vendor_sidebands(priv, "hw-start-pre-tables");
	/* DSA owns the external RTL8367D forwarding database, VLANs and CPU
	 * tagging. RD05 additionally requires the SoC-internal rtl865x/SWCORE
	 * ingress tables to accept frames arriving from the RTL8367D EXT1 CPU
	 * link and punt VID9/L2/broadcast traffic into the CPU DMA ring. This is
	 * a Linux-facing translation of the GPL SDK init sequence, not a private
	 * userspace ABI.
	 */
	if (priv->legacy_rd05_pipeline &&
	    rtl8197f_rtk_rd05_wait_swcore_ready(priv)) {
		rtl8197f_rtk_rd05_seed_l2cpu(priv);
		rtl8197f_rtk_rd05_seed_pipeline(priv);
	}

	/* Do not clear/release port PCRs again after table programming.  v30 did
	 * that twice and re-opened the wrong extension candidates after seeding.
	 */
	rtl8197f_rtk_write(priv, RTL_RTK_CPUIISR, 0xffffffff);
	rtl8197f_rtk_enable_irq(priv);

	val = RTL_RTK_CPUICR_BUSBURST_128 | RTL_RTK_CPUICR_MBUF_2048 |
	      RTL_RTK_CPUICR_TXCMD | RTL_RTK_CPUICR_RXCMD;
	if (!priv->sdk_crc_lengths)
		val |= RTL_RTK_CPUICR_EXCLUDE_CRC;
	rtl8197f_rtk_write(priv, RTL_RTK_CPUICR, val);

	/* Restore the 8197F SDK reinit FIFO watermarks after programming CPUICR burst
	 * size.  v5ag proved the external RTL8367D forwards frames to port7 while
	 * eth0 RX remains exactly zero; use the vendor reinit values and then kick
	 * RXCMD/TRXRDY again so the CPUIF DMA state machine samples descriptor base,
	 * CPUQDM and FIFO thresholds in the same order as the OEM driver.
	 */
	val = rtl8197f_rtk_read(priv, RTL_RTK_DMA_CR0);
	val &= ~(RTL_RTK_DMA_CR0_LOW_FIFO_MARK_MASK |
		 RTL_RTK_DMA_CR0_HIGH_FIFO_MARK_MASK);
	val |= RTL_RTK_DMA_CR0_LOW_FIFO_MARK(0x30) |
	       RTL_RTK_DMA_CR0_HIGH_FIFO_MARK(0xd7);
	rtl8197f_rtk_write(priv, RTL_RTK_DMA_CR0, val);
	readl(priv->base + RTL_RTK_DMA_CR0);

	rtl8197f_rtk_kick_rx(priv);
	rtl8197f_rtk_assert_trxrdy(priv);
	rtl8197f_rtk_rd05_poststart_rearm(priv, "hw-start-active");
	dev_info(priv->dev,
		 "rd05 cpuidma v38.2: cpuicr=0x%08x cpuicr1=0x%08x dma_cr0=0x%08x dma_cr1=0x%08x txringcr=0x%08x cpurpdcr0=0x%08x cpurmdcr0=0x%08x sirr=0x%08x macctrl1=0x%08x sysclk=0x%08x\n",
		 rtl8197f_rtk_read(priv, RTL_RTK_CPUICR),
		 rtl8197f_rtk_read(priv, RTL_RTK_CPUICR1),
		 rtl8197f_rtk_read(priv, RTL_RTK_DMA_CR0),
		 rtl8197f_rtk_read(priv, RTL_RTK_DMA_CR1),
		 rtl8197f_rtk_read(priv, RTL_RTK_TXRINGCR),
		 rtl8197f_rtk_read(priv, RTL_RTK_CPURPDCR0),
		 rtl8197f_rtk_read(priv, RTL_RTK_CPURMDCR0),
		 priv->swcore ? readl(priv->swcore + RTL_RTK_SWCORE_SWMISC_BASE + RTL_RTK_SWMISC_SSIR) : 0,
		 priv->swcore ? readl(priv->swcore + RTL_RTK_SWCORE_MACCTRL1) : 0,
		 priv->system ? readl(priv->system + RTL_RTK_SYSTEM_SYS_CLK_MAG) : 0);
}

static unsigned int rtl8197f_rtk_tx_clean(struct rtl8197f_rtknet_priv *priv)
{
	struct net_device *ndev = priv->ndev;
	unsigned int done = 0;
	unsigned long flags;

	spin_lock_irqsave(&priv->tx_lock, flags);
	while (priv->tx_tail != priv->tx_head) {
		unsigned int idx = priv->tx_tail;
		u32 opts1 = le32_to_cpu(priv->tx_desc[idx].opts1);
		struct sk_buff *skb;

		if (opts1 & RTL_RTK_DESC_OWN)
			break;

		dma_rmb();
		skb = priv->tx_skb[idx];
		if (skb) {
			dma_unmap_single(priv->dev, priv->tx_dma[idx],
					 priv->tx_len[idx], DMA_TO_DEVICE);
			ndev->stats.tx_packets++;
			ndev->stats.tx_bytes += priv->tx_len[idx];
			dev_consume_skb_any(skb);
			priv->tx_skb[idx] = NULL;
			priv->tx_len[idx] = 0;
		}

		priv->tx_desc[idx].addr = 0;
		priv->tx_desc[idx].opts2 = 0;
		priv->tx_desc[idx].opts3 = 0;
		priv->tx_desc[idx].opts4 = 0;
		priv->tx_desc[idx].opts5 = 0;
		priv->tx_desc[idx].opts6 = 0;
		priv->tx_desc[idx].opts7 = 0;
		priv->tx_tail = rtl8197f_rtk_next(idx, priv->tx_ring_size);
		done++;
	}

	if (netif_queue_stopped(ndev) && !rtl8197f_rtk_tx_full(priv))
		netif_wake_queue(ndev);

	spin_unlock_irqrestore(&priv->tx_lock, flags);

	return done;
}

static void rtl8197f_rtk_tx_reset(struct rtl8197f_rtknet_priv *priv)
{
	unsigned long flags;
	unsigned int i;

	spin_lock_irqsave(&priv->tx_lock, flags);
	for (i = 0; i < priv->tx_ring_size; i++) {
		if (priv->tx_skb[i]) {
			dma_unmap_single(priv->dev, priv->tx_dma[i],
					 priv->tx_len[i], DMA_TO_DEVICE);
			dev_kfree_skb_any(priv->tx_skb[i]);
		}

		priv->tx_skb[i] = NULL;
		priv->tx_dma[i] = 0;
		priv->tx_len[i] = 0;
		priv->tx_desc[i].addr = 0;
		priv->tx_desc[i].opts2 = 0;
		priv->tx_desc[i].opts3 = 0;
		priv->tx_desc[i].opts4 = 0;
		priv->tx_desc[i].opts5 = 0;
		priv->tx_desc[i].opts6 = 0;
		priv->tx_desc[i].opts7 = 0;
		priv->tx_desc[i].opts1 = cpu_to_le32(i == priv->tx_ring_size - 1 ?
							RTL_RTK_DESC_WRAP : 0);
	}
	priv->tx_head = 0;
	priv->tx_tail = 0;
	spin_unlock_irqrestore(&priv->tx_lock, flags);
}



#define RTL_RTK_RD05_RXTRACE_MAX_LOGS	96

static u16 rtl8197f_rtk_rd05_ethertype_at(const u8 *data, u32 len, u32 off)
{
	if (len < off + ETH_HLEN)
		return 0;

	return ((u16)data[off + 12] << 8) | data[off + 13];
}

static bool rtl8197f_rtk_rd05_bcast_at(const u8 *data, u32 len, u32 off)
{
	if (len < off + ETH_ALEN)
		return false;

	return is_broadcast_ether_addr(data + off);
}


static bool rtl8197f_rtk_rd05_magic_find(const u8 *data, u32 len,
                                         const char *token)
{
        u32 tlen = strlen(token);
        u32 i;

        if (!data || !token || !tlen || len < tlen)
                return false;

        for (i = 0; i <= len - tlen; i++) {
                if (!memcmp(data + i, token, tlen))
                        return true;
        }

        return false;
}

static u32 rtl8197f_rtk_rd05_mac_lo(const u8 *mac)
{
        if (!mac)
                return 0;

        return ((u32)mac[2] << 24) | ((u32)mac[3] << 16) |
               ((u32)mac[4] << 8) | mac[5];
}

static void rtl8197f_rtk_rd05_rxtrace(struct rtl8197f_rtknet_priv *priv,
					      unsigned int idx,
					      struct rtl8197f_rtk_desc *desc,
					      struct sk_buff *skb, u32 len,
					      u32 opts1, u32 opts2)
{
	const u8 *data;
	u32 opts3, opts4, opts5;
	u32 rx_ext3, rx_src13, rx_src24, rx_spa5;
	u16 p0, p4, p8;
	bool b0, b4, b8, self_loop, external, known_proto, suspect;

	if (!of_machine_is_compatible("xiaomi,r4-rd05"))
		return;

	if (len < ETH_HLEN)
		return;

	data = skb->data;
	opts3 = le32_to_cpu(desc->opts3);
	opts4 = le32_to_cpu(desc->opts4);
	opts5 = le32_to_cpu(desc->opts5);

	/*
	 * The vendor new-descriptor RX layout uses opts2[25:24] for EXTSPA and
	 * opts2[23:20] for DP_EXT.  Keep the older candidate decode fields too: the
	 * counters are useful when comparing raw CPU-ring traffic with the switch
	 * MIBs, even though only a non-local source MAC proves external ingress.
	 */
	rx_ext3 = (opts2 >> 24) & 0x3;
	rx_src13 = (opts4 >> 13) & 0x7;
	rx_src24 = (opts2 >> 20) & 0xf;
	rx_spa5 = rx_ext3;

	p0 = rtl8197f_rtk_rd05_ethertype_at(data, len, 0);
	p4 = rtl8197f_rtk_rd05_ethertype_at(data, len, 4);
	p8 = rtl8197f_rtk_rd05_ethertype_at(data, len, 8);
	b0 = rtl8197f_rtk_rd05_bcast_at(data, len, 0);
	b4 = rtl8197f_rtk_rd05_bcast_at(data, len, 4);
	b8 = rtl8197f_rtk_rd05_bcast_at(data, len, 8);

	/* V30 counted a locally generated, unchanged 0x8899 frame that had been
	 * reflected by the RTL865x extension path as successful PC ingress.  A
	 * standard RTL8367D CPU-tagged frame preserves the Ethernet source address,
	 * so separate that deterministic local loop from frames sourced externally.
	 */
	self_loop = p0 == ETH_P_REALTEK &&
		    ether_addr_equal(data + ETH_ALEN, priv->ndev->dev_addr);
	external = !ether_addr_equal(data + ETH_ALEN, priv->ndev->dev_addr);
	if (self_loop)
		priv->rd05_rx_self_loop++;
	else if (external)
		priv->rd05_rx_external++;

	if (external && READ_ONCE(rd05_magic_enable) &&
	    (rtl8197f_rtk_rd05_magic_find(data, len, RD05_MAGIC_TOKEN_V31) ||
	     rtl8197f_rtk_rd05_magic_find(data, len, RD05_MAGIC_TOKEN_V30) ||
	     rtl8197f_rtk_rd05_magic_find(data, len, RD05_MAGIC_TOKEN_V29) ||
	     rtl8197f_rtk_rd05_magic_find(data, len, RD05_MAGIC_TOKEN_AUTOTUNE))) {
		int cand = READ_ONCE(rd05_magic_candidate);

		priv->rd05_magic_hits++;
		priv->rd05_magic_last_idx = idx;
		priv->rd05_magic_last_len = len;
		priv->rd05_magic_last_candidate = cand;
		priv->rd05_magic_last_proto0 = p0;
		priv->rd05_magic_last_proto4 = p4;
		priv->rd05_magic_last_proto8 = p8;
		priv->rd05_magic_last_src_lo =
			rtl8197f_rtk_rd05_mac_lo(data + ETH_ALEN);
		priv->rd05_magic_last_dst_lo =
			rtl8197f_rtk_rd05_mac_lo(data);

		if ((unsigned int)atomic_inc_return(&rd05_magic_log_count) <=
		    READ_ONCE(rd05_magic_log_limit)) {
			dev_info(priv->dev,
				 "rd05 magic-rx v31: hit=%llu candidate=%d idx=%u len=%u p0/p4/p8=%04x/%04x/%04x src_lo=0x%08llx dst_lo=0x%08llx opts=%08x/%08x/%08x/%08x/%08x\n",
				 priv->rd05_magic_hits, cand, idx, len, p0, p4, p8,
				 priv->rd05_magic_last_src_lo,
				 priv->rd05_magic_last_dst_lo,
				 opts1, opts2, opts3, opts4, opts5);
			print_hex_dump(KERN_INFO, "rd05 magic-rx v31 raw: ",
				       DUMP_PREFIX_OFFSET, 16, 1, data,
				       min_t(u32, len, 96), false);
		}
	}

	priv->rd05_rxtrace_frames++;
	priv->rd05_rxtrace_last_idx = idx;
	priv->rd05_rxtrace_last_len = len;
	priv->rd05_rxtrace_last_opts1 = opts1;
	priv->rd05_rxtrace_last_opts2 = opts2;
	priv->rd05_rxtrace_last_opts3 = opts3;
	priv->rd05_rxtrace_last_opts4 = opts4;
	priv->rd05_rxtrace_last_opts5 = opts5;
	priv->rd05_rxdesc_last_ext3 = rx_ext3;
	priv->rd05_rxdesc_last_src13 = rx_src13;
	priv->rd05_rxdesc_last_src24 = rx_src24;
	priv->rd05_rxdesc_last_spa5 = rx_spa5;

	if (rx_ext3 == 0)
		priv->rd05_rxdesc_ext3_0++;
	else if (rx_ext3 == 2)
		priv->rd05_rxdesc_ext3_2++;

	if (rx_src13 == 6)
		priv->rd05_rxdesc_src13_p6++;
	else if (rx_src13 == 7)
		priv->rd05_rxdesc_src13_p7++;
	else
		priv->rd05_rxdesc_src13_other++;

	if (rx_spa5 == 0)
		priv->rd05_rxdesc_spa5_0++;
	else if (rx_spa5 == 2)
		priv->rd05_rxdesc_spa5_2++;

	if (b0)
		priv->rd05_rxtrace_bcast0++;
	if (b4)
		priv->rd05_rxtrace_bcast4++;
	if (b8)
		priv->rd05_rxtrace_bcast8++;
	if (p0 == ETH_P_ARP)
		priv->rd05_rxtrace_arp0++;
	if (p4 == ETH_P_ARP)
		priv->rd05_rxtrace_arp4++;
	if (p8 == ETH_P_ARP)
		priv->rd05_rxtrace_arp8++;
	if (p0 == ETH_P_REALTEK)
		priv->rd05_rxtrace_rtk0++;
	if (p4 == ETH_P_REALTEK)
		priv->rd05_rxtrace_rtk4++;

	/* User-visible protocol counters describe external ingress only.  Raw trace
	 * counters above deliberately retain all DMA frames, including local loops.
	 */
	if (external) {
		known_proto = false;
		if (p0 != ETH_P_REALTEK && p4 != ETH_P_REALTEK)
			priv->rd05_rx_untagged++;
		if (b0 || b4 || b8)
			priv->rd05_rx_bcast++;
		else if (is_multicast_ether_addr(data))
			priv->rd05_rx_mcast++;
		else
			priv->rd05_rx_ucast++;
		if (p0 == ETH_P_ARP || p4 == ETH_P_ARP || p8 == ETH_P_ARP) {
			priv->rd05_rx_arp++;
			known_proto = true;
		}
		if (p0 == ETH_P_IP || p4 == ETH_P_IP || p8 == ETH_P_IP) {
			priv->rd05_rx_ipv4++;
			known_proto = true;
		}
		if (p0 == ETH_P_IPV6 || p4 == ETH_P_IPV6 || p8 == ETH_P_IPV6) {
			priv->rd05_rx_ipv6++;
			known_proto = true;
		}
		if (!known_proto)
			priv->rd05_rx_other++;
	}

	/* Log useful protocols, local-loop evidence and the first frames. */
	suspect = self_loop || external || b0 || b4 || b8 ||
		  p0 == ETH_P_ARP || p4 == ETH_P_ARP || p8 == ETH_P_ARP ||
		  p0 == ETH_P_IP || p4 == ETH_P_IP || p8 == ETH_P_IP ||
		  p0 == ETH_P_REALTEK || p4 == ETH_P_REALTEK ||
		  p8 == ETH_P_REALTEK || priv->rd05_rxtrace_frames <= 16;

	if (suspect)
		priv->rd05_rxtrace_suspect++;

	if (suspect && priv->rd05_rxtrace_logs < RTL_RTK_RD05_RXTRACE_MAX_LOGS) {
		priv->rd05_rxtrace_logs++;
		dev_info(priv->dev,
			 "rd05 rxtrace v31: idx=%u len=%u self_loop=%u external=%u src=%pM opts=%08x/%08x/%08x/%08x/%08x extspa=%u spa=%u dp_ext=0x%x extspa_copy=%u p0/p4/p8=%04x/%04x/%04x b0/b4/b8=%u/%u/%u logs=%llu frames=%llu\n",
			 idx, len, self_loop, external, data + ETH_ALEN,
			 opts1, opts2, opts3, opts4, opts5,
			 rx_ext3, rx_src13, rx_src24, rx_spa5, p0, p4, p8,
			 b0, b4, b8, priv->rd05_rxtrace_logs,
			 priv->rd05_rxtrace_frames);
		print_hex_dump(KERN_INFO, "rd05 rxtrace v31 raw: ",
			       DUMP_PREFIX_OFFSET, 16, 1, data,
			       min_t(u32, len, 64), false);
	}
}

static u32 rtl8197f_rtk_rd05_legacy_len(struct rtl8197f_rtknet_priv *priv,
					       unsigned int idx)
{
	u32 ph_word1 = le32_to_cpu(priv->rd05_rx_ph[idx].ph_word1);
	u32 mb_word2 = le32_to_cpu(priv->rd05_rx_mbuf[idx].m_word2);
	u32 len = ph_word1 >> 16;

	if (!len)
		len = mb_word2 >> 16;

	return len;
}

static int rtl8197f_rtk_rd05_legacy_rx_poll(struct rtl8197f_rtknet_priv *priv,
						   int budget)
{
	struct net_device *ndev = priv->ndev;
	int work_done = 0;

	if (!priv->rd05_legacy_rx_enabled)
		return 0;

	while (work_done < budget) {
		unsigned int idx = priv->rd05_legacy_rx_tail;
		struct sk_buff *old_skb, *new_skb;
		dma_addr_t new_dma;
		u32 ent, len;
		struct rtl8197f_rtk_desc trace_desc = { };

		ent = le32_to_cpu(priv->rd05_rx_ph_ring[idx]);
		priv->rd05_legacy_rx_last_ent = ent;
		if (ent & RTL_RTK_DESC_OWN) {
			priv->rd05_legacy_rx_empty++;
			break;
		}

		dma_rmb();
		old_skb = priv->rx_skb[idx];
		if (!old_skb) {
			priv->rd05_legacy_rx_bad++;
			priv->rx_missing_skb++;
			ndev->stats.rx_dropped++;
			goto recycle_same;
		}

		len = rtl8197f_rtk_rd05_legacy_len(priv, idx);
		priv->rd05_legacy_rx_last_len = len;
		if (!len || len > priv->rx_buf_size) {
			priv->rd05_legacy_rx_bad++;
			priv->rx_bad_desc++;
			ndev->stats.rx_errors++;
			ndev->stats.rx_dropped++;
			goto recycle_same;
		}

		if (priv->sdk_crc_lengths) {
			if (len <= ETH_FCS_LEN) {
				priv->rd05_legacy_rx_bad++;
				priv->rx_bad_desc++;
				ndev->stats.rx_errors++;
				ndev->stats.rx_dropped++;
				goto recycle_same;
			}
			len -= ETH_FCS_LEN;
			priv->rx_crc_strips++;
		}

		dma_sync_single_for_cpu(priv->dev, priv->rx_dma[idx],
					priv->rx_buf_size, DMA_FROM_DEVICE);

		new_skb = netdev_alloc_skb_ip_align(ndev, priv->rx_buf_size);
		if (!new_skb) {
			priv->rx_alloc_fail++;
			ndev->stats.rx_dropped++;
			goto recycle_same;
		}

		new_dma = dma_map_single(priv->dev, new_skb->data,
					 priv->rx_buf_size, DMA_FROM_DEVICE);
		if (dma_mapping_error(priv->dev, new_dma)) {
			dev_kfree_skb_any(new_skb);
			priv->rx_dma_errors++;
			ndev->stats.rx_dropped++;
			goto recycle_same;
		}

		dma_unmap_single(priv->dev, priv->rx_dma[idx],
				 priv->rx_buf_size, DMA_FROM_DEVICE);

		skb_put(old_skb, len);
		old_skb->ip_summed = CHECKSUM_NONE;
		/* Reuse the existing RD05 RX tracer by projecting the legacy pkthdr
		 * fields into opts3/opts4/opts5 diagnostic slots.
		 */
		trace_desc.opts3 = priv->rd05_rx_ph[idx].ph_word2;
		trace_desc.opts4 = priv->rd05_rx_ph[idx].ph_word3;
		trace_desc.opts5 = priv->rd05_rx_ph[idx].ph_word4;
		rtl8197f_rtk_rd05_rxtrace(priv, idx, &trace_desc, old_skb, len,
					    ent, le32_to_cpu(priv->rd05_rx_ph[idx].ph_word1));
		old_skb->protocol = eth_type_trans(old_skb, ndev);
		napi_gro_receive(&priv->napi, old_skb);

		ndev->stats.rx_packets++;
		ndev->stats.rx_bytes += len;
		priv->rd05_legacy_rx_pkts++;
		priv->rx_skb[idx] = new_skb;
		priv->rx_dma[idx] = new_dma;
		work_done++;

recycle_same:
		if (priv->rx_skb[idx])
			dma_sync_single_for_device(priv->dev, priv->rx_dma[idx],
						   priv->rx_buf_size, DMA_FROM_DEVICE);
		rtl8197f_rtk_rd05_legacy_rx_init_slot(priv, idx);
		dma_wmb();
		priv->rd05_legacy_rx_recycles++;
		priv->rd05_legacy_rx_tail = rtl8197f_rtk_next(idx, priv->rx_ring_size);
	}

	return work_done;
}

static int rtl8197f_rtk_rx_poll(struct rtl8197f_rtknet_priv *priv, int budget)
{
	struct net_device *ndev = priv->ndev;
	int work_done = 0;

	if (!priv->rd05_sdk_newdesc_rx) {
		work_done = rtl8197f_rtk_rd05_legacy_rx_poll(priv, budget);
		if (work_done >= budget)
			return work_done;
	}

	while (work_done < budget) {
		unsigned int idx = priv->rx_tail;
		struct rtl8197f_rtk_desc *desc = &priv->rx_desc[idx];
		struct sk_buff *old_skb, *new_skb;
		dma_addr_t new_dma;
		u32 opts1, opts2, len, hw_idx = 0;
		bool cdp_valid, cdp_advanced;

		opts1 = le32_to_cpu(desc->opts1);
		cdp_valid = rtl8197f_rtk_rx_cdp_index(priv, &hw_idx);
		cdp_advanced = cdp_valid && idx != hw_idx;
		if (cdp_advanced) {
			priv->rx_cdp_advanced++;
			if (opts1 & RTL_RTK_DESC_OWN)
				priv->rx_cdp_owned_advanced++;
		} else if (opts1 & RTL_RTK_DESC_OWN) {
			break;
		}

		dma_rmb();
		old_skb = priv->rx_skb[idx];
		if (!old_skb) {
			priv->rx_missing_skb++;
			ndev->stats.rx_dropped++;
			if (rtl8197f_rtk_alloc_rx_buf(priv, idx)) {
				priv->rx_alloc_fail++;
				break;
			}
			priv->rx_tail = rtl8197f_rtk_next(idx, priv->rx_ring_size);
			priv->rx_restart_pending = true;
			continue;
		}

		opts2 = le32_to_cpu(desc->opts2);
		len = opts2 & RTL_RTK_RX_LEN_MASK;

		if (!len || len > priv->rx_buf_size ||
		    !(opts1 & RTL_RTK_DESC_FIRST) || !(opts1 & RTL_RTK_DESC_LAST)) {
			priv->rx_bad_desc++;
			ndev->stats.rx_errors++;
			ndev->stats.rx_dropped++;
			goto recycle;
		}

		/* SDK mode leaves CPUICR EXCLUDE_CRC clear; descriptors include FCS. */
		if (priv->sdk_crc_lengths) {
			if (len <= ETH_FCS_LEN) {
				priv->rx_bad_desc++;
				ndev->stats.rx_errors++;
				ndev->stats.rx_dropped++;
				goto recycle;
			}
			len -= ETH_FCS_LEN;
			priv->rx_crc_strips++;
		}

		/*
		 * RTL8197F is non-coherent MIPS in this tree.  Descriptor rings
		 * are coherent, but RX payload buffers are streamed with
		 * dma_map_single().  Hand completed buffers to the CPU before
		 * packet reads and hand recycled buffers back before OWN is set.
		 */
		dma_sync_single_for_cpu(priv->dev, priv->rx_dma[idx],
					priv->rx_buf_size, DMA_FROM_DEVICE);

		new_skb = netdev_alloc_skb_ip_align(ndev, priv->rx_buf_size);
		if (!new_skb) {
			priv->rx_alloc_fail++;
			ndev->stats.rx_dropped++;
			goto recycle;
		}

		new_dma = dma_map_single(priv->dev, new_skb->data,
					 priv->rx_buf_size, DMA_FROM_DEVICE);
		if (dma_mapping_error(priv->dev, new_dma)) {
			dev_kfree_skb_any(new_skb);
			priv->rx_dma_errors++;
			ndev->stats.rx_dropped++;
			goto recycle;
		}

		dma_unmap_single(priv->dev, priv->rx_dma[idx],
				 priv->rx_buf_size, DMA_FROM_DEVICE);

		skb_put(old_skb, len);
		old_skb->ip_summed = CHECKSUM_NONE;
		rtl8197f_rtk_rd05_rxtrace(priv, idx, desc, old_skb, len, opts1, opts2);
		old_skb->protocol = eth_type_trans(old_skb, ndev);
		napi_gro_receive(&priv->napi, old_skb);

		ndev->stats.rx_packets++;
		ndev->stats.rx_bytes += len;

		priv->rx_skb[idx] = new_skb;
		priv->rx_dma[idx] = new_dma;
		desc->addr = cpu_to_le32(rtl8197f_rtk_hw_dma_addr(priv, new_dma));
		work_done++;

recycle:
		dma_sync_single_for_device(priv->dev, priv->rx_dma[idx],
					   priv->rx_buf_size, DMA_FROM_DEVICE);

		desc->opts2 = 0;
		desc->opts3 = 0;
		desc->opts4 = 0;
		desc->opts5 = 0;
		desc->opts6 = 0;
		desc->opts7 = 0;
		dma_wmb();
		desc->opts1 = cpu_to_le32(RTL_RTK_DESC_OWN |
						(priv->rx_buf_size << RTL_RTK_RX_EXTSIZE_SHIFT) |
						(idx == priv->rx_ring_size - 1 ?
						 RTL_RTK_DESC_WRAP : 0));
		priv->rx_tail = rtl8197f_rtk_next(idx, priv->rx_ring_size);
	}

	return work_done;
}

static int rtl8197f_rtk_poll(struct napi_struct *napi, int budget)
{
	struct rtl8197f_rtknet_priv *priv =
		container_of(napi, struct rtl8197f_rtknet_priv, napi);
	int work_done;

	priv->napi_polls++;
	rtl8197f_rtk_tx_clean(priv);
	work_done = rtl8197f_rtk_rx_poll(priv, budget);
	if (priv->rx_restart_pending) {
		priv->rx_restart_pending = false;
		rtl8197f_rtk_kick_rx(priv);
	}

	if (work_done < budget && napi_complete_done(napi, work_done))
		rtl8197f_rtk_enable_irq(priv);

	return work_done;
}


static void rtl8197f_rtk_rx_poll_timer(struct timer_list *t)
{
	struct rtl8197f_rtknet_priv *priv =
		from_timer(priv, t, rx_poll_timer);

	if (!priv->rx_poll_fallback || !netif_running(priv->ndev))
		return;

	priv->rx_poll_timer_runs++;
	if (napi_schedule_prep(&priv->napi)) {
		rtl8197f_rtk_disable_irq(priv);
		__napi_schedule(&priv->napi);
	}

	mod_timer(&priv->rx_poll_timer,
		  jiffies + msecs_to_jiffies(priv->rx_poll_ms));
}

static irqreturn_t rtl8197f_rtk_irq(int irq, void *dev_id)
{
	struct net_device *ndev = dev_id;
	struct rtl8197f_rtknet_priv *priv = netdev_priv(ndev);
	u32 status;

	status = rtl8197f_rtk_read(priv, RTL_RTK_CPUIISR);
	if (!(status & RTL_RTK_INT_MASK))
		return IRQ_NONE;

	rtl8197f_rtk_write(priv, RTL_RTK_CPUIISR, status);

	if (status & RTL_RTK_INT_RX_DONE0)
		priv->irq_rx_done++;
	if (status & (RTL_RTK_INT_TX_DONE0 | RTL_RTK_INT_TX_ALL_DONE0))
		priv->irq_tx_done++;
	if (status & RTL_RTK_INT_RX_ERR0) {
		priv->irq_rx_errors++;
		ndev->stats.rx_errors++;
	}
	if (status & RTL_RTK_INT_TX_ERR0) {
		priv->irq_tx_errors++;
		ndev->stats.tx_errors++;
	}
	if (status & RTL_RTK_INT_RX_RUNOUT0) {
		priv->irq_rx_runout++;
		priv->rx_restart_pending = true;
	}
	if (status & RTL_RTK_INT_MBUF_RUNOUT0) {
		priv->irq_mbuf_runout++;
		priv->rx_restart_pending = true;
	}

	if (status & RTL_RTK_INT_LINK_CHANGE)
		priv->irq_link_change++;

	if (napi_schedule_prep(&priv->napi)) {
		rtl8197f_rtk_disable_irq(priv);
		__napi_schedule(&priv->napi);
	}

	return IRQ_HANDLED;
}


struct rtl8197f_rtk_txmeta {
	u32 port_mask;
	u32 dp_ext;
	u32 extspa;
	bool hwlookup;
	bool bridge;
};

static const char *rtl8197f_rtk_rd05_txdesc_mode_name(int mode)
{
	switch (mode) {
	case 0: return "direct-p0-rgmii";
	case 1: return "legacy-dp40-ext2-loop";
	case 2: return "dp40-ext0";
	case 3: return "dp40-ext1";
	case 4: return "dp40-ext3";
	case 5: return "dp20-ext2";
	case 6: return "dp10-ext2";
	case 7: return "dp1f-ext2";
	case 8: return "current-hwlookup";
	case 9: return "current-bridge";
	case 10: return "current-hwlookup-bridge";
	case 11: return "current-extspa1";
	case 12: return "current-extspa2";
	default: return "current-invalid";
	}
}

static void rtl8197f_rtk_rd05_txdesc_meta(struct rtl8197f_rtknet_priv *priv,
						  struct rtl8197f_rtk_txmeta *m)
{
	int mode = READ_ONCE(rd05_txdesc_mode);

	m->port_mask = priv->tx_port_mask;
	m->dp_ext = priv->tx_dp_ext;
	m->extspa = priv->tx_extspa;
	m->hwlookup = priv->tx_hwlookup;
	m->bridge = priv->tx_bridge;

	if (!of_machine_is_compatible("xiaomi,r4-rd05"))
		return;

	/*
	 * The values below intentionally remain inside documented descriptor bit
	 * widths.  They do not alter DSA tagging; they only vary the rtl865x TX DMA
	 * destination-port/extension/source metadata so rd05-netdiag full can prove
	 * which SoC-side egress selector, if any, becomes visible at the PC port.
	 */
	switch (mode) {
	case 0:
		break;
	case 1:
		/* v30 regression reproducer: CPU/extension selector loops back into
		 * the local DMA path instead of driving physical P0/RGMII.
		 */
		m->port_mask = 0x40;
		m->dp_ext = 0x2;
		break;
	case 2:
		m->port_mask = 0x40;
		m->dp_ext = 0x0;
		break;
	case 3:
		m->port_mask = 0x40;
		m->dp_ext = 0x1;
		break;
	case 4:
		m->port_mask = 0x40;
		m->dp_ext = 0x3;
		break;
	case 5:
		m->port_mask = 0x20;
		m->dp_ext = 0x2;
		break;
	case 6:
		m->port_mask = 0x10;
		m->dp_ext = 0x2;
		break;
	case 7:
		m->port_mask = 0x1f;
		m->dp_ext = 0x2;
		break;
	case 8:
		m->hwlookup = true;
		break;
	case 9:
		m->bridge = true;
		break;
	case 10:
		m->hwlookup = true;
		m->bridge = true;
		break;
	case 11:
		m->extspa = 0x1;
		break;
	case 12:
		m->extspa = 0x2;
		break;
	default:
		break;
	}

	m->port_mask &= RTL_RTK_TX_DP_MASK;
	m->dp_ext &= RTL_RTK_TX_DP_EXT_MASK;
	m->extspa &= RTL_RTK_TX_EXTSPA_MASK;
}

static void rtl8197f_rtk_rd05_txdesc_trace(struct rtl8197f_rtknet_priv *priv,
						   struct sk_buff *skb, u32 len,
						   const struct rtl8197f_rtk_txmeta *m,
						   u32 opts1, u32 opts2, u32 opts3,
						   u32 opts4, u32 opts5)
{
	const unsigned char *d = skb->data;
	u16 proto = 0;
	int n;

	if (!of_machine_is_compatible("xiaomi,r4-rd05"))
		return;
	if (len < ETH_HLEN)
		return;
	proto = ((u16)d[12] << 8) | d[13];
	if (proto != ETH_P_ARP && proto != ETH_P_IP && proto != ETH_P_REALTEK)
		return;

	n = atomic_inc_return(&rd05_txdesc_trace_count);
	if ((unsigned int)n > READ_ONCE(rd05_txdesc_trace_limit))
		return;

	dev_info(priv->dev,
		 "RD05 txdesc trace v31: mode=%d/%s len=%u proto=0x%04x dp=0x%x dp_ext=0x%x extspa=0x%x hwlookup=%d bridge=%d opts=%08x/%08x/%08x/%08x/%08x da=%pM sa=%pM logs=%d\n",
		 READ_ONCE(rd05_txdesc_mode),
		 rtl8197f_rtk_rd05_txdesc_mode_name(READ_ONCE(rd05_txdesc_mode)),
		 len, proto, m->port_mask, m->dp_ext, m->extspa, m->hwlookup,
		 m->bridge, opts1, opts2, opts3, opts4, opts5, d, d + ETH_ALEN, n);
}

static netdev_tx_t rtl8197f_rtk_start_xmit(struct sk_buff *skb,
						   struct net_device *ndev)
{
	struct rtl8197f_rtknet_priv *priv = netdev_priv(ndev);
	struct rtl8197f_rtk_desc *desc;
	unsigned int idx;
	dma_addr_t dma;
	u32 len, desc_len, opts1, opts2, opts3, opts4, opts5;
	struct rtl8197f_rtk_txmeta txm;
	unsigned long flags;

	if (skb->len > ndev->mtu + ETH_HLEN + VLAN_HLEN) {
		priv->tx_mtu_drops++;
		ndev->stats.tx_dropped++;
		dev_kfree_skb_any(skb);
		return NETDEV_TX_OK;
	}

	/*
	 * The rtl865x TX DMA engine does not safely synthesize Ethernet padding
	 * for runt frames.  Pad before mapping so the descriptor length never
	 * exposes uninitialised memory and the RTL8367D/DSA CPU link does not see
	 * malformed undersized packets.
	 */
	if (skb_put_padto(skb, ETH_ZLEN)) {
		ndev->stats.tx_dropped++;
		return NETDEV_TX_OK;
	}

	dma = dma_map_single(priv->dev, skb->data, skb->len, DMA_TO_DEVICE);
	if (dma_mapping_error(priv->dev, dma)) {
		priv->tx_dma_errors++;
		ndev->stats.tx_dropped++;
		dev_kfree_skb_any(skb);
		return NETDEV_TX_OK;
	}

	spin_lock_irqsave(&priv->tx_lock, flags);
	if (rtl8197f_rtk_tx_full(priv)) {
		priv->tx_ring_full++;
		netif_stop_queue(ndev);
		spin_unlock_irqrestore(&priv->tx_lock, flags);
		dma_unmap_single(priv->dev, dma, skb->len, DMA_TO_DEVICE);
		return NETDEV_TX_BUSY;
	}

	idx = priv->tx_head;
	desc = &priv->tx_desc[idx];
	if (le32_to_cpu(desc->opts1) & RTL_RTK_DESC_OWN) {
		priv->tx_busy_desc++;
		netif_stop_queue(ndev);
		spin_unlock_irqrestore(&priv->tx_lock, flags);
		dma_unmap_single(priv->dev, dma, skb->len, DMA_TO_DEVICE);
		return NETDEV_TX_BUSY;
	}

	len = skb->len;
	desc_len = len;
	if (priv->sdk_crc_lengths) {
		desc_len = max_t(u32, len + ETH_FCS_LEN, 64);
		priv->tx_crc_len_adjusts++;
	}
	priv->tx_skb[idx] = skb;
	priv->tx_dma[idx] = dma;
	priv->tx_len[idx] = len;

	rtl8197f_rtk_rd05_txdesc_meta(priv, &txm);
	desc->addr = cpu_to_le32(rtl8197f_rtk_hw_dma_addr(priv, dma));
	opts2 = desc_len << RTL_RTK_TX_M_LEN_SHIFT;
	opts3 = (txm.dp_ext & RTL_RTK_TX_DP_EXT_MASK) <<
		 RTL_RTK_TX_DP_EXT_SHIFT;
	opts4 = (txm.port_mask & RTL_RTK_TX_DP_MASK) << RTL_RTK_TX_DP_SHIFT;
	opts5 = (txm.extspa & RTL_RTK_TX_EXTSPA_MASK) <<
		 RTL_RTK_TX_EXTSPA_SHIFT;
	desc->opts2 = cpu_to_le32(opts2);
	desc->opts3 = cpu_to_le32(opts3);
	desc->opts4 = cpu_to_le32(opts4);
	desc->opts5 = cpu_to_le32(opts5);
	desc->opts6 = 0;
	desc->opts7 = 0;

	opts1 = RTL_RTK_DESC_OWN | RTL_RTK_DESC_FIRST | RTL_RTK_DESC_LAST |
		(desc_len << RTL_RTK_TX_PH_LEN_SHIFT);
	if (txm.hwlookup)
		opts1 |= RTL_RTK_TX_HWLKUP;
	if (txm.bridge)
		opts1 |= RTL_RTK_TX_BRIDGE;
	if (idx == priv->tx_ring_size - 1)
		opts1 |= RTL_RTK_DESC_WRAP;

	rtl8197f_rtk_rd05_txdesc_trace(priv, skb, len, &txm, opts1, opts2, opts3, opts4, opts5);

	dma_wmb();
	desc->opts1 = cpu_to_le32(opts1);

	priv->tx_head = rtl8197f_rtk_next(idx, priv->tx_ring_size);
	if (rtl8197f_rtk_tx_full(priv))
		netif_stop_queue(ndev);

	netif_trans_update(ndev);
	priv->tx_kicks++;
	rtl8197f_rtk_write(priv, RTL_RTK_CPUICR,
			    rtl8197f_rtk_read(priv, RTL_RTK_CPUICR) |
			    RTL_RTK_CPUICR_TXFD | RTL_RTK_CPUICR_TXCMD |
			    RTL_RTK_CPUICR_RXCMD);
	rtl8197f_rtk_assert_trxrdy(priv);

	spin_unlock_irqrestore(&priv->tx_lock, flags);
	return NETDEV_TX_OK;
}

static int rtl8197f_rtk_open(struct net_device *ndev)
{
	struct rtl8197f_rtknet_priv *priv = netdev_priv(ndev);
	int ret;

	ret = rtl8197f_rtk_alloc_rings(priv);
	if (ret)
		return ret;

	ret = request_irq(priv->irq, rtl8197f_rtk_irq, 0, ndev->name, ndev);
	if (ret)
		goto err_free_rings;

	napi_enable(&priv->napi);
	rtl8197f_rtk_hw_start(priv);
	netif_carrier_on(ndev);
	netif_start_queue(ndev);
	if (priv->rx_poll_fallback)
		mod_timer(&priv->rx_poll_timer,
			  jiffies + msecs_to_jiffies(priv->rx_poll_ms));
	if (of_machine_is_compatible("xiaomi,r4-rd05"))
		schedule_delayed_work(&priv->rd05_reseed_work, msecs_to_jiffies(3000));

	return 0;

err_free_rings:
	rtl8197f_rtk_free_rings(priv);
	return ret;
}

static int rtl8197f_rtk_stop(struct net_device *ndev)
{
	struct rtl8197f_rtknet_priv *priv = netdev_priv(ndev);

	netif_stop_queue(ndev);
	netif_carrier_off(ndev);
	cancel_delayed_work_sync(&priv->rd05_reseed_work);
	del_timer_sync(&priv->rx_poll_timer);
	rtl8197f_rtk_hw_stop(priv);
	napi_disable(&priv->napi);
	free_irq(priv->irq, ndev);
	rtl8197f_rtk_free_rings(priv);
	return 0;
}

static void rtl8197f_rtk_tx_timeout(struct net_device *ndev,
					    unsigned int txqueue)
{
	struct rtl8197f_rtknet_priv *priv = netdev_priv(ndev);

	priv->tx_timeouts++;
	ndev->stats.tx_errors++;
	netdev_warn(ndev, "TX timeout, resetting rtl865x CPU ring\n");
	rtl8197f_rtk_hw_stop(priv);
	rtl8197f_rtk_tx_reset(priv);
	rtl8197f_rtk_hw_start(priv);
	netif_wake_queue(ndev);
}

static int rtl8197f_rtk_change_mtu(struct net_device *ndev, int mtu)
{
	if (netif_running(ndev))
		return -EBUSY;
	if (mtu < ndev->min_mtu || mtu > ndev->max_mtu)
		return -EINVAL;

	ndev->mtu = mtu;
	return 0;
}

static void rtl8197f_rtk_get_drvinfo(struct net_device *ndev,
				     struct ethtool_drvinfo *info)
{
	strscpy(info->driver, DRV_NAME, sizeof(info->driver));
	strscpy(info->version, DRV_VERSION, sizeof(info->version));
	strscpy(info->bus_info, dev_name(ndev->dev.parent), sizeof(info->bus_info));
}

static u32 rtl8197f_rtk_get_msglevel(struct net_device *ndev)
{
	struct rtl8197f_rtknet_priv *priv = netdev_priv(ndev);

	return priv->msg_enable;
}

static void rtl8197f_rtk_set_msglevel(struct net_device *ndev, u32 value)
{
	struct rtl8197f_rtknet_priv *priv = netdev_priv(ndev);

	priv->msg_enable = value;
}

static int rtl8197f_rtk_get_regs_len(struct net_device *ndev)
{
	return RTL_RTK_REG_DUMP_LEN;
}

static void rtl8197f_rtk_get_regs(struct net_device *ndev,
				  struct ethtool_regs *regs, void *data)
{
	struct rtl8197f_rtknet_priv *priv = netdev_priv(ndev);
	u32 *buf = data;
	u32 off;

	regs->version = 1;
	for (off = 0; off < RTL_RTK_REG_DUMP_LEN; off += sizeof(u32))
		buf[off / sizeof(u32)] = rtl8197f_rtk_read(priv, off);
}

static void rtl8197f_rtk_get_ringparam(struct net_device *ndev,
				       struct ethtool_ringparam *ring,
				       struct kernel_ethtool_ringparam *kernel_ring,
				       struct netlink_ext_ack *extack)
{
	struct rtl8197f_rtknet_priv *priv = netdev_priv(ndev);

	ring->rx_max_pending = RTL_RTK_MAX_RING;
	ring->tx_max_pending = RTL_RTK_MAX_RING;
	ring->rx_pending = priv->rx_ring_size;
	ring->tx_pending = priv->tx_ring_size;
}

static int rtl8197f_rtk_set_ringparam(struct net_device *ndev,
				      struct ethtool_ringparam *ring,
				      struct kernel_ethtool_ringparam *kernel_ring,
				      struct netlink_ext_ack *extack)
{
	struct rtl8197f_rtknet_priv *priv = netdev_priv(ndev);
	u32 rx, tx;

	if (netif_running(ndev))
		return -EBUSY;

	if (ring->rx_mini_pending || ring->rx_jumbo_pending)
		return -EINVAL;

	rx = clamp_t(u32, ring->rx_pending, RTL_RTK_MIN_RING, RTL_RTK_MAX_RING);
	tx = clamp_t(u32, ring->tx_pending, RTL_RTK_MIN_RING, RTL_RTK_MAX_RING);
	priv->rx_ring_size = rx;
	priv->tx_ring_size = tx;

	return 0;
}

static void rtl8197f_rtk_get_strings(struct net_device *ndev, u32 stringset,
				     u8 *data)
{
	if (stringset != ETH_SS_STATS)
		return;

	memcpy(data, rtl8197f_rtk_gstrings_stats,
	       sizeof(rtl8197f_rtk_gstrings_stats));
}

static int rtl8197f_rtk_get_sset_count(struct net_device *ndev, int sset)
{
	if (sset == ETH_SS_STATS)
		return RTL_RTK_NUM_STATS;

	return -EOPNOTSUPP;
}

static void rtl8197f_rtk_get_ethtool_stats(struct net_device *ndev,
					   struct ethtool_stats *stats, u64 *data)
{
	struct rtl8197f_rtknet_priv *priv = netdev_priv(ndev);

	data[RTL_RTK_STAT_IRQ_RX_DONE] = priv->irq_rx_done;
	data[RTL_RTK_STAT_IRQ_TX_DONE] = priv->irq_tx_done;
	data[RTL_RTK_STAT_IRQ_RX_ERRORS] = priv->irq_rx_errors;
	data[RTL_RTK_STAT_IRQ_TX_ERRORS] = priv->irq_tx_errors;
	data[RTL_RTK_STAT_IRQ_RX_RUNOUT] = priv->irq_rx_runout;
	data[RTL_RTK_STAT_IRQ_MBUF_RUNOUT] = priv->irq_mbuf_runout;
	data[RTL_RTK_STAT_IRQ_LINK_CHANGE] = priv->irq_link_change;
	data[RTL_RTK_STAT_NAPI_POLLS] = priv->napi_polls;
	data[RTL_RTK_STAT_RX_BAD_DESC] = priv->rx_bad_desc;
	data[RTL_RTK_STAT_RX_MISSING_SKB] = priv->rx_missing_skb;
	data[RTL_RTK_STAT_RX_ALLOC_FAIL] = priv->rx_alloc_fail;
	data[RTL_RTK_STAT_RX_DMA_ERRORS] = priv->rx_dma_errors;
	data[RTL_RTK_STAT_RX_CDP_READS] = priv->rx_cdp_reads;
	data[RTL_RTK_STAT_RX_CDP_ADVANCED] = priv->rx_cdp_advanced;
	data[RTL_RTK_STAT_RX_CDP_OWNED_ADVANCED] = priv->rx_cdp_owned_advanced;
	data[RTL_RTK_STAT_RX_CDP_INVALID] = priv->rx_cdp_invalid;
	data[RTL_RTK_STAT_RX_CDP_LAST] = priv->rx_cdp_last;
	data[RTL_RTK_STAT_RX_CDP_LAST_IDX] = priv->rx_cdp_last_idx;
	data[RTL_RTK_STAT_TX_MTU_DROPS] = priv->tx_mtu_drops;
	data[RTL_RTK_STAT_TX_DMA_ERRORS] = priv->tx_dma_errors;
	data[RTL_RTK_STAT_TX_RING_FULL] = priv->tx_ring_full;
	data[RTL_RTK_STAT_TX_BUSY_DESC] = priv->tx_busy_desc;
	data[RTL_RTK_STAT_TX_TIMEOUTS] = priv->tx_timeouts;
	data[RTL_RTK_STAT_HW_RX_RESTARTS] = priv->hw_rx_restarts;
	data[RTL_RTK_STAT_RX_POLL_TIMER_RUNS] = priv->rx_poll_timer_runs;
	data[RTL_RTK_STAT_SW_TRXRDY_WRITES] = priv->sw_trxrdy_writes;
	data[RTL_RTK_STAT_CPUICR1_SDK_WRITES] = priv->cpuicr1_sdk_writes;
	data[RTL_RTK_STAT_SWCORE_SDK_WRITES] = priv->swcore_sdk_writes;
	data[RTL_RTK_STAT_TXRINGCR_SDK_WRITES] = priv->txringcr_sdk_writes;
	data[RTL_RTK_STAT_SIRR_DIRECT_WRITES] = priv->sirr_direct_writes;
	data[RTL_RTK_STAT_GIMR_SDK_WRITES] = priv->gimr_sdk_writes;
	data[RTL_RTK_STAT_ACL0_SDK_READS] = priv->acl0_sdk_reads;
	data[RTL_RTK_STAT_TX_KICKS] = priv->tx_kicks;
	data[RTL_RTK_STAT_TX_CRC_LEN_ADJUSTS] = priv->tx_crc_len_adjusts;
	data[RTL_RTK_STAT_RX_CRC_STRIPS] = priv->rx_crc_strips;
	data[RTL_RTK_STAT_RD05_RX_UNTAGGED] = priv->rd05_rx_untagged;
	data[RTL_RTK_STAT_RD05_RX_EXTERNAL] = priv->rd05_rx_external;
	data[RTL_RTK_STAT_RD05_RX_SELF_LOOP] = priv->rd05_rx_self_loop;
	data[RTL_RTK_STAT_RD05_RX_ARP] = priv->rd05_rx_arp;
	data[RTL_RTK_STAT_RD05_RX_IPV4] = priv->rd05_rx_ipv4;
	data[RTL_RTK_STAT_RD05_RX_IPV6] = priv->rd05_rx_ipv6;
	data[RTL_RTK_STAT_RD05_RX_OTHER] = priv->rd05_rx_other;
	data[RTL_RTK_STAT_RD05_RX_BCAST] = priv->rd05_rx_bcast;
	data[RTL_RTK_STAT_RD05_RX_MCAST] = priv->rd05_rx_mcast;
	data[RTL_RTK_STAT_RD05_RX_UCAST] = priv->rd05_rx_ucast;
	data[RTL_RTK_STAT_RD05_RX_DIRECT_LAN2] = priv->rd05_rx_direct_lan2;
	data[RTL_RTK_STAT_RD05_RXTRACE_FRAMES] = priv->rd05_rxtrace_frames;
	data[RTL_RTK_STAT_RD05_RXTRACE_LOGS] = priv->rd05_rxtrace_logs;
	data[RTL_RTK_STAT_RD05_RXTRACE_BCAST0] = priv->rd05_rxtrace_bcast0;
	data[RTL_RTK_STAT_RD05_RXTRACE_BCAST4] = priv->rd05_rxtrace_bcast4;
	data[RTL_RTK_STAT_RD05_RXTRACE_BCAST8] = priv->rd05_rxtrace_bcast8;
	data[RTL_RTK_STAT_RD05_RXTRACE_ARP0] = priv->rd05_rxtrace_arp0;
	data[RTL_RTK_STAT_RD05_RXTRACE_ARP4] = priv->rd05_rxtrace_arp4;
	data[RTL_RTK_STAT_RD05_RXTRACE_ARP8] = priv->rd05_rxtrace_arp8;
	data[RTL_RTK_STAT_RD05_RXTRACE_RTK0] = priv->rd05_rxtrace_rtk0;
	data[RTL_RTK_STAT_RD05_RXTRACE_RTK4] = priv->rd05_rxtrace_rtk4;
	data[RTL_RTK_STAT_RD05_RXTRACE_SUSPECT] = priv->rd05_rxtrace_suspect;
	data[RTL_RTK_STAT_RD05_RXTRACE_LAST_IDX] = priv->rd05_rxtrace_last_idx;
	data[RTL_RTK_STAT_RD05_RXTRACE_LAST_LEN] = priv->rd05_rxtrace_last_len;
	data[RTL_RTK_STAT_RD05_RXTRACE_LAST_OPTS1] = priv->rd05_rxtrace_last_opts1;
	data[RTL_RTK_STAT_RD05_RXTRACE_LAST_OPTS2] = priv->rd05_rxtrace_last_opts2;
	data[RTL_RTK_STAT_RD05_RXTRACE_LAST_OPTS3] = priv->rd05_rxtrace_last_opts3;
	data[RTL_RTK_STAT_RD05_RXTRACE_LAST_OPTS4] = priv->rd05_rxtrace_last_opts4;
	data[RTL_RTK_STAT_RD05_RXTRACE_LAST_OPTS5] = priv->rd05_rxtrace_last_opts5;
	data[RTL_RTK_STAT_RD05_RXDESC_LAST_EXT3] = priv->rd05_rxdesc_last_ext3;
	data[RTL_RTK_STAT_RD05_RXDESC_LAST_SRC13] = priv->rd05_rxdesc_last_src13;
	data[RTL_RTK_STAT_RD05_RXDESC_LAST_SRC24] = priv->rd05_rxdesc_last_src24;
	data[RTL_RTK_STAT_RD05_RXDESC_LAST_SPA5] = priv->rd05_rxdesc_last_spa5;
	data[RTL_RTK_STAT_RD05_RXDESC_EXT3_0] = priv->rd05_rxdesc_ext3_0;
	data[RTL_RTK_STAT_RD05_RXDESC_EXT3_2] = priv->rd05_rxdesc_ext3_2;
	data[RTL_RTK_STAT_RD05_RXDESC_SRC13_P6] = priv->rd05_rxdesc_src13_p6;
	data[RTL_RTK_STAT_RD05_RXDESC_SRC13_P7] = priv->rd05_rxdesc_src13_p7;
	data[RTL_RTK_STAT_RD05_RXDESC_SRC13_OTHER] = priv->rd05_rxdesc_src13_other;
	data[RTL_RTK_STAT_RD05_RXDESC_SPA5_0] = priv->rd05_rxdesc_spa5_0;
	data[RTL_RTK_STAT_RD05_RXDESC_SPA5_2] = priv->rd05_rxdesc_spa5_2;
	data[RTL_RTK_STAT_RD05_MAGIC_HITS] = priv->rd05_magic_hits;
	data[RTL_RTK_STAT_RD05_MAGIC_LAST_IDX] = priv->rd05_magic_last_idx;
	data[RTL_RTK_STAT_RD05_MAGIC_LAST_LEN] = priv->rd05_magic_last_len;
	data[RTL_RTK_STAT_RD05_MAGIC_LAST_CANDIDATE] = priv->rd05_magic_last_candidate;
	data[RTL_RTK_STAT_RD05_MAGIC_LAST_PROTO0] = priv->rd05_magic_last_proto0;
	data[RTL_RTK_STAT_RD05_MAGIC_LAST_PROTO4] = priv->rd05_magic_last_proto4;
	data[RTL_RTK_STAT_RD05_MAGIC_LAST_PROTO8] = priv->rd05_magic_last_proto8;
	data[RTL_RTK_STAT_RD05_MAGIC_LAST_SRC_LO] = priv->rd05_magic_last_src_lo;
	data[RTL_RTK_STAT_RD05_MAGIC_LAST_DST_LO] = priv->rd05_magic_last_dst_lo;
	data[RTL_RTK_STAT_RD05_L2CPU_RUNS] = priv->rd05_l2cpu_runs;
	data[RTL_RTK_STAT_RD05_L2CPU_OK] = priv->rd05_l2cpu_ok;
	data[RTL_RTK_STAT_RD05_L2CPU_FAIL] = priv->rd05_l2cpu_fail;
	data[RTL_RTK_STAT_RD05_L2CPU_LAST_STATUS] = priv->rd05_l2cpu_last_status;
	data[RTL_RTK_STAT_RD05_L2CPU_LAST_EIDX] = priv->rd05_l2cpu_last_eidx;
	data[RTL_RTK_STAT_RD05_L2CPU_LAST_W0] = priv->rd05_l2cpu_last_w0;
	data[RTL_RTK_STAT_RD05_L2CPU_LAST_W1] = priv->rd05_l2cpu_last_w1;
	data[RTL_RTK_STAT_RD05_L2CPU_LAST_SWTACR] = priv->rd05_l2cpu_last_swtacr;
	data[RTL_RTK_STAT_RD05_PIPE_RUNS] = priv->rd05_pipe_runs;
	data[RTL_RTK_STAT_RD05_PIPE_OK] = priv->rd05_pipe_ok;
	data[RTL_RTK_STAT_RD05_PIPE_FAIL] = priv->rd05_pipe_fail;
	data[RTL_RTK_STAT_RD05_PVID_OK] = priv->rd05_pvid_ok;
	data[RTL_RTK_STAT_RD05_NETIF_OK] = priv->rd05_netif_ok;
	data[RTL_RTK_STAT_RD05_TRAP_OK] = priv->rd05_trap_ok;
	data[RTL_RTK_STAT_RD05_PIPE_LAST_REG] = priv->rd05_pipe_last_reg;
	data[RTL_RTK_STAT_RD05_PIPE_LAST_VAL] = priv->rd05_pipe_last_val;
	data[RTL_RTK_STAT_RD05_PIPE_LAST_READ] = priv->rd05_pipe_last_read;
	data[RTL_RTK_STAT_RD05_ACL_RUNS] = priv->rd05_acl_runs;
	data[RTL_RTK_STAT_RD05_ACL_OK] = priv->rd05_acl_ok;
	data[RTL_RTK_STAT_RD05_ACL_FAIL] = priv->rd05_acl_fail;
	data[RTL_RTK_STAT_RD05_ACL_LAST_IDX] = priv->rd05_acl_last_idx;
	data[RTL_RTK_STAT_RD05_ACL_LAST_W7] = priv->rd05_acl_last_w7;
	data[RTL_RTK_STAT_RD05_ACL_LAST_STATUS] = priv->rd05_acl_last_status;
	data[RTL_RTK_STAT_RD05_FFCR_READ] = priv->rd05_ffcr_read;
	data[RTL_RTK_STAT_RD05_DACLRCR_READ] = priv->rd05_daclrcr_read;
	data[RTL_RTK_STAT_RD05_PTRAPCR_READ] = priv->rd05_ptrapcr_read;
	data[RTL_RTK_STAT_TC_SETUP_BLOCK_CALLS] = priv->tc_setup_block_calls;
	data[RTL_RTK_STAT_TC_SETUP_FT_CALLS] = priv->tc_setup_ft_calls;
	data[RTL_RTK_STAT_TC_CLS_FLOWER_REPLACE] = priv->tc_cls_flower_replace;
	data[RTL_RTK_STAT_TC_CLS_FLOWER_DESTROY] = priv->tc_cls_flower_destroy;
	data[RTL_RTK_STAT_TC_CLS_FLOWER_STATS] = priv->tc_cls_flower_stats;
	data[RTL_RTK_STAT_TC_UNSUPPORTED] = priv->tc_unsupported;
	data[RTL_RTK_STAT_TC_FT_BIND] = priv->tc_ft_bind;
	data[RTL_RTK_STAT_TC_FT_UNBIND] = priv->tc_ft_unbind;
	data[RTL_RTK_STAT_TC_ACL_DROP_SEEN] = priv->tc_acl_drop_seen;
	data[RTL_RTK_STAT_TC_ACL_TRAP_SEEN] = priv->tc_acl_trap_seen;
	data[RTL_RTK_STAT_DESC_STRIDE] = sizeof(struct rtl8197f_rtk_desc);
	data[RTL_RTK_STAT_TX_DESC0_OPTS1] = priv->tx_desc ? le32_to_cpu(priv->tx_desc[0].opts1) : 0;
	data[RTL_RTK_STAT_TX_DESC0_OPTS2] = priv->tx_desc ? le32_to_cpu(priv->tx_desc[0].opts2) : 0;
	data[RTL_RTK_STAT_TX_DESC0_OPTS4] = priv->tx_desc ? le32_to_cpu(priv->tx_desc[0].opts4) : 0;
	data[RTL_RTK_STAT_RX_DESC0_OPTS1] = priv->rx_desc ? le32_to_cpu(priv->rx_desc[0].opts1) : 0;
	data[RTL_RTK_STAT_RX_DESC0_OPTS2] = priv->rx_desc ? le32_to_cpu(priv->rx_desc[0].opts2) : 0;
	data[RTL_RTK_STAT_RX_DESC0_OPTS3] = priv->rx_desc ? le32_to_cpu(priv->rx_desc[0].opts3) : 0;
	data[RTL_RTK_STAT_RX_DESC0_OPTS4] = priv->rx_desc ? le32_to_cpu(priv->rx_desc[0].opts4) : 0;
	data[RTL_RTK_STAT_RX_DESC0_OPTS5] = priv->rx_desc ? le32_to_cpu(priv->rx_desc[0].opts5) : 0;
	data[RTL_RTK_STAT_LAST_CPUICR] = rtl8197f_rtk_read(priv, RTL_RTK_CPUICR);
	data[RTL_RTK_STAT_LAST_CPUICR1] = rtl8197f_rtk_read(priv, RTL_RTK_CPUICR1);
	data[RTL_RTK_STAT_LAST_TXRINGCR] = rtl8197f_rtk_read(priv, RTL_RTK_TXRINGCR);
	data[RTL_RTK_STAT_HW_RX_DESC_BASE] = rtl8197f_rtk_hw_dma_addr(priv, priv->rx_desc_dma);
	data[RTL_RTK_STAT_HW_TX_DESC_BASE] = rtl8197f_rtk_hw_dma_addr(priv, priv->tx_desc_dma);
	data[RTL_RTK_STAT_LAST_DMA_CR0] = rtl8197f_rtk_read(priv, RTL_RTK_DMA_CR0);
	data[RTL_RTK_STAT_LAST_CPUQDM0] = rtl8197f_rtk_read(priv, RTL_RTK_CPUQDM0);
	data[RTL_RTK_STAT_LAST_CPUQDM2] = rtl8197f_rtk_read(priv, RTL_RTK_CPUQDM2);
	data[RTL_RTK_STAT_LAST_CPUQDM4] = rtl8197f_rtk_read(priv, RTL_RTK_CPUQDM4);
	data[RTL_RTK_STAT_LAST_SIRR] = priv->swcore ? readl(priv->swcore + RTL_RTK_SWCORE_SWMISC_BASE + RTL_RTK_SWMISC_SSIR) : 0;
	data[RTL_RTK_STAT_LAST_MACCTRL1] = priv->swcore ? readl(priv->swcore + RTL_RTK_SWCORE_MACCTRL1) : 0;
	data[RTL_RTK_STAT_LAST_SYS_CLK_MAG] = priv->system ? readl(priv->system + RTL_RTK_SYSTEM_SYS_CLK_MAG) : 0;
	data[RTL_RTK_STAT_REG_CPURPDCR0] = rtl8197f_rtk_read(priv, RTL_RTK_CPURPDCR0);
	data[RTL_RTK_STAT_REG_CPURMDCR0] = rtl8197f_rtk_read(priv, RTL_RTK_CPURMDCR0);
	data[RTL_RTK_STAT_REG_CPUTPDCR0] = rtl8197f_rtk_read(priv, RTL_RTK_CPUTPDCR0);
	data[RTL_RTK_STAT_RD05_SDK_NEWDESC_RX] = priv->rd05_sdk_newdesc_rx;
	data[RTL_RTK_STAT_RD05_LEGACY_RX_ENABLED] = priv->rd05_legacy_rx_enabled;
	data[RTL_RTK_STAT_RD05_LEGACY_RX_INIT_OK] = priv->rd05_legacy_rx_init_ok;
	data[RTL_RTK_STAT_RD05_LEGACY_RX_PKTS] = priv->rd05_legacy_rx_pkts;
	data[RTL_RTK_STAT_RD05_LEGACY_RX_RECYCLES] = priv->rd05_legacy_rx_recycles;
	data[RTL_RTK_STAT_RD05_LEGACY_RX_BAD] = priv->rd05_legacy_rx_bad;
	data[RTL_RTK_STAT_RD05_LEGACY_RX_EMPTY] = priv->rd05_legacy_rx_empty;
	data[RTL_RTK_STAT_RD05_LEGACY_RX_LAST_ENT] = priv->rd05_legacy_rx_last_ent;
	data[RTL_RTK_STAT_RD05_LEGACY_RX_LAST_LEN] = priv->rd05_legacy_rx_last_len;
	data[RTL_RTK_STAT_RD05_LEGACY_RX_RING_BASE] = priv->rd05_legacy_rx_ring_base;
	data[RTL_RTK_STAT_RD05_LEGACY_MBUF_RING_BASE] = priv->rd05_legacy_mbuf_ring_base;
	data[RTL_RTK_STAT_REG_DMA_CR1] = rtl8197f_rtk_read(priv, RTL_RTK_DMA_CR1);
	data[RTL_RTK_STAT_REG_DMA_CR4] = rtl8197f_rtk_read(priv, RTL_RTK_DMA_CR4);
	data[RTL_RTK_STAT_REG_CPUIIMR] = rtl8197f_rtk_read(priv, RTL_RTK_CPUIIMR);
	data[RTL_RTK_STAT_REG_CPUIISR] = rtl8197f_rtk_read(priv, RTL_RTK_CPUIISR);
}

static const struct ethtool_ops rtl8197f_rtk_ethtool_ops = {
	.get_drvinfo		= rtl8197f_rtk_get_drvinfo,
	.get_msglevel		= rtl8197f_rtk_get_msglevel,
	.set_msglevel		= rtl8197f_rtk_set_msglevel,
	.get_link		= ethtool_op_get_link,
	.get_regs_len		= rtl8197f_rtk_get_regs_len,
	.get_regs		= rtl8197f_rtk_get_regs,
	.get_ringparam		= rtl8197f_rtk_get_ringparam,
	.set_ringparam		= rtl8197f_rtk_set_ringparam,
	.get_strings		= rtl8197f_rtk_get_strings,
	.get_sset_count		= rtl8197f_rtk_get_sset_count,
	.get_ethtool_stats	= rtl8197f_rtk_get_ethtool_stats,
};


/*
 * Mainstream translation entrypoint for legacy SDK swconfig/private-ioctl ACL
 * and FastPath/HWNAT requests.
 *
 * Linux 6.6 routes tc flower and nft flowtable hardware-offload requests to
 * ndo_setup_tc().  We accept the standard block binding so OpenWrt can use the
 * normal control plane.  Rule programming remains conservative and returns
 * -EOPNOTSUPP until a hardware table encoder is validated for RD05; this is
 * upstream-friendly because tc/nft receives an explicit failure/fallback instead
 * of silently relying on Realtek private ABI side effects.
 */
static int rtl8197f_rtk_setup_tc_block_cb(enum tc_setup_type type,
						  void *type_data, void *cb_priv)
{
	struct net_device *ndev = cb_priv;
	struct rtl8197f_rtknet_priv *priv = netdev_priv(ndev);

	switch (type) {
	case TC_SETUP_CLSFLOWER: {
		struct flow_cls_offload *cls = type_data;

		switch (cls->command) {
		case FLOW_CLS_REPLACE:
			priv->tc_cls_flower_replace++;
			/* The compatibility layer maps SDK ACL drop/trap/mirror requests
			 * to tc flower.  Until the RTL865x/RTL8367D ACL encoder is hardware
			 * validated, report explicit no-offload so callers can run software
			 * or retry without skip_sw.
			 */
			priv->tc_acl_drop_seen++;
			NL_SET_ERR_MSG_MOD(cls->common.extack,
					   "RTL8197F/RTL8367D tc flower reached driver; HW ACL programming is not enabled in mainline-safe mode");
			return -EOPNOTSUPP;
		case FLOW_CLS_DESTROY:
			priv->tc_cls_flower_destroy++;
			return 0;
		case FLOW_CLS_STATS:
			priv->tc_cls_flower_stats++;
			return 0;
		default:
			priv->tc_unsupported++;
			return -EOPNOTSUPP;
		}
	}
	default:
		priv->tc_unsupported++;
		return -EOPNOTSUPP;
	}
}

static LIST_HEAD(rtl8197f_rtk_tc_block_cb_list);

static int rtl8197f_rtk_setup_tc_block(struct net_device *ndev,
					       struct flow_block_offload *f)
{
	struct rtl8197f_rtknet_priv *priv = netdev_priv(ndev);
	int ret;

	priv->tc_setup_block_calls++;
	if (f->command == FLOW_BLOCK_BIND)
		priv->tc_ft_bind++;
	else if (f->command == FLOW_BLOCK_UNBIND)
		priv->tc_ft_unbind++;

	ret = flow_block_cb_setup_simple(f, &rtl8197f_rtk_tc_block_cb_list,
					 rtl8197f_rtk_setup_tc_block_cb,
					 ndev, ndev, true);
	if (ret)
		priv->tc_unsupported++;

	return ret;
}

static int rtl8197f_rtk_setup_tc(struct net_device *ndev,
					 enum tc_setup_type type, void *type_data)
{
	struct rtl8197f_rtknet_priv *priv = netdev_priv(ndev);

	switch (type) {
	case TC_SETUP_BLOCK:
		return rtl8197f_rtk_setup_tc_block(ndev, type_data);
	case TC_SETUP_FT:
		priv->tc_setup_ft_calls++;
		return rtl8197f_rtk_setup_tc_block(ndev, type_data);
	default:
		priv->tc_unsupported++;
		return -EOPNOTSUPP;
	}
}

static int rtl8197f_rtk_rd05_reseed_cmd(struct rtl8197f_rtknet_priv *priv,
					 const char *origin, int cmd)
{
	if (!of_machine_is_compatible("xiaomi,r4-rd05"))
		return -EOPNOTSUPP;

	switch (cmd) {
	case RTL_RTK_RD05_PRIV_RESEED:
	case RTL_RTK_RD05_PRIV_RXSTART:
	case RTL_RTK_RD05_PRIV_VENDOR_SIDE:
		break;
	default:
		return -EOPNOTSUPP;
	}

	priv->rd05_private_ioctl_runs++;

	if (cmd == RTL_RTK_RD05_PRIV_RESEED ||
	    cmd == RTL_RTK_RD05_PRIV_VENDOR_SIDE)
		rtl8197f_rtk_rd05_vendor_sidebands(priv, origin);

	if (cmd == RTL_RTK_RD05_PRIV_RESEED) {
		if (priv->rd05_sdk_newdesc_rx)
			rtl8197f_rtk_rd05_program_newdesc_rx(priv, origin);
		else
			rtl8197f_rtk_rd05_program_legacy_rx(priv, origin);
		if (rtl8197f_rtk_rd05_wait_swcore_ready(priv)) {
			rtl8197f_rtk_rd05_seed_l2cpu(priv);
			rtl8197f_rtk_rd05_seed_pipeline(priv);
		}
	}

	if (cmd == RTL_RTK_RD05_PRIV_RXSTART) {
		if (priv->rd05_sdk_newdesc_rx)
			rtl8197f_rtk_rd05_program_newdesc_rx(priv, origin);
		else
			rtl8197f_rtk_rd05_program_legacy_rx(priv, origin);
	}

	rtl8197f_rtk_apply_sdk_cpuif_init(priv);
	rtl8197f_rtk_rd05_poststart_rearm(priv, origin);
	dev_info(priv->dev,
		 "rd05 private ioctl/proc v38.2: origin=%s cmd=0x%x runs=%llu cpuicr=0x%08x cpuicr1=0x%08x dma_cr0=0x%08x cpurpdcr0=0x%08x cpurmdcr0=0x%08x sirr=0x%08x newdesc_rx=%d legacy_rx=%d\n",
		 origin, cmd, (unsigned long long)priv->rd05_private_ioctl_runs,
		 rtl8197f_rtk_read(priv, RTL_RTK_CPUICR),
		 rtl8197f_rtk_read(priv, RTL_RTK_CPUICR1),
		 rtl8197f_rtk_read(priv, RTL_RTK_DMA_CR0),
		 rtl8197f_rtk_read(priv, RTL_RTK_CPURPDCR0),
		 rtl8197f_rtk_read(priv, RTL_RTK_CPURMDCR0),
		 priv->swcore ? readl(priv->swcore + RTL_RTK_SWCORE_SWMISC_BASE + RTL_RTK_SWMISC_SSIR) : 0,
		 priv->rd05_sdk_newdesc_rx,
		 priv->rd05_legacy_rx_enabled);

	return 0;
}

static int rtl8197f_rtk_rd05_cmd_from_name(const char *buf)
{
	if (!strcmp(buf, "reseed"))
		return RTL_RTK_RD05_PRIV_RESEED;
	if (!strcmp(buf, "rxstart"))
		return RTL_RTK_RD05_PRIV_RXSTART;
	if (!strcmp(buf, "vendor-side"))
		return RTL_RTK_RD05_PRIV_VENDOR_SIDE;
	return -EOPNOTSUPP;
}

static ssize_t rtl8197f_rtk_rd05_proc_write(struct file *file,
					    const char __user *ubuf,
					    size_t len, loff_t *ppos)
{
	struct net_device *ndev = pde_data(file_inode(file));
	struct rtl8197f_rtknet_priv *priv;
	char buf[32];
	int cmd, ret;

	if (!ndev)
		return -ENODEV;
	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	if (len >= sizeof(buf))
		return -EINVAL;
	if (copy_from_user(buf, ubuf, len))
		return -EFAULT;
	buf[len] = '\0';
	strim(buf);

	cmd = rtl8197f_rtk_rd05_cmd_from_name(buf);
	if (cmd < 0)
		return cmd;

	priv = netdev_priv(ndev);
	ret = rtl8197f_rtk_rd05_reseed_cmd(priv, "procfs", cmd);
	return ret ? ret : len;
}

static const struct proc_ops rtl8197f_rtk_rd05_proc_ops = {
	.proc_write = rtl8197f_rtk_rd05_proc_write,
};

static void rtl8197f_rtk_rd05_reseed_work(struct work_struct *work)
{
	struct rtl8197f_rtknet_priv *priv =
		container_of(to_delayed_work(work), struct rtl8197f_rtknet_priv,
			     rd05_reseed_work);

	if (!netif_running(priv->ndev))
		return;

	rtl8197f_rtk_rd05_reseed_cmd(priv, "delayed-linkup", RTL_RTK_RD05_PRIV_RESEED);
}

static int rtl8197f_rtk_do_ioctl(struct net_device *ndev, struct ifreq *ifr,
				 int cmd)
{
	struct rtl8197f_rtknet_priv *priv = netdev_priv(ndev);

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	return rtl8197f_rtk_rd05_reseed_cmd(priv, "private-ioctl", cmd);
}

static const struct net_device_ops rtl8197f_rtk_netdev_ops = {
	.ndo_open		= rtl8197f_rtk_open,
	.ndo_stop		= rtl8197f_rtk_stop,
	.ndo_start_xmit		= rtl8197f_rtk_start_xmit,
	.ndo_tx_timeout		= rtl8197f_rtk_tx_timeout,
	.ndo_change_mtu		= rtl8197f_rtk_change_mtu,
	.ndo_set_mac_address	= eth_mac_addr,
	.ndo_validate_addr	= eth_validate_addr,
	.ndo_do_ioctl		= rtl8197f_rtk_do_ioctl,
	.ndo_setup_tc		= rtl8197f_rtk_setup_tc,
};

static void rtl8197f_rtk_read_u32_prop(struct device_node *np,
				       const char *name, u32 *val, u32 min,
				       u32 max)
{
	u32 tmp;

	if (!of_property_read_u32(np, name, &tmp))
		*val = clamp_t(u32, tmp, min, max);
}

static int rtl8197f_rtk_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct rtl8197f_rtknet_priv *priv;
	struct net_device *ndev;
	int ret;

	dev_info(&pdev->dev, "rd05 bootstage v38.2: rtknet probe enter\n");

	ndev = alloc_etherdev(sizeof(*priv));
	if (!ndev)
		return -ENOMEM;

	SET_NETDEV_DEV(ndev, &pdev->dev);
	platform_set_drvdata(pdev, ndev);
	priv = netdev_priv(ndev);
	BUILD_BUG_ON(sizeof(struct rtl8197f_rtk_desc) != 32);
	BUILD_BUG_ON(sizeof(struct rtl8197f_rtk_rd05_pkthdr) != 32);
	BUILD_BUG_ON(sizeof(struct rtl8197f_rtk_rd05_mbuf) != 32);
	priv->dev = &pdev->dev;
	priv->ndev = ndev;
	priv->rx_ring_size = RTL_RTK_DEFAULT_RX_RING;
	priv->tx_ring_size = RTL_RTK_DEFAULT_TX_RING;
	priv->rx_buf_size = RTL_RTK_DEFAULT_RX_BUFSZ;
	priv->tx_port_mask = RTL_RTK_DEFAULT_TX_PORT_MASK;
	priv->tx_dp_ext = RTL_RTK_DEFAULT_TX_DP_EXT;
	priv->tx_extspa = RTL_RTK_DEFAULT_TX_EXTSPA;
	priv->rx_poll_ms = RTL_RTK_DEFAULT_RX_POLL_MS;
	priv->msg_enable = NETIF_MSG_DRV | NETIF_MSG_PROBE | NETIF_MSG_LINK;

	dev_info(&pdev->dev, "rd05 bootstage v38.2: map cpuif\n");
	priv->base = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(priv->base)) {
		ret = PTR_ERR(priv->base);
		goto err_free_netdev;
	}

	priv->swcore = devm_platform_ioremap_resource_byname(pdev, "swcore");
	if (IS_ERR(priv->swcore)) {
		/* Backward-compatible fallback for older DTBs which mapped only SWMISC.
		 * v13 DTBs should provide the whole SWCORE window at child 0x03800000.
		 */
		void __iomem *swmisc;

		swmisc = devm_platform_ioremap_resource_byname(pdev, "swmisc");
		priv->swcore = IS_ERR(swmisc) ? NULL :
			(void __iomem *)((char __iomem *)swmisc -
					 RTL_RTK_SWCORE_SWMISC_BASE);
	}

	priv->system = rtl8197f_rtk_ioremap_norequest(pdev, "system");
	if (of_machine_is_compatible("xiaomi,r4-rd05"))
		priv->acl0 = devm_ioremap(&pdev->dev, RTL_RTK_ACL_TABLE_ENTRY0_PHYS,
					    sizeof(u32));

	dev_info(&pdev->dev, "rd05 bootstage v38.2: request DT irq mapping\n");
	priv->irq = platform_get_irq(pdev, 0);
	dev_info(&pdev->dev, "rd05 bootstage v38.2: DT irq mapped=%d\n", priv->irq);
	if (priv->irq < 0) {
		ret = priv->irq;
		goto err_free_netdev;
	}

	if (np) {
		rtl8197f_rtk_read_u32_prop(np, "realtek,rx-ring-size",
					       &priv->rx_ring_size,
					       RTL_RTK_MIN_RING, RTL_RTK_MAX_RING);
		rtl8197f_rtk_read_u32_prop(np, "realtek,tx-ring-size",
					       &priv->tx_ring_size,
					       RTL_RTK_MIN_RING, RTL_RTK_MAX_RING);
		rtl8197f_rtk_read_u32_prop(np, "realtek,rx-buffer-size",
					       &priv->rx_buf_size, 1536, 16384);
		rtl8197f_rtk_read_u32_prop(np, "realtek,tx-port-mask",
					       &priv->tx_port_mask, 1, RTL_RTK_TX_DP_MASK);
		rtl8197f_rtk_read_u32_prop(np, "realtek,tx-dp-ext",
					       &priv->tx_dp_ext, 0, RTL_RTK_TX_DP_EXT_MASK);
		rtl8197f_rtk_read_u32_prop(np, "realtek,tx-extspa",
					       &priv->tx_extspa, 0, RTL_RTK_TX_EXTSPA_MASK);
		priv->tx_hwlookup = of_property_read_bool(np, "realtek,tx-hwlookup");
		priv->tx_bridge = of_property_read_bool(np, "realtek,tx-bridge");
		priv->desc_addr_kseg1 = of_property_read_bool(np,
						"realtek,desc-addr-kseg1");
		priv->desc_addr_physical = of_property_read_bool(np,
						"realtek,desc-addr-physical");
		if (priv->desc_addr_physical)
			priv->desc_addr_kseg1 = false;
		priv->rx_poll_fallback = of_property_read_bool(np,
						"realtek,rx-poll-fallback");
		priv->sdk_crc_lengths = of_property_read_bool(np,
						"realtek,sdk-crc-lengths");
		priv->legacy_rd05_pipeline = of_property_read_bool(np,
						"realtek,legacy-rd05-sdk-pipeline");
		priv->p0_cpu_tag_passthrough = of_property_read_bool(np,
						"realtek,p0-cpu-tag-pass-through");
		rtl8197f_rtk_read_u32_prop(np, "realtek,rx-poll-ms",
					       &priv->rx_poll_ms, 10, 1000);
	}

	if (of_machine_is_compatible("xiaomi,r4-rd05")) {
		priv->rx_poll_fallback = true;
		priv->sdk_trxrdy = true;
		priv->sdk_cpuicr1_init = true;
		priv->sdk_swcore_init = true;
		priv->sdk_crc_lengths = true;
		priv->rd05_sdk_newdesc_rx = true;
		priv->p0_cpu_tag_passthrough = true;
		/* RD05 has two cascaded switching domains: the external RTL8367D
		 * DSA switch and the RTL8197F internal rtl865x/SWCORE ingress
		 * domain. The RTL8367D MIB now proves frames are emitted on
		 * EXT1/CPU7, while eth0 RX remains zero if the SoC-side rtl865x
		 * VID9/netif/L2/ACL CPU pipeline is not seeded. Enable that
		 * GPL-SDK-derived internal ingress translation by default for this
		 * board. v23 deliberately uses SDK/private-style control paths where
		 * needed for RD05 instead of a mainline-only subset.
		 */
		priv->legacy_rd05_pipeline = true;
	}

	if (np) {
		if (of_property_read_bool(np, "realtek,sdk-trxrdy"))
			priv->sdk_trxrdy = true;
		if (of_property_read_bool(np, "realtek,sdk-cpuicr1-init"))
			priv->sdk_cpuicr1_init = true;
		if (of_property_read_bool(np, "realtek,sdk-swcore-init"))
			priv->sdk_swcore_init = true;
	}

	if (of_machine_is_compatible("xiaomi,r4-rd05")) {
		/* GPL SDK topology for RTL8197F + RTL83xx: physical P0/RGMII is the
		 * SoC-facing cascade link.  RTL8367D CPU port 7/EXT1 is the opposite
		 * endpoint, not RTL8197F internal extension port 7.  Direct TX to P0;
		 * DSA supplies the rtl8_4 tag in the frame itself.
		 */
		priv->tx_hwlookup = false;
		priv->tx_bridge = false;
		priv->tx_port_mask = BIT(0);
		priv->tx_dp_ext = 0;
		priv->tx_extspa = 0;
		dev_info(&pdev->dev,
			 "RD05 DSA master v38.2: RTL8197F P0/RGMII <-> RTL8367D CPU7/EXT1, RX CPU-tag recognition with tag preservation, direct DP=0x1, stock rtl8_4 tags\n");
	} else if ((priv->tx_port_mask & BIT(6)) && !priv->tx_dp_ext) {
		dev_warn(&pdev->dev,
			 "tx-port-mask targets rtl865x extension/CPU path but tx-dp-ext is 0; external traffic may not pass\n");
	}

	/* v38.1 hardware stopped before the first rtknet marker after MTD probe.
	 * Keep probe side-effect free and replay P0/RGMII from ndo_open(), the
	 * sequence already proven to boot in v37.  DSA can register with the fixed
	 * CPU link before the conduit is opened.
	 */
	dev_info(&pdev->dev, "rd05 bootstage v38.2: defer P0/RGMII programming to ndo_open\n");

	ndev->netdev_ops = &rtl8197f_rtk_netdev_ops;
	ndev->ethtool_ops = &rtl8197f_rtk_ethtool_ops;
	ndev->watchdog_timeo = msecs_to_jiffies(5000);
	ndev->min_mtu = ETH_MIN_MTU;
	ndev->max_mtu = priv->rx_buf_size - ETH_HLEN - VLAN_HLEN;

	if (!np || of_get_ethdev_address(np, ndev))
		eth_hw_addr_random(ndev);

	netif_carrier_off(ndev);
	spin_lock_init(&priv->tx_lock);
	timer_setup(&priv->rx_poll_timer, rtl8197f_rtk_rx_poll_timer, 0);
	INIT_DELAYED_WORK(&priv->rd05_reseed_work, rtl8197f_rtk_rd05_reseed_work);
	netif_napi_add(ndev, &priv->napi, rtl8197f_rtk_poll);

	ret = register_netdev(ndev);
	if (ret)
		goto err_del_napi;

	if (of_machine_is_compatible("xiaomi,r4-rd05")) {
		priv->rd05_proc = proc_create_data("rd05-rtknet", 0200, NULL,
						      &rtl8197f_rtk_rd05_proc_ops, ndev);
		if (!priv->rd05_proc)
			dev_warn(&pdev->dev, "rd05 procfs trigger /proc/rd05-rtknet not available\n");
	}

	dev_info(&pdev->dev,
		 "registered %s: rx=%u tx=%u buf=%u desc-stride=%zu tx-port-mask=0x%x tx-dp-ext=0x%x tx-extspa=0x%x tx-hwlookup=%d tx-bridge=%d desc-kseg1=%d desc-phys=%d rx-poll=%d/%ums sdk-cpuif=%d sdk-swcore=%d legacy-pipeline=%d trxrdy=%d sdk-crc=%d rd05-legacy-rxphys=%d\n",
		 ndev->name, priv->rx_ring_size, priv->tx_ring_size,
		 priv->rx_buf_size, sizeof(struct rtl8197f_rtk_desc),
		 priv->tx_port_mask, priv->tx_dp_ext, priv->tx_extspa,
		 priv->tx_hwlookup, priv->tx_bridge, priv->desc_addr_kseg1,
		 priv->desc_addr_physical, priv->rx_poll_fallback, priv->rx_poll_ms,
		 priv->sdk_cpuicr1_init, priv->sdk_swcore_init,
		 priv->legacy_rd05_pipeline, priv->sdk_trxrdy,
		 priv->sdk_crc_lengths,
		 !!of_machine_is_compatible("xiaomi,r4-rd05"));
	return 0;

err_del_napi:
	netif_napi_del(&priv->napi);
err_free_netdev:
	free_netdev(ndev);
	return ret;
}

static void rtl8197f_rtk_remove(struct platform_device *pdev)
{
	struct net_device *ndev = platform_get_drvdata(pdev);
	struct rtl8197f_rtknet_priv *priv = netdev_priv(ndev);

	cancel_delayed_work_sync(&priv->rd05_reseed_work);
	if (priv->rd05_proc)
		proc_remove(priv->rd05_proc);
	unregister_netdev(ndev);
	netif_napi_del(&priv->napi);
	free_netdev(ndev);
}

static const struct of_device_id rtl8197f_rtk_of_match[] = {
	{ .compatible = "realtek,rtl8197d-rtknet" },
	{ .compatible = "realtek,rtl8197f-rtknet" },
	{ .compatible = "realtek,rtl8197fh-rtknet" },
	{ .compatible = "realtek,rtl8197g-rtknet" },
	{ .compatible = "realtek,rtl865x-cpuif" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, rtl8197f_rtk_of_match);

static struct platform_driver rtl8197f_rtk_driver = {
	.probe = rtl8197f_rtk_probe,
	.remove_new = rtl8197f_rtk_remove,
	.driver = {
		.name = DRV_NAME,
		.of_match_table = rtl8197f_rtk_of_match,
	},
};
module_platform_driver(rtl8197f_rtk_driver);

MODULE_AUTHOR("OpenWrt RTL8197F porting work");
MODULE_DESCRIPTION("Native RTL8197F/RTL8197FH rtl865x CPU-interface Ethernet driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);
