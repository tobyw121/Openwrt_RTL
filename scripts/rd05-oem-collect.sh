#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Standalone OEM BusyBox collector. Never reads MTD/NVMEM/flash contents.
# No GPIO export, configuration changes, module operations or listener/upload.
umask 077
VERSION=1.3
OUTPUT=/tmp
ROOT=
LABEL=snapshot
DURATION=60
INTERVAL=5
DEEP=0
DRIVERS=0
REGISTERS=0
PCAP_IFACE=
MAX_FILES=20000
MAX_KB=12288
MAX_FILE_BYTES=524288
MAX_SECONDS=300
COUNT=0
WORK=
LOCK=
CHILD=
TIMER=
SAVED_LOGLEVEL=
BASE_WORK=
CAP_PID=
CAP_TIMER=
PHASE=inventory

usage() {
    cat <<'EOF'
Usage: sh rd05-oem-collect.sh [options]
  --output DIR       Existing local directory (default /tmp)
  --label NAME       Archive label, e.g. idle or ping
  --duration SEC     Live observation, 60..600 seconds (default 60)
  --interval SEC     Target pause between snapshots, 1..60 (default 5)
  --drivers          Copy existing Realtek-related .ko files (max 16 MiB)
  --deep             Inventory additional Realtek state paths (no unknown reads)
  --rd05-registers   Issue the documented RD05 MMIO/8367 READ requests
                     through /proc/rtl865x; briefly reduce console loglevel
  --capture IFACE    Optional bounded ARP/ICMP/DHCP PCAP if tcpdump exists
  --root DIR        Offline proc/sys fixture or mounted root; no commands,
                     packet capture or register requests (duration must be 0)
  --help

Requires root for a live run. No extra packages, timeout or flock required.
Writes a private directory and .tar.gz beside it; keeps both, never uploads.
Default: 60-second repeated sampling, no PCAP or register requests.
Deep mode reads discovered vendor state files; command/sensitive names excluded.
No MTD partition contents or hashes.
The archive contains device/network identities; review before publishing.
EOF
}

die() { echo "ERROR: $*" >&2; exit 1; }
have() { command -v "$1" >/dev/null 2>&1 || type "$1" >/dev/null 2>&1; }
while [ "$#" -gt 0 ]; do
    case "$1" in
        --help|-h) usage; exit 0 ;;
        --version) echo "$VERSION"; exit 0 ;;
        --output|--label|--duration|--interval|--capture|--root)
            [ "$#" -ge 2 ] || die "Missing value for $1"
            case "$1" in
                --output) OUTPUT=$2 ;; --duration) DURATION=$2 ;;
                --label) LABEL=$2 ;;
                --interval) INTERVAL=$2 ;;
                --capture) PCAP_IFACE=$2 ;;
                --root)
                    case "$2" in /|'') die 'Offline root must not be / or empty' ;; esac
                    ROOT=${2%/} ;;
            esac
            shift 2 ;;
        --rd05-registers) REGISTERS=1; shift ;;
        --deep) DEEP=1; shift ;;
        --drivers) DRIVERS=1; shift ;;
        *) die "Unknown option: $1" ;;
    esac
done
case "$LABEL" in ''|*[!a-zA-Z0-9_.-]*) die 'Invalid label' ;; esac
[ "${#LABEL}" -le 32 ] || die 'Label is too long'
case "$DURATION" in ''|*[!0-9]*) die 'Invalid duration' ;; esac
[ "${#DURATION}" -le 3 ] && [ "$DURATION" -le 600 ] || die 'Duration must be 60..600 (offline 0)'
case "$INTERVAL" in ''|*[!0-9]*) die 'Invalid interval' ;; esac
[ "${#INTERVAL}" -le 2 ] && [ "$INTERVAL" -ge 1 ] && [ "$INTERVAL" -le 60 ] || die 'Interval must be 1..60'
[ -d "$OUTPUT" ] || die 'Output directory does not exist'
if [ -n "$ROOT" ]; then
    [ -d "$ROOT/proc" ] && [ -d "$ROOT/sys" ] || die 'Offline root needs proc and sys'
    [ "$DURATION" -eq 0 ] && [ "$REGISTERS" -eq 0 ] && [ -z "$PCAP_IFACE" ] ||
        die 'Offline mode requires duration 0 and forbids register/PCAP options'
else
    [ "$(id -u)" = 0 ] || die 'Run as root on the router'
    [ "$DURATION" -ge 60 ] || die 'Live duration must be at least 60 seconds'
