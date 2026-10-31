#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Standalone OEM BusyBox collector. Never reads MTD/NVMEM/flash contents.
# No GPIO export, configuration changes, module operations or listener/upload.
umask 077
VERSION=1.0
OUTPUT=/tmp
ROOT=
DURATION=20
REGISTERS=0
PCAP_IFACE=
MAX_FILES=768
MAX_KB=12288
MAX_FILE_BYTES=524288
MAX_SECONDS=300
COUNT=0
WORK=
LOCK=
CHILD=
TIMER=
SAVED_LOGLEVEL=

usage() {
    cat <<'EOF'
Usage: sh rd05-oem-collect.sh [options]
  --output DIR       Existing local directory (default /tmp)
  --duration SEC     Counter observation interval, 0..120 (default 20)
  --rd05-registers   Issue the documented RD05 MMIO/8367 READ requests
                     through /proc/rtl865x; briefly reduce console loglevel
  --capture IFACE    Optional bounded ARP/ICMP/DHCP PCAP if tcpdump exists
  --root DIR        Offline proc/sys fixture or mounted root; no commands,
                     packet capture or register requests (duration must be 0)
  --help

Requires root for a live run. No extra packages, timeout or flock required.
Writes a private directory and .tar.gz beside it; keeps both, never uploads.
Default: no PCAP, no register requests. No MTD partition contents or hashes.
The archive contains device/network identities; review before publishing.
EOF
}

die() { echo "ERROR: $*" >&2; exit 1; }
have() { command -v "$1" >/dev/null 2>&1 || type "$1" >/dev/null 2>&1; }
while [ "$#" -gt 0 ]; do
    case "$1" in
        --help|-h) usage; exit 0 ;;
        --output|--duration|--capture|--root)
            [ "$#" -ge 2 ] || die "Missing value for $1"
            case "$1" in
                --output) OUTPUT=$2 ;; --duration) DURATION=$2 ;;
                --capture) PCAP_IFACE=$2 ;;
                --root)
                    case "$2" in /|'') die 'Offline root must not be / or empty' ;; esac
                    ROOT=${2%/} ;;
            esac
            shift 2 ;;
        --rd05-registers) REGISTERS=1; shift ;;
        *) die "Unknown option: $1" ;;
    esac
done
case "$DURATION" in ''|*[!0-9]*) die 'Invalid duration' ;; esac
[ "${#DURATION}" -le 3 ] && [ "$DURATION" -le 120 ] || die 'Duration must be 0..120'
[ -d "$OUTPUT" ] || die 'Output directory does not exist'
if [ -n "$ROOT" ]; then
    [ -d "$ROOT/proc" ] && [ -d "$ROOT/sys" ] || die 'Offline root needs proc and sys'
    [ "$DURATION" -eq 0 ] && [ "$REGISTERS" -eq 0 ] && [ -z "$PCAP_IFACE" ] ||
        die 'Offline mode requires duration 0 and forbids register/PCAP options'
else
    [ "$(id -u)" = 0 ] || die 'Run as root on the router'
fi
if [ -n "$PCAP_IFACE" ]; then
    case "$PCAP_IFACE" in *[!a-zA-Z0-9_.:-]*|'-'*) die 'Invalid capture interface' ;; esac
    [ "${#PCAP_IFACE}" -le 32 ] && [ -d "/sys/class/net/$PCAP_IFACE" ] ||
        die 'Capture interface does not exist'
fi
for tool in dd du awk tar gzip date; do
    have "$tool" || die "Required command unavailable: $tool"
done
WORK=$OUTPUT/rd05-oem-collect.$(date +%Y%m%d-%H%M%S).$$
mkdir "$WORK" || die 'Cannot create private output directory'
LOCK=$OUTPUT/rd05-oem-collect.lock
if ! mkdir "$LOCK" 2>/dev/null; then
    rmdir "$WORK"
    die "Collector lock exists: $LOCK; check for another run before removing a stale lock"
fi
echo "$$" > "$LOCK/pid"
START=$(date +%s)
STATUS=$WORK/status.tsv
printf 'result\texit\toutput\n' > "$STATUS"

cleanup() {
    [ -z "$TIMER" ] || { kill "$TIMER" 2>/dev/null; wait "$TIMER" 2>/dev/null; }
    if [ -n "$CHILD" ]; then
        kill -TERM "$CHILD" 2>/dev/null
        kill -KILL "$CHILD" 2>/dev/null
        wait "$CHILD" 2>/dev/null
    fi
    if [ -n "$SAVED_LOGLEVEL" ]; then
        dmesg -n "$SAVED_LOGLEVEL" 2>/dev/null
    fi
    rm -f "$LOCK/pid"
    rmdir "$LOCK" 2>/dev/null
    :
}
trap cleanup EXIT
trap 'exit 130' INT
trap 'exit 143' TERM HUP

