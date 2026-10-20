#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Run actual nl80211 guards and key deletion with mocked CAM boundaries."""
import subprocess
import tempfile
import re
from pathlib import Path
import unittest

from test_mw5_stability import WLAN, c_function


class KeyLifecycleTests(unittest.TestCase):
    def test_key_bounds_and_indexed_deletion(self):
        cfg = WLAN / '8192cd_cfg80211.c'
        vendor = WLAN / '8192cd_net80211.c'
        code = r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
typedef uint8_t u8;
typedef uint16_t u16;
#define CONFIG_IEEE80211W
#define HW_ENC_FOR_GROUP_CIPHER
#define FALSE 0
#define ETH_ALEN 6
#define BUILD_BUG_ON(c) _Static_assert(!(c), "invalid key backup capacity")
#define GKEY_ID_SECOND 2
#define WLAN_CIPHER_SUITE_WEP40 1
#define WLAN_CIPHER_SUITE_WEP104 2
#define WLAN_CIPHER_SUITE_TKIP 3
#define WLAN_CIPHER_SUITE_CCMP 4
#define WLAN_CIPHER_SUITE_AES_CMAC 5
static u8 BROADCAST_MAC[6]={255,255,255,255,255,255};
struct key_params { const u8 *key; int key_len; unsigned cipher; };
struct ieee80211req_del_key { unsigned idk_keyix; u8 idk_macaddr[6]; };
ACTUAL_KEY_STRUCT
struct Dot11EncryptKey { unsigned dot11TTKeyLen, dot11TMicKeyLen; u8 bytes[96]; };
struct Dot11KeyMappingsEntry {
    unsigned dot11Privacy, keyInCam, keyid;
    struct Dot11EncryptKey dot11EncryptKey, dot11EncryptKey2;
};
struct stat_info { struct Dot11KeyMappingsEntry dot11KeyMapping; };
struct mib {
    struct Dot11KeyMappingsEntry dot11GroupKeysTable, dot11IGTKTable;
    struct { struct { u8 bytes[16]; } keytype[4]; } dot11DefaultKeysTable;
};
struct shared { unsigned CamEntryOccupied; };
struct rtl8192cd_priv {
    int *dev; struct mib *pmib; struct shared *pshare;
    struct { struct { void *pointer; } data; } group_key_cache;
    u8 wk_bk[BACKUP_BYTES];
};
static struct stat_info peer;
static bool peer_exists=true;
static unsigned cam_calls, last_index;
static int is_broadcast_ether_addr(const u8 *a) { return !memcmp(a,BROADCAST_MAC,6); }
static int is_valid_ether_addr(const u8 *a)
{ static const u8 zero[6]; return !(a[0]&1) && memcmp(a,zero,6); }
static int netif_running(int *dev) { return *dev; }
static struct stat_info *get_stainfo(struct rtl8192cd_priv *p, u8 *addr)
{ (void)p; assert(is_valid_ether_addr(addr)); return peer_exists ? &peer : 0; }
static int CamDeleteOneEntry(struct rtl8192cd_priv *p, u8 *a, unsigned i, unsigned d)
{ (void)p; (void)a; assert(!d); cam_calls++; last_index=i; return 1; }
'''
        compat = (WLAN / 'rtl8192cd_net80211_compat.h').read_text()
        request = re.search(r'struct ieee80211req_key \{.*?\n};', compat, re.S)[0]
        size = re.search(r'u8 wk_bk\[(\d+)\]', (WLAN / '8192cd.h').read_text())[1]
        code = code.replace('ACTUAL_KEY_STRUCT', request).replace('BACKUP_BYTES', size)
        code += c_function(cfg, 'static int realtek_cfg80211_key_id(') + '\n'
        code += c_function(cfg, 'static int realtek_cfg80211_key_params(') + '\n'
        code += c_function(cfg, 'static void realtek_cfg80211_cache_group_key(') + '\n'
        code += c_function(vendor, 'static int rtl_net80211_delkey_linux6(') + '\n'
        code += r'''
