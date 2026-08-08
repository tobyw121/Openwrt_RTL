# RTL8197F WLAN initialization phases (v42)

The recovered SDK driver remains in vendor order. Added telemetry records every
major stage through `dmesg` and the module parameters `init_phase`, `last_phase`
and `init_rc`. Phase numbers describe the most recent event, not a monotonically
increasing health score.

## Module and device creation

| Phase | Meaning |
|---:|---|
| 10 | module entry and profile parameters available |
| 20 | optional BSP GPIO hook table ready |
| 30 | legacy vendor core entered |
| 40 | integrated/PCI device table scan |
| 49 | external PCI registration deliberately skipped |
| 50/51 | PCI driver registration begin/result |
| 60/61 | integrated radio `init_one()` begin/result |
| 90 | module initialization returned zero |
| 99 | module initialization failed |
| 100 | root netdevice allocation entered |
| 109/110 | netdevice allocation failed/succeeded |
| 120 | private Wi-Fi MIB allocated |
| 130/131/139 | netdevice registration begin/success/failure |
| 190/199 | root device initialization complete/failed |

## Runtime open, IRQ and DMA

| Phase | Meaning |
|---:|---|
| 200 | root interface open entered |
| 210/211/219 | software state allocation begin/success/failure |
| 220/221/229 | IRQ request begin/success/failure |
| 230/231/239 | hardware initialization dispatch/success/failure |
| 240 | driver state marked open |
| 250 | TX queues started |
| 290/299 | root interface open complete/failed |

## HAL, firmware and RF

| Phase | Meaning |
|---:|---|
| 300 | `rtl8192cd_init_hw_PCI()` entered |
| 310/311/319 | power-on sequence begin/success/failure |
| 320/321/329 | HCI DMA register setup begin/success/failure |
| 330/331/339 | MAC setup begin/success/failure |
| 340/341/349 | firmware download begin/success/failure |
| 350/351/359 | BB/PHY and AGC tables begin/success/failure |
| 360/361/369 | RF tables begin/success/failure |
| 370/371 | LCK/IQK/board RF calibration begin/complete |
| 390 | hardware initialization complete |

## Manual user-space AP bring-up

`rtl8197f-wlan-ap-test` logs phases 400–490 while programming the recovered
private MIB ABI. It performs a dry run unless `--apply` is supplied. For MW5 the
OEM runtime directly supports this mapping:

- `wlan1`: integrated RTL8197FS, 2.4 GHz, RFE 5.
- `wlan0`: PCIe RTL8822B (`10ec:b822`), 5 GHz, RFE 6.
- 2.4 GHz B/G/N band mask: 11.
- 5 GHz A/N/AC band mask: 76.
- HT20/HT40/HT80 width values: `use40M=0/1/2`.
- OEM mixed WPA/WPA2 PSK values: `authtype=2`, `encmode=2`, `psk_enable=3`,
  `wpa_cipher=8`, `wpa2_cipher=8`.

Those settings reconstruct observed OEM behavior; they are not a production
security or regulatory policy. An explicit numeric `--regdomain` and serial
recovery access are required for an actual test.

A final phase 390 or 490 does not prove RF output quality, legal channel/power,
calibration accuracy, association stability or sustained throughput. Those
still require target hardware, a spectrum-safe setup and UART logs.
