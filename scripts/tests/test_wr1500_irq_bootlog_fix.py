"""Static regressions for R76-only PCIe IRQ correction and safe G6 probe."""
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
DTS = ROOT/'target/linux/realtek/dts/rtl8197f_cudy_ax1500-r76.dts'
G6 = ROOT/'package/kernel/rtl8832br-wr1500'
PATCH = G6/'patches/103-wr1500-pci-irq-preflight.patch'
DIAG = ROOT/'target/linux/realtek/base-files/usr/sbin/wr1500-wifi6-diag'

class WR1500InterruptRegression(unittest.TestCase):
    def test_pci_interrupt_routes_through_rtl8197f_ictl(self):
        s = DTS.read_text().split('&pcie0 {', 1)[1].split('};', 1)[0]
        self.assertIn('interrupt-parent = <&intc>;', s)
        self.assertIn('interrupts = <21 4>;', s)
        self.assertNotIn('interrupt-parent = <&cpuintc>', s)
        self.assertIn('0x19000000', s)

    def test_g6_kernel_module_not_autoloaded_during_crash_diagnosis(self):
        s = (G6/'Makefile').read_text()
        self.assertIn('PKG_RELEASE:=4', s)
        self.assertNotIn('AUTOLOAD:=', s)
        self.assertIn('rtk_wifi6.ko', s)

    def test_pci_preflight_patch_applies_before_driver_allocation(self):
        # Patch applies atop the existing 100-102 series; no vendor archive is
        # required to exercise the exact function entry and new preflight.
        # Exercise all old contexts from the real source diff, including
        # request_irq error reporting, rather than a stale function fixture.
        import re
        s = '\n'.join(''.join(line[1:]+'\n' for line in hunk.splitlines()
                    if line.startswith((' ', '-')) and not line.startswith('---'))
                    for hunk in re.split(r'^@@[^\n]*\n', PATCH.read_text(), flags=re.M)[1:])
        with tempfile.TemporaryDirectory() as td:
            p = Path(td)/'os_dep/linux/pci_intf.c'
            p.parent.mkdir(parents=True)
            p.write_text(s)
            r = subprocess.run(['patch', '-p1', '-i', str(PATCH)], cwd=td,
                               capture_output=True, text=True)
            self.assertEqual(r.returncode, 0, r.stdout + r.stderr)
            code = p.read_text()
            self.assertLess(code.find('PCI_INTERRUPT_PIN'), code.find('_adapter *padapter'))
            self.assertIn('return -ENXIO;', code)
            self.assertIn('!pdev->irq', code)
            self.assertIn('dev_info(&pdev->dev', code)

    def test_diagnostic_is_read_only_and_shell_valid(self):
        s = DIAG.read_text()
        self.assertIn("'cudy,ax1500-r76'", s)
        for forbidden in ('flash_erase', 'mtd write', 'gpio export', 'iw reg set', 'uci show wireless'):
            self.assertNotIn(forbidden, s)
        self.assertEqual(subprocess.run(['sh', '-n', str(DIAG)]).returncode, 0)

if __name__ == '__main__':
    unittest.main()
