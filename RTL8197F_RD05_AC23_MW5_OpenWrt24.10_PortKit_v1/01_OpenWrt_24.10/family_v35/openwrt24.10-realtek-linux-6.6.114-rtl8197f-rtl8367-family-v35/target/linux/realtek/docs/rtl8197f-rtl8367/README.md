# RTL8197F + RTL8367 family support (v35)

This directory documents the RTL8197/RTL8367 sources analyzed from the supplied
SDK archive and the Linux 6.6.114/OpenWrt 24.10 implementation in this tree.

## Delivered implementation

- Linux DSA profiles for known RTL8367 **C-map** and **D-map** chip IDs.
- Exact RTL8367D / RTL8367RB-VC `0x6642:0x0030` profile.
- Exact existing C-map profiles `0x6367:0x0020`, `0x0040`, and `0x00a0`.
- Runtime-warning family profiles for IDs `0x0276`, `0x0597`, other `0x6367`
  revisions, and other `0x6642` revisions.
- Correct C/D SSC enable and disable sequences.
- Separate generic C/RB-VB and D/RB-VC DTS templates.
- RTL8197F NIC external-master modes for DSA and legacy swconfig.
- Board-configurable RTL8197F P0 TX/RX/RGTXC timing.
- Optional build coverage for the old OpenWrt `rtl8367`/`rtl8367b` swconfig
  modules, retained for B-map chips with revision-specific initialization tables.

## Support levels

`exact` means the chip ID and revision have a dedicated profile. `family` means
that the SDK register family is known, but hardware validation and possibly a
revision-specific initialization table are still required. `legacy` means the
old swconfig driver is the safer implementation. `inventory-only` means source
was found but no safe Linux 6.6 profile was inferred.

No source file from the vendor SDK is copied into the kernel. The TSV files are
derived symbol/value indices and source inventories. This avoids silently mixing
unknown vendor licensing with GPL kernel code.

Start with:

1. `RTL8367_SUPPORT_STATUS.md`
2. `RTL8367_VARIANT_MATRIX.tsv`
3. `RTL8367_REGISTER_MAP_SUMMARY.md`
4. `RTL8197_SOC_AND_NIC_SUMMARY.md`
5. `register-maps/*.tsv`
