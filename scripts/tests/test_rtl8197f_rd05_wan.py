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


if __name__ == '__main__':
    unittest.main()
