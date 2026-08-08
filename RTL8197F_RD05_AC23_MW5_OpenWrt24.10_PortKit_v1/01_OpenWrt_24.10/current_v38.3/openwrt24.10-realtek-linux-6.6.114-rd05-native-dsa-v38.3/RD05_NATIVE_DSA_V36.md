# Xiaomi RD05 native DSA v36

V36 is the hardware-feedback update to the full SDK-native v35 port.

## Paired hardware result

The PC capture contained 118 frames, all sourced by the PC: 116 ARP requests
and two IPv4 broadcasts. No router frame and no EtherType 0x8899 response was
observed. The old `peer-traffic-observed` result was therefore a capture-side
false positive caused by counting local AF_PACKET outgoing frames.

RTL8367D saw LAN2 ingress and transmitted it on CPU7/EXT1, while RTL8197F
`CPURPDCR0`, descriptor ownership and all software RX counters did not move.
In the reverse direction CPU7 FCS/drop counters tracked router TX frames
one-for-one. This proves two independent physical-link symptoms:

- RTL8197F -> RTL8367D: frames reach CPU7 but fail FCS validation;
- RTL8367D -> RTL8197F: CPU7 emits frames but the P0 RX DMA ring sees none.

## Exact SDK corrections

`_dal_rtl8367d_setAsicPortExtMode(2, RGMII)` clears register `0x03f7` bit 2
before configuring EXT1. Earlier revisions omitted this operation while also
skipping switch software reset, so inherited bootloader mode could remain.
V36 adds the clear at initial setup and every runtime reapply.

RTL8367D EXT1 RGMII timing is controlled through register `0x1307`; the
RTL8367B-family `0x13f9` timing field is no longer modified. The SDK contains
both an SSC-off TX0/RX2 fallback and an SSC-on then TX0/RX5 production
sequence. V36 defaults to the conservative non-SSC fallback and provides
bounded runtime calibration for SoC TX/RX, switch TX/RX and SSC.

The RTL8197F transmit CRC convention was also checked against
`rtl819x_swNic.c`: packets shorter than 60 bytes use descriptor length 64,
other packets use payload length plus four, `CPUICR.EXCLUDE_CRC` remains clear,
and hardware generates FCS. V36 retains this exact SDK convention rather than
introducing an unverified software-CRC workaround.

## Runtime calibration

Router-to-switch calibration does not require a PC:

```sh
rd05-v36-check
rd05-rgmii-calibrate tx-auto save
```

For switch-to-SoC calibration, start continuous traffic on the Linux PC first:

```sh
sudo ./tools/rd05-pc-netdiag.py \
  --interface enx00e04c5562c0 \
  --configure-ip \
  --calibration \
  --duration 90
```

At the same time on the router:

```sh
rd05-rgmii-calibrate rx-auto save
rd05-v36-check
rd05-netdiag full
```

The saved `/etc/rd05-rgmii.conf` is loaded before OpenWrt starts networking.
Compatibility commands `rd05-v34-check` and `rd05-v35-check` forward to the
v36 checker.