fi
if [ -n "$PCAP_IFACE" ]; then
    case "$PCAP_IFACE" in *[!a-zA-Z0-9_.:-]*|'-'*) die 'Invalid capture interface' ;; esac
    [ "${#PCAP_IFACE}" -le 32 ] && [ -d "/sys/class/net/$PCAP_IFACE" ] ||
        die 'Capture interface does not exist'
fi
for tool in dd du awk tar gzip date find sort tr wc; do
    have "$tool" || die "Required command unavailable: $tool"
done
WORK=$OUTPUT/rd05-oem-collect.$LABEL.$(date +%Y%m%d-%H%M%S).$$
mkdir "$WORK" || die 'Cannot create private output directory'
LOCK=$OUTPUT/rd05-oem-collect.lock
if ! mkdir "$LOCK" 2>/dev/null; then
    rmdir "$WORK"
    die "Collector lock exists: $LOCK; check for another run before removing a stale lock"
fi
echo "$$" > "$LOCK/pid"
echo "DIRECTORY=$WORK"
START=$(awk '{print int($1)}' /proc/uptime)
MAX_SECONDS=$((300 + DURATION))
BASE_WORK=$WORK
STATUS=$WORK/status.tsv
printf 'result\texit\tphase\toutput\n' > "$STATUS"

cleanup() {
    stop_capture
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
    now=$(monotonic)
    [ "$((now - START))" -lt "$MAX_SECONDS" ] || return 1
    used=$(du -sk "$BASE_WORK" 2>/dev/null | awk '{print $1}')
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
        printf 'limit\t-\t%s\t%s\n' "$PHASE" "$dest" >> "$STATUS"
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
    printf '%s\t%s\t%s\t%s\n' "$result" "$rc" "$PHASE" "$dest" >> "$STATUS"
    return 0
}

copy() {
    local dest source
    dest=$1
    source=$2
    # Old OEM BusyBox may lack both head -c and mktemp. DD is bounded by
    # 128 reads of at most 4096 bytes; keep its summary/errors separate.
    run "$dest" 4 sh -c 'printf "SOURCE %s\n" "$1" >>"$2"; exec dd if="$1" bs=4096 count=128 2>>"$2"' \
        sh "$source" "$WORK/read-errors.log"
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
        printf 'unavailable\t-\t%s\t%s\n' "$PHASE" "$1" >> "$STATUS"
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

deep_allowed() {
    local lower leaf
    lower=$(printf '%s' "$1" | tr 'A-Z' 'a-z')
    leaf=${lower##*/}
    case "$lower" in
        *mtd*|*flash*|*spirom*|*memdump*|*ramdump*|*coredump*|*nvmem*|*efuse*|*eeprom*|*password*|*passwd*|*secret*|*key*|*pmk*|*psk*)
            return 1 ;;
        *watchdog*|*reset*|*reinit*|*test*|*clear*|*trigger*|*issue*|*scan*|*write*|*gpio_ctrl*)
            return 1 ;;
    esac
    case "$leaf" in
        cmd|command|memory|mem|phyreg*|8367reg*|led|enable*|disable*|open*|close*|set*|remove*|delete*|add*|inject*|start*|stop*|txdesc|txurb|up_event|up_flag|vdev|phypara_file|macdm|err_access|monitor_status|nat25filter|stactrl_info)
            return 1 ;;
        mib_all|mib_auth|mib_ap_profile|mib_staconfig|mib_wsc|mib_wds|mib_dkeytbl|mib_gkeytbl|cam_info|sae_peertable)
            return 1 ;;
    esac
    return 0
}

