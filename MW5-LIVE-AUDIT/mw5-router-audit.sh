#!/bin/sh
# Router-side collector v7.9/v44.66.12 - read-only register/network diagnostics
set -u
BASE="${MW5_AUDIT_BASE:-/tmp/mw5-audit}"
SNAPLEN="${MW5_AUDIT_SNAPLEN:-256}"
LAST_SNAPSHOT_DIR=""

have() { command -v "$1" >/dev/null 2>&1; }
ts() { date '+%Y-%m-%dT%H:%M:%S%z'; }
safe() { echo "$1" | tr -c 'A-Za-z0-9_.-' '_'; }
sdir() { echo "$BASE/$(safe "$1")"; }

timeline() {
    d="$1"; msg="$2"
    printf '%s\t%s\t%s\n' "$(date +%s 2>/dev/null || echo 0)" "$(ts)" "$msg" >> "$d/timeline.tsv"
}

run_to() {
    f="$1"; shift
    { echo "# time=$(ts)"; echo "# cmd=$*"; "$@" 2>&1; echo "# rc=$?"; } > "$f"
    return 0
}

# IMPORTANT: Never read /sys/kernel/debug/regmap/*/registers on RTL8197F.
# The SoC exposes MMIO regmaps containing unsafe/unmapped ranges and Linux
# debugfs performs sequential reads.  On MW5 this can raise a fatal MIPS bus
# error.  Use only the curated read-only /proc/mw5-rtl8367-* diagnostics below.


snapshot() {
    s="$1"; label="$(safe "$2")"; sd="$(sdir "$s")"
    d="$sd/snapshots/$(date +%s)-$label"
    mkdir -p "$d"
    LAST_SNAPSHOT_DIR="$d"
    timeline "$sd" "snapshot:$label"

    printf '%s\n' "$(ts)" > "$d/time.txt"
    cat /proc/uptime > "$d/uptime.txt" 2>/dev/null || true
    {
        echo "legacy_tx_layout=$(cat /sys/module/tag_rtl4_9/parameters/tx_layout 2>/dev/null || echo not-loaded)"
        grep '^cpu_tag_decode ' /proc/mw5-rtl8367 2>/dev/null || true
        echo "expected_dsa_tag=rtl8_4"
        [ -x /usr/sbin/mw5-portmode ] && /usr/sbin/mw5-portmode show 2>/dev/null || true
    } > "$d/tx-layout.txt"
    cat /proc/net/dev > "$d/net-dev.txt" 2>/dev/null || true
    cat /proc/net/softnet_stat > "$d/softnet.txt" 2>/dev/null || true
    cat /proc/interrupts > "$d/interrupts.txt" 2>/dev/null || true
    cat /proc/rd05-rtknet > "$d/rd05-rtknet.txt" 2>/dev/null || true
    for f in mw5-rtl8367 mw5-rtl8367-regs mw5-rtl8367-mib mw5-rtl8367-ports mw5-rtl8367-flowctrl; do
        [ -r "/proc/$f" ] && cat "/proc/$f" > "$d/$f.txt" 2>/dev/null || true
    done

    have mw5-accel && run_to "$d/mw5-accel.txt" mw5-accel status
    have ip && {
        run_to "$d/ip-link.txt" ip -d -s link
        run_to "$d/ip-addr.txt" ip addr show
        run_to "$d/route4.txt" ip -4 route show table all
        run_to "$d/route6.txt" ip -6 route show table all
        run_to "$d/neigh.txt" ip neigh show
    }
    have bridge && {
        run_to "$d/bridge-link.txt" bridge -d -s link show
        run_to "$d/bridge-vlan.txt" bridge -d vlan show
        run_to "$d/bridge-fdb.txt" bridge -d fdb show
        run_to "$d/bridge-mdb.txt" bridge -d mdb show
    }
    have nft && run_to "$d/nft.txt" nft list ruleset
    have conntrack && {
        run_to "$d/conntrack-stats.txt" conntrack -S
        run_to "$d/conntrack-count.txt" conntrack -C
    }

    if have ethtool; then
        mkdir -p "$d/ethtool"
        for dev in eth0 lan wan br-lan wlan0 wlan1; do
            ip link show dev "$dev" >/dev/null 2>&1 || continue
            run_to "$d/ethtool/$dev.txt" ethtool "$dev"
            run_to "$d/ethtool/$dev.stats.txt" ethtool -S "$dev"
            run_to "$d/ethtool/$dev.pause.txt" ethtool -a "$dev"
            run_to "$d/ethtool/$dev.eee.txt" ethtool --show-eee "$dev"
        done
    fi

    dmesg > "$d/dmesg.txt" 2>/dev/null || true
    have logread && logread > "$d/logread.txt" 2>/dev/null || true

    dmesg | grep -E \
      'RTL836|realtek-smi|CPU tag|CPU egress|isolation|threshold|SSC|RGMII|p0 rgmii|rtl865x l2cpu|rtl865x pipe|cpuidma|CPURPDCR|dma_cr|mw5 dsa-rx|RX drop untagged|Link is' \
      > "$d/switch-focus.txt" 2>/dev/null || true
}

