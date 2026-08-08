#!/usr/bin/env python3
"""Generate an exhaustive port-status manifest for the embedded Realtek SDK.

The legacy SDK is proprietary, Linux-2.6-era source.  It is kept byte-for-byte
as traceability material.  Functional hardware support is supplied by native
Linux 6.6/OpenWrt drivers where a verified subsystem replacement exists.
"""

from __future__ import annotations

import argparse
import csv
import hashlib
import os
from collections import Counter
from pathlib import Path
from typing import NamedTuple


class Mapping(NamedTuple):
    component: str
    status: str
    replacement: str
    build_policy: str
    notes: str


def contains_any(path: str, tokens: tuple[str, ...]) -> bool:
    lower = path.lower()
    return any(token in lower for token in tokens)


def classify(path: str) -> Mapping:
    p = path.replace(os.sep, "/")
    low = p.lower()

    # Explicitly isolate chip families that have SDK register/NIC implementations
    # but no verified native Linux 6.6 DSA + Ethernet datapath in this tree.
    if contains_any(low, ("rtl8328", "r8328", "phy_8328", "/esw/")):
        return Mapping(
            "RTL8328 switch/NIC/BSP",
            "legacy-chip-gap-retained",
            "No verified native Linux 6.6 DSA/Ethernet implementation",
            "not-built",
            "Source retained for register-level reference; do not claim operational RTL8328 datapath support.",
        )
    if contains_any(low, ("rtl8389", "r8389", "phy_8389", "/ssw/")):
        return Mapping(
            "RTL8389 switch/NIC/BSP",
            "legacy-chip-gap-retained",
            "No verified native Linux 6.6 DSA/Ethernet implementation",
            "not-built",
            "Source retained for register-level reference; RTL8389 family constants alone are not a functional driver.",
        )

    if low.startswith("system/drv/gpio/"):
        return Mapping(
            "GPIO",
            "native-integrated",
            "GPIO_REALTEK_OTTO, PINCTRL_RTL8231 and gpio-rtl8197f",
            "native-only",
            "Legacy direct-register GPIO API is replaced by gpiolib/pinctrl.",
        )
    if low.startswith("system/drv/intr/"):
        return Mapping(
            "interrupt controller",
            "native-integrated",
            "irqchip/irq-realtek-rtl plus RTL83xx platform IRQ code",
            "native-only",
            "Legacy interrupt API is replaced by irqdomain/irqchip infrastructure.",
        )
    if low.startswith("system/drv/nic/"):
        return Mapping(
            "switch CPU-port NIC",
            "native-integrated-needs-hardware-validation",
            "NET_RTL838X Ethernet DMA driver plus DSA master interface",
            "native-only",
            "SDK descriptor/ioctl NIC is not built; native NAPI/phylink/DSA path is used.",
        )
    if low.startswith("system/drv/rtl8231/"):
        return Mapping(
            "RTL8231 multifunction expander",
            "native-integrated-needs-hardware-validation",
            "MFD_RTL8231, PINCTRL_RTL8231/GPIO and LEDS_RTL8231",
            "native-only",
            "Split into Linux MFD, GPIO/pinctrl and LED class drivers.",
        )
    if low.startswith("system/drv/smi/"):
        return Mapping(
            "SMI/MDIO management bus",
            "native-integrated",
            "MDIO_REALTEK_OTTO_AUX and Linux MDIO bus APIs",
            "native-only",
            "Legacy SMI character/ioctl access is replaced by mdiobus/regmap users.",
        )
    if low.startswith("system/drv/swcore/"):
        return Mapping(
            "switch core",
            "native-integrated-needs-hardware-validation",
            "NET_DSA_RTL83XX with bridge/switchdev/FDB/VLAN/STP support",
            "native-only",
            "Legacy switch-core control path is replaced by DSA and standard netlink interfaces.",
        )
    if low.startswith("system/drv/swled/"):
        return Mapping(
            "switch LED engine",
            "native-integrated-needs-hardware-validation",
            "LEDS_RTL8231 and Linux LED class triggers",
            "native-only",
            "RTL8231 LED scan matrix is enabled for rtl838x and rtl839x.",
        )
    if low.startswith("system/drv/uart/"):
        return Mapping(
            "UART",
            "generic-kernel-replacement",
            "serial 8250/OF platform driver",
            "native-only",
            "Legacy UART abstraction is not built as a second driver.",
        )
    if low.startswith("system/drv/watchdog/"):
        return Mapping(
            "watchdog",
            "native-integrated-needs-hardware-validation",
            "REALTEK_OTTO_WDT and Linux watchdog framework",
            "native-only",
            "Legacy direct-register watchdog API is replaced by watchdog core.",
        )

    if low.startswith("src/hal/phy/") or low.startswith("include/hal/phy/"):
        if contains_any(low, ("8208", "8212")):
            return Mapping(
                "RTL8208/RTL8212 multiport PHY",
                "legacy-phy-gap-retained",
                "No exact verified Linux 6.6 implementation in this tree",
                "not-built",
                "Generic clause-22 probing may expose basic links, but SDK-specific initialization is not claimed ported.",
            )
        if contains_any(low, ("8201",)):
            return Mapping(
                "RTL8201 PHY",
                "generic-kernel-replacement-needs-hardware-validation",
                "upstream Realtek PHY/generic PHY support",
                "native-only",
                "Basic PHY support is delegated to Linux phylib; SDK-private behavior is not preserved as an ABI.",
            )
        if contains_any(low, ("8214", "8218", "8380", "8390", "identify", "probe", "common")):
            return Mapping(
                "RTL83xx switch PHY/SerDes",
                "native-integrated-needs-hardware-validation",
                "REALTEK_SOC_PHY (rtl83xx-phy.c)",
                "native-only",
                "Native phylib driver includes RTL8214/RTL8218 and RTL8380/RTL8390 SerDes paths.",
            )
        return Mapping(
            "PHY support",
            "source-retained-review-required",
            "Linux phylib / rtl83xx-phy.c",
            "not-built",
            "No exact one-to-one automated mapping established.",
        )

    if low.startswith("src/dal/") or low.startswith("include/dal/"):
        return Mapping(
            "data abstraction layer",
            "legacy-control-api-retained",
            "DSA, switchdev, bridge, tc, ethtool and devlink semantics",
            "not-built",
            "The proprietary DAL ABI is not a Linux hardware-driver ABI and is intentionally not loaded in-kernel.",
        )
    if low.startswith("src/rtk/") or low.startswith("include/rtk/"):
        return Mapping(
            "RTK management API",
            "legacy-control-api-retained",
            "iproute2/bridge/ethtool/devlink/tc/netlink interfaces",
            "not-built",
            "Functionality is mapped to standard Linux control planes; binary/API compatibility is not promised.",
        )
    if low.startswith("src/hal/") or low.startswith("include/hal/"):
        return Mapping(
            "hardware abstraction/register tables",
            "native-semantic-replacement",
            "rtl83xx DSA/Ethernet/PHY register operations and regmap/MDIO",
            "not-built",
            "Register descriptions are retained for auditing; native drivers own hardware access.",
        )

    if low.startswith("system/linux/rtcore/") or low.startswith("system/linux/rtdrv/") or low.startswith("system/linux/rtk/") or low.startswith("system/linux/rtnic/"):
        return Mapping(
            "legacy SDK kernel ABI",
            "legacy-kernel-module-replaced",
            "native DSA/Ethernet drivers and standard netlink/ioctl APIs",
            "not-built",
            "Uses removed Linux-2.6 file_operations/ioctl/task APIs; retaining it as a loadable module would duplicate and bypass native subsystems.",
        )
    if low.startswith("system/linux/linux-2.6.32.x/"):
        if contains_any(low, ("/drivers/mtd/", "flash", "spi")):
            replacement = "Linux SPI-NOR/MTD plus RTL8197F SPI-ROM automap where required"
        elif contains_any(low, ("/usb", "/drivers/usb/")):
            replacement = "generic Linux USB host/device subsystem; board-specific validation required"
        elif contains_any(low, ("/pci", "/drivers/pci/")):
            replacement = "generic Linux PCI subsystem; no unverified SDK host controller enabled"
        elif contains_any(low, ("serial", "uart")):
            replacement = "serial 8250/OF"
        elif contains_any(low, ("timer", "clock")):
            replacement = "REALTEK_OTTO_TIMER and RTL83xx clock drivers"
        elif contains_any(low, ("irq", "interrupt")):
            replacement = "irqchip/irq-realtek-rtl"
        else:
            replacement = "modern MIPS Realtek platform code and generic Linux 6.6 subsystem"
        return Mapping(
            "Linux 2.6 BSP copy",
            "legacy-bsp-replaced",
            replacement,
            "not-built",
            "The embedded 2.6.32 kernel fork is reference material, not a second kernel source tree.",
        )
    if low.startswith("system/linux/"):
        return Mapping(
            "legacy Linux glue",
            "legacy-kernel-glue-replaced",
            "OpenWrt target integration and native Linux subsystem APIs",
            "not-built",
            "Legacy SDK glue is retained only for behavior comparison.",
        )

    if low.startswith("system/ioal/") or low.startswith("include/ioal/"):
        return Mapping(
            "MMIO/I/O abstraction",
            "native-semantic-replacement",
            "ioremap, readl/writel, regmap and MDIO helpers",
            "not-built",
            "No direct userspace physical-address access is enabled.",
        )
    if low.startswith("system/osal/") or low.startswith("include/osal/"):
        return Mapping(
            "OS abstraction",
            "native-semantic-replacement",
            "native Linux locking, allocation, timing and userspace libc APIs",
            "not-built",
            "The cross-OS wrapper layer is not needed inside a Linux-only OpenWrt target.",
        )
    if low.startswith("system/common/") or low.startswith("include/common/") or low.startswith("src/common/"):
        return Mapping(
            "SDK common support",
            "legacy-support-library-retained",
            "native Linux helpers or userspace libraries as applicable",
            "not-built",
            "Retained because higher-level SDK sources reference it; not independently a hardware driver.",
        )

    if low.startswith("example/") or "/unittest/" in low or "/unit_test/" in low or low.startswith("src/app/diag") or low.startswith("include/app/diag"):
        return Mapping(
            "diagnostic/example/test",
            "reference-only",
            "OpenWrt userspace tools and standard networking diagnostics",
            "not-built",
            "Diagnostic command source is retained for register/API behavior research.",
        )
    if low.startswith("build/") or low.startswith("config/") or low.endswith("makefile") or "/makefile" in low or low.endswith(".mk"):
        return Mapping(
            "legacy build system",
            "build-support-retained",
            "OpenWrt package/target build system",
            "not-built",
            "Original build metadata is retained but is not sourced by OpenWrt.",
        )
    if low.startswith("include/"):
        return Mapping(
            "legacy SDK headers",
            "source-retained-review-required",
            "native Linux UAPI/internal headers",
            "not-built",
            "Header retained for traceability; inclusion from native drivers is not allowed by default.",
        )
    if low.startswith("src/"):
        return Mapping(
            "legacy SDK implementation",
            "source-retained-review-required",
            "native Linux/OpenWrt subsystem implementation",
            "not-built",
            "No automatic one-to-one driver mapping established.",
        )

    return Mapping(
        "reference/support material",
        "reference-only",
        "none",
        "not-built",
        "Retained verbatim as part of the user-supplied SDK inventory.",
    )