extract_drivers() {
    local base path rel name bytes total count available
    [ "$DRIVERS" -eq 1 ] || return 0
    total=0
    count=0
    mkdir -p "$BASE_WORK/drivers"
    if [ -z "$ROOT" ]; then
        available=$(awk '/^MemAvailable:/{print $2}' /proc/meminfo)
        if [ -n "$available" ] && [ "$available" -lt 32768 ]; then
            echo 'Extraction skipped: less than 32 MiB MemAvailable' > "$BASE_WORK/drivers/summary.txt"
            return 0
        fi
    fi
    printf 'result\tbytes\tpath\n' > "$BASE_WORK/drivers/manifest.tsv"
    for base in /lib/modules /usr/lib/modules; do
        [ -d "$ROOT$base" ] || continue
        # A fixed local list, no recursive device/sysfs traversal.
        find "$ROOT$base" -type f > "$BASE_WORK/drivers/candidates.txt"
        while IFS= read -r path; do
            name=${path##*/}
            case "$name" in rtl*.ko|rtk*.ko|rtw*.ko|ther_ctrl.ko|cfg80211.ko|compat.ko|mac80211.ko) ;; *) continue ;; esac
            rel=${path#"$ROOT"}
            bytes=$(wc -c < "$path")
            case "$bytes" in ''|*[!0-9]*) continue ;; esac
            if [ "$count" -ge 64 ] || [ "$((total + bytes))" -gt 16777216 ]; then
                printf 'size-limit\t%s\t%s\n' "$bytes" "$rel" >> "$BASE_WORK/drivers/manifest.tsv"
                continue
            fi
            mkdir -p "$BASE_WORK/drivers/files${rel%/*}"
            if cp "$path" "$BASE_WORK/drivers/files$rel"; then
                total=$((total + bytes)); count=$((count + 1))
                printf 'copied\t%s\t%s\n' "$bytes" "$rel" >> "$BASE_WORK/drivers/manifest.tsv"
                if have sha256sum; then
                    sha256sum "$BASE_WORK/drivers/files$rel" >> "$BASE_WORK/drivers/sha256.txt"
                fi
            else
                printf 'copy-error\t%s\t%s\n' "$bytes" "$rel" >> "$BASE_WORK/drivers/manifest.tsv"
            fi
        done < "$BASE_WORK/drivers/candidates.txt"
    done
    printf 'copied_files=%s\ncopied_bytes=%s\n' "$count" "$total" > "$BASE_WORK/drivers/summary.txt"
}

