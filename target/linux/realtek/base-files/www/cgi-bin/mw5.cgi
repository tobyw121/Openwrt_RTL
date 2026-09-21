#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Tiny MW5 setup WebUI CGI. No Lua, no LuCI, no ubus/rpcd dependency.
set -u
PATH=/sbin:/bin:/usr/sbin:/usr/bin
MW5_WEBUI_VERSION='v44.66.12-dynamic-router-ui'
STATE_DIR=/tmp/mw5-webui
UI_PORT_MODE_OVERRIDE=''

read_post() {
	case "${REQUEST_METHOD:-GET}" in
		POST) dd bs=1 count="${CONTENT_LENGTH:-0}" 2>/dev/null ;;
		*) printf '%s' "${QUERY_STRING:-}" ;;
	esac
}
FORM_DATA="$(read_post)"
[ -n "${QUERY_STRING:-}" ] && FORM_DATA="$FORM_DATA&$QUERY_STRING"

url_decode() { printf '%b' "$(printf '%s' "$1" | sed 's/+/ /g; s/%/\\x/g')"; }
param() {
	name="$1"
	printf '%s' "&$FORM_DATA&" | tr '&' '\n' | sed -n "s/^$name=//p" | head -n1 | while IFS= read -r v; do url_decode "$v"; done
}
h() { sed 's/&/\&amp;/g; s/</\&lt;/g; s/>/\&gt;/g; s/"/\&quot;/g'; }
hs() { printf '%s' "$1" | h; }
checked() { [ "${1:-}" = "${2:-1}" ] && printf ' checked'; }
selected() { [ "${1:-}" = "${2:-}" ] && printf ' selected'; }

device_mac() { cat /sys/class/net/eth0/address 2>/dev/null | head -n1; }
device_suffix() {
	m="$(device_mac | tr -d ':' | tr '[:lower:]' '[:upper:]')"
	[ ${#m} -ge 4 ] && printf '%s' "${m#????????}" || printf 'ROUTER'
}
device_name() {
	n="$(uci -q get system.@system[0].hostname 2>/dev/null || true)"
	[ -n "$n" ] || n="$(hostname 2>/dev/null || true)"
	case "$n" in ''|OpenWrt|openwrt|MW5|mw5|Tenda*MW5*|*Nova*MW5*) n="Router-$(device_suffix)" ;; esac
	printf '%s' "$n"
}
device_model() {
	m="$(cat /tmp/sysinfo/model 2>/dev/null || true)"
	case "$m" in ''|*MW5*|*Nova*) m="Realtek RTL8197F Router" ;; esac
	printf '%s' "$m"
}

save_identity() {
	name="$(param device_name | tr -cd 'A-Za-z0-9._-' | cut -c1-32)"
	[ -n "$name" ] || name="Router-$(device_suffix)"
	uci -q set system.@system[0].hostname="$name" 2>/dev/null || true
	uci -q commit system 2>/dev/null || true
	hostname "$name" 2>/dev/null || true
	card_start "Gerätename"
	printf '<p class="okbox">Gerätename ist jetzt <strong>%s</strong>.</p>\n' "$(hs "$name")"
	card_end
}

wifi_station_macs() {
	if command -v iw >/dev/null 2>&1; then
		for w in wlan0 wlan1; do iw dev "$w" station dump 2>/dev/null | sed -n 's/^Station \([^ ]*\).*/\1/p'; done
	fi
}

lease_name_for_mac() {
	mac="$1"
	awk -v m="$mac" 'tolower($2)==tolower(m) { if ($4!="*") print $4; exit }' /tmp/dhcp.leases 2>/dev/null
}

lease_ip_for_mac() {
	mac="$1"
	awk -v m="$mac" 'tolower($2)==tolower(m) { print $3; exit }' /tmp/dhcp.leases 2>/dev/null
}

lan_clients_table() {
	wifi_macs="$(wifi_station_macs | tr '[:upper:]' '[:lower:]')"
	rows=0
	printf '<div class="client-table"><div class="client-head"><span>Gerät</span><span>IP</span><span>MAC</span><span>Anschluss</span><span>Status</span></div>'
	ip neigh show dev br-lan 2>/dev/null | while IFS= read -r line; do
		ipaddr="$(printf '%s\n' "$line" | awk '{print $1}')"
		mac="$(printf '%s\n' "$line" | sed -n 's/.* lladdr \([^ ]*\).*/\1/p')"
		state="$(printf '%s\n' "$line" | awk '{print $NF}')"
		[ -n "$mac" ] || continue
		name="$(lease_name_for_mac "$mac")"; [ -n "$name" ] || name="Unbekanntes Gerät"
		case " $wifi_macs " in *" $(printf '%s' "$mac" | tr '[:upper:]' '[:lower:]') "*) port='WLAN' ;; *) port='LAN Port' ;; esac
		printf '<div class="client-row"><strong>%s</strong><span>%s</span><code>%s</code><span class="port-pill">%s</span><span>%s</span></div>' "$(hs "$name")" "$(hs "$ipaddr")" "$(hs "$mac")" "$port" "$(hs "$state")"
	done
	printf '</div>'
}

