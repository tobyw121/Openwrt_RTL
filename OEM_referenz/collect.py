#!/usr/bin/env python3
"""Copy selected original GPL text files without executing vendor tools."""
import argparse,hashlib,json,shutil
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--google',type=Path,required=True);p.add_argument('--tplink',type=Path,required=True);a=p.parse_args()
out=Path(__file__).resolve().parent
rules={
'google_ax1500':(a.google,[
 'LICENSE','COPYING','openwrt/21.02/LICENSE','openwrt/21.02/COPYING','openwrt/21.02/LICENSES/*',
 'openwrt/21.02/target/linux/realtek/files-4.4/arch/mips/bsp/*.[ch]',
 'openwrt/21.02/target/linux/realtek/files-4.4/arch/mips/bsp/Makefile',
 'openwrt/21.02/target/linux/realtek/files-4.4/drivers/net/rtl819x/rtl83xx_v1dot4/dal/rtl8367d/*.[ch]',
 'openwrt/21.02/target/linux/realtek/files-4.4/arch/mips/rtl8197f_config/config.linux*8832BR*',
 'openwrt/21.02/package/kernel/mac80211/patches/rtl/*.patch',
 'openwrt/21.02/package/base-files/files/sbin/wifi']),
 'tplink_ax1500v3':(a.tplink,[
 'rtl8198/linux-4.4.x/arch/mips/bsp/setup.c','rtl8198/linux-4.4.x/arch/mips/bsp/irq.c','rtl8198/linux-4.4.x/arch/mips/bsp/bspchip.h',
 *['rtl8198/linux-4.4.x/drivers/net/wireless/realtek/g6_wifi_driver/'+s for s in [
 'Makefile','platform/mips_97f.mk','platform/mips_97f/*.h','platform/platform_mips_97f_pci.c','include/rtw_nvm.h','include/osdep_service.h','include/osdep_service_linux.h','include/rtw_cfg.h',
 'os_dep/linux/pci_intf.c','os_dep/linux/pci_ops_linux.c','os_dep/linux/os_intfs.c','os_dep/linux/osdep_service.c','os_dep/linux/rtw_cfg.c',
 'platform/rtk_ap/rtw_api_flash.c','phl/phl_types.h','phl/hal_g6/hal_ld_file.c','phl/hal_g6/flash/*.[ch]',
 'core/rtw_mib.c','COPYING','LICENSE']]])}
records=[]
for label,(root,patterns) in rules.items():
 selected=set()
 for pattern in patterns:selected.update(root.glob(pattern))
 for src in sorted(selected):
  if not src.is_file() or src.is_symlink():continue
  data=src.read_bytes()
  if b'\0' in data or len(data)>2*1024*1024:continue
  rel=src.relative_to(root);dst=out/label/rel;dst.parent.mkdir(parents=True,exist_ok=True);dst.write_bytes(data);dst.chmod(0o644)
  records.append({'source':label,'original_path':rel.as_posix(),'path':dst.relative_to(out).as_posix(),'bytes':len(data),'sha256':hashlib.sha256(data).hexdigest()})
manifest={'archives':{'google_ax1500':{'sha256':'2413dbd852e6f039d402f8ffda62e20b9f7b88a38dde0e7d86a527078ed66b93','url':'https://drive.usercontent.google.com/download?id=1sqW7x8sLHnSitW9ZXxgsrVN3lprFLp45&export=download'},'tplink_ax1500v3':{'sha256':'6d133598b134b6878ce802a53b3061d9f2efb3636ad0d31ec4cb3bcbdf438f2a','url':'https://static.tp-link.com/upload/gpl-code/2026/202610/20261022/GPL_ax1500v3.tar.gz'}},'files':records}
(out/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
print(len(records),'files,',sum(x['bytes'] for x in records),'bytes')
