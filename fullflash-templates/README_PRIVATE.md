# PRIVATE full-flash templates

These three files are complete device SPI dumps and contain private per-device data:
MAC addresses, serial numbers, factory/RF calibration, configuration and possibly credentials.
Do not publish this directory or images built from it.

The image build is enabled by default for the RTL8197F device profiles. Each profile uses only
its matching fixed template path:

- `xiaomi-r4-rd05-spi.bin` (16 MiB)
- `tenda-nova-mw5-spi.bin` (8 MiB)
- `tenda-ac23-spi.bin` (8 MiB, Lynx factory markers)

Run `./VERIFY_FULLFLASH_TEMPLATES_V41_1.sh` before building.
Set `RTL8197F_BUILD_FULLFLASH=0` only when intentionally disabling full-flash outputs.
