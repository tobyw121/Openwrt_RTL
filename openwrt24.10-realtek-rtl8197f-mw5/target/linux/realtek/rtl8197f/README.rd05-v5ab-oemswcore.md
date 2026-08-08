# RD05 v5ab OEM SWCORE mirror test

This build keeps the RTL8367D DSA side on external CPU port 7, but restores the
internal RTL865x/SWCORE LAN/CPU membership to match the working OEM firmware:

- VID9 member/untag mask: `0x11f` = ports 0..4 plus internal CPU/extension port 8
- FID0/FID1 broadcast L2 entries: member mask `0x11f`, CPU bit set
- PVID programming: ports 0..4 and internal port 8 set to VID9
- Netif0 ACL range: ingress ACL 0..4, egress ACL 253..253
- ACL table mirrors OEM order: source-filter permit, DHCPv4 to CPU, DHCPv6 to CPU,
  PPPoE discovery to CPU, final permit; no explicit ARP-to-CPU rule

The OEM UART dump showed ARP reaches the router through the broadcast L2 CPU path,
not through an explicit ARP ACL rule.

Expected boot markers:

- `txhdr=direct-ext-rxobserve-v5w-oemswcore-v5ab`
- `rd05 rtl865x l2cpu v5ab: ... mbr=0x11f`
- `rd05 rtl865x pipe v5ab: ... mbr=0x11f`
- `daclrcr=0xfdfd0400` if the DACLRCR layout assumption is correct

Test with:

    rd05-netdiag arptest 45

Then run the PC-side static IP arping/ping test.