room() {
    local used now
    [ "$COUNT" -lt "$MAX_FILES" ] || return 1
    now=$(date +%s)
    [ "$((now - START))" -lt "$MAX_SECONDS" ] || return 1
    used=$(du -sk "$WORK" 2>/dev/null | awk '{print $1}')
    case "$used" in ''|*[!0-9]*) return 1 ;; esac
    [ "$used" -lt "$MAX_KB" ]
}

# Every external query runs as an owned child with an owned watchdog, not
# killall/pkill. Never promises to recover a kernel task stuck in D state.
run() {
    local dest seconds rc result
    dest=$1
    seconds=$2
    shift 2
    if ! room; then
        printf 'limit\t-\t%s\n' "$dest" >> "$STATUS"
        return 1
    fi
    COUNT=$((COUNT + 1))
    mkdir -p "$WORK/${dest%/*}"
    (
        # Units differ between Linux shells (512/1024 bytes). Allow a full
        # 512-KiB ring read in either case, cap command output at <=1 MiB.
        ulimit -f 1024 || exit 125
        exec "$@"
    ) > "$WORK/$dest" 2>&1 &
    CHILD=$!
    (
        sleep "$seconds"
        echo 'query timed out' > "$WORK/$dest.timeout"
        kill -TERM "$CHILD" 2>/dev/null
        sleep 1
        kill -KILL "$CHILD" 2>/dev/null
    ) &
    TIMER=$!
    wait "$CHILD"
    rc=$?
    kill "$TIMER" 2>/dev/null
    wait "$TIMER" 2>/dev/null
    CHILD=
    TIMER=
    result=ok
    [ "$rc" -eq 0 ] || result=error
    [ ! -f "$WORK/$dest.timeout" ] || result=timeout
    printf '%s\t%s\t%s\n' "$result" "$rc" "$dest" >> "$STATUS"
    return 0
}

copy() {
    local dest source
    dest=$1
    source=$2
    # Old OEM BusyBox may lack both head -c and mktemp. DD is bounded by
    # 128 reads of at most 4096 bytes; keep its summary/errors separate.
    run "$dest" 4 sh -c 'exec dd if="$1" bs=4096 count=128 2>"$2"' \
        sh "$source" "$WORK/$dest.stderr"
}

file() {
    local logical
    logical=$1
    # Hard deny even if a future allowlist accidentally adds these surfaces.
    case "$logical" in
        /dev/*|/proc/mtd|/proc/kcore|/proc/kmsg|/proc/keys|/sys/class/mtd/*|*/nvmem|*/eeprom|*/resource[0-9]*|/sys/bus/pci/devices/*/config)
            return ;;
    esac
    [ -f "$ROOT$logical" ] && [ -r "$ROOT$logical" ] || return 0
    copy "files$logical" "$ROOT$logical"
}

query() {
    local dest
    [ -z "$ROOT" ] || return 0
    dest=$1
    shift
    if have "$1"; then
        run "commands/$dest.txt" 5 "$@"
    else
        printf 'unavailable\t-\t%s\n' "$1" >> "$STATUS"
    fi
}

net_snapshot() {
    local phase node name path n iface
    phase=$1
    for node in dev snmp netstat softnet_stat; do
        [ ! -f "$ROOT/proc/net/$node" ] || copy "network/$phase/$node.txt" "$ROOT/proc/net/$node"
    done
    for node in /proc/interrupts /proc/softirqs; do
        [ ! -f "$ROOT$node" ] || copy "network/$phase/${node##*/}.txt" "$ROOT$node"
    done
    # Use known display-only vendor entries, not recursive reads of every
    # proc/debugfs file. In particular never touch test/reset/efuse/MIB keys.
    for name in txRing rxRing mbufRing asicCounter diagnostic nic_skb fc_threshold; do
        path="$ROOT/proc/rtl865x/$name"
        [ ! -f "$path" ] || copy "realtek/$phase/$name.txt" "$path"
    done
    # Prefer wired interfaces so virtual/WLAN interfaces cannot crowd them out.
    n=0
    for path in "$ROOT"/sys/class/net/eth* "$ROOT"/sys/class/net/lan* "$ROOT"/sys/class/net/wan*; do
        [ -d "$path" ] || continue
        n=$((n + 1)); [ "$n" -le 16 ] || break
        iface=${path##*/}
        query "$phase-$iface-driver" ethtool -i "$iface"
        query "$phase-$iface-statistics" ethtool -S "$iface"
    done
}