snapshot_stream() {
    # v7.2: immediate snapshots must survive the exact failure seen on
    # 2026-09-20, where ICMP remained alive but new SSH/banner exchanges timed
    # out.  Keep this payload small and deterministic; the PC opens this as a
    # multiplexed channel on the already-established SSH transport.
    s="$1"; label="$(safe "$2")"; sd="$(sdir "$s")"
    d="$sd/snapshots/$(date +%s)-$label-stream"
    mkdir -p "$d"
    LAST_SNAPSHOT_DIR="$d"
    timeline "$sd" "snapshot-stream:$label"
    printf '%s
' "$(ts)" > "$d/time.txt"
    cat /proc/uptime > "$d/uptime.txt" 2>/dev/null || true
    {
        echo "audit=v7.9-v44.66.12"
        echo "legacy_tx_layout=$(cat /sys/module/tag_rtl4_9/parameters/tx_layout 2>/dev/null || echo not-loaded)"
        grep '^cpu_tag_decode ' /proc/mw5-rtl8367 2>/dev/null || true
        echo "expected_dsa_tag=rtl8_4"
    } > "$d/tx-layout.txt"
    cat /proc/rd05-rtknet > "$d/rd05-rtknet.txt" 2>/dev/null || true
    cat /proc/net/dev > "$d/net-dev.txt" 2>/dev/null || true
    cat /proc/net/softnet_stat > "$d/softnet.txt" 2>/dev/null || true
    cat /proc/interrupts > "$d/interrupts.txt" 2>/dev/null || true
    for f in mw5-rtl8367 mw5-rtl8367-regs mw5-rtl8367-mib mw5-rtl8367-ports mw5-rtl8367-flowctrl; do
        [ -r "/proc/$f" ] && cat "/proc/$f" > "$d/$f.txt" 2>/dev/null || true
    done
    have ip && {
        ip -d -s link > "$d/ip-link.txt" 2>&1 || true
        ip -4 route show table all > "$d/route4.txt" 2>&1 || true
        ip neigh show > "$d/neigh.txt" 2>&1 || true
    }
    dmesg 2>/dev/null | tail -n 600 > "$d/dmesg-tail.txt" || true
    have logread && logread 2>/dev/null | tail -n 600 > "$d/logread-tail.txt" || true
    tar -C "$(dirname "$d")" -cf - "$(basename "$d")"
    rc=$?
    rm -rf "$d"
    return $rc
}

telemetry() {
    sd="$1"
    while :; do
        {
            echo "===== $(ts) ====="
            echo "legacy_tx_layout=$(cat /sys/module/tag_rtl4_9/parameters/tx_layout 2>/dev/null || echo not-loaded) diagnostic_only=1"
            grep '^cpu_tag_decode ' /proc/mw5-rtl8367 2>/dev/null || true
            grep -E '^( *eth0:| *lan:| *wan:| *br-lan:| *wlan0:| *wlan1:)' /proc/net/dev 2>/dev/null || true
            [ -r /proc/rd05-rtknet ] && \
              grep -E '^(mw5_oem_geometry|mw5_oem_vlan|mw5_oem_flowctrl|mw5_rx_oem_state|mw5_rx_recovery|mw5_rx_dsa|mw5_tx_dma|mw5_tx_oem_state|mw5_tx_coherent|mw5_tx_desc_decode|irq |napi_detail|rings |registers |rx packets|tx packets)' \
              /proc/rd05-rtknet 2>/dev/null || true
        } >> "$sd/telemetry-1s.log"
        [ -f "$sd/telemetry-1s.log" ] && [ "$(wc -c < "$sd/telemetry-1s.log" 2>/dev/null || echo 0)" -gt 1048576 ] && { tail -c 524288 "$sd/telemetry-1s.log" > "$sd/telemetry-1s.log.new" 2>/dev/null && mv "$sd/telemetry-1s.log.new" "$sd/telemetry-1s.log"; }
        sleep 1
    done
}

