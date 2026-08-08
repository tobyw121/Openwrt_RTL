# MW5 v43.5 RX/MAC/diagnostic correction

- Keeps the verified physical map: RTL8367 port 1 = LAN, port 3 = WAN.
- Replaces the master ndo_set_mac_address hook with a handler that reseeds the
  RTL865x VID/L2/netif/ACL tables whenever netifd installs the factory MAC.
- Makes /proc/rd05-rtknet readable and exposes MAC, ring, RX trace and TX state.
- Enables raw pre-DSA RX tracing on MW5 as well as RD05.
- Adds bounded protocol-9 TX/RX logs including the encapsulated EtherType.
- Preserves the upstream Linux EtherType-tagger strip sequence and explicitly
  records the encapsulated protocol after stripping.
- Extends mw5-netdiag with direct lan, direct wan and carrier-selected direct auto.
- Avoids errors when odhcpd or full iproute2 diagnostic options are absent.
