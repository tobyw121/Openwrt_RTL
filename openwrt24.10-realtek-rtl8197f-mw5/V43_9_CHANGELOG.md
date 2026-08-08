# v43.9 - MW5 asymmetric RX CPU tag and DMA completion correction

## Hardware evidence addressed

- v43.8 captured a real RTL8367 -> RTL8197F frame as `88 99 04 01` on physical
  LAN port 1.
- v43.8 incorrectly treated that RX tag as the TX protocol-9 layout and dropped
  it as `unknown protocol=0x0`.
- CPU -> switch TX was independently captured immediately before DMA as
  `88 99 92 02` for LAN port 1, proving directional tag-word asymmetry.
- v43.8 still accumulated high RX `bad_desc`/`oversize_drop` counts and exposed
  descriptor-looking words that resemble packet payload.
- user-port TX remained zero even though the master submitted correctly tagged
  frames, so TX completion needs instrumentation after DMA mapping.

## DSA tagger

- Split CPU-tag decoding by direction.
- TX keeps protocol 9, disable-learning and a six-bit one-hot destination mask.
- RX recognizes the observed switch-to-CPU `0x0400` marker and obtains a direct
  source-port index from bits [5:0].
- The proven LAN tag `0x0401` now resolves to DSA port 1.
- A protocol-9 RX compatibility path remains for older/synthetic inputs.
- WAN whole-tag value `0x0403` is not claimed as hardware-proven by the v43.8
  capture; the decoder handles the source-port field generically.

## RTL8197F master

- Driver version is `1.4.9-sdk-mw5-rxtag-ring-v43.9`.
- Real `0x8899` RX frames pass through to DSA unchanged.
- Descriptor fallback, used only when 0x8899 is absent, now synthesizes the
  switch-to-CPU `0x0400 | source-port` layout instead of TX protocol 9.
- Descriptor fallback remains limited to proven source ports 1 and 3.
- MW5 RX completion now requires OWN to be cleared; an advanced CPURPDCR0 no
  longer overrides hardware ownership.
- RX recycle rewrites every descriptor address from `rx_dma[idx]` before OWN is
  returned to hardware.
- Bounded RX descriptor snapshots report descriptor DMA address, CDP, OWN,
  actual/expected buffer address, opts1..opts5 and stride.
- TX submit and completion traces report mapped DMA/descriptor state and
  CPUTPDCR0 progression.
- `/proc/rd05-rtknet` adds MW5 OWN-wait/address-mismatch/descriptor-trace and TX
  submit/completion counters.

## Diagnostics

- `mw5-netdiag` reports v43.9 markers.
- The one-command autotest waits for normal netifd LAN/br-lan readiness before
  taking its original-state snapshot.
- All router-side diagnostics remain built into the image.

## Firewall

- Standard OpenWrt firewall4 policy is unchanged from v43.8.
- No nftables flush and no firewall service disable are performed by
  `mw5-netdiag`.
- The v43.8 migration helper from v43.7 no-firewall images is retained.

## Validation boundary

Static source/regression checks and patch reproducibility are performed before
packaging. A complete MIPS cross-build and real-hardware v43.9 network test are
not claimed unless performed separately.