port_overview() {
	lan_carrier="$(cat /sys/class/net/lan/carrier 2>/dev/null || echo 0)"
	wan_carrier="$(cat /sys/class/net/wan/carrier 2>/dev/null || echo 0)"
	lan_speed="$(cat /sys/class/net/lan/speed 2>/dev/null || true)"; [ -n "$lan_speed" ] || lan_speed='?'
	wan_speed="$(cat /sys/class/net/wan/speed 2>/dev/null || true)"; [ -n "$wan_speed" ] || wan_speed='?'
	wan_ip="$(ip -4 addr show dev wan 2>/dev/null | sed -n 's/.*inet \([^ /]*\).*/\1/p' | head -n1)"
	p1_in="$(sed -n 's/^port1_in_packets=//p' /proc/mw5-rtl8367 2>/dev/null | head -n1)"
	p1_out="$(sed -n 's/^port1_out_packets=//p' /proc/mw5-rtl8367 2>/dev/null | head -n1)"
	p3_in="$(sed -n 's/^port3_in_packets=//p' /proc/mw5-rtl8367 2>/dev/null | head -n1)"
	p3_out="$(sed -n 's/^port3_out_packets=//p' /proc/mw5-rtl8367 2>/dev/null | head -n1)"
	printf '<div class="port-grid">'
	printf '<div class="port-card %s"><div class="port-icon">LAN</div><div><span class="kicker">LAN Port · RTL8367 P1</span><h3>%s</h3><p>%s Mbit/s · RX %s · TX %s</p></div></div>' "$([ "$lan_carrier" = 1 ] && echo linked || echo down)" "$([ "$lan_carrier" = 1 ] && echo Verbunden || echo Kein Link)" "$(hs "$lan_speed")" "$(hs "${p1_in:-0}")" "$(hs "${p1_out:-0}")"
	printf '<div class="port-card %s"><div class="port-icon">WAN</div><div><span class="kicker">WAN Port · RTL8367 P3</span><h3>%s</h3><p>%s Mbit/s · IPv4 %s · RX %s · TX %s</p></div></div>' "$([ "$wan_carrier" = 1 ] && echo linked || echo down)" "$([ "$wan_carrier" = 1 ] && echo Verbunden || echo Kein Link)" "$(hs "$wan_speed")" "$(hs "${wan_ip:-keine}")" "$(hs "${p3_in:-0}")" "$(hs "${p3_out:-0}")"
	printf '</div>'
}

page_begin() {
	DEVNAME="$(device_name)"
	DEVMODEL="$(device_model)"
	printf 'Content-Type: text/html; charset=utf-8\r\nCache-Control: no-store, no-cache, must-revalidate\r\nPragma: no-cache\r\n\r\n'
	cat <<HTML
<!doctype html><html lang="de"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<meta name="color-scheme" content="light dark"><title>$(hs "$DEVNAME") · Router Control</title><link rel="stylesheet" href="/mw5.css">
<script>
function mw5ToggleStatic(){var s=document.querySelector('input[name="mode"]:checked');var box=document.getElementById('static-wan');if(box)box.hidden=!s||s.value!=='router-static';document.querySelectorAll('.mode-card').forEach(function(c){c.classList.toggle('selected',!!c.querySelector('input:checked'));});}
function mw5Submitting(b){if(!b)return true;b.disabled=true;b.dataset.old=b.textContent;b.textContent='Wird angewendet …';return true;}
function mw5Countdown(){document.querySelectorAll('[data-expires]').forEach(function(e){var left=Math.max(0,parseInt(e.dataset.expires,10)-Math.floor(Date.now()/1000));e.textContent=left+' s';});}
window.addEventListener('DOMContentLoaded',function(){document.querySelectorAll('input[name="mode"]').forEach(function(e){e.addEventListener('change',mw5ToggleStatic)});mw5ToggleStatic();mw5Countdown();setInterval(mw5Countdown,1000);});
</script></head><body><main>
<header class="hero"><div><span class="eyebrow">OpenWrt 24.10 · RTL8197F</span><h1>$(hs "$DEVNAME")</h1><p>$(hs "$DEVMODEL") · Netzwerk, WLAN und Diagnose · ${MW5_WEBUI_VERSION}</p></div><div class="hero-chip">$(printf '%s' "$DEVNAME" | cut -c1-2 | tr '[:lower:]' '[:upper:]' | h)</div></header>
<nav><a href="/cgi-bin/mw5.cgi">Übersicht</a><a href="/cgi-bin/mw5.cgi?action=ports">Ports &amp; Geräte</a><a href="/cgi-bin/mw5.cgi?action=wifi">WLAN</a><a href="/cgi-bin/mw5.cgi?action=mesh">Mesh</a><a href="/cgi-bin/mw5.cgi?action=diag">Diagnose</a></nav>
HTML
}

