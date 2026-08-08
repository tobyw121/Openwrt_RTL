# v41.1 changes

- Full-SPI image generation is enabled by default for RD05, MW5 and AC23.
- Added board-matched private templates under `fullflash-templates/`.
- Added strict size, SHA-256, model-marker and signature verification.
- Unified output suffix: `squashfs-spi-full.bin` for all three devices.
- Added SHA-256 and manifest sidecars for every full-flash output.
- Added `BUILD_ALL_FULLFLASH_V41_1.sh` and automatic template checks to the
  existing RTL8197F build helper.
- RD05 builder preserves boot/factory data and validates `model=RD05`.
- MW5 builder retains its structural firmware and model validation.
- AC23 builder validates Tenda/Lynx factory markers and preserves both config copies.
- Host tools, toolchain, feeds and base packages remain source-only and are rebuilt locally.
