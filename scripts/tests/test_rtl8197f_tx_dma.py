#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Exercise the native coherent ring address resolver with non-identity DMA."""
from pathlib import Path
import subprocess
import tempfile
import unittest
from test_mw5_stability import WLAN, c_function


class TXDMATests(unittest.TestCase):
    def test_coherent_offsets_and_out_of_range_entries(self):
        func = c_function(WLAN / '8192cd_osdep.c', 'int rtl8197f_wlan_tx_ring_dma(')
        source = r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
typedef uint64_t dma_addr_t;
typedef struct {
    unsigned char *desc_txbd_dma_buf, *desc_txdesc_dma_buf, *desc_bcn_dma_buf;
    size_t desc_dma_txbd_buf_len, desc_dma_txdesc_buf_len, desc_dma_bcn_buf_len;
    dma_addr_t desc_txbd_dma_addr, desc_txdesc_dma_addr, desc_bcn_dma_addr;
    void *alloc_desc_txbd_addr, *alloc_desc_txdesc_addr, *alloc_desc_bcn_addr;
} HAL_DATA_TYPE;
struct rtl8192cd_priv { HAL_DATA_TYPE *hal; };
#define _GET_HAL_DATA(p) ((p)->hal)
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
''' + func + r'''
int main(void) {
    unsigned char bd[256], desc[512], beacon[128], foreign[32];
    HAL_DATA_TYPE h = { bd, desc, beacon, sizeof(bd), sizeof(desc), sizeof(beacon),
        0x12000000, 0x23000000, 0, bd, desc, beacon };
    struct rtl8192cd_priv p = {&h};
    unsigned char *bases[] = {bd, desc, beacon};
    size_t sizes[] = {sizeof(bd), sizeof(desc), sizeof(beacon)};
    dma_addr_t dma[] = {h.desc_txbd_dma_addr, h.desc_txdesc_dma_addr, h.desc_bcn_dma_addr};
    for (int i = 0; i < 3; i++) for (size_t offset = 0; offset < sizes[i]; offset++) {
        dma_addr_t addr = 42;
        assert(rtl8197f_wlan_tx_ring_dma(&p, bases[i] + offset, sizes[i] - offset, &addr));
        assert(addr == dma[i] + offset);
        addr = 42;
        assert(!rtl8197f_wlan_tx_ring_dma(&p, bases[i] + offset, sizes[i] - offset + 1, &addr));
        assert(addr == 42);
    }
    dma_addr_t addr = 42;
    assert(!rtl8197f_wlan_tx_ring_dma(&p, foreign, 1, &addr));
    assert(!rtl8197f_wlan_tx_ring_dma(&p, bd, SIZE_MAX, &addr));
    assert(!rtl8197f_wlan_tx_ring_dma(&p, bd, 0, &addr));
    assert(!rtl8197f_wlan_tx_ring_dma(&p, NULL, 1, &addr));
    assert(!rtl8197f_wlan_tx_ring_dma(&p, bd, 1, NULL));
    h.alloc_desc_txbd_addr = NULL;
    assert(!rtl8197f_wlan_tx_ring_dma(&p, bd, 1, &addr));
    assert(addr == 42);
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            c = Path(tmp) / 'test.c'; binary = Path(tmp) / 'test'
            c.write_text(source)
            subprocess.run(['cc', '-std=c11', '-Werror', '-fsanitize=undefined',
                            str(c), '-o', str(binary)], check=True)
            subprocess.run([str(binary)], check=True)

    def test_probe_selects_chip_specific_allocator_for_each_board(self):
        source = (WLAN / '8192cd_osdep.c').read_text()
        start = source.index('#if defined(CONFIG_RTL_8198F) || defined(CONFIG_WLAN_HAL_8197F) || \\')
        end = source.index('\n\n\t\t} else', start)
        selection = source[start:end]
        prelude = r"""
#include <assert.h>
#include <errno.h>
#define VERSION_8197F 1
#define VERSION_8197G 2
#define VERSION_8822B 3
#define VERSION_8812F 4
#define VERSION_8814B 5
#define VERSION_8198F 6
#define GET_CHIP_VER(p) (*(p))
#define SUCCESS 1
#define KERN_ERR ""
#define panic_printk(...) ((void)0)
#define rtl8197f_wlan_phase_set(...) ((void)0)
#define pr_info(...) ((void)0)
static int selected, inject_failure;
static int InitWlanBuffer88XX_V1(int *p) { selected = 1; return !inject_failure; }
static int InitWlanBuffer88XX(int *p) { selected = 0; return !inject_failure; }
static int select_buffers(int chip) {
    int *priv = &chip;
"""
        profiles = [('rd05', ['8197G', '8812FE'], [2, 4], [1, 1]),
                    ('ac23', ['8197G', '8814BE'], [2, 5], [1, 1]),
                    ('mw5', ['8197F', '8822BE'], [1, 3], [1, 0]),
                    ('wrd12gn', ['8197F', '8822BE'], [1, 3], [1, 0])]
        with tempfile.TemporaryDirectory() as tmp:
            c = Path(tmp) / 'test.c'; binary = Path(tmp) / 'test'
            for board, flags, chips, expected in profiles:
                with self.subTest(board=board):
                    checks = ''.join(f'assert(select_buffers({chip}) == {want});\n'
                                     for chip, want in zip(chips, expected))
                    checks += f'inject_failure = 1; assert(select_buffers({chips[0]}) == -1);'
                    c.write_text(prelude + selection + r"""
    return selected;
err_out_dev:
    return -1;
}
int main(void) {
""" + checks + '\nreturn 0; }\n')
                    subprocess.run(['cc', '-std=c11', '-Werror', '-DCONFIG_RTL_8197F',
                                    *['-DCONFIG_WLAN_HAL_' + flag for flag in flags],
                                    str(c), '-o', str(binary)], check=True)
                    subprocess.run([str(binary)], check=True)

    def test_passive_binary_and_maximum_length_scan_ssids(self):
        func = c_function(WLAN / '8192cd_cfg80211.c',
                          'static int realtek_cfg80211_scan_ssid(')
        source = r"""
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
struct cfg80211_ssid { unsigned char ssid[32]; unsigned int ssid_len; };
struct cfg80211_scan_request { unsigned int n_ssids; struct cfg80211_ssid *ssids; };
struct rtl8192cd_priv { unsigned char ss_ssid[32]; unsigned int ss_ssidlen; };
""" + func + r"""
int main(void) {
    struct rtl8192cd_priv p;
    struct cfg80211_ssid ssid;
    struct cfg80211_scan_request req = { 1, &ssid };
    memset(&p, 0xcc, sizeof(p));
    memset(&ssid, 'X', sizeof(ssid));
    ssid.ssid_len = 32; /* Deliberately no NUL terminator. */
    assert(realtek_cfg80211_scan_ssid(&p, &req) == 0);
    assert(p.ss_ssidlen == 32 && !memcmp(p.ss_ssid, ssid.ssid, 32));
    ssid.ssid_len = 3; ssid.ssid[0] = 0; ssid.ssid[1] = 0xff;
    assert(realtek_cfg80211_scan_ssid(&p, &req) == 0);
    assert(p.ss_ssidlen == 3 && !memcmp(p.ss_ssid, ssid.ssid, 3));
    for (int i = 3; i < 32; i++) assert(p.ss_ssid[i] == 0);
    req.n_ssids = 0; req.ssids = NULL;
    assert(realtek_cfg80211_scan_ssid(&p, &req) == 0);
    assert(p.ss_ssidlen == 0);
    for (int i = 0; i < 32; i++) assert(p.ss_ssid[i] == 0);
    req.n_ssids = 1;
    assert(realtek_cfg80211_scan_ssid(&p, &req) == -EINVAL);
    req.ssids = &ssid; ssid.ssid_len = 33;
    assert(realtek_cfg80211_scan_ssid(&p, &req) == -EINVAL);
    assert(p.ss_ssidlen == 0);
    return 0;
}
"""
        with tempfile.TemporaryDirectory() as tmp:
            c = Path(tmp) / 'test.c'; binary = Path(tmp) / 'test'
            c.write_text(source)
            subprocess.run(['cc', '-std=c11', '-Werror', '-fsanitize=undefined',
                            str(c), '-o', str(binary)], check=True)
            subprocess.run([str(binary)], check=True)


if __name__ == '__main__':
    unittest.main()
