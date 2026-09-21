#!/usr/bin/env bash
# Tenda Nova MW5 - Interactive two-NIC live audit v7.8 (v44.66.12)
# Physical NICs remain in the host namespace. Only temporary macvlan children
# are moved into isolated test namespaces.
set -Eeuo pipefail

ROUTER_IP="192.168.1.1"
WAN_IP="192.168.178.1"
CYCLES=3
DURATION=45
PARALLEL=4
SNAPLEN=2048
LAN_NS="mw5lan"
WAN_NS="mw5wan"
LAN_VIF="mw5lan0"
WAN_VIF="mw5wan0"
LAN_IF=""
WAN_IF=""
DEFAULT_IF=""
SESSION=""
OUT=""
ROUTER_STARTED=0
DNS_PID=""
IPERF_PID=""
LAN_TCP_PID=""
WAN_TCP_PID=""
WATCH_PID=""
ROUTER_STREAM_PID=""
CLEANUP_RUNNING=0
SNAP_SEQ=0
SSH_KNOWN_HOSTS=""
SSH_CONTROL_PATH=""
SSH_ARGS=()

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROUTER_SCRIPT="$ROOT_DIR/mw5-router-audit.sh"

info(){ echo "[*] $*"; }
ok(){ echo "[+] $*"; }
warn(){ echo "[!] $*"; }
die(){ echo "[X] $*" >&2; exit 1; }

[[ $EUID -eq 0 ]] || die "Bitte mit sudo/root starten."
[[ -r "$ROUTER_SCRIPT" ]] || die "Fehlt: $ROUTER_SCRIPT"

for c in ip tcpdump iperf3 dnsmasq ssh tar awk grep sed date ping timeout; do
    command -v "$c" >/dev/null 2>&1 || die "Fehlendes Programm: $c"
done

# Recover physical NICs left inside namespaces by older v2/v3 runs.
recover_stale_namespaces() {
    local stale=0
    if ip netns list 2>/dev/null | awk '{print $1}' | grep -qx "$LAN_NS"; then
        warn "Verwaisten Namespace $LAN_NS gefunden; räume ihn vor der Adapterauswahl auf."
        ip netns del "$LAN_NS" >/dev/null 2>&1 || true
        stale=1
    fi
    if ip netns list 2>/dev/null | awk '{print $1}' | grep -qx "$WAN_NS"; then
        warn "Verwaisten Namespace $WAN_NS gefunden; räume ihn vor der Adapterauswahl auf."
        ip netns del "$WAN_NS" >/dev/null 2>&1 || true
        stale=1
    fi
    if ((stale)); then
        command -v udevadm >/dev/null 2>&1 && udevadm settle >/dev/null 2>&1 || true
        sleep 1
    fi
}
recover_stale_namespaces

DEFAULT_IF="$(ip route show default 2>/dev/null | awk 'NR==1 {print $5}')"

is_candidate() {
    local d="$1"
    [[ "$d" == "lo" ]] && return 1
    [[ "$d" =~ ^(mw5lan0|mw5wan0|docker|veth|virbr|br-|tun|tap|wg|tailscale|ifb|dummy|bond|team|vxlan|geneve|sit|gre|ip6tnl) ]] && return 1
    [[ -e "/sys/class/net/$d/device" ]] || return 1
    return 0
}

carrier() {
    local d="$1"
    [[ -r "/sys/class/net/$d/carrier" ]] || { echo "?"; return; }
    [[ "$(cat "/sys/class/net/$d/carrier" 2>/dev/null)" == "1" ]] && echo "UP" || echo "DOWN"
}

speed() {
    local d="$1" s
    s="$(cat "/sys/class/net/$d/speed" 2>/dev/null || true)"
    [[ "$s" =~ ^[0-9]+$ ]] && echo "${s}M" || echo "-"
}

ipv4() {
    local x
    x="$(ip -4 -o addr show dev "$1" 2>/dev/null | awk '{print $4}' | paste -sd, -)"
    echo "${x:--}"
}

