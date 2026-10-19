#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Exercise the real MW5 hostapd writers without touching host networking."""
from pathlib import Path
import shlex
import subprocess
import tempfile
import unittest

from test_mw5_stability import BASE, shell_function

WIFI = BASE / "usr/sbin/mw5-wifi"


def definition(name):
    line = next(line for line in WIFI.read_text().splitlines()
                if line.startswith(name + "() {"))
    return line if line.endswith("}") else shell_function(WIFI, name)


def shell(source, command):
    # Match mw5-wifi's nounset mode; it intentionally does not use errexit.
    return subprocess.run(["sh", "-u", "-c", "DEFAULT_KEY=test-password\n" + source + "\n" + command],
                          check=True, text=True, capture_output=True).stdout


class WifiCapabilitiesTests(unittest.TestCase):
    def config(self, band, width, channel=None, profile="auto", sgi="1"):
        funcs = ["sanitize_bool", "sanitize_2g_profile", "valid_psk",
                 "write_security", "vht_center80", "write_ht_capabilities",
                 "write_hostapd_config"]
        source = "\n".join(definition(name) for name in funcs)
        channel = channel or (6 if band == "2g" else 40)
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "hostapd.conf"
            shell(source, f"RUN_DIR={shlex.quote(tmp)}; BRIDGE=nonexistent-test-bridge; "
                  f"REGDOMAIN=DE; R2_PROFILE={shlex.quote(profile)}; "
                  f"R2_SHORT_GI={sgi}; R5_SHORT_GI={sgi}; "
                  f"write_hostapd_config {band} wlan-test test-ssid {channel} {width} "
                  f"psk2 test-password {shlex.quote(str(path))}")
            lines = path.read_text().splitlines()
        # Duplicate keys could silently discard SGI or HT40 configuration.
        keys = [line.split("=", 1)[0] for line in lines]
        self.assertEqual(len(keys), len(set(keys)))
        return dict(line.split("=", 1) for line in lines)

    def test_2g_ht20_advertises_sgi_without_widening_channel(self):
        conf = self.config("2g", 20)
        self.assertEqual(conf["ht_capab"], "[SHORT-GI-20]")
        self.assertEqual(conf["ieee80211n"], "1")
        self.assertEqual(conf["wmm_enabled"], "1")
        self.assertEqual(conf["rsn_pairwise"], "CCMP")
        self.assertNotIn("ieee80211ac", conf)

    def test_2g_ht40_retains_correct_secondary_channel(self):
        for channel, direction in ((6, "+"), (11, "-")):
            with self.subTest(channel=channel):
                conf = self.config("2g", 40, channel)
                self.assertEqual(conf["ht_capab"],
                                 f"[SHORT-GI-20][SHORT-GI-40][HT40{direction}]")
                self.assertNotIn("vht_capab", conf)

    def test_ht20_profile_cannot_accidentally_request_ht40(self):
        conf = self.config("2g", 40, profile="g-ht20")
        self.assertEqual(conf["ht_capab"], "[SHORT-GI-20]")

    def test_5g_ht20_and_ht40_only_advertise_applicable_sgi(self):
        conf = self.config("5g", 20)
        self.assertEqual(conf["ht_capab"], "[SHORT-GI-20]")
        self.assertNotIn("vht_capab", conf)
        for channel, direction in ((44, "+"), (48, "-")):
            with self.subTest(channel=channel):
                conf = self.config("5g", 40, channel)
                self.assertEqual(conf["ht_capab"],
                                 f"[SHORT-GI-20][SHORT-GI-40][HT40{direction}]")
                self.assertNotIn("vht_capab", conf)

    def test_5g_vht80_keeps_primary_and_center_channel(self):
        for channel, direction, center in ((36, "+", 42), (40, "-", 42),
                                            (149, "+", 155), (161, "-", 155)):
            with self.subTest(channel=channel):
                conf = self.config("5g", 80, channel)
                self.assertEqual(conf["channel"], str(channel))
                self.assertEqual(conf["ht_capab"],
                                 f"[SHORT-GI-20][SHORT-GI-40][HT40{direction}]")
                self.assertEqual(conf["vht_capab"], "[SHORT-GI-80]")
                self.assertEqual(conf["vht_oper_chwidth"], "1")
                self.assertEqual(conf["vht_oper_centr_freq_seg0_idx"], str(center))
                self.assertEqual(conf["country_code"], "DE")
                self.assertEqual(conf["ieee80211h"], "1")
                self.assertNotIn("noscan", conf)

    def test_sgi_can_be_disabled_without_changing_width_or_security(self):
        for band, width in (("2g", 20), ("2g", 40), ("5g", 20), ("5g", 80)):
            with self.subTest(band=band, width=width):
                conf = self.config(band, width, sgi="0")
                self.assertNotIn("SHORT-GI", " ".join(conf.values()))
                if width != 20:
                    self.assertIn("HT40", conf["ht_capab"])
                if width == 80:
                    self.assertEqual(conf["vht_oper_chwidth"], "1")
                self.assertEqual(conf["rsn_pairwise"], "CCMP")

    def test_legacy_profiles_do_not_advertise_ht_or_sgi(self):
        for profile in ("b-legacy", "g-legacy", "g-legacy-wmm", "g-long"):
            with self.subTest(profile=profile):
                conf = self.config("2g", 20, profile=profile)
                self.assertNotIn("ieee80211n", conf)
                self.assertNotIn("ht_capab", conf)
                self.assertNotIn("vht_capab", conf)

    def test_existing_configs_inherit_sgi_but_explicit_zero_is_preserved(self):
        source = "\n".join(definition(name) for name in
                            ("set_defaults", "load_config", "get_value"))
        source += "\nRUNTIME_FILE=/nonexistent-mw5-test-file\n"
        out = shell(source, 'uci_get() { :; }; get_value radio2g.short_gi; '
                    'get_value radio5g.short_gi')
        self.assertEqual(out.splitlines(), ["1", "1"])
        out = shell(source, 'uci_get() { [ "$1" != radio2g.short_gi ] || echo 0; }; '
                    'get_value radio2g.short_gi; get_value radio5g.short_gi')
        self.assertEqual(out.splitlines(), ["0", "1"])

    def test_saved_sgi_settings_survive_runtime_and_uci_serialization(self):
        source = "\n".join(definition(name) for name in
                            ("set_defaults", "sanitize_bool", "sq", "write_runtime",
                             "write_config_file", "save_settings"))
        with tempfile.TemporaryDirectory() as tmp:
            configdir = Path(tmp) / "config"
            runtime = Path(tmp) / "runtime"
            source = source.replace("/etc/config", str(configdir))
            shell(source, f"VERSION=test; RUNTIME_FILE={shlex.quote(str(runtime))}; "
                  'load_config() { set_defaults; }; '
                  'save_settings --2g-short-gi 0 --5g-short-gi 1')
            config = (configdir / "mw5wifi").read_text()
            self.assertIn("option short_gi '0'", config.split("config radio 'radio5g'")[0])
            self.assertIn("option short_gi '1'", config.split("config radio 'radio5g'")[1])
            restored = shell("", f". {shlex.quote(str(runtime))}; "
                             'echo "$R2_SHORT_GI $R5_SHORT_GI"')
            self.assertEqual(restored.strip(), "0 1")

    def test_diagnostic_ap_uses_same_capability_writer(self):
        source = "\n".join(definition(name) for name in
                            ("sanitize_bool", "write_ht_capabilities", "at_write_test_hostapd"))
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "test.conf"
            shell(source, f"ALLTEST_DIR={shlex.quote(tmp)}; ALLTEST_2G_SSID=test2; "
                  'ALLTEST_5G_SSID=test5; ALLTEST_KEY=test-password; REGDOMAIN=DE; '
                  f"at_write_test_hostapd 5g wlan0 {shlex.quote(str(path))} 80")
            self.assertIn("vht_capab=[SHORT-GI-80]", path.read_text())


