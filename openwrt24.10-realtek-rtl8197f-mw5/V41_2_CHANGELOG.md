# v41.2 changes

- Fixed fresh-tree build order: host tools and MIPS cross-toolchain are now
  installed before `target/linux/compile`.
- Added `BOOTSTRAP_BUILD_ENV_V41_2.sh` with explicit validation of OpenWrt host
  M4 and `mipsel-openwrt-linux-musl-gcc`.
- Added targeted recovery for an incomplete/broken `staging_dir/host/bin/m4`.
- Added corrected `BUILD_ALL_FULLFLASH_V41_2.sh` and
  `BUILD_RTL8197F_V41_2.sh`.
- Kept v41/v41.1 script names as wrappers to the corrected v41.2 helpers.
- Preserved automatic board-specific RD05, MW5 and AC23 full-flash generation.
- No generated `staging_dir`, `build_dir`, feeds or host binaries are shipped.