page_end() { cat <<'HTML'
<footer>Router-WebUI · LAN/Setup-Zugriff. SSH und Serial bleiben für Recovery aktiv.</footer></main></body></html>
HTML
}
card_start() { printf '<section class="card"><h2>%s</h2>\n' "$(hs "$1")"; }
card_end() { printf '</section>\n'; }
pre_text() { text="$1"; [ -n "$text" ] || text='(keine Ausgabe)'; printf '<pre>%s</pre>' "$(printf '%s' "$text" | h)"; }
pre_cmd() { out="$({ "$@"; } 2>&1)"; pre_text "$out"; }
badge() { printf '<span class="badge %s">%s</span>' "$(hs "$1")" "$(hs "$2")"; }

overlay_warning() {
	if mount 2>/dev/null | grep -q ' on /overlay type tmpfs '; then
		card_start "Overlay nur temporaer"
		printf '<p class="warn">OpenWrt nutzt aktuell ein tmpfs-Overlay. WebUI-Aenderungen werden jetzt direkt in Runtime-Dateien und /etc/config geschrieben, sind aber ohne funktionierendes persistentes Overlay nach einem Reboot wieder weg.</p>\n'
		card_end
	fi
}

current_port_mode() {
	if [ -n "${UI_PORT_MODE_OVERRIDE:-}" ]; then printf '%s' "$UI_PORT_MODE_OVERRIDE"; return; fi
	mode="$(mw5-portmode show 2>/dev/null | sed -n 's/^mode=//p' | head -n1)"
	[ -n "$mode" ] || mode=safe
	case "$mode" in router|router-dhcp) mode=safe ;; setup|both-lan|rescue|isolated) mode=offline ;; bridge) mode=ap ;; static) mode=router-static ;; esac
	printf '%s' "$mode"
}

pending_port_box() {
	mkdir -p "$STATE_DIR" 2>/dev/null || true
	found=0
	for f in "$STATE_DIR"/pending-*; do
		[ -f "$f" ] || continue
		token="${f##*/pending-}"
		[ -n "$token" ] || continue
		created="$(sed -n 's/^created=//p' "$f" | head -n1)"; timeout="$(sed -n 's/^timeout=//p' "$f" | head -n1)"
		case "$created:$timeout" in *[!0-9:]*) expires=0 ;; *) expires=$((created + timeout)) ;; esac
		if [ "$found" -eq 0 ]; then
			printf '<section class="confirm-panel pending-top"><div class="confirm-head"><div><span class="kicker">Sicherheits-Rollback aktiv</span><h2>Funktioniert die neue Netzwerkkonfiguration?</h2></div><div class="countdown" data-expires="%s">…</div></div>\n' "$expires"
			printf '<p>Nur bei einem bewusst gestarteten Sicherheitstest nötig. Wenn du nichts bestätigst, wird die vorherige Konfiguration automatisch wiederhergestellt.</p>\n'
			found=1
		fi
		printf '<div class="button-row"><form method="post" action="/cgi-bin/mw5.cgi?action=ports"><input type="hidden" name="op" value="confirm_ports"><input type="hidden" name="token" value="%s"><button class="ok">Neue Konfiguration behalten</button></form>' "$(hs "$token")"
		printf '<form method="post" action="/cgi-bin/mw5.cgi?action=ports"><input type="hidden" name="op" value="rollback_ports"><input type="hidden" name="token" value="%s"><button class="danger">Jetzt zurückrollen</button></form></div>' "$(hs "$token")"
		printf '<p class="muted token-line">Token: <code>%s</code></p>\n' "$(hs "$token")"
	done
	[ "$found" -eq 0 ] || printf '</section>\n'
}

apply_ports() {
	mode="$(param mode)"; [ -n "$mode" ] || mode=safe
	strategy="$(param apply_strategy)"
	mkdir -p "$STATE_DIR" 2>/dev/null || true
	set -- mw5-portmode apply "$mode" --web
	if [ "$strategy" = rollback ]; then set -- "$@" --timeout 300; else set -- "$@" --no-rollback; fi
	if [ "$mode" = router-static ]; then
		set -- "$@" --wan-ip "$(param wan_ip)" --wan-netmask "$(param wan_netmask)" --wan-gateway "$(param wan_gateway)" --wan-dns "$(param wan_dns)"
	fi
	out="$("$@" 2>&1)"; rc=$?
	applied="$(printf '%s\n' "$out" | sed -n 's/^applied=//p' | tail -n1)"
	effective="$(printf '%s\n' "$out" | sed -n 's/^effective=//p' | tail -n1)"
	verified="$(printf '%s\n' "$out" | sed -n 's/^verified=//p' | tail -n1)"
	[ -n "$applied" ] || applied="$mode"
	[ -n "$effective" ] || effective="$applied"
	UI_PORT_MODE_OVERRIDE="$applied"
	token="$(printf '%s\n' "$out" | sed -n 's/^token=//p' | head -n1)"
	card_start "Netzwerkmodus angewendet"
	if [ "$rc" -eq 0 ] && [ "$verified" = 1 ]; then
		printf '<p class="okbox"><strong>Übernommen und geprüft.</strong> Gespeichert: '; badge ok "$applied"; printf ' · Effektiv: '; badge info "$effective"; printf '</p>\n'
	else
		printf '<p class="badbox"><strong>Apply fehlgeschlagen oder Readback inkonsistent.</strong> RC %s, verified=%s. Die Details stehen unten.</p>\n' "$rc" "$(hs "$verified")"
	fi
	case "$effective" in
		safe|router) printf '<p class="notice">Safe Internet aktiv: WAN ist ein getrennter DHCP-Client hinter Firewall/NAT. Der Router-DHCP-Server bleibt ausschließlich auf LAN/WLAN.</p>\n' ;;
		offline) printf '<p class="notice">Offline/Rescue aktiv: WAN ist physisch getrennt und besitzt kein IP-Protokoll.</p>\n' ;;
		ap) printf '<p class="notice">AP/Bridge aktiv: Router-DHCP ist aus. Nur dein Hauptnetz vergibt DHCP-Adressen. Die Management-IP kann sich dadurch ändern.</p>\n' ;;
		router-static) printf '<p class="notice">Statisches WAN aktiv: kein DHCP-Client auf WAN; NAT und Firewall bleiben aktiv.</p>\n' ;;
	esac
	if [ "$strategy" = rollback ] && [ -n "$token" ]; then
		printf '<p class="warn"><strong>Sicherheitstest läuft.</strong> Bestätige unten innerhalb von 300 Sekunden, sonst wird automatisch zurückgerollt.</p>\n'
	elif [ "$rc" -eq 0 ]; then
		printf '<p class="okbox">Die Änderung wurde sofort dauerhaft übernommen; kein automatischer Rückroll-Timer läuft.</p>\n'
	fi
	pre_text "$out"
	card_end
}

