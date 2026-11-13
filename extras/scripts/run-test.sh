#!/bin/bash
# usage: run-test.sh <name> [capture-seconds]  -> RAM-boots images/uImage-<name>, log in boot-<name>.log
cd /home/user/Documents/MC74/lineage
rm -f usbboot-$1.out
(python3 -I ../build-output/usbboot.py images/uImage-$1 boot-$1.log --display --capture ${2:-90} > usbboot-$1.out 2>&1 &)
sleep 1
ssh -o ConnectTimeout=8 root@10.74.0.1 'sync; reboot' >/dev/null 2>&1 || echo "(ssh reboot failed; reboot the MC74 by hand)"
timeout 400 bash -c "until grep -q 'capture done\|never appeared\|failed' usbboot-$1.out; do sleep 3; done"
cat usbboot-$1.out | tail -3
