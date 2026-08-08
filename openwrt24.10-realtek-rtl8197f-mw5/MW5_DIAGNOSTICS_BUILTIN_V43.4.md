# MW5 diagnostics built into OpenWrt v43.4

No SCP, TFTP, USB storage or network connection is required to place the test
scripts on the router. They are copied into the squashfs image through the
Realtek target `base-files` overlay.

After flashing, use the serial console:

```
mw5-netdiag help
mw5-netdiag state
mw5-netdiag full
mw5-netdiag direct
```

The two previously distributed scripts remain callable by their exact names:

```
MW5_V43.3_DIRECT_LAN_DIAG_FIX.sh direct
MW5_V43.2_RUNTIME_TX_NETWORK_TEST.sh
```

The preferred command is `mw5-netdiag`, because it uses a common result
directory, preserves MAC addresses during temporary topology changes, creates
versioned backups and provides a single `restore` action.
