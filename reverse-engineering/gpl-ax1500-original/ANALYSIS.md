# Original Google Drive AX1500 GPL review — 2026-11-20

Downloaded the supplied Google Drive file (ID1sqW7x8sLHnSitW9ZXxgsrVN3lprFLp45).
Archive root: `GPL_for_AX1500_rtl8197/openwrt/21.02`. SHA256 and selected
source-file hashes are recorded in source-manifest.json. This is distinct
from the TP-Link AX1500v3 archive used to build our G6 WLAN module.
Archive retained locally at /tmp/original-gpl.download; extracted source at
/workspace/gpl-original. Neither external SDK nor archive is committed.

## Switch evidence and implemented changes

`dal_rtl8367d_vlan.c` confirms:
- word0 low8 membership, high8 untag; word1 bits0..3 FID/IVL/SVL controls;
- VID range0..4095, physical member mask8 bits;
- CVLAN read operation, address selection, busy polling and read-data registers;
- port PVID is a VID, not an index into a member-configuration table.
`rtl8367d_reg.h` confirms BUSY0x0502[13], read-data0/1=0x0520/0x0521.

The R76 writer now checks input bounds and waits BEFORE staging table data.
It reads the just-written VID through the documented CVLAN read command and
verifies membership, untag mask and FID/IVL bits. I/O failure, timeout or
mismatch aborts setup rather than accepting unverified WAN/LAN configuration.
Existing unconditional commands, port isolation, OEM PVID1/2, ingress policy,
RD05/MW5 scopes and firewall remain unchanged. This is not a proven fix for
WAN traffic; actual switch hardware validation remains required.

## WLAN findings and limits

The extracted archive has no `g6_wifi_driver`, `rtk_wifi6` or `rtl8192cd`
source directory. It therefore does NOT supply replacement RTL8832BR HAL,
firmware/eFuse access, RFE loader or adapter timer teardown implementations.
Its rtlwifi patch `002-v5.13-rtlwifi-implement-set_tim-by-update-beacon-content`
is already present upstream in Linux6.6: `rtl_update_beacon_work_callback`
and `.set_tim = rtl_op_set_tim`. It targets the different rtlwifi driver
family (not our vendor G6 RTL8832BR module). Reapplying or transplanting it
would neither resolve the IRQ/timer failure nor establish WR1500 radio support.
No unsupported WLAN behavior or OEM acceleration bypass was imported.
The existing WR1500 module remains manual-probe while failures remain open.

## Validation

Production CVLAN functions run against a fake switch-table model with
separate staged/write/read state: consecutive VID1/2 commands, corrupt member
readback, incorrect IVL flags, individual I/O faults, busy timeout before any
write and invalid VID/mask. Updated the existing R76 setup model for mandatory
readback. New343 applies without fuzz after the prepared342 baseline; older
pre-existing patches need context fuzz against vanilla Linux instead of a
complete OpenWrt kernel preparation. No cross-compiler/configured build tree
was present in this replacement environment; no kernel/image build claimed.

## PC build on the normal development checkout

```sh
cd ~/Downloads/openwrt/github/Openwrt_RTL
git pull --ff-only origin MW5_Stable
cp configs/rtl8197f_ax1500_defconfig .config
make defconfig
make target/linux/clean
make download -j8
make -j"$(nproc)" V=s
```

Router after loading the new WR1500 image, individually via UART38400:

```sh
dmesg | grep -E 'R76|VID|CVLAN|failed'
ifstatus wan
ethtool -S wan
```

PC at LAN with Fritzbox at WAN:

```sh
sudo ip link set enx14ebb6300bb2 up
sudo ip addr replace 192.168.1.3/24 dev enx14ebb6300bb2
sudo arping -I enx14ebb6300bb2 -S 192.168.1.3 -c 10 192.168.1.1
ping -I 192.168.1.3 -c 10 192.168.1.1
```

Send bootlog especially any `VID verification failed` message and WAN status.
Do not enable diagnostic WAN filter bypass during the first comparison.
