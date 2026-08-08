# RTL8197F / RTL8367 family v35 validation

## Reproducible source baseline

- OpenWrt branch: 24.10 delivery tree supplied by the user.
- Kernel source: `linux-6.6.114.tar.xz`.
- Kernel archive SHA-256: `ca4175a03ce2943ae192d77ad91e37ee292f1f1bb7b2954b062b0ef7eb0cb97c`.
- Generic OpenWrt patches: 398 backport + 151 pending + 51 hack.
- Realtek target patches: 38.
- Total sequentially applied kernel patches: 638.

## Patch application

The full patch sequence was applied to a clean Linux 6.6.114 extraction in the
same order used by the target preparation. This exposed and fixed a pre-existing
v34 problem in patch 325: seven hunks had parsed correctly but no longer matched
after the OpenWrt generic patch stack. Patches 325 and 326 plus the new family patch 327
were regenerated against the real prepared kernel state.

Result:

- all 638 patches applied;
- no `.rej` or `.orig` files;
- final prepared `drivers/net/dsa/realtek/rtl8365mb.c` exactly matches the
  expected v35 source;
- `MANUAL_KERNEL_PREPARE_PASS`;
- `RTL8367_FINAL_SOURCE_MATCH_PASS`.

See `validation/linux-6.6.114-patch-prepare.log`.

## Static project validator

Result: **186 PASS, 7 WARN, 0 FAIL**.

The warnings are intentional limits, not hidden test failures:

1. RTL8328 native Linux 6.6 datapath not verified.
2. RTL8389 native Linux 6.6 datapath not verified.
3. RTL8208/RTL8212 SDK-specific PHY initialization not verified.
4. Old B-map RTL8367 silicon still needs exact legacy extif/init selection.
5. Non-exact C/D family fallbacks need physical-board validation.
6. Standard DSA bridge/VLAN/FDB/MDB/LAG/TC hardware offloads are incomplete.
7. A full OpenWrt cross-build and physical boot/traffic tests remain required.

See `validation/project-validator.log`.

## Compile smoke tests

The exact final prepared switch source and exact delivered RTL8197F NIC source
were compiled as external objects against installed Linux headers with `W=1`
and warnings promoted to errors:

- `rtl8365mb.o`: PASS;
- `rtl8197f_rtknet.o`: PASS.

These are API/compile tests, not MIPS runtime tests. See the two host-compile
logs under `validation/`.

## Binding and script checks

- RTL8197F NIC YAML parses with PyYAML.
- New shell scripts pass syntax checking.
- Register-map generator passes Python byte-code compilation.
- Patches 325 and 326: checkpatch 0 errors, 0 warnings.
- Patch 327: checkpatch 0 errors, 1 warning. The warning requests a
  separate DT-binding patch; it is a submission-organization warning rather
  than a code defect.

## Full OpenWrt build status

A full `make defconfig` / firmware build was attempted in a disposable tree but
stopped at host prerequisite checks because this execution environment lacks
`ncurses.h`/`libncurses.so` and GNU awk. The delivered tree was not modified by
that failed attempt. The exact log is retained as
`validation/openwrt-defconfig-prerequisite-failure.log`.

Therefore this delivery does **not** claim a completed MIPS firmware build.

## Hardware validation still required

Before production use, each PCB/chip revision needs serial recovery and tests
for reset polarity, SMI timing, RGMII delay sweep, FCS counters, DSA tag RX/TX,
port source decode, VLAN isolation, sustained bidirectional traffic, MTU,
reprobe/reset, optional SSC-off mode, and flash persistence. RD05 specifically
still needs confirmation of inbound LAN RX descriptor completion and DSA slave
delivery on physical hardware.
