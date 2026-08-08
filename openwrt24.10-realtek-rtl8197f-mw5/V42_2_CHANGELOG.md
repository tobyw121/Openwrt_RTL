# RTL8197F OpenWrt 24.10 v42.2

## rtl8192cd build correction

### Hard syntax failure

- Fixes the reported `Hal88XXGen.c` cascade beginning at `case HW_VAR_REG_CR`.
- The actual cause was one surplus closing brace in `GetHwReg88XX()`, inside
  the `HW_VAR_POWERLIMITFILE_SIZE` / `TXPWR_LMT_8812F` block.
- When RTL8812F power-limit support was enabled, that brace closed the function
  before the later `case` labels, so GCC parsed them at file scope.

### Correct per-board chip matrices

The previous common package enabled external chip families together. That is
unsafe because the vendor PHYDM/HAL sources contain mutually incompatible
compile-time constants, including different RF path counts.

v42.2 builds one module variant per real board radio pair:

- MW5: integrated RTL8197F/FS + PCIe RTL8822B
- RD05: integrated RTL8197F/FH + PCIe RTL8812F
- AC23: integrated RTL8197F/FH + PCIe RTL8814B

The three packages conflict with each other and use separate build directories.
Each image profile selects only its matching package.

### Kbuild/preprocessor synchronization

- Converts all enabled OpenWrt/Kbuild WLAN symbols to matching `-D` defines.
- Common HAL source and object selection now see the same chip matrix.
- Removes the old one-off `CONFIG_WLAN_HAL_8812FE` define.

### Additional compiler corrections

- Makes the RX driver-info mask order explicit.
- Prints `dma_addr_t` with kernel `%pad` formatting.
- Raises package release to 5.

## Validation

- Existing v42 source, board, patch and private full-flash checks pass.
- The three board configurations were preprocessed independently.
- `GetHwReg88XX()` remains structurally complete for MW5, RD05 and AC23.
- The former combined-build `NUM_PATH` collision no longer occurs.
- A complete target compile still requires a prepared OpenWrt host environment;
  this container lacks GNU awk and ncurses development headers.