discover_states() {
    local path n list entry logical selected
    [ "$DEEP" -eq 1 ] || return 0
    n=0
    for path in "$ROOT"/proc/rtl* "$ROOT"/proc/rtk* "$ROOT"/proc/rtw* "$ROOT"/proc/wlan* \
        "$ROOT"/proc/wl[0-9]* "$ROOT"/proc/net/rtl* "$ROOT"/proc/net/rtk* \
        "$ROOT"/proc/fast* "$ROOT"/proc/hw* "$ROOT"/proc/brsc \
        "$ROOT"/sys/kernel/debug/rtl* "$ROOT"/sys/kernel/debug/rtk* \
        "$ROOT"/sys/kernel/debug/rtw* "$ROOT"/sys/kernel/debug/ieee80211 \
        "$ROOT"/sys/module/rtl*/parameters "$ROOT"/sys/module/rtk*/parameters \
        "$ROOT"/sys/module/rtw*/parameters "$ROOT"/sys/realtek_boards \
        "$ROOT"/proc/port_status "$ROOT"/proc/link_status "$ROOT"/proc/asicCounter \
        "$ROOT"/proc/ethPort_mibStats "$ROOT"/proc/rxRing "$ROOT"/proc/txRing \
        "$ROOT"/proc/netif "$ROOT"/proc/vlan "$ROOT"/proc/pvid "$ROOT"/proc/l2 \
        "$ROOT"/proc/l3 "$ROOT"/proc/napt "$ROOT"/proc/igmp "$ROOT"/proc/br_igmp*; do
        [ -e "$path" ] || continue
        n=$((n + 1))
        run "inventory/realtek-tree-$n.txt" 10 find "$path" -type f
    done
    list=$BASE_WORK/inventory/realtek-candidates.txt
    : > "$list"
    for entry in "$BASE_WORK"/inventory/realtek-tree-*.txt; do
        [ -f "$entry" ] || continue
        cat "$entry" >> "$list"
    done
    sort -u "$list" > "$list.sorted"
    mv "$list.sorted" "$list"
    : > "$BASE_WORK/inventory/realtek-eligible.txt"
    printf 'decision\tpath\n' > "$BASE_WORK/inventory/realtek-decisions.tsv"
    selected=0
    while IFS= read -r entry; do
        logical=${entry#"$ROOT"}
        # Find output/error text is not a command; accept only these roots.
        case "$logical" in /proc/*|/sys/*) ;; *) continue ;; esac
        if ! deep_allowed "$logical"; then
            printf 'excluded-command-or-sensitive\t%s\n' "$logical" >> "$BASE_WORK/inventory/realtek-decisions.tsv"
        elif [ ! -f "$entry" ] || [ ! -r "$entry" ]; then
            printf 'unreadable\t%s\n' "$logical" >> "$BASE_WORK/inventory/realtek-decisions.tsv"
        elif [ "$selected" -ge 2048 ]; then
            printf 'node-limit\t%s\n' "$logical" >> "$BASE_WORK/inventory/realtek-decisions.tsv"
        else
            # OEM 4.4.176 stack-protector panic during the first deep read.
            # Filename filtering cannot establish callback safety. Inventory
            # unknown nodes only; base ring/counter allowlist remains separate.
            printf 'inventory-only-unverified-callback\t%s\n' "$logical" >> "$BASE_WORK/inventory/realtek-decisions.tsv"
        fi
    done < "$list"
}

append_state() {
    local logical source bytes frame
    logical=$1
    source=$WORK/files$logical
    [ -f "$source" ] || return 0
    bytes=$(wc -c < "$source")
    frame=$WORK/state-frame.tmp
    printf '\nSTATE path=%s bytes=%s uptime=%s\n' "$logical" "$bytes" "$(monotonic)" > "$frame"
    cat "$source" >> "$frame" || return 1
    # Concatenated gzip members are decoded together by standard gzip/Python.
    # One aggregate file avoids thousands of tmpfs inodes per live snapshot.
    gzip -c "$frame" >> "$WORK/realtek-state.txt.gz" || return 1
    rm -f "$source" "$frame"
}

deep_snapshot() {
    local logical path
    [ "$DEEP" -eq 1 ] || return 0
    while IFS= read -r logical; do
        file "$logical"
        append_state "$logical" || return 1
    done < "$BASE_WORK/inventory/realtek-eligible.txt"
    # Re-sample exported GPIO levels, thermal state and interface counters.
    # Never export new GPIOs or sweep arbitrary MMIO addresses.
    for path in "$ROOT"/sys/class/gpio/gpio*/value \
        "$ROOT"/sys/class/thermal/thermal_zone*/temp \
        "$ROOT"/sys/class/net/*/statistics/* \
        "$ROOT"/sys/devices/system/cpu/cpu[0-9]*/cpufreq/scaling_cur_freq; do
        [ -f "$path" ] || continue
        file "${path#"$ROOT"}"
        append_state "${path#"$ROOT"}" || return 1
    done
}

monotonic() {
    # Do not shorten a measurement when OEM NTP adjusts the wall clock.
    awk '{print int($1)}' /proc/uptime
}

stop_capture() {
    if [ -n "$CAP_TIMER" ]; then
        kill "$CAP_TIMER" 2>/dev/null
        wait "$CAP_TIMER" 2>/dev/null
        CAP_TIMER=
    fi
    if [ -n "$CAP_PID" ]; then
        kill -TERM "$CAP_PID" 2>/dev/null
        sleep 1
        kill -KILL "$CAP_PID" 2>/dev/null
        wait "$CAP_PID" 2>/dev/null
        printf 'finished\t%s\tcapture\tnetwork/capture.pcap\n' "$?" >> "$STATUS"
        CAP_PID=
    fi
    :
}

start_capture() {
    [ -n "$PCAP_IFACE" ] || return 0
    if ! have tcpdump; then
        printf 'unavailable\t-\tcapture\ttcpdump; counter snapshots still collected\n' >> "$STATUS"
        return 0
    fi
    mkdir -p "$BASE_WORK/network"
    (
        ulimit -f 1024 || exit 125
        exec tcpdump -p -U -ni "$PCAP_IFACE" -s 128 -c 4096 -w - \
            'arp or icmp or icmp6 or (udp and (port 67 or port 68))'
    ) > "$BASE_WORK/network/capture.pcap" 2> "$BASE_WORK/network/capture-stderr.txt" &
    CAP_PID=$!
    (
        sleep "$DURATION"
        kill -TERM "$CAP_PID" 2>/dev/null
        sleep 1
        kill -KILL "$CAP_PID" 2>/dev/null
    ) &
    CAP_TIMER=$!
}

live_snapshot() {
    local label stage begin end path
    label=$1
    begin=$(monotonic)
    stage=$BASE_WORK/live-raw-$label
    mkdir "$stage" || return 1
    WORK=$stage
    PHASE=$label
    printf 'start_uptime=%s\n' "$begin" > "$stage/timing.txt"
    net_snapshot "$label"
    deep_snapshot || return 1
    for path in /proc/stat /proc/loadavg /proc/meminfo /proc/net/wireless; do file "$path"; done
    query "$label-dmesg" dmesg
    end=$(monotonic)
    printf 'end_uptime=%s\n' "$end" >> "$stage/timing.txt"
    WORK=$BASE_WORK
    # A private snapshot archive is verified before deleting only its own
    # temporary raw directory. Keep failed staging directories for analysis.
    if tar -czf "$BASE_WORK/live/$label.tar.gz.partial" -C "$stage" . &&
        tar -tzf "$BASE_WORK/live/$label.tar.gz.partial" >/dev/null; then
        mv "$BASE_WORK/live/$label.tar.gz.partial" "$BASE_WORK/live/$label.tar.gz"
        case "$stage" in "$BASE_WORK"/live-raw-[0-9][0-9][0-9][0-9]) rm -rf "$stage" ;; esac
        printf '%s\t%s\t%s\tpacked\n' "$label" "$begin" "$end" >> "$BASE_WORK/live/timeline.tsv"
    else
        printf '%s\t%s\t%s\tpack-failed\n' "$label" "$begin" "$end" >> "$BASE_WORK/live/timeline.tsv"
    fi
    PHASE=inventory
}

