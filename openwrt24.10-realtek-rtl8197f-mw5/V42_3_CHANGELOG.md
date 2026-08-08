# RTL8197F OpenWrt 24.10 v42.3

## MW5 rtl8192cd modpost correction

- Fixes `ERROR: modpost: "sha256_prf" ... undefined`.
- `sha256.o` was already present in `rtl8192cd-objs`, but its complete
  implementation was hidden behind vendor feature macros.
- `8192cd_psk.c` receives `CONFIG_IEEE80211W` through `8192cd_cfg.h`; the
  intentionally self-contained `sha256.c` does not include that private header
  graph, so the old conditional produced an empty object.
- SHA-256, HMAC-SHA256, `sha256_prf_bits()` and `sha256_prf()` now compile
  unconditionally inside the module.
- No external Linux symbol and no hostapd symbol is used.
- Package release raised from 5 to 6.

## Validation

- Host compilation confirms the old source emitted no symbols and the corrected
  source emits the complete local SHA-256/PRF symbol set.
- Known-answer tests cover SHA-256(`abc`) and IEEE-style SHA256 PRF output.
- Existing board, patch, WLAN-variant and private full-flash checks remain
  unchanged and pass.
- A real OpenWrt MIPS build is still required to reveal any later vendor-code
  blocker after modpost.
