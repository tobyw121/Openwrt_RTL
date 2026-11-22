# R76 full-analysis integration, 2026-11-22

Source: user-supplied Google Drive archive ID1_Et71Xe9PTZ5mRxyCkkKtsLUwFHv5eGF.
Archive SHA2568004ef029ced5afe41ba79e750dd0fa35c265b35be2b6f7ef764cc0b10a84bc3.
Flash SHA256f50f3147274a82fcbf4fef7a0c7500aeb7868b60e4e17975faea588237abd188;
kernel SHA2567d7e41a50d6659b5c84c524205bcca25306e2e98a5949c61032810ff84fd003e.
Both binary hashes were independently checked. Reconstructed code is not
original manufacturer C. The symbolized ELF is an analysis artifact only.

## Integration decisions

| Evidence | Action |
|---|---|
| Active NEW DMA path verified by netdev/tasklet registration | Retain32-byte R76 descriptors and single allocated TX ring0. Preserve prior CDP+OWN lifetime guard. |
| Checked reference normalizes physical/KSEG aliases | R76 TX CDP validation normalizes aliases, rejects zero geometry, misalignment and outside-ring addresses. Other boards unchanged. |
| NEW RX decodes metadata across six DWORDs | Separate R76 bounded double-snapshot guard before using address/length/source/VLAN. Retry on changing metadata without consuming/rearming an uncertain slot. |
| DW0 init vs completed RX bits differ | Retain Linux clean rearm, full buffer size and DMA API; do not blindly copy0x06a4000d or OEM1536-byte cache operation. |
| OEM NEW TX completion uses CDP without OWN test | Document distinction: our additional OWN/producer guard is a Linux safety policy, not a claim of instruction equivalence. |
| OEM uses old cfg80211 generic netdevice calls | G6 patch105 uses Linux6.6 cfg80211 register/unregister helpers when the current task is in add/del callbacks with RTNL+wiphy mutex already held. Probe/remove and RTNL-only legacy rename keep ordinary APIs. Monitor teardown uses the same context helper. |
| WLAN WD checksum XOR and lengths16/32/56 verified | Patch106 replaces potentially unaligned native loads/stores with explicit unaligned LE accesses, preserving the checksum-field semantics including short command descriptors. |
| RFE50 dump files | All16 installed active files already match byte-for-byte; no replacement or per-device calibration invented. |
| Separate rtl8192cd/8197G and rtk_wifi6/8832BR modules | Preserve separate families. A reconstructed RX decoder/power interpreter does not supply a complete Linux6.6 integrated2.4GHz driver. |
| Switch EXT1=SDK17 maps physical7; LAN order3/0/1, WAN2 | Existing R76 DT already matches. No port renumbering. |
| Kernel OEM ALL/4-byte CPU tags vs our DSA8-byte tags | Keep Linux DSA-compatible format. Do not copy OEM CPU_CTRL2b9 into a different tagger. |
| OEM external VID8/9 and internal VID1/2, plus runtime changes | Do not replace proven current Linux VLAN/DSA isolation from one static table. Firewall/bridge continue owning forwarding. |
| C-family, SerDes/fiber tables present but unused on D/RGMII | Do not apply unrelated analog patches or turn on OEM acceleration/shortcut hooks. |

Selected reports, original instruction disassemblies and reconstructed reference
functions are stored under OEM_referenz/rekonstruktion/cudy-r76-20261122 with
per-file SHA256 and provenance. No full firmware, kernel modules, private RootFS
or automatic Ghidra code is added to the source/build.

## Validation

- Reran original-MIPS Ethernet reference comparison:3591 cases PASS.
- Reran WLAN reference comparisons:3000 descriptor/RX cases and200 power cases
  (20 intentional timeouts) PASS using unicorn2.0.1.post1; no hardware execution.
- Production port tests compile actual helper bodies with UBSan:2048 checksum
  inputs at unaligned addresses; callback/probe/legacy-RTNL/foreign-owner
  selection and error propagation; R76 CDP aliases/bounds; changing/owned RX
  descriptor snapshots.
- All7 G6 patches apply to a fresh OEM source copy with fuzz0. Related build,
  RFE, switch, MW5-unchanged, RX/DMA and TX regression tests pass.
- Full MIPS firmware compilation and real-router load/radio tests are not
  available here. No claim that TCP stalls or all WLAN problems are resolved.

MW5-specific functions remain byte-identical to the prior baseline, and new
Ethernet behavior is explicitly selected only for R76. RD05 behavior unchanged.
WLAN package release7, Ethernet driver1.8.50-r76-oem-verified. The WLAN module
remains manual-probe until hardware validation; package selection remains default.

## PC build

```
cd ~/Schreibtisch/openwrt_git_RTL/Openwrt_RTL
git pull --ff-only origin MW5_Stable
# This replaces the existing build configuration with the AX1500 preset.
cp configs/rtl8197f_ax1500_defconfig .config
make defconfig
make target/linux/clean
make -j"$(nproc)" V=s
```

## Router UART38400, after flashing, individually

```
ethtool -i eth0
cat /proc/cudy-r76-rtknet
ethtool -S wan
```

Ethernet check first with WLAN disabled. On the PC:

```
sudo ip link set dev enx00e04c5562c0 up
sudo ip addr replace 192.168.1.3/24 dev enx00e04c5562c0
ping -I 192.168.1.3 -c 5 192.168.1.1
for n in $(seq 1 20); do curl --interface 192.168.1.3 -I --max-time 5 http://192.168.1.1/; done
```

Repeat under the previously failing WAN load and collect router counters again.
Separately test WLAN from UART:

```
modprobe rtk_wifi6
iw dev
logread | tail -80
```

After adding/removing an AP interface through LuCI, check HTTP/SSH responsiveness
and capture logread/UART. Do not run wifi status when sharing logs: it may expose
configured keys. If a hang occurs, UART logs and blocked-task stacks are needed;
the new changes address identified issues, not all possible OEM porting faults.
