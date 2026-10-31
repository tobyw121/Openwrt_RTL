#!/usr/bin/env python3
"""Offline regression tests for the RD05 audit fixes (not a kernel/HW build).

Uses the real edited source fragments with mocked kernel APIs and board paths.
Run from any directory: PYTHONPATH=scripts/tests python3 -m unittest test_rd05_dsa_auditfix
Requires Python 3.9+, cc, git, patch and sh. BusyBox ash is checked if present.
"""
from __future__ import annotations

import hashlib
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
PATCHDIR = ROOT / "target/linux/realtek/patches-6.6"
DRIVER = ROOT / "target/linux/realtek/files-6.6/drivers/net/ethernet/rtl8197f_rtknet.c"
BASE = ROOT / "package/network/config/rtl8197f-rd05-base"
COMPAT = ROOT / "package/network/config/rtl8367d-compat"
PATCH340 = PATCHDIR / "340-net-dsa-realtek-rd05-verify-cpu-isolation.patch"


def run(args: list[str], *, text: str | None = None, cwd: Path | None = None,
        env: dict[str, str] | None = None) -> subprocess.CompletedProcess:
    return subprocess.run(args, input=text, text=True, stdout=subprocess.PIPE,
                          stderr=subprocess.PIPE, cwd=cwd, env=env, timeout=45)


def compile_run(code: str) -> subprocess.CompletedProcess:
    cc = shutil.which("cc")
    if not cc:
        raise unittest.SkipTest("cc not available: isolated C tests not run")
    with tempfile.TemporaryDirectory(prefix="rd05-c-") as tmp:
        path = Path(tmp)
        (path / "test.c").write_text(code)
        build = run([cc, "-std=gnu11", "-Wall", "-Wextra", "-Werror", "-O2",
                     str(path / "test.c"), "-o", str(path / "test")])
        if build.returncode:
            raise AssertionError(build.stdout + build.stderr)
        return run([str(path / "test")])


def tx_fragment(source: str) -> str:
    start = source.index("\tcoherent_tx = priv->tx_coherent_enabled &&", source.index("static netdev_tx_t rtl8197f_rtk_start_xmit("))
    end = source.index("\tneeded = nr_frags + 1;", start)
    return source[start:end]


