# Legacy-SDK-Kbuild-Probe

Die proprietären SDK-Kernelmodule wurden versuchsweise als externe Module gegen lokal vorhandene Linux-6.12-Header kompiliert. Dieser Lauf ist **kein Ersatz für einen Linux-6.6.114/OpenWrt-Build**; er diente ausschließlich dazu, die API-Distanz des Linux-2.6-SDKs sichtbar zu machen.

Repräsentative Blocker:

- fehlende BSP-/Plattformheader: `bspchip.h`, `soc/plf/plf_interrupt_setting.h`,
- entfernter `file_operations.ioctl`-Callback,
- entfernte Netzwerkfelder/Callbacks: `ndo_set_multicast_list`, `net_device.last_rx`, `net_device.trans_start`,
- entfernte Zeit-/Task-/Waitqueue-APIs: `do_gettimeofday`, `interruptible_sleep_on_timeout`, alte Task-State-Makros,
- entfernte Architekturhelper: `dma_cache_wback_inv`,
- direkte 32-Bit-Integer-/Pointer-MMIO-Konvertierungen,
- nicht zueinander passende SDK-DAL-/NIC-Strukturen und Funktionssignaturen,
- chipabhängige, unvollständige RTL8390ES-Registerdefinitionen.

Die Proben belegen, dass ein seriöser Port nicht durch wenige Kompatibilitätsmakros erreicht wird. Die Treiberfunktionen müssen in DSA, phylib, gpiolib, MFD, LED, watchdog, serial, MTD und die übrigen Linux-Subsysteme überführt werden. Genau diese nativen Pfade sind im Realtek-Target aktiviert; die nicht abgedeckten RTL8328-/RTL8389-/RTL8208-/RTL8212-Bereiche bleiben explizite Lücken.