int main(void)
{
    u8 addr[6]={2,0,0,0,0,1}, zero[6]={0}, bytes[64]={0};
    assert(!realtek_cfg80211_key_id(-1,0,true,addr));
    assert(!realtek_cfg80211_key_id(0,0,true,addr));
    assert(realtek_cfg80211_key_id(1,0,true,addr)==-EOPNOTSUPP);
    assert(realtek_cfg80211_key_id(-2,0,true,addr)==-EOPNOTSUPP);
    assert(realtek_cfg80211_key_id(-1,0,true,0)==-EINVAL);
    assert(realtek_cfg80211_key_id(-1,0,true,zero)==-EINVAL);
    assert(realtek_cfg80211_key_id(-1,0,true,BROADCAST_MAC)==-EINVAL);
    assert(realtek_cfg80211_key_id(-1,1,true,addr)==-EINVAL);
    assert(realtek_cfg80211_key_id(-1,6,false,0)==-EINVAL);
    assert(!realtek_cfg80211_key_id(-1,5,false,0));
    unsigned lengths[]={0,5,13,32,16,16};
    for(unsigned cipher=1;cipher<=5;cipher++) {
        for(int n=-1;n<=65;n++) {
            struct key_params params={bytes,n,cipher};
            assert((realtek_cfg80211_key_params(cipher==5 ? 4 : 1,false,&params)==0)
                   == (n==(int)lengths[cipher]));
        }
    }
    struct key_params params={bytes,16,WLAN_CIPHER_SUITE_CCMP};
    assert(realtek_cfg80211_key_params(4,false,&params)==-EINVAL);
    assert(realtek_cfg80211_key_params(0,false,0)==-EINVAL);
    params.key=0;
    assert(realtek_cfg80211_key_params(0,false,&params)==-EINVAL);
    params.key=bytes;params.cipher=WLAN_CIPHER_SUITE_AES_CMAC;
    assert(realtek_cfg80211_key_params(4,true,&params)==-EINVAL);
    assert(realtek_cfg80211_key_params(3,false,&params)==-EINVAL);

    struct mib mib={0}; struct shared shared={2}; int running=1;
    struct rtl8192cd_priv priv={.dev=&running,.pmib=&mib,.pshare=&shared};
    struct ieee80211req_key request={.ik_type=3,.ik_keyix=2,.ik_keylen=16};
    memset(request.ik_keydata,0x42,sizeof(request.ik_keydata));
    assert(sizeof(request)>64);
    realtek_cfg80211_cache_group_key(&priv,&request);
    assert(priv.group_key_cache.data.pointer==priv.wk_bk);
    struct ieee80211req_key saved=request;
    memset(&request,0,sizeof(request));
    assert(!memcmp(priv.wk_bk,&saved,sizeof(saved)));
    struct ieee80211req_del_key wk={0,{0}};
    assert(rtl_net80211_delkey_linux6(&priv,&wk)==-EINVAL && !cam_calls);
    memcpy(wk.idk_macaddr,addr,6);
    peer_exists=false;
    assert(rtl_net80211_delkey_linux6(&priv,&wk)==-ENOENT && !cam_calls);
    peer_exists=true;peer.dot11KeyMapping.keyInCam=1;
    memset(&peer.dot11KeyMapping.dot11EncryptKey,0x55,sizeof(struct Dot11EncryptKey));
    assert(!rtl_net80211_delkey_linux6(&priv,&wk) && cam_calls==1);
    assert(shared.CamEntryOccupied==1 && !peer.dot11KeyMapping.keyInCam);
    struct Dot11EncryptKey empty={0};
    assert(!memcmp(&peer.dot11KeyMapping.dot11EncryptKey,&empty,sizeof(empty)));
    assert(!rtl_net80211_delkey_linux6(&priv,&wk) && cam_calls==1);

    memcpy(wk.idk_macaddr,BROADCAST_MAC,6);wk.idk_keyix=1;
    mib.dot11GroupKeysTable.keyid=2;mib.dot11GroupKeysTable.keyInCam=1;
    memset(&mib.dot11GroupKeysTable.dot11EncryptKey,0x55,sizeof(empty));
    memset(&mib.dot11GroupKeysTable.dot11EncryptKey2,0x66,sizeof(empty));
    struct Dot11EncryptKey replacement=mib.dot11GroupKeysTable.dot11EncryptKey2;
    memset(priv.wk_bk,0x77,sizeof(priv.wk_bk));
    assert(!rtl_net80211_delkey_linux6(&priv,&wk) && cam_calls==1);
    assert(!memcmp(&mib.dot11GroupKeysTable.dot11EncryptKey,&empty,sizeof(empty)));
    assert(!memcmp(&mib.dot11GroupKeysTable.dot11EncryptKey2,&replacement,sizeof(empty)));
    assert(priv.wk_bk[0]==0x77 && mib.dot11GroupKeysTable.keyInCam);
    wk.idk_keyix=2;
    assert(!rtl_net80211_delkey_linux6(&priv,&wk) && cam_calls==2 && last_index==2);
    assert(!shared.CamEntryOccupied && !mib.dot11GroupKeysTable.keyInCam);
    assert(!memcmp(&mib.dot11GroupKeysTable.dot11EncryptKey2,&empty,sizeof(empty)));
    for(unsigned i=0;i<sizeof(priv.wk_bk);i++) assert(!priv.wk_bk[i]);
    assert(!priv.group_key_cache.data.pointer);
    /* Deleting GTK2 after GTK1 replacement must preserve the shared CAM. */
    mib.dot11GroupKeysTable.keyid=1;mib.dot11GroupKeysTable.keyInCam=1;
    mib.dot11GroupKeysTable.dot11EncryptKey=replacement;
    assert(!rtl_net80211_delkey_linux6(&priv,&wk) && cam_calls==2);
    assert(!memcmp(&mib.dot11GroupKeysTable.dot11EncryptKey,&replacement,sizeof(empty)));
    /* A stale IGTK deletion cannot clear its replacement or the data GTK. */
    mib.dot11IGTKTable.keyid=5;mib.dot11IGTKTable.dot11EncryptKey=replacement;
    wk.idk_keyix=4;
    assert(!rtl_net80211_delkey_linux6(&priv,&wk));
    assert(!memcmp(&mib.dot11IGTKTable.dot11EncryptKey,&replacement,sizeof(empty)));
    wk.idk_keyix=5;
    assert(!rtl_net80211_delkey_linux6(&priv,&wk));
    assert(!memcmp(&mib.dot11IGTKTable.dot11EncryptKey,&empty,sizeof(empty)));
    assert(cam_calls==2 && mib.dot11GroupKeysTable.keyInCam);
    wk.idk_keyix=6;
    assert(rtl_net80211_delkey_linux6(&priv,&wk)==-EINVAL);
    running=0;
    assert(rtl_net80211_delkey_linux6(&priv,&wk)==-ENETDOWN);
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            src = Path(tmp) / 'keys.c'
            exe = Path(tmp) / 'keys'
            src.write_text(code)
            subprocess.run(['cc', '-std=gnu11', '-Wall', '-Wextra', '-Werror',
                            '-fsanitize=undefined', '-fno-sanitize-recover=all',
                            str(src), '-o', str(exe)], check=True)
            subprocess.run([str(exe)], check=True)


if __name__ == '__main__':
    unittest.main()
