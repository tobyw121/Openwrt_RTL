#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Enable R76 base packages and both image types in an existing .config."""
import argparse
from pathlib import Path

REQUIRED = ('CONFIG_PACKAGE_rtl8197f-cudy-r76-base',
            'CONFIG_PACKAGE_rtl8197f-cudy-ax1500-base',
            'CONFIG_PACKAGE_kmod-rtl8832br-wr1500',
            'CONFIG_TARGET_ROOTFS_SQUASHFS', 'CONFIG_TARGET_ROOTFS_INITRAMFS')


def update(text):
    selected = ('CONFIG_TARGET_realtek_rtl8197f_DEVICE_cudy_ax1500_r76=y',
                'CONFIG_TARGET_DEVICE_realtek_rtl8197f_DEVICE_cudy_ax1500_r76=y')
    if not any(line in text.splitlines() for line in selected):
        raise ValueError('select the cudy_ax1500_r76 device profile first')
    lines = [line for line in text.splitlines()
             if not any(line.startswith(name+'=') or line == '# '+name+' is not set'
                        for name in REQUIRED)]
    return '\n'.join(lines + [name+'=y' for name in REQUIRED]) + '\n'


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--config', type=Path, default=Path('.config'))
    args = p.parse_args()
    try:
        original = args.config.read_text()
        result = update(original)
        if result != original:
            backup = args.config.with_name(args.config.name+'.before-cudy-defaults')
            if not backup.exists():
                backup.write_text(original)
            args.config.write_text(result)
    except (OSError, ValueError) as exc:
        p.error(str(exc))
    print('R76 base packages, SquashFS and initramfs selected; run make defconfig next.')


if __name__ == '__main__':
    main()
