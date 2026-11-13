#!/bin/bash
# Builds the installable MC74 Lineage kernel for boot2 (p16):
# uncompressed Image (zImage hangs in its decompressor on the MC74) with the
# Lineage ramdisk built in, gzip-wrapped as a uImage that u-boot's bootcmd
# (`bootm 90000000` from boot2) unpacks.  Output: images/uImage-lineage-prod.gz
set -e
cd /home/user/Documents/MC74/lineage
python3 -I make-initramfs-list.py
CMDLINE="console=ttyS0,115200n8 androidboot.console=ttyS0 androidboot.hardware=capri androidboot.selinux=enforcing mem=510M panic=10 capri_dormant.dormant_disable=1 bcmmac=${MC74_MAC:-xx-xx-xx-xx-xx-xx}" \
INITRAMFS=/home/user/Documents/MC74/lineage/images/android-initramfs.list \
EXTRA="--disable DEBUG_LL --disable EARLY_PRINTK" ./build-test.sh lineage-prod
gzip -9 -c kobj/arch/arm/boot/Image > images/Image-lineage-prod.gz
../build-output/uboot-tools/tools/mkimage -A arm -O linux -T kernel -C gzip -a 0xA0008000 -e 0xA0008000 \
  -n "MC74 LineageOS 14.1" -d images/Image-lineage-prod.gz images/uImage-lineage-prod.gz