cat > "$WORK/README.txt" <<EOF
Realtek OEM collector $VERSION
Default allowlist inventory; missing/unavailable/failed reads are not zeros.
Per query deadline 4..5 s (PCAP 10 s), file limit $MAX_FILE_BYTES bytes,
collection quota $MAX_FILES queries and approximately $MAX_KB KiB plus one query.
Virtual files are copied up to 512 KiB; command output is capped at <=1 MiB.
New queries stop after $MAX_SECONDS seconds; packaging follows afterward.
Files at the size limit may be truncated. Kernel-hung reads cannot be made safe
by a shell watchdog. Reads of vendor diagnostics can affect timing or latch counters.
No MTD contents/hashes, /dev/mem, NVMEM/eFuse, rootfs binaries, UCI/NVRAM dumps,
process environments, WPA/hostapd configuration or private keys are collected.
No GPIO writes/exports, network configuration, firewall changes or services started.
PCI sysfs devices are enumerated; no PCI config/resource memory is read.
GPIO data describes registered controllers and exported lines; not every physical pin.
Optional PCAP contains ARP/ICMP/DHCP prefixes, may include host identities/payload.
Register READ requests enabled: $REGISTERS. PCAP interface: ${PCAP_IFACE:-none}.
Snapshots are sequential, not atomic. Publication/descriptor reads do not prove DMA data.
The collector does not clear dmesg or counters. No automatic transfer/deletion.
EOF
query date date -u
query uname uname -a
for path in /proc/cpuinfo /proc/meminfo /proc/cmdline /proc/version /proc/uptime \
    /proc/modules /proc/iomem /proc/ioports /proc/interrupts /proc/softirqs \
    /proc/bus/pci/devices /proc/tty/driver/serial /proc/devices /proc/crypto \
    /proc/net/route /proc/net/ipv6_route /proc/net/arp /proc/net/wireless \
    /proc/net/vlan/config /proc/sys/kernel/tainted /proc/sys/kernel/printk \
    /proc/fast_nat /proc/hw_nat /proc/rtl865x/brsc \
    /etc/openwrt_release /etc/os-release /tmp/sysinfo/board_name /tmp/sysinfo/model; do
    file "$path"
done
query dmesg-before dmesg
query proc-realtek-names ls -l /proc/rtl865x
query ip-link ip -s link
query ip-address ip addr
query ip-route ip route show table all
query ip-rule ip rule
query ip-neighbour ip neigh
query bridges brctl show
query bridge-links bridge link show
query bridge-vlans bridge vlan show
query firewall-ipv4 iptables-save
query firewall-ipv6 ip6tables-save
query firewall-nft nft list ruleset
query wireless-devices iw dev
query wireless-capabilities iw phy
query wireless-legacy-module modinfo rtl8192cd
query wireless-wifi6-module modinfo rtk_wifi6
query pci-devices lspci -nn

net_snapshot before

echo 'Collecting PCI, platform, GPIO and network inventory...'
for bus in pci platform mdio spi; do
    [ ! -d "$ROOT/sys/bus/$bus/devices" ] || run "inventory/$bus.txt" 4 ls -l "$ROOT/sys/bus/$bus/devices"
done
n=0
for path in "$ROOT"/sys/bus/platform/devices/*; do
    [ -d "$path" ] || continue
    case "${path##*/}" in
        *ethernet*|*serial*|*gpio*|*pci*|*rtl*|*rtk*|*smi*|*mdio*) ;;
        *) continue ;;
    esac
    n=$((n + 1)); [ "$n" -le 32 ] || break
    logical=${path#"$ROOT"}
    for attr in uevent modalias resource irq of_node/compatible of_node/reg of_node/interrupts; do
        file "$logical/$attr"
    done
    [ ! -L "$path/driver" ] || run "platform/${path##*/}-driver.txt" 4 readlink "$path/driver"
done
n=0
for path in "$ROOT"/sys/bus/pci/devices/*; do
    [ -d "$path" ] || continue
    n=$((n + 1)); [ "$n" -le 64 ] || break
    logical=${path#"$ROOT"}
    for attr in vendor device subsystem_vendor subsystem_device class revision irq \
        modalias uevent resource current_link_speed current_link_width \
        max_link_speed max_link_width; do
        file "$logical/$attr"
    done
    [ ! -L "$path/driver" ] || run "pci/${path##*/}-driver.txt" 4 readlink "$path/driver"
