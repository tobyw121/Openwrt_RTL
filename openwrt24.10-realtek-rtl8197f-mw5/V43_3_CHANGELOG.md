# v43.3 MW5 TX CPU-tag fix

## Runtime evidence from v43.2

- RTL8367 protocol-9 RX tags are decoded correctly.
- RX DMA works: `eth0` and `lan` receive counters increase.
- `br-lan` emits replies, but the LAN user port does not transmit them.

## Root cause

The RTL8197F vendor stack enables `CFG_TX_CPUC_TAG` because vendor packets are
submitted without an in-band CPU tag; P0 creates the tag from the vendor
rtl865x TX path. Linux DSA `tag_rtl4_9` already inserts the complete four-byte
CPU tag into the skb. Enabling both mechanisms makes the SoC TX path process the
same tag twice before the packet reaches the external RTL8367.

## Changes

- Keep `CFG_CPUC_TAG`/P0 CPU-tag RX parsing enabled on MW5.
- Disable `CFG_TX_CPUC_TAG`/hardware TX tag generation on MW5 only.
- Preserve RD05 behaviour.
- Set protocol-9 disable-learning bit 9 on CPU-to-switch frames.
- Bump MW5 runtime markers to v43.3.

Expected boot marker:

```
mw5 p0 rgmii sdk: ... cputag-rx=1 cputag-tx=0 ... tag=rtl4_9/4byte
```
