# v43.4 - MW5 diagnostics built into the image

This revision does not change the RTL8197F/RTL8367 data path introduced by
v43.3. It embeds every MW5 runtime diagnostic that was previously supplied as
an external file, so tests can be executed immediately from the serial console.

## Added to target base-files

- `/usr/sbin/mw5-netdiag`
  - `state`, `full`, `boot`, `watch`
  - `direct`, `bridge`, `restore`
  - `legacy-tx-test`
- `/usr/sbin/MW5_V43.3_DIRECT_LAN_DIAG_FIX.sh`
- `/usr/sbin/MW5_V43.2_RUNTIME_TX_NETWORK_TEST.sh`
- `/etc/init.d/mw5-diag`
- `/etc/uci-defaults/97-mw5-diag-enable`
- `/etc/profile.d/98-mw5-diag-help.sh`
- `/etc/mw5-diag-help`

## Automatic capture

On the MW5, `/etc/init.d/mw5-diag` creates a read-only post-boot report at:

```
/tmp/mw5-diag/boot-latest.txt
```

Manual reports are stored below `/tmp/mw5-diag/` and linked or copied to
`/tmp/mw5-diag/latest.txt`.

## Standard serial-console workflow

```
mw5-netdiag state before
mw5-netdiag direct
mw5-netdiag state after-direct
mw5-netdiag restore
```
