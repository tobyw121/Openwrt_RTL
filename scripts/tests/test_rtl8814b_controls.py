#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Run the actual SDK duty-cycle parser with a fake firmware command boundary."""
from pathlib import Path
import subprocess
import tempfile
import unittest

from test_mw5_stability import WLAN, c_function


class DutyCycleTests(unittest.TestCase):
    def test_valid_malformed_and_oversized_controls(self):
        function = c_function(WLAN / "8192cd_hw.c", "void set_pwr_reduce(")
        program = r'''
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef unsigned char u1Byte;
struct shared { struct { int mp_specific; } rf_ft_var; };
struct rtl8192cd_priv { struct shared *pshare; };
static unsigned calls;
static unsigned last[5];
static void send_command(struct rtl8192cd_priv *p, unsigned char en,
                         unsigned char force, unsigned char duty,
                         unsigned char count, unsigned char location)
{ (void)p; calls++; last[0]=en; last[1]=force; last[2]=duty; last[3]=count; last[4]=location; }
struct hal { void (*SetPWRReduceHandler)(struct rtl8192cd_priv *,
    unsigned char,unsigned char,unsigned char,unsigned char,unsigned char); };
static struct hal ops={send_command};
#define GET_HAL_INTERFACE(p) (&ops)
#define DEBUG_ERR(...) ((void)0)
static int _atoi(char *p, int base) { return strtol(p,NULL,base); }
'''
        program += function + r'''
int main(void) {
    struct shared shared={0};
    struct rtl8192cd_priv p={&shared};
    set_pwr_reduce(&p,(unsigned char *)"1,0,16,8,4");
    assert(calls==1 && last[0]==1 && last[1]==0 && last[2]==20 && last[3]==8 && last[4]==4);
    set_pwr_reduce(&p,(unsigned char *)"1,0,110,8,4");
    assert(calls==2 && last[2]==100);
    set_pwr_reduce(&p,(unsigned char *)"1,0,0,8,4");
    assert(calls==3 && last[2]==5);
    const char *bad[]={"1","1,0","1,0,20","1,0,20,8",",0,20,8,4", "1,,20,8,4"};
    for(unsigned i=0;i<sizeof(bad)/sizeof(bad[0]);i++)
        set_pwr_reduce(&p,(unsigned char *)bad[i]);
    assert(calls==3);
    set_pwr_reduce(&p,NULL);
    assert(calls==3);
    char long_field[128], input[1024];
    memset(long_field,'1',sizeof(long_field)-1);long_field[sizeof(long_field)-1]=0;
    for(unsigned i=0;i<4;i++) {
        snprintf(input,sizeof(input),"%s,%s,%s,%s,4", i==0?long_field:"1",
                 i==1?long_field:"0",i==2?long_field:"20",i==3?long_field:"8");
        set_pwr_reduce(&p,(unsigned char *)input);
        assert(calls==3);
    }
    shared.rf_ft_var.mp_specific=1;
    set_pwr_reduce(&p,(unsigned char *)"0,0,20,8,4");
    assert(calls==4 && last[0]==1 && last[1]==1 && last[2]==20 && last[3]==0 && last[4]==0);
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            source = Path(tmp) / "duty.c"
            binary = Path(tmp) / "duty"
            source.write_text(program)
            compiled = subprocess.run(["cc", "-std=c11", "-Wall", "-Wextra", "-Werror",
                                       str(source), "-o", str(binary)], text=True, capture_output=True)
            self.assertEqual(compiled.returncode, 0, compiled.stderr)
            subprocess.run([str(binary)], check=True, capture_output=True)


if __name__ == "__main__":
    unittest.main()