confirm_ports() {
	token="$(param token)"
	card_start "Netzwerkänderung bestätigen"
	if [ -n "$token" ]; then
		out="$(mw5-portmode confirm "$token" 2>&1)"; rc=$?
		pre_text "$out"
		if [ "$rc" -eq 0 ]; then printf '<p class="okbox">Die neue Port-Konfiguration ist jetzt dauerhaft bestätigt.</p>\n'; else printf '<p class="badbox">Bestätigung fehlgeschlagen (RC %s). Der Token war eventuell bereits abgelaufen.</p>\n' "$rc"; fi
	else
		printf '<p class="warn">Kein Token übergeben.</p>\n'
	fi
	UI_PORT_MODE_OVERRIDE="$(current_port_mode)"
	card_end
}
rollback_ports() {
	token="$(param token)"
	card_start "Portmodus Rollback"
	if [ -n "$token" ]; then pre_cmd mw5-portmode rollback "$token"; else printf '<p class="warn">Kein Token uebergeben.</p>\n'; fi
	UI_PORT_MODE_OVERRIDE="$(current_port_mode)"
	card_end
}
repair_ports() {
	card_start "Netzwerkkonfiguration reparieren"
	out="$(mw5-portmode repair-current 2>&1)"; rc=$?
	pre_text "$out"
	if [ "$rc" -eq 0 ] && printf '%s\n' "$out" | grep -q 'verified=1'; then
		printf '<p class="okbox">Gespeicherter Modus und reale UCI/Bridge-Konfiguration wurden neu aufgebaut und geprüft.</p>\n'
	else
		printf '<p class="badbox">Die automatische Reparatur war nicht vollständig erfolgreich (RC %s).</p>\n' "$rc"
	fi
	UI_PORT_MODE_OVERRIDE="$(current_port_mode)"
	card_end
}

wifi_get() { mw5-wifi get "$1" 2>/dev/null || true; }

save_wifi() {
	mkdir -p "$STATE_DIR" 2>/dev/null || true
	[ "$(param wifi_enabled)" = 1 ] && en=1 || en=0
	[ "$(param r2_enabled)" = 1 ] && r2=1 || r2=0
	[ "$(param r5_enabled)" = 1 ] && r5=1 || r5=0
	if [ "$(param apply_now)" = 1 ]; then
		en=1
		if [ "$r2" = 0 ] && [ "$r5" = 0 ]; then r2=1; r5=1; fi
	fi
	SAVE_OUT="$STATE_DIR/wifi-save.out"
	: >"$SAVE_OUT"
	printf 'webui-submit enabled=%s 2g=%s 5g=%s apply_now=%s autoload=%s\n' "$en" "$r2" "$r5" "$(param apply_now)" "$(param autoload)" >>"$SAVE_OUT"
	mw5-wifi save \
		--enabled "$en" --autoload "$(param autoload)" --regdomain "$(param regdomain)" --bridge "$(param bridge)" \
		--2g-enabled "$r2" --2g-ssid "$(param r2_ssid)" --2g-channel "$(param r2_channel)" --2g-width "$(param r2_width)" --2g-encryption "$(param r2_encryption)" --2g-key "$(param r2_key)" \
		--5g-enabled "$r5" --5g-ssid "$(param r5_ssid)" --5g-channel "$(param r5_channel)" --5g-width "$(param r5_width)" --5g-encryption "$(param r5_encryption)" --5g-key "$(param r5_key)" >>"$SAVE_OUT" 2>&1
	save_rc=$?
	rb_en="$(wifi_get general.enabled)"; rb_r2="$(wifi_get radio2g.enabled)"; rb_r5="$(wifi_get radio5g.enabled)"
	printf 'readback enabled=%s 2g=%s 5g=%s save_rc=%s expected=%s:%s:%s\n' "$rb_en" "$rb_r2" "$rb_r5" "$save_rc" "$en" "$r2" "$r5" >>"$SAVE_OUT"
	if [ "$(param apply_now)" = 1 ]; then
		mw5-wifi apply >>"$SAVE_OUT" 2>&1
		apply_rc=$?
	else
		apply_rc=0
	fi
	card_start "WLAN gespeichert"
	if [ "$save_rc" -eq 0 ] && [ "$rb_en:$rb_r2:$rb_r5" = "$en:$r2:$r5" ]; then
		printf '<p class="okbox">WLAN-Einstellungen wurden gespeichert und korrekt zurueckgelesen.</p>\n'
	else
		printf '<p class="badbox">WLAN-Speichern/Readback passt noch nicht. Details unten.</p>\n'
	fi
	if [ "$(param apply_now)" = 1 ]; then
		[ "$apply_rc" -eq 0 ] && printf '<p class="okbox">WLAN-Konfiguration wurde geschrieben; Treiberload ist wegen Hard-Hang blockiert. Status siehe unten.</p>\n' || printf '<p class="badbox">WLAN-Aktivierung meldete Fehlercode %s. Log siehe unten.</p>\n' "$apply_rc"
	fi
	pre_text "$(cat "$SAVE_OUT" 2>/dev/null || true)"
	printf '<h3>Aktueller WLAN-Status</h3>'
	pre_cmd mw5-wifi status
	card_end
}

