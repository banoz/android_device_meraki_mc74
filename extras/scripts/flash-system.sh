#!/bin/bash
# Writes images/system-raw.img to the MC74's userdata partition (p24) and verifies it.
set -e
cd /home/user/Documents/MC74/lineage
H=root@10.74.0.1
ssh $H 'grep -q "PARTNAME=userdata" /sys/class/block/mmcblk0p24/uevent && [ "$(cat /sys/class/block/mmcblk0p24/size)" = 1990656 ] && ! grep -q mmcblk0p24 /proc/mounts' || { echo "p24 check failed"; exit 1; }
gzip -1 -c images/system-raw.img | ssh $H 'gunzip -c | dd of=/dev/mmcblk0p24 bs=4M conv=fsync 2>&1 | tail -1'
R=$(ssh $H 'sync; echo 3 > /proc/sys/vm/drop_caches; md5sum /dev/mmcblk0p24' | cut -d' ' -f1)
L=$(cut -d' ' -f1 images/system-raw.md5)
echo "device $R / local $L"; [ "$R" = "$L" ]
