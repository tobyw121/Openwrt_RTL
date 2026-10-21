#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Native tests of RD05 WAN identity parsing and collision-safe CPU entries."""
from pathlib import Path
import subprocess
import tempfile
import unittest
from test_mw5_stability import ROOT, c_function

DRIVER = ROOT / 'target/linux/realtek/files-6.6/drivers/net/ethernet/rtl8197f_rtknet.c'


class RD05WANTests(unittest.TestCase):
    def test_factory_mac_formats_and_l2_hash_collisions(self):
        names = ['static int rtl8197f_rtk_hex_nibble(',
                 'static int rtl8197f_rtk_parse_ascii_mac(',
                 'static void rtl8197f_rtk_read_named_mac(',
                 'static u32 rtl8197f_rtk_rd05_l2_w0(',
                 'static u32 rtl8197f_rtk_rd05_l2_w1(',
                 'static u8 rtl8197f_rtk_rd05_l2_row(',
                 'static void rtl8197f_rtk_rd05_seed_wan_cpu(']
        code = r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
#define ETH_ALEN 6
#define BIT(n) (1U << (n))
#define IS_ERR(p) (!(p))
#define RTL_RTK_ASICTBL_TYPE_L2 0
#define kfree free
#define ether_addr_copy(a,b) memcpy(a,b,6)
static bool is_valid_ether_addr(const void *p) {
    const u8 *a = p; u8 zero[6] = {0};
    return !(a[0] & 1) && memcmp(a,zero,6);
}
struct device {}; struct nvmem_cell {};
static const u8 *cell_data; static size_t cell_len;
static struct nvmem_cell cell;
static struct nvmem_cell *nvmem_cell_get(struct device *d, const char *s) { return &cell; }
static void nvmem_cell_put(struct nvmem_cell *c) {}
static void *nvmem_cell_read(struct nvmem_cell *c, size_t *len) {
    *len = cell_len; void *p = malloc(cell_len); memcpy(p,cell_data,cell_len); return p;
}
struct rtl8197f_rtknet_priv { bool wan_mac_valid; u8 wan_mac[6]; };
static int rd05 = 1, count;
static u32 slots[2], words[2];
static int of_machine_is_compatible(const char *s) { return rd05; }
static int rtl8197f_rtk_rd05_force_table2(struct rtl8197f_rtknet_priv *p,
    u32 type, u32 slot, u32 w0, u32 w1, const char *name) {
    assert(count < 2); slots[count] = slot; words[count++] = w1; return 0;
}
''' + '\n'.join(c_function(DRIVER, name) for name in names) + r'''
int main(void) {
    struct device dev; u8 addr[6]; bool valid;
    const u8 expected[] = {0xa4,0xba,0x70,0x29,0x36,0xa8};
    cell_data = expected; cell_len = 6;
    rtl8197f_rtk_read_named_mac(&dev,"wan-mac-address",addr,&valid);
    assert(valid && !memcmp(addr,expected,6));
    const char *ascii[] = {"a4:ba:70:29:36:a8", "A4BA702936A8", "a4-ba-70-29-36-a8"};
    for (int i=0;i<3;i++) {
        cell_data = (const u8 *)ascii[i]; cell_len = strlen(ascii[i]);
        rtl8197f_rtk_read_named_mac(&dev,"wan-mac-address",addr,&valid);
        assert(valid && !memcmp(addr,expected,6));
    }
    const char *bad[] = {"", "00:00:00:00:00:00", "ff:ff:ff:ff:ff:ff", "a4:ba:70:29:36", "a4:ba:70:29:36:a8:00", "invalid-value"};
    for (int i=0;i<6;i++) {
        cell_data = (const u8 *)bad[i]; cell_len = strlen(bad[i]);
        rtl8197f_rtk_read_named_mac(&dev,"wan-mac-address",addr,&valid);
        assert(!valid);
    }
    /* Distinct LAN/WAN addresses with identical hardware row hashes. */
    const u8 lan[] = {2,0,0,0,0,0}, wan[] = {2,0,0,0,1,1};
    struct rtl8197f_rtknet_priv p = {.wan_mac_valid = true};
    memcpy(p.wan_mac,wan,6);
    assert(rtl8197f_rtk_rd05_l2_row(lan,0) == rtl8197f_rtk_rd05_l2_row(wan,0));
    rtl8197f_rtk_rd05_seed_wan_cpu(&p); assert(count == 2);
    for (int fid=0;fid<2;fid++) {
        assert(slots[fid] == ((u32)rtl8197f_rtk_rd05_l2_row(wan,fid) << 2 | 1));
        assert((words[fid] & (BIT(17)|BIT(18))) == (BIT(17)|BIT(18)));
        assert(((words[fid] >> 23) & 3) == fid);
        assert(!(words[fid] & (0x3f << 8)));
    }
    count = 0; p.wan_mac_valid = false;
    rtl8197f_rtk_rd05_seed_wan_cpu(&p); assert(count == 0);
    p.wan_mac_valid = true; rd05 = 0;
    rtl8197f_rtk_rd05_seed_wan_cpu(&p); assert(count == 0);
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            c = Path(tmp) / 'test.c'; binary = Path(tmp) / 'test'
            c.write_text(code)
            subprocess.run(['cc', '-std=gnu11', '-Werror', '-fsanitize=undefined',
                            str(c), '-o', str(binary)], check=True)
            subprocess.run([str(binary)], check=True)


    def test_p0_mmio_keeps_rd05_dsa_tag_opaque_with_old_and_new_dt(self):
        # Compile the complete production P0 setup function against fake MMIO.
        # Use the driver's real register definitions, including VG timing fields.
        definitions = []
        lines = DRIVER.read_text().splitlines(keepends=True)
        i = 0
        while i < len(lines):
            line = lines[i]
            if line.startswith('#define RTL_RTK_'):
                definition = line
                while line.rstrip().endswith('\\'):
                    i += 1
                    line = lines[i]
                    definition += line
                definitions.append(definition)
            i += 1
        code = r"""
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
typedef uint32_t u32;
#define BIT(n) (1U << (n))
#define GENMASK(h,l) ((~0U << (l)) & (~0U >> (31-(h))))
#define READ_ONCE(x) (x)
#define min_t(t,a,b) ((t)(a) < (t)(b) ? (t)(a) : (t)(b))
#define dev_info(...) ((void)0)
#define udelay(...) ((void)0)
static u32 readl(void *p) { return *(u32 *)p; }
static void writel(u32 v, void *p) { *(u32 *)p = v; }
static const char *board;
static bool of_machine_is_compatible(const char *s) { return !strcmp(s,board); }
static unsigned int rd05_p0_tx_delay = 0, rd05_p0_rx_delay = ~0U;
struct rtl8197f_rtknet_priv {
    void *swcore, *system, *dev;
    bool mw5_p0_transparent, p0_cpu_tag_passthrough;
};
""" + ''.join(definitions) + c_function(DRIVER, 'static void rtl8197f_rtk_init_p0_rgmii(') + r"""
int main(void) {
    u32 regs[0x10000 / 4], system[0x4000 / 4];
    struct rtl8197f_rtknet_priv p = {.swcore=regs, .system=system};
    const char *boards[] = {"xiaomi,r4-rd05", "xiaomi,r4-rd05", "tenda,nova-mw5", "legacy"};
    const u32 chip_ids[] = {0x8197f000, 0x81970001, 0x8197f000, 0};
    const u32 rx_default[] = {5, 6, 5, 6};
    for (int b=0;b<4;b++) for (int old_dt=0;old_dt<2;old_dt++) {
        board = boards[b]; memset(regs,0xff,sizeof(regs)); memset(system,0xff,sizeof(system));
        system[0] = chip_ids[b];
        p.p0_cpu_tag_passthrough = old_dt; p.mw5_p0_transparent = true;
        for (int reseed=0;reseed<2;reseed++) {
            rtl8197f_rtk_init_p0_rgmii(&p,"regression");
            u32 gmii = readl(p.swcore + RTL_RTK_SWCORE_P0GMIICR);
            u32 maccr1 = readl(p.swcore + RTL_RTK_SWCORE_MACCR1);
            u32 bits = gmii & (RTL_RTK_P0GMIICR_CPU_TAG_RX | RTL_RTK_P0GMIICR_CPU_TAG_TX);
            assert(!!bits == (b == 3 && old_dt));
            assert(!!(maccr1 & RTL_RTK_MACCR1_P0_ROUTER_MODE) == (b == 3 && old_dt));
            assert(gmii & RTL_RTK_P0GMIICR_CONF_DONE);
            assert((gmii & RTL_RTK_P0GMIICR_RX_DELAY_MASK) == rx_default[b]);
            if (b == 1 || b == 3) {
                assert((readl(p.swcore + RTL_RTK_SWCORE_PITCR) & RTL_RTK_PITCR_FIX_IPG_MASK) == RTL_RTK_PITCR_FIX_IPG_8BYTE);
                assert((readl(p.swcore + RTL_RTK_SWCORE_EXTPCR0) & RTL_RTK_EXTPCR0_TX_IPG_MASK) == RTL_RTK_EXTPCR0_TX_IPG_8BYTE);
            } else {
                assert((readl(p.swcore + RTL_RTK_SWCORE_PITCR) & RTL_RTK_PITCR_FIX_IPG_MASK) == RTL_RTK_PITCR_FIX_IPG_MASK);
                assert((readl(p.swcore + RTL_RTK_SWCORE_EXTPCR0) & RTL_RTK_EXTPCR0_TX_IPG_MASK) == RTL_RTK_EXTPCR0_TX_IPG_MASK);
            }
            if (b == 0)
                assert(readl(p.system + RTL_RTK_SYSTEM_PAD_CTRL_1) == ((0xffffffffU & 0x01bfffff) | 0xd8000000));
            if (b == 1)
                assert(readl(p.system + RTL_RTK_SYSTEM_PAD_CTRL_1) == ((0xffffffffU & 0x019fffff) | 0xda600000));
        }
    }
    board = boards[0]; system[0] = chip_ids[0];
    writel(0xdbffff7f, p.system + RTL_RTK_SYSTEM_PAD_CTRL_1);
    rtl8197f_rtk_init_p0_rgmii(&p,"logged-pad-regression");
    assert(readl(p.system + RTL_RTK_SYSTEM_PAD_CTRL_1) == 0xd9bfff7f);
    rd05_p0_rx_delay = 3;
    rtl8197f_rtk_init_p0_rgmii(&p,"manual-override");
    assert((readl(p.swcore + RTL_RTK_SWCORE_P0GMIICR) & RTL_RTK_P0GMIICR_RX_DELAY_MASK) == 3);
    return 0;
}
"""
        with tempfile.TemporaryDirectory() as tmp:
            c = Path(tmp) / 'p0.c'; binary = Path(tmp) / 'p0'
            c.write_text(code)
            subprocess.run(['cc', '-std=gnu11', '-Werror', '-fsanitize=undefined',
                            str(c), '-o', str(binary)], check=True)
            subprocess.run([str(binary)], check=True)


    def test_switch_cpu_stats_report_cpu_port_and_read_failures(self):
        patch = ROOT / 'target/linux/realtek/patches-6.6/336-net-dsa-realtek-rtl8367d-cpu-path-stats.patch'
        added = '\n'.join(line[1:] for line in patch.read_text().splitlines()
                          if line.startswith('+') and not line.startswith('+++'))
        start = added.index('static void rtl8367d_get_cpu_path_stats(')
        helper = added[start:added.index('\n}', start) + 2]
        code = r"""
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
typedef uint32_t u32; typedef uint64_t u64;
#define U64_MAX UINT64_MAX
#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))
#define RTL8365MB_MSTI_CTRL_REG(m,p) (0xa00 + (p)/8)
#define RTL8365MB_MSTI_CTRL_PORT_STATE_OFFSET(p) ((p)*2)
#define RTL8365MB_PORT_ISOLATION_REG(p) (0x800 + (p))
#define mutex_lock(x) ((void)0)
#define mutex_unlock(x) ((void)0)
static const char rtl8367d_cpu_stat_names[7][32];
struct rtl8365mb { struct { int trap_port; } cpu; int mib_lock; };
struct realtek_priv { struct rtl8365mb *chip_data; int num_ports; void *map; };
static int fail, reads;
static int rtl8365mb_mib_counter_read(struct realtek_priv *p,int port,int off,int len,u64 *out) {
    assert(port==7); reads++; *out = 123; /* partial result must be discarded on error */
    if (fail) return -1;
    *out = port*10000+off*10+len; return 0;
}
static int regmap_read(void *m,int reg,u32 *out) {
    reads++; if (fail) return -1;
    if (reg==0xa00) *out=(3U<<14)|(3U<<4);
    else if (reg==0x807) *out=0x1f;
    else if (reg==0x802) *out=0x80;
    else assert(0);
    return 0;
}
""" + helper + r"""
int main(void) {
    struct rtl8365mb mb = {.cpu.trap_port=7};
    struct realtek_priv p = {.chip_data=&mb,.num_ports=11};
    u64 out[8]; out[7]=0xdeadbeef;
    rtl8367d_get_cpu_path_stats(&p,2,out);
    assert(reads==7 && out[0]==70004 && out[1]==70604 && out[2]==70042);
    assert(out[3]==3 && out[4]==3 && out[5]==0x1f && out[6]==0x80);
    assert(out[7]==0xdeadbeef);
    fail=1; rtl8367d_get_cpu_path_stats(&p,2,out);
    for (int i=0;i<7;i++) assert(out[i]==U64_MAX);
    reads=0; mb.cpu.trap_port=11;
    rtl8367d_get_cpu_path_stats(&p,2,out); assert(reads==0);
    for (int i=0;i<7;i++) assert(out[i]==U64_MAX);
    assert(out[7]==0xdeadbeef);
    return 0;
}
"""
        with tempfile.TemporaryDirectory() as tmp:
            c = Path(tmp) / 'stats.c'; binary = Path(tmp) / 'stats'
            c.write_text(code)
            subprocess.run(['cc', '-std=gnu11', '-Werror', '-fsanitize=undefined',
                            str(c), '-o', str(binary)], check=True)
            subprocess.run([str(binary)], check=True)


if __name__ == '__main__':
    unittest.main()
