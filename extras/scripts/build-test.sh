#!/bin/bash
# usage: build-test.sh <name>  -> images/uImage-<name> (uncompressed Image, test cmdline + tiny initramfs)
set -e
cd /home/user/Documents/MC74/lineage/kernel-mc74
export PATH=$HOME/Documents/MC74/toolchains/armv7-eabihf--glibc--stable/bin:$PATH
export LOCALVERSION=
M="make O=../kobj ARCH=arm CROSS_COMPILE=arm-linux- KCFLAGS=-std=gnu89"
$M mc74_defconfig >/dev/null 2>&1
./scripts/config --file ../kobj/.config --set-str CMDLINE "${CMDLINE:-console=ttyS0,115200n8 mem=510M rdinit=/init earlyprintk keep_bootcon debug l2off panic=5 oops=panic capri_dormant.dormant_disable=1}" --enable CMDLINE_FORCE --enable BLK_DEV_INITRD --set-str INITRAMFS_SOURCE "${INITRAMFS:-/home/user/Documents/MC74/build-output/initramfs/initramfs.list}" --enable DEBUG_LL --enable EARLY_PRINTK $EXTRA
$M oldconfig </dev/null >/dev/null 2>&1
$M -j4 Image > ../kbuild.log 2>&1 || { grep -E "error:|undefined" ../kbuild.log | head -20; exit 1; }
cd ..
../build-output/uboot-tools/tools/mkimage -A arm -O linux -T kernel -C none -a 0xA0008000 -e 0xA0008000 -n "MC74 3.0.101 $1" -d kobj/arch/arm/boot/Image images/uImage-$1 >/dev/null
cp kobj/.config images/config-$1
echo built images/uImage-$1