def tx_harness(fragment: str) -> str:
    return r'''
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#define NETIF_F_SG 1u
#define NETDEV_TX_OK 0
#define max_t(type, a, b) ((type)(a) > (type)(b) ? (type)(a) : (type)(b))
struct sk_buff { bool nonlinear; unsigned headlen, nr_frags, freed, calls; int linearize_ret; };
struct net_device { unsigned features; struct { unsigned tx_dropped; } stats; };
struct rtl8197f_rtknet_priv { bool is_mw5, tx_coherent_enabled; unsigned tx_sg_guard_linearize; };
static unsigned mw5_sg_max_frags = 4;
static bool rd05;
static bool of_machine_is_compatible(const char *name) { return rd05 && !strcmp(name, "xiaomi,r4-rd05"); }
static bool skb_is_nonlinear(struct sk_buff *s) { return s->nonlinear; }
static unsigned skb_headlen(struct sk_buff *s) { return s->headlen; }
static struct sk_buff *skb_shinfo(struct sk_buff *s) { return s; }
static int skb_linearize(struct sk_buff *s) {
 s->calls++; if (s->linearize_ret) return s->linearize_ret;
 s->nonlinear = false; s->nr_frags = 0; return 0;
}
static void dev_kfree_skb_any(struct sk_buff *s) { s->freed++; }
static int subject(struct rtl8197f_rtknet_priv *priv, struct net_device *ndev, struct sk_buff *skb) {
 bool coherent_tx, hw_sg; unsigned nr_frags;
''' + fragment + r'''
 return 10 + (int)nr_frags;
}
static int failures;
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #x); failures++; } } while (0)
int main(void) {
 struct rtl8197f_rtknet_priv p = {0};
 struct net_device n = {0};
 struct sk_buff s = {.nonlinear=true,.headlen=20,.nr_frags=2,.linearize_ret=-12};
 rd05=true; CHECK(subject(&p,&n,&s)==NETDEV_TX_OK); CHECK(s.freed==1); CHECK(n.stats.tx_dropped==1);
 /* MW5 guarded SG failure, which also leaked in the old source. */
 p.is_mw5=true; rd05=false; n.features=NETIF_F_SG; n.stats.tx_dropped=0;
 s=(struct sk_buff){.nonlinear=true,.headlen=20,.nr_frags=2,.linearize_ret=-12};
 CHECK(subject(&p,&n,&s)==0); CHECK(s.freed==1); CHECK(n.stats.tx_dropped==1);
 /* Excess fragments select the same guarded error path. */
 s=(struct sk_buff){.nonlinear=true,.headlen=128,.nr_frags=5,.linearize_ret=-12};
 CHECK(subject(&p,&n,&s)==0); CHECK(s.freed==1);
 /* Successful fallback must not free the caller's packet here. */
 s=(struct sk_buff){.nonlinear=true,.headlen=20,.nr_frags=2};
 CHECK(subject(&p,&n,&s)==10); CHECK(s.freed==0); CHECK(s.calls==1); CHECK(p.tx_sg_guard_linearize==1);
 /* Valid SG layout remains untouched. */
 s=(struct sk_buff){.nonlinear=true,.headlen=128,.nr_frags=2};
 CHECK(subject(&p,&n,&s)==12); CHECK(s.freed==0); CHECK(s.calls==0);
 /* Coherent RD05 path also requires linearization, regardless of feature bit. */
 p.is_mw5=false; p.tx_coherent_enabled=true; rd05=true;
 s=(struct sk_buff){.nonlinear=true,.headlen=128,.nr_frags=2,.linearize_ret=-12};
 CHECK(subject(&p,&n,&s)==0); CHECK(s.freed==1);
 /* Linear packet cannot enter either allocation-failure branch. */
 s=(struct sk_buff){.headlen=64}; CHECK(subject(&p,&n,&s)==10); CHECK(s.freed==0); CHECK(s.calls==0);
 return failures ? 1 : 0;
}
'''


def old_force_function() -> str:
    text = (PATCHDIR / "325-net-dsa-realtek-rtl8365mb-add-native-rtl8367d.patch").read_text()
    start = text.index("+static int rtl8367d_rd05_force_dsa_cpu_path(")
    end = text.index("+static int rtl8367d_rd05_reapply_runtime(", start)
    return "\n".join(line[1:] for line in text[start:end].splitlines() if line.startswith("+")) + "\n"


def apply_patch_to_fragment() -> str:
    """Check new-patch context against the existing added function, not all Linux."""
    with tempfile.TemporaryDirectory(prefix="rd05-patch-") as tmp:
        root = Path(tmp)
        path = root / "drivers/net/dsa/realtek/rtl8365mb.c"
        path.parent.mkdir(parents=True)
        path.write_text(old_force_function())
        result = run(["patch", "--batch", "--fuzz=0", "-p1", "-i", str(PATCH340)], cwd=root)
        if result.returncode:
            raise AssertionError(result.stdout + result.stderr)
        return path.read_text()


