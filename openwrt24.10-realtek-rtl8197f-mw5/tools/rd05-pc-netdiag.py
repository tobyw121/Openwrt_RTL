#!/usr/bin/env python3
"""Small AF_PACKET companion for Xiaomi RD05 CPU-port/DSA validation.

No third-party Python modules are required.  The program captures Ethernet
frames, emits ARP and EtherType 0x8899 probes, and writes PCAP/JSON/text output.
It changes only the selected interface when --configure-ip is requested.
"""

from __future__ import annotations

import argparse
import fcntl
import json
import os
import select
import socket
import struct
import subprocess
import sys
import time
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import BinaryIO

ETH_P_ALL = 0x0003
ETH_P_ARP = 0x0806
ETH_P_IP = 0x0800
ETH_P_RD05 = 0x8899
RD05_TOKEN = b"RD05MAGIC-V31"
PACKET_OUTGOING = 4
SIOCGIFHWADDR = 0x8927
BROADCAST = b"\xff" * 6


def mac_text(value: bytes) -> str:
    return ":".join(f"{part:02x}" for part in value)


def mac_bytes(value: str) -> bytes:
    parts = value.split(":")
    if len(parts) != 6:
        raise ValueError(f"invalid MAC address: {value}")
    return bytes(int(part, 16) for part in parts)


def ipv4_bytes(value: str) -> bytes:
    return socket.inet_aton(value)


def interface_mac(name: str) -> bytes:
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        request = struct.pack("256s", name.encode("ascii")[:15])
        response = fcntl.ioctl(sock.fileno(), SIOCGIFHWADDR, request)
        return response[18:24]
    finally:
        sock.close()


def run_ip(*args: str) -> None:
    subprocess.run(["ip", *args], check=True)


def configure_interface(name: str, address: str, prefix: int) -> None:
    run_ip("link", "set", "dev", name, "up")
    run_ip("address", "replace", f"{address}/{prefix}", "dev", name)


def checksum(data: bytes) -> int:
    if len(data) & 1:
        data += b"\x00"
    total = sum(struct.unpack(f"!{len(data) // 2}H", data))
    total = (total & 0xFFFF) + (total >> 16)
    total += total >> 16
    return (~total) & 0xFFFF


def build_arp(src_mac: bytes, src_ip: str, target_ip: str) -> bytes:
    ethernet = BROADCAST + src_mac + struct.pack("!H", ETH_P_ARP)
    arp = struct.pack(
        "!HHBBH6s4s6s4s",
        1,
        ETH_P_IP,
        6,
        4,
        1,
        src_mac,
        ipv4_bytes(src_ip),
        b"\x00" * 6,
        ipv4_bytes(target_ip),
    )
    return ethernet + arp


def build_icmp(src_mac: bytes, dst_mac: bytes, src_ip: str, dst_ip: str,
               sequence: int) -> bytes:
    payload = b"RD05-NATIVE-V36-ICMP" + struct.pack("!I", sequence)
    ident = os.getpid() & 0xFFFF
    icmp = struct.pack("!BBHHH", 8, 0, 0, ident, sequence & 0xFFFF) + payload
    icmp = struct.pack("!BBHHH", 8, 0, checksum(icmp), ident,
                       sequence & 0xFFFF) + payload
    version_ihl = 0x45
    total_length = 20 + len(icmp)
    ip_header = struct.pack(
        "!BBHHHBBH4s4s",
        version_ihl,
        0,
        total_length,
        sequence & 0xFFFF,
        0,
        64,
        1,
        0,
        ipv4_bytes(src_ip),
        ipv4_bytes(dst_ip),
    )
    ip_header = ip_header[:10] + struct.pack("!H", checksum(ip_header)) + ip_header[12:]
    return dst_mac + src_mac + struct.pack("!H", ETH_P_IP) + ip_header + icmp


def build_magic(src_mac: bytes, dst_mac: bytes, role: str, kind: str,
                session: str, sequence: int, interface: str) -> bytes:
    payload = (
        RD05_TOKEN
        + f" version=1 role={role} type={kind} session={session} "
          f"seq={sequence} mono_ns={time.monotonic_ns()} if={interface}".encode("ascii")
    )
    payload = payload.ljust(46, b"\x00")
    return dst_mac + src_mac + struct.pack("!H", ETH_P_RD05) + payload


