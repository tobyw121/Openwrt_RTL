#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-only
# RD05 serial recovery guard.  UART0 currently has to run without a validated
# IRQ route on this board.  In polling mode fast pasted input can overrun the
# 16-byte FIFO, so prefer software input flow control and quiet the kernel
# console once the root shell is active.

[ -t 0 ] || return 0

case "$(cat /tmp/sysinfo/board_name 2>/dev/null) $(tr '\0' ' ' </proc/device-tree/compatible 2>/dev/null)" in
	*xiaomi,r4-rd05*|*r4-rd05*) ;;
	*) return 0 ;;
esac

tty_path="$(readlink /proc/self/fd/0 2>/dev/null)"
case "$tty_path" in
	/dev/ttyS0|/dev/console) ;;
	*) return 0 ;;
esac

# ixoff asks the tty layer to send XOFF/XON to terminals that support it;
# this is the only safe mitigation while UART IRQ remains disabled.
stty -F "$tty_path" sane echo icanon isig ixon ixoff -crtscts min 1 time 0 2>/dev/null || true
dmesg -n 1 2>/dev/null || true
