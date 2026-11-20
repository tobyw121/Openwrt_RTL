"""Preprocess the production GPL platform header after applying package patches."""
import os,subprocess,unittest
from pathlib import Path
class RFRoot(unittest.TestCase):
 def test_compiled_platform_rf_root(self):
  root=os.environ.get('WR1500_G6_SOURCE')
  if not root:self.skipTest('prepared GPL source required')
  header=Path(root)/'platform/mips_97f/rtw_pltfm_preconf.h'
  result=subprocess.run(['cpp','-dM','-DCONFIG_LOAD_PHY_PARA_FROM_FILE','-include',str(header),'-'],input='',text=True,capture_output=True,check=True)
  self.assertIn('#define REALTEK_CONFIG_PATH "/etc/conf/"',result.stdout)
  self.assertIn('#define RTW_CUSTOM_PARA_DIR',result.stdout)
  # These are the installed files selected by ic/RFE/subtype assembly.
  pkg=Path(__file__).resolve().parents[2]/'package/kernel/rtl8832br-wr1500/files/RFE50'
  for rel in ['RadioA.txt','AP_5G/RadioA.txt','TXPWR_LMT.txt','AP_5G/TXPWR_LMT.txt']:
   self.assertTrue((pkg/rel).is_file())