def parse_magic(frame: bytes) -> dict[str, str]:
    pos = frame.find(RD05_TOKEN)
    if pos < 0:
        return {}
    text = frame[pos:].split(b"\x00", 1)[0].decode("ascii", "replace")
    fields: dict[str, str] = {}
    for item in text.split()[1:]:
        if "=" in item:
            key, value = item.split("=", 1)
            fields[key] = value
    return fields


class PcapWriter:
    def __init__(self, file: BinaryIO) -> None:
        self.file = file
        self.file.write(struct.pack("<IHHIIII", 0xA1B2C3D4, 2, 4, 0, 0, 65535, 1))

    def write(self, frame: bytes, timestamp: float) -> None:
        seconds = int(timestamp)
        microseconds = int((timestamp - seconds) * 1_000_000)
        self.file.write(struct.pack("<IIII", seconds, microseconds,
                                    len(frame), len(frame)))
        self.file.write(frame)


@dataclass
class Counters:
    captured: int = 0
    captured_bytes: int = 0
    arp: int = 0
    ipv4: int = 0
    rd05_magic: int = 0
    outgoing_captured: int = 0
    self_source: int = 0
    peer_frames: int = 0
    from_router: int = 0
    to_router: int = 0
    arp_replies: int = 0
    router_arp_replies: int = 0
    router_magic: int = 0
    malformed: int = 0
    arp_requests_sent: int = 0
    icmp_requests_sent: int = 0
    magic_broadcast_sent: int = 0
    magic_unicast_sent: int = 0
    magic_acks_sent: int = 0


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--interface", required=True)
    parser.add_argument("--duration", type=int, default=120)
    parser.add_argument("--configure-ip", action="store_true")
    parser.add_argument("--pc-ip", default="192.168.1.3")
    parser.add_argument("--router-ip", default="192.168.1.1")
    parser.add_argument("--prefix", type=int, default=24)
    parser.add_argument("--router-mac", help="optional known router LAN MAC")
    parser.add_argument("--interval-ms", type=int, default=1000,
                        help="probe interval in milliseconds (default 1000)")
    parser.add_argument("--calibration", action="store_true",
                        help="send probes every 20 ms for RGMII calibration")
    parser.add_argument("--output-dir", default="rd05-pc-netdiag-output")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    if os.geteuid() != 0:
        print("root privileges are required for AF_PACKET", file=sys.stderr)
        return 2
    if args.duration <= 0:
        print("--duration must be positive", file=sys.stderr)
        return 2
    if args.calibration:
        args.interval_ms = 20
    if args.interval_ms < 10 or args.interval_ms > 60000:
        print("--interval-ms must be in the range 10..60000", file=sys.stderr)
        return 2

    if args.configure_ip:
        configure_interface(args.interface, args.pc_ip, args.prefix)

    src_mac = interface_mac(args.interface)
    router_mac = mac_bytes(args.router_mac) if args.router_mac else None
    output = Path(args.output_dir)
    output.mkdir(parents=True, exist_ok=True)
    counters = Counters()
    sources: dict[str, int] = {}

    sock = socket.socket(socket.AF_PACKET, socket.SOCK_RAW, socket.htons(ETH_P_ALL))
    sock.bind((args.interface, 0))
    sock.setblocking(False)

    start = time.monotonic()
    deadline = start + args.duration
    next_probe = start
    sequence = 0
    session = f"pc-{os.getpid()}-{int(time.time())}"

    with (output / "rd05-pc-netdiag.pcap").open("wb") as pcap_file:
        pcap = PcapWriter(pcap_file)
        while time.monotonic() < deadline:
            now = time.monotonic()
            if now >= next_probe:
                sequence += 1
                sock.send(build_arp(src_mac, args.pc_ip, args.router_ip))
                counters.arp_requests_sent += 1
                sock.send(build_magic(src_mac, BROADCAST, "pc", "probe", session, sequence, args.interface))
                counters.magic_broadcast_sent += 1
                if router_mac is not None:
                    sock.send(build_magic(src_mac, router_mac, "pc", "probe", session, sequence, args.interface))
                    counters.magic_unicast_sent += 1
                    sock.send(build_icmp(src_mac, router_mac, args.pc_ip,
                                         args.router_ip, sequence))
                    counters.icmp_requests_sent += 1
                next_probe = now + args.interval_ms / 1000.0

            readable, _, _ = select.select([sock], [], [], min(0.2, deadline - now))
            if not readable:
                continue
            while True:
                try:
                    frame, packet_addr = sock.recvfrom(65535)
                except BlockingIOError:
                    break
                packet_type = packet_addr[2] if len(packet_addr) > 2 else -1
                timestamp = time.time()
                pcap.write(frame, timestamp)
                counters.captured += 1
                counters.captured_bytes += len(frame)
                if len(frame) < 14:
                    counters.malformed += 1
                    continue
                dst, src, proto = frame[:6], frame[6:12], struct.unpack("!H", frame[12:14])[0]
                sources[mac_text(src)] = sources.get(mac_text(src), 0) + 1
                outgoing = packet_type == PACKET_OUTGOING
                self_source = src == src_mac
                if outgoing:
                    counters.outgoing_captured += 1
                if self_source:
                    counters.self_source += 1
                if not outgoing and not self_source:
                    counters.peer_frames += 1

                from_router_frame = False
                if proto == ETH_P_ARP:
                    counters.arp += 1
                    if len(frame) >= 42:
                        operation = struct.unpack("!H", frame[20:22])[0]
                        sender_ip = socket.inet_ntoa(frame[28:32])
                        target_ip = socket.inet_ntoa(frame[38:42])
                        if operation == 2:
                            counters.arp_replies += 1
                        if sender_ip == args.router_ip and not self_source:
                            router_mac = frame[22:28]
                            from_router_frame = True
                            if operation == 2 and target_ip == args.pc_ip:
                                counters.router_arp_replies += 1
                elif proto == ETH_P_IP:
                    counters.ipv4 += 1
                elif proto == ETH_P_RD05:
                    counters.rd05_magic += 1
                    fields = parse_magic(frame)
                    if not outgoing and not self_source and fields.get("role") == "router":
                        router_mac = src
                        from_router_frame = True
                        counters.router_magic += 1
                        if fields.get("type") == "probe":
                            try:
                                rx_seq = int(fields.get("seq", "0"), 10)
                            except ValueError:
                                rx_seq = 0
                            ack_session = fields.get("session", session)
                            sock.send(build_magic(src_mac, src, "pc", "ack",
                                                  ack_session, rx_seq,
                                                  args.interface))
                            counters.magic_acks_sent += 1
                if router_mac is not None and src == router_mac and not self_source:
                    from_router_frame = True
                if from_router_frame:
                    counters.from_router += 1
                if router_mac is not None and dst == router_mac and not outgoing:
                    counters.to_router += 1

    result = {
        "interface": args.interface,
        "pc_ip": args.pc_ip,
        "router_ip": args.router_ip,
        "pc_mac": mac_text(src_mac),
        "router_mac": mac_text(router_mac) if router_mac else None,
        "duration_seconds": args.duration,
        "probe_interval_ms": args.interval_ms,
        "counters": asdict(counters),
        "source_macs": dict(sorted(sources.items(), key=lambda item: (-item[1], item[0]))),
        "verdict": ("router-reply-observed" if counters.from_router else
                    "peer-traffic-observed-no-router" if counters.peer_frames else
                    "no-router-reply-observed"),
    }
    (output / "rd05-pc-netdiag.json").write_text(json.dumps(result, indent=2) + "\n")
    lines = [
        f"interface={result['interface']}",
        f"pc_mac={result['pc_mac']}",
        f"router_mac={result['router_mac'] or '-'}",
        f"verdict={result['verdict']}",
    ] + [f"{key}={value}" for key, value in asdict(counters).items()]
    (output / "rd05-pc-netdiag.txt").write_text("\n".join(lines) + "\n")
    print("\n".join(lines))
    return 0 if result["verdict"] == "router-reply-observed" else 1


if __name__ == "__main__":
    raise SystemExit(main())