apply_wifi_now() {
	card_start "WLAN Konfiguration"
	pre_cmd mw5-wifi apply
	printf '<h3>Aktueller WLAN-Status</h3>'
	pre_cmd mw5-wifi status
	card_end
}
stop_wifi_now() {
	card_start "WLAN gestoppt"
	pre_cmd mw5-wifi stop
	pre_cmd mw5-wifi status
	card_end
}

save_mesh() {
	[ "$(param mesh_enabled)" = 1 ] && men=1 || men=0
	out="$(mw5-wifi save --mesh-enabled "$men" --mesh-mode "$(param mesh_mode)" --mesh-id "$(param mesh_id)" --mesh-backhaul "$(param mesh_backhaul)" --mesh-pairing "$(param mesh_pairing)" --mesh-key "$(param mesh_key)" 2>&1)"
	card_start "Mesh vorbereitet"
	printf '<p>Mesh-/Pairing-Einstellungen sind gespeichert. Realtek/Tenda-Mesh bleibt bis zum Zwei-Knoten-Hardwaretest experimentell.</p>'
	pre_text "$out"
	card_end
}

status_page() {
	pm="$(mw5-portmode show 2>/dev/null || true)"
	mode="$(printf '%s\n' "$pm" | sed -n 's/^mode=//p' | head -n1)"
	eff="$(printf '%s\n' "$pm" | sed -n 's/^effective_mode=//p' | head -n1)"
	iso="$(printf '%s\n' "$pm" | sed -n 's/^wan_isolation=//p' | head -n1)"
	dhcp="$(printf '%s\n' "$pm" | sed -n 's/^lan_dhcp_server=//p' | head -n1)"
	card_start "Systemübersicht"
	printf '<div class="identity-row"><div><span class="kicker">Gerät</span><h2 class="device-title">%s</h2><p class="muted">%s</p></div><form method="post" action="/cgi-bin/mw5.cgi"><input type="hidden" name="op" value="save_identity"><label>Gerätename<input name="device_name" maxlength="32" value="%s"></label><button>Umbenennen</button></form></div>' "$(hs "$(device_name)")" "$(hs "$(device_model)")" "$(hs "$(device_name)")"
	printf '<div class="stat-grid"><div class="stat"><span>Portmodus</span><strong>%s</strong></div><div class="stat"><span>Effektiv</span><strong>%s</strong></div><div class="stat"><span>WAN-Schutz</span><strong>%s</strong></div><div class="stat"><span>LAN DHCP</span><strong>%s</strong></div></div>' "$(hs "$mode")" "$(hs "$eff")" "$(hs "$iso")" "$(hs "$dhcp")"
	printf '<h3>Physische Ports</h3>'; port_overview
	printf '<h3>Verbundene Geräte</h3><p class="muted">Geräte am physischen LAN-Port werden aus Neighbor/DHCP-Daten erkannt; bekannte WLAN-Stationen werden getrennt markiert.</p>'; lan_clients_table
	printf '<details class="readback"><summary>Technischen Netzwerkstatus anzeigen</summary>'; pre_text "$pm"; pre_cmd sh -c 'ip -4 addr show 2>/dev/null; echo; ip -4 route show 2>/dev/null'; printf '</details>'
	card_end
}

