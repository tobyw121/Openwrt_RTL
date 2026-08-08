# MW5 v43.6 tagger format build correction

v43.6 is a focused compile correction on top of v43.5.

## Cause

The v43.5 TX trace passed `BIT(dp->index)` directly to a `%x` format field.
`BIT()` is an `unsigned long`, while `%x` requires `unsigned int`. The OpenWrt
kernel build uses global `-Werror`, so GCC promotes the format warning to a
fatal error while building the `net` subtree.

## Correction

- Store the destination mask in `u32 port_mask`.
- Use `port_mask` for `FIELD_PREP()` and the trace message.
- Keep the v43.5 RX/MAC/diagnostic logic unchanged.
- Update runtime/version markers to v43.6.
- Add a regression check rejecting direct `BIT()` use in the `%x` trace.

LAN remains RTL8367 port 1 and WAN remains RTL8367 port 3.