class SourceTests(unittest.TestCase):
    def test_driver_version_and_real_ownership_paths(self):
        text = DRIVER.read_text()
        self.assertIn('"1.8.47-rd05-dsa-auditfix"', text)
        result = compile_run(tx_harness(tx_fragment(text)))
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_tx_test_detects_the_original_leaks(self):
        text = tx_fragment(DRIVER.read_text())
        # Remove exactly the two newly added frees. Regression must then fail.
        self.assertEqual(text.count("dev_kfree_skb_any(skb);"), 2)
        old = text.replace("dev_kfree_skb_any(skb);", "/* old leaking path */")
        # Keep the mock free referenced to avoid an unused-function warning.
        code = tx_harness(old).replace("int main(void) {", "int main(void) { (void)dev_kfree_skb_any;")
        result = compile_run(code)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("s.freed==1", result.stderr)

    def test_padto_failure_is_not_double_freed(self):
        text = DRIVER.read_text()
        start = text.index("if (skb_put_padto(skb, ETH_ZLEN))", text.index("static netdev_tx_t rtl8197f_rtk_start_xmit("))
        block = text[start:text.index("\n\t}", start)]
        self.assertNotIn("dev_kfree_skb", block)

    def test_patch_syntax(self):
        selected = [p for p in sorted(PATCHDIR.glob("*.patch")) if 323 <= int(p.name[:3]) <= 340]
        self.assertGreaterEqual(len(selected), 19)
        for patch in selected:
            result = run(["git", "apply", "--numstat", str(patch)])
            self.assertEqual(result.returncode, 0, patch.name + ": " + result.stderr)

    def test_new_patch_context_and_readback_faults(self):
        text = apply_patch_to_fragment()
        checker = text[:text.index("static int rtl8367d_rd05_force_dsa_cpu_path(")]
        code = r'''
#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
typedef unsigned int u32;
/* Mock register space: IDs below model names, not a hardware SDK. */
#define RTL8365MB_CPU_PORT_MASK_REG 1
#define RTL8365MB_RTL8367D_PHY_PORTS_MASK 0xffu
#define RTL8365MB_PORT_ISOLATION_REG(p) (16+(p))
#define RTL8367D_VLAN_PVID_CTRL_REG(p) (32+(p))
#define RTL8367D_VLAN_PVID_CTRL_MASK 0xfffu
#define RTL8367D_RD05_OEM_VID 9
#define dev_err(...) do {} while(0)
struct mock_map { u32 regs[64]; int error_reg, reads; };
struct mock_ds { unsigned active, cpu; };
struct realtek_priv { struct mock_map *map; struct mock_ds *ds; int num_ports; void *dev; };
static bool dsa_is_unused_port(struct mock_ds *ds, int p) { return !(ds->active & (1u << p)); }
static bool dsa_is_cpu_port(struct mock_ds *ds, int p) { return !!(ds->cpu & (1u << p)); }
static int regmap_read(struct mock_map *m, u32 reg, u32 *value) {
 m->reads++; if ((int)reg == m->error_reg) return -EREMOTEIO;
 *value=m->regs[reg]; return 0;
}
''' + checker + r'''
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #x); return 1; } } while(0)
int main(void) {
 struct mock_map map = {.error_reg=-1};
 struct mock_ds ds = {.active=0x9f,.cpu=0x80};
 struct realtek_priv priv = {.map=&map,.ds=&ds,.num_ports=8};
 map.regs[1]=0x80;
 for (int i=0; i<8; i++) { map.regs[16+i]=i==7 ? 0x1f : 0x80; map.regs[32+i]=9; }
 CHECK(rtl8367d_rd05_verify_isolation(&priv,0x80,0x1f)==0);
 CHECK(map.reads==13);
 map.regs[16]=0x82; CHECK(rtl8367d_rd05_verify_isolation(&priv,0x80,0x1f)==-EIO); map.regs[16]=0x80;
 map.regs[1]=0x40; CHECK(rtl8367d_rd05_verify_isolation(&priv,0x80,0x1f)==-EIO); map.regs[1]=0x80;
 map.regs[23]=0x1d; CHECK(rtl8367d_rd05_verify_isolation(&priv,0x80,0x1f)==-EIO); map.regs[23]=0x1f;
 map.regs[34]=8; CHECK(rtl8367d_rd05_verify_isolation(&priv,0x80,0x1f)==-EIO); map.regs[34]=0xf009;
 CHECK(rtl8367d_rd05_verify_isolation(&priv,0x80,0x1f)==0); /* unrelated high bits ignored */
 map.error_reg=18; CHECK(rtl8367d_rd05_verify_isolation(&priv,0x80,0x1f)==-EREMOTEIO);
 map.error_reg=21; CHECK(rtl8367d_rd05_verify_isolation(&priv,0x80,0x1f)==0); /* unused port 5 */
 return 0;
}
'''
        result = compile_run(code)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("if (mode <= 1 && !ingress_mode && !extcpu_mode)", text)
        self.assertIn("rtl8367d_ext_config_forcemode(priv, 7, false,", text)
        # Existing exact-board/chip gate must still be present in the patch series.
        scope = (PATCHDIR / "337-net-dsa-realtek-scope-rd05-cpu-policy.patch").read_text()
        self.assertIn('of_machine_is_compatible("xiaomi,r4-rd05")', scope)
        self.assertIn("mb->chip_info->rtl8367d", scope)

    def test_dts_hardware_properties_unchanged(self):
        text = (ROOT / "target/linux/realtek/dts/rtl8197f_xiaomi_r4-rd05.dts").read_text()
        text = re.sub(r"/\*.*?\*/|//[^\n]*", "", text, flags=re.S)
        digest = hashlib.sha256(" ".join(text.split()).encode()).hexdigest()
        self.assertEqual(digest, "284edc9e322d8099c08c1c89495a1a36d80a349964d07fb99db2a577609f755a")

    def test_new_patch_matches_later_rd05_prerequisite_context(self):
        def hunks(text):
            result = []
            for part in re.split(r"^@@ .*?@@[^\n]*\n", text, flags=re.M)[1:]:
                lines = []
                for line in part.splitlines(True):
                    if not line or line[0] not in " +-" or line.startswith(("--- ", "+++ ")):
                        break
                    lines.append(line)
                before = "".join(line[1:] for line in lines if line[0] in " -")
                after = "".join(line[1:] for line in lines if line[0] in " +")
                result.append((before, after))
            return result
        # This is a projection of provided source contexts, NOT a Linux tree.
        # It includes the complete RD05 functions introduced in 325, then
        # replays relevant 337/338/339 hunks to detect context regressions.
        p325 = (PATCHDIR / "325-net-dsa-realtek-rtl8365mb-add-native-rtl8367d.patch").read_text()
        projected = "\n/* context gap: upstream source not supplied */\n".join(after for _, after in hunks(p325))
        counts = []
        for name in ("337-net-dsa-realtek-scope-rd05-cpu-policy.patch",
                     "338-net-dsa-realtek-rd05-prepare-ext1-before-bootstrap.patch",
                     "339-net-dsa-realtek-rd05-quiesce-and-check-rgmii.patch"):
            n = 0
            for before, after in hunks((PATCHDIR / name).read_text()):
                if before and projected.count(before) == 1:
                    projected = projected.replace(before, after, 1); n += 1
            counts.append(n)
        self.assertGreaterEqual(counts[0], 2)
        self.assertGreaterEqual(counts[1], 3)
        self.assertGreaterEqual(counts[2], 5)
        self.assertIn("rtl8367d_rd05_prepare_link(priv, tx_delay, rx_delay)", projected)
        self.assertIn("if (!rtl8367d_rd05_board(mb))", projected)
        with tempfile.TemporaryDirectory(prefix="rd05-context-") as tmp:
            root = Path(tmp); path = root / "drivers/net/dsa/realtek/rtl8365mb.c"
            path.parent.mkdir(parents=True); path.write_text(projected)
            result = run(["patch", "--batch", "--fuzz=0", "-p1", "-i", str(PATCH340)], cwd=root)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn("rtl8367d_rd05_verify_isolation(priv, cpu_mask, user_mask)", path.read_text())

    def test_shell_syntax(self):
        files = [ROOT / "target/linux/realtek/base-files/etc/board.d/02_network",
                 BASE / "files/etc/uci-defaults/99-rd05-dsa-network",
                 BASE / "files/usr/sbin/rd05-dsa-check", COMPAT / "files/rtl8367dctl"]
        shells = [["sh"]]
        if shutil.which("busybox"):
            shells.append(["busybox", "ash"])
        for shell in shells:
            for file in files:
                result = run(shell + ["-n", str(file)])
                self.assertEqual(result.returncode, 0, result.stderr)

    def test_board_defaults_do_not_wait_for_dsa_or_use_conduit(self):
        source = (ROOT / "target/linux/realtek/base-files/etc/board.d/02_network").read_text()
        source = re.sub(r"^\. /lib/.*$", "# mocked library", source, flags=re.M)
        stubs = "\n".join(f'{name}() {{ printf "{name}"; printf " <%s>" "$@"; echo; }}'
                          for name in sorted(set(re.findall(r"\bucidef_[a-z_]+", source))))
        common = r'''
board_config_update() { :; }
board_config_flush() { :; }
mtd_get_mac_ascii() { echo 02:00:00:00:00:01; }
macaddr_add() { echo "$1"; }
macaddr_setbit_la() { echo "$1"; }
ls() { printf '/sys/class/net/lan3\n/sys/class/net/lan4\n'; }
'''
        for board, expected in [("xiaomi,r4-rd05", "<lan1 lan2> <wan>"), ("tenda,nova-mw5", "<lan> <wan>")]:
            result = run(["sh"], text=stubs + common + f'\nboard_name() {{ echo "{board}"; }}\n' + source)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn("ucidef_set_interfaces_lan_wan " + expected, result.stdout)
            self.assertNotIn("ucidef_set_bridge_device", result.stdout)
            self.assertNotIn("ucidef_set_bridge_mac", result.stdout)
            self.assertNotIn("<lan1 lan2 lan3", result.stdout)
            self.assertIn("ucidef_set_network_device_mac <eth0> <02:00:00:00:00:01>", result.stdout)
            self.assertIn("ucidef_set_interface_macaddr <wan> <02:00:00:00:00:01>", result.stdout)

    def test_existing_network_is_preserved_and_empty_generated(self):
        source = (BASE / "files/etc/uci-defaults/99-rd05-dsa-network").read_text()
        source = re.sub(r"^\. /lib/.*$", "# mocked library", source, flags=re.M)
        with tempfile.TemporaryDirectory(prefix="rd05-uci-") as tmp:
            root = Path(tmp); config = root / "network"; generator = root / "generate"
            marker = root / "called"
            generator.write_text(f'#!/bin/sh\necho called > "{marker}"\necho generated > "{config}"\n')
            generator.chmod(0o755)
            text = source.replace("/etc/config/network", str(config)).replace("/bin/config_generate", str(generator))
            text = 'board_name() { echo xiaomi,r4-rd05; }\n' + text
            original = "config device 'custom'\n option name 'br-custom'\n"
            config.write_text(original)
            result = run(["sh"], text=text)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(config.read_text(), original)
            self.assertFalse(marker.exists())
            config.unlink()
            result = run(["sh"], text=text)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertTrue(marker.exists())
            self.assertEqual(config.read_text(), "generated\n")
            config.unlink(); marker.unlink(); generator.write_text("#!/bin/sh\nexit 1\n")
            result = run(["sh"], text=text)
            self.assertNotEqual(result.returncode, 0)
            # A non-RD05 must not generate or edit anything.
            result = run(["sh"], text=text.replace("echo xiaomi,r4-rd05", "echo tenda,nova-mw5"))
            self.assertEqual(result.returncode, 0)
            self.assertFalse(config.exists())

    def test_vlan_wrapper_refuses_rd05_before_commands(self):
        source = (COMPAT / "files/rtl8367dctl").read_text()
        with tempfile.TemporaryDirectory(prefix="rd05-vlan-") as tmp:
            root = Path(tmp); board = root / "board"; board.write_text("xiaomi,r4-rd05\n")
            source = source.replace("/tmp/sysinfo/board_name", str(board))
            script = root / "ctl"; script.write_text(source)
            for args in (["bridge-init"], ["vlan-add", "10", "lan1:u"], ["vlan-del", "10", "lan1"],
                         ["pvid", "lan1", "10"], ["ioctl", "set_vlan", "10", "lan1:u"],
                         ["swconfig-vlan", "10", "lan1:u"]):
                result = run(["sh", str(script)] + list(args))
                self.assertEqual(result.returncode, 95, str(args) + result.stdout + result.stderr)
                self.assertIn("No configuration changed", result.stderr)

    def test_vlan_wrapper_propagates_non_rd05_failure(self):
        source = (COMPAT / "files/rtl8367dctl").read_text()
        with tempfile.TemporaryDirectory(prefix="rd05-vlan-error-") as tmp:
            root = Path(tmp); board = root / "board"; board.write_text("other,board\n")
            source = source.replace("/tmp/sysinfo/board_name", str(board)).replace("/proc/device-tree/compatible", str(root / "absent"))
            source = source.replace("/sys/class/net/", str(root / "net") + "/")
            (root / "net/lan1").mkdir(parents=True)
            binary = root / "bridge"; binary.write_text("#!/bin/sh\necho 'simulated kernel rejection' >&2\nexit 42\n"); binary.chmod(0o755)
            script = root / "ctl"; script.write_text(source)
            env = dict(os.environ, PATH=str(root) + ":" + os.environ.get("PATH", ""))
            result = run(["sh", str(script), "vlan-add", "10", "lan1:u"], env=env)
            self.assertEqual(result.returncode, 42, result.stdout + result.stderr)
            self.assertIn("simulated kernel rejection", result.stderr)

    def test_read_only_report_and_topology_warnings(self):
        source = (BASE / "files/usr/sbin/rd05-dsa-check").read_text()
        with tempfile.TemporaryDirectory(prefix="rd05-report-") as tmp:
            root = Path(tmp)
            for path in ("tmp/sysinfo", "sys/class/net", "proc", "bin"):
                (root / path).mkdir(parents=True)
            (root / "tmp/sysinfo/board_name").write_text("xiaomi,r4-rd05\n")
            for i, dev in enumerate(("eth0", "lan1", "lan2", "wan", "lan3", "lan4")):
                path = root / "sys/class/net" / dev; path.mkdir()
                for name, value in (("carrier", "0"), ("operstate", "down"), ("phys_port_name", f"p{i}")):
                    (path / name).write_text(value + "\n")
            for cmd in ("ethtool", "ip", "bridge", "uci", "dmesg"):
                binary = root / "bin" / cmd
                binary.write_text('#!/bin/sh\nprintf "mock command"; printf " %s" "$@"; echo\n')
                binary.chmod(0o755)
            env = dict(os.environ, PATH=str(root / "bin") + ":" + os.environ.get("PATH", ""))
            text = source.replace("/tmp/sysinfo/", str(root / "tmp/sysinfo") + "/").replace("/sys/", str(root / "sys") + "/").replace("/proc/", str(root / "proc") + "/")
            script = root / "check"; script.write_text(text)
            before = {str(p): p.read_bytes() for p in root.rglob("*") if p.is_file()}
            result = run(["sh", str(script)], env=env)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn("does not validate the hardware", result.stdout)
            after = {str(p): p.read_bytes() for p in root.rglob("*") if p.is_file()}
            self.assertEqual(before, after)
            (root / "sys/class/net/wan/master").symlink_to("../../br-lan")
            result = run(["sh", str(script), "--ports"], env=env)
            self.assertEqual(result.returncode, 1)
            self.assertIn("wan is in br-lan", result.stdout)
            (root / "tmp/sysinfo/board_name").write_text("other,board\n")
            result = run(["sh", str(script)], env=env)
            self.assertEqual(result.returncode, 2)


if __name__ == "__main__":
    print("Scope: offline source regression tests; NO full kernel build or hardware validation.", flush=True)
    unittest.main(verbosity=2)
