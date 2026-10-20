#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Compile the actual package variants in disposable directories.

Requires a prepared Linux build tree (make modules_prepare). Native headers
test the base kernel API only; the OpenWrt package build also checks backports.
Never produces a flashable image or edits the checked-out vendor source.
"""
import argparse
import filecmp
from pathlib import Path
import re
import shutil
import subprocess
import tempfile


ROOT = Path(__file__).resolve().parents[1]
PACKAGE = ROOT / "package/kernel/rtl8192cd-rtl8197f"


def package_flags(variant):
    lines = (PACKAGE / "Makefile").read_text().replace("\\\n", " ").splitlines()
    result = []
    for name in ("RTL8197F_COMMON_VARS", "RTL8197F_VARIANT_VARS_" + variant):
        result.extend(next(line.split(":=", 1)[1].split() for line in lines
                           if line.startswith(name + ":=")))
    return result


def sync_tree(source_root, dest_root):
    dest_root.mkdir(parents=True, exist_ok=True)
    for source in source_root.rglob("*"):
        dest = dest_root / source.relative_to(source_root)
        if source.is_dir():
            dest.mkdir(exist_ok=True)
        elif not dest.is_file() or not filecmp.cmp(source, dest, shallow=False):
            shutil.copy2(source, dest)
            # The checkout may predate an object built from a different copy.
            # Kbuild must see the new contents as newer than that object.
            dest.touch()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--kernel", required=True, type=Path, help="prepared kernel build directory")
    parser.add_argument("--kernel-source", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path, help="logs and object directories outside checkout")
    parser.add_argument("--arch", default="mips")
    parser.add_argument("--cross-compile", default="mips-linux-gnu-")
    parser.add_argument("--cc", default="mips-linux-gnu-gcc")
    parser.add_argument("--jobs", type=int, default=4)
    parser.add_argument("--extra-cflags", default="",
                        help="additional compiler flags for optional feature builds")
    parser.add_argument("--resume", action="store_true", help="reuse latest variant directory and rebuild changed sources")
    parser.add_argument("--backports", type=Path, help="configured OpenWrt backports source tree")
    parser.add_argument("--variants", nargs="+", choices=("mw5", "rd05", "ac23", "wrd12gn"),
                        default=["mw5", "rd05", "ac23", "wrd12gn"])
    args = parser.parse_args()
    if args.backports:
        header = args.backports / "include/net/cfg80211.h"
        if not header.is_file() or not re.search(
                r"\(\*set_monitor_channel\)\(struct wiphy \*wiphy,\s*struct net_device \*dev,",
                header.read_text()):
            parser.error("--backports needs the repository's OpenWrt build/subsys patches (including 334)")
    output = args.output.resolve()
    if output == ROOT or ROOT in output.parents:
        parser.error("--output must be outside the repository")
    output.mkdir(parents=True, exist_ok=True)
    failed = False
    for variant in args.variants:
        previous = sorted((p for p in output.glob(variant + "-*") if p.is_dir()),
                          key=lambda p: p.stat().st_mtime)
        resume = args.resume and bool(previous)
        build = previous[-1] if resume else Path(tempfile.mkdtemp(prefix=variant + "-", dir=output))
        sync_tree(PACKAGE / "src", build)
        # These generated C files are tracked; avoid vendor regeneration.
        if not resume:
            for path in build.glob("data_*.c"):
                path.touch()
        compat = build / "rtl8197f_linux_compat.h"
        flags = (f"-I{args.kernel_source.resolve()} -include {compat} -Wno-error "
                 "-Wno-implicit-function-declaration -Wno-int-conversion "
                 "-Wno-address-of-packed-member")
        if args.extra_cflags:
            flags += " " + args.extra_cflags
        command = ["make", "-C", str(args.kernel.resolve()), f"M={build}",
                   f"ARCH={args.arch}", f"CROSS_COMPILE={args.cross_compile}",
                   f"CC={args.cc}", f"-j{args.jobs}", *package_flags(variant),
                   f"KCFLAGS={flags}", "rtl8192cd.o"]
        if args.backports:
            bp = args.backports.resolve()
            if not (bp / "backport-include/backport/autoconf.h").is_file():
                parser.error("configure backports and generate backport/autoconf.h first")
            # Match mac80211 Build/InstallDev: external modules use native
            # linux/module.h, without backports' internal version macros.
            bp_headers = build / ".backport-include"
            sync_tree(bp / "backport-include", bp_headers)
            (bp_headers / "linux/module.h").unlink(missing_ok=True)
            includes = [build, bp_headers, bp_headers / "uapi",
                        bp / "include", bp / "include/uapi"]
            command.insert(-1, "NOSTDINC_FLAGS=" + " ".join("-I" + str(p) for p in includes)
                           + " -include backport/backport.h -DRTL8192CD_CFG80211_BACKPORT_612")
        log = output / (build.name + ".log")
        with log.open("w") as stream:
            result = subprocess.run(command, stdout=stream, stderr=subprocess.STDOUT)
        status = "PASS" if result.returncode == 0 else "FAIL"
        print(f"{variant}: {status} (compile/link only; log {log})", flush=True)
        failed |= result.returncode != 0
        if result.returncode == 0:
            symbols = subprocess.run([args.cross_compile + "nm", "-u", str(build / "rtl8192cd.o")],
                                     capture_output=True, text=True, check=True).stdout
            forbidden = ("wireless_send_event", "iwe_stream", "iw_handler", "wext_")
            if any(word in symbols for word in forbidden):
                print(f"{variant}: FAIL: unresolved WEXT dependency", flush=True)
                failed = True
            if "set_dbf_rotation" in symbols:
                print(f"{variant}: FAIL: unresolved disabled debug-ioctl helper", flush=True)
                failed = True
    return int(failed)


if __name__ == "__main__":
    raise SystemExit(main())
