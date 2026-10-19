#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Exercise the actual NVMEM MAC reader, including RD05 environment cells."""
from pathlib import Path
import subprocess
import tempfile
import unittest

from test_mw5_stability import WLAN, c_function


class BoardMacTests(unittest.TestCase):
    def test_binary_ascii_invalid_and_deferred_cells(self):
        function = c_function(WLAN / "linux6/rtl8192cd_board.c",
                              "static int rtl8192cd_linux6_read_mac_cell(")
        program = r'''
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
typedef unsigned char u8;
#define ETH_ALEN 6
#define EPROBE_DEFER 517
#define IS_ERR(p) ((uintptr_t)(p) >= (uintptr_t)-4095)
#define PTR_ERR(p) ((int)(intptr_t)(p))
#define ERR_PTR(e) ((void *)(intptr_t)(e))
struct device { int unused; };
struct nvmem_cell { int unused; };
static unsigned char input[32];
static size_t input_len;
static int get_error, read_error, frees;
static struct nvmem_cell cell;
static struct nvmem_cell *devm_nvmem_cell_get(struct device *d, const char *s)
{ (void)d; assert(!strcmp(s,"mac-address")); return get_error ? ERR_PTR(get_error) : &cell; }
static void *nvmem_cell_read(struct nvmem_cell *c, size_t *len)
{
    assert(c == &cell);
    if (read_error) return ERR_PTR(read_error);
    *len=input_len;
    void *p=malloc(input_len);
    memcpy(p,input,input_len);
    return p;
}
static void kfree(void *p) { frees++; free(p); }
static void ether_addr_copy(u8 *to, const void *from) { memcpy(to,from,6); }
static bool is_valid_ether_addr(const u8 *p)
{ return !(p[0] & 1) && (p[0]|p[1]|p[2]|p[3]|p[4]|p[5]); }
static bool mac_pton(const char *s, u8 *p)
{
    unsigned int b[6];
    assert(strlen(s)==17);
    if (sscanf(s,"%2x:%2x:%2x:%2x:%2x:%2x",b,b+1,b+2,b+3,b+4,b+5)!=6) return false;
    for(int i=0;i<6;i++) p[i]=b[i];
    return true;
}
'''
        program += function + r'''
int main(void)
{
    struct device dev = {0};
    u8 mac[6], expected[6]={2,0x11,0x22,0x33,0x44,0x55};
    input_len=6; memcpy(input,expected,6);
    assert(rtl8192cd_linux6_read_mac_cell(&dev,mac)==0);
    assert(!memcmp(mac,expected,6));
    input_len=17; memcpy(input,"02:11:22:33:44:55",17);
    assert(rtl8192cd_linux6_read_mac_cell(&dev,mac)==0);
    assert(!memcmp(mac,expected,6));
    input_len=18; input[17]=0;
    assert(rtl8192cd_linux6_read_mac_cell(&dev,mac)==0);
    input[17]='x';
    assert(rtl8192cd_linux6_read_mac_cell(&dev,mac)==-EINVAL);
    input_len=16;
    assert(rtl8192cd_linux6_read_mac_cell(&dev,mac)==-EINVAL);
    input_len=17; memcpy(input,"zz:11:22:33:44:55",17);
    assert(rtl8192cd_linux6_read_mac_cell(&dev,mac)==-EINVAL);
    memcpy(input,"03:11:22:33:44:55",17);
    assert(rtl8192cd_linux6_read_mac_cell(&dev,mac)==-EINVAL);
    memcpy(input,"00:00:00:00:00:00",17);
    assert(rtl8192cd_linux6_read_mac_cell(&dev,mac)==-EINVAL);
    assert(frees==8);
    get_error=-EPROBE_DEFER;
    assert(rtl8192cd_linux6_read_mac_cell(&dev,mac)==-EPROBE_DEFER);
    get_error=-ENOENT;
    assert(rtl8192cd_linux6_read_mac_cell(&dev,mac)==-ENOENT);
    get_error=0; read_error=-EIO;
    assert(rtl8192cd_linux6_read_mac_cell(&dev,mac)==-EIO);
    assert(frees==8);
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            source = Path(tmp) / "mac.c"
            binary = Path(tmp) / "mac"
            source.write_text(program)
            subprocess.run(["cc", "-std=c11", "-Wall", "-Wextra", "-Werror",
                            str(source), "-o", str(binary)], check=True, capture_output=True)
            subprocess.run([str(binary)], check=True, capture_output=True)


class RadioBandTests(unittest.TestCase):
    def test_physical_radio_families_expose_the_correct_band_and_ac_mode(self):
        functions = "\n".join(c_function(WLAN / "8192cd_cfg80211.c", name) for name in
                              ("static void rtk_get_band_capa(", "int is_support_ac("))
        program = r'''
#include <assert.h>
#include <stdbool.h>
typedef bool BOOLEAN;
enum { VERSION_8812E, VERSION_8814A, VERSION_8822B, VERSION_8812F,
       VERSION_8814B, VERSION_8192D, VERSION_8881A, VERSION_8197F };
struct rtl8192cd_priv { int version; };
#define GET_CHIP_VER(p) ((p)->version)
'''
        program += functions + r'''
int main(void) {
    struct rtl8192cd_priv p;
    BOOLEAN band2, band5;
    int ac_chips[]={VERSION_8812F,VERSION_8814B,VERSION_8822B};
    for(unsigned i=0;i<sizeof(ac_chips)/sizeof(ac_chips[0]);i++) {
        p.version=ac_chips[i];
        rtk_get_band_capa(&p,&band2,&band5);
        assert(!band2 && band5);
        assert(is_support_ac(&p));
    }
    p.version=VERSION_8197F;
    rtk_get_band_capa(&p,&band2,&band5);
    assert(band2 && !band5);
    assert(!is_support_ac(&p));
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            source = Path(tmp) / "bands.c"
            binary = Path(tmp) / "bands"
            source.write_text(program)
            subprocess.run(["cc", "-std=c11", "-Wall", "-Wextra", "-Werror",
                            str(source), "-o", str(binary)], check=True, capture_output=True)
            subprocess.run([str(binary)], check=True, capture_output=True)


class VariantConfigurationTests(unittest.TestCase):
    def test_standalone_header_does_not_override_linux6_chip_or_control_plane(self):
        for hal in ("CONFIG_WLAN_HAL_8822BE", "CONFIG_WLAN_HAL_8812FE",
                    "CONFIG_WLAN_HAL_8814BE"):
            with self.subTest(hal=hal):
                result = subprocess.run(
                    ["cc", "-E", "-dM", "-x", "c", "-DCONFIG_RTL8192CD_LINUX6_BUS=1",
                     "-D" + hal + "=1", "-include", str(WLAN / "config_8192cd.h"), "-"],
                    input="", text=True, capture_output=True, check=True)
                macros = {line.split()[1] for line in result.stdout.splitlines()}
                for candidate in ("CONFIG_WLAN_HAL_8822BE", "CONFIG_WLAN_HAL_8812FE",
                                  "CONFIG_WLAN_HAL_8814BE"):
                    self.assertEqual(candidate in macros, candidate == hal)
                self.assertNotIn("CONFIG_MW5_LEGACY_WEXT", macros)
                self.assertNotIn("CONFIG_MW5_OEM_WLAN_ORDER", macros)
                self.assertNotIn("CONFIG_WLAN_MACHAL_API_V0", macros)


if __name__ == "__main__":
    unittest.main()
