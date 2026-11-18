# LineageOS 14.1 (Android 7.1) on the Cisco Meraki MC74

Hardware-accelerated LineageOS 14.1 for the MC74, built on the Samsung Galaxy Grand Duos
(GT-I9082) port, which uses the same BCM28155 "Capri" SoC. It reuses the MC74's own Android 4.2
VideoCore graphics libraries, which match the unit's VideoCore firmware.

## Status

| Area | State |
|---|---|
| Boot | stock u-boot runs a gzip uImage (uncompressed kernel + built-in ramdisk) from `boot2`; stock Android stays as fallback; the stock Cisco Meraki splash is redrawn by the kernel until the boot animation |
| Graphics | VideoCore IV GLES 2.0 via the MC74's `libGLES_vc4`, `gralloc.capri`, `hwcomposer.capri`; smooth UI |
| Display | 800x1280 panel shown as 1280x800 landscape (`ro.sf.hwrotation=270`), auto-rotate off; MP3309 backlight with I2C dimming (no flicker) |
| Touch | Cypress/Parade TTSP5 (backport of the mainline cyttsp5 driver); a touch wakes the screen, double-tap on the status bar sleeps it (the panel stays powered, only the backlight goes off) |
| Ethernet | ESUB switch (backport of the mainline kona-esub driver, RX interrupt), network jack = switch port 1 |
| Wi-Fi | BCM4339 on SDIO1 via backported brcmfmac (backports-3.14.22, see `wifi/README.md`) + stock firmware, nl80211 wpa_supplicant |
| Bluetooth | BCM4339 on ttyS2 (BT_REG_ON GPIO 82), stock patch file, A2DP output |
| Audio | handsfree speaker + mic, and the handset earpiece + handset mic (used in call/voice-chat mode): kernel ALSA card (`CONFIG_MC74_CAPH`, port of the mainline kona-caph driver) + AOSP usbaudio HAL pinned to hw:0,0 |
| Sensors | CM3232 ambient light (I2C bus 2) for auto-brightness |
| Lights | backlight, front RGBW LED (PCA9632 on I2C bus 2, channel order W,G,R,B) for notifications |
| USB | device-only transceiver, adb works; gadget re-plugged once after boot (the PC could enumerate it half-configured) |
| L2 cache | on; Capri **A3** needs the same PL310 physical-address remap as A2 |
| SELinux | enforcing |
| UI | no battery UI, no lock screen by default, trimmed Quick Settings (PoE wall panel) |
| Not yet | headset jack, proximity sensor (CM36283), suspend (blocked with a wakelock), the Meraki dialer |

## Repositories (all branch `cm-14.1`)

| Path | Repo |
|---|---|
| device/meraki/mc74 | banoz/android_device_meraki_mc74 (this repo) |
| kernel/meraki/mc74 | banoz/android_kernel_meraki_mc74 (I9082 3.0.101 kernel + MC74 board, `mc74_defconfig`) |
| vendor/meraki/mc74 | banoz/proprietary_vendor_meraki_mc74 (private; list in `extras/proprietary-files.txt`) |
| frameworks/av, frameworks/native, system/core | banoz forks with the I9082 compatibility patches (Osas514/build_tools) |
| vendor/cmsdk, packages/apps/CMParts, vendor/cm | banoz forks: LiveDisplay removed (no display-tuning hardware) |

## Build

```
repo init -u https://github.com/LineageOS/android.git -b cm-14.1 --depth=1
mkdir -p .repo/local_manifests && cp <this repo>/manifest/mc74.xml .repo/local_manifests/
repo sync -c --no-tags
# kernel (armv7-eabihf gcc, KCFLAGS=-std=gnu89, LOCALVERSION_AUTO off, LOCALVERSION= exported):
#   extras/scripts/build-test.sh / make-boot2-image.sh; Wi-Fi modules: wifi/README.md
# Android: extras/docker (Ubuntu 16.04 + JDK 8): extras/docker/start-build.sh "bootimage systemimage" 3
```

`extras/` holds the build container, RAM-test/flash scripts (paths assume ~/Documents/MC74),
small register tools and notes.

## Partition use on the test unit

| Partition | Lineage use |
|---|---|
| p16 `boot2` | kernel uImage (`scripts/make-boot2-image.sh`) |
| p24 `userdata` | /system |
| p18 `system2` | /data |
| /cache | tmpfs |

Back up every partition before writing it.

## Notes

- Compressed zImages hang in their decompressor on this board; ship the uncompressed Image
  (gzip-wrapped uImage, unpacked by u-boot).
- u-boot's `bootm` passes ATAGs out of the kernel's reach; the kernel falls back to a built-in
  copy of the MC74 dt-blob, and the command line is built in (`CONFIG_CMDLINE_FORCE`).
- The secure monitor leaves PL310 AUX_CTRL at 0x1e050000; the kernel can't change it from the
  normal world. With the A3 remap, `mc74_dmatest.autorun=1` (patch 0011) passes 12/12 with L2 on.
