# Meraki MC74 (BCM28155 "capri", board CAPRI_ME1). Based on the I9082 Lineage 14.1 tree.

-include vendor/meraki/mc74/BoardConfigVendor.mk

# Platform
TARGET_BOARD_PLATFORM := capri
TARGET_BOOTLOADER_BOARD_NAME := capri
TARGET_NO_BOOTLOADER := true
TARGET_NO_RADIOIMAGE := true

# CPU
TARGET_ARCH := arm
TARGET_CPU_ABI := armeabi-v7a
TARGET_CPU_ABI2 := armeabi
TARGET_ARCH_VARIANT := armv7-a-neon
TARGET_CPU_VARIANT := cortex-a9
TARGET_CPU_SMP := true

# Kernel: built outside the tree (lineage/kernel-mc74, mc74_defconfig) and dropped in prebuilt/
TARGET_PREBUILT_KERNEL := device/meraki/mc74/prebuilt/zImage
BOARD_KERNEL_CMDLINE := console=ttyS0,115200n8 androidboot.console=ttyS0 androidboot.hardware=capri mem=510M bcmmac=xx-xx-xx-xx-xx-xx
BOARD_KERNEL_BASE := 0xa0000000
BOARD_KERNEL_PAGESIZE := 2048
# Stock MC74 boot.img: kernel 0xa0008000, ramdisk 0xa1000000, tags 0xa0000100
BOARD_MKBOOTIMG_ARGS := --ramdisk_offset 0x01000000 --tags_offset 0x00000100

# Partitions (stock MC74 GPT, sizes in bytes)
TARGET_USERIMAGES_USE_EXT4 := true
BOARD_BOOTIMAGE_PARTITION_SIZE := 8388608
BOARD_RECOVERYIMAGE_PARTITION_SIZE := 16777216
BOARD_SYSTEMIMAGE_PARTITION_SIZE := 1019215872
BOARD_SYSTEMIMAGE_JOURNAL_SIZE := 0
BOARD_USERDATAIMAGE_PARTITION_SIZE := 1019215872
BOARD_CACHEIMAGE_PARTITION_SIZE := 1342177280
BOARD_CACHEIMAGE_FILE_SYSTEM_TYPE := ext4
BOARD_FLASH_BLOCK_SIZE := 131072

# Recovery
TARGET_RECOVERY_FSTAB := device/meraki/mc74/rootdir/fstab.capri
TARGET_RECOVERY_PIXEL_FORMAT := ABGR_8888

# Graphics: Broadcom VideoCore stack from the MC74's own 4.2 firmware (vendor/meraki/mc74)
USE_OPENGL_RENDERER := true
BOARD_EGL_CFG := device/meraki/mc74/configs/egl.cfg
BOARD_EGL_WORKAROUND_BUG_10194508 := true
TARGET_RUNNING_WITHOUT_SYNC_FRAMEWORK := true
TARGET_REQUIRES_SYNCHRONOUS_SETSURFACE := true
NUM_FRAMEBUFFER_SURFACE_BUFFERS := 3
BOARD_GLOBAL_CFLAGS += -DNEEDS_VECTORIMPL_SYMBOLS -DCAPRI_HWC -DREFBASE_JB_MR1_COMPAT_SYMBOLS
BOARD_GLOBAL_CFLAGS += -DDISABLE_ASHMEM_TRACKING

# No modem, Bluetooth, GPS or camera on the MC74 (for now)
BOARD_HAVE_BLUETOOTH := false
USE_CAMERA_STUB := true

# 512 MB of RAM
MALLOC_SVELTE := true

# Some of our vendor libs have text relocations
TARGET_NEEDS_PLATFORM_TEXT_RELOCATIONS := true

WITHOUT_CHECK_API := true
ANDROID_NO_TEST_CHECK := true

# SELinux
BOARD_KERNEL_CMDLINE += androidboot.selinux=enforcing
BOARD_SEPOLICY_DIRS += device/meraki/mc74/sepolicy

# Wi-Fi: BCM4339 on SDIO1 with backported brcmfmac (plain nl80211, no bcmdhd
# private commands). Modules load from init.capri.rc; the framework never
# loads or unloads them.
WPA_SUPPLICANT_VERSION      := VER_0_8_X
BOARD_WPA_SUPPLICANT_DRIVER := NL80211
BOARD_HOSTAPD_DRIVER        := NL80211

# Bluetooth: BCM4339 on UART ttyS2, BT_REG_ON GPIO 82 (kernel bcmbt rfkill)
BOARD_HAVE_BLUETOOTH := true
BOARD_HAVE_BLUETOOTH_BCM := true
BOARD_CUSTOM_BT_CONFIG := device/meraki/mc74/bluetooth/libbt_vndcfg.txt
BOARD_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR := device/meraki/mc74/bluetooth
