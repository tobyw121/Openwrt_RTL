// SPDX-License-Identifier: GPL-2.0-only
/*
 * Native Linux 6.6 Ethernet CPU-interface driver for Realtek RTL8197F/RTL8197FH rtl865x.
 *
 * This is a clean net_device/NAPI port built from the RTL8197F/rtl865x SDK
 * register and descriptor definitions.  It intentionally does not import the
 * old Realtek rtknet forwarding/NAT/fastpath stack as private kernel ABI.
 * VLAN and external RTL8367 switching are handled by DSA/bridge, software
 * fastpath by nftables flowtables and hardware ACL/NAT integration by tc/flowtable
 * callbacks rather than Realtek private ioctls.
 */

#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/capability.h>
#include <linux/dma-mapping.h>
#include <linux/dmapool.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/interrupt.h>
#include <linux/atomic.h>
#include <linux/if_vlan.h>
#include <linux/hash.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/rcupdate.h>
#include <linux/rtnetlink.h>
#include <linux/iopoll.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/mutex.h>
#include <linux/phy.h>
#include <linux/of_mdio.h>
#include <linux/nvmem-consumer.h>
#include <linux/sockios.h>
#include <linux/string.h>
#include <linux/of.h>
#include <linux/of_net.h>
#include <linux/platform_device.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/skbuff.h>
#include <linux/mm.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/uaccess.h>
#include <linux/workqueue.h>
#include <linux/if_ether.h>
#include <asm/addrspace.h>
#include <asm/unaligned.h>
#include <linux/timer.h>
#include <linux/netlink.h>
#include <net/ip.h>
#include <net/tcp.h>
#include <net/udp.h>
#include <net/checksum.h>
#include <net/flow_offload.h>
#include <net/pkt_cls.h>
#include <net/page_pool/helpers.h>

#define DRV_NAME			"rtl8197f-rtknet"
#define DRV_VERSION			"1.8.34-mw5-oem-v212-v44.66.12"
#define MW5_ACCEL_SW_HASH_BITS		8
#define MW5_ACCEL_SW_HASH_SIZE		(1U << MW5_ACCEL_SW_HASH_BITS)

/* v44.65.21: keep Linux software IRQ coalescing disabled by default on MW5.
 * Hardware testing of v44.65.20 showed malformed SPA0/random-EtherType RX
 * frames soon after boot with gro_flush_timeout/napi_defer_hard_irqs enabled.
 * The OWN-based NAPI completion fix and 1-second lost-IRQ watchdog remain.
 */
#define RTL_RTK_MW5_GRO_FLUSH_NS	0UL
#define RTL_RTK_MW5_NAPI_DEFER_HARD_IRQS	0
#define RTL_RTK_MW5_RX_WATCHDOG_MS	1000U
#define RTL_RTK_MW5_RX_RUNOUT_RECHECK_MS	10U
#define RTL_RTK_MW5_RX_PUBLISH_RECHECK_MS	1U
#define RTL_RTK_MW5_RX_RUNOUT_GUARD_MS	500U
#define RTL_RTK_MW5_RX_CORRUPT_WINDOW_MS	2000U
#define RTL_RTK_MW5_RX_CORRUPT_WINDOW_LIMIT	3U
#define RTL_RTK_MW5_NAPI_WEIGHT		128

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
#define RTL_RTK_IE_MBUF_RUNOUT0		BIT(11)
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
#define RTL_RTK_INT_STATUS_MASK	(RTL_RTK_INT_RX_MASK | \
					 RTL_RTK_INT_TX_MASK | \
					 RTL_RTK_INT_LINK_CHANGE)
/* CPUIIMR and CPUIISR are almost, but not completely, bit-identical.
 * The RTL8197F SDK defines MBUF_DESC_RUNOUT_IE0 at bit 11 while the
 * corresponding CPUIISR pending bit is 16.  v44.65.12 reused the status
 * bit as the mask bit and therefore never enabled the real mbuf-runout IRQ. */
#define RTL_RTK_INT_ENABLE_MASK	((RTL_RTK_INT_STATUS_MASK & \
					 ~RTL_RTK_INT_MBUF_RUNOUT0) | \
					 RTL_RTK_IE_MBUF_RUNOUT0)

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
#define RTL_RTK_P0GMIICR_RX_DELAY_FS	5U
#define RTL_RTK_P0GMIICR_CONF_DONE	BIT(6)

/* RTL865x internal Clause-22 MDIO block (SWMACCR_BASE + 0x4/0x8). */
#define RTL_RTK_SWCORE_MDCIOCR		0x4004
#define RTL_RTK_SWCORE_MDCIOSR		0x4008
#define RTL_RTK_MDIO_CMD_WRITE		BIT(31)
#define RTL_RTK_MDIO_PHY_SHIFT		24
#define RTL_RTK_MDIO_REG_SHIFT		16
#define RTL_RTK_MDIO_BUSY		BIT(31)
#define RTL_RTK_MDIO_READ_ERROR	BIT(30)
#define RTL_RTK_IBALL_RTL8211F_PHY	6U
#define RTL_RTK_P0GMIICR_RX_DELAY_VG	6U

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
#define RTL_RTK_ASICTBL_TYPE_ARP		1
#define RTL_RTK_ASICTBL_TYPE_ROUTE	2
#define RTL_RTK_ASICTBL_TYPE_NETIF	4
#define RTL_RTK_ASICTBL_TYPE_EXT_INT_IP	5
#define RTL_RTK_ASICTBL_TYPE_VLAN	6
#define RTL_RTK_ASICTBL_TYPE_L4_TCPUDP	9
#define RTL_RTK_ASICTBL_TYPE_ACL		12
#define RTL_RTK_ASICTBL_TYPE_NEXT_HOP	13
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
/* RTL865x Output Queue Number Control block (SWCORE_BASE + 0x4700). */
#define RTL_RTK_SWCORE_UPTCMCR2		0x4720 /* priority->QID map for 3 queues */
#define RTL_RTK_SWCORE_QNUMCR		0x4754
#define RTL_RTK_QNUM_P0_MASK		GENMASK(2, 0)
#define RTL_RTK_QNUM_3			2 /* 8197F encoding is queue-count minus one */
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
#define RTL_RTK_MSCR_EN_L4		BIT(2)
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
#define RTL_RTK_IBALL_LAN_VID_DEFAULT	9U
#define RTL_RTK_IBALL_WAN_VID_DEFAULT	8U
#define RTL_RTK_IBALL_LAN_MASK_DEFAULT	0x1eU
#define RTL_RTK_IBALL_WAN_MASK_DEFAULT	0x01U
#define RTL_RTK_NAPT_TABLE_SIZE		1024U
#define RTL_RTK_EXTIP_TABLE_SIZE	16U
#define RTL_RTK_ARP_TABLE_SIZE		512U
#define RTL_RTK_ROUTE_TABLE_SIZE	8U
#define RTL_RTK_NEXT_HOP_TABLE_SIZE	32U
#define RTL_RTK_NAPT_TCP_AGE_SEC	120U
#define RTL_RTK_NAPT_UDP_AGE_SEC	90U
#define RTL_RTK_NAPT_HASH_FOR_VERI	2U


/* RTL8197F New_swNic six-DWORD TX descriptor (24 bytes).
 *
 * The names below follow the vendor GPL SDK layout instead of treating the
 * OEM words as anonymous/reserved data:
 *   DW0/opts1: type[31:29], vi[28], li[27], pi[26], pppidx[25:23],
 *              ph_len[22:6], bridge[5], hwlookup[4], fs[3], ls[2],
 *              eor[1], own[0]
 *   DW1:       buffer address
 *   DW2/opts2: m_len[31:15], qid[14:12], pqid[11:9], vlantagset[8:0]
 *   DW3/opts3: ptp_pkt[31], ptp_typ[30:27], ptp_ver[26:25], dpri[24:22],
 *              po[21], l3cs[20], l4cs[19], ipv6[18], ipv4[17],
 *              ipv4_1st[16], reserved[15], dp_ext[14:12], dvid[11:0]
 *   DW4/opts4: lso[31], dp[30:24], reserved[23], linked[22:16],
 *              ipv6_hdrlen[15:0]
 *   DW5/opts5: extspa[31:30], mss[29:16], ipv4_hlen[7:4], tcp_hlen[3:0]
 *
 * V212 captures give DVID=9+DP=BIT(1) and DVID=8+DP=BIT(3) inside the OEM
 * RTL865x dataplane.  The v44.66.4 hardware trace proves those DP bits cannot
 * be used as external RTL8367 user-port selectors in Linux DSA: this board's
 * only physical SoC-to-switch conduit is internal P0/RGMII.  Normal DSA TX
 * therefore uses DP=P0 and carries the external target in the RTL8367/RTL8365MB eight-byte protocol-4
 * CPU tag; the V212 P1/P3 descriptor form remains diagnostic evidence only.
 */
#define RTL_RTK_DESC_OWN		BIT(0)
#define RTL_RTK_DESC_WRAP		BIT(1)
#define RTL_RTK_DESC_LAST		BIT(2)
#define RTL_RTK_DESC_FIRST		BIT(3)
#define RTL_RTK_TX_HWLKUP		BIT(4)
#define RTL_RTK_TX_BRIDGE		BIT(5)
#define RTL_RTK_RX_EXTSIZE_SHIFT		16
/* RTL8197F New_swNic RX descriptor opts2/opts4 fields (OEM SDK). */
#define RTL_RTK_RX_LEN_MASK		GENMASK(13, 0)
#define RTL_RTK_RX_QID_SHIFT		17
#define RTL_RTK_RX_QID_MASK		GENMASK(19, 17)
#define RTL_RTK_RX_DP_EXT_SHIFT		20
#define RTL_RTK_RX_DP_EXT_MASK		GENMASK(23, 20)
#define RTL_RTK_RX_EXTSPA_SHIFT		24
#define RTL_RTK_RX_EXTSPA_MASK		GENMASK(25, 24)
#define RTL_RTK_RX_SPA_SHIFT		13
#define RTL_RTK_RX_SPA_MASK		GENMASK(15, 13)

/* MW5/RTL8197FS receive metadata observed on real hardware.  P0 may consume
 * the external RTL8367 CPU tag before the DMA buffer while preserving
 * the physical switch source port in opts4[15:13].  Linux DSA needs the tag
 * at the master boundary, so v43.8 uses this descriptor field only as a
 * fallback when the 0x8899 header is absent from the received frame.
 */
#define RTL_RTK_MW5_RX_SPA_SHIFT	13
#define RTL_RTK_MW5_RX_SPA_MASK	0x7
#define RTL_RTK_MW5_CPU_TAG4_LEN	4
#define RTL_RTK_MW5_CPU_TAG8_LEN	8
#define RTL_RTK_MW5_CPU_TAG_LEN	RTL_RTK_MW5_CPU_TAG4_LEN
#define RTL_RTK_MW5_RTL8_PROTO_WORD	0x0400
#define RTL_RTK_MW5_RTL8_PORT_MASK	0x07ff
#define RTL_RTK_MW5_RX_TAG_MARKER	0x0400
#define RTL_RTK_MW5_RX_TAG_PORT_MASK	0x003f
#define RTL_RTK_MW5_TX_NATIVE_MARKER	0x0400
#define RTL_RTK_MW5_TX_NATIVE_MASK	0xffc0
#define RTL_RTK_MW5_TX_PROTO9_MASK	0xf000
#define RTL_RTK_MW5_TX_PROTO9_MARKER	0x9000
/* Software-only DSA egress metadata. tag_rtl4_9 layout 3 inserts this marker
 * for diagnostics of the OEM descriptor form.  v44.66.4 proved that directly
 * targeting RTL8197F P1/P3 disconnects the external-switch conduit, so normal
 * MW5 operation uses an on-wire RTL8367 0x8899/0x0400 tag with DP=P0 instead. */
#define RTL_RTK_MW5_TX_META_MAGIC	0xd000
#define RTL_RTK_MW5_TX_META_MAGIC_MASK	0xff00
#define RTL_RTK_MW5_TX_META_PORT_MASK	0x003f
#define RTL_RTK_MW5_LAN_PORT		1
#define RTL_RTK_MW5_WAN_PORT		3
#define RTL_RTK_MW5_CPU_EXT_PORT	8
#define RTL_RTK_MW5_LAN_VID		9
#define RTL_RTK_MW5_WAN_VID		8
#define RTL_RTK_TX_PH_LEN_SHIFT		6
#define RTL_RTK_TX_PH_LEN_MASK		GENMASK(22, 6)
#define RTL_RTK_TX_M_LEN_SHIFT		15
#define RTL_RTK_TX_M_LEN_MASK		GENMASK(31, 15)
#define RTL_RTK_TX_DVID_MASK		GENMASK(11, 0)
#define RTL_RTK_TX_DP_EXT_SHIFT		12
#define RTL_RTK_TX_DP_EXT_MASK		0x7
#define RTL_RTK_TX_DP_SHIFT		24
#define RTL_RTK_TX_DP_MASK		0x7f
#define RTL_RTK_TX_EXTSPA_SHIFT		30
#define RTL_RTK_TX_EXTSPA_MASK		0x3
/* RTL8197F new-descriptor checksum/QoS/LSO fields from the GPL SDK. */
#define RTL_RTK_TX_QID_SHIFT		12
#define RTL_RTK_TX_PQID_SHIFT		9
#define RTL_RTK_TX_DPRI_SHIFT		22
#define RTL_RTK_TX_L3CS		BIT(20)
#define RTL_RTK_TX_L4CS		BIT(19)
#define RTL_RTK_TX_IPV6		BIT(18)
#define RTL_RTK_TX_IPV4		BIT(17)
#define RTL_RTK_TX_IPV4_1ST		BIT(16)
#define RTL_RTK_TX_LSO			BIT(31)
#define RTL_RTK_TX_TYPE_SHIFT		29
#define RTL_RTK_TX_TYPE_MASK		GENMASK(31, 29)
#define RTL_RTK_TX_TYPE_TCP		(5U << RTL_RTK_TX_TYPE_SHIFT)
#define RTL_RTK_TX_MSS_SHIFT		16
#define RTL_RTK_TX_IPV4_HLEN_SHIFT	4
#define RTL_RTK_RX_IPV4		BIT(8)
#define RTL_RTK_RX_IPV6		BIT(9)
#define RTL_RTK_RX_FRAG		BIT(11)
#define RTL_RTK_RX_L3CSOK		BIT(31)
#define RTL_RTK_RX_L4CSOK		BIT(30)

#define RTL_RTK_DEFAULT_RX_RING		128
#define RTL_RTK_DEFAULT_TX_RING		128
#define RTL_RTK_MIN_RING		16
#define RTL_RTK_MAX_RX_RING		1024
#define RTL_RTK_MAX_TX_RING		1024
#define RTL_RTK_MW5_OEM_RX_RING0	900
#define RTL_RTK_MW5_OEM_TX_RING0	768
/* V212 also exposes five RX and three TX auxiliary rings with two entries
 * each.  They are evidence for the OEM queue geometry, not a reason to enable
 * unverified Linux queues. Keep them diagnostic-only until their register and
 * interrupt semantics have been reconstructed independently. */
#define RTL_RTK_MW5_OEM_RX_AUX_RINGS	5
#define RTL_RTK_MW5_OEM_TX_AUX_RINGS	3
#define RTL_RTK_MW5_OEM_AUX_RING_SIZE	2
#define RTL_RTK_MW5_OEM_FREE_SKB_THRESHOLD	128
#define RTL_RTK_MW5_OEM_FREE_SKB_SNAPSHOT	789
#define RTL_RTK_DEFAULT_RX_BUFSZ		2048
#define RTL_RTK_DEFAULT_TX_PORT_MASK	0x7f
#define RTL_RTK_DEFAULT_TX_DP_EXT	0
#define RTL_RTK_DEFAULT_TX_EXTSPA	0
#define RTL_RTK_DEFAULT_RX_POLL_MS	100
#define RTL_RTK_REG_DUMP_LEN		(RTL_RTK_CPUICR1 + sizeof(u32))

/* MW5 classic-routing stability policy (v44.65.14).
 *
 * v44.65.5-.11 progressively ruled out flowtable, optional offloads, TX-ring
 * exhaustion and RTL8367 discards.  The remaining speed-test failure is a burst
 * of SPA0 completions with random-looking payload headers on non-coherent MIPS.
 *
 * V212 OEM captures replace the old 128/128 bring-up geometry with the observed
 * ring-0 sizes (RX=900, TX=768). NAPI/BQL remain Linux-owned and RX/TX payload
 * lifetime is isolated from reusable skb storage through fixed coherent slots.
 * v44.65.13 proved that even stateless TX checksum can break the MW5 raw-DSA
 * control plane although it does not bypass firewall policy. v44.65.17 retires
 * those independent MW5 acceleration controls in favor of one owner. */
static bool mw5_rx_page_pool;
module_param(mw5_rx_page_pool, bool, 0444);
MODULE_PARM_DESC(mw5_rx_page_pool,
		 "MW5 page-pool RX recycling (default off on non-coherent MIPS; diagnostic opt-in)");
/* v44.65.12: the stable MW5 RX backend uses a permanently mapped coherent
 * DMA bounce ring. The RTL8197FS CPU-DMA engine never owns a Linux skb and
 * Linux never reads a streaming-mapped skb while the engine can still write
 * it. This mirrors the OEM preference for uncached/coherent RX ownership and
 * removes map/unmap/cache-alias churn from the single-core MIPS hot path. */
static bool mw5_rx_coherent_bounce = true;
module_param(mw5_rx_coherent_bounce, bool, 0444);
MODULE_PARM_DESC(mw5_rx_coherent_bounce,
		 "MW5 coherent fixed RX DMA bounce ring (stable default on; set 0 only for A/B diagnostics)");

/* v44.65.17 MW5 accelerator ownership.
 *
 * One policy engine owns all MW5 acceleration.  Linux nftables/conntrack and
 * nf_flow_table remain authoritative; the driver may only accelerate an exact
 * flow after TC_SETUP_FT has installed it.  The software engine performs the
 * NAT/L2 rewrite shortcut.  The hardware engine owns only verified DMA/DSA
 * assists on Realtek-DSA-tagged boards; unsafe RTL865x L3/L4 lookup remains fail-closed.
 */
static bool mw5_accel_enable = true;
module_param(mw5_accel_enable, bool, 0644);
MODULE_PARM_DESC(mw5_accel_enable,
		 "MW5 unified accelerator master gate (default on; firewall/conntrack remains authoritative)");
static bool mw5_accel_sw_enable = true;
module_param(mw5_accel_sw_enable, bool, 0644);
MODULE_PARM_DESC(mw5_accel_sw_enable,
		 "MW5 exact-flow software accelerator (default on; only nf_flow_table-authorized IPv4 TCP/UDP NAT)");
static bool mw5_accel_hw_enable = true;
module_param(mw5_accel_hw_enable, bool, 0644);
MODULE_PARM_DESC(mw5_accel_hw_enable,
		 "MW5 hardware assist engine (default on; coherent DMA/DSA egress, RTL865x L34 fail-closed unless topology-safe)");
static bool mw5_hw_csum;
module_param(mw5_hw_csum, bool, 0444);
MODULE_PARM_DESC(mw5_hw_csum, "MW5 TX checksum boot default (stable off; use mw5-hwaccel tx on/off for live changes)");
static bool mw5_hw_rx_csum;
module_param(mw5_hw_rx_csum, bool, 0444);
MODULE_PARM_DESC(mw5_hw_rx_csum, "MW5 RX checksum metadata trust (default off; descriptor metadata remains under validation)");
static bool mw5_hw_sg;
module_param(mw5_hw_sg, bool, 0444);
MODULE_PARM_DESC(mw5_hw_sg, "MW5 RTL8197F SG boot default (stable off; use mw5-hwaccel sg on/off for live changes)");
static bool mw5_hw_tso;
module_param(mw5_hw_tso, bool, 0444);
MODULE_PARM_DESC(mw5_hw_tso, "MW5 RTL8197F TSO/TSO6 (default off; diagnostic opt-in)");
static bool mw5_hw_qos;
module_param(mw5_hw_qos, bool, 0444);
MODULE_PARM_DESC(mw5_hw_qos, "MW5 RTL8197F P0 three-queue hardware QoS (default off in v44.65.10 stable baseline; explicit A/B opt-in)");
static unsigned int mw5_sg_max_frags = 4;
module_param(mw5_sg_max_frags, uint, 0644);
MODULE_PARM_DESC(mw5_sg_max_frags, "MW5 SG safety cap: maximum skb frags mapped to RTL8197F TX descriptors (default 4)");
static bool mw5_hwlookup_enable;
module_param(mw5_hwlookup_enable, bool, 0444);
MODULE_PARM_DESC(mw5_hwlookup_enable, "LEGACY MW5 HWLOOKUP gate (retired by v44.65.17; kept off)");
static bool mw5_hw_napt_enable;
module_param(mw5_hw_napt_enable, bool, 0400);
MODULE_PARM_DESC(mw5_hw_napt_enable, "LEGACY MW5 RTL865x HW-NAPT gate (retired by v44.65.17; kept off)");
/* Generic RTL8197F gates.  These never create policy on their own: the NAPT
 * gate only allows nf_flow_table/tc flower rules already accepted by
 * firewall4+conntrack to be mirrored into the ASIC. */
static bool rtl8197f_hw_csum_enable;
module_param(rtl8197f_hw_csum_enable, bool, 0644);
MODULE_PARM_DESC(rtl8197f_hw_csum_enable, "RTL8197F checksum offload opt-in for non-DSA boards");
static bool rtl8197f_hw_sg_enable;
module_param(rtl8197f_hw_sg_enable, bool, 0644);
MODULE_PARM_DESC(rtl8197f_hw_sg_enable, "RTL8197F SG offload opt-in for non-DSA boards");
static bool rtl8197f_hw_tso_enable;
module_param(rtl8197f_hw_tso_enable, bool, 0644);
MODULE_PARM_DESC(rtl8197f_hw_tso_enable, "RTL8197F TSO/TSO6 opt-in; requires SG+checksum");
static bool rtl8197f_hw_qos_enable;
module_param(rtl8197f_hw_qos_enable, bool, 0644);
MODULE_PARM_DESC(rtl8197f_hw_qos_enable, "RTL8197F three-queue QoS opt-in");
static bool rtl8197f_hw_napt_enable;
module_param(rtl8197f_hw_napt_enable, bool, 0600);
MODULE_PARM_DESC(rtl8197f_hw_napt_enable, "RTL865x HW-NAPT flowtable mirror; firewall/conntrack remains authoritative");
static bool mw5_extport_wlan_enable;
module_param(mw5_extport_wlan_enable, bool, 0400);
MODULE_PARM_DESC(mw5_extport_wlan_enable, "LEGACY MW5 extension-port gate (retired by v44.65.17; kept off)");
static bool mw5_drop_invalid_untagged;
module_param(mw5_drop_invalid_untagged, bool, 0600);
MODULE_PARM_DESC(mw5_drop_invalid_untagged,
		 "MW5 diagnostic fail-closed drop for untagged RX frames with unknown descriptor SPA");

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


/*
 * v31 stable raw-RX validation: the PC may continuously transmit a broadcast/raw
 * Ethernet or UDP test frame containing one of these ASCII tokens.  The
 * RTL8197F RX path scans raw DMA payload bytes before eth_type_trans()/DSA/
 * bridge/IP handling, so rd05-netdiag can identify the first hardware init
 * candidate that really makes a PC-originated frame reach the CPU descriptor.
 */
static bool rd05_magic_enable;
/* v44.57: raw magic-token scanning is diagnostic-only and performs several
 * bytewise searches per RX frame. Default it off for production throughput;
 * rd05_magic_enable=1 still restores the exact diagnostic path at runtime. */
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
 * The RTL8197F SDK has distinct RX and TX new-descriptor bit layouts, but
 * both use the same DMA container size:
 *
 *   - RTL8197F/RTL8197FS (non-VG): 6 dwords / 24 bytes
 *   - RTL8197F-VG/RTL8197FH-VG with CONFIG_RTL_{TX,RX}_CACHE_ALIGN:
 *     8 dwords / 32 bytes
 *
 * v43.11 follows the OEM New_swNic completion model as well: CPURPDCR0 and
 * CPUTPDCR0 are the authoritative current-descriptor pointers. OWN remains
 * useful for the RX runout case and diagnostics, but it must not override an
 * already-advanced hardware CDP on the 24-byte RTL8197F rings.
 *
 * The Tenda Nova MW5 is the non-VG RTL8197FS variant.  Its CPUICR1 hardware
 * readback keeps CF_TXDESC/CF_RXDESC at zero, exactly matching the SDK's
 * six-dword default.  Walking its rings in 32-byte steps leaves TX OWN set and
 * RX permanently empty.  Keep the maximum eight-dword software container but
 * select the actual ring stride per board.
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

struct rtl8197f_rtk_napt_key {
	__be32 int_ip;
	__be32 ext_ip;
	__be32 rem_ip;
	__be16 int_port;
	__be16 ext_port;
	__be16 rem_port;
	u8 proto;
};

struct rtl8197f_rtk_napt_hw {
	struct list_head list;
	struct rtl8197f_rtk_napt_key key;
	u16 in_idx;
	u16 out_idx;
	u8 extip_idx;
	u32 refs;
};

struct rtl8197f_rtk_napt_cookie {
	struct list_head list;
	unsigned long cookie;
	struct rtl8197f_rtk_napt_hw *hw;
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
	struct page **rx_page;
	dma_addr_t *rx_dma;
	struct page_pool *rx_page_pool;
	u32 rx_headroom;
	bool rx_page_pool_enabled;
	void *rx_coherent_pool;
	dma_addr_t rx_coherent_dma;
	size_t rx_coherent_size;
	u32 rx_coherent_stride;
	bool rx_coherent_enabled;
	struct dma_pool *tx_coherent_pool;
	void **tx_coherent_cpu;
	dma_addr_t *tx_coherent_dma_slot;
	u32 tx_coherent_stride;
	bool tx_coherent_enabled;
	u32 rx_ring_size;
	u32 rx_buf_size;
	u32 rx_tail;

	struct rtl8197f_rtk_desc *tx_desc;
	dma_addr_t tx_desc_dma;
	struct sk_buff **tx_skb;
	dma_addr_t *tx_dma;
	u32 *tx_len;
	u32 *tx_pkt_len;
	bool *tx_dma_is_page;
	u32 tx_ring_size;
	u32 tx_head;
	u32 tx_tail;
	u32 desc_dwords;
	u32 desc_stride;

	u32 tx_port_mask;
	u32 tx_dp_ext;
	u32 tx_extspa;
	bool tx_hwlookup;
	bool tx_bridge;
	bool is_mw5;
	bool is_iball;
	bool internal_vlan_split;
	bool napt_capable;
	bool napt_hw_initialized;
	u16 lan_vid;
	u16 wan_vid;
	u32 lan_port_mask;
	u32 wan_port_mask;
	u8 wan_mac[ETH_ALEN];
	bool wan_mac_valid;
	struct mii_bus *mii_bus;
	struct phy_device *phydev;
	struct device_node *phy_node;
	phy_interface_t phy_mode;
	bool mdiobus_registered;
	struct mutex napt_lock;
	struct list_head napt_hw_flows;
	struct list_head napt_cookies;
	DECLARE_BITMAP(napt_used, RTL_RTK_NAPT_TABLE_SIZE);
	__be32 napt_extip[RTL_RTK_EXTIP_TABLE_SIZE];
	u16 napt_extip_ref[RTL_RTK_EXTIP_TABLE_SIZE];
	bool hw_csum;
	bool hw_rx_csum;
	bool hw_sg;
	bool hw_tso;
	bool hw_qos;
	bool hw_napt;
	bool accel_sw;
	bool accel_hw;
	bool accel_hw_l34_safe;
	u64 accel_hw_packets;
	u64 accel_hw_fallback;
	u64 accel_hw_flow_attempts;
	u64 accel_hw_flow_rejects;
	spinlock_t accel_sw_lock;
	struct list_head accel_sw_flows;
	struct hlist_head accel_sw_hash[MW5_ACCEL_SW_HASH_SIZE];
	u64 accel_sw_hash_lookups;
	u64 accel_sw_hash_steps;
	u64 accel_sw_hash_max_steps;
	u64 accel_sw_replace_ok;
	u64 accel_sw_replace_fallback;
	u64 accel_sw_destroy_ok;
	u64 accel_sw_flushes;
	u64 accel_sw_hits;
	u64 accel_sw_misses;
	u64 accel_sw_guard_reject;
	u64 accel_sw_xmit_ok;
	u64 accel_sw_xmit_fail;
	u64 accel_sw_idle_bypass;
	u64 accel_sw_meta_source_ok;
	u64 accel_sw_meta_source_fail;
	u32 accel_sw_active;
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
	bool mw5_p0_transparent;
	u32 rx_poll_ms;
	struct timer_list rx_poll_timer;
	struct delayed_work rd05_reseed_work;
	struct proc_dir_entry *rd05_proc;
	u8 rd05_seeded_mac[ETH_ALEN];

	u64 rd05_mac_changes;
	u64 rd05_mac_reseeds;
	u64 irq_rx_done;
	u64 irq_tx_done;
	u64 irq_rx_errors;
	u64 irq_tx_errors;
	u64 irq_rx_runout;
	u64 irq_mbuf_runout;
	u64 irq_link_change;
	u64 rx_runout_acks;
	bool rx_runout_pending;
	unsigned long rx_runout_guard_until;
	u64 rx_runout_recoveries;
	u64 rx_runout_fast_timer_runs;
	u64 rx_runout_fast_schedules;
	u64 rx_runout_trxrdy_writes;
	bool rx_publish_wait_pending;
	u64 rx_publish_waits;
	u64 rx_publish_ready;
	u64 rx_publish_cdp_invalid;
	u64 rx_publish_timer_runs;
	bool mw5_rx_seq_valid;
	bool mw5_rx_runout_seq_accounted;
	u32 mw5_rx_hw_last_idx;
	u64 mw5_rx_hw_seq;
	u64 mw5_rx_sw_seq;
	u64 mw5_rx_seq_advances;
	u64 mw5_rx_seq_waits;
	bool mw5_rx_catchup_pending;
	unsigned long mw5_rx_corrupt_window_start;
	u32 mw5_rx_corrupt_window_count;
	u64 mw5_rx_corrupt_clusters;
	u64 mw5_rx_catchups;
	u64 mw5_rx_catchup_descs;
	u64 mw5_rx_invalid_streak_max;
	u32 mw5_rx_invalid_streak;
	u64 napi_polls;
	u64 napi_rx_work;
	u64 napi_tx_clean;
	u64 napi_zero_rx_polls;
	u64 napi_budget_polls;
	u64 napi_rearm_race;
	u64 rx_bad_desc;
	u64 rx_missing_skb;
	u64 rx_alloc_fail;
	u64 rx_dma_errors;
	u64 rx_cdp_reads;
	u64 rx_cdp_advanced;
	u64 rx_cdp_owned_advanced;
	u64 rx_cdp_owned_wait;
	u64 rx_cdp_owned_recovered;
	u64 rx_cdp_invalid;
	u64 rx_cdp_last;
	u64 rx_cdp_last_idx;
	u64 tx_mtu_drops;
	u64 tx_dma_errors;
	u64 rx_page_pool_alloc;
	u64 rx_page_pool_recycle;
	u64 rx_page_pool_fallback;
	u64 rx_coherent_copies;
	u64 rx_coherent_alloc_fail;
	u64 tx_coherent_copies;
	u64 tx_coherent_bytes;
	u64 tx_coherent_alloc_fail;
	u64 tx_coherent_oversize;
	/* v44.66.4+: a coherent slot is never overwritten unless both the HW OWN
	 * bit and the software pending skb state say the descriptor is reusable. */
	u64 mw5_tx_reuse_guard_own;
	u64 mw5_tx_reuse_guard_swbusy;
	u64 mw5_tx_meta_lan;
	u64 mw5_tx_meta_wan;
	u64 mw5_tx_meta_invalid;
	u64 mw5_tx_wiretag_lan;
	u64 mw5_tx_wiretag_wan;
	u64 mw5_tx_wiretag_other;
	u64 rx_desc_unstable;
	u64 rx_desc_stable_retry;
	u64 rx_csum_ok;
	u64 rx_csum_none;
	u64 tx_csum_hw;
	u64 tx_csum_sw_fallback;
	u64 tx_sg_packets;
	u64 tx_sg_descs;
	u64 tx_sg_guard_linearize;
	u64 tx_tso4;
	u64 tx_tso6;
	u64 tx_qos_packets;
	u64 tx_hwlookup_dsa_bypass;
	u64 tx_ring_full;
	u64 tx_busy_desc;
	u64 tx_timeouts;
	u64 hw_rx_restarts;
	u64 rx_poll_timer_runs;
	/* v44.56: RX poll is a watchdog only when the normal IRQ path stalls. */
	u64 rx_poll_last_irq_rx_done;
	u64 rx_poll_irq_progress_skips;
	u64 rx_poll_fallback_schedules;
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
	u64 tx_kick_deferred;
	u64 tx_clean_empty;
	u64 tx_bql_sent_bytes;
	u64 tx_bql_completed_pkts;
	u64 tx_bql_completed_bytes;
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
	u64 mw5_rx_tag_passthrough;
	u64 mw5_rx_tag_synthesized;
	u64 mw5_rx_tag_repaired_inplace;
	u64 mw5_rx_tag_fallback_wan_da;
	u64 mw5_rx_tag_fallback_lan_da;
	u64 mw5_rx_tag_unclassified;
	u64 mw5_rx_tag_expand_fail;
	u64 mw5_rx_tag_invalid_port;
	u64 mw5_rx_invalid_spa[8];
	u64 mw5_rx_invalid_ipv4;
	u64 mw5_rx_invalid_ipv6;
	u64 mw5_rx_invalid_arp;
	u64 mw5_rx_invalid_vlan;
	u64 mw5_rx_invalid_realtek;
	u64 mw5_rx_invalid_other;
	u64 mw5_rx_invalid_bcast;
	u64 mw5_rx_invalid_mcast;
	u64 mw5_rx_invalid_ucast;
	u64 mw5_rx_unclassified_known_proto;
	u64 mw5_rx_unclassified_proto4_known;
	u64 mw5_rx_unclassified_sa_lan;
	u64 mw5_rx_unclassified_sa_wan;
	u64 mw5_rx_invalid_drop;
	u64 mw5_rx_oversize_drop;
	u64 mw5_rx_own_wait;
	u64 mw5_rx_cdp_complete;
	u64 mw5_rx_runout_complete;
	u64 mw5_rx_empty_wait;
	u64 mw5_rx_cdp_fallback;
	u64 mw5_rx_addr_mismatch;
	u64 mw5_rx_desc_trace;
	u64 mw5_rx_bad_snapshot;
	u64 mw5_tx_submit;
	u64 mw5_tx_complete;
	u64 mw5_tx_own_after_kick;
	u64 mw5_tx_cdp_seen;
	u64 mw5_tx_cdp_clean;
	u64 mw5_tx_cdp_owned_clean;
	u64 mw5_tx_cdp_invalid;
	u64 mw5_tx_cdp_invalid_defer;
	u64 mw5_tx_own_fallback_clean;
	u64 mw5_tx_own_defer;
	u64 mw5_tx_own_completed;
	u64 mw5_tx_completion_mismatch;
	u64 mw5_tx_own_defer_max;
	u32 mw5_tx_own_defer_streak;
	u32 mw5_tx_defer_idx;
	bool mw5_tx_defer_active;
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
	u64 napt_replace_ok;
	u64 napt_replace_fallback;
	u64 napt_destroy_ok;
	u64 napt_collision;
	u64 napt_extip_alloc;
	u64 napt_extip_free;
	u64 napt_flushes;
	u64 iball_rx_lan;
	u64 iball_rx_wan;
	u64 iball_rx_unknown;
	u64 iball_tx_lan;
	u64 iball_tx_wan;

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

static inline struct rtl8197f_rtk_desc *
rtl8197f_rtk_desc_uncached(struct rtl8197f_rtknet_priv *priv,
			   dma_addr_t ring_dma, void *ring, u32 idx)
{
	dma_addr_t dma = ring_dma + idx * priv->desc_stride;

	/*
	 * RTL8197F OEM New_swNic always consumes the six-dword descriptors
	 * through their KSEG1/UNCACHE_MASK alias.  This matters because a
	 * 24-byte descriptor straddles 32-byte MIPS cache lines: a cached
	 * writeback for one slot can otherwise restore stale words in the
	 * neighbouring slot after hardware has updated them.
	 *
	 * Board DTS files request the SDK KSEG1 bus-address form where the
	 * non-coherent RTL8197F CPU-DMA engine needs it.  Use the matching CPU
	 * alias as well, while retaining the dma_alloc_coherent() pointer for
	 * allocation/free bookkeeping.
	 */
	if (priv->desc_addr_kseg1)
		return (struct rtl8197f_rtk_desc *)
			CKSEG1ADDR(lower_32_bits(dma) & 0x1fffffff);

	return (struct rtl8197f_rtk_desc *)((u8 *)ring +
					    idx * priv->desc_stride);
}

static inline struct rtl8197f_rtk_desc *
rtl8197f_rtk_rx_desc(struct rtl8197f_rtknet_priv *priv, u32 idx)
{
	return rtl8197f_rtk_desc_uncached(priv, priv->rx_desc_dma,
					  priv->rx_desc, idx);
}

static inline struct rtl8197f_rtk_desc *
rtl8197f_rtk_tx_desc(struct rtl8197f_rtknet_priv *priv, u32 idx)
{
	return rtl8197f_rtk_desc_uncached(priv, priv->tx_desc_dma,
					  priv->tx_desc, idx);
}

static inline void
rtl8197f_rtk_desc_clear_padding(struct rtl8197f_rtknet_priv *priv,
				struct rtl8197f_rtk_desc *desc)
{
	if (priv->desc_dwords > 6) {
		desc->opts6 = 0;
		desc->opts7 = 0;
	}
}

static bool rtl8197f_rtk_p0_external_switch_board(void)
{
	return of_machine_is_compatible("xiaomi,r4-rd05") ||
	       of_machine_is_compatible("tenda,nova-mw5");
}

static const char *rtl8197f_rtk_p0_board_name(void)
{
	return of_machine_is_compatible("tenda,nova-mw5") ? "mw5" : "rd05";
}

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
	RTL_RTK_STAT_RX_POLL_IRQ_PROGRESS_SKIPS,
	RTL_RTK_STAT_RX_POLL_FALLBACK_SCHEDULES,
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
	[RTL_RTK_STAT_RX_POLL_IRQ_PROGRESS_SKIPS] = "rx_poll_irq_progress_skips",
	[RTL_RTK_STAT_RX_POLL_FALLBACK_SCHEDULES] = "rx_poll_fallback_schedules",
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

static void rtl8197f_rtk_napt_flush(struct rtl8197f_rtknet_priv *priv,
                                      bool forget_scrub);
int rtl8197f_mw5_accel_rx(struct sk_buff *skb);
int rtl8197f_mw5_accel_setup_tc(struct net_device *source,
                               enum tc_setup_type type, void *type_data);
void rtl8197f_mw5_accel_purge_dev(struct net_device *dev);
static void rtl8197f_mw5_accel_sw_flush(struct rtl8197f_rtknet_priv *priv);

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
static bool rtl8197f_rtk_rx_cdp_index_value(struct rtl8197f_rtknet_priv *priv,
					  u32 cdp, u32 *hw_idx)
{
	u32 base = lower_32_bits(priv->rx_desc_dma) & 0x1fffffff;
	u32 span = priv->rx_ring_size * priv->desc_stride;
	u32 cdp_bus = cdp & 0x1fffffff;
	u32 offset;

	/* RTL8197F may expose the current descriptor through a physical, KSEG0 or
	 * KSEG1 alias. Compare the 29-bit bus address, exactly as the OEM-facing
	 * completion path does, but do not mutate statistics from diagnostics. */
	if (cdp_bus < base || cdp_bus >= base + span)
		return false;

	offset = cdp_bus - base;
	if (offset % priv->desc_stride)
		return false;

	*hw_idx = offset / priv->desc_stride;
	return true;
}

static bool rtl8197f_rtk_rx_cdp_index(struct rtl8197f_rtknet_priv *priv,
				     u32 *hw_idx)
{
	u32 cdp = rtl8197f_rtk_read(priv, RTL_RTK_CPURPDCR0);

	priv->rx_cdp_reads++;
	priv->rx_cdp_last = cdp;

	if (!rtl8197f_rtk_rx_cdp_index_value(priv, cdp, hw_idx)) {
		priv->rx_cdp_invalid++;
		return false;
	}

	priv->rx_cdp_last_idx = *hw_idx;
	return true;
}

static void rtl8197f_rtk_mw5_rx_seq_reset(struct rtl8197f_rtknet_priv *priv)
{
	priv->mw5_rx_seq_valid = false;
	priv->mw5_rx_runout_seq_accounted = false;
	priv->mw5_rx_hw_last_idx = priv->rx_tail;
	priv->mw5_rx_hw_seq = 0;
	priv->mw5_rx_sw_seq = 0;
	priv->mw5_rx_catchup_pending = false;
	priv->mw5_rx_corrupt_window_start = 0;
	priv->mw5_rx_corrupt_window_count = 0;
}

static bool rtl8197f_rtk_mw5_rx_update_hw_seq(struct rtl8197f_rtknet_priv *priv,
					       u32 *hw_idx_out)
{
	u32 hw_idx, delta;

	if (!priv->is_mw5 || !priv->rx_ring_size ||
	    !rtl8197f_rtk_rx_cdp_index(priv, &hw_idx))
		return false;

	if (!priv->mw5_rx_seq_valid) {
		delta = (hw_idx + priv->rx_ring_size - priv->rx_tail) %
			priv->rx_ring_size;
		priv->mw5_rx_hw_seq = priv->mw5_rx_sw_seq + delta;
		priv->mw5_rx_hw_last_idx = hw_idx;
		priv->mw5_rx_seq_valid = true;
	} else {
		delta = (hw_idx + priv->rx_ring_size -
			 priv->mw5_rx_hw_last_idx) % priv->rx_ring_size;
		if (delta) {
			priv->mw5_rx_hw_seq += delta;
			priv->mw5_rx_seq_advances += delta;
			priv->mw5_rx_hw_last_idx = hw_idx;
		}
	}

	/* A full ring may advance CPURPDCR0 by exactly one complete wrap, leaving
	 * the index unchanged. Account that wrap once while the real runout IRQ is
	 * pending; otherwise producer/consumer sequence numbers remain monotonic.
	 */
	if (READ_ONCE(priv->rx_runout_pending)) {
		if (!priv->mw5_rx_runout_seq_accounted &&
		    priv->mw5_rx_hw_seq <= priv->mw5_rx_sw_seq) {
			priv->mw5_rx_hw_seq += priv->rx_ring_size;
			priv->mw5_rx_seq_advances += priv->rx_ring_size;
			priv->mw5_rx_runout_seq_accounted = true;
		}
	} else {
		priv->mw5_rx_runout_seq_accounted = false;
	}

	if (hw_idx_out)
		*hw_idx_out = hw_idx;
	return true;
}

static bool rtl8197f_rtk_rx_work_pending(struct rtl8197f_rtknet_priv *priv)
{
	struct rtl8197f_rtk_desc *desc;
	u32 hw_idx = 0;
	u32 idx, opts1;
	bool cdp_valid;

	if (!priv->rx_desc || !priv->rx_ring_size)
		return false;

	idx = priv->rx_tail;
	desc = rtl8197f_rtk_rx_desc(priv, idx);
	opts1 = le32_to_cpu(READ_ONCE(desc->opts1));
	cdp_valid = rtl8197f_rtk_rx_cdp_index(priv, &hw_idx);

	/* SDK USE_SWITCH_RX_CDP semantics: if HW already advanced past the
	 * software tail, there is RX work even when the cached OWN bit lags. */
	if (cdp_valid && idx != hw_idx)
		return true;

	return !(opts1 & RTL_RTK_DESC_OWN);
}

/* v44.65.18: CPURPDCR0 is not a safe NAPI-completion readiness oracle on
 * RTL8197FS/MW5. Under sustained RX load the pointer frequently advances to
 * the next hardware-owned descriptor before the CPU-visible OWN transition.
 * Treating idx != CDP as pending work created an immediate complete/re-arm
 * loop (809k rearm races for 141k real packets in the OpenSpeedTest trace).
 *
 * The stable-descriptor path already requires OWN == 0 before consuming a
 * frame, therefore the completion/timer gate must use that same invariant.
 * Keep the old CDP helper for non-MW5 boards where it is part of the vendor
 * receive contract.
 */
static bool rtl8197f_rtk_mw5_rx_publish_ready(struct rtl8197f_rtknet_priv *priv,
					      u32 idx)
{
	u32 hw_idx;

	if (!priv->is_mw5)
		return true;

	(void)idx;
	if (READ_ONCE(priv->rx_runout_pending)) {
		rtl8197f_rtk_mw5_rx_update_hw_seq(priv, &hw_idx);
		dma_rmb();
		WRITE_ONCE(priv->rx_publish_wait_pending, false);
		return true;
	}

	/* v44.65.23: readiness is generation-aware. A raw idx != CDP test cannot
	 * distinguish producer-ahead from producer-behind after a wrap. The
	 * monotonic producer sequence can. OWN is checked by the caller first.
	 */
	if (!rtl8197f_rtk_mw5_rx_update_hw_seq(priv, &hw_idx)) {
		priv->rx_publish_cdp_invalid++;
		dma_rmb();
		WRITE_ONCE(priv->rx_publish_wait_pending, false);
		return true;
	}

	if (priv->mw5_rx_hw_seq <= priv->mw5_rx_sw_seq) {
		priv->rx_publish_waits++;
		WRITE_ONCE(priv->rx_publish_wait_pending, true);
		return false;
	}

	if (READ_ONCE(priv->rx_publish_wait_pending))
		priv->rx_publish_ready++;
	dma_rmb();
	WRITE_ONCE(priv->rx_publish_wait_pending, false);
	return true;
}

static bool rtl8197f_rtk_rx_napi_ready(struct rtl8197f_rtknet_priv *priv)
{
	struct rtl8197f_rtk_desc *desc;
	u32 opts1;

	if (!priv->rx_desc || !priv->rx_ring_size)
		return false;

	if (!priv->is_mw5)
		return rtl8197f_rtk_rx_work_pending(priv);

	desc = rtl8197f_rtk_rx_desc(priv, priv->rx_tail);
	dma_rmb();
	opts1 = le32_to_cpu(READ_ONCE(desc->opts1));
	if (opts1 & RTL_RTK_DESC_OWN)
		return false;

	return rtl8197f_rtk_mw5_rx_publish_ready(priv, priv->rx_tail);
}

static bool rtl8197f_rtk_tx_cdp_index_value(struct rtl8197f_rtknet_priv *priv,
					       u32 cdp, u32 *hw_idx)
{
	u32 base = rtl8197f_rtk_hw_dma_addr(priv, priv->tx_desc_dma);
	u32 span = priv->tx_ring_size * priv->desc_stride;
	u32 offset;

	if (cdp < base || cdp >= base + span)
		return false;
	offset = cdp - base;
	if (offset % priv->desc_stride)
		return false;

	*hw_idx = offset / priv->desc_stride;
	return true;
}

static bool rtl8197f_rtk_tx_cdp_index(struct rtl8197f_rtknet_priv *priv, u32 *hw_idx)
{
	u32 cdp = rtl8197f_rtk_read(priv, RTL_RTK_CPUTPDCR0);

	if (!rtl8197f_rtk_tx_cdp_index_value(priv, cdp, hw_idx)) {
		priv->mw5_tx_cdp_invalid++;
		return false;
	}

	return true;
}

static unsigned int rtl8197f_rtk_tx_avail(struct rtl8197f_rtknet_priv *priv)
{
	return (priv->tx_tail + priv->tx_ring_size - priv->tx_head - 1) %
		priv->tx_ring_size;
}

static void rtl8197f_rtk_enable_irq(struct rtl8197f_rtknet_priv *priv)
{
	rtl8197f_rtk_write(priv, RTL_RTK_CPUIIMR, RTL_RTK_INT_ENABLE_MASK);
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

static void rtl8197f_rtk_tx_doorbell(struct rtl8197f_rtknet_priv *priv)
{
	u32 cpuicr;

	/* TXFD is the actual packet doorbell.  Preserve the CPU-interface mode bits
	 * but do not rewrite SWCORE TRXRDY for every skb: TRXRDY is asserted during
	 * start/recovery and stays latched while the dataplane is running.
	 */
	cpuicr = rtl8197f_rtk_read(priv, RTL_RTK_CPUICR);
	cpuicr |= RTL_RTK_CPUICR_TXFD | RTL_RTK_CPUICR_TXCMD | RTL_RTK_CPUICR_RXCMD;
	rtl8197f_rtk_write(priv, RTL_RTK_CPUICR, cpuicr);
	priv->tx_kicks++;
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


static int rtl8197f_rtk_mdio_wait(struct rtl8197f_rtknet_priv *priv, u32 *status)
{
	u32 val;
	int ret;

	if (!priv->swcore)
		return -ENODEV;

	ret = readl_poll_timeout(priv->swcore + RTL_RTK_SWCORE_MDCIOSR, val,
				 !(val & RTL_RTK_MDIO_BUSY), 2, 10000);
	if (status)
		*status = val;
	return ret;
}

static int rtl8197f_rtk_mdio_read(struct mii_bus *bus, int phy_id, int regnum)
{
	struct rtl8197f_rtknet_priv *priv = bus->priv;
	u32 status;
	int ret;

	if (phy_id < 0 || phy_id > 31 || regnum < 0 || regnum > 31)
		return -EINVAL;

	ret = rtl8197f_rtk_mdio_wait(priv, NULL);
	if (ret)
		return ret;

	writel(((u32)phy_id << RTL_RTK_MDIO_PHY_SHIFT) |
	       ((u32)regnum << RTL_RTK_MDIO_REG_SHIFT),
	       priv->swcore + RTL_RTK_SWCORE_MDCIOCR);
	ret = rtl8197f_rtk_mdio_wait(priv, &status);
	if (ret)
		return ret;
	if (status & RTL_RTK_MDIO_READ_ERROR)
		return -EIO;

	return status & 0xffff;
}

static int rtl8197f_rtk_mdio_write(struct mii_bus *bus, int phy_id,
				    int regnum, u16 val)
{
	struct rtl8197f_rtknet_priv *priv = bus->priv;
	int ret;

	if (phy_id < 0 || phy_id > 31 || regnum < 0 || regnum > 31)
		return -EINVAL;

	ret = rtl8197f_rtk_mdio_wait(priv, NULL);
	if (ret)
		return ret;

	writel(RTL_RTK_MDIO_CMD_WRITE |
	       ((u32)phy_id << RTL_RTK_MDIO_PHY_SHIFT) |
	       ((u32)regnum << RTL_RTK_MDIO_REG_SHIFT) | val,
	       priv->swcore + RTL_RTK_SWCORE_MDCIOCR);
	return rtl8197f_rtk_mdio_wait(priv, NULL);
}

static int rtl8197f_rtk_register_mdio(struct rtl8197f_rtknet_priv *priv,
				      struct device_node *np)
{
	struct device_node *mdio_np;
	struct mii_bus *bus;
	int ret;

	if (!priv->is_iball || !np || !priv->swcore)
		return 0;

	mdio_np = of_get_child_by_name(np, "mdio");
	if (!mdio_np)
		return -ENODEV;

	bus = devm_mdiobus_alloc(priv->dev);
	if (!bus) {
		of_node_put(mdio_np);
		return -ENOMEM;
	}

	bus->name = "rtl8197f internal MDIO";
	bus->read = rtl8197f_rtk_mdio_read;
	bus->write = rtl8197f_rtk_mdio_write;
	bus->priv = priv;
	bus->parent = priv->dev;
	snprintf(bus->id, MII_BUS_ID_SIZE, "%s-%s", DRV_NAME, dev_name(priv->dev));

	ret = of_mdiobus_register(bus, mdio_np);
	of_node_put(mdio_np);
	if (ret)
		return ret;

	priv->mii_bus = bus;
	priv->mdiobus_registered = true;
	priv->phy_node = of_parse_phandle(np, "phy-handle", 0);
	if (!priv->phy_node) {
		mdiobus_unregister(bus);
		priv->mdiobus_registered = false;
		return -ENODEV;
	}
	if (of_get_phy_mode(np, &priv->phy_mode))
		priv->phy_mode = PHY_INTERFACE_MODE_RGMII;

	dev_info(priv->dev,
		 "iBall RTL8211F: registered internal MDIO, phy=%u mode=%s\n",
		 RTL_RTK_IBALL_RTL8211F_PHY, phy_modes(priv->phy_mode));
	return 0;
}

static void rtl8197f_rtk_adjust_link(struct net_device *ndev)
{
	struct rtl8197f_rtknet_priv *priv = netdev_priv(ndev);
	struct phy_device *phydev = priv->phydev;

	if (!phydev)
		return;
	if (phydev->link) {
		if (!netif_carrier_ok(ndev))
			netif_carrier_on(ndev);
	} else if (netif_carrier_ok(ndev)) {
		netif_carrier_off(ndev);
	}
}

static void rtl8197f_rtk_init_iball_p0_rtl8211f(struct rtl8197f_rtknet_priv *priv,
						 const char *reason)
{
	u32 pcr, gmii, val;
	int adv;

	if (!priv->is_iball || !priv->swcore)
		return;

	/* Reproduce the RTL8197F GPL SDK external-PHY P0 sequence, but leave link
	 * negotiation to phylib/RTL8211F.  P0 polls PHY6; unlike the RTL836x cascade
	 * path it must not be forced permanently link-up.
	 */
	pcr = readl(priv->swcore + RTL_RTK_SWCORE_PCRP(0));
	pcr &= ~(RTL_RTK_PCRP_EXT_PHYID_MASK |
		 RTL_RTK_PCRP_EN_FORCE_MODE |
		 RTL_RTK_PCRP_FORCE_LINK |
		 RTL_RTK_PCRP_FORCE_STATUS_MASK |
		 RTL_RTK_PCRP_PAUSE_MASK |
		 RTL_RTK_PCRP_STP_MASK |
		 RTL_RTK_PCRP_MAC_SW_RESET |
		 RTL_RTK_PCRP_ENABLE_PHY_IF);
	pcr |= (RTL_RTK_IBALL_RTL8211F_PHY << RTL_RTK_PCRP_EXT_PHYID_SHIFT) |
	       RTL_RTK_PCRP_POLL_LINK_STATUS |
	       RTL_RTK_PCRP_MII_RXER |
	       RTL_RTK_PCRP_STP_FORWARDING |
	       RTL_RTK_PCRP_ENABLE_PHY_IF;
	writel(pcr, priv->swcore + RTL_RTK_SWCORE_PCRP(0));
	readl(priv->swcore + RTL_RTK_SWCORE_PCRP(0));
	udelay(10);
	pcr |= RTL_RTK_PCRP_MAC_SW_RESET;
	writel(pcr, priv->swcore + RTL_RTK_SWCORE_PCRP(0));

	gmii = readl(priv->swcore + RTL_RTK_SWCORE_P0GMIICR);
	gmii &= ~(RTL_RTK_P0GMIICR_CPU_TAG_RX |
		  RTL_RTK_P0GMIICR_CPU_TAG_TX |
		  RTL_RTK_P0GMIICR_GMAC_MASK |
		  RTL_RTK_P0GMIICR_RGTXC_MASK |
		  RTL_RTK_P0GMIICR_TX_DELAY_MASK |
		  RTL_RTK_P0GMIICR_RX_DELAY_MASK |
		  RTL_RTK_P0GMIICR_CONF_DONE);
	/* GPL SDK external RTL8211F path: RGTXC=3, TX delay=1, RX delay=5. */
	gmii |= RTL_RTK_P0GMIICR_RGTXC_3 |
		RTL_RTK_P0GMIICR_TX_DELAY_MASK | 5U;
	writel(gmii, priv->swcore + RTL_RTK_SWCORE_P0GMIICR);

	val = readl(priv->swcore + RTL_RTK_SWCORE_PITCR);
	val |= RTL_RTK_PITCR_P0_EXT_INTERFACE;
	writel(val, priv->swcore + RTL_RTK_SWCORE_PITCR);
	val = readl(priv->swcore + RTL_RTK_SWCORE_MACCR);
	val |= RTL_RTK_MACCR_P0_GIGA_LINK;
	writel(val, priv->swcore + RTL_RTK_SWCORE_MACCR);

	gmii |= RTL_RTK_P0GMIICR_CONF_DONE;
	writel(gmii, priv->swcore + RTL_RTK_SWCORE_P0GMIICR);
	readl(priv->swcore + RTL_RTK_SWCORE_P0GMIICR);

	/* Vendor init advertises symmetric/asymmetric pause in PHY register 4. */
	if (priv->mii_bus) {
		adv = rtl8197f_rtk_mdio_read(priv->mii_bus,
					    RTL_RTK_IBALL_RTL8211F_PHY, MII_ADVERTISE);
		if (adv >= 0)
			rtl8197f_rtk_mdio_write(priv->mii_bus,
					     RTL_RTK_IBALL_RTL8211F_PHY, MII_ADVERTISE,
					     adv | ADVERTISE_PAUSE_CAP | ADVERTISE_PAUSE_ASYM);
	}

	dev_info(priv->dev,
		 "iBall RTL8211F P0: %s pcr=0x%08x gmii=0x%08x phy=%u txdly=1 rxdly=5\n",
		 reason, readl(priv->swcore + RTL_RTK_SWCORE_PCRP(0)),
		 readl(priv->swcore + RTL_RTK_SWCORE_P0GMIICR),
		 RTL_RTK_IBALL_RTL8211F_PHY);
}

static void rtl8197f_rtk_init_p0_rgmii(struct rtl8197f_rtknet_priv *priv,
					       const char *reason)
{
	u32 pcr, gmii, val, pad = 0;
	bool mw5 = of_machine_is_compatible("tenda,nova-mw5");
	u32 tx_delay;
	u32 rx_delay;

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

	/* Preserve the external-switch CPU tag after the source MAC for Linux DSA.
	 * v44.66.7 proved that the custom four-byte 0x900x TX format is not consumed
	 * correctly on user egress. v44.66.8/9 then proved that the native rtl8_4
	 * tag itself is correct, but the RTL8197F SwitchCore prepends an 802.1Q
	 * header while P0 remains in PORT0_ROUTER_MODE even though CFG_CPUC_TAG and
	 * CFG_TX_CPUC_TAG are deliberately disabled for Linux DSA. The GPL SDK only
	 * enables PORT0_ROUTER_MODE inside the same CONFIG_RTL_CPU_TAG block as those
	 * two P0 CPU-tag bits. v44.66.10 therefore defaults MW5 to normal/transparent
	 * P0 mode: all three SoC CPU-tag/router controls are off and the external
	 * rtl8_4 bytes are transported unchanged through descriptor DP=P0.
	 */
	/* The non-VG RTL8197FS SDK uses fixed TX0/RX5.  RD05 is an
	 * RTL8197FH-VG board and keeps its bounded runtime calibration knobs.
	 */
	if (mw5) {
		tx_delay = 0;
		rx_delay = RTL_RTK_P0GMIICR_RX_DELAY_FS;
	} else {
		tx_delay = min_t(u32, READ_ONCE(rd05_p0_tx_delay), 1);
		rx_delay = min_t(u32, READ_ONCE(rd05_p0_rx_delay), 7);
	}

	gmii = readl(priv->swcore + RTL_RTK_SWCORE_P0GMIICR);
	gmii &= ~(RTL_RTK_P0GMIICR_CPU_TAG_RX |
		  RTL_RTK_P0GMIICR_CPU_TAG_TX |
		  RTL_RTK_P0GMIICR_GMAC_MASK |
		  RTL_RTK_P0GMIICR_RGTXC_MASK |
		  RTL_RTK_P0GMIICR_TX_DELAY_MASK |
		  RTL_RTK_P0GMIICR_RX_DELAY_MASK |
		  RTL_RTK_P0GMIICR_CONF_DONE);
	/* LINK_RGMII is encoded as zero in bits 24:23. */
	gmii |= RTL_RTK_P0GMIICR_RGTXC_3 | rx_delay;
	if (tx_delay)
		gmii |= RTL_RTK_P0GMIICR_TX_DELAY_MASK;
	if (priv->p0_cpu_tag_passthrough) {
		/* RD05 keeps the vendor hardware parser.  On MW5 the hardware log
		 * proves that enabling CPU_TAG_RX can deliver an already stripped
		 * Ethernet frame to DMA.  Leave RX parsing disabled so a real 0x8899
		 * header can pass unchanged to Linux DSA; the RX descriptor fallback
		 * below reconstructs it when this RTL8197FS revision still strips it.
		 */
		if (!mw5) {
			gmii |= RTL_RTK_P0GMIICR_CPU_TAG_RX;
			gmii |= RTL_RTK_P0GMIICR_CPU_TAG_TX;
		}
	}
	writel(gmii, priv->swcore + RTL_RTK_SWCORE_P0GMIICR);

	val = readl(priv->swcore + RTL_RTK_SWCORE_MACCR1);
	val &= ~(RTL_RTK_MACCR1_P0_ROUTER_MODE |
		 RTL_RTK_MACCR1_RMD_TAG_MASK);
	/* The vendor SDK couples PORT0_ROUTER_MODE to the RTL8197F CPU-tag
	 * parser/generator. MW5 Linux DSA intentionally keeps both CPU_TAG_RX/TX
	 * disabled, so leave P0 in normal mode as well. The old router-mode path is
	 * retained only as a live A/B diagnostic through /proc/rd05-rtknet. */
	if (priv->p0_cpu_tag_passthrough && (!mw5 || !READ_ONCE(priv->mw5_p0_transparent)))
		val |= RTL_RTK_MACCR1_P0_ROUTER_MODE;
	writel(val, priv->swcore + RTL_RTK_SWCORE_MACCR1);

	/* RTL8197F-VG init_8197f_p0() fixes the transmit inter-packet gap
	 * at eight bytes on both PITCR and EXTPCR0.  Without these fields the
	 * external RTL8367 can count each otherwise valid SoC frame as an FCS/drop
	 * event even when the RGMII delay taps themselves are in range.
	 */
	val = readl(priv->swcore + RTL_RTK_SWCORE_PITCR);
	val |= RTL_RTK_PITCR_P0_EXT_INTERFACE;
	if (!mw5) {
		val &= ~RTL_RTK_PITCR_FIX_IPG_MASK;
		val |= RTL_RTK_PITCR_FIX_IPG_8BYTE;
	}
	writel(val, priv->swcore + RTL_RTK_SWCORE_PITCR);

	/* The SDK applies the fixed eight-byte IPG only on RTL8197F-VG. */
	if (!mw5) {
		val = readl(priv->swcore + RTL_RTK_SWCORE_EXTPCR0);
		val &= ~RTL_RTK_EXTPCR0_TX_IPG_MASK;
		val |= RTL_RTK_EXTPCR0_TX_IPG_8BYTE;
		writel(val, priv->swcore + RTL_RTK_SWCORE_EXTPCR0);
	}

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
		       (RTL_RTK_PAD_P0_DRIVE_VG << RTL_RTK_PAD_P0_RGMII_DP_SHIFT);
		if (!mw5)
			pad |= RTL_RTK_PAD_P0_RGMII_MODE |
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
		 "%s p0 rgmii sdk: %s pcrp0=0x%08x p0gmii=0x%08x pitcr=0x%08x extpcr0=0x%08x maccr=0x%08x maccr1=0x%08x pad=0x%08x txdelay=%u rxdelay=%u cputag-rx=%u cputag-tx=%u p0mode=%s preserve-tag=1 tag=%s rx-policy=%s txdp=0x1/dpext=0\n",
		 mw5 ? "mw5" : "rd05",
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
		    RTL_RTK_P0GMIICR_CPU_TAG_TX),
		 mw5 && READ_ONCE(priv->mw5_p0_transparent) ? "transparent" : "router",
		 "rtl8_4/8byte",
		 mw5 ? "wire-pass+desc-fallback" : "hardware-parser");
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
	rtl8197f_rtk_init_p0_rgmii(priv, reason);

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

/* Silent TACI writer used by the flowtable-owned L3/L4 tables.  Unlike the
 * RD05 bring-up helpers this path can touch hundreds of entries during the
 * one-time security scrub, so it deliberately avoids per-entry printk noise.
 */
static int rtl8197f_rtk_force_table_raw(struct rtl8197f_rtknet_priv *priv,
                                        u32 type, u32 eidx,
                                        const u32 *words, unsigned int nwords)
{
    u32 status, addr;
    unsigned int i;
    int ret;

    if (!priv->swcore || nwords > 8)
        return -EINVAL;

    ret = rtl8197f_rtk_rd05_taci_wait(priv, "flow-pre");
    if (ret)
        return ret;

    for (i = 0; i < 8; i++)
        writel(i < nwords ? words[i] : 0,
               priv->swcore + RTL_RTK_TCR0 + i * sizeof(u32));

    addr = RTL_RTK_ASICTBL_BASE_KSEG1 +
           (type << 16) + eidx * RTL_RTK_ASICTBL_ENTRY_LEN;
    writel(addr, priv->swcore + RTL_RTK_SWTAA);
    wmb();
    writel(RTL_RTK_TACI_ACTION_START | RTL_RTK_TACI_CMD_FORCE,
           priv->swcore + RTL_RTK_SWTACR);

    ret = rtl8197f_rtk_rd05_taci_wait(priv, "flow-post");
    if (ret)
        return ret;
    status = readl(priv->swcore + RTL_RTK_SWTASR);
    return (status & RTL_RTK_TACI_TABSTS_FAIL) ? -EIO : 0;
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

static void rtl8197f_rtk_iball_seed_internal_switch(struct rtl8197f_rtknet_priv *priv)
{
	u32 lan_w0, wan_w0, w[5];
	const u8 *wan_mac;
	unsigned int port;

	if (!priv->is_iball || !priv->internal_vlan_split || !priv->swcore)
		return;
	if (!rtl8197f_rtk_rd05_wait_swcore_ready(priv))
		return;

	/* Preserve the OEM LAN extension member (0x100) while Linux owns only the
	 * physical P1..P4 egress mask.  VID8 is the dedicated P0/RGMII WAN domain.
	 */
	lan_w0 = rtl8197f_rtk_rd05_vlan_w0(priv->lan_port_mask | BIT(8), 0);
	wan_w0 = rtl8197f_rtk_rd05_vlan_w0(priv->wan_port_mask, 1);
	rtl8197f_rtk_rd05_force_table2(priv, RTL_RTK_ASICTBL_TYPE_VLAN,
				       priv->lan_vid, lan_w0, 0, "iball-lan-vid9");
	rtl8197f_rtk_rd05_force_table2(priv, RTL_RTK_ASICTBL_TYPE_VLAN,
				       priv->wan_vid, wan_w0, 0, "iball-wan-vid8");

	/* OEM: P0 is WAN/VID8, P1..P4 are LAN/VID9. */
	rtl8197f_rtk_rd05_set_pvid(priv, 0, priv->wan_vid);
	for (port = 1; port <= 4; port++)
		rtl8197f_rtk_rd05_set_pvid(priv, port, priv->lan_vid);

	/* Two real rtl865x NETIF entries.  L3/L4 forwarding remains globally off
	 * until a firewall-created flowtable rule is installed by the NAPT backend.
	 */
	memset(w, 0, sizeof(w));
	w[0] = rtl8197f_rtk_rd05_netif_w0(priv->ndev->dev_addr, priv->lan_vid);
	w[1] = rtl8197f_rtk_rd05_netif_w1(priv->ndev->dev_addr, true, false, 0);
	w[2] = rtl8197f_rtk_rd05_netif_w2(0, 4, 253, 253, 1);
	w[3] = rtl8197f_rtk_rd05_netif_w3(priv->ndev->mtu, priv->ndev->mtu, 1);
	rtl8197f_rtk_rd05_force_table5(priv, RTL_RTK_ASICTBL_TYPE_NETIF,
				       0, w, "iball-netif-lan-vid9");

	wan_mac = priv->wan_mac_valid ? priv->wan_mac : priv->ndev->dev_addr;
	memset(w, 0, sizeof(w));
	w[0] = rtl8197f_rtk_rd05_netif_w0(wan_mac, priv->wan_vid);
	w[1] = rtl8197f_rtk_rd05_netif_w1(wan_mac, true, false, 0);
	w[2] = rtl8197f_rtk_rd05_netif_w2(0, 4, 253, 253, 1);
	w[3] = rtl8197f_rtk_rd05_netif_w3(priv->ndev->mtu, priv->ndev->mtu, 1);
	rtl8197f_rtk_rd05_force_table5(priv, RTL_RTK_ASICTBL_TYPE_NETIF,
				       1, w, "iball-netif-wan-vid8");

	/* VLAN-aware L2 only at baseline. Unknown traffic is sent to CPU, not
	 * accelerated. L3/L4 is switched on only while a validated NAPT rule exists.
	 */
	rtl8197f_rtk_rd05_reg_update(priv, RTL_RTK_SWCORE_SWTCR0,
				     RTL_RTK_SWTCR0_EN_UKVID_TO_CPU |
				     RTL_RTK_SWTCR0_LIMDBC_MASK,
				     RTL_RTK_SWTCR0_EN_UKVID_TO_CPU |
				     RTL_RTK_SWTCR0_LIMDBC_VLAN,
				     "iball-vlan-decision");
	rtl8197f_rtk_rd05_reg_update(priv, RTL_RTK_SWCORE_MSCR,
				     RTL_RTK_MSCR_EN_L2 | RTL_RTK_MSCR_EN_L3 |
				     RTL_RTK_MSCR_EN_L4 | RTL_RTK_MSCR_EN_IN_ACL,
				     RTL_RTK_MSCR_EN_L2,
				     "iball-l2-baseline");
	rtl8197f_rtk_rd05_reg_update(priv, RTL_RTK_SWCORE_FFCR,
				     RTL_RTK_FFCR_EN_UNK_MCAST_TOCPU |
				     RTL_RTK_FFCR_EN_UNK_UCAST_TOCPU,
				     RTL_RTK_FFCR_EN_UNK_MCAST_TOCPU |
				     RTL_RTK_FFCR_EN_UNK_UCAST_TOCPU,
				     "iball-unknown-to-cpu");

	dev_info(priv->dev,
		 "iBall rtl865x split: LAN vid=%u mask=0x%x, WAN vid=%u mask=0x%x; L3/L4 fail-closed until nf_flow_table rule\n",
		 priv->lan_vid, priv->lan_port_mask, priv->wan_vid, priv->wan_port_mask);
}

static void rtl8197f_rtk_mw5_seed_oem_pipeline(struct rtl8197f_rtknet_priv *priv)
{
	static const u16 pvid[9] = { 1, 9, 1, 8, 1, 1, 1, 1, 9 };
	const u8 *lan_mac = priv->ndev->dev_addr;
	u8 wan_mac[ETH_ALEN];
	u32 netif[5];
	unsigned int port;

	if (!priv->swcore)
		return;

	/* V212 live OEM state.  Unlike the older P0-only bootstrap model, the
	 * RTL865x tables themselves distinguish LAN P1/P8 (VID9/FID0) from WAN
	 * P3 (VID8/FID1).  Keep this MW5-only; RD05 still uses its P0 cascade seed.
	 */
	for (port = 0; port < ARRAY_SIZE(pvid); port++)
		rtl8197f_rtk_rd05_set_pvid(priv, port, pvid[port]);

	rtl8197f_rtk_rd05_reg_update(priv, RTL_RTK_SWCORE_SWTCR0,
				     RTL_RTK_SWTCR0_LIMDBC_MASK |
				     RTL_RTK_SWTCR0_EN_UKVID_TO_CPU,
				     RTL_RTK_SWTCR0_LIMDBC_VLAN |
				     RTL_RTK_SWTCR0_EN_UKVID_TO_CPU,
				     "mw5-v212-vlan-netdec");

	/* Do not enable L3/L4/HW NAPT while DMA/DSA is under validation.  The
	 * OEM NETIF route bits are reproduced below because they are table facts,
	 * but MSCR remains L2-only, making this fail-closed for acceleration.
	 */
	rtl8197f_rtk_rd05_reg_update(priv, RTL_RTK_SWCORE_MSCR,
				     RTL_RTK_MSCR_EN_L2 | RTL_RTK_MSCR_EN_L3 |
				     RTL_RTK_MSCR_EN_L4 | RTL_RTK_MSCR_EN_IN_ACL,
				     RTL_RTK_MSCR_EN_L2,
				     "mw5-v212-l2-only");
	rtl8197f_rtk_rd05_reg_update(priv, RTL_RTK_SWCORE_FFCR,
				     RTL_RTK_FFCR_EN_UNK_MCAST_TOCPU |
				     RTL_RTK_FFCR_EN_UNK_UCAST_TOCPU,
				     RTL_RTK_FFCR_EN_UNK_MCAST_TOCPU |
				     RTL_RTK_FFCR_EN_UNK_UCAST_TOCPU,
				     "mw5-v212-unknown-to-cpu");

	/* V212 NETIF[0]: VID9, LAN MAC, route v4/v6, ingress ACL 0..1,
	 * egress ACL 253, one MAC, MTU/MTUv6 1500. */
	memset(netif, 0, sizeof(netif));
	netif[0] = rtl8197f_rtk_rd05_netif_w0(lan_mac, RTL_RTK_MW5_LAN_VID);
	netif[1] = rtl8197f_rtk_rd05_netif_w1(lan_mac, true, true, 0);
	netif[2] = rtl8197f_rtk_rd05_netif_w2(0, 1, 253, 253, 1);
	netif[3] = rtl8197f_rtk_rd05_netif_w3(1500, 1500, 1);
	if (!rtl8197f_rtk_rd05_force_table5(priv, RTL_RTK_ASICTBL_TYPE_NETIF,
					   0, netif, "mw5-v212-netif0-vid9"))
		priv->rd05_netif_ok++;

	if (priv->wan_mac_valid) {
		ether_addr_copy(wan_mac, priv->wan_mac);
	} else {
		ether_addr_copy(wan_mac, lan_mac);
		eth_addr_add(wan_mac, 7);
	}

	/* V212 NETIF[1]: VID8, base+7 WAN MAC, route v4/v6, ingress ACL 2..3. */
	memset(netif, 0, sizeof(netif));
	netif[0] = rtl8197f_rtk_rd05_netif_w0(wan_mac, RTL_RTK_MW5_WAN_VID);
	netif[1] = rtl8197f_rtk_rd05_netif_w1(wan_mac, true, true, 2);
	netif[2] = rtl8197f_rtk_rd05_netif_w2(2, 3, 253, 253, 1);
	netif[3] = rtl8197f_rtk_rd05_netif_w3(1500, 1500, 1);
	if (!rtl8197f_rtk_rd05_force_table5(priv, RTL_RTK_ASICTBL_TYPE_NETIF,
					   1, netif, "mw5-v212-netif1-vid8"))
		priv->rd05_netif_ok++;

	/* V212 also shows ACL0/2 as IP proto 2 -> CPU and ACL1/3 as permit-all.
	 * We intentionally do not synthesize their raw 11-DWORD encoding here:
	 * the proc dump proves semantics, not every table bit. IN_ACL is disabled
	 * above, so leaving those entries untouched is safer than another guess.
	 */
	dev_info(priv->dev,
		 "mw5 V212 rtl865x topology: PVID p0..p8=1,9,1,8,1,1,1,1,9; netif0 VID9 ACL0-1, netif1 VID8 ACL2-3; L3/L4/HW-NAPT disabled\n");
}

static void rtl8197f_rtk_rd05_seed_pipeline(struct rtl8197f_rtknet_priv *priv)
{
	const u8 *lan_mac = priv->ndev->dev_addr;
	const char *board = rtl8197f_rtk_p0_board_name();
	u32 netif[5];

	if (!rtl8197f_rtk_p0_external_switch_board())
		return;

	if (!priv->swcore) {
		priv->rd05_pipe_fail++;
		dev_warn(priv->dev,
			 "rd05 rtl865x pipe v5ag: no swcore mapping, skip PVID/netif/ACL seed\n");
		return;
	}

	if (priv->is_mw5) {
		rtl8197f_rtk_mw5_seed_oem_pipeline(priv);
		return;
	}

	dev_info(priv->dev,
		 "%s rtl865x pipe v43.6: seed P0/RGMII host-link ingress PVID->netif->ACL->CPU lan_vid=%u host_port=%u host_mbr=0x%x mac=%pM\n",
		 board, RTL_RTK_RD05_LAN_VID, RTL_RTK_RD05_HOST_PORT,
		 (u32)RTL_RTK_RD05_HOST_MBR, lan_mac);

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
		 "%s rtl865x pipe v43.6: seed done pipe_runs=%llu pipe_ok=%llu pipe_fail=%llu pvid_ok=%llu netif_ok=%llu acl_ok=%llu acl_fail=%llu ffcr=0x%llx\n",
		 board,
		 (unsigned long long)priv->rd05_pipe_runs,
		 (unsigned long long)priv->rd05_pipe_ok,
		 (unsigned long long)priv->rd05_pipe_fail,
		 (unsigned long long)priv->rd05_pvid_ok,
		 (unsigned long long)priv->rd05_netif_ok,
		 (unsigned long long)priv->rd05_acl_ok,
		 (unsigned long long)priv->rd05_acl_fail,
		 (unsigned long long)priv->rd05_ffcr_read);
}

static void rtl8197f_rtk_mw5_seed_oem_l2cpu(struct rtl8197f_rtknet_priv *priv)
{
	static const u8 bcast[ETH_ALEN] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
	static const u8 oem_cpu_marker[ETH_ALEN] = { 0x00, 0x00, 0x0a, 0x00, 0x00, 0x0f };
	/* v44.66.9 DSA host-link adaptation: the OEM tables mark every member
	 * also untagged.  Add physical P0/RGMII to both internal VID domains so
	 * a descriptor carrying DVID 9/8 can leave the SoC on the cascade without
	 * SwitchCore prepending the priority-tag VLAN 0 observed on v44.66.8.
	 * External LAN/WAN selection remains exclusively in the rtl8_4 CPU tag. */
	const u16 lan_members = BIT(RTL_RTK_RD05_HOST_PORT) |
				BIT(RTL_RTK_MW5_LAN_PORT) |
				BIT(RTL_RTK_MW5_CPU_EXT_PORT);
	const u16 wan_members = BIT(RTL_RTK_RD05_HOST_PORT) |
				BIT(RTL_RTK_MW5_WAN_PORT);
	const u16 bcast_members = BIT(RTL_RTK_MW5_LAN_PORT) |
				  BIT(RTL_RTK_MW5_WAN_PORT);
	u32 vlan9, vlan8;

	vlan9 = rtl8197f_rtk_rd05_vlan_w0(lan_members, 0);
	vlan8 = rtl8197f_rtk_rd05_vlan_w0(wan_members, 1);
	rtl8197f_rtk_rd05_force_table2(priv, RTL_RTK_ASICTBL_TYPE_VLAN,
				       RTL_RTK_MW5_LAN_VID, vlan9, 0,
				       "mw5-dsa-vlan9-p0-p1-p8-untag-fid0");
	rtl8197f_rtk_rd05_force_table2(priv, RTL_RTK_ASICTBL_TYPE_VLAN,
				       RTL_RTK_MW5_WAN_VID, vlan8, 0,
				       "mw5-dsa-vlan8-p0-p3-untag-fid1");

	/* Exact static entries in V212 /proc/rtl865x/l2: broadcast is visible to
	 * P1/P3 plus CPU in both FIDs; 00:00:0a:00:00:0f is CPU/STA/NH in FID0/1.
	 * The router LAN MAC on P8 is dynamic in OEM, so do not force it static.
	 */
	rtl8197f_rtk_rd05_force_l2(priv, bcast, 0, bcast_members,
				      true, true, true, true, "mw5-v212-bcast-fid0");
	rtl8197f_rtk_rd05_force_l2(priv, oem_cpu_marker, 0, 0,
				      true, true, true, true, "mw5-v212-cpu-marker-fid0");
	rtl8197f_rtk_rd05_force_l2(priv, oem_cpu_marker, 1, 0,
				      true, true, true, true, "mw5-v212-cpu-marker-fid1");
	rtl8197f_rtk_rd05_force_l2(priv, bcast, 1, bcast_members,
				      true, true, true, true, "mw5-v212-bcast-fid1");

	ether_addr_copy(priv->rd05_seeded_mac, priv->ndev->dev_addr);
	priv->rd05_mac_reseeds++;
	dev_info(priv->dev,
		 "mw5 DSA rtl865x L2/VLAN v44.66.9: VID9=P0+P1+P8/untag/FID0 VID8=P0+P3/untag/FID1 bcast=P1+P3+CPU marker=%pM\n",
		 oem_cpu_marker);
}

static void rtl8197f_rtk_rd05_seed_l2cpu(struct rtl8197f_rtknet_priv *priv)
{
	static const u8 bcast[ETH_ALEN] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
	const u8 *cpu_mac = priv->ndev->dev_addr;
	const char *board = rtl8197f_rtk_p0_board_name();
	u8 mw5_wan_mac[ETH_ALEN];
	bool seed_mw5_wan = false;
	u32 vlan9 = rtl8197f_rtk_rd05_vlan_w0(RTL_RTK_RD05_HOST_MBR, 0);
	u32 vlan8 = rtl8197f_rtk_rd05_vlan_w0(RTL_RTK_RD05_HOST_MBR, 0);

	if (!rtl8197f_rtk_p0_external_switch_board())
		return;

	if (!priv->swcore) {
		priv->rd05_l2cpu_fail++;
		dev_warn(priv->dev,
			 "rd05 rtl865x l2cpu v5ag: no swcore mapping, skip OEM L2/VID seed\n");
		return;
	}

	if (priv->is_mw5) {
		rtl8197f_rtk_mw5_seed_oem_l2cpu(priv);
		return;
	}

	dev_info(priv->dev,
		 "%s rtl865x l2cpu v43.6: seed VID9/L2 CPU entries on physical P0 host link mbr=0x%x cpu-mac=%pM\n",
		 board, (u32)RTL_RTK_RD05_HOST_MBR, cpu_mac);

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

	/* v44.65.15: prefer the WAN identity parsed from factory NVMEM at probe.
	 * That makes the very first RTL865x seed deterministic and independent of
	 * later bridge/netifd MAC selection.  Keep the global-base derivation as a
	 * compatibility fallback for older DTBs. */
	if (priv->is_mw5 && priv->wan_mac_valid) {
		ether_addr_copy(mw5_wan_mac, priv->wan_mac);
		seed_mw5_wan = true;
	} else if (priv->is_mw5 && is_valid_ether_addr(cpu_mac) &&
		   !is_local_ether_addr(cpu_mac)) {
		ether_addr_copy(mw5_wan_mac, cpu_mac);
		eth_addr_add(mw5_wan_mac, 7);
		seed_mw5_wan = is_valid_ether_addr(mw5_wan_mac);
	}
	if (seed_mw5_wan) {
		rtl8197f_rtk_rd05_force_l2(priv, mw5_wan_mac, 0, 0,
				       true, true, true, true, "l2-mw5-wan-fid0");
		rtl8197f_rtk_rd05_force_l2(priv, mw5_wan_mac, 1, 0,
				       true, true, true, true, "l2-mw5-wan-fid1");
		dev_info(priv->dev,
			 "mw5 rtl865x l2cpu v44.65.15: static local base=%pM routed-wan=%pM\n",
			 cpu_mac, mw5_wan_mac);
	} else if (of_machine_is_compatible("tenda,nova-mw5")) {
		dev_warn(priv->dev,
			 "mw5 rtl865x l2cpu v44.65.15: no valid factory WAN identity for eth0=%pM; WAN CPU seed deferred\n",
			 cpu_mac);
	}

	rtl8197f_rtk_rd05_force_l2(priv, bcast, 1, RTL_RTK_RD05_HOST_MBR,
				       true, true, true, true, "l2-bcast-fid1");

	ether_addr_copy(priv->rd05_seeded_mac, cpu_mac);
	priv->rd05_mac_reseeds++;
	dev_info(priv->dev,
		 "%s rtl865x l2cpu v43.6: seed done runs=%llu ok=%llu fail=%llu last_status=0x%llx seeded-mac=%pM reseeds=%llu\n",
		 board,
		 (unsigned long long)priv->rd05_l2cpu_runs,
		 (unsigned long long)priv->rd05_l2cpu_ok,
		 (unsigned long long)priv->rd05_l2cpu_fail,
		 (unsigned long long)priv->rd05_l2cpu_last_status,
		 priv->rd05_seeded_mac,
		 (unsigned long long)priv->rd05_mac_reseeds);
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
	 * the swNic hooks to New_swNic_*.  Non-VG chips use the native six-dword
	 * format; VG builds add cacheline padding and program an eight-dword stride.
	 * Keep CPUICR1 and CPURPDCR0/CPUTPDCR0 consistent with the selected board.
	 */
	val |= RTL_RTK_CPUICR1_TXRX_DIV_LX | RTL_RTK_CPUICR1_LE |
	       RTL_RTK_CPUICR1_TSO_ID_SEL | RTL_RTK_CPUICR1_TX_GATHER |
	       RTL_RTK_CPUICR1_PKT_HDR_SHORTCUT_LSO;
	if (priv->desc_dwords > 6)
		val |= (priv->desc_dwords << RTL_RTK_CPUICR1_TXDESC_OFFSET) |
		       (priv->desc_dwords << RTL_RTK_CPUICR1_RXDESC_OFFSET);
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
	       RTL_RTK_CPUICR1_PKT_HDR_SHORTCUT_LSO;
	if (priv->desc_dwords > 6)
		val |= (priv->desc_dwords << RTL_RTK_CPUICR1_TXDESC_OFFSET) |
		       (priv->desc_dwords << RTL_RTK_CPUICR1_RXDESC_OFFSET);
	return val;
}

static void rtl8197f_rtk_program_tx_ring_geometry(struct rtl8197f_rtknet_priv *priv,
						  const char *reason)
{
	u32 tx_base = rtl8197f_rtk_hw_dma_addr(priv, priv->tx_desc_dma);
	u32 expected_dma_cr1 = (priv->tx_ring_size - 1) * priv->desc_stride;
	u32 txringcr, actual_base, actual_dma_cr1;
	bool mw5 = of_machine_is_compatible("tenda,nova-mw5");

	/* New_swNic_init() programs the ring-0 CDP, the byte offset of the last
	 * descriptor and tail-aware mode. RTL8197FS/non-VG then disables only TX
	 * rings 2/3, leaving the SDK's ring0/ring1 state-machine pair enabled.
	 * The old MW5 path attempted the RTL8197F-VG multi-ring value while CPUIF
	 * was stopped; hardware kept bootloader DMA_CR1=0x02ff02ff instead of the
	 * 64-entry x 24-byte value 0x000005e8.
	 */
	rtl8197f_rtk_write(priv, RTL_RTK_CPUTPDCR0, tx_base);
	rtl8197f_rtk_write(priv, RTL_RTK_DMA_CR1, expected_dma_cr1);
	rtl8197f_rtk_write(priv, RTL_RTK_DMA_CR4,
			   RTL_RTK_DMA_CR4_TX_RING0_TAIL_AWARE);

	if (priv->sdk_swcore_init) {
		if (mw5) {
			txringcr = rtl8197f_rtk_read(priv, RTL_RTK_TXRINGCR);
			/* The MW5 uses the OEM single-TX-ring path: DMA_CR1 only
			 * describes ring 0 and only CPUTPDCR0 is programmed.  Do not
			 * leave ring 1 enabled without a base/length pair.
			 */
			txringcr &= ~(RTL_RTK_TXRINGCR_TX_RING0_EN |
				      RTL_RTK_TXRINGCR_TX_RING1_EN |
				      RTL_RTK_TXRINGCR_TX_RING2_EN |
				      RTL_RTK_TXRINGCR_TX_RING3_EN);
			txringcr |= RTL_RTK_TXRINGCR_TX_RING0_EN;
		} else {
			txringcr = RTL_RTK_TXRINGCR_8197F_VG_INIT;
		}
		rtl8197f_rtk_write(priv, RTL_RTK_TXRINGCR, txringcr);
		priv->txringcr_sdk_writes++;
	}

	/* Publish ring geometry before readback and before CPUIF consumes it. */
	wmb();

	actual_base = rtl8197f_rtk_read(priv, RTL_RTK_CPUTPDCR0);
	actual_dma_cr1 = rtl8197f_rtk_read(priv, RTL_RTK_DMA_CR1);
	txringcr = rtl8197f_rtk_read(priv, RTL_RTK_TXRINGCR);
	if (actual_base != tx_base || actual_dma_cr1 != expected_dma_cr1 ||
	    (mw5 && ((txringcr & (RTL_RTK_TXRINGCR_TX_RING0_EN |
				  RTL_RTK_TXRINGCR_TX_RING1_EN |
				  RTL_RTK_TXRINGCR_TX_RING2_EN |
				  RTL_RTK_TXRINGCR_TX_RING3_EN)) !=
			       RTL_RTK_TXRINGCR_TX_RING0_EN)))
		dev_warn(priv->dev,
			 "%s txring sdk v43.6: %s rejected base=0x%08x/0x%08x dma_cr1=0x%08x/0x%08x txringcr=0x%08x\n",
			 mw5 ? "mw5" : "rd05", reason, actual_base, tx_base,
			 actual_dma_cr1, expected_dma_cr1, txringcr);
	else
		dev_info(priv->dev,
			 "%s txring sdk v43.6: %s base=0x%08x entries=%u stride=%u dma_cr1=0x%08x txringcr=0x%08x\n",
			 mw5 ? "mw5" : "rd05", reason, actual_base,
			 priv->tx_ring_size, priv->desc_stride, actual_dma_cr1,
			 txringcr);
}

static void rtl8197f_rtk_rd05_poststart_rearm(struct rtl8197f_rtknet_priv *priv,
					       const char *reason)
{
	u32 expected, actual, rx_base;

	if (!of_machine_is_compatible("xiaomi,r4-rd05") &&
	    !of_machine_is_compatible("tenda,nova-mw5"))
		return;

	/*
	 * RTL8197F hardware traces prove that descriptor-format/base writes made
	 * while CPUICR is stopped may be discarded.  Reapply the board-correct
	 * 6/8-dword format after TXCMD/RXCMD and TRXRDY are live.
	 *
	 * The RD05 hardware trace proved that writes made while CPUICR is stopped
	 * are discarded: CPURPDCR0 read back as zero and CPUICR1 retained the
	 * bootloader's 6-dword stride (0x00186180).  Reapply the SDK 8-dword
	 * format only after TXCMD/RXCMD and TRXRDY are live, then reset both ring
	 * current pointers.
	 */
	expected = rtl8197f_rtk_expected_cpuicr1(priv);
	rtl8197f_rtk_write(priv, RTL_RTK_CPUICR1, expected);
	readl(priv->base + RTL_RTK_CPUICR1);

	if (priv->rd05_sdk_newdesc_rx) {
		rtl8197f_rtk_rd05_program_newdesc_rx(priv, reason);
	} else if (priv->rd05_legacy_rx_enabled) {
		rtl8197f_rtk_rd05_program_legacy_rx(priv, reason);
	} else {
		rx_base = rtl8197f_rtk_hw_dma_addr(priv, priv->rx_desc_dma);
		rtl8197f_rtk_write(priv, RTL_RTK_CPURPDCR0, rx_base);
		rtl8197f_rtk_write(priv, RTL_RTK_CPURMDCR0, 0);
	}

	/* Repeat the complete TX geometry after CPUIF/TRXRDY is live.  The
	 * RTL8197F silently ignores some stopped-state geometry writes.
	 */
	rtl8197f_rtk_program_tx_ring_geometry(priv, reason);

	rtl8197f_rtk_kick_rx(priv);
	rtl8197f_rtk_assert_trxrdy(priv);

	actual = rtl8197f_rtk_read(priv, RTL_RTK_CPUICR1);
	if ((actual & (RTL_RTK_CPUICR1_TXDESC_MASK |
		       RTL_RTK_CPUICR1_RXDESC_MASK)) !=
	    (expected & (RTL_RTK_CPUICR1_TXDESC_MASK |
		         RTL_RTK_CPUICR1_RXDESC_MASK)))
		dev_warn(priv->dev,
			 "rtl8197f poststart v43.6: CPUICR1 descriptor format rejected: expected=0x%08x actual=0x%08x\n",
			 expected, actual);
	else
		dev_info(priv->dev,
			 "rtl8197f poststart v43.6: %s desc=%u/%u cpuicr1=0x%08x rxbase=0x%08x txbase=0x%08x dma_cr1=0x%08x txringcr=0x%08x rxdesc0=0x%08x\n",
			 reason, priv->desc_dwords, priv->desc_stride, actual,
			 rtl8197f_rtk_read(priv, RTL_RTK_CPURPDCR0),
			 rtl8197f_rtk_read(priv, RTL_RTK_CPUTPDCR0),
			 rtl8197f_rtk_read(priv, RTL_RTK_DMA_CR1),
			 rtl8197f_rtk_read(priv, RTL_RTK_TXRINGCR),
			 priv->rx_desc ? le32_to_cpu(rtl8197f_rtk_rx_desc(priv, 0)->opts1) : 0);
}

static void *rtl8197f_rtk_rx_data(struct rtl8197f_rtknet_priv *priv,
				   unsigned int idx)
{
	if (priv->rx_coherent_enabled && priv->rx_coherent_pool &&
	    idx < priv->rx_ring_size)
		return (u8 *)priv->rx_coherent_pool +
			idx * priv->rx_coherent_stride;
	if (priv->rx_page_pool_enabled && priv->rx_page && priv->rx_page[idx])
		return page_address(priv->rx_page[idx]) + priv->rx_headroom;
	if (priv->rx_skb && priv->rx_skb[idx])
		return priv->rx_skb[idx]->data;
	return NULL;
}

static int rtl8197f_rtk_page_pool_create(struct rtl8197f_rtknet_priv *priv)
{
	struct page_pool_params pp = {
		.order = 0,
		.pool_size = priv->rx_ring_size * 2,
		.flags = PP_FLAG_DMA_MAP | PP_FLAG_DMA_SYNC_DEV,
		.dma_dir = DMA_FROM_DEVICE,
		/* The DMA engine writes at rx_headroom, not at page offset 0.
		 * Keep page_pool's recycle sync aligned with the actual RX area.
		 */
		.max_len = priv->rx_buf_size,
		.offset = priv->rx_headroom,
		.nid = NUMA_NO_NODE,
		.dev = priv->dev,
		.napi = &priv->napi,
	};

	if (!priv->rx_page_pool_enabled)
		return 0;
	if (priv->rx_buf_size + priv->rx_headroom > PAGE_SIZE) {
		priv->rx_page_pool_enabled = false;
		priv->rx_page_pool_fallback++;
		return 0;
	}

	priv->rx_page_pool = page_pool_create(&pp);
	if (IS_ERR(priv->rx_page_pool)) {
		int err = PTR_ERR(priv->rx_page_pool);
		priv->rx_page_pool = NULL;
		priv->rx_page_pool_enabled = false;
		priv->rx_page_pool_fallback++;
		dev_warn(priv->dev, "v44.62 page_pool unavailable (%d), using skb DMA fallback\n", err);
		return 0;
	}
	return 0;
}

static void rtl8197f_rtk_page_pool_destroy(struct rtl8197f_rtknet_priv *priv)
{
	if (priv->rx_page_pool) {
		page_pool_destroy(priv->rx_page_pool);
		priv->rx_page_pool = NULL;
	}
}

static void rtl8197f_rtk_rx_checksum(struct rtl8197f_rtknet_priv *priv,
				     struct rtl8197f_rtk_desc *desc,
				     struct sk_buff *skb)
{
	u32 o4, o5;

	skb->ip_summed = CHECKSUM_NONE;
	if (!(priv->ndev->features & NETIF_F_RXCSUM))
		goto none;
	o4 = le32_to_cpu(READ_ONCE(desc->opts4));
	o5 = le32_to_cpu(READ_ONCE(desc->opts5));
	if (o4 & RTL_RTK_RX_FRAG)
		goto none;
	if ((o4 & RTL_RTK_RX_IPV4) &&
	    (o5 & RTL_RTK_RX_L3CSOK) && (o5 & RTL_RTK_RX_L4CSOK)) {
		skb->ip_summed = CHECKSUM_UNNECESSARY;
		priv->rx_csum_ok++;
		return;
	}
	if ((o4 & RTL_RTK_RX_IPV6) && (o5 & RTL_RTK_RX_L4CSOK)) {
		skb->ip_summed = CHECKSUM_UNNECESSARY;
		priv->rx_csum_ok++;
		return;
	}
none:
	priv->rx_csum_none++;
}

static unsigned int rtl8197f_rtk_tx_priority(struct sk_buff *skb)
{
	__be16 proto = skb->protocol;
	unsigned int p = skb->priority & 7;

	if (proto == htons(ETH_P_ARP) || proto == htons(ETH_P_PAE))
		return 7;
	return p;
}

static unsigned int rtl8197f_rtk_qid_from_priority(unsigned int p)
{
	if (p >= 6)
		return 2;
	if (p >= 3)
		return 1;
	return 0;
}

static bool rtl8197f_rtk_has_mw5_dsa_tag(const struct sk_buff *skb)
{
	return skb->len >= ETH_HLEN + RTL_RTK_MW5_CPU_TAG4_LEN &&
		get_unaligned_be16(skb->data + 2 * ETH_ALEN) == ETH_P_REALTEK;
}

static netdev_features_t rtl8197f_rtk_features_check(struct sk_buff *skb,
                                                      struct net_device *ndev,
                                                      netdev_features_t features)
{
    struct rtl8197f_rtknet_priv *priv = netdev_priv(ndev);

    if (!priv->is_mw5 || !rtl8197f_rtk_has_mw5_dsa_tag(skb))
        return features;

    /* v44.66.9: MW5 normal DSA traffic uses the upstream RTL8365MB
     * protocol-4 eight-byte tag.  The RTL865x L3/LSO parser has no proven
     * offset mode for either four- or eight-byte Realtek tags, so tagged
     * packets stay fail-closed for checksum/TSO while coherent DMA/BQL remain
     * available.  Do not infer the tag length from the inner EtherType here. */
    return features & ~(NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM |
                        NETIF_F_TSO | NETIF_F_TSO6);
}

static netdev_features_t rtl8197f_rtk_fix_features(struct net_device *ndev,
                                                   netdev_features_t features)
{
    struct rtl8197f_rtknet_priv *priv = netdev_priv(ndev);

    /* v44.65.17: the MW5 unified accelerator replaces every legacy NIC
     * acceleration switch. Keep checksum/SG/TSO disabled at the netdev layer
     * even if an old userspace or ethtool request tries to restore them. The
     * new hardware engine uses the proven coherent DMA + DSA egress path and
     * never requires the fixed-function RTL865x L3 parser for Realtek DSA tags. */
    if (priv->is_mw5) {
        features &= ~(NETIF_F_RXCSUM | NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM |
                      NETIF_F_SG | NETIF_F_TSO | NETIF_F_TSO6);
        return features;
    }

    /* Generic non-MW5 relationship: TSO needs SG + TX checksum. */
    if (priv->is_iball &&
        (!(features & NETIF_F_SG) ||
         !(features & (NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM))))
        features &= ~(NETIF_F_TSO | NETIF_F_TSO6);

    return features;
}

static void rtl8197f_rtk_apply_hw_qos(struct rtl8197f_rtknet_priv *priv)
{
	u32 qnum, map = 0;
	unsigned int p;

	if (!priv->swcore)
		return;

	qnum = readl(priv->swcore + RTL_RTK_SWCORE_QNUMCR);
	qnum &= ~RTL_RTK_QNUM_P0_MASK;

	/* v44.65.10 retains the stable MW5 baseline with a single P0
	 * hardware queue.  This removes the last always-on descriptor/SWCORE QoS
	 * manipulation from the classic Linux DSA + firewall4/NAT path.  Writing
	 * queue-count-minus-one = 0 also reverses a prior qos-on at runtime instead
	 * of merely stopping new descriptor priority bits. */
	if (!priv->hw_qos) {
		writel(qnum, priv->swcore + RTL_RTK_SWCORE_QNUMCR);
		readl(priv->swcore + RTL_RTK_SWCORE_QNUMCR);
		if (priv->is_mw5)
			dev_info(priv->dev,
				 "v44.65.10 hw-qos stable baseline: P0=1 queue qnum=0x%08x\n",
				 qnum);
		return;
	}

	qnum |= RTL_RTK_QNUM_3;
	writel(qnum, priv->swcore + RTL_RTK_SWCORE_QNUMCR);
	for (p = 0; p < 8; p++)
		map |= rtl8197f_rtk_qid_from_priority(p) << (p * 3);
	writel(map, priv->swcore + RTL_RTK_SWCORE_UPTCMCR2);
	readl(priv->swcore + RTL_RTK_SWCORE_UPTCMCR2);
	dev_info(priv->dev, "v44.62 hw-qos: P0=3 queues pri0-2->q0 pri3-5->q1 pri6-7->q2 qnum=0x%08x map=0x%08x\n",
		 qnum, map);
}

static int rtl8197f_rtk_alloc_rx_buf(struct rtl8197f_rtknet_priv *priv,
				     unsigned int idx)
{
	struct net_device *ndev = priv->ndev;
	struct rtl8197f_rtk_desc *desc = rtl8197f_rtk_rx_desc(priv, idx);
	dma_addr_t dma;

	if (priv->rx_coherent_enabled && priv->rx_coherent_pool) {
		dma = priv->rx_coherent_dma + idx * priv->rx_coherent_stride;
		priv->rx_skb[idx] = NULL;
		priv->rx_page[idx] = NULL;
		priv->rx_dma[idx] = dma;
	} else if (priv->rx_page_pool_enabled && priv->rx_page_pool) {
		struct page *page = page_pool_dev_alloc_pages(priv->rx_page_pool);
		if (!page)
			return -ENOMEM;
		dma = page_pool_get_dma_addr(page) + priv->rx_headroom;
		priv->rx_page[idx] = page;
		priv->rx_skb[idx] = NULL;
		priv->rx_dma[idx] = dma;
		priv->rx_page_pool_alloc++;
	} else {
		struct sk_buff *skb = netdev_alloc_skb_ip_align(ndev, priv->rx_buf_size);
		if (!skb)
			return -ENOMEM;
		dma = dma_map_single(priv->dev, skb->data, priv->rx_buf_size,
				     DMA_FROM_DEVICE);
		if (dma_mapping_error(priv->dev, dma)) {
			dev_kfree_skb_any(skb);
			return -ENOMEM;
		}
		priv->rx_skb[idx] = skb;
		priv->rx_page[idx] = NULL;
		priv->rx_dma[idx] = dma;
	}

	desc->addr = cpu_to_le32(rtl8197f_rtk_hw_dma_addr(priv, dma));
	desc->opts2 = 0;
	desc->opts3 = 0;
	desc->opts4 = 0;
	desc->opts5 = 0;
	rtl8197f_rtk_desc_clear_padding(priv, desc);
	desc->opts1 = cpu_to_le32(RTL_RTK_DESC_OWN |
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
	 * native 6- or 8-dword RX descriptor ring.  CPURMDCR0 is only meaningful for the
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
		 priv->rx_desc ? le32_to_cpu(rtl8197f_rtk_rx_desc(priv, 0)->opts1) : 0,
		 priv->rx_desc ? le32_to_cpu(rtl8197f_rtk_rx_desc(priv, 0)->opts2) : 0,
		 priv->rx_desc ? le32_to_cpu(rtl8197f_rtk_rx_desc(priv, 0)->opts3) : 0,
		 rtl8197f_rtk_hw_dma_addr(priv, priv->tx_desc_dma));
}

static void rtl8197f_rtk_free_rings(struct rtl8197f_rtknet_priv *priv)
{
	unsigned int i;

	rtl8197f_rtk_rd05_free_legacy_rx(priv);

	if (!priv->rx_coherent_enabled && (priv->rx_skb || priv->rx_page)) {
		for (i = 0; i < priv->rx_ring_size; i++) {
			if (priv->rx_page_pool_enabled && priv->rx_page && priv->rx_page[i]) {
				page_pool_put_full_page(priv->rx_page_pool, priv->rx_page[i], false);
				priv->rx_page[i] = NULL;
				continue;
			}
			if (!priv->rx_skb || !priv->rx_skb[i])
				continue;
			dma_unmap_single(priv->dev, priv->rx_dma[i],
					 priv->rx_buf_size, DMA_FROM_DEVICE);
			dev_kfree_skb_any(priv->rx_skb[i]);
		}
	}
	rtl8197f_rtk_page_pool_destroy(priv);
	if (priv->rx_coherent_pool) {
		dma_free_coherent(priv->dev, priv->rx_coherent_size,
				  priv->rx_coherent_pool, priv->rx_coherent_dma);
		priv->rx_coherent_pool = NULL;
		priv->rx_coherent_dma = 0;
		priv->rx_coherent_size = 0;
	}

	if (priv->tx_coherent_pool) {
		if (priv->tx_coherent_cpu && priv->tx_coherent_dma_slot) {
			for (i = 0; i < priv->tx_ring_size; i++) {
				if (!priv->tx_coherent_cpu[i])
					continue;
				dma_pool_free(priv->tx_coherent_pool,
					      priv->tx_coherent_cpu[i],
					      priv->tx_coherent_dma_slot[i]);
			}
		}
		dma_pool_destroy(priv->tx_coherent_pool);
		priv->tx_coherent_pool = NULL;
	}

	if (priv->tx_dma) {
		for (i = 0; i < priv->tx_ring_size; i++) {
			if (priv->tx_dma[i]) {
				if (priv->tx_dma_is_page && priv->tx_dma_is_page[i])
					dma_unmap_page(priv->dev, priv->tx_dma[i], priv->tx_len[i], DMA_TO_DEVICE);
				else
					dma_unmap_single(priv->dev, priv->tx_dma[i], priv->tx_len[i], DMA_TO_DEVICE);
			}
			if (priv->tx_skb && priv->tx_skb[i])
				dev_kfree_skb_any(priv->tx_skb[i]);
		}
	}

	kfree(priv->rx_skb);
	kfree(priv->rx_page);
	kfree(priv->rx_dma);
	kfree(priv->tx_skb);
	kfree(priv->tx_dma);
	kfree(priv->tx_len);
	kfree(priv->tx_pkt_len);
	kfree(priv->tx_dma_is_page);
	kfree(priv->tx_coherent_cpu);
	kfree(priv->tx_coherent_dma_slot);
	priv->rx_skb = NULL;
	priv->rx_page = NULL;
	priv->rx_dma = NULL;
	priv->tx_skb = NULL;
	priv->tx_dma = NULL;
	priv->tx_len = NULL;
	priv->tx_pkt_len = NULL;
	priv->tx_dma_is_page = NULL;
	priv->tx_coherent_cpu = NULL;
	priv->tx_coherent_dma_slot = NULL;
	priv->tx_coherent_stride = 0;

	if (priv->rx_desc) {
		dma_free_coherent(priv->dev,
				  priv->rx_ring_size * priv->desc_stride,
				  priv->rx_desc, priv->rx_desc_dma);
		priv->rx_desc = NULL;
	}

	if (priv->tx_desc) {
		dma_free_coherent(priv->dev,
				  priv->tx_ring_size * priv->desc_stride,
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
				priv->rx_ring_size * priv->desc_stride,
				&priv->rx_desc_dma, GFP_KERNEL);
	if (!priv->rx_desc)
		return -ENOMEM;

	priv->tx_desc = dma_alloc_coherent(priv->dev,
				priv->tx_ring_size * priv->desc_stride,
				&priv->tx_desc_dma, GFP_KERNEL);
	if (!priv->tx_desc) {
		ret = -ENOMEM;
		goto err_free;
	}
	memset(priv->rx_desc, 0, priv->rx_ring_size * priv->desc_stride);
	memset(priv->tx_desc, 0, priv->tx_ring_size * priv->desc_stride);

	priv->rx_skb = kcalloc(priv->rx_ring_size, sizeof(*priv->rx_skb), GFP_KERNEL);
	priv->rx_page = kcalloc(priv->rx_ring_size, sizeof(*priv->rx_page), GFP_KERNEL);
	priv->rx_dma = kcalloc(priv->rx_ring_size, sizeof(*priv->rx_dma), GFP_KERNEL);
	priv->tx_skb = kcalloc(priv->tx_ring_size, sizeof(*priv->tx_skb), GFP_KERNEL);
	priv->tx_dma = kcalloc(priv->tx_ring_size, sizeof(*priv->tx_dma), GFP_KERNEL);
	priv->tx_len = kcalloc(priv->tx_ring_size, sizeof(*priv->tx_len), GFP_KERNEL);
	priv->tx_pkt_len = kcalloc(priv->tx_ring_size, sizeof(*priv->tx_pkt_len), GFP_KERNEL);
	priv->tx_dma_is_page = kcalloc(priv->tx_ring_size, sizeof(*priv->tx_dma_is_page), GFP_KERNEL);
	if (priv->tx_coherent_enabled) {
		priv->tx_coherent_cpu = kcalloc(priv->tx_ring_size,
					       sizeof(*priv->tx_coherent_cpu), GFP_KERNEL);
		priv->tx_coherent_dma_slot = kcalloc(priv->tx_ring_size,
						    sizeof(*priv->tx_coherent_dma_slot), GFP_KERNEL);
	}
	if (!priv->rx_skb || !priv->rx_page || !priv->rx_dma || !priv->tx_skb ||
	    !priv->tx_dma || !priv->tx_len || !priv->tx_pkt_len || !priv->tx_dma_is_page ||
	    (priv->tx_coherent_enabled &&
	     (!priv->tx_coherent_cpu || !priv->tx_coherent_dma_slot))) {
		ret = -ENOMEM;
		goto err_free;
	}

	if (priv->rx_coherent_enabled) {
		priv->rx_coherent_stride = ALIGN(priv->rx_buf_size, 32);
		priv->rx_coherent_size = (size_t)priv->rx_ring_size *
					 priv->rx_coherent_stride;
		priv->rx_coherent_pool = dma_alloc_coherent(priv->dev,
				priv->rx_coherent_size, &priv->rx_coherent_dma, GFP_KERNEL);
		if (!priv->rx_coherent_pool) {
			priv->rx_coherent_alloc_fail++;
			dev_err(priv->dev,
				"v44.65.12 coherent RX pool allocation failed (%zu bytes)\n",
				priv->rx_coherent_size);
			ret = -ENOMEM;
			goto err_free;
		}
		memset(priv->rx_coherent_pool, 0, priv->rx_coherent_size);
		dev_info(priv->dev,
			 "v44.65.12 MW5 coherent RX bounce ring: slots=%u stride=%u bytes=%zu dma=%pad\n",
			 priv->rx_ring_size, priv->rx_coherent_stride,
			 priv->rx_coherent_size, &priv->rx_coherent_dma);
	}

	if (priv->tx_coherent_enabled) {
		priv->tx_coherent_stride = ALIGN(priv->rx_buf_size, 32);
		priv->tx_coherent_pool = dma_pool_create("rtl8197f-mw5-tx", priv->dev,
						 priv->tx_coherent_stride, 32, 0);
		if (!priv->tx_coherent_pool) {
			priv->tx_coherent_alloc_fail++;
			ret = -ENOMEM;
			goto err_free;
		}
		for (i = 0; i < priv->tx_ring_size; i++) {
			priv->tx_coherent_cpu[i] = dma_pool_alloc(priv->tx_coherent_pool,
							      GFP_KERNEL,
							      &priv->tx_coherent_dma_slot[i]);
			if (!priv->tx_coherent_cpu[i]) {
				priv->tx_coherent_alloc_fail++;
				ret = -ENOMEM;
				goto err_free;
			}
			memset(priv->tx_coherent_cpu[i], 0, priv->tx_coherent_stride);
		}
		dev_info(priv->dev,
			 "v44.66.0 MW5 coherent TX slots: slots=%u stride=%u total=%zu\n",
			 priv->tx_ring_size, priv->tx_coherent_stride,
			 (size_t)priv->tx_ring_size * priv->tx_coherent_stride);
	}

	ret = rtl8197f_rtk_page_pool_create(priv);
	if (ret)
		goto err_free;

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

	for (i = 0; i < priv->tx_ring_size; i++) {
		u32 tx_eor = 0;

		if (!of_machine_is_compatible("tenda,nova-mw5") &&
		    i == priv->tx_ring_size - 1)
			tx_eor = RTL_RTK_DESC_WRAP;
		rtl8197f_rtk_tx_desc(priv, i)->opts1 = cpu_to_le32(tx_eor);
	}

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

	/* Descriptor base registers.  CONFIG_RTL_SWITCH_NEW_DESCRIPTOR is common
	 * to RTL8197F, but the native stride is six dwords on non-VG RTL8197FS and
	 * eight dwords only on VG/cache-aligned builds.
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
	/* Program the board-specific New_swNic TX geometry. It is replayed after
	 * CPUICR/TRXRDY becomes live because stopped-state writes are not reliable.
	 */
	rtl8197f_rtk_program_tx_ring_geometry(priv, "hw-start");

	rtl8197f_rtk_apply_sdk_cpuif_init(priv);
	rtl8197f_rtk_apply_sdk_swcore_init(priv);
	rtl8197f_rtk_apply_hw_qos(priv);
	rtl8197f_rtk_apply_sdk_start_sidebands(priv);
	if (priv->is_iball)
		rtl8197f_rtk_init_iball_p0_rtl8211f(priv, "hw-start-iball");
	else if (of_machine_is_compatible("tenda,nova-mw5"))
		rtl8197f_rtk_init_p0_rgmii(priv, "hw-start-mw5");
	else
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
		dev_info(priv->dev,
			 "%s p0 cpu-pipeline v43.6: l2_ok=%llu l2_fail=%llu pipe_ok=%llu pipe_fail=%llu pvid_ok=%llu netif_ok=%llu acl_ok=%llu acl_fail=%llu ffcr=0x%llx\n",
			 rtl8197f_rtk_p0_board_name(),
			 (unsigned long long)priv->rd05_l2cpu_ok,
			 (unsigned long long)priv->rd05_l2cpu_fail,
			 (unsigned long long)priv->rd05_pipe_ok,
			 (unsigned long long)priv->rd05_pipe_fail,
			 (unsigned long long)priv->rd05_pvid_ok,
			 (unsigned long long)priv->rd05_netif_ok,
			 (unsigned long long)priv->rd05_acl_ok,
			 (unsigned long long)priv->rd05_acl_fail,
			 (unsigned long long)priv->rd05_ffcr_read);
	}

	if (priv->is_iball)
		rtl8197f_rtk_iball_seed_internal_switch(priv);

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

static void rtl8197f_rtk_mw5_tx_complete_trace(struct rtl8197f_rtknet_priv *priv,
					       unsigned int idx, u32 opts1)
{
	static atomic_t trace_count = ATOMIC_INIT(0);
	dma_addr_t desc_dma;
	int n;

	if (!of_machine_is_compatible("tenda,nova-mw5"))
		return;

	priv->mw5_tx_complete++;
	/* v44.57: completion trace is boot diagnostics only; after eight samples
	 * avoid both atomic RMW and CPUTPDCR0 MMIO reads in the clean hotpath. */
	if (likely(atomic_read(&trace_count) >= 8))
		return;
	n = atomic_inc_return(&trace_count);
	if (n > 8)
		return;
	desc_dma = priv->tx_desc_dma + idx * priv->desc_stride;
	dev_info(priv->dev,
		 "mw5 tx-complete v44.57: idx=%u desc=%pad opts1=0x%08x cputpdcr0=0x%08x head=%u tail=%u\n",
		 idx, &desc_dma, opts1,
		 rtl8197f_rtk_read(priv, RTL_RTK_CPUTPDCR0),
		 priv->tx_head, priv->tx_tail);
}

static unsigned int rtl8197f_rtk_tx_clean(struct rtl8197f_rtknet_priv *priv)
{
	struct net_device *ndev = priv->ndev;
	bool mw5 = priv->is_mw5;
	unsigned int done = 0, completed_pkts = 0, completed_bytes = 0;
	unsigned long flags;
	u32 hw_idx = 0;
	bool cdp_valid = false;

	spin_lock_irqsave(&priv->tx_lock, flags);
	if (unlikely(priv->tx_tail == priv->tx_head)) {
		priv->tx_clean_empty++;
		spin_unlock_irqrestore(&priv->tx_lock, flags);
		return 0;
	}

	/*
	 * v44.65.25: CPUTPDCR0 is only a progress hint, never sufficient proof that
	 * the DMA buffer behind a descriptor is no longer owned by SwitchCore.
	 * The v44.65.24 live capture reproduced frames whose Ethernet header had
	 * already been overwritten by unrelated payload while CDP-based cleanup had
	 * unmapped/freed an OWNed descriptor.  OEM symbols also expose a dedicated
	 * New_check_tx_done_desc_swCore_own() completion check.
	 *
	 * Therefore MW5 reclamation is fail-closed: tx_head is the Linux/OEM
	 * txCurrIdx equivalent, tx_tail is the software txDoneIdx equivalent, and
	 * CPUTPDCR0 is the hardware CDP. A descriptor before CDP is only eligible
	 * for inspection; dma_unmap()/skb free happens after hardware also clears
	 * OWN. CDP/OWN disagreement leaves txDoneIdx unchanged, so start_xmit()
	 * cannot reuse the slot. No reclaim_lag or time-based guess is permitted.
	 */
	if (mw5) {
		u32 pending, completed;

		cdp_valid = rtl8197f_rtk_tx_cdp_index(priv, &hw_idx);
		if (cdp_valid) {
			pending = (priv->tx_head + priv->tx_ring_size -
				   priv->tx_tail) % priv->tx_ring_size;
			completed = (hw_idx + priv->tx_ring_size -
				     priv->tx_tail) % priv->tx_ring_size;
			if (completed > pending) {
				priv->mw5_tx_cdp_invalid++;
				cdp_valid = false;
			}
		}
		/* OEM V212 keeps txCurrIdx, txDoneIdx and CPUTPDCR0 coherent.
		 * An invalid CDP is therefore not permission to reclaim by OWN alone.
		 * Hold txDoneIdx (tx_tail) and retry rather than reusing an uncertain
		 * slot, which is exactly the lifetime failure seen in corrupted PCAPs. */
		if (!cdp_valid) {
			priv->mw5_tx_cdp_invalid_defer++;
			spin_unlock_irqrestore(&priv->tx_lock, flags);
			return 0;
		}
	}

	while (priv->tx_tail != priv->tx_head) {
		unsigned int idx = priv->tx_tail;
		struct rtl8197f_rtk_desc *desc = rtl8197f_rtk_tx_desc(priv, idx);
		u32 opts1;
		struct sk_buff *skb;

		/* Observe device descriptor writes before deciding ownership. */
		dma_rmb();
		opts1 = le32_to_cpu(READ_ONCE(desc->opts1));

		if (cdp_valid) {
			u32 confirm;

			if (idx == hw_idx)
				break;

			priv->mw5_tx_cdp_seen++;

			/* CDP progressed past this slot, but OWN is authoritative for
			 * buffer lifetime. Re-read once after a DMA barrier to avoid
			 * turning a just-completed descriptor into a false mismatch.
			 */
			if (opts1 & RTL_RTK_DESC_OWN) {
				dma_rmb();
				confirm = le32_to_cpu(READ_ONCE(desc->opts1));
				if (confirm & RTL_RTK_DESC_OWN) {
					priv->mw5_tx_cdp_owned_clean++;
					priv->mw5_tx_own_defer++;
					if (!priv->mw5_tx_defer_active ||
					    priv->mw5_tx_defer_idx != idx) {
						priv->mw5_tx_completion_mismatch++;
						priv->mw5_tx_defer_idx = idx;
						priv->mw5_tx_own_defer_streak = 0;
						priv->mw5_tx_defer_active = true;
					}
					priv->mw5_tx_own_defer_streak++;
					if (priv->mw5_tx_own_defer_streak >
					    priv->mw5_tx_own_defer_max)
						priv->mw5_tx_own_defer_max =
							priv->mw5_tx_own_defer_streak;
					break;
				}
				opts1 = confirm;
			}

			priv->mw5_tx_cdp_clean++;
			priv->mw5_tx_own_completed++;
			if (priv->mw5_tx_defer_active &&
			    priv->mw5_tx_defer_idx == idx) {
				priv->mw5_tx_defer_active = false;
				priv->mw5_tx_own_defer_streak = 0;
			}
		} else {
			/* Non-MW5 boards retain the legacy OWN-only cleanup path. */
			if (opts1 & RTL_RTK_DESC_OWN)
				break;
		}

		dma_rmb();
		rtl8197f_rtk_mw5_tx_complete_trace(priv, idx, opts1);
		skb = priv->tx_skb[idx];
		if (priv->tx_dma[idx]) {
			if (priv->tx_dma_is_page[idx])
				dma_unmap_page(priv->dev, priv->tx_dma[idx], priv->tx_len[idx], DMA_TO_DEVICE);
			else
				dma_unmap_single(priv->dev, priv->tx_dma[idx], priv->tx_len[idx], DMA_TO_DEVICE);
			priv->tx_dma[idx] = 0;
			priv->tx_len[idx] = 0;
			priv->tx_dma_is_page[idx] = false;
		}
		if (skb) {
			completed_pkts++;
			completed_bytes += priv->tx_pkt_len[idx];
			ndev->stats.tx_packets++;
			ndev->stats.tx_bytes += priv->tx_pkt_len[idx];
			dev_consume_skb_any(skb);
			priv->tx_skb[idx] = NULL;
			priv->tx_pkt_len[idx] = 0;
		}

		/* OEM USE_SWITCH_TX_CDP only advances the software done index; it
		 * does not rewrite a completed 24-byte descriptor.  On MW5 leave the
		 * hardware-updated slot untouched until start_xmit() reuses it.  This
		 * also avoids writes into a cache line shared with the next descriptor.
		 */
		if (!mw5) {
			desc->addr = 0;
			desc->opts2 = 0;
			desc->opts3 = 0;
			desc->opts4 = 0;
			desc->opts5 = 0;
			rtl8197f_rtk_desc_clear_padding(priv, desc);
			desc->opts1 = cpu_to_le32(idx == priv->tx_ring_size - 1 ?
						 RTL_RTK_DESC_WRAP : 0);
		}
		priv->tx_tail = rtl8197f_rtk_next(idx, priv->tx_ring_size);
		done++;
	}

	if (netif_queue_stopped(ndev) &&
	    rtl8197f_rtk_tx_avail(priv) > MAX_SKB_FRAGS + 1)
		netif_wake_queue(ndev);

	priv->tx_bql_completed_pkts += completed_pkts;
	priv->tx_bql_completed_bytes += completed_bytes;
	spin_unlock_irqrestore(&priv->tx_lock, flags);

	if (completed_pkts)
		netdev_completed_queue(ndev, completed_pkts, completed_bytes);

	return done;
}

static void rtl8197f_rtk_tx_reset(struct rtl8197f_rtknet_priv *priv)
{
	unsigned long flags;
	unsigned int i;

	spin_lock_irqsave(&priv->tx_lock, flags);
	for (i = 0; i < priv->tx_ring_size; i++) {
		if (priv->tx_dma[i]) {
			if (priv->tx_dma_is_page[i])
				dma_unmap_page(priv->dev, priv->tx_dma[i], priv->tx_len[i], DMA_TO_DEVICE);
			else
				dma_unmap_single(priv->dev, priv->tx_dma[i], priv->tx_len[i], DMA_TO_DEVICE);
		}
		if (priv->tx_skb[i])
			dev_kfree_skb_any(priv->tx_skb[i]);

		priv->tx_skb[i] = NULL;
		priv->tx_dma[i] = 0;
		priv->tx_len[i] = 0;
		priv->tx_pkt_len[i] = 0;
		priv->tx_dma_is_page[i] = false;
		rtl8197f_rtk_tx_desc(priv, i)->addr = 0;
		rtl8197f_rtk_tx_desc(priv, i)->opts2 = 0;
		rtl8197f_rtk_tx_desc(priv, i)->opts3 = 0;
		rtl8197f_rtk_tx_desc(priv, i)->opts4 = 0;
		rtl8197f_rtk_tx_desc(priv, i)->opts5 = 0;
		rtl8197f_rtk_desc_clear_padding(priv, rtl8197f_rtk_tx_desc(priv, i));
		if (!of_machine_is_compatible("tenda,nova-mw5") &&
		    i == priv->tx_ring_size - 1)
			rtl8197f_rtk_tx_desc(priv, i)->opts1 =
				cpu_to_le32(RTL_RTK_DESC_WRAP);
		else
			rtl8197f_rtk_tx_desc(priv, i)->opts1 = 0;
	}
	priv->tx_head = 0;
	priv->tx_tail = 0;
	priv->mw5_tx_defer_active = false;
	priv->mw5_tx_own_defer_streak = 0;
	spin_unlock_irqrestore(&priv->tx_lock, flags);
	netdev_reset_queue(priv->ndev);
}



#define RTL_RTK_RD05_RXTRACE_MAX_LOGS	16

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

	if (!rtl8197f_rtk_p0_external_switch_board())
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
		if (priv->rx_page_pool_enabled && priv->rx_page && priv->rx_page[idx])
			old_skb = (struct sk_buff *)priv->rx_page[idx]; /* non-NULL ownership sentinel */
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

static bool rtl8197f_rtk_mw5_known_payload_proto(u16 proto)
{
	switch (proto) {
	case ETH_P_IP:
	case ETH_P_IPV6:
	case ETH_P_ARP:
	case ETH_P_8021Q:
#ifdef ETH_P_8021AD
	case ETH_P_8021AD:
#endif
	case ETH_P_PAE:
	case ETH_P_PPP_DISC:
	case ETH_P_PPP_SES:
#ifdef ETH_P_LLDP
	case ETH_P_LLDP:
#endif
	case 0x88e1: /* HomePlug AV control traffic seen on the MW5 WAN side. */
		return true;
	default:
		return false;
	}
}

static bool rtl8197f_rtk_mw5_recover_source_port(struct rtl8197f_rtknet_priv *priv,
						  struct sk_buff *skb,
						  u8 *source_port)
{
	u8 wan_mac[ETH_ALEN];
	const u8 *da = skb->data;
	const u8 *sa = skb->data + ETH_ALEN;
	const u8 *base = priv->ndev->dev_addr;

	/* v44.65.9: Never guess a DSA slave for broadcast/multicast or a local
	 * reflection.  Only router-destined unicast has an unambiguous physical
	 * ingress identity on the MW5 routed topology:
	 *   base MAC     -> LAN P1
	 *   factory+7 MAC -> WAN P3
	 * This is the same factory+7 invariant already used by the rtl865x L2 CPU
	 * seed and by netifd for the routed WAN slave.
	 */
	if (is_multicast_ether_addr(da) || ether_addr_equal(sa, base) ||
	    !is_valid_ether_addr(base) || is_local_ether_addr(base))
		return false;

	ether_addr_copy(wan_mac, base);
	eth_addr_add(wan_mac, 7);
	if (is_valid_ether_addr(wan_mac) && ether_addr_equal(da, wan_mac)) {
		*source_port = 3;
		return true;
	}

	if (ether_addr_equal(da, base)) {
		*source_port = 1;
		return true;
	}

	return false;
}

static bool rtl8197f_rtk_mw5_corrupt_window(struct rtl8197f_rtknet_priv *priv,
					      const struct sk_buff *skb,
					      u16 proto, u16 proto4,
					      u8 source_port)
{
	const u8 *da = skb->data;
	const u8 *sa = skb->data + ETH_ALEN;
	unsigned long now = jiffies;

	if (source_port != 0 || rtl8197f_rtk_mw5_known_payload_proto(proto) ||
	    (proto4 && rtl8197f_rtk_mw5_known_payload_proto(proto4)))
		return false;

	/* A source MAC on a received Ethernet frame must be a non-zero unicast.
	 * The post-Speedtest failure repeatedly produced zero/multicast source MACs
	 * together with SPA0 and unknown EtherTypes. Treat only that narrow shape as
	 * a corruption candidate so legitimate uncommon EtherTypes stay untouched.
	 */
	if (!is_zero_ether_addr(da) && is_valid_ether_addr(sa))
		return false;

	if (!priv->mw5_rx_corrupt_window_start ||
	    time_after(now, priv->mw5_rx_corrupt_window_start +
		       msecs_to_jiffies(RTL_RTK_MW5_RX_CORRUPT_WINDOW_MS))) {
		priv->mw5_rx_corrupt_window_start = now;
		priv->mw5_rx_corrupt_window_count = 1;
	} else {
		priv->mw5_rx_corrupt_window_count++;
	}

	if (priv->mw5_rx_corrupt_window_count < RTL_RTK_MW5_RX_CORRUPT_WINDOW_LIMIT)
		return false;

	priv->mw5_rx_corrupt_clusters++;
	priv->mw5_rx_catchup_pending = true;
	priv->mw5_rx_corrupt_window_start = 0;
	priv->mw5_rx_corrupt_window_count = 0;
	return true;
}

static int rtl8197f_rtk_mw5_prepare_dsa_rx(struct rtl8197f_rtknet_priv *priv,
					       struct rtl8197f_rtk_desc *desc,
					       struct sk_buff *skb)
{
	u16 proto, proto4 = 0, proto8 = 0;
	u32 opts4;
	u8 source_port;
	bool da_fallback = false;
	bool repair_inplace = false;

	if (!of_machine_is_compatible("tenda,nova-mw5") ||
	    !priv->p0_cpu_tag_passthrough || skb->len < ETH_HLEN)
		return 0;

	proto = get_unaligned_be16(skb->data + 2 * ETH_ALEN);
	if (proto == ETH_P_REALTEK) {
		priv->mw5_rx_tag_passthrough++;
		priv->mw5_rx_invalid_streak = 0;
		return 0;
	}
	if (skb->len >= ETH_HLEN + RTL_RTK_MW5_CPU_TAG4_LEN)
		proto4 = get_unaligned_be16(skb->data + 2 * ETH_ALEN +
					  RTL_RTK_MW5_CPU_TAG4_LEN);
	if (skb->len >= ETH_HLEN + RTL_RTK_MW5_CPU_TAG8_LEN)
		proto8 = get_unaligned_be16(skb->data + 2 * ETH_ALEN +
					  RTL_RTK_MW5_CPU_TAG8_LEN);

	/* Real MW5 captures prove opts4[15:13] for clean stripped-tag packets.
	 * Under sustained v44.65.8 load, 155 frames instead reported SPA0 while
	 * the switch/rings remained alive; 62 were unicast.  Recover only the
	 * unambiguous routed-MAC cases and leave multicast/unknown traffic for the
	 * tagger's v44.62.14 fail-closed anti-XDSA-loop guard.
	 */
	opts4 = le32_to_cpu(desc->opts4);
	source_port = (opts4 >> RTL_RTK_MW5_RX_SPA_SHIFT) &
		      RTL_RTK_MW5_RX_SPA_MASK;
	if (source_port != 1 && source_port != 3) {
		priv->mw5_rx_tag_invalid_port++;
		if (source_port < ARRAY_SIZE(priv->mw5_rx_invalid_spa))
			priv->mw5_rx_invalid_spa[source_port]++;
		da_fallback = rtl8197f_rtk_mw5_recover_source_port(priv, skb,
							       &source_port);
	}

	if (source_port != 1 && source_port != 3) {
		u16 effective_proto = proto;
		u8 wan_mac[ETH_ALEN];
		const u8 *base = priv->ndev->dev_addr;
		const u8 *sa = skb->data + ETH_ALEN;

		/* v44.65.14: retain a consecutive-invalid diagnostic only. v44.65.13
		 * reset the live RX ring after eight such frames; hardware testing showed
		 * 851 resets in normal operation, so automatic ring reinitialisation is
		 * forbidden here. Unknown frames still remain fail-closed at the DSA
		 * master boundary. */
		priv->mw5_rx_invalid_streak++;
		if (priv->mw5_rx_invalid_streak > priv->mw5_rx_invalid_streak_max)
			priv->mw5_rx_invalid_streak_max = priv->mw5_rx_invalid_streak;
		priv->mw5_rx_tag_unclassified++;
		if (rtl8197f_rtk_mw5_known_payload_proto(proto))
			priv->mw5_rx_unclassified_known_proto++;
		if (proto8 && rtl8197f_rtk_mw5_known_payload_proto(proto8))
			priv->mw5_rx_unclassified_proto4_known++;
		if (is_valid_ether_addr(base) && !is_local_ether_addr(base)) {
			if (ether_addr_equal(sa, base))
				priv->mw5_rx_unclassified_sa_lan++;
			ether_addr_copy(wan_mac, base);
			eth_addr_add(wan_mac, 7);
			if (is_valid_ether_addr(wan_mac) && ether_addr_equal(sa, wan_mac))
				priv->mw5_rx_unclassified_sa_wan++;
		}

		/* v44.65.11: keep a tiny bounded sample.  If the switch-side parser
		 * fix does not eliminate this class, the next UART report will show
		 * whether these are CPU TX reflections, structurally shifted frames,
		 * or genuinely unknown external traffic without flooding the console.
		 */
		if (priv->mw5_rx_tag_unclassified <= 16)
			dev_info(priv->dev,
				 "mw5 dsa-rx v44.66.9 unclassified: spa=%u opts4=0x%08x proto0=0x%04x proto4=0x%04x proto8=0x%04x da=%pM sa=%pM len=%u\n",
				 source_port, opts4, proto, proto4, proto8,
				 skb->data, sa, skb->len);

		if (is_broadcast_ether_addr(skb->data))
			priv->mw5_rx_invalid_bcast++;
		else if (is_multicast_ether_addr(skb->data))
			priv->mw5_rx_invalid_mcast++;
		else
			priv->mw5_rx_invalid_ucast++;

		if (proto == ETH_P_8021Q && skb->len >= ETH_HLEN + VLAN_HLEN) {
			priv->mw5_rx_invalid_vlan++;
			effective_proto = get_unaligned_be16(skb->data +
						       2 * ETH_ALEN + VLAN_HLEN);
		}

		switch (effective_proto) {
		case ETH_P_IP:
			priv->mw5_rx_invalid_ipv4++;
			break;
		case ETH_P_IPV6:
			priv->mw5_rx_invalid_ipv6++;
			break;
		case ETH_P_ARP:
			priv->mw5_rx_invalid_arp++;
			break;
		case ETH_P_REALTEK:
			priv->mw5_rx_invalid_realtek++;
			break;
		default:
			priv->mw5_rx_invalid_other++;
			break;
		}

		if (rtl8197f_rtk_mw5_corrupt_window(priv, skb, proto, proto8,
						      source_port)) {
			priv->mw5_rx_invalid_drop++;
			return -EPROTO;
		}

		if (READ_ONCE(mw5_drop_invalid_untagged)) {
			priv->mw5_rx_invalid_drop++;
			return -EPROTO;
		}
		return 0;
	}

	priv->mw5_rx_invalid_streak = 0;

	/* v44.66.9 uses the rtl8365mb native eight-byte protocol-4 tag. Two
	 * fallback shapes remain possible on the RTL8197F P0 RX path:
	 *
	 *  A) the external CPU tag was completely stripped: byte 12 is already a
	 *     normal EtherType/VLAN. Insert eight bytes before it;
	 *  B) eight tag bytes are stale/corrupt: byte 12 is bogus but byte 20 is a
	 *     plausible inner EtherType/VLAN. Rewrite those eight bytes in place.
	 *
	 * Normal hardware operation should preserve the switch-generated tag, so
	 * this remains a descriptor-source fallback only.
	 */
	if (da_fallback) {
		if (rtl8197f_rtk_mw5_known_payload_proto(proto)) {
			repair_inplace = false;
		} else if (proto8 && rtl8197f_rtk_mw5_known_payload_proto(proto8)) {
			repair_inplace = true;
		} else {
			priv->mw5_rx_tag_unclassified++;
			if (priv->mw5_rx_tag_unclassified <= 32)
				dev_info(priv->dev,
					 "mw5 dsa-rx v44.66.9: DA-derived port=%u but no structural tag proof proto0=0x%04x proto4=0x%04x proto8=0x%04x da=%pM sa=%pM len=%u; leave fail-closed\n",
					 source_port, proto, proto4, proto8, skb->data,
					 skb->data + ETH_ALEN, skb->len);
			return 0;
		}
		if (source_port == 3)
			priv->mw5_rx_tag_fallback_wan_da++;
		else
			priv->mw5_rx_tag_fallback_lan_da++;
	} else if (!rtl8197f_rtk_mw5_known_payload_proto(proto) &&
		   proto8 && rtl8197f_rtk_mw5_known_payload_proto(proto8)) {
		repair_inplace = true;
	}

	if (repair_inplace) {
		put_unaligned_be16(ETH_P_REALTEK, skb->data + 2 * ETH_ALEN);
		put_unaligned_be16(RTL_RTK_MW5_RTL8_PROTO_WORD,
				   skb->data + 2 * ETH_ALEN + 2);
		put_unaligned_be16(0, skb->data + 2 * ETH_ALEN + 4);
		put_unaligned_be16(source_port & 0xf,
				   skb->data + 2 * ETH_ALEN + 6);
		priv->mw5_rx_tag_repaired_inplace++;
		if (priv->mw5_rx_tag_repaired_inplace <= 32)
			dev_info(priv->dev,
				 "mw5 dsa-rx v44.66.9: repair rtl8_4 source-port=%u old0=0x%04x inner8=0x%04x da=%pM len=%u\n",
				 source_port, proto, proto8, skb->data, skb->len);
		return 0;
	}

	if (skb_tailroom(skb) < RTL_RTK_MW5_CPU_TAG8_LEN &&
	    pskb_expand_head(skb, 0, RTL_RTK_MW5_CPU_TAG8_LEN, GFP_ATOMIC)) {
		priv->mw5_rx_tag_expand_fail++;
		return -ENOMEM;
	}

	/* Tag completely absent: rebuild the upstream rtl8_4 switch-to-CPU layout
	 * before master eth_type_trans():
	 * DA | SA | 8899 | 0400 | 0000 | source-port | EtherType ...
	 */
	skb_put(skb, RTL_RTK_MW5_CPU_TAG8_LEN);
	memmove(skb->data + 2 * ETH_ALEN + RTL_RTK_MW5_CPU_TAG8_LEN,
		skb->data + 2 * ETH_ALEN,
		skb->len - RTL_RTK_MW5_CPU_TAG8_LEN - 2 * ETH_ALEN);
	put_unaligned_be16(ETH_P_REALTEK, skb->data + 2 * ETH_ALEN);
	put_unaligned_be16(RTL_RTK_MW5_RTL8_PROTO_WORD,
			   skb->data + 2 * ETH_ALEN + 2);
	put_unaligned_be16(0, skb->data + 2 * ETH_ALEN + 4);
	put_unaligned_be16(source_port & 0xf, skb->data + 2 * ETH_ALEN + 6);
	priv->mw5_rx_tag_synthesized++;

	if (priv->mw5_rx_tag_synthesized <= 32)
		dev_info(priv->dev,
			 "mw5 dsa-rx v44.66.9: synth rtl8_4 source-port=%u encap=0x%04x da=%pM len=%u fallback=%u\n",
			 source_port, proto, skb->data, skb->len, da_fallback);

	return 0;
}

static void rtl8197f_rtk_mw5_rxdesc_trace(struct rtl8197f_rtknet_priv *priv,
					  unsigned int idx,
					      struct rtl8197f_rtk_desc *desc,
					      u32 opts1, u32 opts2,
					      bool cdp_valid, u32 hw_idx)
{
	dma_addr_t desc_dma;
	u32 actual_addr, expected_addr;

	if (!of_machine_is_compatible("tenda,nova-mw5") ||
	    priv->mw5_rx_desc_trace >= 64)
		return;

	priv->mw5_rx_desc_trace++;
	desc_dma = priv->rx_desc_dma + idx * priv->desc_stride;
	actual_addr = le32_to_cpu(READ_ONCE(desc->addr));
	expected_addr = rtl8197f_rtk_hw_dma_addr(priv, priv->rx_dma[idx]);
	if (actual_addr != expected_addr)
		priv->mw5_rx_addr_mismatch++;

	dev_info(priv->dev,
		 "mw5 rxdesc v43.11: idx=%u v=%p dma=%pad cdp_valid=%u cdp_idx=%u own=%u addr=0x%08x expected=0x%08x opts=%08x/%08x/%08x/%08x/%08x stride=%u\n",
		 idx, desc, &desc_dma, cdp_valid, hw_idx,
		 !!(opts1 & RTL_RTK_DESC_OWN), actual_addr, expected_addr,
		 opts1, opts2, le32_to_cpu(READ_ONCE(desc->opts3)),
		 le32_to_cpu(READ_ONCE(desc->opts4)),
		 le32_to_cpu(READ_ONCE(desc->opts5)), priv->desc_stride);
}

static void rtl8197f_mw5_rx_bad(struct rtl8197f_rtknet_priv *priv,
				unsigned int idx,
				struct rtl8197f_rtk_desc *desc,
				const char *reason)
{
	void *data;
	u32 cdp;

	if (!of_machine_is_compatible("tenda,nova-mw5") ||
	    priv->mw5_rx_bad_snapshot >= 16)
		return;

	priv->mw5_rx_bad_snapshot++;
	data = rtl8197f_rtk_rx_data(priv, idx);
	cdp = rtl8197f_rtk_read(priv, RTL_RTK_CPURPDCR0);
	dev_info(priv->dev,
		 "mw5 rx-bad v43.11: reason=%s idx=%u cdp=0x%08x opts=%08x/%08x/%08x/%08x/%08x addr=0x%08x\n",
		 reason, idx, cdp,
		 le32_to_cpu(READ_ONCE(desc->opts1)),
		 le32_to_cpu(READ_ONCE(desc->opts2)),
		 le32_to_cpu(READ_ONCE(desc->opts3)),
		 le32_to_cpu(READ_ONCE(desc->opts4)),
		 le32_to_cpu(READ_ONCE(desc->opts5)),
		 le32_to_cpu(READ_ONCE(desc->addr)));

	if (!data)
		return;

	if (!priv->rx_coherent_enabled)
		dma_sync_single_for_cpu(priv->dev, priv->rx_dma[idx],
					priv->rx_buf_size, DMA_FROM_DEVICE);
	print_hex_dump(KERN_INFO, "mw5 rx-bad v44.65.14 data: ", DUMP_PREFIX_OFFSET,
		       16, 1, data, min_t(u32, priv->rx_buf_size, 64),
		       false);
	if (!priv->rx_coherent_enabled)
		dma_sync_single_for_device(priv->dev, priv->rx_dma[idx],
					   priv->rx_buf_size, DMA_FROM_DEVICE);
}

static void rtl8197f_rtk_iball_rx_vlan(struct rtl8197f_rtknet_priv *priv,
					struct rtl8197f_rtk_desc *desc,
					struct sk_buff *skb)
{
	u32 opts4, spa;
	u16 vid = 0;

	if (!priv->is_iball || !priv->internal_vlan_split)
		return;

	opts4 = le32_to_cpu(READ_ONCE(desc->opts4));
	spa = (opts4 & RTL_RTK_RX_SPA_MASK) >> RTL_RTK_RX_SPA_SHIFT;
	if (spa == 0) {
		vid = priv->wan_vid;
		priv->iball_rx_wan++;
	} else if (spa >= 1 && spa <= 4) {
		vid = priv->lan_vid;
		priv->iball_rx_lan++;
	} else {
		priv->iball_rx_unknown++;
		return;
	}

	__vlan_hwaccel_put_tag(skb, htons(ETH_P_8021Q), vid);
}

static bool rtl8197f_rtk_mw5_rx_desc_stable(struct rtl8197f_rtknet_priv *priv,
					       struct rtl8197f_rtk_desc *desc,
					       u32 *opts1, u32 *opts2)
{
	u32 a1, a2, a3, a4, a5, aa;
	u32 b1, b2, b3, b4, b5, ba;
	unsigned int retry;

	if (!priv->is_mw5) {
		*opts1 = le32_to_cpu(READ_ONCE(desc->opts1));
		*opts2 = le32_to_cpu(READ_ONCE(desc->opts2));
		return !(*opts1 & RTL_RTK_DESC_OWN);
	}

	/* RTL8197FS exposes a 24-byte descriptor over a non-coherent MIPS/
	 * switch-core boundary. OWN is necessary but v44.65.10 proved it is not
	 * sufficient as the only completion fence: after a line-rate burst the CPU
	 * could observe a cleared OWN while neighbouring descriptor words were still
	 * changing. Require a full, repeated snapshot before the payload is copied. */
	for (retry = 0; retry < 4; retry++) {
		dma_rmb();
		a1 = le32_to_cpu(READ_ONCE(desc->opts1));
		a2 = le32_to_cpu(READ_ONCE(desc->opts2));
		a3 = le32_to_cpu(READ_ONCE(desc->opts3));
		a4 = le32_to_cpu(READ_ONCE(desc->opts4));
		a5 = le32_to_cpu(READ_ONCE(desc->opts5));
		aa = le32_to_cpu(READ_ONCE(desc->addr));
		dma_rmb();
		b1 = le32_to_cpu(READ_ONCE(desc->opts1));
		b2 = le32_to_cpu(READ_ONCE(desc->opts2));
		b3 = le32_to_cpu(READ_ONCE(desc->opts3));
		b4 = le32_to_cpu(READ_ONCE(desc->opts4));
		b5 = le32_to_cpu(READ_ONCE(desc->opts5));
		ba = le32_to_cpu(READ_ONCE(desc->addr));

		if (!(b1 & RTL_RTK_DESC_OWN) && a1 == b1 && a2 == b2 &&
		    a3 == b3 && a4 == b4 && a5 == b5 && aa == ba) {
			*opts1 = b1;
			*opts2 = b2;
			return true;
		}
		priv->rx_desc_stable_retry++;
		udelay(1);
	}

	priv->rx_desc_unstable++;
	return false;
}

static void rtl8197f_rtk_mw5_rearm_rx_slot(struct rtl8197f_rtknet_priv *priv,
					       unsigned int idx)
{
	struct rtl8197f_rtk_desc *desc = rtl8197f_rtk_rx_desc(priv, idx);

	if (priv->rx_coherent_enabled && priv->rx_coherent_pool)
		memset(rtl8197f_rtk_rx_data(priv, idx), 0,
		       min_t(u32, priv->rx_buf_size, 64));

	desc->addr = cpu_to_le32(rtl8197f_rtk_hw_dma_addr(priv, priv->rx_dma[idx]));
	desc->opts2 = 0;
	desc->opts3 = 0;
	desc->opts4 = 0;
	desc->opts5 = 0;
	rtl8197f_rtk_desc_clear_padding(priv, desc);
	dma_wmb();
	desc->opts1 = cpu_to_le32(RTL_RTK_DESC_OWN |
			(priv->rx_buf_size << RTL_RTK_RX_EXTSIZE_SHIFT) |
			(idx == priv->rx_ring_size - 1 ? RTL_RTK_DESC_WRAP : 0));
}

static unsigned int rtl8197f_rtk_mw5_rx_catchup(struct rtl8197f_rtknet_priv *priv)
{
	u64 available;
	unsigned int dropped = 0;
	u32 hw_idx;

	if (!priv->is_mw5 || !READ_ONCE(priv->mw5_rx_catchup_pending) ||
	    !priv->rx_coherent_enabled)
		return 0;

	if (!rtl8197f_rtk_mw5_rx_update_hw_seq(priv, &hw_idx))
		return 0;

	if (priv->mw5_rx_hw_seq <= priv->mw5_rx_sw_seq)
		return 0;

	available = priv->mw5_rx_hw_seq - priv->mw5_rx_sw_seq;
	if (available > priv->rx_ring_size)
		available = priv->rx_ring_size;

	/* Drop only descriptors the monotonic producer sequence proves DMA has
	 * already passed. CPURPDCR0 itself is never rewritten, so TX/DSA/WLAN and
	 * the live hardware producer stay untouched. This merely catches the
	 * software consumer up to the current hardware generation.
	 */
	while (available--) {
		unsigned int idx = priv->rx_tail;

		rtl8197f_rtk_mw5_rearm_rx_slot(priv, idx);
		priv->rx_tail = rtl8197f_rtk_next(idx, priv->rx_ring_size);
		priv->mw5_rx_sw_seq++;
		dropped++;
	}

	WRITE_ONCE(priv->mw5_rx_catchup_pending, false);
	if (dropped) {
		priv->mw5_rx_catchups++;
		priv->mw5_rx_catchup_descs += dropped;
		priv->ndev->stats.rx_dropped += dropped;
		dma_wmb();
		rtl8197f_rtk_assert_trxrdy(priv);
	}

	return dropped;
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

	if (priv->is_mw5) {
		rtl8197f_rtk_mw5_rx_update_hw_seq(priv, NULL);
		rtl8197f_rtk_mw5_rx_catchup(priv);
	}

	while (work_done < budget) {
		unsigned int idx = priv->rx_tail;
		struct rtl8197f_rtk_desc *desc = rtl8197f_rtk_rx_desc(priv, idx);
		struct sk_buff *old_skb, *new_skb;
		dma_addr_t new_dma;
		u32 opts1, opts2, len, hw_idx = 0;
		bool cdp_valid, cdp_advanced;

		opts1 = le32_to_cpu(READ_ONCE(desc->opts1));

		/* v44.65.18 MW5 load-stall fix: OWN + stable snapshot is the
		 * authoritative completion contract. CPURPDCR0 is diagnostic only on
		 * this non-coherent 24-byte descriptor implementation; using it as a
		 * readiness gate caused a self-sustaining zero-work NAPI loop at line
		 * rate. This also removes one MMIO read from every MW5 RX packet.
		 */
		if (priv->is_mw5) {
			cdp_valid = priv->mw5_rx_seq_valid;
			hw_idx = priv->mw5_rx_hw_last_idx;
			cdp_advanced = cdp_valid &&
				       priv->mw5_rx_hw_seq > priv->mw5_rx_sw_seq;
			if (opts1 & RTL_RTK_DESC_OWN) {
				priv->mw5_rx_own_wait++;
				priv->mw5_rx_empty_wait++;
				break;
			}

			/* v44.65.23: index-only idx!=CDP is ambiguous after a ring
			 * wrap. Do not consume an OWN-cleared slot unless the monotonic
			 * producer sequence is strictly ahead of the software consumer.
			 * Re-sample CDP only when we catch the known producer position.
			 */
			if (cdp_valid && !cdp_advanced) {
				rtl8197f_rtk_mw5_rx_update_hw_seq(priv, &hw_idx);
				cdp_advanced = priv->mw5_rx_seq_valid &&
					       priv->mw5_rx_hw_seq > priv->mw5_rx_sw_seq;
				if (!cdp_advanced) {
					priv->mw5_rx_seq_waits++;
					break;
				}
			}
		} else {
			cdp_valid = rtl8197f_rtk_rx_cdp_index(priv, &hw_idx);
			cdp_advanced = cdp_valid && idx != hw_idx;
			if (cdp_advanced)
				priv->rx_cdp_advanced++;

			if (cdp_valid) {
				if (idx == hw_idx) {
					if (opts1 & RTL_RTK_DESC_OWN)
						break;
				} else if (opts1 & RTL_RTK_DESC_OWN) {
					break;
				}
			} else if (opts1 & RTL_RTK_DESC_OWN) {
				break;
			}
		}

		dma_rmb();
		/* Re-read the entire completion, not OWN alone. If hardware is still
		 * publishing metadata, leave the descriptor untouched and retry on the
		 * next NAPI/watchdog pass. */
		if (!rtl8197f_rtk_mw5_rx_desc_stable(priv, desc, &opts1, &opts2)) {
			priv->mw5_rx_own_wait++;
			break;
		}

		/* v44.65.23: the monotonic producer sequence above is the MW5 publish
		 * fence. The current descriptor is consumed only when CPURPDCR0 has
		 * advanced in the same ring generation and OWN is clear. Pair the MMIO
		 * observation with payload reads.
		 */
		if (priv->is_mw5)
			dma_rmb();
		if (unlikely(le32_to_cpu(READ_ONCE(desc->addr)) !=
		    rtl8197f_rtk_hw_dma_addr(priv, priv->rx_dma[idx]))) {
			priv->mw5_rx_addr_mismatch++;
			priv->rx_bad_desc++;
			ndev->stats.rx_errors++;
			ndev->stats.rx_dropped++;
			rtl8197f_mw5_rx_bad(priv, idx, desc, "dma-address-mismatch");
			goto recycle;
		}
		old_skb = priv->rx_skb[idx];
		/* v44.62 page_pool deliberately keeps rx_skb[idx] == NULL and
		 * stores the active RX buffer in rx_page[idx].  The old common
		 * missing-skb check therefore classified every completed page_pool
		 * descriptor as empty and dropped it before the page_pool branch was
		 * reached.  This killed all master RX, which in turn killed LAN DHCP
		 * requests and WAN DHCP replies.  Validate the storage backend that is
		 * actually active instead.
		 */
		if ((priv->rx_coherent_enabled && !priv->rx_coherent_pool) ||
		    (!priv->rx_coherent_enabled && priv->rx_page_pool_enabled &&
		     priv->rx_page_pool && (!priv->rx_page || !priv->rx_page[idx])) ||
		    (!priv->rx_coherent_enabled &&
		     (!priv->rx_page_pool_enabled || !priv->rx_page_pool) && !old_skb)) {
			priv->rx_missing_skb++;
			ndev->stats.rx_dropped++;
			if (rtl8197f_rtk_alloc_rx_buf(priv, idx)) {
				priv->rx_alloc_fail++;
				break;
			}
			priv->rx_tail = rtl8197f_rtk_next(idx, priv->rx_ring_size);
			if (priv->is_mw5)
				priv->mw5_rx_sw_seq++;
			priv->rx_restart_pending = true;
			work_done++;
			continue;
		}

		len = opts2 & RTL_RTK_RX_LEN_MASK;
		rtl8197f_rtk_mw5_rxdesc_trace(priv, idx, desc, opts1, opts2,
					      cdp_valid, hw_idx);

		if (!len || len > priv->rx_buf_size ||
		    !(opts1 & RTL_RTK_DESC_FIRST) || !(opts1 & RTL_RTK_DESC_LAST)) {
			rtl8197f_mw5_rx_bad(priv, idx, desc,
					    "shape-or-length");
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

		/* A 2046-byte length repeatedly appeared only after a live ring pointer
		 * was reset by the old delayed reseed.  MW5 has MTU 1500 and no jumbo
		 * mode, so reject impossible frames before they can be interpreted as
		 * Ethernet data. Allow CPU tag, one VLAN header and a possible FCS.
		 */
		if (priv->is_mw5 &&
		    len > ndev->mtu + ETH_HLEN + VLAN_HLEN +
			  RTL_RTK_MW5_CPU_TAG8_LEN + ETH_FCS_LEN) {
			rtl8197f_mw5_rx_bad(priv, idx, desc,
					    "oversize");
			priv->mw5_rx_oversize_drop++;
			priv->rx_bad_desc++;
			ndev->stats.rx_errors++;
			ndev->stats.rx_dropped++;
			goto recycle;
		}

		/*
		 * RTL8197F is non-coherent MIPS in this tree.  Descriptor rings
		 * are coherent, but RX payload buffers are streamed with
		 * dma_map_single().  Hand completed buffers to the CPU before
		 * packet reads and hand recycled buffers back before OWN is set.
		 */
		if (!priv->rx_coherent_enabled) {
			if (priv->rx_page_pool_enabled && priv->rx_page_pool)
				dma_sync_single_range_for_cpu(priv->dev,
					page_pool_get_dma_addr(priv->rx_page[idx]),
					priv->rx_headroom, priv->rx_buf_size,
					DMA_FROM_DEVICE);
			else
				dma_sync_single_for_cpu(priv->dev, priv->rx_dma[idx],
						priv->rx_buf_size, DMA_FROM_DEVICE);
		}

		if (priv->rx_coherent_enabled) {
			void *src = rtl8197f_rtk_rx_data(priv, idx);

			new_skb = napi_alloc_skb(&priv->napi, len);
			if (!new_skb) {
				priv->rx_alloc_fail++;
				ndev->stats.rx_dropped++;
				goto recycle;
			}
			memcpy(skb_put(new_skb, len), src, len);
			old_skb = new_skb;
			priv->rx_coherent_copies++;
		} else if (priv->rx_page_pool_enabled && priv->rx_page_pool) {
			struct page *old_page = priv->rx_page[idx];
			struct page *new_page;

			new_page = page_pool_dev_alloc_pages(priv->rx_page_pool);
			if (!new_page) {
				priv->rx_alloc_fail++;
				ndev->stats.rx_dropped++;
				goto recycle;
			}
			new_skb = napi_build_skb(page_address(old_page), PAGE_SIZE);
			if (!new_skb) {
				page_pool_put_full_page(priv->rx_page_pool, new_page, true);
				priv->rx_alloc_fail++;
				ndev->stats.rx_dropped++;
				goto recycle;
			}
			skb_reserve(new_skb, priv->rx_headroom);
			skb_put(new_skb, len);
			skb_mark_for_recycle(new_skb);
			old_skb = new_skb;
			new_dma = page_pool_get_dma_addr(new_page) + priv->rx_headroom;
			priv->rx_page[idx] = new_page;
			priv->rx_dma[idx] = new_dma;
			priv->rx_page_pool_recycle++;
		} else {
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
			priv->rx_skb[idx] = new_skb;
			priv->rx_dma[idx] = new_dma;
		}
		/* napi_build_skb/page_pool does not guarantee skb->dev is populated
		 * before eth_type_trans(). Safe MW5-ACCEL-SW runs earlier by design, therefore
		 * establish the master ingress device explicitly for its guards. */
		old_skb->dev = ndev;
		rtl8197f_rtk_rx_checksum(priv, desc, old_skb);
		rtl8197f_rtk_rd05_rxtrace(priv, idx, desc, old_skb, len, opts1, opts2);
		if (rtl8197f_rtk_mw5_prepare_dsa_rx(priv, desc, old_skb)) {
			dev_kfree_skb_any(old_skb);
			old_skb = NULL;
			ndev->stats.rx_dropped++;
		} else if (priv->is_mw5 && priv->accel_sw &&
			   rtl8197f_mw5_accel_rx(old_skb)) {
			/* Consumed only after an exact nf_flow_table-authorized MW5-ACCEL-SW hit. */
			old_skb = NULL;
		} else {
			old_skb->protocol = eth_type_trans(old_skb, ndev);
			rtl8197f_rtk_iball_rx_vlan(priv, desc, old_skb);
			napi_gro_receive(&priv->napi, old_skb);
		}

		if (old_skb) {
			ndev->stats.rx_packets++;
			ndev->stats.rx_bytes += len;
		}

		desc->addr = cpu_to_le32(rtl8197f_rtk_hw_dma_addr(priv, priv->rx_dma[idx]));

recycle:
		work_done++;
		if (priv->rx_coherent_enabled && priv->rx_coherent_pool) {
			/* Poison only the next Ethernet header before ownership returns to
			 * hardware. If a broken/late completion ever advances without a
			 * payload DMA, it becomes an all-zero frame that the DSA guard drops
			 * instead of replaying stale packet bytes into the stack. */
			memset(rtl8197f_rtk_rx_data(priv, idx), 0,
			       min_t(u32, priv->rx_buf_size, 64));
		}
		if (!priv->rx_coherent_enabled) {
			if (priv->rx_page_pool_enabled && priv->rx_page_pool)
				dma_sync_single_range_for_device(priv->dev,
					page_pool_get_dma_addr(priv->rx_page[idx]),
					priv->rx_headroom, priv->rx_buf_size,
					DMA_FROM_DEVICE);
			else
				dma_sync_single_for_device(priv->dev, priv->rx_dma[idx],
						   priv->rx_buf_size, DMA_FROM_DEVICE);
		}

		/* Re-arm every slot with the software-owned RX buffer address.  This
		 * also repairs an address field if a malformed completion corrupted it.
		 */
		desc->addr = cpu_to_le32(rtl8197f_rtk_hw_dma_addr(priv,
								  priv->rx_dma[idx]));
		desc->opts2 = 0;
		desc->opts3 = 0;
		desc->opts4 = 0;
		desc->opts5 = 0;
		rtl8197f_rtk_desc_clear_padding(priv, desc);
		dma_wmb();
		desc->opts1 = cpu_to_le32(RTL_RTK_DESC_OWN |
						(priv->rx_buf_size << RTL_RTK_RX_EXTSIZE_SHIFT) |
						(idx == priv->rx_ring_size - 1 ?
						 RTL_RTK_DESC_WRAP : 0));
		priv->rx_tail = rtl8197f_rtk_next(idx, priv->rx_ring_size);
		if (priv->is_mw5) {
			priv->mw5_rx_sw_seq++;
			if (READ_ONCE(priv->mw5_rx_catchup_pending)) {
				/* The threshold frame has just been safely recycled. Try the
				 * RX-only catch-up immediately so recovery does not depend on
				 * another WAN interrupt arriving after the corruption burst.
				 */
				rtl8197f_rtk_mw5_rx_catchup(priv);
				break;
			}
		}
	}

	return work_done;
}

static int rtl8197f_rtk_poll(struct napi_struct *napi, int budget)
{
	struct rtl8197f_rtknet_priv *priv =
		container_of(napi, struct rtl8197f_rtknet_priv, napi);
	unsigned int tx_cleaned;
	int work_done;

	priv->napi_polls++;
	tx_cleaned = rtl8197f_rtk_tx_clean(priv);
	work_done = rtl8197f_rtk_rx_poll(priv, budget);
	priv->napi_rx_work += work_done;
	priv->napi_tx_clean += tx_cleaned;
	if (!work_done)
		priv->napi_zero_rx_polls++;
	if (work_done >= budget)
		priv->napi_budget_polls++;

	if (priv->rx_restart_pending) {
		bool runout_recovery = priv->is_mw5 && priv->rx_runout_pending;

		priv->rx_restart_pending = false;
		priv->rx_runout_pending = false;
		/* v44.65.19: once descriptors have been recycled, explicitly
		 * re-assert both switch-core TRXRDY and CPU RXCMD after an MW5
		 * runout.  This is deliberately RX-only: no ring pointer, DSA,
		 * L2/ACL or TX state is rewound.
		 */
		if (runout_recovery) {
			dma_wmb();
			rtl8197f_rtk_assert_trxrdy(priv);
			priv->rx_runout_trxrdy_writes++;
			priv->rx_runout_recoveries++;
		}
		rtl8197f_rtk_kick_rx(priv);
	}

	/* v44.62.10: never fake RX work for a TX-only completion.  With
	 * gro_flush_timeout/napi_defer_hard_irqs enabled that kept the NAPI timer
	 * armed after almost every packet and produced tens of thousands of empty
	 * polls on the single-core RTL8197F.  Complete with the real RX work count
	 * so TX-only interrupts cannot create a self-sustaining softirq loop.
	 */
	if (work_done < budget && napi_complete_done(napi, work_done)) {
		rtl8197f_rtk_enable_irq(priv);

		/* Close the classic NAPI completion race: a frame may arrive after
		 * the final descriptor test but before CPUIIMR is re-enabled.
		 */
		if (unlikely(priv->rx_restart_pending ||
		    rtl8197f_rtk_rx_napi_ready(priv)) &&
		    napi_schedule_prep(&priv->napi)) {
			rtl8197f_rtk_disable_irq(priv);
			__napi_schedule(&priv->napi);
			priv->napi_rearm_race++;
		}
	}

	return work_done;
}


static bool rtl8197f_rtk_mw5_runout_guard_active(struct rtl8197f_rtknet_priv *priv)
{
	unsigned long until;

	if (!priv || !priv->is_mw5)
		return false;
	until = READ_ONCE(priv->rx_runout_guard_until);
	if (!until)
		return false;
	if (time_before(jiffies, until))
		return true;

	/* jiffies starts close to wrap on Linux. Keeping an expired zero/stale
	 * deadline would otherwise make the 10-ms guard appear active again.
	 */
	WRITE_ONCE(priv->rx_runout_guard_until, 0);
	return false;
}

static void rtl8197f_rtk_rx_poll_timer(struct timer_list *t)
{
	struct rtl8197f_rtknet_priv *priv =
		from_timer(priv, t, rx_poll_timer);
	u32 delay_ms = priv->rx_poll_ms;
	u64 irq_rx_done;
	bool pending, publish_wait;

	if (!priv->rx_poll_fallback || !netif_running(priv->ndev))
		return;

	priv->rx_poll_timer_runs++;
	irq_rx_done = READ_ONCE(priv->irq_rx_done);
	publish_wait = READ_ONCE(priv->rx_publish_wait_pending);

	/* v44.65.22: a descriptor whose OWN cleared before CPURPDCR0 advanced is
	 * retried quickly without spinning NAPI. The timer remains one-shot-fast
	 * only while that publication fence is pending.
	 */
	if (publish_wait) {
		delay_ms = RTL_RTK_MW5_RX_PUBLISH_RECHECK_MS;
		priv->rx_publish_timer_runs++;
	}

	/* Once real RX interrupts have been proved, a 10-ms timer is pure overhead
	 * on this one-core SoC.  Keep the normal path as a one-second lost-IRQ
	 * watchdog.  v44.65.19 temporarily switches back to a 10-ms guard after
	 * a real RX/mbuf runout so a lost restart interrupt cannot turn a short
	 * burst into a visible one-second WAN outage.
	 */
	if (rtl8197f_rtk_mw5_runout_guard_active(priv)) {
		delay_ms = RTL_RTK_MW5_RX_RUNOUT_RECHECK_MS;
		priv->rx_runout_fast_timer_runs++;
	} else if (publish_wait) {
		delay_ms = RTL_RTK_MW5_RX_PUBLISH_RECHECK_MS;
	} else if (irq_rx_done) {
		delay_ms = max_t(u32, priv->rx_poll_ms, RTL_RTK_MW5_RX_WATCHDOG_MS);
	}

	if (irq_rx_done != priv->rx_poll_last_irq_rx_done) {
		priv->rx_poll_last_irq_rx_done = irq_rx_done;
		priv->rx_poll_irq_progress_skips++;
		if (!publish_wait &&
		    !(rtl8197f_rtk_mw5_runout_guard_active(priv)))
			goto rearm;
	}

	pending = priv->rx_restart_pending || rtl8197f_rtk_rx_napi_ready(priv);
	if (pending && napi_schedule_prep(&priv->napi)) {
		priv->rx_poll_fallback_schedules++;
		if (rtl8197f_rtk_mw5_runout_guard_active(priv))
			priv->rx_runout_fast_schedules++;
		rtl8197f_rtk_disable_irq(priv);
		__napi_schedule(&priv->napi);
	}

rearm:
	mod_timer(&priv->rx_poll_timer,
		  jiffies + msecs_to_jiffies(delay_ms));
}

static irqreturn_t rtl8197f_rtk_irq(int irq, void *dev_id)
{
	struct net_device *ndev = dev_id;
	struct rtl8197f_rtknet_priv *priv = netdev_priv(ndev);
	u32 status;

	/* CPUIISR is W1C.  Acknowledge only causes owned by this driver instead of
	 * clearing unrelated/reserved switch-core status observed in the register.
	 */
	status = rtl8197f_rtk_read(priv, RTL_RTK_CPUIISR) & RTL_RTK_INT_STATUS_MASK;
	if (!status)
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
		priv->rx_runout_pending = true;
	}
	if (status & RTL_RTK_INT_MBUF_RUNOUT0) {
		priv->irq_mbuf_runout++;
		priv->rx_restart_pending = true;
		priv->rx_runout_pending = true;
	}
	if (priv->is_mw5 &&
	    (status & (RTL_RTK_INT_RX_RUNOUT0 | RTL_RTK_INT_MBUF_RUNOUT0))) {
		WRITE_ONCE(priv->rx_runout_guard_until,
			jiffies + msecs_to_jiffies(RTL_RTK_MW5_RX_RUNOUT_GUARD_MS));
		if (priv->rx_poll_fallback)
			mod_timer(&priv->rx_poll_timer,
				jiffies + msecs_to_jiffies(RTL_RTK_MW5_RX_RUNOUT_RECHECK_MS));
		netdev_warn(ndev,
			"MW5 RX runout: status=0x%08x tail=%u ring=%u cdp=0x%08x; fast RX-only recovery armed\n",
			status, priv->rx_tail, priv->rx_ring_size,
			rtl8197f_rtk_read(priv, RTL_RTK_CPURPDCR0));
	}
	/* v44.65.14: CPUIISR was already W1C-acknowledged above. Count an RX
	 * runout acknowledgement only when hardware actually reported one. Do
	 * not clear runout bits once per recycled packet as v44.65.13 did. */
	if (status & (RTL_RTK_INT_RX_RUNOUT0 | RTL_RTK_INT_MBUF_RUNOUT0))
		priv->rx_runout_acks++;

	if (status & RTL_RTK_INT_LINK_CHANGE)
		priv->irq_link_change++;

	if (napi_schedule_prep(&priv->napi)) {
		rtl8197f_rtk_disable_irq(priv);
		__napi_schedule_irqoff(&priv->napi);
	}

	return IRQ_HANDLED;
}


struct rtl8197f_rtk_txmeta {
	u32 port_mask;
	u32 dp_ext;
	u32 extspa;
	u16 vid;
	bool vid_valid;
	bool hwlookup;
	bool bridge;
};

static void rtl8197f_rtk_rd05_txdesc_meta(struct rtl8197f_rtknet_priv *priv,
						  struct rtl8197f_rtk_txmeta *m)
{
	int mode = READ_ONCE(rd05_txdesc_mode);

	m->port_mask = priv->tx_port_mask;
	m->dp_ext = priv->tx_dp_ext;
	m->extspa = priv->tx_extspa;
	m->vid = 0;
	m->vid_valid = false;
	m->hwlookup = priv->tx_hwlookup;
	m->bridge = priv->tx_bridge;

	if (priv->is_mw5 || !of_machine_is_compatible("xiaomi,r4-rd05"))
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

static void rtl8197f_rtk_mw5_tx_wire_trace(struct rtl8197f_rtknet_priv *priv,
					      struct sk_buff *skb)
{
	static atomic_t trace_count = ATOMIC_INIT(0);
	const u8 *d = skb->data;
	u16 outer = 0, tag = 0, encap = 0;
	int n;

	if (!priv->is_mw5 || skb->len < ETH_HLEN)
		return;
	/* v44.57: once the bounded startup trace is complete, leave the TX
	 * hotpath before parsing headers or touching an atomic counter. */
	if (likely(atomic_read(&trace_count) >= 8))
		return;
	n = atomic_inc_return(&trace_count);
	if (n > 8)
		return;

	outer = get_unaligned_be16(d + 2 * ETH_ALEN);
	if (outer == ETH_P_REALTEK && skb->len >= ETH_HLEN + RTL_RTK_MW5_CPU_TAG4_LEN) {
		tag = get_unaligned_be16(d + 2 * ETH_ALEN + sizeof(__be16));
		if (tag == RTL_RTK_MW5_RTL8_PROTO_WORD &&
		    skb->len >= ETH_HLEN + RTL_RTK_MW5_CPU_TAG8_LEN)
			encap = get_unaligned_be16(d + 2 * ETH_ALEN + RTL_RTK_MW5_CPU_TAG8_LEN);
		else
			encap = get_unaligned_be16(d + 2 * ETH_ALEN + RTL_RTK_MW5_CPU_TAG4_LEN);
	}

	dev_info(priv->dev,
		 "mw5 tx-wire v44.66.9: pre-dma len=%u outer=0x%04x tag=0x%04x encap=0x%04x da=%pM sa=%pM\n",
		 skb->len, outer, tag, encap, d, d + ETH_ALEN);
	print_hex_dump(KERN_INFO, "mw5 tx-wire v44.66.9 raw: ", DUMP_PREFIX_OFFSET,
		       16, 1, d, min_t(u32, skb->len, 48), false);
}

static void rtl8197f_rtk_mw5_tx_submit_trace(struct rtl8197f_rtknet_priv *priv,
					     unsigned int idx, dma_addr_t dma,
						u32 opts1, u32 opts2, u32 opts3,
						u32 opts4, u32 opts5)
{
	static atomic_t trace_count = ATOMIC_INIT(0);
	dma_addr_t desc_dma;
	u32 cdp_after, own_after;
	int n;

	if (!priv->is_mw5)
		return;

	priv->mw5_tx_submit++;
	/* v44.62.6: CPUTPDCR0 is MMIO. Keep the read entirely inside this bounded
	 * startup trace. v44.62 accidentally reintroduced one MMIO read per packet
	 * in start_xmit(), which directly penalised routed WLAN/WAN throughput.
	 */
	if (likely(atomic_read(&trace_count) >= 8))
		return;
	n = atomic_inc_return(&trace_count);
	if (n > 8)
		return;
	cdp_after = rtl8197f_rtk_read(priv, RTL_RTK_CPUTPDCR0);
	own_after = le32_to_cpu(READ_ONCE(rtl8197f_rtk_tx_desc(priv, idx)->opts1));
	if (own_after & RTL_RTK_DESC_OWN)
		priv->mw5_tx_own_after_kick++;

	desc_dma = priv->tx_desc_dma + idx * priv->desc_stride;
	dev_info(priv->dev,
		 "mw5 tx-submit v44.62.6: idx=%u desc=%pad buf=%pad opts=%08x/%08x/%08x/%08x/%08x cdp=0x%08x own_after=%u head=%u tail=%u\n",
		 idx, &desc_dma, &dma, opts1, opts2, opts3, opts4, opts5,
		 cdp_after, !!(own_after & RTL_RTK_DESC_OWN),
		 priv->tx_head, priv->tx_tail);
}

static bool rtl8197f_rtk_tx_hw_csum_supported(struct sk_buff *skb)
{
	/* The MW5 DSA tag is already in the Ethernet byte stream at the master
	 * netdev. The old rtl865x parser does not understand EtherType 0x8899,
	 * so do not ask SwitchCore to checksum such frames until a tag-aware
	 * descriptor offset is proven on hardware. */
	if (rtl8197f_rtk_has_mw5_dsa_tag(skb))
		return false;
	if (skb->protocol == htons(ETH_P_IP)) {
		if (!pskb_network_may_pull(skb, sizeof(struct iphdr)))
			return false;
		return ip_hdr(skb)->protocol == IPPROTO_TCP ||
		       ip_hdr(skb)->protocol == IPPROTO_UDP;
	}
	if (skb->protocol == htons(ETH_P_IPV6)) {
		if (!pskb_network_may_pull(skb, sizeof(struct ipv6hdr)))
			return false;
		/* Keep extension headers on the software path until the precise
		 * RTL8197F IPv6 header-length semantics are hardware-verified. */
		return ipv6_hdr(skb)->nexthdr == IPPROTO_TCP ||
		       ipv6_hdr(skb)->nexthdr == IPPROTO_UDP;
	}
	return false;
}

static int rtl8197f_rtk_tx_csum_bits(struct rtl8197f_rtknet_priv *priv,
				    struct sk_buff *skb, u32 *opts1,
				    u32 *opts3, u32 *opts4, u32 *opts5)
{
	if (skb->ip_summed != CHECKSUM_PARTIAL)
		return 0;
	if (!(priv->ndev->features & (NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM)) ||
	    !rtl8197f_rtk_tx_hw_csum_supported(skb))
		return -EOPNOTSUPP;

	if (skb->protocol == htons(ETH_P_IP)) {
		struct iphdr *iph;
		if (!pskb_network_may_pull(skb, sizeof(*iph)))
			return -EOPNOTSUPP;
		iph = ip_hdr(skb);
		if (iph->protocol != IPPROTO_TCP && iph->protocol != IPPROTO_UDP)
			return -EOPNOTSUPP;
		*opts3 |= RTL_RTK_TX_L3CS | RTL_RTK_TX_L4CS |
			  RTL_RTK_TX_IPV4 | RTL_RTK_TX_IPV4_1ST;
		priv->tx_csum_hw++;
		return 0;
	}
	if (skb->protocol == htons(ETH_P_IPV6)) {
		struct ipv6hdr *ip6h;
		if (!pskb_network_may_pull(skb, sizeof(*ip6h)))
			return -EOPNOTSUPP;
		ip6h = ipv6_hdr(skb);
		/* Direct TCP/UDP only. Extension-header parsing falls back to software. */
		if (ip6h->nexthdr != IPPROTO_TCP && ip6h->nexthdr != IPPROTO_UDP)
			return -EOPNOTSUPP;
		*opts3 |= RTL_RTK_TX_L4CS | RTL_RTK_TX_IPV6;
		*opts4 |= sizeof(*ip6h);
		priv->tx_csum_hw++;
		return 0;
	}
	return -EOPNOTSUPP;
}

static void rtl8197f_rtk_tx_tso_bits(struct rtl8197f_rtknet_priv *priv,
				     struct sk_buff *skb, u32 *opts1,
				     u32 *opts3, u32 *opts4, u32 *opts5)
{
	unsigned int thlen;

	if (!(priv->ndev->features & (NETIF_F_TSO | NETIF_F_TSO6)) ||
	    !skb_is_gso(skb) || rtl8197f_rtk_has_mw5_dsa_tag(skb))
		return;
	thlen = tcp_hdrlen(skb) / 4;
	*opts1 &= ~RTL_RTK_TX_TYPE_MASK;
	*opts1 |= RTL_RTK_TX_TYPE_TCP;
	*opts4 |= RTL_RTK_TX_LSO;
	*opts5 |= (skb_shinfo(skb)->gso_size & 0x3fff) << RTL_RTK_TX_MSS_SHIFT;
	*opts5 |= thlen & 0xf;
	if (skb->protocol == htons(ETH_P_IP)) {
		*opts3 |= RTL_RTK_TX_L3CS | RTL_RTK_TX_L4CS |
			  RTL_RTK_TX_IPV4 | RTL_RTK_TX_IPV4_1ST;
		*opts5 |= (ip_hdrlen(skb) / 4) << RTL_RTK_TX_IPV4_HLEN_SHIFT;
		priv->tx_tso4++;
	} else if (skb->protocol == htons(ETH_P_IPV6)) {
		*opts3 |= RTL_RTK_TX_L4CS | RTL_RTK_TX_IPV6;
		*opts4 |= sizeof(struct ipv6hdr);
		priv->tx_tso6++;
	}
}

static void rtl8197f_rtk_iball_tx_vlan(struct rtl8197f_rtknet_priv *priv,
					struct sk_buff *skb,
					struct rtl8197f_rtk_txmeta *m)
{
	u16 vid;

	if (!priv->is_iball || !priv->internal_vlan_split)
		return;

	/* With NETIF_F_HW_VLAN_CTAG_TX, eth0.8/eth0.9 leave the VLAN in skb
	 * metadata. The internal switch performs PVID/untagging; no 802.1Q header is
	 * emitted on the cable.
	 */
	if (!skb_vlan_tag_present(skb))
		return;
	vid = skb_vlan_tag_get_id(skb);
	if (vid == priv->wan_vid) {
		m->port_mask = priv->wan_port_mask;
		m->dp_ext = 0;
		m->extspa = 0;
		m->hwlookup = false;
		m->bridge = false;
		priv->iball_tx_wan++;
	} else if (vid == priv->lan_vid) {
		m->port_mask = priv->lan_port_mask;
		m->dp_ext = 0;
		m->extspa = 0;
		m->hwlookup = false;
		m->bridge = false;
		priv->iball_tx_lan++;
	}
}

static bool rtl8197f_rtk_mw5_wire_cputag(struct rtl8197f_rtknet_priv *priv,
					      struct sk_buff *skb, u16 *port_mask_out)
{
	u16 proto, proto_reason, port_mask;

	if (!priv->is_mw5)
		return false;
	if (!pskb_may_pull(skb, ETH_HLEN + RTL_RTK_MW5_CPU_TAG8_LEN))
		return false;

	proto = get_unaligned_be16(skb->data + 2 * ETH_ALEN);
	if (proto != ETH_P_REALTEK)
		return false;

	/* Upstream tag_rtl8_4 emits:
	 *   8899 | 0400 | 0020 | <11-bit destination mask> | EtherType
	 * for CPU->switch traffic.  Require the protocol/reason word used by the
	 * Linux tagger so legacy four-byte 0x040x/protocol-9 diagnostics cannot be
	 * mistaken for the normal production path. */
	proto_reason = get_unaligned_be16(skb->data + 2 * ETH_ALEN + 2);
	if (proto_reason != RTL_RTK_MW5_RTL8_PROTO_WORD)
		return false;

	port_mask = get_unaligned_be16(skb->data + 2 * ETH_ALEN + 6) &
		    RTL_RTK_MW5_RTL8_PORT_MASK;
	if (!port_mask)
		return false;

	if (port_mask_out)
		*port_mask_out = port_mask;
	return true;
}

static int rtl8197f_rtk_mw5_extract_dsa_txmeta(struct rtl8197f_rtknet_priv *priv,
					       struct sk_buff *skb, u8 *port)
{
	u16 proto, tag, mask;

	if (!priv->is_mw5)
		return 0;
	if (!pskb_may_pull(skb, ETH_HLEN + RTL_RTK_MW5_CPU_TAG_LEN))
		return 0;

	proto = get_unaligned_be16(skb->data + 2 * ETH_ALEN);
	if (proto != ETH_P_REALTEK)
		return 0;

	tag = get_unaligned_be16(skb->data + 2 * ETH_ALEN + 2);
	if ((tag & RTL_RTK_MW5_TX_META_MAGIC_MASK) != RTL_RTK_MW5_TX_META_MAGIC)
		return 0;

	mask = tag & RTL_RTK_MW5_TX_META_PORT_MASK;
	if (mask != BIT(RTL_RTK_MW5_LAN_PORT) &&
	    mask != BIT(RTL_RTK_MW5_WAN_PORT)) {
		priv->mw5_tx_meta_invalid++;
		return -EINVAL;
	}

	*port = __ffs(mask);
	/* Reverse dsa_alloc_etype_header(): move DA/SA over the software-only
	 * marker, then pull it. The marker is never visible to RTL8367 or wire. */
	memmove(skb->data + RTL_RTK_MW5_CPU_TAG_LEN, skb->data, 2 * ETH_ALEN);
	skb_pull(skb, RTL_RTK_MW5_CPU_TAG_LEN);
	skb_reset_mac_header(skb);
	return 1;
}

static int rtl8197f_rtk_mw5_apply_dsa_txmeta(struct rtl8197f_rtknet_priv *priv,
					     u8 port,
					     struct rtl8197f_rtk_txmeta *m)
{
	if (port == RTL_RTK_MW5_LAN_PORT) {
		m->vid = RTL_RTK_MW5_LAN_VID;
		priv->mw5_tx_meta_lan++;
	} else if (port == RTL_RTK_MW5_WAN_PORT) {
		m->vid = RTL_RTK_MW5_WAN_VID;
		priv->mw5_tx_meta_wan++;
	} else {
		priv->mw5_tx_meta_invalid++;
		return -EINVAL;
	}

	m->vid_valid = true;
	m->port_mask = BIT(port);
	m->dp_ext = 0;
	m->extspa = 0;
	m->hwlookup = false;
	m->bridge = false;
	return 0;
}

static netdev_tx_t rtl8197f_rtk_start_xmit(struct sk_buff *skb,
					   struct net_device *ndev)
{
	struct rtl8197f_rtknet_priv *priv = netdev_priv(ndev);
	struct rtl8197f_rtk_txmeta txm;
	unsigned long flags;
	unsigned int nr_frags, needed, seg, idx, last_idx = 0;
	u32 pkt_len, desc_len, base_opts1, base_opts3, base_opts4, base_opts5;
	u32 prio, qid;
	dma_addr_t mapped[MAX_SKB_FRAGS + 1] = { 0 };
	u32 mapped_len[MAX_SKB_FRAGS + 1] = { 0 };
	bool mapped_page[MAX_SKB_FRAGS + 1] = { 0 };
	bool hw_sg, hw_qos, xmit_more, ring_low, doorbell, coherent_tx;
	bool mw5_wire_cputag;
	u16 mw5_wire_mask = 0;
	u8 dsa_port = 0xff;
	int dsa_meta, ret = 0;

	if (!skb_is_gso(skb) &&
	    skb->len > ndev->mtu + ETH_HLEN + VLAN_HLEN +
		       (priv->is_mw5 ? RTL_RTK_MW5_CPU_TAG8_LEN : 0)) {
		/* DSA inserts the MW5 rtl8_4 header before the master ndo_start_xmit.
		 * Account for those eight bytes in the master-side sanity limit; the
		 * user-port MTU remains 1500 and the external switch consumes the tag. */
		priv->tx_mtu_drops++;
		ndev->stats.tx_dropped++;
		dev_kfree_skb_any(skb);
		return NETDEV_TX_OK;
	}
	mw5_wire_cputag = rtl8197f_rtk_mw5_wire_cputag(priv, skb, &mw5_wire_mask);
	dsa_meta = rtl8197f_rtk_mw5_extract_dsa_txmeta(priv, skb, &dsa_port);
	if (unlikely(dsa_meta < 0)) {
		ndev->stats.tx_dropped++;
		dev_kfree_skb_any(skb);
		return NETDEV_TX_OK;
	}

	if (skb_put_padto(skb, ETH_ZLEN)) {
		ndev->stats.tx_dropped++;
		return NETDEV_TX_OK;
	}

	if (skb->ip_summed == CHECKSUM_PARTIAL && !skb_is_gso(skb) &&
	    (!(ndev->features & (NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM)) ||
	     !rtl8197f_rtk_tx_hw_csum_supported(skb))) {
		if (skb_checksum_help(skb)) {
			ndev->stats.tx_dropped++;
			dev_kfree_skb_any(skb);
			return NETDEV_TX_OK;
		}
		priv->tx_csum_sw_fallback++;
	}

	coherent_tx = priv->is_mw5 && priv->tx_coherent_enabled;
	hw_sg = !coherent_tx && !!(ndev->features & NETIF_F_SG);
	/* v44.66.9 normal MW5 TX keeps the upstream 0x8899/protocol-4 eight-byte
	 * CPU tag in the skb and carries it over the physical RTL8197F P0/RGMII cascade.
	 * Layout3 remains a diagnostic software-only 0xd0xx shim and is consumed
	 * above. SG itself does not require the RTL865x L3 parser, but
	 * keeping the complete L2/L3 header in the linear head and limiting the
	 * descriptor fan-out avoids stressing the non-coherent single-core DMA
	 * engine with pathological skb layouts.  Oversized layouts fall back to a
	 * linear skb rather than being dropped or handed to an unproven fast path.
	 */
	if (priv->is_mw5 && hw_sg && skb_is_nonlinear(skb) &&
	    (skb_headlen(skb) < 64 ||
	     skb_shinfo(skb)->nr_frags > max_t(unsigned int, 1, mw5_sg_max_frags))) {
		if (skb_linearize(skb)) {
			ndev->stats.tx_dropped++;
			return NETDEV_TX_OK;
		}
		priv->tx_sg_guard_linearize++;
	}
	nr_frags = hw_sg ? skb_shinfo(skb)->nr_frags : 0;
	if (!hw_sg && skb_is_nonlinear(skb) && skb_linearize(skb)) {
		ndev->stats.tx_dropped++;
		return NETDEV_TX_OK;
	}
	needed = nr_frags + 1;
	if (needed > MAX_SKB_FRAGS + 1)
		return NETDEV_TX_BUSY;

	/* Non-MW5 boards keep the streaming DMA path. MW5 copies into a fixed
	 * coherent slot only after it owns a ring index, so hardware never reads
	 * from reusable skb storage. */
	if (!coherent_tx) {
		mapped_len[0] = skb_headlen(skb);
		mapped[0] = dma_map_single(priv->dev, skb->data, mapped_len[0], DMA_TO_DEVICE);
		if (dma_mapping_error(priv->dev, mapped[0]))
			goto map_error;
		for (seg = 0; seg < nr_frags; seg++) {
			const skb_frag_t *frag = &skb_shinfo(skb)->frags[seg];
			mapped_len[seg + 1] = skb_frag_size(frag);
			mapped[seg + 1] = skb_frag_dma_map(priv->dev, frag, 0,
							  mapped_len[seg + 1], DMA_TO_DEVICE);
			mapped_page[seg + 1] = true;
			if (dma_mapping_error(priv->dev, mapped[seg + 1]))
				goto map_error;
		}
	}

	spin_lock_irqsave(&priv->tx_lock, flags);
	if (rtl8197f_rtk_tx_avail(priv) < needed) {
		priv->tx_ring_full++;
		netif_stop_queue(ndev);
		spin_unlock_irqrestore(&priv->tx_lock, flags);
		ret = -EBUSY;
		goto unmap_all;
	}
	if (!priv->is_mw5) {
		unsigned int check = priv->tx_head;
		for (seg = 0; seg < needed; seg++) {
			if (le32_to_cpu(rtl8197f_rtk_tx_desc(priv, check)->opts1) & RTL_RTK_DESC_OWN) {
				priv->tx_busy_desc++;
				netif_stop_queue(ndev);
				spin_unlock_irqrestore(&priv->tx_lock, flags);
				ret = -EBUSY;
				goto unmap_all;
			}
			check = rtl8197f_rtk_next(check, priv->tx_ring_size);
		}
	}

	if (coherent_tx) {
		struct rtl8197f_rtk_desc *reuse_desc;
		u32 reuse_opts1;

		idx = priv->tx_head;
		if (unlikely(!priv->tx_coherent_pool || !priv->tx_coherent_cpu ||
			     !priv->tx_coherent_dma_slot || !priv->tx_coherent_cpu[idx] ||
			     skb->len > priv->tx_coherent_stride)) {
			priv->tx_coherent_oversize++;
			ndev->stats.tx_dropped++;
			spin_unlock_irqrestore(&priv->tx_lock, flags);
			ret = -EMSGSIZE;
			goto unmap_all;
		}

		/* V212/OEM completion rule, enforced again at the exact reuse point:
		 * tx_tail/CDP accounting may make the ring look available, but Linux
		 * must never overwrite this fixed coherent slot while hardware still
		 * owns the descriptor or while the software completion state still has
		 * an skb attached.  This is an ownership fence, not a reclaim delay. */
		reuse_desc = rtl8197f_rtk_tx_desc(priv, idx);
		reuse_opts1 = le32_to_cpu(READ_ONCE(reuse_desc->opts1));
		if (unlikely((reuse_opts1 & RTL_RTK_DESC_OWN) ||
			     READ_ONCE(priv->tx_skb[idx]))) {
			if (reuse_opts1 & RTL_RTK_DESC_OWN)
				priv->mw5_tx_reuse_guard_own++;
			if (READ_ONCE(priv->tx_skb[idx]))
				priv->mw5_tx_reuse_guard_swbusy++;
			priv->tx_busy_desc++;
			netif_stop_queue(ndev);
			spin_unlock_irqrestore(&priv->tx_lock, flags);
			ret = -EBUSY;
			goto unmap_all;
		}

		memcpy(priv->tx_coherent_cpu[idx], skb->data, skb->len);
		mapped[0] = priv->tx_coherent_dma_slot[idx];
		mapped_len[0] = skb->len;
		mapped_page[0] = false;
		priv->tx_coherent_copies++;
		priv->tx_coherent_bytes += skb->len;
	}

	pkt_len = skb->len;
	desc_len = priv->sdk_crc_lengths ? max_t(u32, pkt_len + ETH_FCS_LEN, 64) : pkt_len;
	if (priv->sdk_crc_lengths)
		priv->tx_crc_len_adjusts++;
	rtl8197f_rtk_rd05_txdesc_meta(priv, &txm);
	rtl8197f_rtk_iball_tx_vlan(priv, skb, &txm);
	if (mw5_wire_cputag) {
		u16 wire_mask = mw5_wire_mask;

		/* v44.66.10 physical-topology invariant:
		 *
		 *   RTL8197F SwitchCore P0/RGMII -> RTL8367 CPU port 6
		 *
		 * The final word of the rtl8_4 CPU tag selects the *external* RTL8367
		 * user port mask. It is not an RTL8197F descriptor DP field. Keep the
		 * wire tag intact and always send tagged DSA traffic through P0.
		 *
		 * Hardware PCAPs proved that with PORT0_ROUTER_MODE active, DVID=0
		 * becomes VLAN0 on wire (v44.66.8) and DVID=9/8 becomes VLAN9/8 on wire
		 * (v44.66.9), always before 0x8899. In transparent mode the descriptor
		 * therefore carries no DVID at all. The old DVID9/8 router-mode path is
		 * kept only for a controlled live A/B diagnostic. */
		txm.port_mask = BIT(0);
		txm.dp_ext = 0;
		txm.extspa = 0;
		if (READ_ONCE(priv->mw5_p0_transparent)) {
			txm.vid = 0;
			txm.vid_valid = false;
		} else if (wire_mask == BIT(RTL_RTK_MW5_LAN_PORT)) {
			txm.vid = RTL_RTK_MW5_LAN_VID;
			txm.vid_valid = true;
		} else if (wire_mask == BIT(RTL_RTK_MW5_WAN_PORT)) {
			txm.vid = RTL_RTK_MW5_WAN_VID;
			txm.vid_valid = true;
		} else {
			txm.vid = 0;
			txm.vid_valid = false;
		}
		if (wire_mask == BIT(RTL_RTK_MW5_LAN_PORT))
			priv->mw5_tx_wiretag_lan++;
		else if (wire_mask == BIT(RTL_RTK_MW5_WAN_PORT))
			priv->mw5_tx_wiretag_wan++;
		else
			priv->mw5_tx_wiretag_other++;
		txm.hwlookup = false;
		txm.bridge = false;
	} else if (dsa_meta > 0 &&
		   rtl8197f_rtk_mw5_apply_dsa_txmeta(priv, dsa_port, &txm)) {
		spin_unlock_irqrestore(&priv->tx_lock, flags);
		ndev->stats.tx_dropped++;
		ret = -EINVAL;
		goto unmap_all;
	}
	/* v44.66.10: normal MW5 DSA egress is the upstream protocol-4 eight-byte
	 * RTL8367/RTL8365MB CPU tag transported unchanged through SoC P0 normal
	 * mode. Legacy RTL865x HWLOOKUP/bridge lookup must not reinterpret the
	 * master frame, and the descriptor carries neither internal LAN/WAN DP nor
	 * DVID metadata in transparent mode. Layout3/router-mode metadata remains
	 * diagnostic-only.
	 *
	 * Keep the HWLOOKUP capability/gate available for the explicit
	 * nf_flow_table/MW5-ACCEL-SW backend, but never apply legacy descriptor
	 * HWLOOKUP to the DSA conduit. This is fail-closed.
	 */
	if (priv->is_mw5 && txm.hwlookup) {
		txm.hwlookup = false;
		txm.bridge = false;
		priv->tx_hwlookup_dsa_bypass++;
	}
	prio = rtl8197f_rtk_tx_priority(skb);
	qid = rtl8197f_rtk_qid_from_priority(prio);
	hw_qos = priv->hw_qos;
	if (hw_qos)
		priv->tx_qos_packets++;
	base_opts3 = txm.vid_valid ? (txm.vid & RTL_RTK_TX_DVID_MASK) : 0;
	base_opts3 |= (txm.dp_ext & RTL_RTK_TX_DP_EXT_MASK) << RTL_RTK_TX_DP_EXT_SHIFT;
	if (hw_qos)
		base_opts3 |= (prio & 7) << RTL_RTK_TX_DPRI_SHIFT;
	base_opts4 = (txm.port_mask & RTL_RTK_TX_DP_MASK) << RTL_RTK_TX_DP_SHIFT;
	base_opts5 = (txm.extspa & RTL_RTK_TX_EXTSPA_MASK) << RTL_RTK_TX_EXTSPA_SHIFT;
	base_opts1 = desc_len << RTL_RTK_TX_PH_LEN_SHIFT;
	if (txm.hwlookup)
		base_opts1 |= RTL_RTK_TX_HWLKUP;
	if (txm.bridge)
		base_opts1 |= RTL_RTK_TX_BRIDGE;

	if (skb->ip_summed == CHECKSUM_PARTIAL && !skb_is_gso(skb)) {
		if (rtl8197f_rtk_tx_csum_bits(priv, skb, &base_opts1,
					      &base_opts3, &base_opts4, &base_opts5)) {
			spin_unlock_irqrestore(&priv->tx_lock, flags);
			ret = -EINVAL;
			goto unmap_all;
		}
	}
	rtl8197f_rtk_tx_tso_bits(priv, skb, &base_opts1,
				   &base_opts3, &base_opts4, &base_opts5);

	rtl8197f_rtk_mw5_tx_wire_trace(priv, skb);

	for (seg = 0; seg < needed; seg++) {
		struct rtl8197f_rtk_desc *desc;
		u32 o1, o2;

		idx = priv->tx_head;
		desc = rtl8197f_rtk_tx_desc(priv, idx);
		o1 = base_opts1;
		if (seg == 0)
			o1 |= RTL_RTK_DESC_FIRST;
		if (seg == needed - 1)
			o1 |= RTL_RTK_DESC_LAST;
		if (!priv->is_mw5 && idx == priv->tx_ring_size - 1)
			o1 |= RTL_RTK_DESC_WRAP;
		/* Each SG descriptor carries total packet length in PH_LEN; M_LEN is
		 * the mapped segment length. SDK adds FCS only to the final segment. */
		o2 = (mapped_len[seg] + ((seg == needed - 1 && priv->sdk_crc_lengths) ? ETH_FCS_LEN : 0))
			<< RTL_RTK_TX_M_LEN_SHIFT;
		if (hw_qos)
			o2 |= (qid & 7) << RTL_RTK_TX_QID_SHIFT |
			      (qid & 7) << RTL_RTK_TX_PQID_SHIFT;
		desc->addr = cpu_to_le32(rtl8197f_rtk_hw_dma_addr(priv, mapped[seg]));
		desc->opts2 = cpu_to_le32(o2);
		desc->opts3 = cpu_to_le32(base_opts3);
		desc->opts4 = cpu_to_le32(base_opts4);
		desc->opts5 = cpu_to_le32(base_opts5);
		rtl8197f_rtk_desc_clear_padding(priv, desc);
		priv->tx_dma[idx] = coherent_tx ? 0 : mapped[seg];
		priv->tx_len[idx] = mapped_len[seg];
		priv->tx_dma_is_page[idx] = coherent_tx ? false : mapped_page[seg];
		priv->tx_skb[idx] = NULL;
		priv->tx_pkt_len[idx] = 0;
		/* All fragment OWN bits may be exposed now; first descriptor OWN is
		 * intentionally published last, matching New_swNic_send_tso_sg(). */
		if (seg != 0)
			desc->opts1 = cpu_to_le32(o1 | RTL_RTK_DESC_OWN);
		else
			desc->opts1 = cpu_to_le32(o1);
		last_idx = idx;
		priv->tx_head = rtl8197f_rtk_next(idx, priv->tx_ring_size);
	}
	priv->tx_skb[last_idx] = skb;
	priv->tx_pkt_len[last_idx] = pkt_len;
	if (needed > 1) {
		priv->tx_sg_packets++;
		priv->tx_sg_descs += needed;
	}
	dma_wmb();
	/* First descriptor gates the entire packet. */
	idx = (priv->tx_head + priv->tx_ring_size - needed) % priv->tx_ring_size;
	WRITE_ONCE(rtl8197f_rtk_tx_desc(priv, idx)->opts1,
		   cpu_to_le32(le32_to_cpu(rtl8197f_rtk_tx_desc(priv, idx)->opts1) |
			       RTL_RTK_DESC_OWN));

	ring_low = rtl8197f_rtk_tx_avail(priv) <= MAX_SKB_FRAGS + 1;
	if (ring_low)
		netif_stop_queue(ndev);
	netif_trans_update(ndev);

	/* Linux BQL + xmit_more coalesces descriptor publication into fewer MMIO
	 * TXFD doorbells. __netdev_sent_queue() forces a kick at the end of a batch
	 * and whenever the queue has been stopped because descriptor space is low.
	 */
	xmit_more = netdev_xmit_more();
	doorbell = __netdev_sent_queue(ndev, pkt_len, xmit_more);
	priv->tx_bql_sent_bytes += pkt_len;
	if (doorbell)
		rtl8197f_rtk_tx_doorbell(priv);
	else
		priv->tx_kick_deferred++;
	/* Keep the existing trace only for single-descriptor startup packets. */
	if (needed == 1)
		rtl8197f_rtk_mw5_tx_submit_trace(priv, idx, mapped[0],
			le32_to_cpu(rtl8197f_rtk_tx_desc(priv, idx)->opts1),
			le32_to_cpu(rtl8197f_rtk_tx_desc(priv, idx)->opts2),
			base_opts3, base_opts4, base_opts5);
	spin_unlock_irqrestore(&priv->tx_lock, flags);
	return NETDEV_TX_OK;

map_error:
	priv->tx_dma_errors++;
	ndev->stats.tx_dropped++;
	ret = -ENOMEM;
unmap_all:
	/* dma_pool_alloc() returns permanently coherent TX slots. They are owned
	 * by the ring for the lifetime of the interface and must never be passed
	 * through dma_unmap_* on an xmit error path. */
	if (!coherent_tx) {
		for (seg = 0; seg < needed; seg++) {
			if (!mapped[seg])
				continue;
			if (mapped_page[seg])
				dma_unmap_page(priv->dev, mapped[seg], mapped_len[seg], DMA_TO_DEVICE);
			else
				dma_unmap_single(priv->dev, mapped[seg], mapped_len[seg], DMA_TO_DEVICE);
		}
	}
	if (ret == -EBUSY)
		return NETDEV_TX_BUSY;
	dev_kfree_skb_any(skb);
	return NETDEV_TX_OK;
}


static int rtl8197f_rtk_open(struct net_device *ndev)
{
	struct rtl8197f_rtknet_priv *priv = netdev_priv(ndev);
	int ret;

	/* v44.65.6: page_pool is a reopen-time policy, not a probe-time latch.
	 * This lets the recovery CLI move a running MW5 back to strict
	 * map/unmap ownership without requiring a full router reboot. */
	if (priv->is_mw5) {
		WRITE_ONCE(priv->rx_publish_wait_pending, false);
		rtl8197f_rtk_mw5_rx_seq_reset(priv);
		/* Coherent fixed DMA is now part of the MW5 network baseline rather
		 * than a competing accelerator. Keep page_pool/streaming ownership
		 * retired even when the optional acceleration policy is switched off. */
		priv->rx_coherent_enabled = true;
		priv->tx_coherent_enabled = true;
		priv->rx_page_pool_enabled = false;
	}

	ret = rtl8197f_rtk_alloc_rings(priv);
	if (ret)
		return ret;

	ret = request_irq(priv->irq, rtl8197f_rtk_irq, 0, ndev->name, ndev);
	if (ret)
		goto err_free_rings;

	napi_enable(&priv->napi);
	rtl8197f_rtk_hw_start(priv);
	if (priv->is_mw5)
		rtl8197f_rtk_mw5_rx_update_hw_seq(priv, NULL);
	priv->rx_poll_last_irq_rx_done = READ_ONCE(priv->irq_rx_done);
	if (priv->phydev) {
		netif_carrier_off(ndev);
		phy_start(priv->phydev);
	} else {
		netif_carrier_on(ndev);
	}
	netif_start_queue(ndev);
	if (priv->rx_poll_fallback)
		mod_timer(&priv->rx_poll_timer,
			  jiffies + msecs_to_jiffies(priv->rx_poll_ms));
	if (priv->legacy_rd05_pipeline &&
	    !of_machine_is_compatible("tenda,nova-mw5"))
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
	if (priv->phydev)
		phy_stop(priv->phydev);
	netif_carrier_off(ndev);
	cancel_delayed_work_sync(&priv->rd05_reseed_work);
	del_timer_sync(&priv->rx_poll_timer);
	WRITE_ONCE(priv->rx_publish_wait_pending, false);
	WRITE_ONCE(priv->mw5_rx_catchup_pending, false);
	priv->mw5_rx_seq_valid = false;
	rtl8197f_mw5_accel_sw_flush(priv);
	rtl8197f_rtk_napt_flush(priv, true);
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

	/* v44.65.18: a MW5 timeout during OpenSpeedTest was secondary to an RX
	 * NAPI re-arm livelock. The old recovery stopped both RX and TX, rewound
	 * descriptor bases and replayed the rtl865x/DSA ingress tables while the
	 * interface was live. That turned a transient starvation into a persistent
	 * LAN/WLAN -> WAN outage.
	 *
	 * MW5 recovery is deliberately TX-only and non-destructive: reclaim any
	 * CDP-confirmed completions, re-ring TXFD/TRXRDY, and schedule NAPI to
	 * finish reclamation. Never reset RX descriptors or L2/ACL/DSA state here.
	 */
	if (priv->is_mw5) {
		unsigned int cleaned, avail, pending;
		u32 cdp;

		netif_stop_queue(ndev);
		cleaned = rtl8197f_rtk_tx_clean(priv);
		pending = (READ_ONCE(priv->tx_head) + priv->tx_ring_size -
			   READ_ONCE(priv->tx_tail)) % priv->tx_ring_size;
		avail = rtl8197f_rtk_tx_avail(priv);
		cdp = rtl8197f_rtk_read(priv, RTL_RTK_CPUTPDCR0);

		if (pending) {
			dma_wmb();
			rtl8197f_rtk_tx_doorbell(priv);
			rtl8197f_rtk_assert_trxrdy(priv);
		}

		if (napi_schedule_prep(&priv->napi)) {
			rtl8197f_rtk_disable_irq(priv);
			__napi_schedule(&priv->napi);
		}

		if (avail > MAX_SKB_FRAGS + 1)
			netif_wake_queue(ndev);

		netdev_warn(ndev,
			    "MW5 TX timeout soft-recovery: cleaned=%u pending=%u avail=%u cdp=0x%08x head=%u tail=%u own_defer=%llu mismatch=%llu defer_streak=%u defer_max=%llu\n",
			    cleaned, pending, avail, cdp,
			    READ_ONCE(priv->tx_head), READ_ONCE(priv->tx_tail),
			    (unsigned long long)priv->mw5_tx_own_defer,
			    (unsigned long long)priv->mw5_tx_completion_mismatch,
			    priv->mw5_tx_own_defer_streak,
			    (unsigned long long)priv->mw5_tx_own_defer_max);
		return;
	}

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

	ring->rx_max_pending = RTL_RTK_MAX_RX_RING;
	ring->tx_max_pending = RTL_RTK_MAX_TX_RING;
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

	rx = clamp_t(u32, ring->rx_pending, RTL_RTK_MIN_RING, RTL_RTK_MAX_RX_RING);
	tx = clamp_t(u32, ring->tx_pending, RTL_RTK_MIN_RING, RTL_RTK_MAX_TX_RING);
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
	struct rtl8197f_rtk_desc *tx0 = priv->tx_desc ?
		rtl8197f_rtk_tx_desc(priv, 0) : NULL;
	struct rtl8197f_rtk_desc *rx0 = priv->rx_desc ?
		rtl8197f_rtk_rx_desc(priv, 0) : NULL;

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
	data[RTL_RTK_STAT_RX_POLL_IRQ_PROGRESS_SKIPS] = priv->rx_poll_irq_progress_skips;
	data[RTL_RTK_STAT_RX_POLL_FALLBACK_SCHEDULES] = priv->rx_poll_fallback_schedules;
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
	data[RTL_RTK_STAT_DESC_STRIDE] = priv->desc_stride;
	data[RTL_RTK_STAT_TX_DESC0_OPTS1] = tx0 ? le32_to_cpu(tx0->opts1) : 0;
	data[RTL_RTK_STAT_TX_DESC0_OPTS2] = tx0 ? le32_to_cpu(tx0->opts2) : 0;
	data[RTL_RTK_STAT_TX_DESC0_OPTS4] = tx0 ? le32_to_cpu(tx0->opts4) : 0;
	data[RTL_RTK_STAT_RX_DESC0_OPTS1] = rx0 ? le32_to_cpu(rx0->opts1) : 0;
	data[RTL_RTK_STAT_RX_DESC0_OPTS2] = rx0 ? le32_to_cpu(rx0->opts2) : 0;
	data[RTL_RTK_STAT_RX_DESC0_OPTS3] = rx0 ? le32_to_cpu(rx0->opts3) : 0;
	data[RTL_RTK_STAT_RX_DESC0_OPTS4] = rx0 ? le32_to_cpu(rx0->opts4) : 0;
	data[RTL_RTK_STAT_RX_DESC0_OPTS5] = rx0 ? le32_to_cpu(rx0->opts5) : 0;
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



/* -------------------------------------------------------------------------
 * MW5 firewall-owned MW5 accelerator software path fast path
 * -------------------------------------------------------------------------
 * This is deliberately not the legacy rtl_wlanFwdToEth() path.  Entries are
 * created only by TC_SETUP_FT, i.e. after nftables/conntrack/nf_flow_table has
 * accepted the flow.  The fast path handles only routed IPv4 TCP/UDP NAT
 * between the WAN DSA slave and an associated WLAN netdev.  LAN<->WLAN bridge
 * traffic is never eligible and therefore always traverses Linux bridge and
 * nft bridge hooks.
 */
struct rtl8197f_mw5_accel_sw_flow {
	struct list_head list;
	struct hlist_node hnode;
	u16 hash_bucket;
	unsigned long cookie;
	__be32 src, dst, new_src, new_dst;
	__be16 sport, dport, new_sport, new_dport;
	u8 proto;
	u8 h_source[ETH_ALEN];
	u8 h_dest[ETH_ALEN];
	struct net_device *in_dev;
	struct net_device *out_dev;
	u32 refs;
};

static struct rtl8197f_rtknet_priv *rtl8197f_mw5_runtime;

static bool rtl8197f_mw5_accel_sw_dev_is_wlan(const struct net_device *dev)
{
	return dev && !strncmp(dev->name, "wlan", 4);
}

static bool rtl8197f_mw5_accel_sw_dev_is_wan(const struct net_device *dev)
{
	return dev && !strcmp(dev->name, "wan");
}

static bool rtl8197f_mw5_accel_sw_dev_is_lan(const struct net_device *dev)
{
	return dev && !strcmp(dev->name, "lan");
}

static bool rtl8197f_mw5_accel_sw_dev_is_br_lan(const struct net_device *dev)
{
	return dev && !strcmp(dev->name, "br-lan");
}

static bool rtl8197f_mw5_accel_sw_dev_is_private(const struct net_device *dev)
{
	return rtl8197f_mw5_accel_sw_dev_is_lan(dev) ||
	       rtl8197f_mw5_accel_sw_dev_is_br_lan(dev) ||
	       rtl8197f_mw5_accel_sw_dev_is_wlan(dev);
}

/* v44.65.20: nft flowtable metadata may name br-lan, lan or a WLAN netdev,
 * while the RX hot path sees the physical DSA/WLAN ingress. Treat all private
 * LAN-side interfaces as one authorization class so a flow installed for
 * br-lan can match the same packet when it arrives from lan/wlan0/wlan1.
 * 1=wan, 2=private. Zero is deliberately ineligible.
 */
static u8 rtl8197f_mw5_accel_sw_ingress_class(const struct net_device *dev)
{
	if (rtl8197f_mw5_accel_sw_dev_is_wan(dev))
		return 1;
	if (rtl8197f_mw5_accel_sw_dev_is_private(dev))
		return 2;
	return 0;
}

static u16 rtl8197f_mw5_accel_sw_hash(__be32 src, __be32 dst,
                                      __be16 sport, __be16 dport, u8 proto,
                                      u8 ingress_class)
{
	u32 ports = ((__force u16)sport << 16) | (__force u16)dport;
	u32 v = (__force u32)src ^ rol32((__force u32)dst, 11) ^
	        rol32(ports, 5) ^ ((u32)proto << 24) ^
	        ((u32)ingress_class * 0x9e3779b9U);

	return hash_32(v, MW5_ACCEL_SW_HASH_BITS);
}

static int rtl8197f_mw5_accel_sw_mangle_port(const struct flow_action_entry *act,
					 __be16 *src, __be16 *dst)
{
	u32 val = be32_to_cpu((__force __be32)act->mangle.val);

	switch (act->mangle.offset) {
	case 0:
		if ((__force __be32)act->mangle.mask == ~cpu_to_be32(0xffff))
			*dst = cpu_to_be16(val);
		else
			*src = cpu_to_be16(val >> 16);
		break;
	case 2:
		*dst = cpu_to_be16(val);
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int rtl8197f_mw5_accel_sw_mangle_eth(const struct flow_action_entry *act,
					u8 *eth, u16 *known)
{
	u8 *dest;
	const u8 *src = (const u8 *)&act->mangle.val;
	u32 off = act->mangle.offset;

	if (off > 8)
		return -EOPNOTSUPP;
	dest = eth + off;
	if (act->mangle.mask == 0xffff) {
		src += 2;
		dest += 2;
		off += 2;
	}
	if (act->mangle.mask) {
		memcpy(dest, src, 2);
		*known |= BIT(off) | BIT(off + 1);
	} else {
		memcpy(dest, src, 4);
		*known |= BIT(off) | BIT(off + 1) | BIT(off + 2) | BIT(off + 3);
	}
	return 0;
}

static bool rtl8197f_mw5_accel_sw_flow_equal(const struct rtl8197f_mw5_accel_sw_flow *a,
					 const struct rtl8197f_mw5_accel_sw_flow *b)
{
	return a->src == b->src && a->dst == b->dst &&
	       a->new_src == b->new_src && a->new_dst == b->new_dst &&
	       a->sport == b->sport && a->dport == b->dport &&
	       a->new_sport == b->new_sport && a->new_dport == b->new_dport &&
	       a->proto == b->proto && a->in_dev == b->in_dev &&
	       a->out_dev == b->out_dev &&
	       ether_addr_equal(a->h_source, b->h_source) &&
	       ether_addr_equal(a->h_dest, b->h_dest);
}

static int rtl8197f_mw5_accel_sw_rule_to_flow(struct flow_cls_offload *f,
					  struct rtl8197f_mw5_accel_sw_flow *wf,
					  struct net_device *source)
{
	struct flow_rule *rule = flow_cls_offload_flow_rule(f);
	struct flow_match_control control;
	struct flow_match_basic basic;
	struct flow_match_ipv4_addrs addrs;
	struct flow_match_ports ports;
	struct flow_action_entry *act;
	u8 eth[ETH_ALEN * 2] = {};
	u16 eth_known = 0;
	int i, ret;
	bool nat_changed;

	if (!flow_rule_match_key(rule, FLOW_DISSECTOR_KEY_CONTROL) ||
	    !flow_rule_match_key(rule, FLOW_DISSECTOR_KEY_BASIC) ||
	    !flow_rule_match_key(rule, FLOW_DISSECTOR_KEY_IPV4_ADDRS) ||
	    !flow_rule_match_key(rule, FLOW_DISSECTOR_KEY_PORTS))
		return -EOPNOTSUPP;

	flow_rule_match_control(rule, &control);
	if (control.key->addr_type != FLOW_DISSECTOR_KEY_IPV4_ADDRS ||
	    flow_rule_has_control_flags(control.mask->flags, f->common.extack))
		return -EOPNOTSUPP;
	flow_rule_match_basic(rule, &basic);
	if (basic.key->n_proto != htons(ETH_P_IP) ||
	    basic.mask->n_proto != htons(0xffff) || basic.mask->ip_proto != 0xff ||
	    (basic.key->ip_proto != IPPROTO_TCP && basic.key->ip_proto != IPPROTO_UDP))
		return -EOPNOTSUPP;
	flow_rule_match_ipv4_addrs(rule, &addrs);
	flow_rule_match_ports(rule, &ports);
	if (addrs.mask->src != cpu_to_be32(~0U) ||
	    addrs.mask->dst != cpu_to_be32(~0U) ||
	    ports.mask->src != cpu_to_be16(0xffff) ||
	    ports.mask->dst != cpu_to_be16(0xffff))
		return -EOPNOTSUPP;

	memset(wf, 0, sizeof(*wf));
	/* The callback endpoint is part of the authorization boundary. A rule
	 * installed on LAN/master/another WLAN must never become eligible merely
	 * because its L3 tuple happens to match an accelerated connection. */
	if (!rtl8197f_mw5_accel_sw_ingress_class(source))
		return -EOPNOTSUPP;
	wf->in_dev = source;
	wf->cookie = f->cookie;
	wf->src = wf->new_src = addrs.key->src;
	wf->dst = wf->new_dst = addrs.key->dst;
	wf->sport = wf->new_sport = ports.key->src;
	wf->dport = wf->new_dport = ports.key->dst;
	wf->proto = basic.key->ip_proto;

	flow_action_for_each(i, act, &rule->action) {
		switch (act->id) {
		case FLOW_ACTION_MANGLE:
			switch (act->mangle.htype) {
			case FLOW_ACT_MANGLE_HDR_TYPE_IP4:
				if (act->mangle.mask)
					return -EOPNOTSUPP;
				if (act->mangle.offset == offsetof(struct iphdr, saddr))
					memcpy(&wf->new_src, &act->mangle.val, sizeof(wf->new_src));
				else if (act->mangle.offset == offsetof(struct iphdr, daddr))
					memcpy(&wf->new_dst, &act->mangle.val, sizeof(wf->new_dst));
				else
					return -EOPNOTSUPP;
				break;
			case FLOW_ACT_MANGLE_HDR_TYPE_TCP:
				if (wf->proto != IPPROTO_TCP)
					return -EOPNOTSUPP;
				ret = rtl8197f_mw5_accel_sw_mangle_port(act, &wf->new_sport, &wf->new_dport);
				if (ret)
					return ret;
				break;
			case FLOW_ACT_MANGLE_HDR_TYPE_UDP:
				if (wf->proto != IPPROTO_UDP)
					return -EOPNOTSUPP;
				ret = rtl8197f_mw5_accel_sw_mangle_port(act, &wf->new_sport, &wf->new_dport);
				if (ret)
					return ret;
				break;
			case FLOW_ACT_MANGLE_HDR_TYPE_ETH:
				ret = rtl8197f_mw5_accel_sw_mangle_eth(act, eth, &eth_known);
				if (ret)
					return ret;
				break;
			default:
				return -EOPNOTSUPP;
			}
			break;
		case FLOW_ACTION_REDIRECT:
			if (wf->out_dev)
				return -EOPNOTSUPP;
			wf->out_dev = act->dev;
			break;
		case FLOW_ACTION_CSUM:
			break;
		default:
			/* VLAN/PPPoE/bridge actions stay on Linux until separately proven. */
			return -EOPNOTSUPP;
		}
	}

	if (!wf->out_dev || !rtl8197f_mw5_accel_sw_ingress_class(wf->out_dev))
		return -EOPNOTSUPP;
	/* Only WAN<->private directions are eligible. br-lan, lan and WLAN are
	 * one private zone; private<->private and WAN<->WAN stay on Linux.
	 */
	if (rtl8197f_mw5_accel_sw_dev_is_wan(source) ==
	    rtl8197f_mw5_accel_sw_dev_is_wan(wf->out_dev))
		return -EOPNOTSUPP;
	/* Both Ethernet addresses must come from nf_flow_table/neighbour state.
	 * Never invent a next hop in the shortcut. */
	if ((eth_known & GENMASK(11, 0)) != GENMASK(11, 0))
		return -EOPNOTSUPP;
	ether_addr_copy(wf->h_dest, eth);
	ether_addr_copy(wf->h_source, eth + ETH_ALEN);
	if (!is_valid_ether_addr(wf->h_dest) || !is_valid_ether_addr(wf->h_source))
		return -EINVAL;

	nat_changed = wf->new_src != wf->src || wf->new_dst != wf->dst ||
		      wf->new_sport != wf->sport || wf->new_dport != wf->dport;
	/* Requiring NAT makes the eligibility boundary unambiguously L3-routed;
	 * bridge-only WLAN<->LAN flows can never enter MW5 accelerator. */
	if (!nat_changed)
		return -EOPNOTSUPP;
	wf->hash_bucket = rtl8197f_mw5_accel_sw_hash(wf->src, wf->dst,
						  wf->sport, wf->dport, wf->proto,
						  rtl8197f_mw5_accel_sw_ingress_class(wf->in_dev));
	return 0;
}

static int rtl8197f_mw5_accel_sw_replace(struct rtl8197f_rtknet_priv *priv,
				     struct flow_cls_offload *f,
				     struct net_device *source)
{
	struct rtl8197f_mw5_accel_sw_flow *wf, *it;
	unsigned long flags;
	int ret;

	if (!priv->is_mw5 || !priv->accel_sw) {
		priv->accel_sw_replace_fallback++;
		return -EOPNOTSUPP;
	}
	wf = kzalloc(sizeof(*wf), GFP_KERNEL);
	if (!wf)
		return -ENOMEM;
	ret = rtl8197f_mw5_accel_sw_rule_to_flow(f, wf, source);
	if (ret) {
		kfree(wf);
		priv->accel_sw_replace_fallback++;
		return ret;
	}
	dev_hold(wf->in_dev);
	dev_hold(wf->out_dev);
	wf->refs = 1;
	INIT_HLIST_NODE(&wf->hnode);

	spin_lock_irqsave(&priv->accel_sw_lock, flags);
	list_for_each_entry(it, &priv->accel_sw_flows, list) {
		if (it->cookie != wf->cookie || it->in_dev != wf->in_dev)
			continue;
		/* A cookie may legitimately be used for the opposite flow direction
		 * on a second endpoint.  It must not, however, silently mutate while
		 * bound to the same ingress endpoint. */
		if (!rtl8197f_mw5_accel_sw_flow_equal(it, wf)) {
			spin_unlock_irqrestore(&priv->accel_sw_lock, flags);
			dev_put(wf->in_dev);
			dev_put(wf->out_dev);
			kfree(wf);
			priv->accel_sw_replace_fallback++;
			return -EEXIST;
		}
		it->refs++;
		spin_unlock_irqrestore(&priv->accel_sw_lock, flags);
		dev_put(wf->in_dev);
		dev_put(wf->out_dev);
		kfree(wf);
		priv->accel_sw_replace_ok++;
		return 0;
	}
	list_add_tail(&wf->list, &priv->accel_sw_flows);
	hlist_add_head(&wf->hnode, &priv->accel_sw_hash[wf->hash_bucket]);
	priv->accel_sw_active++;
	spin_unlock_irqrestore(&priv->accel_sw_lock, flags);
	priv->accel_sw_replace_ok++;
	return 0;
}

static int rtl8197f_mw5_accel_sw_destroy(struct rtl8197f_rtknet_priv *priv,
				     struct flow_cls_offload *f,
				     struct net_device *source)
{
	struct rtl8197f_mw5_accel_sw_flow *wf, *tmp, *dead = NULL;
	unsigned long flags;

	if (!priv->is_mw5)
		return 0;
	spin_lock_irqsave(&priv->accel_sw_lock, flags);
	list_for_each_entry_safe(wf, tmp, &priv->accel_sw_flows, list) {
		if (wf->cookie != f->cookie || (source && wf->in_dev != source))
			continue;
		if (--wf->refs == 0) {
			hlist_del_init(&wf->hnode);
			list_del(&wf->list);
			if (priv->accel_sw_active)
				priv->accel_sw_active--;
			dead = wf;
		}
		break;
	}
	spin_unlock_irqrestore(&priv->accel_sw_lock, flags);
	if (dead) {
		dev_put(dead->in_dev);
		dev_put(dead->out_dev);
		kfree(dead);
	}
	priv->accel_sw_destroy_ok++;
	return 0;
}

static void rtl8197f_mw5_accel_sw_flush(struct rtl8197f_rtknet_priv *priv)
{
	LIST_HEAD(dead);
	struct rtl8197f_mw5_accel_sw_flow *wf, *tmp;
	unsigned long flags;

	if (!priv || !priv->is_mw5)
		return;
	spin_lock_irqsave(&priv->accel_sw_lock, flags);
	list_splice_init(&priv->accel_sw_flows, &dead);
	list_for_each_entry(wf, &dead, list)
		hlist_del_init(&wf->hnode);
	priv->accel_sw_active = 0;
	spin_unlock_irqrestore(&priv->accel_sw_lock, flags);
	list_for_each_entry_safe(wf, tmp, &dead, list) {
		list_del(&wf->list);
		dev_put(wf->in_dev);
		dev_put(wf->out_dev);
		kfree(wf);
	}
	priv->accel_sw_flushes++;
}

void rtl8197f_mw5_accel_purge_dev(struct net_device *dev)
{
	struct rtl8197f_rtknet_priv *priv = READ_ONCE(rtl8197f_mw5_runtime);
	LIST_HEAD(dead);
	struct rtl8197f_mw5_accel_sw_flow *wf, *tmp;
	unsigned long flags;

	if (!priv || !priv->is_mw5 || !dev)
		return;
	spin_lock_irqsave(&priv->accel_sw_lock, flags);
	list_for_each_entry_safe(wf, tmp, &priv->accel_sw_flows, list) {
		if (wf->in_dev != dev && wf->out_dev != dev)
			continue;
		hlist_del_init(&wf->hnode);
		list_move_tail(&wf->list, &dead);
		if (priv->accel_sw_active)
			priv->accel_sw_active--;
	}
	spin_unlock_irqrestore(&priv->accel_sw_lock, flags);
	list_for_each_entry_safe(wf, tmp, &dead, list) {
		list_del(&wf->list);
		dev_put(wf->in_dev);
		dev_put(wf->out_dev);
		kfree(wf);
	}
}
EXPORT_SYMBOL_GPL(rtl8197f_mw5_accel_purge_dev);

static bool rtl8197f_mw5_accel_sw_tuple_match(const struct rtl8197f_mw5_accel_sw_flow *wf,
					  const struct iphdr *iph,
					  __be16 sport, __be16 dport)
{
	return wf->proto == iph->protocol && wf->src == iph->saddr &&
	       wf->dst == iph->daddr && wf->sport == sport && wf->dport == dport;
}

static int rtl8197f_mw5_accel_sw_forward(struct rtl8197f_rtknet_priv *priv,
				    struct sk_buff *skb)
{
	struct rtl8197f_mw5_accel_sw_flow snap, *wf;
	struct net_device *out = NULL;
	struct sk_buff *nskb;
	struct ethhdr *eth;
	struct iphdr *iph;
	struct tcphdr *th = NULL;
	struct udphdr *uh = NULL;
	unsigned long flags;
	unsigned int l3off = ETH_HLEN, l4len;
	__be16 sport, dport;
	u16 outer, bucket;
	u32 steps = 0;
	u8 ingress_class = 0;
	bool dsa_in = false, udp_zero = false;
	int ret;

	if (!priv || !priv->is_mw5 || !READ_ONCE(mw5_accel_enable) || !READ_ONCE(priv->accel_sw) ||
	    !skb || !skb->dev || skb->len < ETH_HLEN + sizeof(struct iphdr))
		return 0;
	/* v44.65.20: until nf_flow_table has installed at least one exact rule,
	 * avoid all DSA/IP parsing and hash work in the single-core RX hot path.
	 */
	if (!READ_ONCE(priv->accel_sw_active)) {
		priv->accel_sw_idle_bypass++;
		return 0;
	}
	if (is_multicast_ether_addr(skb->data))
		return 0;

	outer = get_unaligned_be16(skb->data + 2 * ETH_ALEN);
	if (outer == ETH_P_REALTEK) {
		u16 proto_reason, source_word;

		if (skb->len < ETH_HLEN + RTL_RTK_MW5_CPU_TAG8_LEN + sizeof(struct iphdr))
			return 0;
		proto_reason = get_unaligned_be16(skb->data + 2 * ETH_ALEN + 2);
		if ((proto_reason & 0xff00) != RTL_RTK_MW5_RTL8_PROTO_WORD) {
			priv->accel_sw_guard_reject++;
			return 0;
		}
		source_word = get_unaligned_be16(skb->data + 2 * ETH_ALEN + 6);
		switch (source_word & 0xf) {
		case 1:
			ingress_class = 2; /* LAN P1 */
			break;
		case 3:
			ingress_class = 1; /* WAN P3 */
			break;
		default:
			priv->accel_sw_guard_reject++;
			return 0;
		}
		if (get_unaligned_be16(skb->data + 2 * ETH_ALEN + RTL_RTK_MW5_CPU_TAG8_LEN) != ETH_P_IP)
			return 0;
		l3off += RTL_RTK_MW5_CPU_TAG8_LEN;
		dsa_in = true;
	} else {
		/* Untagged ingress is eligible only when it came from running WLAN. */
		if (outer != ETH_P_IP || !rtl8197f_mw5_accel_sw_dev_is_wlan(skb->dev) ||
		    !netif_running(skb->dev))
			return 0;
		ingress_class = 2;
	}

	if (!pskb_may_pull(skb, l3off + sizeof(struct iphdr)))
		return 0;
	iph = (struct iphdr *)(skb->data + l3off);
	if (iph->version != 4 || iph->ihl != 5 || ip_is_fragment(iph) || iph->ttl <= 1 ||
	    (iph->protocol != IPPROTO_TCP && iph->protocol != IPPROTO_UDP)) {
		priv->accel_sw_guard_reject++;
		return 0;
	}
	if (ntohs(iph->tot_len) < sizeof(*iph) + sizeof(struct udphdr) ||
	    l3off + ntohs(iph->tot_len) > skb->len)
		return 0;
	if (iph->protocol == IPPROTO_TCP) {
		if (!pskb_may_pull(skb, l3off + sizeof(*iph) + sizeof(struct tcphdr)))
			return 0;
		iph = (struct iphdr *)(skb->data + l3off);
		th = (struct tcphdr *)((u8 *)iph + sizeof(*iph));
		if (th->syn || th->fin || th->rst) {
			priv->accel_sw_guard_reject++;
			return 0;
		}
		sport = th->source;
		dport = th->dest;
	} else {
		if (!pskb_may_pull(skb, l3off + sizeof(*iph) + sizeof(struct udphdr)))
			return 0;
		iph = (struct iphdr *)(skb->data + l3off);
		uh = (struct udphdr *)((u8 *)iph + sizeof(*iph));
		sport = uh->source;
		dport = uh->dest;
	}

	memset(&snap, 0, sizeof(snap));
	bucket = rtl8197f_mw5_accel_sw_hash(iph->saddr, iph->daddr, sport, dport,
					       iph->protocol, ingress_class);
	spin_lock_irqsave(&priv->accel_sw_lock, flags);
	priv->accel_sw_hash_lookups++;
	hlist_for_each_entry(wf, &priv->accel_sw_hash[bucket], hnode) {
		steps++;
		if (!rtl8197f_mw5_accel_sw_tuple_match(wf, iph, sport, dport))
			continue;
		if ((ingress_class == 3 &&
		     (wf->in_dev != skb->dev || !rtl8197f_mw5_accel_sw_dev_is_wlan(wf->in_dev) ||
		      !rtl8197f_mw5_accel_sw_dev_is_wan(wf->out_dev))) ||
		    (ingress_class == 2 &&
		     (!rtl8197f_mw5_accel_sw_dev_is_lan(wf->in_dev) ||
		      !rtl8197f_mw5_accel_sw_dev_is_wan(wf->out_dev))) ||
		    (ingress_class == 1 &&
		     (!rtl8197f_mw5_accel_sw_dev_is_wan(wf->in_dev) ||
		      (!rtl8197f_mw5_accel_sw_dev_is_lan(wf->out_dev) &&
		       !rtl8197f_mw5_accel_sw_dev_is_wlan(wf->out_dev)))))
			continue;
		snap = *wf;
		out = wf->out_dev;
		dev_hold(out);
		break;
	}
	priv->accel_sw_hash_steps += steps;
	if (steps > priv->accel_sw_hash_max_steps)
		priv->accel_sw_hash_max_steps = steps;
	spin_unlock_irqrestore(&priv->accel_sw_lock, flags);
	if (!out) {
		priv->accel_sw_misses++;
		return 0;
	}

	if (!netif_running(out) ||
	    ((rtl8197f_mw5_accel_sw_dev_is_wan(out) || rtl8197f_mw5_accel_sw_dev_is_lan(out)) &&
	     !netif_carrier_ok(out))) {
		dev_put(out);
		priv->accel_sw_guard_reject++;
		return 0;
	}
	if (ntohs(iph->tot_len) > out->mtu) {
		dev_put(out);
		priv->accel_sw_guard_reject++;
		return 0;
	}

	/* Copy first: a failed guard/xmit leaves the original skb untouched so it
	 * can continue through the complete Linux bridge/netfilter path. */
	nskb = skb_copy(skb, GFP_ATOMIC);
	if (!nskb) {
		dev_put(out);
		return 0;
	}
	if (skb_linearize(nskb)) {
		kfree_skb(nskb);
		dev_put(out);
		return 0;
	}
	if (dsa_in) {
		memmove(nskb->data + 2 * ETH_ALEN,
			nskb->data + 2 * ETH_ALEN + RTL_RTK_MW5_CPU_TAG8_LEN,
			nskb->len - 2 * ETH_ALEN - RTL_RTK_MW5_CPU_TAG8_LEN);
		skb_trim(nskb, nskb->len - RTL_RTK_MW5_CPU_TAG8_LEN);
		l3off = ETH_HLEN;
	}

	eth = (struct ethhdr *)nskb->data;
	ether_addr_copy(eth->h_dest, snap.h_dest);
	ether_addr_copy(eth->h_source, snap.h_source);
	eth->h_proto = htons(ETH_P_IP);
	iph = (struct iphdr *)(nskb->data + l3off);
	iph->saddr = snap.new_src;
	iph->daddr = snap.new_dst;
	iph->ttl--;
	l4len = ntohs(iph->tot_len) - sizeof(*iph);
	if (iph->protocol == IPPROTO_TCP) {
		th = (struct tcphdr *)((u8 *)iph + sizeof(*iph));
		th->source = snap.new_sport;
		th->dest = snap.new_dport;
		th->check = 0;
		th->check = tcp_v4_check(l4len, iph->saddr, iph->daddr,
					 csum_partial((char *)th, l4len, 0));
	} else {
		uh = (struct udphdr *)((u8 *)iph + sizeof(*iph));
		udp_zero = uh->check == 0;
		uh->source = snap.new_sport;
		uh->dest = snap.new_dport;
		if (!udp_zero) {
			uh->check = 0;
			uh->check = udp_v4_check(l4len, iph->saddr, iph->daddr,
						 csum_partial((char *)uh, l4len, 0));
			if (!uh->check)
				uh->check = CSUM_MANGLED_0;
		}
	}
	iph->check = 0;
	ip_send_check(iph);
	nskb->dev = out;
	nskb->protocol = eth->h_proto;
	nskb->ip_summed = CHECKSUM_NONE;
	nskb->csum = 0;
	skb_reset_mac_header(nskb);
	skb_set_network_header(nskb, ETH_HLEN);
	skb_set_transport_header(nskb, ETH_HLEN + sizeof(*iph));

	ret = dev_queue_xmit(nskb);
	dev_put(out);
	if (net_xmit_eval(ret) == 0) {
		priv->accel_sw_hits++;
		priv->accel_sw_xmit_ok++;
		/* Hardware mode owns the verified RTL8197F DMA + DSA/WLAN egress
		 * stage.  It intentionally does not turn on the old autonomous
		 * RTL865x HWLOOKUP/NAPT path on DSA-tagged frames. */
		if (READ_ONCE(priv->accel_hw))
			priv->accel_hw_packets++;
		else
			priv->accel_hw_fallback++;
		consume_skb(skb);
		return 1;
	}
	priv->accel_sw_xmit_fail++;
	return 0;
}

int rtl8197f_mw5_accel_rx(struct sk_buff *skb)
{
	struct rtl8197f_rtknet_priv *priv = READ_ONCE(rtl8197f_mw5_runtime);

	return rtl8197f_mw5_accel_sw_forward(priv, skb);
}
EXPORT_SYMBOL_GPL(rtl8197f_mw5_accel_rx);


/* -------------------------------------------------------------------------
 * RTL865x NAPT mirror for Linux nf_flow_table
 * -------------------------------------------------------------------------
 *
 * Security model:
 *   - only TC_SETUP_FT (netfilter flowtable) is allowed to reach this encoder;
 *     ordinary tc flower remains a separate callback and cannot create NAPT.
 *   - rules must be exact IPv4 TCP/UDP NAT tuples with a known LAN/WAN redirect;
 *   - every successful Linux flow cookie owns a hardware reference;
 *   - destroy/unbind/ndo_stop synchronously removes the hardware state;
 *   - before the first flow, all legacy L3/L4 tables that could survive an OEM
 *     boot are scrubbed while L3/L4 lookup is disabled.
 *
 * This ports only the GPL SDK hash/table representation.  The old Realtek
 * conntrack hooks/FastPath callbacks are intentionally not used.
 */
static u8 rtl8197f_rtk_napt_age_unit(u32 sec)
{
    u32 value = 0, scale = 1;

    sec++;
    sec = sec * 3 / 5;
    while (sec >= (scale << 3)) {
        sec -= scale << 3;
        scale <<= 2;
        value++;
    }

    return (sec % scale > (scale >> 1)) ?
           ((value << 3) + sec / scale + 1) :
           ((value << 3) + sec / scale);
}

static u16 rtl8197f_rtk_napt_hash(bool is_tcp, __be32 src_addr,
                                  __be16 src_port, __be32 dst_addr,
                                  __be16 dst_port, bool verify)
{
    u32 saddr = (__force u32)src_addr;
    u32 daddr = (__force u32)dst_addr;
    u16 sport = (__force u16)src_port;
    u16 dport = (__force u16)dst_port;
    u32 proto = is_tcp ? 1 : 0;
    u32 eidx;

    if (verify)
        proto |= RTL_RTK_NAPT_HASH_FOR_VERI;

    if (!daddr && !dport && !(proto & RTL_RTK_NAPT_HASH_FOR_VERI)) {
        eidx = (saddr & 0x3ff) ^ ((saddr >> 10) & 0x3ff) ^
               ((saddr >> 20) & 0x3ff) ^ (sport & 0x3ff) ^
               (((proto & 1) << 8) | ((saddr & 0xc0000000) >> 24) |
                ((sport >> 10) & 0x3f));
        return eidx & (RTL_RTK_NAPT_TABLE_SIZE - 1);
    }

    eidx = (sport & 0x3ff) ^
           (((sport & 0xfc00) >> 10) | ((saddr & 0xf) << 6)) ^
           ((saddr >> 4) & 0x3ff) ^ ((saddr >> 14) & 0x3ff) ^
           (((saddr & 0xff000000) >> 24) | ((proto & 1) << 8) |
            ((dport & 1) << 9)) ^ ((dport >> 1) & 0x3ff) ^
           (((dport >> 11) & 0x1f) | ((daddr & 0x1f) << 5)) ^
           ((daddr >> 5) & 0x3ff) ^ ((daddr >> 15) & 0x3ff) ^
           ((daddr >> 25) & 0x7f);

    return eidx & (RTL_RTK_NAPT_TABLE_SIZE - 1);
}

static bool rtl8197f_rtk_napt_key_equal(const struct rtl8197f_rtk_napt_key *a,
                                        const struct rtl8197f_rtk_napt_key *b)
{
    return a->int_ip == b->int_ip && a->ext_ip == b->ext_ip &&
           a->rem_ip == b->rem_ip && a->int_port == b->int_port &&
           a->ext_port == b->ext_port && a->rem_port == b->rem_port &&
           a->proto == b->proto;
}

static struct rtl8197f_rtk_napt_cookie *
rtl8197f_rtk_napt_find_cookie(struct rtl8197f_rtknet_priv *priv,
                              unsigned long cookie)
{
    struct rtl8197f_rtk_napt_cookie *c;

    list_for_each_entry(c, &priv->napt_cookies, list)
        if (c->cookie == cookie)
            return c;
    return NULL;
}

static struct rtl8197f_rtk_napt_hw *
rtl8197f_rtk_napt_find_hw(struct rtl8197f_rtknet_priv *priv,
                          const struct rtl8197f_rtk_napt_key *key)
{
    struct rtl8197f_rtk_napt_hw *hw;

    list_for_each_entry(hw, &priv->napt_hw_flows, list)
        if (rtl8197f_rtk_napt_key_equal(&hw->key, key))
            return hw;
    return NULL;
}

static void rtl8197f_rtk_napt_set_l34(struct rtl8197f_rtknet_priv *priv,
                                      bool enable)
{
    u32 val;

    if (!priv->swcore)
        return;
    val = readl(priv->swcore + RTL_RTK_SWCORE_MSCR);
    val &= ~(RTL_RTK_MSCR_EN_L3 | RTL_RTK_MSCR_EN_L4);
    val |= RTL_RTK_MSCR_EN_L2;
    if (enable)
        val |= RTL_RTK_MSCR_EN_L3 | RTL_RTK_MSCR_EN_L4;
    writel(val, priv->swcore + RTL_RTK_SWCORE_MSCR);
    readl(priv->swcore + RTL_RTK_SWCORE_MSCR);
}

static int rtl8197f_rtk_napt_security_scrub(struct rtl8197f_rtknet_priv *priv)
{
    static const u32 zero[3];
    u32 i;
    int ret;

    if (priv->napt_hw_initialized)
        return 0;

    /* Fail closed before touching any table. */
    rtl8197f_rtk_napt_set_l34(priv, false);

    for (i = 0; i < RTL_RTK_NAPT_TABLE_SIZE; i++) {
        ret = rtl8197f_rtk_force_table_raw(priv, RTL_RTK_ASICTBL_TYPE_L4_TCPUDP,
                                            i, zero, 3);
        if (ret)
            return ret;
    }
    for (i = 0; i < RTL_RTK_EXTIP_TABLE_SIZE; i++) {
        ret = rtl8197f_rtk_force_table_raw(priv, RTL_RTK_ASICTBL_TYPE_EXT_INT_IP,
                                            i, zero, 3);
        if (ret)
            return ret;
    }
    /* Old OEM bootloaders/firmware can leave routing state behind.  It must
     * never become authoritative merely because OpenWrt enables L3/L4 lookup.
     */
    for (i = 0; i < RTL_RTK_ROUTE_TABLE_SIZE; i++) {
        ret = rtl8197f_rtk_force_table_raw(priv, RTL_RTK_ASICTBL_TYPE_ROUTE,
                                            i, zero, 3);
        if (ret)
            return ret;
    }
    for (i = 0; i < RTL_RTK_ARP_TABLE_SIZE; i++) {
        ret = rtl8197f_rtk_force_table_raw(priv, RTL_RTK_ASICTBL_TYPE_ARP,
                                            i, zero, 3);
        if (ret)
            return ret;
    }
    for (i = 0; i < RTL_RTK_NEXT_HOP_TABLE_SIZE; i++) {
        ret = rtl8197f_rtk_force_table_raw(priv, RTL_RTK_ASICTBL_TYPE_NEXT_HOP,
                                            i, zero, 3);
        if (ret)
            return ret;
    }

    bitmap_zero(priv->napt_used, RTL_RTK_NAPT_TABLE_SIZE);
    memset(priv->napt_extip, 0, sizeof(priv->napt_extip));
    memset(priv->napt_extip_ref, 0, sizeof(priv->napt_extip_ref));
    priv->napt_hw_initialized = true;
    priv->napt_flushes++;
    dev_info(priv->dev,
             "RTL865x flowtable security scrub: route/arp/nexthop/extip/napt cleared; L3/L4 remain gated\n");
    return 0;
}

static int rtl8197f_rtk_napt_extip_get(struct rtl8197f_rtknet_priv *priv,
                                       __be32 ext_ip, u8 *idx)
{
    u32 w[3] = { 0, (__force u32)ext_ip, BIT(0) };
    int free_idx = -1;
    u32 i;
    int ret;

    for (i = 0; i < RTL_RTK_EXTIP_TABLE_SIZE; i++) {
        if (priv->napt_extip_ref[i] && priv->napt_extip[i] == ext_ip) {
            priv->napt_extip_ref[i]++;
            *idx = i;
            return 0;
        }
        if (!priv->napt_extip_ref[i] && free_idx < 0)
            free_idx = i;
    }
    if (free_idx < 0)
        return -ENOSPC;

    ret = rtl8197f_rtk_force_table_raw(priv, RTL_RTK_ASICTBL_TYPE_EXT_INT_IP,
                                        free_idx, w, ARRAY_SIZE(w));
    if (ret)
        return ret;
    priv->napt_extip[free_idx] = ext_ip;
    priv->napt_extip_ref[free_idx] = 1;
    priv->napt_extip_alloc++;
    *idx = free_idx;
    return 0;
}

static void rtl8197f_rtk_napt_extip_put(struct rtl8197f_rtknet_priv *priv,
                                        u8 idx)
{
    static const u32 zero[3];

    if (idx >= RTL_RTK_EXTIP_TABLE_SIZE || !priv->napt_extip_ref[idx])
        return;
    if (--priv->napt_extip_ref[idx])
        return;

    rtl8197f_rtk_force_table_raw(priv, RTL_RTK_ASICTBL_TYPE_EXT_INT_IP,
                                  idx, zero, ARRAY_SIZE(zero));
    priv->napt_extip[idx] = 0;
    priv->napt_extip_free++;
}

static int rtl8197f_rtk_napt_program_one(struct rtl8197f_rtknet_priv *priv,
                                         const struct rtl8197f_rtk_napt_key *key,
                                         bool outbound, u16 index, u8 extip_idx)
{
    bool tcp = key->proto == IPPROTO_TCP;
    u16 ext_port = (__force u16)key->ext_port;
    u16 verify = rtl8197f_rtk_napt_hash(tcp, key->rem_ip, key->rem_port,
                                        0, 0, true);
    u32 timeout = tcp ? RTL_RTK_NAPT_TCP_AGE_SEC : RTL_RTK_NAPT_UDP_AGE_SEC;
    u32 offset = outbound ? (ext_port >> 10) : (ext_port & 0x3f);
    u32 sel_eidx = outbound ? (ext_port & 0x3ff) : (verify & 0x3ff);
    u32 sel_ip = outbound ? extip_idx : ((ext_port & 0x3ff) >> 6);
    u32 tcpflag = 0x2 | (outbound ? 0x1 : 0x0);
    u32 w[3];

    w[0] = (__force u32)key->int_ip;
    w[1] = BIT(0) | BIT(1) |
           ((u32)rtl8197f_rtk_napt_age_unit(timeout) << 2) |
           ((offset & 0x3f) << 8) | BIT(14) | BIT(16) |
           ((sel_ip & 0xf) << 17) | ((sel_eidx & 0x3ff) << 21);
    w[2] = (__force u16)key->int_port;
    w[2] |= (tcpflag & 0x7) << 16;
    if (tcp)
        w[2] |= BIT(19);

    return rtl8197f_rtk_force_table_raw(priv, RTL_RTK_ASICTBL_TYPE_L4_TCPUDP,
                                         index, w, ARRAY_SIZE(w));
}

static void rtl8197f_rtk_napt_clear_index(struct rtl8197f_rtknet_priv *priv,
                                          u16 index)
{
    static const u32 zero[3];

    if (index >= RTL_RTK_NAPT_TABLE_SIZE)
        return;
    rtl8197f_rtk_force_table_raw(priv, RTL_RTK_ASICTBL_TYPE_L4_TCPUDP,
                                  index, zero, ARRAY_SIZE(zero));
    __clear_bit(index, priv->napt_used);
}

static int rtl8197f_rtk_napt_output_vid(struct net_device *ndev,
                                        struct net_device *odev,
                                        int action_vid)
{
    if (odev && is_vlan_dev(odev) && vlan_dev_real_dev(odev) == ndev)
        return vlan_dev_vlan_id(odev);
    if (odev == ndev && action_vid > 0)
        return action_vid;
    return -1;
}

static int rtl8197f_rtk_napt_mangle_port(const struct flow_action_entry *act,
                                         __be16 *src, __be16 *dst)
{
    u32 val = be32_to_cpu((__force __be32)act->mangle.val);

    switch (act->mangle.offset) {
    case 0:
        if ((__force __be32)act->mangle.mask == ~cpu_to_be32(0xffff))
            *dst = cpu_to_be16(val);
        else
            *src = cpu_to_be16(val >> 16);
        break;
    case 2:
        *dst = cpu_to_be16(val);
        break;
    default:
        return -EINVAL;
    }
    return 0;
}

static int rtl8197f_rtk_napt_rule_to_key(struct rtl8197f_rtknet_priv *priv,
                                         struct flow_cls_offload *f,
                                         struct rtl8197f_rtk_napt_key *key)
{
    struct flow_rule *rule = flow_cls_offload_flow_rule(f);
    struct flow_match_control control;
    struct flow_match_basic basic;
    struct flow_match_ipv4_addrs addrs;
    struct flow_match_ports ports;
    struct flow_action_entry *act;
    struct net_device *odev = NULL;
    __be32 osrc, odst, nsrc, ndst;
    __be16 osport, odport, nsport, ndport;
    int action_vid = -1, out_vid, i, ret;
    bool src_ip_changed, dst_ip_changed;

    if (!flow_rule_match_key(rule, FLOW_DISSECTOR_KEY_CONTROL) ||
        !flow_rule_match_key(rule, FLOW_DISSECTOR_KEY_BASIC) ||
        !flow_rule_match_key(rule, FLOW_DISSECTOR_KEY_IPV4_ADDRS) ||
        !flow_rule_match_key(rule, FLOW_DISSECTOR_KEY_PORTS))
        return -EOPNOTSUPP;

    flow_rule_match_control(rule, &control);
    if (control.key->addr_type != FLOW_DISSECTOR_KEY_IPV4_ADDRS ||
        flow_rule_has_control_flags(control.mask->flags, f->common.extack))
        return -EOPNOTSUPP;

    flow_rule_match_basic(rule, &basic);
    if (basic.key->n_proto != htons(ETH_P_IP) ||
        (basic.key->ip_proto != IPPROTO_TCP &&
         basic.key->ip_proto != IPPROTO_UDP) ||
        basic.mask->n_proto != htons(0xffff) || basic.mask->ip_proto != 0xff)
        return -EOPNOTSUPP;

    flow_rule_match_ipv4_addrs(rule, &addrs);
    if (addrs.mask->src != cpu_to_be32(~0U) ||
        addrs.mask->dst != cpu_to_be32(~0U))
        return -EOPNOTSUPP;
    flow_rule_match_ports(rule, &ports);
    if (ports.mask->src != cpu_to_be16(0xffff) ||
        ports.mask->dst != cpu_to_be16(0xffff))
        return -EOPNOTSUPP;

    osrc = nsrc = addrs.key->src;
    odst = ndst = addrs.key->dst;
    osport = nsport = ports.key->src;
    odport = ndport = ports.key->dst;

    flow_action_for_each(i, act, &rule->action) {
        switch (act->id) {
        case FLOW_ACTION_MANGLE:
            switch (act->mangle.htype) {
            case FLOW_ACT_MANGLE_HDR_TYPE_IP4:
                if (act->mangle.offset == offsetof(struct iphdr, saddr))
                    memcpy(&nsrc, &act->mangle.val, sizeof(nsrc));
                else if (act->mangle.offset == offsetof(struct iphdr, daddr))
                    memcpy(&ndst, &act->mangle.val, sizeof(ndst));
                else
                    return -EOPNOTSUPP;
                break;
            case FLOW_ACT_MANGLE_HDR_TYPE_TCP:
                if (basic.key->ip_proto != IPPROTO_TCP)
                    return -EOPNOTSUPP;
                ret = rtl8197f_rtk_napt_mangle_port(act, &nsport, &ndport);
                if (ret)
                    return ret;
                break;
            case FLOW_ACT_MANGLE_HDR_TYPE_UDP:
                if (basic.key->ip_proto != IPPROTO_UDP)
                    return -EOPNOTSUPP;
                ret = rtl8197f_rtk_napt_mangle_port(act, &nsport, &ndport);
                if (ret)
                    return ret;
                break;
            case FLOW_ACT_MANGLE_HDR_TYPE_ETH:
                /* L2 rewrite remains under Linux/neighbour control.  Do not
                 * infer or install an RTL865x next-hop from an incomplete
                 * pedit sequence in this first safe backend.
                 */
                break;
            default:
                return -EOPNOTSUPP;
            }
            break;
        case FLOW_ACTION_REDIRECT:
            if (odev)
                return -EOPNOTSUPP;
            odev = act->dev;
            break;
        case FLOW_ACTION_CSUM:
            break;
        case FLOW_ACTION_VLAN_PUSH:
            if (act->vlan.proto != htons(ETH_P_8021Q) || action_vid > 0)
                return -EOPNOTSUPP;
            action_vid = act->vlan.vid;
            break;
        case FLOW_ACTION_VLAN_POP:
            break;
        default:
            return -EOPNOTSUPP;
        }
    }

    out_vid = rtl8197f_rtk_napt_output_vid(priv->ndev, odev, action_vid);
    src_ip_changed = nsrc != osrc;
    dst_ip_changed = ndst != odst;
    if (src_ip_changed == dst_ip_changed)
        return -EOPNOTSUPP; /* neither NAT nor double/hairpin NAT */

    memset(key, 0, sizeof(*key));
    key->proto = basic.key->ip_proto;
    if (src_ip_changed) {
        if (out_vid != priv->wan_vid || ndst != odst || ndport != odport)
            return -EOPNOTSUPP;
        key->int_ip = osrc;
        key->int_port = osport;
        key->ext_ip = nsrc;
        key->ext_port = nsport;
        key->rem_ip = odst;
        key->rem_port = odport;
    } else {
        if (out_vid != priv->lan_vid || nsrc != osrc || nsport != osport)
            return -EOPNOTSUPP;
        key->rem_ip = osrc;
        key->rem_port = osport;
        key->ext_ip = odst;
        key->ext_port = odport;
        key->int_ip = ndst;
        key->int_port = ndport;
    }

    if (!key->int_ip || !key->ext_ip || !key->rem_ip ||
        !key->int_port || !key->ext_port || !key->rem_port)
        return -EOPNOTSUPP;

    return 0;
}

static int rtl8197f_rtk_napt_replace(struct rtl8197f_rtknet_priv *priv,
                                     struct flow_cls_offload *f)
{
    struct rtl8197f_rtk_napt_cookie *cookie;
    struct rtl8197f_rtk_napt_hw *hw, *new_hw;
    struct rtl8197f_rtk_napt_key key;
    bool tcp;
    u16 in, out;
    u8 extidx;
    int ret;

    if (!priv->is_iball || !priv->internal_vlan_split || !priv->napt_capable ||
        !priv->hw_napt || !priv->swcore) {
        priv->napt_replace_fallback++;
        NL_SET_ERR_MSG_MOD(f->common.extack,
                           "RTL865x HW-NAPT is disabled/not supported; Linux software flowtable remains active");
        return -EOPNOTSUPP;
    }

    ret = rtl8197f_rtk_napt_rule_to_key(priv, f, &key);
    if (ret) {
        priv->napt_replace_fallback++;
        NL_SET_ERR_MSG_MOD(f->common.extack,
                           "RTL865x HW-NAPT supports exact IPv4 TCP/UDP SNAT/DNAT between iBall VID9 LAN and VID8 WAN only");
        return ret;
    }

    cookie = kzalloc(sizeof(*cookie), GFP_KERNEL);
    new_hw = kzalloc(sizeof(*new_hw), GFP_KERNEL);
    if (!cookie || !new_hw) {
        kfree(cookie);
        kfree(new_hw);
        return -ENOMEM;
    }
    cookie->cookie = f->cookie;
    new_hw->key = key;

    mutex_lock(&priv->napt_lock);
    if (rtl8197f_rtk_napt_find_cookie(priv, f->cookie)) {
        ret = -EEXIST;
        goto out_unlock;
    }

    ret = rtl8197f_rtk_napt_security_scrub(priv);
    if (ret)
        goto out_unlock;

    hw = rtl8197f_rtk_napt_find_hw(priv, &key);
    if (hw) {
        hw->refs++;
        cookie->hw = hw;
        list_add_tail(&cookie->list, &priv->napt_cookies);
        priv->napt_replace_ok++;
        mutex_unlock(&priv->napt_lock);
        kfree(new_hw);
        return 0;
    }

    tcp = key.proto == IPPROTO_TCP;
    in = rtl8197f_rtk_napt_hash(tcp, key.rem_ip, key.rem_port,
                                key.ext_ip, key.ext_port, false);
    out = rtl8197f_rtk_napt_hash(tcp, key.int_ip, key.int_port,
                                 key.rem_ip, key.rem_port, false);
    if (in == out || test_bit(in, priv->napt_used) ||
        test_bit(out, priv->napt_used)) {
        priv->napt_collision++;
        ret = -ENOSPC;
        goto out_unlock;
    }

    ret = rtl8197f_rtk_napt_extip_get(priv, key.ext_ip, &extidx);
    if (ret)
        goto out_unlock;
    ret = rtl8197f_rtk_napt_program_one(priv, &key, true, out, extidx);
    if (ret)
        goto out_extip;
    __set_bit(out, priv->napt_used);
    ret = rtl8197f_rtk_napt_program_one(priv, &key, false, in, extidx);
    if (ret)
        goto out_clear_out;
    __set_bit(in, priv->napt_used);

    new_hw->in_idx = in;
    new_hw->out_idx = out;
    new_hw->extip_idx = extidx;
    new_hw->refs = 1;
    list_add_tail(&new_hw->list, &priv->napt_hw_flows);
    cookie->hw = new_hw;
    list_add_tail(&cookie->list, &priv->napt_cookies);
    /* L3/L4 is switched on only after both directions are present.  Unknown
     * routes/NAPT entries were scrubbed immediately above, so non-offloaded
     * traffic cannot inherit OEM forwarding state and continues to the CPU.
     */
    rtl8197f_rtk_napt_set_l34(priv, true);
    priv->napt_replace_ok++;
    mutex_unlock(&priv->napt_lock);
    return 0;

out_clear_out:
    rtl8197f_rtk_napt_clear_index(priv, out);
out_extip:
    rtl8197f_rtk_napt_extip_put(priv, extidx);
out_unlock:
    priv->napt_replace_fallback++;
    mutex_unlock(&priv->napt_lock);
    kfree(cookie);
    kfree(new_hw);
    return ret;
}

static int rtl8197f_rtk_napt_destroy(struct rtl8197f_rtknet_priv *priv,
                                     struct flow_cls_offload *f)
{
    struct rtl8197f_rtk_napt_cookie *cookie;
    struct rtl8197f_rtk_napt_hw *hw;

    mutex_lock(&priv->napt_lock);
    cookie = rtl8197f_rtk_napt_find_cookie(priv, f->cookie);
    if (!cookie) {
        mutex_unlock(&priv->napt_lock);
        return 0;
    }
    hw = cookie->hw;
    list_del(&cookie->list);
    kfree(cookie);

    if (--hw->refs) {
        priv->napt_destroy_ok++;
        mutex_unlock(&priv->napt_lock);
        return 0;
    }

    rtl8197f_rtk_napt_clear_index(priv, hw->in_idx);
    rtl8197f_rtk_napt_clear_index(priv, hw->out_idx);
    rtl8197f_rtk_napt_extip_put(priv, hw->extip_idx);
    list_del(&hw->list);
    kfree(hw);
    if (list_empty(&priv->napt_hw_flows))
        rtl8197f_rtk_napt_set_l34(priv, false);
    priv->napt_destroy_ok++;
    mutex_unlock(&priv->napt_lock);
    return 0;
}

static void rtl8197f_rtk_napt_flush(struct rtl8197f_rtknet_priv *priv,
                                    bool forget_scrub)
{
    struct rtl8197f_rtk_napt_cookie *c, *ctmp;
    struct rtl8197f_rtk_napt_hw *hw, *htmp;
    static const u32 zero[3];
    u32 i;

    if (!priv->is_iball)
        return;

    mutex_lock(&priv->napt_lock);
    rtl8197f_rtk_napt_set_l34(priv, false);
    list_for_each_entry_safe(c, ctmp, &priv->napt_cookies, list) {
        list_del(&c->list);
        kfree(c);
    }
    list_for_each_entry_safe(hw, htmp, &priv->napt_hw_flows, list) {
        rtl8197f_rtk_napt_clear_index(priv, hw->in_idx);
        rtl8197f_rtk_napt_clear_index(priv, hw->out_idx);
        list_del(&hw->list);
        kfree(hw);
    }
    for (i = 0; i < RTL_RTK_EXTIP_TABLE_SIZE; i++) {
        if (priv->napt_extip_ref[i])
            rtl8197f_rtk_force_table_raw(priv, RTL_RTK_ASICTBL_TYPE_EXT_INT_IP,
                                          i, zero, ARRAY_SIZE(zero));
        priv->napt_extip_ref[i] = 0;
        priv->napt_extip[i] = 0;
    }
    bitmap_zero(priv->napt_used, RTL_RTK_NAPT_TABLE_SIZE);
    if (forget_scrub)
        priv->napt_hw_initialized = false;
    priv->napt_flushes++;
    mutex_unlock(&priv->napt_lock);
}

static struct net_device *
rtl8197f_mw5_accel_source_from_rule(struct net_device *conduit,
				    struct flow_cls_offload *cls)
{
	struct flow_rule *rule;
	struct flow_match_meta meta;
	struct net_device *source;

	if (!conduit || !cls || cls->command != FLOW_CLS_REPLACE)
		return NULL;
	rule = flow_cls_offload_flow_rule(cls);
	if (!rule || !flow_rule_match_key(rule, FLOW_DISSECTOR_KEY_META))
		return NULL;
	flow_rule_match_meta(rule, &meta);
	if (meta.mask->ingress_ifindex != -1 || meta.key->ingress_ifindex <= 0)
		return NULL;

	source = dev_get_by_index(dev_net(conduit), meta.key->ingress_ifindex);
	if (!source)
		return NULL;
	if (!rtl8197f_mw5_accel_sw_ingress_class(source)) {
		dev_put(source);
		return NULL;
	}
	return source;
}

static int rtl8197f_rtk_setup_tc_ft_cb(enum tc_setup_type type,
				       void *type_data, void *cb_priv)
{
	struct net_device *ndev = cb_priv;
	struct rtl8197f_rtknet_priv *priv = netdev_priv(ndev);
	struct flow_cls_offload *cls;

	if (type != TC_SETUP_CLSFLOWER)
		return -EOPNOTSUPP;
	cls = type_data;

	switch (cls->command) {
	case FLOW_CLS_REPLACE:
		priv->tc_cls_flower_replace++;
		if (priv->is_mw5) {
			struct net_device *source;
			int ret;

			/* Linux 6.6 DSA forwards TC_SETUP_FT to the conduit. Recover
			 * the real ingress endpoint from nf_flow_table META instead
			 * of treating eth0 as the authorization source.
			 */
			source = rtl8197f_mw5_accel_source_from_rule(ndev, cls);
			if (!source) {
				priv->accel_sw_meta_source_fail++;
				priv->accel_sw_replace_fallback++;
				return -EOPNOTSUPP;
			}
			priv->accel_sw_meta_source_ok++;
			if (READ_ONCE(priv->accel_hw)) {
				priv->accel_hw_flow_attempts++;
				if (!READ_ONCE(priv->accel_hw_l34_safe))
					priv->accel_hw_flow_rejects++;
			}
			ret = rtl8197f_mw5_accel_sw_replace(priv, cls, source);
			dev_put(source);
			return ret;
		}
		return rtl8197f_rtk_napt_replace(priv, cls);
	case FLOW_CLS_DESTROY:
		priv->tc_cls_flower_destroy++;
		if (priv->is_mw5)
			return rtl8197f_mw5_accel_sw_destroy(priv, cls, NULL);
		return rtl8197f_rtk_napt_destroy(priv, cls);
	case FLOW_CLS_STATS:
		priv->tc_cls_flower_stats++;
		/* Linux/netfilter accounting remains authoritative. Accelerator
		 * counters are diagnostic only and never keep a rule alive.
		 */
		return 0;
	default:
		return -EOPNOTSUPP;
	}
}

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
static LIST_HEAD(rtl8197f_rtk_ft_block_cb_list);

static int rtl8197f_rtk_setup_tc_block(struct net_device *ndev,
                                       struct flow_block_offload *f,
                                       bool flowtable)
{
    struct rtl8197f_rtknet_priv *priv = netdev_priv(ndev);
    int ret;

    if (flowtable) {
        priv->tc_setup_ft_calls++;
        if (f->command == FLOW_BLOCK_BIND)
            priv->tc_ft_bind++;
        else if (f->command == FLOW_BLOCK_UNBIND)
            priv->tc_ft_unbind++;
    } else {
        priv->tc_setup_block_calls++;
    }

    if (flowtable)
        ret = flow_block_cb_setup_simple(f, &rtl8197f_rtk_ft_block_cb_list,
                                         rtl8197f_rtk_setup_tc_ft_cb,
                                         ndev, ndev, true);
    else
        ret = flow_block_cb_setup_simple(f, &rtl8197f_rtk_tc_block_cb_list,
                                         rtl8197f_rtk_setup_tc_block_cb,
                                         ndev, ndev, true);
    if (ret)
        priv->tc_unsupported++;

    /* Even if userspace removes a flowtable without delivering every
     * individual destroy notification, no ASIC rule is allowed to outlive the
     * netfilter control plane that authorized it.
     */
    if (flowtable && f->command == FLOW_BLOCK_UNBIND) {
        rtl8197f_mw5_accel_sw_flush(priv);
        rtl8197f_rtk_napt_flush(priv, false);
    }

    return ret;
}

static int rtl8197f_rtk_setup_tc(struct net_device *ndev,
                                 enum tc_setup_type type, void *type_data)
{
    struct rtl8197f_rtknet_priv *priv = netdev_priv(ndev);

    switch (type) {
    case TC_SETUP_BLOCK:
        return rtl8197f_rtk_setup_tc_block(ndev, type_data, false);
    case TC_SETUP_FT:
        return rtl8197f_rtk_setup_tc_block(ndev, type_data, true);
    default:
        priv->tc_unsupported++;
        return -EOPNOTSUPP;
    }
}

static int rtl8197f_rtk_setup_tc_ft_source_cb(enum tc_setup_type type,
                                              void *type_data, void *cb_priv)
{
    struct net_device *source = cb_priv;
    struct rtl8197f_rtknet_priv *priv = READ_ONCE(rtl8197f_mw5_runtime);
    struct flow_cls_offload *cls;

    if (!priv || !priv->is_mw5 || !READ_ONCE(mw5_accel_enable) || !READ_ONCE(priv->accel_sw) ||
        type != TC_SETUP_CLSFLOWER)
        return -EOPNOTSUPP;
    cls = type_data;
    switch (cls->command) {
    case FLOW_CLS_REPLACE:
        priv->tc_cls_flower_replace++;
        if (READ_ONCE(priv->accel_hw)) {
            priv->accel_hw_flow_attempts++;
            /* The MW5 external switch uses an rtl8_4 CPU-tag path.  Until a
             * hardware self-test proves the RTL865x L3 parser sees a normal
             * EtherType at offset 12 for this exact topology, autonomous L34
             * table forwarding stays fail-closed.  The exact rule is still
             * accelerated by our software engine and final hardware DMA/DSA
             * egress. */
            if (!READ_ONCE(priv->accel_hw_l34_safe))
                priv->accel_hw_flow_rejects++;
        }
        return rtl8197f_mw5_accel_sw_replace(priv, cls, source);
    case FLOW_CLS_DESTROY:
        priv->tc_cls_flower_destroy++;
        return rtl8197f_mw5_accel_sw_destroy(priv, cls, source);
    case FLOW_CLS_STATS:
        priv->tc_cls_flower_stats++;
        return 0;
    default:
        return -EOPNOTSUPP;
    }
}

int rtl8197f_mw5_accel_setup_tc(struct net_device *source,
                                enum tc_setup_type type, void *type_data)
{
    struct rtl8197f_rtknet_priv *priv = READ_ONCE(rtl8197f_mw5_runtime);
    struct flow_block_offload *f = type_data;
    int ret;

    if (!priv || !priv->is_mw5 || !READ_ONCE(mw5_accel_enable) || !READ_ONCE(priv->accel_sw) ||
        !source || type != TC_SETUP_FT ||
        !rtl8197f_mw5_accel_sw_ingress_class(source))
        return -EOPNOTSUPP;

    priv->tc_setup_ft_calls++;
    if (f->command == FLOW_BLOCK_BIND)
        priv->tc_ft_bind++;
    else if (f->command == FLOW_BLOCK_UNBIND)
        priv->tc_ft_unbind++;

    /* cb_ident/cb_priv are the real Linux endpoint. This makes direction a
     * first-class part of the authorization, not something inferred later
     * from a tuple or interface name. */
    ret = flow_block_cb_setup_simple(f, &rtl8197f_rtk_ft_block_cb_list,
                                     rtl8197f_rtk_setup_tc_ft_source_cb,
                                     source, source, true);
    if (ret)
        priv->tc_unsupported++;
    if (f->command == FLOW_BLOCK_UNBIND)
        rtl8197f_mw5_accel_purge_dev(source);
    return ret;
}
EXPORT_SYMBOL_GPL(rtl8197f_mw5_accel_setup_tc);

static int rtl8197f_rtk_rd05_reseed_cmd(struct rtl8197f_rtknet_priv *priv,
					 const char *origin, int cmd)
{
	if (!rtl8197f_rtk_p0_external_switch_board())
		return -EOPNOTSUPP;

	if (cmd == RTL_RTK_RD05_PRIV_VENDOR_SIDE &&
	    of_machine_is_compatible("tenda,nova-mw5"))
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
		/* A running MW5 must never have CPURPDCR0/CPUTPDCR0 rewound by a
		 * table/MAC reseed.  v43.7 hardware logs showed that doing so turns
		 * valid 64-byte frames into repeated len=2046 garbage and eventually
		 * triggers a TX watchdog.  Keep explicit rxstart as the only runtime
		 * command allowed to reprogram RX geometry.
		 */
		if (!of_machine_is_compatible("tenda,nova-mw5")) {
			if (priv->rd05_sdk_newdesc_rx)
				rtl8197f_rtk_rd05_program_newdesc_rx(priv, origin);
			else
				rtl8197f_rtk_rd05_program_legacy_rx(priv, origin);
		}
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

	if (!of_machine_is_compatible("tenda,nova-mw5") ||
	    cmd == RTL_RTK_RD05_PRIV_RXSTART) {
		rtl8197f_rtk_apply_sdk_cpuif_init(priv);
		rtl8197f_rtk_rd05_poststart_rearm(priv, origin);
	}
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

static int rtl8197f_rtk_rd05_proc_show(struct seq_file *m, void *v)
{
	struct net_device *ndev = m->private;
	struct rtl8197f_rtknet_priv *priv;

	if (!ndev)
		return -ENODEV;

	priv = netdev_priv(ndev);
	seq_printf(m, "driver=%s version=%s board=%s\n",
		   DRV_NAME, DRV_VERSION, rtl8197f_rtk_p0_board_name());
	seq_printf(m, "netdev=%s running=%u carrier=%u\n",
		   ndev->name, netif_running(ndev), netif_carrier_ok(ndev));
	seq_printf(m, "mac=%pM seeded_mac=%pM\n",
		   ndev->dev_addr, priv->rd05_seeded_mac);
	seq_printf(m, "descriptor_policy dwords=%u stride=%u kseg1=%u sdk_crc=%u txringcr=0x%08x\n",
		   priv->desc_dwords, priv->desc_stride,
		   priv->desc_addr_kseg1, priv->sdk_crc_lengths,
		   rtl8197f_rtk_read(priv, RTL_RTK_TXRINGCR));
	seq_printf(m, "mw5_accel enabled=%u sw=%u hw=%u hw_engine=coherent-dma+dsa-wlan-egress asic_l34_safe=%u rings=%u/%u coherent_rx=%u coherent_tx=%u page_pool=%u hw_packets=%llu hw_fallback=%llu hw_flow_attempts=%llu hw_flow_rejects=%llu\n",
		   priv->is_mw5 && READ_ONCE(mw5_accel_enable),
		   priv->accel_sw, priv->accel_hw, priv->accel_hw_l34_safe,
		   priv->rx_ring_size, priv->tx_ring_size,
		   priv->rx_coherent_enabled, priv->tx_coherent_enabled,
		   priv->rx_page_pool_enabled,
		   (unsigned long long)priv->accel_hw_packets,
		   (unsigned long long)priv->accel_hw_fallback,
		   (unsigned long long)priv->accel_hw_flow_attempts,
		   (unsigned long long)priv->accel_hw_flow_rejects);
	seq_printf(m, "mw5_accel_sw replace_ok=%llu fallback=%llu destroy=%llu flushes=%llu hits=%llu misses=%llu guard_reject=%llu xmit_ok=%llu xmit_fail=%llu active=%u idle_bypass=%llu meta_source_ok=%llu meta_source_fail=%llu hash_lookups=%llu hash_steps=%llu hash_max_steps=%llu\n",
		   (unsigned long long)priv->accel_sw_replace_ok,
		   (unsigned long long)priv->accel_sw_replace_fallback,
		   (unsigned long long)priv->accel_sw_destroy_ok,
		   (unsigned long long)priv->accel_sw_flushes,
		   (unsigned long long)priv->accel_sw_hits,
		   (unsigned long long)priv->accel_sw_misses,
		   (unsigned long long)priv->accel_sw_guard_reject,
		   (unsigned long long)priv->accel_sw_xmit_ok,
		   (unsigned long long)priv->accel_sw_xmit_fail,
		   READ_ONCE(priv->accel_sw_active),
		   (unsigned long long)priv->accel_sw_idle_bypass,
		   (unsigned long long)priv->accel_sw_meta_source_ok,
		   (unsigned long long)priv->accel_sw_meta_source_fail,
		   (unsigned long long)priv->accel_sw_hash_lookups,
		   (unsigned long long)priv->accel_sw_hash_steps,
		   (unsigned long long)priv->accel_sw_hash_max_steps);
	seq_printf(m, "tc_flow setup_ft=%llu bind=%llu unbind=%llu replace=%llu destroy=%llu stats=%llu unsupported=%llu\n",
		   (unsigned long long)priv->tc_setup_ft_calls,
		   (unsigned long long)priv->tc_ft_bind,
		   (unsigned long long)priv->tc_ft_unbind,
		   (unsigned long long)priv->tc_cls_flower_replace,
		   (unsigned long long)priv->tc_cls_flower_destroy,
		   (unsigned long long)priv->tc_cls_flower_stats,
		   (unsigned long long)priv->tc_unsupported);
	seq_printf(m, "mw5_legacy_accel hwlookup=%u napt=%u extport=%u txcsum=%u rxcsum=%u sg=%u tso=%u qos=%u retired=1\n",
		   priv->tx_hwlookup, priv->hw_napt,
		   READ_ONCE(mw5_extport_wlan_enable),
		   !!(ndev->features & (NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM)),
		   !!(ndev->features & NETIF_F_RXCSUM),
		   !!(ndev->features & NETIF_F_SG),
		   !!(ndev->features & (NETIF_F_TSO | NETIF_F_TSO6)),
		   priv->hw_qos);
	if (priv->is_mw5) {
		seq_printf(m,
			   "mw5_oem_geometry tx_ring0_observed=%u rx_ring0_observed=%u tx_aux_rings=%u tx_aux_each=%u rx_aux_rings=%u rx_aux_each=%u aux_enabled=0 freeSkbThreshold_observed=%u eth_skb_free_num_snapshot=%u\n",
			   RTL_RTK_MW5_OEM_TX_RING0, RTL_RTK_MW5_OEM_RX_RING0,
			   RTL_RTK_MW5_OEM_TX_AUX_RINGS, RTL_RTK_MW5_OEM_AUX_RING_SIZE,
			   RTL_RTK_MW5_OEM_RX_AUX_RINGS, RTL_RTK_MW5_OEM_AUX_RING_SIZE,
			   RTL_RTK_MW5_OEM_FREE_SKB_THRESHOLD,
			   RTL_RTK_MW5_OEM_FREE_SKB_SNAPSHOT);
		seq_printf(m,
			   "mw5_oem_vlan lan_vid=%u lan_port=%u lan_mask=0x%02x lan_fid=0 wan_vid=%u wan_port=%u wan_mask=0x%02x wan_fid=1 pvid_p1=9 pvid_p3=8 pvid_p8=9\n",
			   priv->lan_vid, RTL_RTK_MW5_LAN_PORT, priv->lan_port_mask,
			   priv->wan_vid, RTL_RTK_MW5_WAN_PORT, priv->wan_port_mask);
		seq_printf(m,
			   "mw5_dsa_host_vlan version=v44.66.12 p0_mode=%s p0_router_mode=%u tx_dvid_lan=%u tx_dvid_wan=%u physical_dp=p0 vlan9_p0_untag=1 vlan8_p0_untag=1 goal=8899-immediately-after-sa\n",
			   READ_ONCE(priv->mw5_p0_transparent) ? "transparent" : "router-vlan-diag",
			   !!(priv->swcore && (readl(priv->swcore + RTL_RTK_SWCORE_MACCR1) & RTL_RTK_MACCR1_P0_ROUTER_MODE)),
			   READ_ONCE(priv->mw5_p0_transparent) ? 0 : RTL_RTK_MW5_LAN_VID,
			   READ_ONCE(priv->mw5_p0_transparent) ? 0 : RTL_RTK_MW5_WAN_VID);
		seq_puts(m,
			 "mw5_oem_flowctrl source=rtl8197f-switchcore external_rtl8367_programming=0 S_DSC_RUNOUT=500 SDC_FCOFF=410 SDC_FCON=428 S_MaxSBuf_FCOFF=308 S_MaxSBuf_FCON=320 IQ_DSC_FCON=150 IQ_DSC_FCOFF=200 QLEN_GAP=24\n");
	}
	seq_printf(m, "napt replace_ok=%llu fallback=%llu destroy=%llu collision=%llu extip_alloc=%llu extip_free=%llu flushes=%llu active_hw=%u active_cookie=%u\n",
		   (unsigned long long)priv->napt_replace_ok,
		   (unsigned long long)priv->napt_replace_fallback,
		   (unsigned long long)priv->napt_destroy_ok,
		   (unsigned long long)priv->napt_collision,
		   (unsigned long long)priv->napt_extip_alloc,
		   (unsigned long long)priv->napt_extip_free,
		   (unsigned long long)priv->napt_flushes,
		   !list_empty(&priv->napt_hw_flows),
		   !list_empty(&priv->napt_cookies));
	seq_printf(m, "hwaccel_rx pp_alloc=%llu pp_recycle=%llu pp_fallback=%llu csum_ok=%llu csum_none=%llu\n",
		   (unsigned long long)priv->rx_page_pool_alloc,
		   (unsigned long long)priv->rx_page_pool_recycle,
		   (unsigned long long)priv->rx_page_pool_fallback,
		   (unsigned long long)priv->rx_csum_ok,
		   (unsigned long long)priv->rx_csum_none);
	seq_printf(m, "hwaccel_tx csum_hw=%llu csum_sw=%llu sg_packets=%llu sg_descs=%llu sg_guard_linearize=%llu tso4=%llu tso6=%llu qos=%llu hwlookup_dsa_bypass=%llu\n",
		   (unsigned long long)priv->tx_csum_hw,
		   (unsigned long long)priv->tx_csum_sw_fallback,
		   (unsigned long long)priv->tx_sg_packets,
		   (unsigned long long)priv->tx_sg_descs,
		   (unsigned long long)priv->tx_sg_guard_linearize,
		   (unsigned long long)priv->tx_tso4,
		   (unsigned long long)priv->tx_tso6,
		   (unsigned long long)priv->tx_qos_packets,
		   (unsigned long long)priv->tx_hwlookup_dsa_bypass);
	seq_printf(m, "mac_changes=%llu table_seeds=%llu\n",
		   (unsigned long long)priv->rd05_mac_changes,
		   (unsigned long long)priv->rd05_mac_reseeds);
	seq_printf(m, "rx packets=%lu bytes=%lu errors=%lu dropped=%lu\n",
		   ndev->stats.rx_packets, ndev->stats.rx_bytes,
		   ndev->stats.rx_errors, ndev->stats.rx_dropped);
	seq_printf(m, "rx_internal bad_desc=%llu dma_errors=%llu crc_strips=%llu coherent_copies=%llu coherent_alloc_fail=%llu desc_unstable=%llu desc_stable_retry=%llu\n",
		   (unsigned long long)priv->rx_bad_desc,
		   (unsigned long long)priv->rx_dma_errors,
		   (unsigned long long)priv->rx_crc_strips,
		   (unsigned long long)priv->rx_coherent_copies,
		   (unsigned long long)priv->rx_coherent_alloc_fail,
		   (unsigned long long)priv->rx_desc_unstable,
		   (unsigned long long)priv->rx_desc_stable_retry);
	seq_printf(m, "mw5_rx_recovery runout_acks=%llu recoveries=%llu fast_timer=%llu fast_schedule=%llu trxrdy=%llu guard_active=%u publish_wait=%u publish_waits=%llu publish_ready=%llu publish_cdp_invalid=%llu publish_timer=%llu seq_valid=%u hw_seq=%llu sw_seq=%llu seq_adv=%llu seq_wait=%llu catchup_pending=%u corrupt_clusters=%llu catchups=%llu catchup_desc=%llu invalid_streak=%u invalid_streak_max=%llu\n",
		   (unsigned long long)priv->rx_runout_acks,
		   (unsigned long long)priv->rx_runout_recoveries,
		   (unsigned long long)priv->rx_runout_fast_timer_runs,
		   (unsigned long long)priv->rx_runout_fast_schedules,
		   (unsigned long long)priv->rx_runout_trxrdy_writes,
		   rtl8197f_rtk_mw5_runout_guard_active(priv),
		   READ_ONCE(priv->rx_publish_wait_pending),
		   (unsigned long long)priv->rx_publish_waits,
		   (unsigned long long)priv->rx_publish_ready,
		   (unsigned long long)priv->rx_publish_cdp_invalid,
		   (unsigned long long)priv->rx_publish_timer_runs,
		   priv->mw5_rx_seq_valid,
		   (unsigned long long)priv->mw5_rx_hw_seq,
		   (unsigned long long)priv->mw5_rx_sw_seq,
		   (unsigned long long)priv->mw5_rx_seq_advances,
		   (unsigned long long)priv->mw5_rx_seq_waits,
		   READ_ONCE(priv->mw5_rx_catchup_pending),
		   (unsigned long long)priv->mw5_rx_corrupt_clusters,
		   (unsigned long long)priv->mw5_rx_catchups,
		   (unsigned long long)priv->mw5_rx_catchup_descs,
		   priv->mw5_rx_invalid_streak,
		   (unsigned long long)priv->mw5_rx_invalid_streak_max);
	seq_printf(m, "rxtrace frames=%llu logs=%llu external=%llu self_loop=%llu\n",
		   (unsigned long long)priv->rd05_rxtrace_frames,
		   (unsigned long long)priv->rd05_rxtrace_logs,
		   (unsigned long long)priv->rd05_rx_external,
		   (unsigned long long)priv->rd05_rx_self_loop);
	seq_printf(m, "rxtrace_type rtk0=%llu rtk4=%llu arp0=%llu arp4=%llu arp8=%llu\n",
		   (unsigned long long)priv->rd05_rxtrace_rtk0,
		   (unsigned long long)priv->rd05_rxtrace_rtk4,
		   (unsigned long long)priv->rd05_rxtrace_arp0,
		   (unsigned long long)priv->rd05_rxtrace_arp4,
		   (unsigned long long)priv->rd05_rxtrace_arp8);
	seq_printf(m, "rxproto ipv4=%llu ipv6=%llu other=%llu\n",
		   (unsigned long long)priv->rd05_rx_ipv4,
		   (unsigned long long)priv->rd05_rx_ipv6,
		   (unsigned long long)priv->rd05_rx_other);
	seq_printf(m, "rxtrace_last idx=%llu len=%llu\n",
		   (unsigned long long)priv->rd05_rxtrace_last_idx,
		   (unsigned long long)priv->rd05_rxtrace_last_len);
	seq_printf(m, "rxtrace_opts %08llx/%08llx/%08llx/%08llx/%08llx\n",
		   (unsigned long long)priv->rd05_rxtrace_last_opts1,
		   (unsigned long long)priv->rd05_rxtrace_last_opts2,
		   (unsigned long long)priv->rd05_rxtrace_last_opts3,
		   (unsigned long long)priv->rd05_rxtrace_last_opts4,
		   (unsigned long long)priv->rd05_rxtrace_last_opts5);
	seq_printf(m, "rxtrace_ports extspa=%llu spa=%llu dp_ext=%llu\n",
		   (unsigned long long)priv->rd05_rxdesc_last_ext3,
		   (unsigned long long)priv->rd05_rxdesc_last_spa5,
		   (unsigned long long)priv->rd05_rxdesc_last_src24);
	seq_printf(m, "mw5_rx_dsa passthrough=%llu synthesized=%llu repaired_inplace=%llu fallback_wan_da=%llu fallback_lan_da=%llu unclassified=%llu expand_fail=%llu invalid_port=%llu invalid_drop=%llu drop_mode=%u oversize_drop=%llu own_wait=%llu cdp_complete=%llu cdp_owned_advanced=%llu cdp_owned_wait=%llu cdp_owned_recovered=%llu runout_complete=%llu empty_wait=%llu cdp_fallback=%llu addr_mismatch=%llu desc_trace=%llu bad_snapshot=%llu\n",
		   (unsigned long long)priv->mw5_rx_tag_passthrough,
		   (unsigned long long)priv->mw5_rx_tag_synthesized,
		   (unsigned long long)priv->mw5_rx_tag_repaired_inplace,
		   (unsigned long long)priv->mw5_rx_tag_fallback_wan_da,
		   (unsigned long long)priv->mw5_rx_tag_fallback_lan_da,
		   (unsigned long long)priv->mw5_rx_tag_unclassified,
		   (unsigned long long)priv->mw5_rx_tag_expand_fail,
		   (unsigned long long)priv->mw5_rx_tag_invalid_port,
		   (unsigned long long)priv->mw5_rx_invalid_drop,
		   READ_ONCE(mw5_drop_invalid_untagged),
		   (unsigned long long)priv->mw5_rx_oversize_drop,
		   (unsigned long long)priv->mw5_rx_own_wait,
		   (unsigned long long)priv->mw5_rx_cdp_complete,
		   (unsigned long long)priv->rx_cdp_owned_advanced,
		   (unsigned long long)priv->rx_cdp_owned_wait,
		   (unsigned long long)priv->rx_cdp_owned_recovered,
		   (unsigned long long)priv->mw5_rx_runout_complete,
		   (unsigned long long)priv->mw5_rx_empty_wait,
		   (unsigned long long)priv->mw5_rx_cdp_fallback,
		   (unsigned long long)priv->mw5_rx_addr_mismatch,
		   (unsigned long long)priv->mw5_rx_desc_trace,
		   (unsigned long long)priv->mw5_rx_bad_snapshot);
	seq_printf(m, "mw5_rx_invalid_spa p0=%llu p1=%llu p2=%llu p3=%llu p4=%llu p5=%llu p6=%llu p7=%llu\n",
		   (unsigned long long)priv->mw5_rx_invalid_spa[0],
		   (unsigned long long)priv->mw5_rx_invalid_spa[1],
		   (unsigned long long)priv->mw5_rx_invalid_spa[2],
		   (unsigned long long)priv->mw5_rx_invalid_spa[3],
		   (unsigned long long)priv->mw5_rx_invalid_spa[4],
		   (unsigned long long)priv->mw5_rx_invalid_spa[5],
		   (unsigned long long)priv->mw5_rx_invalid_spa[6],
		   (unsigned long long)priv->mw5_rx_invalid_spa[7]);
	seq_printf(m, "mw5_rx_unclassified_detail known_proto0=%llu known_proto4=%llu sa_lan=%llu sa_wan=%llu\n",
		   (unsigned long long)priv->mw5_rx_unclassified_known_proto,
		   (unsigned long long)priv->mw5_rx_unclassified_proto4_known,
		   (unsigned long long)priv->mw5_rx_unclassified_sa_lan,
		   (unsigned long long)priv->mw5_rx_unclassified_sa_wan);
	seq_printf(m, "mw5_rx_invalid_proto ipv4=%llu ipv6=%llu arp=%llu vlan=%llu realtek=%llu other=%llu bcast=%llu mcast=%llu ucast=%llu\n",
		   (unsigned long long)priv->mw5_rx_invalid_ipv4,
		   (unsigned long long)priv->mw5_rx_invalid_ipv6,
		   (unsigned long long)priv->mw5_rx_invalid_arp,
		   (unsigned long long)priv->mw5_rx_invalid_vlan,
		   (unsigned long long)priv->mw5_rx_invalid_realtek,
		   (unsigned long long)priv->mw5_rx_invalid_other,
		   (unsigned long long)priv->mw5_rx_invalid_bcast,
		   (unsigned long long)priv->mw5_rx_invalid_mcast,
		   (unsigned long long)priv->mw5_rx_invalid_ucast);
	if (priv->is_mw5 && priv->rx_desc && priv->rx_ring_size) {
		u32 cdp = rtl8197f_rtk_read(priv, RTL_RTK_CPURPDCR0);
		u32 hw_idx = 0;
		u32 sw_idx = READ_ONCE(priv->rx_tail);
		bool cdp_ok = rtl8197f_rtk_rx_cdp_index_value(priv, cdp, &hw_idx);
		struct rtl8197f_rtk_desc *d = rtl8197f_rtk_rx_desc(priv, sw_idx);
		u32 d0 = le32_to_cpu(READ_ONCE(d->opts1));

		seq_printf(m,
			   "mw5_rx_oem_state ring0=%u rxSwIdx=%u HW_CDP=0x%08x hw_idx=%u cdp_valid=%u sw_own=%u desc_dwords=%u desc_bytes=%u coherent=%u\n",
			   priv->rx_ring_size, sw_idx, cdp, hw_idx, cdp_ok,
			   !!(d0 & RTL_RTK_DESC_OWN), priv->desc_dwords,
			   priv->desc_stride, priv->rx_coherent_enabled);
	}
	seq_printf(m, "mw5_tx_dma submit=%llu complete=%llu own_after_kick=%llu cdp_seen=%llu cdp_clean=%llu cdp_owned_seen=%llu cdp_invalid=%llu cdp_invalid_defer=%llu own_fallback_clean=%llu own_defer=%llu own_completed=%llu completion_mismatch=%llu defer_streak=%u defer_max=%llu\n",
		   (unsigned long long)priv->mw5_tx_submit,
		   (unsigned long long)priv->mw5_tx_complete,
		   (unsigned long long)priv->mw5_tx_own_after_kick,
		   (unsigned long long)priv->mw5_tx_cdp_seen,
		   (unsigned long long)priv->mw5_tx_cdp_clean,
		   (unsigned long long)priv->mw5_tx_cdp_owned_clean,
		   (unsigned long long)priv->mw5_tx_cdp_invalid,
		   (unsigned long long)priv->mw5_tx_cdp_invalid_defer,
		   (unsigned long long)priv->mw5_tx_own_fallback_clean,
		   (unsigned long long)priv->mw5_tx_own_defer,
		   (unsigned long long)priv->mw5_tx_own_completed,
		   (unsigned long long)priv->mw5_tx_completion_mismatch,
		   priv->mw5_tx_own_defer_streak,
		   (unsigned long long)priv->mw5_tx_own_defer_max);
	if (priv->is_mw5 && priv->tx_desc && priv->tx_ring_size) {
		u32 cdp = rtl8197f_rtk_read(priv, RTL_RTK_CPUTPDCR0);
		u32 hw_idx = 0;
		u32 curr = READ_ONCE(priv->tx_head);
		u32 done_idx = READ_ONCE(priv->tx_tail);
		u32 pending = (curr + priv->tx_ring_size - done_idx) % priv->tx_ring_size;
		bool cdp_ok = rtl8197f_rtk_tx_cdp_index_value(priv, cdp, &hw_idx);
		struct rtl8197f_rtk_desc *d = rtl8197f_rtk_tx_desc(priv, done_idx);
		u32 d0 = le32_to_cpu(READ_ONCE(d->opts1));
		u32 d2 = le32_to_cpu(READ_ONCE(d->opts2));
		u32 d3 = le32_to_cpu(READ_ONCE(d->opts3));
		u32 d4 = le32_to_cpu(READ_ONCE(d->opts4));
		u32 d5 = le32_to_cpu(READ_ONCE(d->opts5));

		seq_printf(m,
			   "mw5_tx_oem_state ring0=%u txCurrIdx=%u txDoneIdx=%u HW_CDP=0x%08x hw_idx=%u cdp_valid=%u tail_own=%u pending=%u free=%u desc_dwords=%u desc_bytes=%u\n",
			   priv->tx_ring_size, curr, done_idx, cdp, hw_idx, cdp_ok,
			   !!(d0 & RTL_RTK_DESC_OWN), pending,
			   (priv->tx_ring_size - 1) - pending,
			   priv->desc_dwords, priv->desc_stride);
		seq_printf(m,
			   "mw5_tx_coherent enabled=%u stride=%u slots=%u copies=%llu bytes=%llu alloc_fail=%llu oversize=%llu reuse_guard_own=%llu reuse_guard_swbusy=%llu rtl8_lan=%llu rtl8_wan=%llu rtl8_other=%llu normal_dp=p0 meta_diag_lan_p1_vid9=%llu meta_diag_wan_p3_vid8=%llu meta_invalid=%llu\n",
			   priv->tx_coherent_enabled, priv->tx_coherent_stride,
			   priv->tx_ring_size,
			   (unsigned long long)priv->tx_coherent_copies,
			   (unsigned long long)priv->tx_coherent_bytes,
			   (unsigned long long)priv->tx_coherent_alloc_fail,
			   (unsigned long long)priv->tx_coherent_oversize,
			   (unsigned long long)priv->mw5_tx_reuse_guard_own,
			   (unsigned long long)priv->mw5_tx_reuse_guard_swbusy,
			   (unsigned long long)priv->mw5_tx_wiretag_lan,
			   (unsigned long long)priv->mw5_tx_wiretag_wan,
			   (unsigned long long)priv->mw5_tx_wiretag_other,
			   (unsigned long long)priv->mw5_tx_meta_lan,
			   (unsigned long long)priv->mw5_tx_meta_wan,
			   (unsigned long long)priv->mw5_tx_meta_invalid);
		seq_printf(m,
			   "mw5_tx_desc_done DW0=%08x DW1=%08x DW2=%08x DW3=%08x DW4=%08x DW5=%08x\n",
			   d0, le32_to_cpu(READ_ONCE(d->addr)),
			   d2, d3, d4, d5);
		seq_printf(m,
			   "mw5_tx_desc_decode own=%u eor=%u ls=%u fs=%u hwlookup=%u bridge=%u type=%u ph_len=%u m_len=%u qid=%u pqid=%u dvid=%u dp_ext=%u dpri=%u l3cs=%u l4cs=%u ipv4=%u ipv6=%u dp=0x%02x lso=%u extspa=%u mss=%u\n",
			   !!(d0 & RTL_RTK_DESC_OWN), !!(d0 & RTL_RTK_DESC_WRAP),
			   !!(d0 & RTL_RTK_DESC_LAST), !!(d0 & RTL_RTK_DESC_FIRST),
			   !!(d0 & RTL_RTK_TX_HWLKUP), !!(d0 & RTL_RTK_TX_BRIDGE),
			   (unsigned int)((d0 & RTL_RTK_TX_TYPE_MASK) >> RTL_RTK_TX_TYPE_SHIFT),
			   (unsigned int)((d0 & RTL_RTK_TX_PH_LEN_MASK) >> RTL_RTK_TX_PH_LEN_SHIFT),
			   (unsigned int)((d2 & RTL_RTK_TX_M_LEN_MASK) >> RTL_RTK_TX_M_LEN_SHIFT),
			   (unsigned int)((d2 >> RTL_RTK_TX_QID_SHIFT) & 0x7),
			   (unsigned int)((d2 >> RTL_RTK_TX_PQID_SHIFT) & 0x7),
			   (unsigned int)(d3 & RTL_RTK_TX_DVID_MASK),
			   (unsigned int)((d3 >> RTL_RTK_TX_DP_EXT_SHIFT) & RTL_RTK_TX_DP_EXT_MASK),
			   (unsigned int)((d3 >> RTL_RTK_TX_DPRI_SHIFT) & 0x7),
			   !!(d3 & RTL_RTK_TX_L3CS), !!(d3 & RTL_RTK_TX_L4CS),
			   !!(d3 & RTL_RTK_TX_IPV4), !!(d3 & RTL_RTK_TX_IPV6),
			   (unsigned int)((d4 >> RTL_RTK_TX_DP_SHIFT) & RTL_RTK_TX_DP_MASK),
			   !!(d4 & RTL_RTK_TX_LSO),
			   (unsigned int)((d5 >> RTL_RTK_TX_EXTSPA_SHIFT) & RTL_RTK_TX_EXTSPA_MASK),
			   (unsigned int)((d5 >> RTL_RTK_TX_MSS_SHIFT) & 0x3fff));
	}
	seq_printf(m, "tx packets=%lu bytes=%lu errors=%lu dropped=%lu\n",
		   ndev->stats.tx_packets, ndev->stats.tx_bytes,
		   ndev->stats.tx_errors, ndev->stats.tx_dropped);
	seq_printf(m, "tx_internal kicks=%llu deferred=%llu clean_empty=%llu ring_full=%llu busy_desc=%llu timeouts=%llu\n",
		   (unsigned long long)priv->tx_kicks,
		   (unsigned long long)priv->tx_kick_deferred,
		   (unsigned long long)priv->tx_clean_empty,
		   (unsigned long long)priv->tx_ring_full,
		   (unsigned long long)priv->tx_busy_desc,
		   (unsigned long long)priv->tx_timeouts);
	seq_printf(m, "tx_bql sent_bytes=%llu completed_pkts=%llu completed_bytes=%llu\n",
		   (unsigned long long)priv->tx_bql_sent_bytes,
		   (unsigned long long)priv->tx_bql_completed_pkts,
		   (unsigned long long)priv->tx_bql_completed_bytes);
	seq_printf(m, "irq rx_done=%llu tx_done=%llu rx_err=%llu tx_err=%llu rx_runout=%llu mbuf_runout=%llu napi=%llu\n",
		   (unsigned long long)priv->irq_rx_done,
		   (unsigned long long)priv->irq_tx_done,
		   (unsigned long long)priv->irq_rx_errors,
		   (unsigned long long)priv->irq_tx_errors,
		   (unsigned long long)priv->irq_rx_runout,
		   (unsigned long long)priv->irq_mbuf_runout,
		   (unsigned long long)priv->napi_polls);
	seq_printf(m, "napi_detail rx_work=%llu tx_clean=%llu zero_rx=%llu budget=%llu rearm_race=%llu gro_flush_ns=%lu defer_hard_irqs=%u\n",
		   (unsigned long long)priv->napi_rx_work,
		   (unsigned long long)priv->napi_tx_clean,
		   (unsigned long long)priv->napi_zero_rx_polls,
		   (unsigned long long)priv->napi_budget_polls,
		   (unsigned long long)priv->napi_rearm_race,
		   ndev->gro_flush_timeout, ndev->napi_defer_hard_irqs);
	seq_printf(m, "rx_poll base_ms=%u active_watchdog_ms=%u runout_guard_ms=%u runs=%llu irq_progress=%llu fallback=%llu restarts=%llu\n",
		   priv->rx_poll_ms,
		   rtl8197f_rtk_mw5_runout_guard_active(priv) ?
		   RTL_RTK_MW5_RX_RUNOUT_RECHECK_MS :
		   (priv->irq_rx_done ? max_t(u32, priv->rx_poll_ms,
				RTL_RTK_MW5_RX_WATCHDOG_MS) : priv->rx_poll_ms),
		   RTL_RTK_MW5_RX_RUNOUT_GUARD_MS,
		   (unsigned long long)priv->rx_poll_timer_runs,
		   (unsigned long long)priv->rx_poll_irq_progress_skips,
		   (unsigned long long)priv->rx_poll_fallback_schedules,
		   (unsigned long long)priv->hw_rx_restarts);
	seq_printf(m, "registers cpuicr=%08x cpuicr1=%08x dma_cr0=%08x\n",
		   rtl8197f_rtk_read(priv, RTL_RTK_CPUICR),
		   rtl8197f_rtk_read(priv, RTL_RTK_CPUICR1),
		   rtl8197f_rtk_read(priv, RTL_RTK_DMA_CR0));
	seq_printf(m, "rings dma_cr1=%08x txringcr=%08x rxbase=%08x txbase=%08x\n",
		   rtl8197f_rtk_read(priv, RTL_RTK_DMA_CR1),
		   rtl8197f_rtk_read(priv, RTL_RTK_TXRINGCR),
		   rtl8197f_rtk_read(priv, RTL_RTK_CPURPDCR0),
		   rtl8197f_rtk_read(priv, RTL_RTK_CPUTPDCR0));

	return 0;
}

static int rtl8197f_rtk_rd05_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, rtl8197f_rtk_rd05_proc_show,
			   pde_data(inode));
}

static int rtl8197f_rtk_set_feature(struct net_device *ndev,
					      netdev_features_t feature,
					      bool enable)
{
	struct rtl8197f_rtknet_priv *priv = netdev_priv(ndev);
	bool active;

	/* v44.65.17: classic per-feature Realtek acceleration is retired on MW5.
	 * The unified accelerator deliberately owns all MW5 fast-path policy, so
	 * ethtool/proc cannot silently re-enable TXCSUM/SG/TSO/RXCSUM behind it.
	 * iBall keeps the generic opt-in path. */
	if (priv->is_mw5)
		return -EOPNOTSUPP;
	if (!priv->is_iball || (ndev->hw_features & feature) != feature)
		return -EOPNOTSUPP;

	rtnl_lock();
	if (enable)
		ndev->wanted_features |= feature;
	else
		ndev->wanted_features &= ~feature;
	netdev_update_features(ndev);
	active = (ndev->features & feature) == feature;
	rtnl_unlock();

	if (feature & NETIF_F_RXCSUM) {
		if (priv->is_mw5)
			WRITE_ONCE(mw5_hw_rx_csum, active);
		priv->hw_rx_csum = active;
	}
	if (feature & (NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM)) {
		if (priv->is_mw5)
			WRITE_ONCE(mw5_hw_csum, active);
		priv->hw_csum = active;
	} else if (feature == NETIF_F_SG) {
		if (priv->is_mw5)
			WRITE_ONCE(mw5_hw_sg, active);
		priv->hw_sg = active;
	} else if (feature == (NETIF_F_TSO | NETIF_F_TSO6)) {
		if (priv->is_mw5)
			WRITE_ONCE(mw5_hw_tso, active);
		priv->hw_tso = active;
	}

	return active == enable ? 0 : -EIO;
}

static void rtl8197f_rtk_mw5_set_coalesce(struct net_device *ndev, bool enable)
{
	if (enable) {
		/* Diagnostic-only conservative mode: one deferred repoll, not four. */
		ndev->gro_flush_timeout = 200000UL;
		ndev->napi_defer_hard_irqs = 1;
	} else {
		ndev->gro_flush_timeout = 0;
		ndev->napi_defer_hard_irqs = 0;
	}
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

	priv = netdev_priv(ndev);
	if (!strcmp(buf, "rxcsum-on")) {
		ret = rtl8197f_rtk_set_feature(ndev, NETIF_F_RXCSUM, true);
		return ret ? ret : len;
	}
	if (!strcmp(buf, "rxcsum-off")) {
		ret = rtl8197f_rtk_set_feature(ndev, NETIF_F_RXCSUM, false);
		return ret ? ret : len;
	}
	if (!strcmp(buf, "txcsum-on")) {
		ret = rtl8197f_rtk_set_feature(ndev, NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM, true);
		return ret ? ret : len;
	}
	if (!strcmp(buf, "txcsum-off")) {
		ret = rtl8197f_rtk_set_feature(ndev, NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM, false);
		return ret ? ret : len;
	}
	if (!strcmp(buf, "sg-on")) {
		ret = rtl8197f_rtk_set_feature(ndev, NETIF_F_SG, true);
		return ret ? ret : len;
	}
	if (!strcmp(buf, "sg-off")) {
		ret = rtl8197f_rtk_set_feature(ndev, NETIF_F_SG, false);
		return ret ? ret : len;
	}
	if (!strcmp(buf, "tso-on")) {
		if ((!priv->is_iball && !priv->is_mw5) || !priv->hw_csum || !priv->hw_sg)
			return -EOPNOTSUPP;
		ret = rtl8197f_rtk_set_feature(ndev, NETIF_F_TSO | NETIF_F_TSO6, true);
		return ret ? ret : len;
	}
	if (!strcmp(buf, "tso-off")) {
		ret = rtl8197f_rtk_set_feature(ndev, NETIF_F_TSO | NETIF_F_TSO6, false);
		return ret ? ret : len;
	}
	if (!strcmp(buf, "p0-transparent")) {
		if (!priv->is_mw5)
			return -EOPNOTSUPP;
		WRITE_ONCE(priv->mw5_p0_transparent, true);
		rtl8197f_rtk_init_p0_rgmii(priv, "proc-p0-transparent");
		return len;
	}
	if (!strcmp(buf, "p0-router-vlan")) {
		if (!priv->is_mw5)
			return -EOPNOTSUPP;
		WRITE_ONCE(priv->mw5_p0_transparent, false);
		rtl8197f_rtk_init_p0_rgmii(priv, "proc-p0-router-vlan");
		return len;
	}
	if (!strcmp(buf, "accel-on")) {
		if (!priv->is_mw5)
			return -EOPNOTSUPP;
		WRITE_ONCE(mw5_accel_enable, true);
		WRITE_ONCE(mw5_accel_sw_enable, true);
		WRITE_ONCE(mw5_accel_hw_enable, true);
		priv->accel_sw = true;
		priv->accel_hw = true;
		priv->hw_napt = false;
		priv->tx_hwlookup = false;
		priv->hw_csum = false;
		priv->hw_rx_csum = false;
		priv->hw_sg = false;
		priv->hw_tso = false;
		priv->hw_qos = false;
		priv->rx_coherent_enabled = true;
		priv->rx_page_pool_enabled = false;
		return len;
	}
	if (!strcmp(buf, "accel-off")) {
		if (!priv->is_mw5)
			return -EOPNOTSUPP;
		WRITE_ONCE(mw5_accel_enable, false);
		priv->accel_sw = false;
		priv->accel_hw = false;
		rtl8197f_mw5_accel_sw_flush(priv);
		return len;
	}
	if (!strcmp(buf, "accel-sw-on")) {
		if (!priv->is_mw5)
			return -EOPNOTSUPP;
		WRITE_ONCE(mw5_accel_sw_enable, true);
		priv->accel_sw = READ_ONCE(mw5_accel_enable);
		return len;
	}
	if (!strcmp(buf, "accel-sw-off")) {
		if (!priv->is_mw5)
			return -EOPNOTSUPP;
		WRITE_ONCE(mw5_accel_sw_enable, false);
		priv->accel_sw = false;
		rtl8197f_mw5_accel_sw_flush(priv);
		return len;
	}
	if (!strcmp(buf, "accel-hw-on")) {
		if (!priv->is_mw5)
			return -EOPNOTSUPP;
		WRITE_ONCE(mw5_accel_hw_enable, true);
		priv->accel_hw = READ_ONCE(mw5_accel_enable);
		return len;
	}
	if (!strcmp(buf, "accel-hw-off")) {
		if (!priv->is_mw5)
			return -EOPNOTSUPP;
		WRITE_ONCE(mw5_accel_hw_enable, false);
		priv->accel_hw = false;
		return len;
	}
	if (!strcmp(buf, "napt-on")) {
		if (priv->is_mw5)
			return -EOPNOTSUPP; /* retired: unified MW5 accelerator owns policy */
		if (!priv->is_iball)
			return -EOPNOTSUPP;
		priv->hw_napt = true;
		return len;
	}
	if (!strcmp(buf, "napt-off")) {
		if (priv->is_mw5)
			return -EOPNOTSUPP;
		if (!priv->is_iball)
			return -EOPNOTSUPP;
		priv->hw_napt = false;
		rtl8197f_rtk_napt_flush(priv, false);
		return len;
	}
	if (!strcmp(buf, "wfo-on") || !strcmp(buf, "wfo-off"))
		return -EOPNOTSUPP; /* legacy WFO retired on MW5 */
	if (!strcmp(buf, "coalesce-off")) {
		rtl8197f_rtk_mw5_set_coalesce(ndev, false);
		return len;
	}
	if (!strcmp(buf, "coalesce-test")) {
		rtl8197f_rtk_mw5_set_coalesce(ndev, true);
		return len;
	}
	if (!strcmp(buf, "qos-on")) {
		if (priv->is_mw5)
			return -EOPNOTSUPP; /* retired: unified accelerator owns MW5 */
		if (!priv->is_iball)
			return -EOPNOTSUPP;
		priv->hw_qos = true;
		rtl8197f_rtk_apply_hw_qos(priv);
		return len;
	}
	if (!strcmp(buf, "qos-off")) {
		if (priv->is_mw5)
			return -EOPNOTSUPP;
		priv->hw_qos = false;
		rtl8197f_rtk_apply_hw_qos(priv);
		return len;
	}
	if (!strcmp(buf, "invalid-drop-on")) {
		if (!of_machine_is_compatible("tenda,nova-mw5"))
			return -EOPNOTSUPP;
		WRITE_ONCE(mw5_drop_invalid_untagged, true);
		return len;
	}
	if (!strcmp(buf, "invalid-drop-off")) {
		WRITE_ONCE(mw5_drop_invalid_untagged, false);
		return len;
	}

	cmd = rtl8197f_rtk_rd05_cmd_from_name(buf);
	if (cmd < 0)
		return cmd;

	ret = rtl8197f_rtk_rd05_reseed_cmd(priv, "procfs", cmd);
	return ret ? ret : len;
}

static const struct proc_ops rtl8197f_rtk_rd05_proc_ops = {
	.proc_open = rtl8197f_rtk_rd05_proc_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
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

static int rtl8197f_rtk_set_mac_address(struct net_device *ndev, void *addr)
{
	struct rtl8197f_rtknet_priv *priv = netdev_priv(ndev);
	u8 old_addr[ETH_ALEN];
	int ret;

	ether_addr_copy(old_addr, ndev->dev_addr);
	ret = eth_mac_addr(ndev, addr);
	if (ret)
		return ret;
	if (ether_addr_equal(old_addr, ndev->dev_addr))
		return 0;

	priv->rd05_mac_changes++;
	netdev_info(ndev,
		    "MW5/RD05 MAC sync v43.9: %pM -> %pM running=%u changes=%llu\n",
		    old_addr, ndev->dev_addr, netif_running(ndev),
		    (unsigned long long)priv->rd05_mac_changes);

	if (!rtl8197f_rtk_p0_external_switch_board() ||
	    !netif_running(ndev))
		return 0;

	ret = rtl8197f_rtk_rd05_reseed_cmd(priv, "mac-change", RTL_RTK_RD05_PRIV_RESEED);
	if (ret)
		netdev_warn(ndev,
			    "failed to reseed RTL865x tables after MAC change: %d\n",
			    ret);

	/* The address is already installed. Keep netifd in sync and report the
	 * hardware re-seed failure through the log and /proc diagnostics.
	 */
	return 0;
}

static const struct net_device_ops rtl8197f_rtk_netdev_ops = {
	.ndo_open		= rtl8197f_rtk_open,
	.ndo_stop		= rtl8197f_rtk_stop,
	.ndo_start_xmit		= rtl8197f_rtk_start_xmit,
	.ndo_tx_timeout		= rtl8197f_rtk_tx_timeout,
	.ndo_change_mtu		= rtl8197f_rtk_change_mtu,
	.ndo_set_mac_address	= rtl8197f_rtk_set_mac_address,
	.ndo_validate_addr	= eth_validate_addr,
	.ndo_do_ioctl		= rtl8197f_rtk_do_ioctl,
	.ndo_setup_tc		= rtl8197f_rtk_setup_tc,
	.ndo_features_check	= rtl8197f_rtk_features_check,
	.ndo_fix_features	= rtl8197f_rtk_fix_features,
};

static void rtl8197f_rtk_read_u32_prop(struct device_node *np,
				       const char *name, u32 *val, u32 min,
				       u32 max)
{
	u32 tmp;

	if (!of_property_read_u32(np, name, &tmp))
		*val = clamp_t(u32, tmp, min, max);
}

static int rtl8197f_rtk_hex_nibble(u8 c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return -EINVAL;
}

static int rtl8197f_rtk_parse_ascii_mac(const u8 *value, size_t len, u8 *addr)
{
	int high = -1;
	int octet = 0;
	size_t i;

	for (i = 0; i < len; i++) {
		int nibble;

		if (value[i] == ':' || value[i] == '-' || value[i] == ' ' || value[i] == '\t')
			continue;
		nibble = rtl8197f_rtk_hex_nibble(value[i]);
		if (nibble < 0)
			return nibble;
		if (high < 0) {
			high = nibble;
			continue;
		}
		if (octet >= ETH_ALEN)
			return -EINVAL;
		addr[octet++] = (high << 4) | nibble;
		high = -1;
	}

	if (octet != ETH_ALEN || high >= 0 || !is_valid_ether_addr(addr))
		return -EINVAL;
	return 0;
}

/* The MW5 factory partition is a NUL-separated ASCII key/value store, not a
 * six-byte nvmem MAC cell.  Read HW_NIC0_ADDR here so the Ethernet master has
 * the real board identity before its first ndo_open()/RTL865x table seed.
 * Userspace UCI defaults are deliberately too late for that first seed.
 */
static int rtl8197f_rtk_read_mw5_factory_mac(struct device *dev, u8 *addr)
{
	static const char key[] = "HW_NIC0_ADDR=";
	struct nvmem_cell *cell;
	u8 *buf;
	size_t len, i;
	int ret = -ENOENT;

	cell = nvmem_cell_get(dev, "factory");
	if (IS_ERR(cell))
		return PTR_ERR(cell);
	buf = nvmem_cell_read(cell, &len);
	nvmem_cell_put(cell);
	if (IS_ERR(buf))
		return PTR_ERR(buf);

	for (i = 0; i + sizeof(key) - 1 < len; i++) {
		size_t start, end;

		if (memcmp(buf + i, key, sizeof(key) - 1))
			continue;
		start = i + sizeof(key) - 1;
		end = start;
		while (end < len && buf[end] != '\0' && buf[end] != '\n' &&
		       buf[end] != '\r' && buf[end] != 0xff)
			end++;
		ret = rtl8197f_rtk_parse_ascii_mac(buf + start, end - start, addr);
		break;
	}

	kfree(buf);
	return ret;
}

static void rtl8197f_rtk_read_named_mac(struct device *dev, const char *name,
                                        u8 *addr, bool *valid)
{
    struct nvmem_cell *cell;
    void *buf;
    size_t len;

    *valid = false;
    cell = nvmem_cell_get(dev, name);
    if (IS_ERR(cell))
        return;
    buf = nvmem_cell_read(cell, &len);
    nvmem_cell_put(cell);
    if (IS_ERR(buf))
        return;
    if (len >= ETH_ALEN && is_valid_ether_addr(buf)) {
        ether_addr_copy(addr, buf);
        *valid = true;
    }
    kfree(buf);
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
	BUILD_BUG_ON(offsetof(struct rtl8197f_rtk_desc, opts5) != 20);
	BUILD_BUG_ON(sizeof(struct rtl8197f_rtk_rd05_pkthdr) != 32);
	BUILD_BUG_ON(sizeof(struct rtl8197f_rtk_rd05_mbuf) != 32);
	priv->dev = &pdev->dev;
	priv->ndev = ndev;
	priv->is_mw5 = of_machine_is_compatible("tenda,nova-mw5");
	priv->is_iball = of_machine_is_compatible("iball,wrd12gn");
	mutex_init(&priv->napt_lock);
	spin_lock_init(&priv->accel_sw_lock);
	INIT_LIST_HEAD(&priv->accel_sw_flows);
	{
		unsigned int i;
		for (i = 0; i < MW5_ACCEL_SW_HASH_SIZE; i++)
			INIT_HLIST_HEAD(&priv->accel_sw_hash[i]);
	}
	INIT_LIST_HEAD(&priv->napt_hw_flows);
	INIT_LIST_HEAD(&priv->napt_cookies);
	bitmap_zero(priv->napt_used, RTL_RTK_NAPT_TABLE_SIZE);
	priv->lan_vid = RTL_RTK_IBALL_LAN_VID_DEFAULT;
	priv->wan_vid = RTL_RTK_IBALL_WAN_VID_DEFAULT;
	priv->lan_port_mask = RTL_RTK_IBALL_LAN_MASK_DEFAULT;
	priv->wan_port_mask = RTL_RTK_IBALL_WAN_MASK_DEFAULT;
	if (priv->is_mw5) {
		/* V212 live VLAN table: LAN VID9 -> P1 + internal P8/FID0,
		 * WAN VID8 -> P3/FID1. The CPU descriptor DP field is one-hot P1/P3. */
		priv->lan_vid = RTL_RTK_MW5_LAN_VID;
		priv->wan_vid = RTL_RTK_MW5_WAN_VID;
		priv->lan_port_mask = BIT(RTL_RTK_MW5_LAN_PORT);
		priv->wan_port_mask = BIT(RTL_RTK_MW5_WAN_PORT);
	}
	priv->rx_ring_size = RTL_RTK_DEFAULT_RX_RING;
	priv->tx_ring_size = RTL_RTK_DEFAULT_TX_RING;
	priv->rx_buf_size = RTL_RTK_DEFAULT_RX_BUFSZ;
	priv->tx_port_mask = RTL_RTK_DEFAULT_TX_PORT_MASK;
	priv->tx_dp_ext = RTL_RTK_DEFAULT_TX_DP_EXT;
	priv->tx_extspa = RTL_RTK_DEFAULT_TX_EXTSPA;
	priv->rx_poll_ms = RTL_RTK_DEFAULT_RX_POLL_MS;
	priv->desc_dwords = 8;
	priv->desc_stride = 8 * sizeof(u32);
	priv->msg_enable = NETIF_MSG_DRV | NETIF_MSG_PROBE | NETIF_MSG_LINK;
	priv->rx_headroom = NET_SKB_PAD + NET_IP_ALIGN;

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
	if (rtl8197f_rtk_p0_external_switch_board())
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
					       RTL_RTK_MIN_RING, RTL_RTK_MAX_RX_RING);
		rtl8197f_rtk_read_u32_prop(np, "realtek,tx-ring-size",
					       &priv->tx_ring_size,
					       RTL_RTK_MIN_RING, RTL_RTK_MAX_TX_RING);
		rtl8197f_rtk_read_u32_prop(np, "realtek,rx-buffer-size",
					       &priv->rx_buf_size, 1536, 16384);
		rtl8197f_rtk_read_u32_prop(np, "realtek,tx-port-mask",
					       &priv->tx_port_mask, 1, RTL_RTK_TX_DP_MASK);
		rtl8197f_rtk_read_u32_prop(np, "realtek,tx-dp-ext",
					       &priv->tx_dp_ext, 0, RTL_RTK_TX_DP_EXT_MASK);
		rtl8197f_rtk_read_u32_prop(np, "realtek,tx-extspa",
					       &priv->tx_extspa, 0, RTL_RTK_TX_EXTSPA_MASK);
		if (!of_property_read_u32(np, "realtek,descriptor-dwords", &priv->desc_dwords)) {
			if (priv->desc_dwords != 6 && priv->desc_dwords != 8) {
				dev_warn(&pdev->dev,
					 "unsupported descriptor-dwords=%u; using 8\n",
					 priv->desc_dwords);
				priv->desc_dwords = 8;
			}
			priv->desc_stride = priv->desc_dwords * sizeof(u32);
		}
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
		priv->rd05_sdk_newdesc_rx = of_property_read_bool(np,
						"realtek,new-descriptor-rx");
		priv->hw_csum = of_property_read_bool(np,
						"realtek,hw-checksum");
		priv->hw_sg = of_property_read_bool(np, "realtek,hw-sg");
		priv->hw_tso = of_property_read_bool(np, "realtek,hw-tso");
		priv->hw_qos = of_property_read_bool(np, "realtek,hw-qos");
		priv->internal_vlan_split = of_property_read_bool(np,
						"realtek,internal-vlan-split");
		priv->napt_capable = of_property_read_bool(np,
						"realtek,hw-napt-flowtable-capable");
		{
			u32 vid;

			if (!of_property_read_u32(np, "realtek,lan-vid", &vid))
				priv->lan_vid = clamp_t(u32, vid, 1, VLAN_VID_MASK);
			if (!of_property_read_u32(np, "realtek,wan-vid", &vid))
				priv->wan_vid = clamp_t(u32, vid, 1, VLAN_VID_MASK);
		}
		rtl8197f_rtk_read_u32_prop(np, "realtek,lan-port-mask",
				       &priv->lan_port_mask, 1, 0x1ff);
		rtl8197f_rtk_read_u32_prop(np, "realtek,wan-port-mask",
				       &priv->wan_port_mask, 1, 0x1ff);
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


	if (of_machine_is_compatible("tenda,nova-mw5")) {
		/* GPL SDK CONFIG_RTL_8197F + CONFIG_RTL_8367R_SUPPORT:
		 * external switch on physical P0/RGMII and CPU-tag parsing enabled
		 * with the tag preserved for Linux DSA.  The OEM CPUICR leaves
		 * EXCLUDE_CRC clear: RX descriptor length includes the four-byte FCS
		 * and TX ph_len/m_len include the hardware-generated FCS as well.
		 */
		priv->rx_poll_fallback = true;
		priv->sdk_crc_lengths = true;
		priv->sdk_trxrdy = true;
		priv->sdk_cpuicr1_init = true;
		priv->sdk_swcore_init = true;
		priv->p0_cpu_tag_passthrough = true;
		/* v44.66.10: Linux DSA carries the RTL8367 rtl8_4 tag itself. The GPL
		 * SDK couples P0 router mode to the SoC CPU-tag parser/generator, so with
		 * those parser bits disabled the host link must default to normal mode. */
		priv->mw5_p0_transparent = true;
		/* MW5 uses CONFIG_RTL_SWITCH_NEW_DESCRIPTOR with the native
		 * six-dword/24-byte RTL8197FS RX descriptor. Mark it explicitly so
		 * ring allocation and polling never enter the legacy rxmbuf path.
		 */
		priv->rd05_sdk_newdesc_rx = true;
		/* The RTL8363/RTL8367 CPU link terminates on physical P0, but
		 * packets still traverse the RTL8197FS internal rtl865x domain
		 * before reaching CPUIF DMA. Seed the OEM PVID/VLAN/netif/L2/ACL
		 * path with the real MW5 MAC instead of leaving legacy-pipeline=0.
		 */
		priv->legacy_rd05_pipeline = true;
		/* RTL8197FS is non-VG: SDK leaves CF_TXDESC/CF_RXDESC at
		 * zero and uses the native six-dword descriptor.
		 */
		priv->desc_dwords = 6;
		priv->desc_stride = 6 * sizeof(u32);
		/* Unified HW engine baseline: coherent fixed DMA is the only MW5 RX
		 * ownership model. All classic Realtek checksum/SG/TSO/QoS knobs are
		 * retired and held off, even if stale module arguments are present. */
		priv->rx_coherent_enabled = true;
		priv->rx_page_pool_enabled = false;
		priv->hw_csum = false;
		priv->hw_rx_csum = false;
		priv->hw_sg = false;
		priv->hw_tso = false;
		priv->hw_qos = false;
		/* v44.65.17: unified accelerator owns MW5 fast paths.  Never enable
		 * the old HWLOOKUP/NAPT/EXT_PORT gates in parallel. */
		priv->hw_napt = false;
		priv->tx_hwlookup = false;
		/* V212 OEM runtime: ring0 is RX=900/TX=768 with 24-byte descriptors.
		 * Board-scope these values instead of retaining the old 128/128 bring-up
		 * assumption. Additional OEM RX/TX rings remain intentionally disabled. */
		priv->rx_ring_size = RTL_RTK_MW5_OEM_RX_RING0;
		priv->tx_ring_size = RTL_RTK_MW5_OEM_TX_RING0;
		priv->accel_sw = mw5_accel_enable && mw5_accel_sw_enable;
		priv->accel_hw = mw5_accel_enable && mw5_accel_hw_enable;
		priv->accel_hw_l34_safe = false; /* Realtek DSA tag offset is not RTL865x-L34-proven */
	}

	if (priv->is_iball) {
		/* Bring-up defaults stay fail-closed. Each dataplane accelerator is an
		 * explicit module opt-in. HW-NAPT additionally requires a flowtable
		 * callback; no packet is offloaded merely because this flag is set.
		 */
		priv->rx_page_pool_enabled = false;
		priv->hw_csum = rtl8197f_hw_csum_enable;
		priv->hw_rx_csum = rtl8197f_hw_csum_enable;
		priv->hw_sg = rtl8197f_hw_sg_enable;
		priv->hw_tso = rtl8197f_hw_tso_enable;
		priv->hw_qos = rtl8197f_hw_qos_enable;
		priv->hw_napt = priv->napt_capable && rtl8197f_hw_napt_enable;
		priv->tx_hwlookup = false;
		priv->tx_bridge = false;
		priv->tx_port_mask = priv->lan_port_mask;
		priv->tx_dp_ext = 0;
		priv->tx_extspa = 0;
		priv->sdk_trxrdy = true;
		priv->sdk_cpuicr1_init = true;
		priv->sdk_swcore_init = true;
		priv->rd05_sdk_newdesc_rx = true;
		/* Keep descriptor geometry from DT/OEM evidence. The iBall serial
		 * identifies the WLAN bond as 97FN/PKG2, so physical RTL8197FH marking
		 * alone is not enough evidence to force the VG 8-dword ring layout. */
	}

	if (np) {
		if (of_property_read_bool(np, "realtek,sdk-trxrdy"))
			priv->sdk_trxrdy = true;
		if (of_property_read_bool(np, "realtek,sdk-cpuicr1-init"))
			priv->sdk_cpuicr1_init = true;
		if (of_property_read_bool(np, "realtek,sdk-swcore-init"))
			priv->sdk_swcore_init = true;
	}

	if (of_machine_is_compatible("xiaomi,r4-rd05") ||
	    of_machine_is_compatible("tenda,nova-mw5")) {
		/* GPL SDK topology for RTL8197F + RTL83xx: physical P0/RGMII is the
		 * SoC-facing cascade link. The external switch CPU port is the opposite
		 * endpoint, not RTL8197F internal extension port 7. Direct TX to P0;
		 * DSA supplies the board-selected CPU tag in the frame.
		 */
		/* RD05 may retain its legacy diagnostic semantics; MW5 must never
		 * resurrect retired HWLOOKUP after the unified-accelerator setup. */
		priv->tx_hwlookup = false;
		priv->tx_bridge = false;
		priv->tx_port_mask = BIT(0);
		priv->tx_dp_ext = 0;
		priv->tx_extspa = 0;
		dev_info(&pdev->dev,
			 "%s DSA master: RTL8197F P0/RGMII <-> external RTL83xx CPU port, RX CPU-tag recognition with tag preservation, direct DP=0x1, tag=%s\n",
			 of_machine_is_compatible("tenda,nova-mw5") ? "MW5" : "RD05",
			 "rtl8_4/8byte");
	} else if ((priv->tx_port_mask & BIT(6)) && !priv->tx_dp_ext) {
		dev_warn(&pdev->dev,
			 "tx-port-mask targets rtl865x extension/CPU path but tx-dp-ext is 0; external traffic may not pass\n");
	}

	if (priv->is_iball) {
		ret = rtl8197f_rtk_register_mdio(priv, np);
		if (ret) {
			dev_err(&pdev->dev, "iBall RTL8211F MDIO registration failed: %d\n", ret);
			goto err_free_netdev;
		}
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

	/* v44.65.15: MW5 must not enter ndo_open with alloc_etherdev's random
	 * locally-administered address.  The RTL865x L2/netif tables are seeded
	 * during that first open, before netifd/UCI can correct the MAC. */
	ret = np ? of_get_ethdev_address(np, ndev) : -ENOENT;
	if (ret && priv->is_mw5) {
		u8 factory_mac[ETH_ALEN];

		ret = rtl8197f_rtk_read_mw5_factory_mac(&pdev->dev, factory_mac);
		if (!ret) {
			eth_hw_addr_set(ndev, factory_mac);
			ether_addr_copy(priv->wan_mac, factory_mac);
			eth_addr_add(priv->wan_mac, 7);
			priv->wan_mac_valid = is_valid_ether_addr(priv->wan_mac);
			dev_info(&pdev->dev,
				 "MW5 factory identity v44.65.15: eth0=%pM routed-wan=%pM valid=%u\n",
				 ndev->dev_addr, priv->wan_mac, priv->wan_mac_valid);
		} else {
			dev_warn(&pdev->dev,
				 "MW5 factory HW_NIC0_ADDR unavailable (%d); using a temporary random MAC\n",
				 ret);
		}
	}
	if (!is_valid_ether_addr(ndev->dev_addr))
		eth_hw_addr_random(ndev);

	/* If a future DT supplies a binary base MAC directly, still derive the
	 * OEM routed-WAN identity deterministically before the first seed. */
	if (priv->is_mw5 && !priv->wan_mac_valid &&
	    is_valid_ether_addr(ndev->dev_addr) && !is_local_ether_addr(ndev->dev_addr)) {
		ether_addr_copy(priv->wan_mac, ndev->dev_addr);
		eth_addr_add(priv->wan_mac, 7);
		priv->wan_mac_valid = is_valid_ether_addr(priv->wan_mac);
	}
	if (priv->is_iball)
		rtl8197f_rtk_read_named_mac(&pdev->dev, "wan-mac-address",
					       priv->wan_mac, &priv->wan_mac_valid);

	if (of_machine_is_compatible("tenda,nova-mw5")) {
		/* v44.66.9 exposes every native NIC feature by default. The master
		 * recognizes the Realtek DSA 0x8899 tag per packet through
		 * ndo_features_check(); fixed-function L3/LSO operations are removed
		 * from a tagged skb unless the parser geometry is safe. */
		ndev->hw_features |= NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM |
			NETIF_F_RXCSUM | NETIF_F_SG | NETIF_F_TSO | NETIF_F_TSO6;
		if (priv->hw_csum)
			ndev->features |= NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM;
		if (priv->hw_rx_csum)
			ndev->features |= NETIF_F_RXCSUM;
		if (priv->hw_sg)
			ndev->features |= NETIF_F_SG;
		if (priv->hw_tso && priv->hw_sg && priv->hw_csum)
			ndev->features |= NETIF_F_TSO | NETIF_F_TSO6;
	} else if (priv->is_iball) {
		/* Internal RTL865x VLAN split has no in-band RTL836x CPU tag. Expose
		 * the native RTL8197FH checksum/SG/TSO blocks as runtime-testable
		 * capabilities while keeping every feature off unless explicitly opted in.
		 */
		ndev->hw_features |= NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM |
			NETIF_F_RXCSUM | NETIF_F_SG | NETIF_F_TSO | NETIF_F_TSO6;
		if (priv->hw_csum)
			ndev->features |= NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM;
		if (priv->hw_rx_csum)
			ndev->features |= NETIF_F_RXCSUM;
		if (priv->hw_sg)
			ndev->features |= NETIF_F_SG;
		if (priv->hw_tso && priv->hw_sg && priv->hw_csum)
			ndev->features |= NETIF_F_TSO | NETIF_F_TSO6;
	} else {
		if (priv->hw_csum) {
			ndev->hw_features |= NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM | NETIF_F_RXCSUM;
			ndev->features |= NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM;
			if (priv->hw_rx_csum)
				ndev->features |= NETIF_F_RXCSUM;
		}
		if (priv->hw_sg) {
			ndev->hw_features |= NETIF_F_SG;
			ndev->features |= NETIF_F_SG;
		}
		if (priv->hw_tso && priv->hw_sg && priv->hw_csum) {
			ndev->hw_features |= NETIF_F_TSO | NETIF_F_TSO6;
			ndev->features |= NETIF_F_TSO | NETIF_F_TSO6;
		}
	}
	ndev->vlan_features |= ndev->hw_features &
		(NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM | NETIF_F_RXCSUM |
		 NETIF_F_SG | NETIF_F_TSO | NETIF_F_TSO6);
	if (priv->is_iball && priv->internal_vlan_split) {
		ndev->hw_features |= NETIF_F_HW_VLAN_CTAG_RX | NETIF_F_HW_VLAN_CTAG_TX;
		ndev->features |= NETIF_F_HW_VLAN_CTAG_RX | NETIF_F_HW_VLAN_CTAG_TX;
	}

	netif_carrier_off(ndev);
	spin_lock_init(&priv->tx_lock);
	timer_setup(&priv->rx_poll_timer, rtl8197f_rtk_rx_poll_timer, 0);
	INIT_DELAYED_WORK(&priv->rd05_reseed_work, rtl8197f_rtk_rd05_reseed_work);
	if (priv->is_mw5)
		netif_napi_add_weight(ndev, &priv->napi, rtl8197f_rtk_poll,
				RTL_RTK_MW5_NAPI_WEIGHT);
	else
		netif_napi_add(ndev, &priv->napi, rtl8197f_rtk_poll);

	if (of_machine_is_compatible("tenda,nova-mw5")) {
		/* v44.65.21: revert v44.65.20 software IRQ coalescing after UART
		 * testing showed early malformed RX/DSA frames. Keep the profile
		 * available through the diagnostic proc command, but default to the
		 * v44.65.18/19 direct-IRQ NAPI behavior proven under load.
		 */
		ndev->gro_flush_timeout = RTL_RTK_MW5_GRO_FLUSH_NS;
		ndev->napi_defer_hard_irqs = RTL_RTK_MW5_NAPI_DEFER_HARD_IRQS;
	}

	ret = register_netdev(ndev);
	if (ret)
		goto err_del_napi;
	if (priv->is_mw5)
		WRITE_ONCE(rtl8197f_mw5_runtime, priv);

	if (priv->is_iball && priv->phy_node) {
		priv->phydev = of_phy_connect(ndev, priv->phy_node,
					      rtl8197f_rtk_adjust_link, 0,
					      priv->phy_mode);
		if (!priv->phydev) {
			dev_err(&pdev->dev, "failed to attach RTL8211F PHY\n");
			ret = -ENODEV;
			goto err_unregister_netdev;
		}
		phy_support_asym_pause(priv->phydev);
	}

	dev_info(&pdev->dev,
		 "v44.66.0 OEM ring/descriptor integration: coherent_rx=%d coherent_tx=%d page_pool=%d txcsum_default=%d rxcsum_default=%d sg_default=%d tso=%d qos=%d hwlookup_gate=%d dsa_desc_hwlookup=0 hw_napt=%d extport_wlan=%d accel_sw=%d accel_hw=%d asic_l34_safe=%d rings=%u/%u gro_flush_ns=%lu defer_hard_irqs=%u\n",
		 priv->rx_coherent_enabled, priv->tx_coherent_enabled, priv->rx_page_pool_enabled,
		 priv->hw_csum, priv->hw_rx_csum, priv->hw_sg,
		 priv->hw_tso, priv->hw_qos, priv->tx_hwlookup,
		 priv->hw_napt, READ_ONCE(mw5_extport_wlan_enable),
		 priv->accel_sw, priv->accel_hw, priv->accel_hw_l34_safe,
		 priv->rx_ring_size, priv->tx_ring_size,
		 ndev->gro_flush_timeout, ndev->napi_defer_hard_irqs);

	if (rtl8197f_rtk_p0_external_switch_board() || priv->is_iball) {
		priv->rd05_proc = proc_create_data(priv->is_iball ? "rtl8197f-rtknet" : "rd05-rtknet",
						   0600, NULL,
						   &rtl8197f_rtk_rd05_proc_ops, ndev);
		if (!priv->rd05_proc)
			dev_warn(&pdev->dev, "%s procfs trigger /proc/rd05-rtknet not available\n",
				 rtl8197f_rtk_p0_board_name());
	}

	dev_info(&pdev->dev,
		 "registered %s: rx=%u tx=%u buf=%u desc=%u/%u tx-port-mask=0x%x tx-dp-ext=0x%x tx-extspa=0x%x tx-hwlookup=%d tx-bridge=%d desc-kseg1=%d desc-phys=%d rx-poll=%d/%ums sdk-cpuif=%d sdk-swcore=%d legacy-pipeline=%d trxrdy=%d sdk-crc=%d rd05-legacy-rxphys=%d\n",
		 ndev->name, priv->rx_ring_size, priv->tx_ring_size,
		 priv->rx_buf_size, priv->desc_dwords, priv->desc_stride,
		 priv->tx_port_mask, priv->tx_dp_ext, priv->tx_extspa,
		 priv->tx_hwlookup, priv->tx_bridge, priv->desc_addr_kseg1,
		 priv->desc_addr_physical, priv->rx_poll_fallback, priv->rx_poll_ms,
		 priv->sdk_cpuicr1_init, priv->sdk_swcore_init,
		 priv->legacy_rd05_pipeline, priv->sdk_trxrdy,
		 priv->sdk_crc_lengths,
		 !!of_machine_is_compatible("xiaomi,r4-rd05"));
	return 0;

err_unregister_netdev:
	unregister_netdev(ndev);
err_del_napi:
	netif_napi_del(&priv->napi);
err_free_netdev:
	if (priv->mdiobus_registered) {
		mdiobus_unregister(priv->mii_bus);
		priv->mdiobus_registered = false;
	}
	of_node_put(priv->phy_node);
	priv->phy_node = NULL;
	free_netdev(ndev);
	return ret;
}

static void rtl8197f_rtk_remove(struct platform_device *pdev)
{
	struct net_device *ndev = platform_get_drvdata(pdev);
	struct rtl8197f_rtknet_priv *priv = netdev_priv(ndev);

	cancel_delayed_work_sync(&priv->rd05_reseed_work);
	if (priv->is_mw5 && READ_ONCE(rtl8197f_mw5_runtime) == priv) {
		WRITE_ONCE(rtl8197f_mw5_runtime, NULL);
		synchronize_net();
	}
	rtl8197f_mw5_accel_sw_flush(priv);
	rtl8197f_rtk_napt_flush(priv, true);
	if (priv->rd05_proc)
		proc_remove(priv->rd05_proc);
	unregister_netdev(ndev);
	if (priv->phydev) {
		phy_disconnect(priv->phydev);
		priv->phydev = NULL;
	}
	if (priv->mdiobus_registered) {
		mdiobus_unregister(priv->mii_bus);
		priv->mdiobus_registered = false;
	}
	of_node_put(priv->phy_node);
	priv->phy_node = NULL;
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
