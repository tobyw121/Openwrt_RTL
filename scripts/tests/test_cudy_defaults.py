import importlib.util
from pathlib import Path
import subprocess
import tempfile
import unittest
from test_mw5_stability import ROOT

spec = importlib.util.spec_from_file_location('cudy_defaults', ROOT/'scripts/rtl8197f-cudy-defaults.py')
config = importlib.util.module_from_spec(spec)
spec.loader.exec_module(config)


class DefaultsTests(unittest.TestCase):
    def test_existing_config_without_menu_and_idempotence(self):
        original = ('CONFIG_TARGET_realtek_rtl8197f_DEVICE_cudy_ax1500_r76=y\n'
                    '# CONFIG_PACKAGE_rtl8197f-cudy-r76-base is not set\n'
                    'CONFIG_PACKAGE_rtl8197f-cudy-ax1500-base=m\n'
                    'CONFIG_SOMETHING_ELSE=y\n')
        new = config.update(original)
        self.assertIn('CONFIG_SOMETHING_ELSE=y\n', new)
        for name in config.REQUIRED:
            self.assertIn(name+'=y\n', new)
        self.assertEqual(new, config.update(new))
        with self.assertRaises(ValueError):
            config.update('CONFIG_TARGET_realtek_rtl8197f_DEVICE_xiaomi_r4_rd05=y\n')

    def test_backup_and_actual_package_kconfig_dependencies(self):
        package = ROOT/'package/network/config/rtl8197f-cudy-ax1500-base'
        source = '\n'.join(x for x in (package/'Makefile').read_text().splitlines()
                           if not x.startswith('include '))
        with tempfile.TemporaryDirectory() as tmp:
            p = Path(tmp)
            cfg = p/'config'
            original = 'CONFIG_TARGET_realtek_rtl8197f_DEVICE_cudy_ax1500_r76=y\n'
            cfg.write_text(original)
            subprocess.run(['python3', str(ROOT/'scripts/rtl8197f-cudy-defaults.py'),
                            '--config', str(cfg)], check=True, capture_output=True)
            self.assertEqual((p/'config.before-cudy-defaults').read_text(), original)
            mk = p/'Makefile'
            mk.write_text(source + '''
$(eval $(Package/rtl8197f-cudy-r76-base))
$(info DEPS=$(DEPENDS))
$(info DEFAULT=$(DEFAULT))
all: ; @:
''')
            output = subprocess.check_output(['make', '-sf', str(mk)], text=True)
            deps = output.split('DEPS=', 1)[1].splitlines()[0]
            names = [x[1:] for x in deps.split() if x.startswith('+')]
            required = {'rtl8197f-cudy-ax1500-base', 'ip-full', 'ip-bridge', 'ethtool',
                        'dnsmasq', 'odhcpd-ipv6only', 'firewall4', 'dropbear', 'luci',
                        'uhttpd', 'uhttpd-mod-ubus', 'rpcd', 'kmod-cfg80211', 'wifi-scripts', 'iw',
                        'iwinfo', 'rpcd-mod-iwinfo', 'wireless-regdb', 'hostapd-mini',
                        'tcpdump-mini', 'pciutils', 'kmod-rtl8832br-wr1500'}
            self.assertEqual(set(names), required)
            self.assertFalse(any('rtl8192cd' in x for x in names))
            # Exercise the real OpenWrt metadata generator, including + selects.
            meta = p/'packageinfo'
            default = output.split('DEFAULT=', 1)[1].splitlines()[0]
            text = 'Source-Makefile: package/cudy/Makefile\n'
            text += ('Package: rtl8197f-cudy-r76-base\nVersion: 1\nSection: utils\nCategory: Utilities\n'
                     'Title: Cudy base\nDepends: '+deps+'\nDefault: '+default+'\n\n')
            for name in names:
                text += f'Package: {name}\nVersion: 1\nSection: utils\nCategory: Utilities\nTitle: {name}\n\n'
            meta.write_text(text)
            generated = subprocess.check_output(['perl', str(ROOT/'scripts/package-metadata.pl'),
                                                 'config', str(meta)], text=True, stderr=subprocess.PIPE)
            for name in names:
                self.assertIn('select PACKAGE_'+name, generated)
            self.assertIn('default y if TARGET_realtek_rtl8197f_DEVICE_cudy_ax1500_r76', generated)


if __name__ == '__main__':
    unittest.main()
