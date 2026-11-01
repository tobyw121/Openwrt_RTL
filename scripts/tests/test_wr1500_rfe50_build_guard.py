"""WR1500 WLAN package builds must NOT depend on research OEM manifests."""
from pathlib import Path
import importlib.util
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
PKG = ROOT / 'package/kernel/rtl8832br-wr1500'
VERIFY = ROOT / 'scripts/verify-wr1500-rfe50.py'


def build_fixture(root):
    """Exercise the real Build/Prepare first recipe independently of vendor GPL tar."""
    makefile = (PKG/'Makefile').read_text()
    key = 'WR1500_RFE50_SHA256_FILES :='
    start = makefile.index(key)
    end = makefile.index('\n\n', start)
    values = makefile[start:end]
    beginning = makefile.index('define Build/Prepare\n')
    check = makefile.index('\t$(RM) -r $(PKG_BUILD_DIR)', beginning)
    guard = makefile[beginning:check] + 'endef\n'
    (root/'Makefile').write_text(values + '\n' + guard + '\ncheck:\n\t$(Build/Prepare)\n')
    rf = root/'files/RFE50'
    shutil.copytree(PKG/'files/RFE50', rf)
    return rf


class NoExternalManifestRegression(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(prefix='wr1500-no-manifest-')
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        self.rf = build_fixture(self.root)

    def run_preflight(self):
        return subprocess.run(['make', '-s', '-C', str(self.root), 'check'],
                              text=True, capture_output=True)

    def test_no_research_manifest_or_reference_files_required(self):
        self.assertFalse((self.root/'reverse-engineering').exists())
        ret = self.run_preflight()
        self.assertEqual(ret.returncode, 0, ret.stdout + ret.stderr)
        self.assertIn('16 active OEM calibration files verified', ret.stdout)

    def test_missing_active_file_fails_readably(self):
        (self.rf/'AP_5G/RadioB.txt').unlink()
        ret = self.run_preflight()
        self.assertNotEqual(ret.returncode, 0)
        self.assertIn('missing/empty active calibration: AP_5G/RadioB.txt', ret.stderr)
        self.assertNotIn('FileNotFoundError', ret.stderr)

    def test_corrupt_active_file_fails_readably(self):
        (self.rf/'RadioA.txt').write_bytes(b'corrupt data')
        ret = self.run_preflight()
        self.assertNotEqual(ret.returncode, 0)
        self.assertIn('wrong SHA256 for active calibration: RadioA.txt', ret.stderr)

    def test_package_prepare_has_no_external_manifest_dependency(self):
        source = (PKG/'Makefile').read_text()
        prepare = source.split('define Build/Prepare', 1)[1].split('endef', 1)[0]
        self.assertNotIn('reverse-engineering/', prepare)
        self.assertNotIn('verify-wr1500-rfe50.py', prepare)
        self.assertNotIn('oem-rfe50-manifest.json', prepare)


if __name__ == '__main__':
    unittest.main()