class WifiDiagnosticsTests(unittest.TestCase):
    def test_legacy_proc_values_distinguish_missing_from_real_zero(self):
        source = shell_function(BASE / "usr/sbin/mw5-netdiag", "wlan_proc_value")
        with tempfile.TemporaryDirectory() as tmp:
            source = source.replace("/proc/", tmp + "/")
            self.assertEqual(shell(source, "wlan_proc_value wlan1 mw5_netif_rx_pkt").strip(), "NA")
            stats = Path(tmp) / "wlan1/stats"
            stats.parent.mkdir()
            for data, expected in (("mw5_netif_rx_pkt: 0\n", "0"),
                                   ("mw5_netif_rx_pkt: 123\n", "123"),
                                   ("different_counter: 3\n", "NA")):
                stats.write_text(data)
                self.assertEqual(shell(source, "wlan_proc_value wlan1 mw5_netif_rx_pkt").strip(), expected)

    def test_monitor_summary_does_not_turn_unavailable_counters_into_zero(self):
        source = shell_function(BASE / "usr/sbin/mw5-netdiag", "monitor_summary")
        source += '\nsection() { :; }\n'
        with tempfile.TemporaryDirectory() as tmp:
            monitor = Path(tmp) / "monitor.tsv"
            monitor.write_text("epoch sp sd ss nrx erx etx w0 w1\n"
                               "1 200001 0 3 10 20 30 NA 5\n"
                               "2 4 0 0 2 1 1 0 6\n")
            out = shell(source, f"monitor_summary {shlex.quote(str(monitor))}")
        self.assertIn("peak_wlan0_netif_rx_per_s=NA", out)
        self.assertIn("sum_wlan0_netif_rx=NA", out)
        self.assertIn("sum_wlan1_netif_rx=11", out)
        self.assertIn("cannot exclude WLAN as a source", out)

    def test_snapshot_reads_only_known_counters_and_marks_missing_data(self):
        source = shell_function(BASE / "usr/sbin/mw5-netdiag", "wlan_rx_snapshot")
        with tempfile.TemporaryDirectory() as tmp:
            radio = Path(tmp) / "radio-soc"
            radio.mkdir()
            for name, value in (("radio_index", "1"), ("gro_packets", "100"),
                                ("gro_merged", "60"), ("registers", "UNSAFE-REGISTER-DUMP")):
                (radio / name).write_text(value + "\n")
            source = source.replace("/sys/kernel/debug/rtl8192cd", tmp)
            out = shell(source, "wlan_rx_snapshot")
            self.assertIn("radio_index=1", out)
            self.assertIn("gro_packets=100", out)
            self.assertIn("gro_merged=60", out)
            self.assertIn("backlog_packets=NA", out)
            self.assertNotIn("UNSAFE-REGISTER-DUMP", out)

    def test_absent_debugfs_does_not_claim_zero_gro_activity(self):
        source = shell_function(BASE / "usr/sbin/mw5-netdiag", "wlan_rx_snapshot")
        with tempfile.TemporaryDirectory() as tmp:
            source = source.replace("/sys/kernel/debug/rtl8192cd", tmp)
            out = shell(source, "wlan_rx_snapshot")
        self.assertIn("GRO activity cannot be measured", out)


if __name__ == "__main__":
    unittest.main()