show_adapters() {
    CANDS=()
    while read -r d; do is_candidate "$d" && CANDS+=("$d"); done < <(ls -1 /sys/class/net | sort)
    ((${#CANDS[@]} >= 2)) || die "Mindestens zwei physische Netzwerkadapter werden benötigt."

    echo
    echo "Verfügbare Netzwerkadapter:"
    printf '%-4s %-16s %-8s %-8s %-18s %-22s %s\n' "Nr" "Interface" "Link" "Speed" "MAC" "IPv4" "Hinweis"
    printf '%-4s %-16s %-8s %-8s %-18s %-22s %s\n' "--" "---------" "----" "-----" "---" "----" "-------"
    local i=1 d note
    for d in "${CANDS[@]}"; do
        note=""
        [[ "$d" == "$DEFAULT_IF" ]] && note="DEFAULT/Management"
        [[ -d "/sys/class/net/$d/wireless" ]] && note="${note:+$note, }WLAN"
        printf '%-4s %-16s %-8s %-8s %-18s %-22s %s\n' \
            "$i" "$d" "$(carrier "$d")" "$(speed "$d")" \
            "$(cat "/sys/class/net/$d/address")" "$(ipv4 "$d")" "$note"
        ((i++))
    done
    echo
    [[ -n "$DEFAULT_IF" ]] && warn "Default-/Management-NIC: $DEFAULT_IF"
}

choose_adapter() {
    local role="$1" exclude="${2:-}" n d answer
    while :; do
        read -r -p "$role auswählen [Nummer]: " n
        [[ "$n" =~ ^[0-9]+$ ]] || { warn "Bitte Nummer eingeben."; continue; }
        ((n>=1 && n<=${#CANDS[@]})) || { warn "Ungültige Nummer."; continue; }
        d="${CANDS[$((n-1))]}"
        [[ "$d" != "$exclude" ]] || { warn "LAN und WAN müssen verschieden sein."; continue; }

        if [[ "$d" == "$DEFAULT_IF" ]]; then
            warn "$d ist deine aktuelle Management-/Default-NIC."
            read -r -p "Wirklich verwenden? Die PC-Verbindung kann abbrechen [ja/NEIN]: " answer
            [[ "$answer" =~ ^(ja|j|yes|y)$ ]] || continue
        fi
        echo "$d"
        return
    done
}

show_adapters
LAN_IF="$(choose_adapter "Adapter am MW5-LAN")"
WAN_IF="$(choose_adapter "Adapter am MW5-WAN" "$LAN_IF")"

echo
echo "Auswahl:"
echo "  MW5 LAN -> $LAN_IF (physisch bleibt sichtbar; Test über $LAN_VIF)"
echo "  MW5 WAN -> $WAN_IF (physisch bleibt sichtbar; Test über $WAN_VIF)"
echo "  PC simuliert WAN-Gateway: $WAN_IP/24"
echo "  MW5 LAN-IP erwartet: $ROUTER_IP"
echo
read -r -p "Sind die beiden Kabel so angeschlossen? [J/n]: " answer
[[ -z "$answer" || "$answer" =~ ^(j|ja|y|yes)$ ]] || exit 0

read -r -p "Anzahl Testzyklen [$CYCLES]: " x
[[ "$x" =~ ^[1-9][0-9]*$ ]] && CYCLES="$x"
read -r -p "Dauer pro Upload/Download in Sekunden [$DURATION]: " x
[[ "$x" =~ ^[1-9][0-9]*$ ]] && DURATION="$x"
read -r -p "Parallele iperf3 Streams [$PARALLEL]: " x
[[ "$x" =~ ^[1-9][0-9]*$ ]] && PARALLEL="$x"

SESSION="mw5-$(date +%Y%m%d-%H%M%S)"
OUT="$PWD/mw5-audit-results/$SESSION"
SSH_CONTROL_PATH="/tmp/${SESSION}-ssh.sock"
mkdir -p "$OUT"/{pc,router,iperf}
touch "$OUT/timeline.tsv"
cat > "$OUT/pc/audit-version.txt" <<EOF
pc_audit=v7.9-v44.66.12
pc_snaplen=$SNAPLEN
ssh_transport=multiplexed-controlmaster
expected_dsa_tag=rtl8_4
EOF

# The MW5 firmware intentionally generates a fresh tmpfs SSH host key on every
# boot. Never use root's global known_hosts for this isolated direct-cable test.
# A per-run file accepts the current boot key once and will still detect a
# key change (e.g. an unexpected router reboot) during the same test session.
SSH_KNOWN_HOSTS="$OUT/pc/ssh-known-hosts"
: > "$SSH_KNOWN_HOSTS"
chmod 600 "$SSH_KNOWN_HOSTS"
SSH_ARGS=(
    -o BatchMode=yes
    -o ConnectTimeout=5
    -o ServerAliveInterval=3
    -o ServerAliveCountMax=2
    -o UserKnownHostsFile="$SSH_KNOWN_HOSTS"
    -o GlobalKnownHostsFile=/dev/null
    -o StrictHostKeyChecking=accept-new
    -o LogLevel=ERROR
    # v44.66.12/v7.8: all short snapshot/mark/final-collect channels reuse one
    # already-established SSH transport.  The 2026-09-20 failure kept ICMP
    # alive while new SSH handshakes timed out during banner exchange.
    -o ControlMaster=auto
    -o ControlPersist=180
    -o ControlPath="$SSH_CONTROL_PATH"
)

ssh_router() {
    timeout 15s ip netns exec "$LAN_NS" ssh \
        "${SSH_ARGS[@]}" \
        root@"$ROUTER_IP" "$@"
}

ssh_router_long() {
    timeout 45s ip netns exec "$LAN_NS" ssh \
        "${SSH_ARGS[@]}" \
        root@"$ROUTER_IP" "$@"
}

start_router_live_stream() {
    mkdir -p "$OUT/router"
    info "Starte v7-Live-Stream über eine persistente SSH-Verbindung ..."
    ip netns exec "$LAN_NS" ssh \
        "${SSH_ARGS[@]}" \
        root@"$ROUTER_IP" "/tmp/mw5-router-audit.sh live-stream '$SESSION'" \
        >"$OUT/router/live-stream-1s.log" \
        2>"$OUT/router/live-stream.err" &
    ROUTER_STREAM_PID=$!
    printf '%s\n' "$ROUTER_STREAM_PID" > "$OUT/router/live-stream.pid"
}

router_snapshot() {
    local label="$1" safe_label archive tmp extract
    safe_label="$(printf '%s' "$label" | tr -c 'A-Za-z0-9_.-' '_')"
    SNAP_SEQ=$((SNAP_SEQ + 1))
    mkdir -p "$OUT/router/live-snapshots" "$OUT/router/live-extracted"
    archive="$OUT/router/live-snapshots/$(printf '%03d' "$SNAP_SEQ")-$safe_label.tar"
    tmp="$archive.part"
    extract="$OUT/router/live-extracted/$(printf '%03d' "$SNAP_SEQ")-$safe_label"

    info "Router-Snapshot: $label (sofortiger PC-Transfer)"
    if ssh_router_long "/tmp/mw5-router-audit.sh snapshot-stream '$SESSION' '$safe_label'" >"$tmp" 2>"$archive.err" &&
       [[ -s "$tmp" ]] && tar -tf "$tmp" >/dev/null 2>&1; then
        mv "$tmp" "$archive"
        mkdir -p "$extract"
        tar -xf "$archive" -C "$extract" >/dev/null 2>&1 || true
        ok "Snapshot lokal gesichert: $archive"
    else
        rm -f "$tmp"
        warn "Snapshot $label konnte nicht mehr per SSH übertragen werden; frühere v7-Snapshots bleiben lokal erhalten."
    fi
}

mark() {
    local label="$1"
    printf '%s\t%s\t%s\n' "$(date +%s.%N)" "$(date --iso-8601=ns)" "$label" >> "$OUT/timeline.tsv"
    ((ROUTER_STARTED)) && ssh_router "/tmp/mw5-router-audit.sh mark '$SESSION' '$label'" >/dev/null 2>&1 || true
}

restore_nics() {
    info "Entferne temporäre Testinterfaces und stelle NetworkManager wieder her ..."

    # Deleting the namespaces deletes only the temporary macvlan children.
    ip netns del "$LAN_NS" >/dev/null 2>&1 || true
    ip netns del "$WAN_NS" >/dev/null 2>&1 || true

    # The physical adapters never leave the host namespace.
    ip link set "$LAN_IF" up >/dev/null 2>&1 || true
    ip link set "$WAN_IF" up >/dev/null 2>&1 || true

    if command -v nmcli >/dev/null 2>&1; then
        nmcli device set "$LAN_IF" managed yes >/dev/null 2>&1 || true
        nmcli device set "$WAN_IF" managed yes >/dev/null 2>&1 || true
        nmcli device connect "$LAN_IF" >/dev/null 2>&1 || true
        nmcli device connect "$WAN_IF" >/dev/null 2>&1 || true
    fi
}

collect_router() {
    ((ROUTER_STARTED)) || return 0
    mkdir -p "$OUT/router/extracted"
    info "Hole Router-Diagnosearchiv ..."
    ssh_router "tar -C /tmp/mw5-audit -czf - '$SESSION'" \
        > "$OUT/router/$SESSION.tar.gz" 2> "$OUT/router/collect.err" || true
    [[ -s "$OUT/router/$SESSION.tar.gz" ]] && \
        tar -xzf "$OUT/router/$SESSION.tar.gz" -C "$OUT/router/extracted" || true
}

cleanup() {
    ((CLEANUP_RUNNING)) && return 0
    CLEANUP_RUNNING=1
    set +e
    [[ -n "$WATCH_PID" ]] && kill "$WATCH_PID" >/dev/null 2>&1
    [[ -n "$ROUTER_STREAM_PID" ]] && kill "$ROUTER_STREAM_PID" >/dev/null 2>&1
    ((ROUTER_STARTED)) && ssh_router "/tmp/mw5-router-audit.sh stop '$SESSION'" >/dev/null 2>&1
    [[ -n "$LAN_TCP_PID" ]] && kill "$LAN_TCP_PID" >/dev/null 2>&1
    [[ -n "$WAN_TCP_PID" ]] && kill "$WAN_TCP_PID" >/dev/null 2>&1
    [[ -n "$DNS_PID" ]] && kill "$DNS_PID" >/dev/null 2>&1
    [[ -n "$IPERF_PID" ]] && kill "$IPERF_PID" >/dev/null 2>&1
    sleep 1
    collect_router
    if command -v python3 >/dev/null 2>&1 && [ -r "$ROOT_DIR/mw5-pcap-compare.py" ] && \
       [ -s "$OUT/pc/LAN.pcap" ] && [ -s "$OUT/pc/WAN.pcap" ]; then
        python3 "$ROOT_DIR/mw5-pcap-compare.py" "$OUT/pc/LAN.pcap" "$OUT/pc/WAN.pcap" \
            > "$OUT/pc/pcap-compare.txt" 2> "$OUT/pc/pcap-compare.err" || true
        python3 "$ROOT_DIR/mw5-pcap-compare.py" --json "$OUT/pc/LAN.pcap" "$OUT/pc/WAN.pcap" \
            > "$OUT/pc/pcap-compare.json" 2>> "$OUT/pc/pcap-compare.err" || true
    fi
    # Close the multiplex master only after the last possible diagnostic
    # channel.  Ignore failure if the dataplane already died completely.
    ip netns exec "$LAN_NS" ssh "${SSH_ARGS[@]}" -O exit root@"$ROUTER_IP" >/dev/null 2>&1 || true
    rm -f "$SSH_CONTROL_PATH" >/dev/null 2>&1 || true
    restore_nics
    if [[ -n "$OUT" && -d "$OUT" ]]; then
        tar -C "$(dirname "$OUT")" -czf "$OUT.tar.gz" "$(basename "$OUT")" 2>/dev/null || true
        echo
        ok "Ergebnisse: $OUT"
        [[ -f "$OUT.tar.gz" ]] && ok "Gesamtarchiv: $OUT.tar.gz"
    fi
}
trap cleanup EXIT INT TERM HUP

info "Bereite die beiden physischen Testadapter vor (sie bleiben sichtbar) ..."
if command -v nmcli >/dev/null 2>&1; then
    nmcli device disconnect "$LAN_IF" >/dev/null 2>&1 || true
    nmcli device disconnect "$WAN_IF" >/dev/null 2>&1 || true
    nmcli device set "$LAN_IF" managed no >/dev/null 2>&1 || true
    nmcli device set "$WAN_IF" managed no >/dev/null 2>&1 || true
fi

# Keep the real interfaces in the default namespace, but remove host L3 state.
ip addr flush dev "$LAN_IF" >/dev/null 2>&1 || true
ip addr flush dev "$WAN_IF" >/dev/null 2>&1 || true
ip link set "$LAN_IF" up
ip link set "$WAN_IF" up

# Clean any leftovers from a prior v4 run.
ip link del "$LAN_VIF" >/dev/null 2>&1 || true
ip link del "$WAN_VIF" >/dev/null 2>&1 || true
ip netns del "$LAN_NS" >/dev/null 2>&1 || true
ip netns del "$WAN_NS" >/dev/null 2>&1 || true

ip netns add "$LAN_NS"
ip netns add "$WAN_NS"

info "Erzeuge temporäre macvlan-Testinterfaces $LAN_VIF und $WAN_VIF ..."
ip link add link "$LAN_IF" name "$LAN_VIF" type macvlan mode bridge
ip link add link "$WAN_IF" name "$WAN_VIF" type macvlan mode bridge
ip link set "$LAN_VIF" netns "$LAN_NS"
ip link set "$WAN_VIF" netns "$WAN_NS"

ip -n "$LAN_NS" link set lo up
ip -n "$WAN_NS" link set lo up
ip -n "$LAN_NS" link set "$LAN_VIF" up
ip -n "$WAN_NS" link set "$WAN_VIF" up
ip -n "$WAN_NS" addr add "$WAN_IP/24" dev "$WAN_VIF"

# Disable common offloads on the real adapters to make captures closer to wire format.
if command -v ethtool >/dev/null 2>&1; then
    ethtool -K "$LAN_IF" gro off gso off tso off lro off >/dev/null 2>&1 || true
    ethtool -K "$WAN_IF" gro off gso off tso off lro off >/dev/null 2>&1 || true
fi

{
    echo "physical_lan=$LAN_IF"
    echo "physical_wan=$WAN_IF"
    echo "test_lan=$LAN_VIF"
    echo "test_wan=$WAN_VIF"
    echo "lan_parent_mac=$(cat /sys/class/net/$LAN_IF/address)"
    echo "wan_parent_mac=$(cat /sys/class/net/$WAN_IF/address)"
    echo "lan_test_mac=$(ip netns exec "$LAN_NS" cat /sys/class/net/$LAN_VIF/address)"
    echo "wan_test_mac=$(ip netns exec "$WAN_NS" cat /sys/class/net/$WAN_VIF/address)"
} > "$OUT/pc/interface-map.txt"

info "Starte WAN-DHCP/DNS-Simulator ..."
ip netns exec "$WAN_NS" dnsmasq \
    --no-daemon --bind-interfaces --interface="$WAN_VIF" \
    --dhcp-range=192.168.178.91,192.168.178.99,255.255.255.0,12h \
    --dhcp-option=3,"$WAN_IP" --dhcp-option=6,"$WAN_IP" \
    --address=/mw5-test.local/"$WAN_IP" \
    --log-dhcp --log-queries \
    > "$OUT/pc/wan-dnsmasq.log" 2>&1 &
DNS_PID=$!

ip netns exec "$WAN_NS" iperf3 -s -B "$WAN_IP" \
    > "$OUT/pc/wan-iperf3-server.log" 2>&1 &
IPERF_PID=$!

info "Starte vollständige PCAP-Mitschnitte auf den physischen Leitungen ..."
tcpdump -i "$LAN_IF" -nn -e -s "$SNAPLEN" -B 8192 \
    -w "$OUT/pc/LAN.pcap" > "$OUT/pc/LAN-tcpdump.log" 2>&1 &
LAN_TCP_PID=$!
tcpdump -i "$WAN_IF" -nn -e -s "$SNAPLEN" -B 8192 \
    -w "$OUT/pc/WAN.pcap" > "$OUT/pc/WAN-tcpdump.log" 2>&1 &
WAN_TCP_PID=$!

info "Warte kurz auf MW5 und fordere LAN-DHCP an (max. 20 s) ..."
sleep 4
DHCP_OK=0
if command -v dhclient >/dev/null 2>&1; then
    timeout 20s ip netns exec "$LAN_NS" dhclient -1 -v \
        -pf "/tmp/dhclient-$LAN_VIF.pid" \
        -lf "/tmp/dhclient-$LAN_VIF.leases" \
        "$LAN_VIF" > "$OUT/pc/LAN-DHCP.txt" 2>&1 && DHCP_OK=1 || true
elif command -v udhcpc >/dev/null 2>&1; then
    timeout 20s ip netns exec "$LAN_NS" udhcpc -n -q -t 4 -T 4 -i "$LAN_VIF" \
        > "$OUT/pc/LAN-DHCP.txt" 2>&1 && DHCP_OK=1 || true
fi

if ! ip -n "$LAN_NS" -4 addr show dev "$LAN_VIF" | grep -q 'inet '; then
    warn "Kein DHCP-Lease innerhalb von 20 s erhalten. Setze für die Diagnose 192.168.1.2/24."
    ip -n "$LAN_NS" addr add 192.168.1.2/24 dev "$LAN_VIF"
else
    ok "LAN-DHCP erfolgreich."
fi
ip -n "$LAN_NS" route show default | grep -q . || ip -n "$LAN_NS" route add default via "$ROUTER_IP"

ip -n "$LAN_NS" addr show > "$OUT/pc/LAN-addresses.txt"
ip -n "$WAN_NS" addr show > "$OUT/pc/WAN-addresses.txt"
ip link show "$LAN_IF" > "$OUT/pc/LAN-parent-link.txt"
ip link show "$WAN_IF" > "$OUT/pc/WAN-parent-link.txt"

info "Prüfe SSH zum MW5 und etabliere den persistenten v7.8-ControlMaster ..."
ROUTER_SSH_OK=1
if ! ssh_router true; then
    ROUTER_SSH_OK=0
    warn "SSH zu root@$ROUTER_IP nicht erreichbar. v7.8 bleibt im passiven No-LAN-Modus aktiv und sammelt PCAP/ARP/DHCP/Linkdiagnose."
    {
        echo "router_ssh=FAIL"
        echo "mode=passive-no-lan"
        date --iso-8601=ns 2>/dev/null || date
    } > "$OUT/router/no-ssh-preflight.txt"
    ip -n "$LAN_NS" neigh show > "$OUT/pc/LAN-neigh-preflight.txt" 2>&1 || true
    ip -n "$LAN_NS" route show > "$OUT/pc/LAN-route-preflight.txt" 2>&1 || true
    ip netns exec "$LAN_NS" arping -I "$LAN_VIF" -c 8 -w 10 "$ROUTER_IP" \
        > "$OUT/pc/LAN-arping-no-ssh.txt" 2>&1 || true
    ip netns exec "$LAN_NS" ping -n -c 8 -W 1 "$ROUTER_IP" \
        > "$OUT/pc/LAN-ping-no-ssh.txt" 2>&1 || true
    for n in 1 2 3; do
        printf '%s\t' "$(date --iso-8601=ns 2>/dev/null || date)" >> "$OUT/pc/no-ssh-link-samples.tsv"
        printf 'lan=' >> "$OUT/pc/no-ssh-link-samples.tsv"
        cat "/sys/class/net/$LAN_IF/carrier" 2>/dev/null | tr -d '\n' >> "$OUT/pc/no-ssh-link-samples.tsv" || true
        printf '\twan=' >> "$OUT/pc/no-ssh-link-samples.tsv"
        cat "/sys/class/net/$WAN_IF/carrier" 2>/dev/null | tr -d '\n' >> "$OUT/pc/no-ssh-link-samples.tsv" || true
        printf '\n' >> "$OUT/pc/no-ssh-link-samples.tsv"
        sleep 2
    done
    warn "Routerseitige Snapshots entfallen mangels LAN. Die physischen PCAPs laufen bis Cleanup weiter; No-LAN-Diagnose ist gespeichert."
    exit 2
fi
ip netns exec "$LAN_NS" ssh "${SSH_ARGS[@]}" -O check root@"$ROUTER_IP" \
    > "$OUT/router/controlmaster-check.txt" 2>&1 || warn "SSH-ControlMaster konnte nicht verifiziert werden; weitere Transfers können bei TCP-Störung ausfallen."

info "Installiere temporären Router-Collector ..."
if ! timeout 15s ip netns exec "$LAN_NS" ssh \
    "${SSH_ARGS[@]}" \
    root@"$ROUTER_IP" \
    "cat >/tmp/mw5-router-audit.sh && chmod 700 /tmp/mw5-router-audit.sh" \
    < "$ROUTER_SCRIPT"; then
    die "Kopieren des Router-Collectors fehlgeschlagen."
fi

info "Starte sicheren Router-Collector (ohne debugfs-regmap scan) ..."
if ! timeout 25s ip netns exec "$LAN_NS" ssh \
    "${SSH_ARGS[@]}" \
    root@"$ROUTER_IP" "/tmp/mw5-router-audit.sh start '$SESSION'" \
    >/dev/null 2>"$OUT/router/collector-start.err"; then
    warn "Router-Collector konnte nicht innerhalb von 25 s gestartet werden."
    die "Collector-Start fehlgeschlagen; Details: $OUT/router/collector-start.err"
fi
ROUTER_STARTED=1
ok "Router-Collector läuft."
router_snapshot BASELINE

ROUTER_CPU_DECODE="$(ssh_router "grep '^cpu_tag_decode ' /proc/mw5-rtl8367 2>/dev/null || true" 2>/dev/null | tail -n1 || true)"
ROUTER_VID0_ACTION="$(ssh_router "grep '^vid0_egress_action=' /proc/mw5-rtl8367 2>/dev/null || true" 2>/dev/null | tail -n1 || true)"
ROUTER_LEGACY_LAYOUT="$(ssh_router 'cat /sys/module/tag_rtl4_9/parameters/tx_layout 2>/dev/null || echo not-loaded' 2>/dev/null | tail -n1 || true)"
printf 'expected_dsa_tag=rtl8_4\ncpu_tag_decode=%s\nvid0_action=%s\nlegacy_tx_layout=%s\n' \
    "${ROUTER_CPU_DECODE:-missing}" "${ROUTER_VID0_ACTION:-missing}" "${ROUTER_LEGACY_LAYOUT:-not-loaded}" > "$OUT/router/dsa-tag-preflight.txt"
if [[ "${ROUTER_CPU_DECODE:-missing}" != *"format8b=1"* && "${MW5_AUDIT_ALLOW_NONV212_LAYOUT:-0}" != 1 ]]; then
    router_snapshot "FAIL-CPU-TAG-MODE"
    die "MW5 RTL8367 CPU tag mode is not eight-byte rtl8_4: ${ROUTER_CPU_DECODE:-missing}. Set MW5_AUDIT_ALLOW_NONV212_LAYOUT=1 only for an intentional legacy-tag diagnostic."
fi
if [[ "${ROUTER_VID0_ACTION:-missing}" != *"vid0_egress_action=untag"* && "${MW5_AUDIT_ALLOW_VID0_TAG:-0}" != 1 ]]; then
    router_snapshot "FAIL-VID0-EGRESS"
    die "RTL8367 reserved VID0 egress policy is not UNTAG: ${ROUTER_VID0_ACTION:-missing}."
fi
ok "DSA preflight: rtl8_4 / format8b=1 and reserved VID0 action=UNTAG; legacy tx_layout=${ROUTER_LEGACY_LAYOUT:-not-loaded} is diagnostic-only."
start_router_live_stream

(
    while :; do
        now="$(date --iso-8601=ns)"
        ip netns exec "$LAN_NS" ping -n -c1 -W1 "$ROUTER_IP" >/dev/null 2>&1 && a=OK || a=FAIL
        ip netns exec "$LAN_NS" ping -n -c1 -W1 "$WAN_IP" >/dev/null 2>&1 && b=OK || b=FAIL
        printf '%s\trouter=%s\twan=%s\n' "$now" "$a" "$b"
        sleep 1
    done
) > "$OUT/pc/reachability-1s.log" 2>&1 &
WATCH_PID=$!

health() {
    local label="$1" a=0 b=0
    ip netns exec "$LAN_NS" ping -n -c2 -W1 "$ROUTER_IP" >/dev/null 2>&1 && a=1
    ip netns exec "$LAN_NS" ping -n -c2 -W1 "$WAN_IP" >/dev/null 2>&1 && b=1
    printf '%s\t%s\tLAN-router=%s\tWAN-target=%s\n' "$(date --iso-8601=ns)" "$label" "$a" "$b" >> "$OUT/health.tsv"
    if [[ $a -eq 1 && $b -eq 0 ]]; then
        echo
        warn "FORWARDING-AUSFALL erkannt: MW5-LAN erreichbar, WAN-Ziel nicht mehr erreichbar."
        router_snapshot "FAIL-$label"
        mark "FAIL-$label"
        return 1
    fi
}

run_test() {
    local label="$1" rc=0; shift
    router_snapshot "PRE-$label"
    mark "$label-start"
    info "Test $label"
    timeout "$((DURATION + 20))s" ip netns exec "$LAN_NS" \
        iperf3 -c "$WAN_IP" --connect-timeout 5000 --json "$@" \
        > "$OUT/iperf/$label.json" 2> "$OUT/iperf/$label.err" || rc=$?
    router_snapshot "POST-$label"
    mark "$label-end"

    # v7: ICMP can remain healthy while TCP payload is already corrupted; PRE/POST snapshots are already local.
    # Treat any iperf control/data failure as a first-class reproduction and
    # snapshot the router immediately instead of continuing the test blindly.
    if [[ $rc -ne 0 ]] || grep -q '"error"[[:space:]]*:' "$OUT/iperf/$label.json" 2>/dev/null; then
        warn "IPERF-/TCP-FEHLER erkannt in $label (rc=$rc)."
        router_snapshot "FAIL-IPERF-$label"
        mark "FAIL-IPERF-$label"
        health "$label" || true
        return 1
    fi

    health "$label"
}

info "Grundtest: ARP, IPv4 und IPv6 ..."
ip netns exec "$LAN_NS" ping -c5 "$ROUTER_IP" > "$OUT/pc/ping-router.txt" 2>&1 || true
ip netns exec "$LAN_NS" ping -c5 "$WAN_IP" > "$OUT/pc/ping-wan.txt" 2>&1 || true
command -v arping >/dev/null 2>&1 && \
    ip netns exec "$LAN_NS" arping -c5 -I "$LAN_VIF" "$ROUTER_IP" > "$OUT/pc/arping.txt" 2>&1 || true
ip netns exec "$LAN_NS" ping -6 -c3 "ff02::1%$LAN_VIF" > "$OUT/pc/ipv6-allnodes.txt" 2>&1 || true
router_snapshot AFTER-PROTOCOL-TEST

FAILED=0
for ((n=1; n<=CYCLES; n++)); do
    echo
    echo "========== Zyklus $n / $CYCLES =========="
    run_test "cycle-${n}-download" -P "$PARALLEL" -t "$DURATION" -R || { FAILED=1; break; }
    sleep 3
    run_test "cycle-${n}-upload" -P "$PARALLEL" -t "$DURATION" || { FAILED=1; break; }
    sleep 5
    health "cycle-${n}-complete" || { FAILED=1; break; }
done

if [[ $FAILED -eq 0 ]]; then
    for rate in 5M 15M 30M; do
        run_test "udp-${rate}-upload" -u -b "$rate" -t 20 || { FAILED=1; break; }
        run_test "udp-${rate}-download" -u -b "$rate" -t 20 -R || { FAILED=1; break; }
    done
fi

router_snapshot FINISHED
mark FINISHED

if [[ $FAILED -eq 1 ]]; then
    warn "Fehler wurde reproduziert. FAIL-Snapshot und PCAPs sind gespeichert."
else
    ok "Testlauf ohne erkannten Forwarding-Totalausfall beendet."
fi
