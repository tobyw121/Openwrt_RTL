# RTL8197F OpenWrt 24.10 development tree v42.1

## WLAN build correction

- Fixed `WlanHAL/HalCommon.c`: chip-specific HCI selector variables are now
  unique for RTL8822B, RTL8822C, RTL8812F and RTL8821C. This removes the
  Linux 6.6/GNU11 `hci_type` redefinition that stopped `HalCommon.o`.
- Added a deterministic default of `BEAMFORMING_SUPPORT=0` when the vendor SDK
  does not define the optional feature.
- Replaced the duplicate `get_desc`/`set_desc` definitions explicitly with
  `#undef` before the vendor's PCI/HCI-specific definitions. The effective
  descriptor conversion behaviour is unchanged.
- Guarded the duplicate RTL8197F RF thermal-meter register definition.
- Removed whitespace following C macro continuation backslashes in the
  RTL8197F power-sequence table.
- Bumped `rtl8192cd-rtl8197f` package release from 3 to 4.

## Validation status

The exact compiler error supplied from the Linux 6.6.114 build is removed by
source inspection. The complete package must be rebuilt in the user's prepared
OpenWrt toolchain to expose the next vendor-port incompatibility, if any.
