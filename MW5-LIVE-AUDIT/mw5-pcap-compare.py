#!/usr/bin/env python3
"""Compare the two physical MW5 audit PCAPs without third-party modules.

The MW5 tests track three failure signatures seen on 2026-09-20:
  * the same CPU-egress frame appearing on both LAN and WAN physical cables;
  * VLAN0 frames whose Ethernet header is made from printable payload bytes;
  * v44.66.8 priority VLAN0 inserted *before* the Realtek rtl8_4 CPU tag;
  * v44.66.10 residual priority VID0 emitted after rtl8_4 was consumed.

This tool is diagnostic only.  It does not try to infer unknown RTL8367
registers or modify the router.
"""
from __future__ import annotations

import argparse
import bisect
import hashlib
import json
import struct
from collections import Counter, defaultdict
from pathlib import Path

KNOWN_ETHERTYPES = {0x0800, 0x0806, 0x86DD, 0x88CC, 0x888E, 0x8899, 0x8863, 0x8864}


def read_pcap(path: Path):
    raw = path.read_bytes()
    if len(raw) < 24:
        raise ValueError(f"{path}: truncated pcap header")
    magic = raw[:4]
    if magic == b"\xd4\xc3\xb2\xa1":
        endian, scale = "<", 1e6
    elif magic == b"\xa1\xb2\xc3\xd4":
        endian, scale = ">", 1e6
    elif magic == b"\x4d\x3c\xb2\xa1":
        endian, scale = "<", 1e9
    elif magic == b"\xa1\xb2\x3c\x4d":
        endian, scale = ">", 1e9
    else:
        raise ValueError(f"{path}: unsupported pcap magic {magic.hex()}")
    _, _, _, _, _, snaplen, network = struct.unpack(endian + "IHHIIII", raw[:24])
    if network != 1:
        raise ValueError(f"{path}: only Ethernet/DLT_EN10MB is supported, got linktype={network}")
    off = 24
    packets = []
    while off + 16 <= len(raw):
        sec, frac, incl, orig = struct.unpack(endian + "IIII", raw[off:off + 16])
        off += 16
        if off + incl > len(raw):
            break
        pkt = raw[off:off + incl]
        off += incl
        packets.append((sec + frac / scale, pkt, orig))
    return snaplen, packets


def decode_l2(pkt: bytes):
    if len(pkt) < 14:
        return None
    outer = struct.unpack("!H", pkt[12:14])[0]
    vlan = None
    inner = outer
    if outer == 0x8100 and len(pkt) >= 18:
        tci = struct.unpack("!H", pkt[14:16])[0]
        vlan = tci & 0x0FFF
        inner = struct.unpack("!H", pkt[16:18])[0]
    return {
        "dst": pkt[0:6],
        "src": pkt[6:12],
        "outer": outer,
        "inner": inner,
        "vlan": vlan,
    }


def printable_ratio(buf: bytes) -> float:
    if not buf:
        return 0.0
    return sum(1 for b in buf if 0x20 <= b <= 0x7E) / len(buf)


def summarize(path: Path):
    snaplen, packets = read_pcap(path)
    counts = Counter()
    vids = Counter()
    ascii_header = []
    unknown_vlan0 = []
    vlan0_before_rtl8 = []
    vlan_before_rtl8 = []
    vlan_before_rtl8_vids = Counter()
    rtl8_on_user_wire = []
    priority_vlan0 = []
    for t, pkt, orig in packets:
        l2 = decode_l2(pkt)
        counts["packets"] += 1
        if not l2:
            counts["short"] += 1
            continue
        if l2["outer"] == 0x8100:
            counts["vlan"] += 1
            vids[str(l2["vlan"])] += 1
            if l2["vlan"] == 0:
                priority_vlan0.append((t, pkt, orig, l2))
        if l2["inner"] == 0x0800:
            counts["ipv4"] += 1
        elif l2["inner"] == 0x86DD:
            counts["ipv6"] += 1
        elif l2["inner"] == 0x0806:
            counts["arp"] += 1
        else:
            counts["other_ethertype"] += 1
        # v44.66.8/v44.66.9 hardware signature: RTL8197F SwitchCore inserted
        # an 802.1Q header *before* the rtl8_4 header. v44.66.8 produced VID0;
        # v44.66.9 proved descriptor DVID9/8 becomes VID9/8 on wire. Any VLAN
        # in front of 0x8899 prevents the external switch's after-SA parser from
        # seeing the CPU tag at the expected position.
        if l2["outer"] == 0x8100 and l2["inner"] == 0x8899 and len(pkt) >= 26:
            proto = struct.unpack("!H", pkt[18:20])[0]
            if proto == 0x0400:
                vlan_before_rtl8.append((t, pkt, orig, l2))
                vlan_before_rtl8_vids[str(l2["vlan"])] += 1
                if l2["vlan"] == 0:
                    vlan0_before_rtl8.append((t, pkt, orig, l2))

        # Any Realtek CPU tag visible on a physical user cable means the
        # external switch did not consume it. Count both direct and VLAN-
        # prefixed forms so post-fix captures have an explicit zero target.
        if l2["outer"] == 0x8899 or (l2["outer"] == 0x8100 and l2["inner"] == 0x8899):
            rtl8_on_user_wire.append((t, pkt, orig, l2))

        if l2["vlan"] == 0 and l2["inner"] not in KNOWN_ETHERTYPES and len(pkt) >= 64:
            unknown_vlan0.append((t, pkt, orig, l2))
            # Payload-as-header signature: most of DA+SA is printable data.  The
            # observed SSH failure starts 00:00:0d:68:6d:61:63:2d:73:68:61:32.
            if printable_ratio(pkt[:12]) >= 0.60:
                ascii_header.append((t, pkt, orig, l2))
    return {
        "path": str(path),
        "snaplen": snaplen,
        "packets": packets,
        "counts": dict(counts),
        "vlan_ids": dict(vids),
        "unknown_vlan0": len(unknown_vlan0),
        "vlan0_before_rtl8_leaks": len(vlan0_before_rtl8),
        "vlan_before_rtl8_leaks": len(vlan_before_rtl8),
        "vlan_before_rtl8_vids": dict(vlan_before_rtl8_vids),
        "realtek_cpu_tag_on_user_wire": len(rtl8_on_user_wire),
        "priority_vlan0_on_user_wire": len(priority_vlan0),
        "vlan_before_rtl8_examples": [
            {
                "timestamp": t,
                "wire_len": orig,
                "vlan": l2["vlan"],
                "first64_hex": pkt[:64].hex(),
            }
            for t, pkt, orig, l2 in vlan_before_rtl8[:8]
        ],
        "vlan0_before_rtl8_examples": [
            {
                "timestamp": t,
                "wire_len": orig,
                "first64_hex": pkt[:64].hex(),
            }
            for t, pkt, orig, _ in vlan0_before_rtl8[:8]
        ],
        "payload_as_header_candidates": len(ascii_header),
        "payload_as_header_examples": [
            {
                "timestamp": t,
                "wire_len": orig,
                "first64_hex": pkt[:64].hex(),
            }
            for t, pkt, orig, _ in ascii_header[:8]
        ],
    }


