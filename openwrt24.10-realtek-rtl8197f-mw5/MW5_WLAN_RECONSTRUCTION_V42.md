# Tenda Nova MW5 WLAN reconstruction notes (v42)

## Direct OEM evidence

The collected OEM `/proc/wlan*/mib_all` data resolves the runtime ordering more
reliably than the earlier board inventory:

| OEM interface | Silicon/path | Band | RFE | Observed configuration |
|---|---|---|---:|---|
| `wlan1` | integrated RTL8197FS MP-B | 2.4 GHz | 5 | channel 6, B/G/N, HT20 |
| `wlan0` | external RTL8822B MP PCIe | 5 GHz | 6 | channel 40, A/N/AC, HT80 |

The external PCI evidence uses `10ec:b822`. A separate board list named
RTL8812BRH, but it is weaker evidence and is retained only as a conflict note.
The module can still be loaded integrated-only with
`rtl8197f-wlan-load`; `--dual` enables PCI registration.

## Reconstructed initialization order

1. Select board/bond profile (`BSP_BOND_97FS`).
2. Create root netdevice and private MIB.
3. Map integrated MMIO or external PCI BAR/IRQ.
4. Associate chip-specific HAL and HALMAC functions.
5. Execute power-on sequence.
6. Initialize HCI DMA registers and software rings.
7. Initialize MAC registers.
8. Download chip firmware.
9. Apply MAC/BB/AGC parameter tables.
10. Apply RF path and synthesizer tables.
11. Load EFUSE/RFE and board power data.
12. Run LCK/IQK and chip-specific RF calibration.
13. Register IRQ/netdevice and open TX/RX queues.
14. Apply private MIB AP configuration in user space.

The C driver now emits phase markers for each major stage. The manual AP helper
replays the observed private-MIB order without automatically enabling RF.

## First controlled tests

```sh
# Inventory only
rtl8197f-wlan-load --probe

# Integrated RTL8197FS only
rtl8197f-wlan-load
rtl8197f-wlan-check /tmp/mw5-wlan-integrated.txt

# Print, but do not apply, the OEM-like 2.4 GHz AP sequence
rtl8197f-wlan-ap-test --profile mw5 --band 2g \
  --ssid MW5-LAB --regdomain 1 --open --dry-run

# Actual application is deliberately explicit
rtl8197f-wlan-ap-test --profile mw5 --band 2g \
  --ssid MW5-LAB --regdomain 1 --open --apply
```

The numeric regulatory domain above is only an example matching the recovered
private ABI. Use the value appropriate to the tested firmware and jurisdiction.
Do not test RF without valid calibration data and a controlled setup.

## Remaining blockers

- Exact RTL8363NB cold-start table and stable Ethernet path.
- MIPS/Linux 6.6 module compilation in the actual OpenWrt toolchain.
- Target verification of MMIO, IRQ, coherent DMA and EFUSE reads.
- Association, encryption, calibration and sustained load tests.
- Confirmation that the specific physical MW5 unit also enumerates `10ec:b822`.


## v42.3 SHA-256/PRF linkage correction

The MW5 WPA/PMF path compiles `8192cd_psk.c` with `CONFIG_IEEE80211W` from
`8192cd_cfg.h`.  The isolated `sha256.c` intentionally avoids that private
header graph, so its legacy feature guard previously emitted an empty
`sha256.o`.  v42.3 always compiles the local SHA-256, HMAC-SHA256 and
`sha256_prf` implementation because that object is always part of
`rtl8192cd.o`.  This resolves the Linux 6.6 `modpost` undefined symbol without
requiring or exporting a kernel-global crypto helper.