live_stream() {
    # v7: this stream is consumed by one long-lived SSH connection opened
    # before the traffic test. If the router later stops accepting new TCP/SSH
    # sessions, samples already emitted are still present on the PC and the
    # existing connection can continue until the dataplane actually dies.
    s="$1"
    while :; do
        echo "===== epoch=$(date +%s 2>/dev/null || echo 0) iso=$(ts) session=$(safe "$s") ====="
        echo "legacy_tx_layout=$(cat /sys/module/tag_rtl4_9/parameters/tx_layout 2>/dev/null || echo not-loaded) diagnostic_only=1"
            grep '^cpu_tag_decode ' /proc/mw5-rtl8367 2>/dev/null || true
        grep -E '^(mw5_oem_geometry|mw5_oem_vlan|mw5_oem_flowctrl|mw5_rx_oem_state|mw5_rx_recovery|mw5_rx_dsa|mw5_tx_dma|mw5_tx_oem_state|mw5_tx_coherent|mw5_tx_desc_decode|mw5_legacy_accel|irq |napi_detail|rings |registers |rx packets|tx packets)' /proc/rd05-rtknet 2>/dev/null || true
        grep -E '^( *eth0:| *lan:| *wan:| *br-lan:| *wlan0:| *wlan1:)' /proc/net/dev 2>/dev/null || true
        sleep 1
    done
}

mw5_pcap_loop() {
    sd="$1"
    n=0
    while :; do
        mw5-pcap eth0 "$sd/router-master-eth0-$n.pcap" 300 >> "$sd/mw5-pcap.log" 2>&1 || break
        n=$((n + 1))
    done
}

start_capture() {
    sd="$1"
    if have tcpdump; then
        # v7.8: the router has only 128 MiB RAM and /tmp is tmpfs. The old
        # 120k-packet capture could consume ~60 MiB shmem and trigger the OOM
        # killer during iperf. Keep only a small corroborating master capture;
        # the full physical LAN/WAN PCAPs already live on the PC.
        tcpdump -i eth0 -nn -e -s 192 -B 512 -c 6000 \
            -w "$sd/router-master-eth0.pcap" > "$sd/tcpdump.log" 2>&1 &
        echo $! >> "$sd/pids"
        return 0
    fi
    if have mw5-pcap; then
        mw5_pcap_loop "$sd" &
        echo $! >> "$sd/pids"
        return 0
    fi
    echo "Neither tcpdump nor mw5-pcap is available; no eth0 PCAP captured." > "$sd/pcap-unavailable.txt"
}

start_session() {
    s="$1"; sd="$(sdir "$s")"
    mkdir -p "$sd/snapshots"
    : > "$sd/pids"
    : > "$sd/timeline.tsv"
    timeline "$sd" start
    snapshot "$s" baseline
    have logread && { logread -f > "$sd/logread-follow.txt" 2>&1 & echo $! >> "$sd/pids"; }
    telemetry "$sd" & echo $! >> "$sd/pids"
    start_capture "$sd"
}

stop_session() {
    s="$1"; sd="$(sdir "$s")"
    [ -d "$sd" ] || exit 0
    snapshot "$s" final
    timeline "$sd" stop
    [ -r "$sd/pids" ] && while read p; do kill "$p" >/dev/null 2>&1 || true; done < "$sd/pids"
}

case "${1:-}" in
    start) start_session "$2" ;;
    snapshot) snapshot "$2" "$3" ;;
    snapshot-stream) snapshot_stream "$2" "$3" ;;
    live-stream) live_stream "$2" ;;
    mark)
        sd="$(sdir "$2")"; mkdir -p "$sd"
        timeline "$sd" "mark:$(safe "$3")"
        ;;
    stop) stop_session "$2" ;;
    *) echo "Usage: $0 start SESSION | snapshot SESSION LABEL | snapshot-stream SESSION LABEL | live-stream SESSION | mark SESSION LABEL | stop SESSION" >&2; exit 2 ;;
esac