def duplicate_summary(lan, wan, window_s: float):
    # Packet-body equality is intentional: this catches the exact CPU frame on
    # both physical jacks.  A small time window avoids matching unrelated ARP
    # or repeated TCP retransmissions seconds later.
    index = defaultdict(list)
    for j, (t, pkt, orig) in enumerate(wan):
        index[hashlib.sha1(pkt).digest()].append((t, j, orig))
    for values in index.values():
        values.sort()

    matches = []
    types = Counter()
    for i, (t, pkt, orig) in enumerate(lan):
        values = index.get(hashlib.sha1(pkt).digest())
        if not values:
            continue
        times = [x[0] for x in values]
        pos = bisect.bisect_left(times, t - window_s)
        if pos < len(values) and abs(values[pos][0] - t) <= window_s:
            tw, j, worig = values[pos]
            l2 = decode_l2(pkt)
            types[f"0x{(l2['inner'] if l2 else 0):04x}"] += 1
            matches.append((i, j, abs(tw - t), pkt, orig, worig, l2))

    examples = []
    for i, j, dt, pkt, orig, worig, l2 in matches[:12]:
        examples.append({
            "lan_index": i,
            "wan_index": j,
            "delta_ms": round(dt * 1000, 6),
            "wire_len": orig,
            "dst": (l2["dst"].hex(":") if l2 else ""),
            "src": (l2["src"].hex(":") if l2 else ""),
            "outer": f"0x{l2['outer']:04x}" if l2 else "",
            "inner": f"0x{l2['inner']:04x}" if l2 else "",
            "vlan": l2["vlan"] if l2 else None,
        })
    return {
        "window_ms": window_s * 1000,
        "exact_cross_port_duplicates": len(matches),
        "duplicate_inner_ethertypes": dict(types),
        "examples": examples,
    }


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("lan_pcap", type=Path)
    ap.add_argument("wan_pcap", type=Path)
    ap.add_argument("--window-ms", type=float, default=10.0)
    ap.add_argument("--json", action="store_true")
    args = ap.parse_args()

    lan = summarize(args.lan_pcap)
    wan = summarize(args.wan_pcap)
    dup = duplicate_summary(lan["packets"], wan["packets"], args.window_ms / 1000.0)
    for x in (lan, wan):
        x.pop("packets", None)
    result = {"lan": lan, "wan": wan, "cross_port": dup}

    if args.json:
        print(json.dumps(result, indent=2, sort_keys=True))
        return

    print("MW5 physical PCAP comparison")
    for name in ("lan", "wan"):
        x = result[name]
        print(f"{name.upper()}: snaplen={x['snaplen']} counts={x['counts']} vlan_ids={x['vlan_ids']}")
        print(f"  unknown_vlan0={x['unknown_vlan0']} vlan_before_rtl8_leaks={x['vlan_before_rtl8_leaks']} "
              f"vlan_before_rtl8_vids={x['vlan_before_rtl8_vids']} vlan0_before_rtl8_leaks={x['vlan0_before_rtl8_leaks']} "
              f"realtek_cpu_tag_on_user_wire={x['realtek_cpu_tag_on_user_wire']} "
              f"payload_as_header_candidates={x['payload_as_header_candidates']}")
    print(f"cross_port exact_duplicates_10ms={dup['exact_cross_port_duplicates']} inner={dup['duplicate_inner_ethertypes']}")
    if dup["examples"]:
        print("first cross-port duplicate examples:")
        for e in dup["examples"]:
            print("  {src} -> {dst} vlan={vlan} inner={inner} dt={delta_ms}ms len={wire_len}".format(**e))


if __name__ == "__main__":
    main()