done
for path in "$ROOT"/sys/class/gpio/gpio*; do
    [ -d "$path" ] || continue
    logical=${path#"$ROOT"}
    for attr in label base ngpio direction value active_low edge; do
        file "$logical/$attr"
    done
done
file /sys/kernel/debug/gpio
for path in "$ROOT"/sys/kernel/debug/pinctrl/*; do
    [ -d "$path" ] || continue
    logical=${path#"$ROOT"}
    for attr in pins pinmux-pins pinconf-pins gpio-ranges; do file "$logical/$attr"; done
done
for path in "$ROOT"/sys/class/net/*; do
    [ -d "$path" ] || continue
    logical=${path#"$ROOT"}
    for attr in address addr_assign_type ifindex iflink mtu type operstate carrier \
        speed duplex flags phys_port_name phys_switch_id; do file "$logical/$attr"; done
done
for group in thermal hwmon leds; do
    for path in "$ROOT"/sys/class/"$group"/*; do
        [ -d "$path" ] || continue
        logical=${path#"$ROOT"}
        for attr in type temp name brightness max_brightness temp1_input temp1_max temp1_crit; do
            file "$logical/$attr"
        done
    done
done
for path in "$ROOT"/sys/devices/system/cpu/cpu[0-9]*/cpufreq; do
    [ -d "$path" ] || continue
    logical=${path#"$ROOT"}
    for attr in cpuinfo_cur_freq cpuinfo_max_freq cpuinfo_min_freq scaling_cur_freq \
        scaling_governor scaling_available_frequencies; do file "$logical/$attr"; done
done
file /sys/kernel/debug/clk/clk_summary
# Existing DT only, no mounting debugfs or probing unregistered hardware.
for base in /sys/firmware/devicetree/base /proc/device-tree; do
    if [ -d "$ROOT$base" ]; then
        for attr in model compatible '#address-cells' '#size-cells'; do file "$base/$attr"; done
        run inventory/devicetree-paths.txt 4 find "$ROOT$base" -type d
        break
    fi
done

if [ "$REGISTERS" -eq 1 ]; then
    [ -w /proc/rtl865x/memory ] && [ -w /proc/rtl865x/phyReg ] ||
        die 'RD05 read-request endpoints missing; partial inventory retained'
    echo 'Issuing explicit RD05 register READ requests; no register writes.'
    read SAVED_LOGLEVEL rest < /proc/sys/kernel/printk
    dmesg -n 1
    for request in \
        'read 0xb8010000 0x28' 'read 0xb801003c 0x10' 'read 0xb80100a4 4' \
        'read 0xbb804000 4' 'read 0xbb804058 4' 'read 0xbb804100 0x50' \
        'read 0xbb805108 4' 'read 0xb8000850 4'; do
        # Fixed strings only: no eval, arbitrary ranges or stale buffer pointers.
        run "registers/mmio-$(echo "$request" | awk '{print $2}').txt" 4 \
            sh -c 'printf "%s\n" "$1" > /proc/rtl865x/memory' sh "$request"
    done
    for reg in 1305 1307 13f9 13f7 1d52 1d53 1d54 1d55 1219 121a 1303 1304 1d70 1d78 12c7 12cf 00ee 03f7; do
        run "registers/switch-$reg.txt" 4 sh -c \
            'printf "8367read %s\n" "$1" > /proc/rtl865x/phyReg' sh "$reg"
    done
    query rd05-register-readback dmesg
    dmesg -n "$SAVED_LOGLEVEL"
    SAVED_LOGLEVEL=
fi
if [ -n "$PCAP_IFACE" ]; then
    if have tcpdump; then
        # tcpdump may drop privileges: shell owns the new archive file, and
        # tcpdump writes PCAP to stdout. Keep diagnostics separate from PCAP.
        run network/capture.pcap 10 sh -c \
            'exec tcpdump -p -U -ni "$1" -s 128 -c 128 -w - "arp or icmp or icmp6 or (udp and (port 67 or port 68))" 2> "$2"' \
            sh "$PCAP_IFACE" "$WORK/network/capture-stderr.txt"
    else
        printf 'unavailable\t-\ttcpdump; counter snapshots still collected\n' >> "$STATUS"
    fi
fi
echo "Observation window: $DURATION seconds. Generate PC ping traffic now."
[ "$DURATION" -eq 0 ] || sleep "$DURATION"
net_snapshot after
query dmesg-after dmesg
echo 'Packing local archive...'
parent=${WORK%/*}
name=${WORK##*/}
tar -czf "$WORK.tar.gz" -C "$parent" "$name" || die 'Archive failed; directory retained'
echo "ARCHIVE=$WORK.tar.gz"
echo "DIRECTORY=$WORK"
echo 'Keep the archive private until reviewed. No firmware build required.'