def digest(path: Path) -> tuple[str, int, str]:
    if path.is_symlink():
        target = os.readlink(path)
        return hashlib.sha256(target.encode("utf-8", errors="surrogateescape")).hexdigest(), len(target.encode()), target
    h = hashlib.sha256()
    size = 0
    with path.open("rb") as f:
        for block in iter(lambda: f.read(1024 * 1024), b""):
            h.update(block)
            size += len(block)
    return h.hexdigest(), size, ""


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("source", type=Path)
    parser.add_argument("manifest", type=Path)
    parser.add_argument("summary", type=Path)
    args = parser.parse_args()

    source = args.source.resolve()
    rows: list[dict[str, str | int]] = []
    status_counts: Counter[str] = Counter()
    component_counts: Counter[str] = Counter()

    paths = sorted(
        (p for p in source.rglob("*") if p.is_file() or p.is_symlink()),
        key=lambda p: p.relative_to(source).as_posix(),
    )
    for item in paths:
        rel = item.relative_to(source).as_posix()
        mapping = classify(rel)
        sha256, size, link_target = digest(item)
        file_type = "symlink" if item.is_symlink() else "file"
        rows.append(
            {
                "path": rel,
                "type": file_type,
                "bytes": size,
                "sha256": sha256,
                "link_target": link_target,
                "component": mapping.component,
                "port_status": mapping.status,
                "linux_6_6_replacement": mapping.replacement,
                "build_policy": mapping.build_policy,
                "notes": mapping.notes,
            }
        )
        status_counts[mapping.status] += 1
        component_counts[mapping.component] += 1

    args.manifest.parent.mkdir(parents=True, exist_ok=True)
    with args.manifest.open("w", newline="", encoding="utf-8") as f:
        writer = csv.DictWriter(f, fieldnames=list(rows[0].keys()), delimiter="\t", lineterminator="\n")
        writer.writeheader()
        writer.writerows(rows)

    with args.summary.open("w", newline="", encoding="utf-8") as f:
        writer = csv.writer(f, delimiter="\t", lineterminator="\n")
        writer.writerow(["dimension", "value", "entries"])
        writer.writerow(["inventory", "all regular files and symlinks", len(rows)])
        for key, value in sorted(status_counts.items()):
            writer.writerow(["port_status", key, value])
        for key, value in sorted(component_counts.items()):
            writer.writerow(["component", key, value])

    print(f"manifest entries: {len(rows)}")
    for status, count in sorted(status_counts.items()):
        print(f"{status}: {count}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
