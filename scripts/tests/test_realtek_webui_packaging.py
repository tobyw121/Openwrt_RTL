"""Exercise the real base-files install hook without an OpenWrt toolchain."""
from pathlib import Path
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[2]
PLATFORM = ROOT / "target/linux/realtek"


class WebUiPackagingTests(unittest.TestCase):
    def test_landing_page_ownership(self):
        # Globally copied overlays must not claim the file owned by luci-base.
        self.assertFalse((PLATFORM / "base-files/www/index.html").exists())
        with tempfile.TemporaryDirectory() as directory:
            work = Path(directory)
            stage = work / "root"
            (stage / "www").mkdir(parents=True)
            makefile = work / "Makefile"
            makefile.write_text(
                f"PLATFORM_DIR := {PLATFORM}\n"
                "INSTALL_DIR := install -d -m0755\n"
                "INSTALL_DATA := install -m0644\n"
                f"include {PLATFORM}/base-files.mk\n"
                "all:\n"
                f"\t$(call Package/base-files/install-target,{stage})\n"
            )
            for board in ("mw5", "rd05", "ac23"):
                for luci in ("", "y", "m"):
                    with self.subTest(board=board, luci=luci):
                        index = stage / "www/index.html"
                        # Simulate a previous build with a different selection.
                        index.write_text("stale landing page")
                        subprocess.run(
                            ["make", "--no-print-directory", "-f", str(makefile),
                             "CONFIG_TARGET_realtek_rtl8197f_DEVICE_tenda_nova_mw5="
                             + ("y" if board == "mw5" else ""),
                             "CONFIG_PACKAGE_luci-base=" + luci],
                            check=True, capture_output=True, text=True,
                        )
                        if board == "mw5" and not luci:
                            self.assertEqual(index.read_bytes(),
                                             (PLATFORM / "rtl8197f/mw5-index.html").read_bytes())
                        else:
                            self.assertFalse(index.exists())
                        # Simulate luci-base installation without any overwrite.
                        if luci:
                            with index.open("x") as output:
                                output.write("LuCI")


if __name__ == "__main__":
    unittest.main()