ports_page() {
	mode="$(current_port_mode)"
	wan_ip="$(uci -q get mw5webui.system.wan_ip 2>/dev/null || true)"
	wan_mask="$(uci -q get mw5webui.system.wan_netmask 2>/dev/null || echo 255.255.255.0)"
	wan_gw="$(uci -q get mw5webui.system.wan_gateway 2>/dev/null || true)"
	wan_dns="$(uci -q get mw5webui.system.wan_dns 2>/dev/null || true)"
	pm="$(mw5-portmode show 2>/dev/null || true)"
	wan_live="$(printf '%s\n' "$pm" | sed -n 's/^wan_ipv4_live=//p' | head -n1)"
	consistent="$(printf '%s\n' "$pm" | sed -n 's/^config_consistent=//p' | head -n1)"
	wan_carrier="$(printf '%s\n' "$pm" | sed -n 's/^wan_carrier=//p' | head -n1)"
	default_route="$(printf '%s\n' "$pm" | sed -n 's/^default_route=//p' | head -n1)"
	if [ -n "$wan_live" ] && [ -n "$default_route" ]; then internet_state='online'; elif [ "$wan_carrier" = 1 ]; then internet_state='wartet auf WAN-IP'; else internet_state='offline'; fi
	pending_port_box
	card_start "Ports, Geräte & Netzwerkmodus"
	port_overview
	printf '<h3>Geräte am LAN/WLAN</h3>'; lan_clients_table
	printf '<div class="status-strip"><div><span>Modus</span><strong>%s</strong></div><div><span>WAN Link</span><strong>%s</strong></div><div><span>WAN IPv4</span><strong>%s</strong></div><div><span>Internet</span><strong>%s</strong></div><div><span>Konfiguration</span><strong>%s</strong></div></div>' "$(hs "$mode")" "$(hs "${wan_carrier:-?}")" "$(hs "${wan_live:-keine}")" "$(hs "$internet_state")" "$([ "$consistent" = 1 ] && printf 'konsistent' || printf 'prüfen')"
	if [ "$consistent" != 1 ]; then
		printf '<div class="badbox inline-action"><div><strong>Portmodus und UCI-Konfiguration passen nicht zusammen.</strong><br><small>Das kann nach einem alten Rollback/Upgrade passieren. Die Reparatur baut den gespeicherten Modus deterministisch neu auf.</small></div><form method="post" action="/cgi-bin/mw5.cgi?action=ports"><input type="hidden" name="op" value="repair_ports"><button class="secondary">Konfiguration reparieren</button></form></div>'
	fi
	printf '<p class="lead">Im empfohlenen <strong>Safe Internet</strong>-Modus bleiben WAN und LAN/WLAN auf Layer 2 getrennt. WAN holt sich nur als <strong>DHCP-Client</strong> eine Adresse vom Hauptnetz; der Router-DHCP-Server ist ausschließlich auf dem privaten LAN/WLAN aktiv. Dadurch gibt es Internet ohne doppelten DHCP im Hauptnetz.</p>'
	cat <<HTML
<form method="post" action="/cgi-bin/mw5.cgi?action=ports"><input type="hidden" name="op" value="apply_ports">
<div class="mode-grid mode-grid-clean">
<label class="mode-card"><input type="radio" name="mode" value="safe"$(checked "$mode" safe)><span><span class="mode-title"><strong>Safe Internet · isoliert</strong><em>Empfohlen</em></span><small>WAN: DHCP-Client + NAT/Firewall. LAN/WLAN: 192.168.1.1 mit eigenem DHCP. Keine Layer-2-DHCP-Leaks zum Hauptnetz.</small></span></label>
<label class="mode-card"><input type="radio" name="mode" value="router-static"$(checked "$mode" router-static)><span><span class="mode-title"><strong>WAN statisch</strong></span><small>Wie Safe Internet, aber ohne DHCP-Client am WAN. IP, Gateway und DNS werden manuell gesetzt.</small></span></label>
<label class="mode-card"><input type="radio" name="mode" value="ap"$(checked "$mode" ap)><span><span class="mode-title"><strong>AP / Hauptnetz-DHCP</strong></span><small>LAN + WAN + WLAN werden gebridged. Der Router-DHCP-Server ist aus; ausschließlich dein Hauptrouter vergibt Adressen.</small></span></label>
<label class="mode-card"><input type="radio" name="mode" value="offline"$(checked "$mode" offline)><span><span class="mode-title"><strong>Offline / Rescue</strong></span><small>Nur LAN + WLAN lokal. WAN bleibt getrennt mit <code>proto none</code>. Kein Internet über WAN.</small></span></label>
</div>
<fieldset id="static-wan" class="subpanel" hidden><legend>Statische WAN-Konfiguration</legend><div class="form-grid">
<label>WAN IPv4<input name="wan_ip" inputmode="decimal" placeholder="192.168.0.2" value="$(hs "$wan_ip")"></label>
<label>Netzmaske<input name="wan_netmask" inputmode="decimal" value="$(hs "$wan_mask")"></label>
<label>Gateway<input name="wan_gateway" inputmode="decimal" placeholder="192.168.0.1" value="$(hs "$wan_gw")"></label>
<label>DNS <small>(Leerzeichen getrennt)</small><input name="wan_dns" placeholder="1.1.1.1 9.9.9.9" value="$(hs "$wan_dns")"></label>
</div></fieldset>
<div class="apply-box"><div><strong>Änderung übernehmen</strong><p>„Anwenden &amp; behalten“ ist der normale Weg. Der Sicherheitstest ist nur sinnvoll, wenn du bei einer riskanten Änderung einen automatischen 5-Minuten-Rollback möchtest.</p></div><div class="button-row"><button type="submit" name="apply_strategy" value="keep" onclick="return mw5Submitting(this)">Anwenden &amp; behalten</button><button type="submit" class="secondary" name="apply_strategy" value="rollback" onclick="return mw5Submitting(this)">5-Min-Sicherheitstest</button></div></div>
</form>
HTML
	printf '<details class="readback"><summary>Technischen Readback anzeigen</summary>'
	pre_text "$pm"
	pre_cmd sh -c 'uci -q show network | grep -E "network\\.(br_lan|lan|wan|wan_dev)" || true; echo; uci -q show dhcp.lan || true; echo; uci -q show dhcp.wan || true'
	printf '</details>'
	card_end
}

