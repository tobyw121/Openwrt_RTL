#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Validate the v42 RTL8197F board evidence database against DTS/image files."""
from __future__ import annotations
import json
import re
import sys
from pathlib import Path

TOP = Path(__file__).resolve().parents[1]
DB = TOP / "target/linux/realtek/rtl8197f/board-profiles-v42.json"
DTS = {
    "xiaomi,r4-rd05": TOP / "target/linux/realtek/dts/rtl8197f_xiaomi_r4-rd05.dts",
    "tenda,nova-mw5": TOP / "target/linux/realtek/dts/rtl8197f_tenda_nova-mw5.dts",
    "tenda,ac23": TOP / "target/linux/realtek/dts/rtl8197f_tenda_ac23.dts",
}

def require(text: str, needle: str, label: str, errors: list[str]) -> None:
    if needle not in text:
        errors.append(f"{label}: missing {needle!r}")

def main() -> int:
    db = json.loads(DB.read_text(encoding="utf-8"))
    errors: list[str] = []
    for board, path in DTS.items():
        profile = db["boards"][board]
        text = path.read_text(encoding="utf-8")
        require(text, f'compatible = "{board}"', board, errors)
        require(text, f'<0x00000000 0x{profile["ram_bytes"]:08x}>', board, errors)
        if "smi_mdc_gpio" in profile:
            require(text, f'<&gpio0 {profile["smi_mdc_gpio"]} GPIO_ACTIVE_HIGH>', board, errors)
        if "smi_mdio_gpio" in profile:
            require(text, f'<&gpio0 {profile["smi_mdio_gpio"]} GPIO_ACTIVE_HIGH>', board, errors)
        if "switch_reset_gpio" in profile:
            require(text, f'<&gpio0 {profile["switch_reset_gpio"]} GPIO_ACTIVE_LOW>', board, errors)
        if "status_led_gpio" in profile:
            require(text, f'<&gpio0 {profile["status_led_gpio"]} GPIO_ACTIVE_LOW>', board, errors)
        if "reset_button_gpio" in profile:
            require(text, f'<&gpio0 {profile["reset_button_gpio"]} GPIO_ACTIVE_LOW>', board, errors)
        if "firmware_size" in profile:
            want = f'<0x{profile["firmware_offset"]:06x} 0x{profile["firmware_size"]:06x}>'
            require(text, want, board, errors)
    image = (TOP / "target/linux/realtek/image/rtl8197f.mk").read_text(encoding="utf-8")

    def device_block(name: str) -> str:
        match = re.search(
            rf"define Device/{re.escape(name)}\n(.*?)\nendef", image, re.S
        )
        if not match:
            errors.append(f"image: missing Device/{name}")
            return ""
        return match.group(1)

    ac23_image = device_block("tenda_ac23")
    mw5_image = device_block("tenda_nova_mw5")
    require(ac23_image, "RTL8197F_HEADER_SIZE := 60", "AC23 image", errors)
    require(ac23_image, "RTL8197F_LEN_INCLUDES_HEADER_PAD := 1", "AC23 image", errors)
    require(mw5_image, "RTL8197F_HEADER_SIZE := 16", "MW5 image", errors)
    if "RTL8197F_LEN_INCLUDES_HEADER_PAD" in mw5_image:
        errors.append("MW5 image: OEM payload prefix must not be counted or emitted")
    require(image, "append-rtl8197f-rootfs", "Tenda image", errors)
    if errors:
        print("RTL8197F board profile validation FAILED", file=sys.stderr)
        for error in errors:
            print(f"  - {error}", file=sys.stderr)
        return 1
    print(f"RTL8197F board profile validation OK: {len(DTS)} boards")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
