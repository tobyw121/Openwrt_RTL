# RTL8197F v42.8

## MW5 Linux-entry watchdog diagnostics

- Correct the RTL8197F/RTL8197FS watchdog stop value from `0xa5000000` to the SDK-defined `0xa5f00000` in the LZMA loader, loader board code and Linux early platform code.
- Stop the watchdog directly in generic MIPS `kernel_entry`, before CPU setup and BSS clearing.
- Emit bounded, polling-only UART markers before the normal Linux console exists:
  - `K`: raw Linux kernel entry reached;
  - `S`: CPU/BSS/stack entry work completed and `start_kernel` is about to run;
  - `P`: RTL8197F `prom_init()` reached.
- Use UART byte accesses and select THR `+0x24` for RTL8197F/FS or `+0x00` for FH/VG.
- Add a checksum-corrected MW5 diagnostic full-flash image whose decompressed kernel is instrumented with the same K/S/P stages. This direct image is for diagnosis; the source tree produces equivalent markers after a clean rebuild.

The v42.7 direct image fixed only the already-built LZMA loader. It still contained the older v42.6 Linux kernel, so the `prom_init()` source changes were not present in that binary.