wifi_status_summary() {
	gen="$(wifi_get general.enabled)"; r2="$(wifi_get radio2g.enabled)"; r5="$(wifi_get radio5g.enabled)"
	printf '<p>Aktueller gespeicherter WLAN-Zustand: '
	[ "$gen" = 1 ] && badge ok 'global aktiv' || badge warn 'global aus'
	printf ' '
	[ "$r2" = 1 ] && badge ok '2.4 GHz aktiv' || badge warn '2.4 GHz aus'
	printf ' '
	[ "$r5" = 1 ] && badge ok '5 GHz aktiv' || badge warn '5 GHz aus'
	printf '</p>\n'
	last_rc="$(wifi_get general.last_apply_rc)"
	[ -n "$last_rc" ] && printf '<p class="muted">Letzter Apply-RC: <code>%s</code></p>\n' "$(hs "$last_rc")"
}

wifi_page() {
	mw5-wifi init >/dev/null 2>&1 || true
	gen="$(wifi_get general.enabled)"; autoload="$(wifi_get general.autoload)"; reg="$(wifi_get general.regdomain)"; bridge="$(wifi_get general.bridge)"
	r2en="$(wifi_get radio2g.enabled)"; r2ssid="$(wifi_get radio2g.ssid)"; r2ch="$(wifi_get radio2g.channel)"; r2w="$(wifi_get radio2g.width)"; r2e="$(wifi_get radio2g.encryption)"
	r5en="$(wifi_get radio5g.enabled)"; r5ssid="$(wifi_get radio5g.ssid)"; r5ch="$(wifi_get radio5g.channel)"; r5w="$(wifi_get radio5g.width)"; r5e="$(wifi_get radio5g.encryption)"
	card_start "WLAN"
	wifi_status_summary
	cat <<HTML
<p>Der Router steuert beide Radios ueber <code>cfg80211/nl80211</code> und <code>hostapd</code>: 2.4 GHz ist der integrierte RTL8197FS auf <code>wlan1</code> (RFE5), 5 GHz der physische PCIe-RTL8812BRH auf <code>wlan0</code> (RFE6, PCI <code>10ec:b822</code>; der rekonstruierte Vendor-Treiber verwendet intern den 8822B-HAL-Pfad). Die Standard-SSIDs bleiben <code>Openwrt_2,4GHz</code> und <code>Openwrt_5GHz</code>. AP-Autostart bleibt aus Sicherheitsgruenden opt-in, bis dieser Build auf der konkreten Hardware getestet wurde.</p>
<form method="post" action="/cgi-bin/mw5.cgi?action=wifi"><input type="hidden" name="op" value="save_wifi">
<label><input type="checkbox" name="wifi_enabled" value="1"$(checked "$gen" 1)> WLAN global aktivieren</label>
<label><input type="checkbox" name="autoload" value="1"$(checked "$autoload" 1)> APs beim Boot via hostapd/nl80211 starten</label>
<div class="grid"><label>Regdomain ISO-Alpha2 (00=world)<input name="regdomain" value="$(hs "$reg")"></label><label>Bridge<input name="bridge" value="$(hs "$bridge")"></label></div>
<h3>2.4 GHz / wlan1 / RTL8197FS</h3>
<label><input type="checkbox" name="r2_enabled" value="1"$(checked "$r2en" 1)> 2.4 GHz aktiv</label>
<div class="grid"><label>SSID<input name="r2_ssid" value="$(hs "$r2ssid")"></label><label>Kanal<input name="r2_channel" value="$(hs "$r2ch")"></label><label>Breite<select name="r2_width"><option$(selected "$r2w" 20)>20</option><option$(selected "$r2w" 40)>40</option></select></label><label>Verschluesselung<select name="r2_encryption"><option value="psk2"$(selected "$r2e" psk2)>WPA2-PSK</option><option value="mixed"$(selected "$r2e" mixed)>WPA/WPA2 mixed</option><option value="open"$(selected "$r2e" open)>Offen</option></select></label><label>Passphrase<input type="password" name="r2_key" value="" placeholder="leer = gespeicherten/default Key behalten"></label></div>
<h3>5 GHz / wlan0 / RTL8812BRH</h3>
<label><input type="checkbox" name="r5_enabled" value="1"$(checked "$r5en" 1)> 5 GHz aktiv</label>
<div class="grid"><label>SSID<input name="r5_ssid" value="$(hs "$r5ssid")"></label><label>Kanal<input name="r5_channel" value="$(hs "$r5ch")"></label><label>Breite<select name="r5_width"><option$(selected "$r5w" 20)>20</option><option$(selected "$r5w" 40)>40</option><option$(selected "$r5w" 80)>80</option></select></label><label>Verschluesselung<select name="r5_encryption"><option value="psk2"$(selected "$r5e" psk2)>WPA2-PSK</option><option value="mixed"$(selected "$r5e" mixed)>WPA/WPA2 mixed</option><option value="open"$(selected "$r5e" open)>Offen</option></select></label><label>Passphrase<input type="password" name="r5_key" value="" placeholder="leer = gespeicherten/default Key behalten"></label></div>
<div class="button-row"><button name="save_only" value="1">Nur speichern</button><button class="ok" name="apply_now" value="1">Speichern &amp; anwenden (cfg80211)</button></div></form>
<div class="button-row"><form method="post" action="/cgi-bin/mw5.cgi?action=wifi"><input type="hidden" name="op" value="apply_wifi_now"><button class="ok">Gespeicherte WLAN-Konfiguration via hostapd anwenden</button></form><form method="post" action="/cgi-bin/mw5.cgi?action=wifi"><input type="hidden" name="op" value="stop_wifi"><button class="danger">WLAN Interfaces stoppen</button></form></div>
HTML
	printf '<h3>Letztes WLAN-Apply-Log</h3>'
	pre_text "$(cat /tmp/mw5-webui/wlan-apply.log 2>/dev/null | tail -n 80 || true)"
	card_end
}