observe() {
    local started now index label remaining pause sample_started rest
    mkdir -p "$BASE_WORK/live"
    printf 'sample\tstart_uptime\tend_uptime\tresult\n' > "$BASE_WORK/live/timeline.tsv"
    started=$(monotonic)
    index=0
    if [ "${DEEP:-0}" -eq 1 ] && [ -z "${ROOT:-}" ]; then
        read SAVED_LOGLEVEL rest < /proc/sys/kernel/printk
        dmesg -n 1
    fi
    start_capture
    while :; do
        label=$(printf '%04d' "$index")
        sample_started=$(monotonic)
        live_snapshot "$label" || return 1
        now=$(monotonic)
        # Include a snapshot starting at/after the requested time span.
        # Slow reads may lengthen the run, never shorten it to one sleep.
        [ "$((sample_started - started))" -lt "$DURATION" ] || [ "$index" -lt 1 ] || break
        remaining=$((DURATION - (now - started)))
        pause=$INTERVAL
        [ "$remaining" -ge "$pause" ] || pause=$remaining
        [ "$pause" -le 0 ] || sleep "$pause"
        index=$((index + 1))
    done
    stop_capture
    if [ -n "${SAVED_LOGLEVEL:-}" ]; then
        dmesg -n "$SAVED_LOGLEVEL"
        SAVED_LOGLEVEL=
    fi
    printf 'requested_seconds=%s\nobserved_seconds=%s\ninterval_target=%s\n' \
        "$DURATION" "$((now - started))" "$INTERVAL" > "$BASE_WORK/live/window.txt"
}

cat > "$WORK/README.txt" <<EOF
Realtek OEM collector $VERSION
Run label: $LABEL
Base allowlist inventory; deep Realtek discovery enabled: $DEEP.
Missing/unavailable/failed reads are not zeros.
Per query deadline 4..5 s (discovery 10 s), file limit $MAX_FILE_BYTES bytes,
collection quota $MAX_FILES queries and approximately $MAX_KB KiB plus one query.
Virtual files are copied up to 512 KiB; command output is capped at <=1 MiB.
New queries stop after $MAX_SECONDS seconds; packaging follows afterward.
Files at the size limit may be truncated. Kernel-hung reads cannot be made safe
by a shell watchdog. Reads of vendor diagnostics can affect timing or latch counters.
No MTD contents/hashes, /dev/mem, NVMEM/eFuse, UCI/NVRAM dumps,
process environments or WPA/hostapd configuration are intentionally collected.
Driver module extraction explicitly enabled: $DRIVERS (up to 16 MiB extra).
Deep mode inventories unknown vendor files without reading them.
No GPIO writes/exports, network configuration, firewall changes or services started.
PCI sysfs devices are enumerated; no PCI config/resource memory is read.
GPIO data describes registered controllers and exported lines; not every physical pin.
Optional PCAP contains ARP/ICMP/DHCP prefixes, may include host identities/payload.
PCAP runs concurrently for the requested window, up to 4096 packets/1 MiB.
Live snapshots are timestamped and individually compressed in live/*.tar.gz.
Target pause $INTERVAL seconds; actual read time lengthens the sampling interval.
Register READ requests enabled: $REGISTERS. PCAP interface: ${PCAP_IFACE:-none}.
Snapshots are sequential, not atomic. Publication/descriptor reads do not prove DMA data.
The collector does not clear dmesg or counters. No automatic transfer.
Only temporary raw live directories are deleted after their archive is verified.
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
discover_states
echo "Live observation: at least $DURATION seconds, target pause $INTERVAL seconds. Generate PC ping traffic now."
observe || die 'Live snapshot failed; partial data retained'
net_snapshot after
query dmesg-after dmesg
extract_drivers
echo 'Packing local archive...'
parent=${WORK%/*}
name=${WORK##*/}
tar -czf "$WORK.tar.gz" -C "$parent" "$name" || die 'Archive failed; directory retained'
echo "ARCHIVE=$WORK.tar.gz"
echo "DIRECTORY=$WORK"
echo 'Keep the archive private until reviewed. No firmware build required.'