mesh_page() {
	mw5-wifi init >/dev/null 2>&1 || true
	men="$(wifi_get mesh.enabled)"; mmode="$(wifi_get mesh.mode)"; mid="$(wifi_get mesh.mesh_id)"; back="$(wifi_get mesh.backhaul)"; pair="$(wifi_get mesh.pairing)"
	card_start "Mesh / Tenda Nova Pairing"
	cat <<HTML
<p>Vorbereitung fuer Mesh: Einstellungen werden gespeichert. Mesh-Pairing bleibt experimentell, solange zwei Router-Knoten nicht gegeneinander validiert sind.</p>
<form method="post" action="/cgi-bin/mw5.cgi?action=mesh"><input type="hidden" name="op" value="save_mesh">
<label><input type="checkbox" name="mesh_enabled" value="1"$(checked "$men" 1)> Mesh vorbereiten</label>
<div class="grid"><label>Modus<select name="mesh_mode"><option value="prepared"$(selected "$mmode" prepared)>Vorbereitet / deaktiviert</option><option value="openwrt-80211s"$(selected "$mmode" openwrt-80211s)>OpenWrt 802.11s vorbereitet</option><option value="realtek-vendor"$(selected "$mmode" realtek-vendor)>Realtek Vendor Mesh vorbereitet</option><option value="tenda-nova"$(selected "$mmode" tenda-nova)>Tenda Nova Pairing vorbereitet</option></select></label><label>Mesh-ID<input name="mesh_id" value="$(hs "$mid")"></label><label>Backhaul<select name="mesh_backhaul"><option value="5g"$(selected "$back" 5g)>5 GHz bevorzugt</option><option value="2g"$(selected "$back" 2g)>2.4 GHz</option><option value="auto"$(selected "$back" auto)>Auto</option></select></label><label>Pairing<select name="mesh_pairing"><option value="manual"$(selected "$pair" manual)>Manuell</option><option value="button"$(selected "$pair" button)>Button-Hotplug vorbereitet</option><option value="tenda"$(selected "$pair" tenda)>Tenda-kompatibel vorbereitet</option></select></label><label>Mesh Key<input type="password" name="mesh_key" value=""></label></div>
<button>Mesh-Einstellungen speichern</button></form>
HTML
	card_end
}

diag_page() {
	card_start "Diagnose"
	printf '<p>Minimaldiagnose ohne Kernel-Hexdump-Spam.</p>'
	printf '<h3>Ports</h3>'; pre_cmd mw5-portmode show
	printf '<h3>WLAN</h3>'; pre_cmd mw5-wifi status
	printf '<h3>Switch / RTL8363NB</h3>'; printf '<p class="muted">Treiberintern: rtl8365mb / RTL8367-kompatibler Registerpfad.</p>'; pre_cmd sh -c 'cat /proc/mw5-rtl8367 2>/dev/null || true'
	card_end
}

op="$(param op)"
action="$(param action)"
page_begin
overlay_warning
case "$op" in
	save_identity) save_identity ;;
	apply_ports) apply_ports ;;
	confirm_ports) confirm_ports ;;
	rollback_ports) rollback_ports ;;
	repair_ports) repair_ports ;;
	save_wifi) save_wifi ;;
	apply_wifi_now) apply_wifi_now ;;
	stop_wifi) stop_wifi_now ;;
	save_mesh) save_mesh ;;
esac
case "$action" in
	ports) ports_page ;;
	wifi) wifi_page ;;
	mesh) mesh_page ;;
	diag) diag_page ;;
	*) status_page ;;
esac
page_end
