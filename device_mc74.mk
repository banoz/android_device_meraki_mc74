LOCAL_PATH := device/meraki/mc74

# MC74's own VideoCore blobs
$(call inherit-product, vendor/meraki/mc74/mc74-vendor.mk)

DEVICE_PACKAGE_OVERLAYS += $(LOCAL_PATH)/overlay

# 800x1280 portrait panel, 160 dpi in stock
TARGET_SCREEN_WIDTH := 800
TARGET_SCREEN_HEIGHT := 1280
PRODUCT_AAPT_CONFIG := normal mdpi tvdpi hdpi
PRODUCT_AAPT_PREF_CONFIG := mdpi

# Init
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/rootdir/fstab.capri:root/fstab.capri \
    $(LOCAL_PATH)/rootdir/init.capri.rc:root/init.capri.rc \
    $(LOCAL_PATH)/rootdir/init.capri.usb.rc:root/init.capri.usb.rc \
    $(LOCAL_PATH)/rootdir/ueventd.capri.rc:root/ueventd.capri.rc \
    $(LOCAL_PATH)/idc/cyttsp5.idc:system/usr/idc/cyttsp5.idc

# Ethernet is the MC74's network
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.ethernet.xml:system/etc/permissions/android.hardware.ethernet.xml \
    frameworks/native/data/etc/android.hardware.touchscreen.multitouch.jazzhand.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.jazzhand.xml \
    frameworks/native/data/etc/tablet_core_hardware.xml:system/etc/permissions/tablet_core_hardware.xml

PRODUCT_PACKAGES += \
    libstlport \
    power.default \
    lights.capri

# Low-RAM device: 512 MB
PRODUCT_PROPERTY_OVERRIDES += \
    ro.config.low_ram=true \
    qemu.hw.mainkeys=0 \
    ro.sf.lcd_density=160 \
    ro.sf.hwrotation=270 \
    ro.opengles.version=131072 \
    ro.zygote.disable_gl_preload=1 \
    debug.hwui.render_dirty_regions=false \
    config.disable_atlas=true \
    dalvik.vm.jit.codecachesize=0 \
    ro.config.max_starting_bg=8 \
    persist.sys.force_highendgfx=true \
    ro.sys.fw.bg_apps_limit=8 \
    persist.brcm.log=none \
    persist.brcm.ap_crash=none \
    persist.brcm.cp_crash=none

$(call inherit-product, frameworks/native/build/phone-hdpi-512-dalvik-heap.mk)
PRODUCT_PROPERTY_OVERRIDES += \
    dalvik.vm.heapgrowthlimit=64m \
    dalvik.vm.heapsize=128m

# ADB over USB and network, no auth during bring-up
ADDITIONAL_DEFAULT_PROPERTIES += \
    ro.adb.secure=0 \
    ro.secure=0 \
    service.adb.root=1 \
    persist.sys.usb.config=adb \
    service.adb.tcp.port=5555

# Fonts, hyphenation and keyboard maps (core_minimal has none; zygote aborts without fonts)
$(call inherit-product-if-exists, frameworks/base/data/fonts/fonts.mk)
$(call inherit-product-if-exists, external/google-fonts/dancing-script/fonts.mk)
$(call inherit-product-if-exists, external/google-fonts/carrois-gothic-sc/fonts.mk)
$(call inherit-product-if-exists, external/google-fonts/coming-soon/fonts.mk)
$(call inherit-product-if-exists, external/google-fonts/cutive-mono/fonts.mk)
$(call inherit-product-if-exists, external/noto-fonts/fonts.mk)
$(call inherit-product-if-exists, external/naver-fonts/fonts.mk)
$(call inherit-product-if-exists, external/roboto-fonts/fonts.mk)
$(call inherit-product-if-exists, external/hyphenation-patterns/patterns.mk)
$(call inherit-product-if-exists, frameworks/base/data/keyboards/keyboards.mk)
$(call inherit-product-if-exists, frameworks/webview/chromium/chromium.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/core.mk)

# Pieces of generic_no_telephony that core_minimal lacks (no SystemUI = no boot)
PRODUCT_PACKAGES += \
    SystemUI \
    Provision \
    OneTimeInitializer \
    WallpaperCropper \
    librs_jni \
    local_time.default \
    vibrator.default \
    screenrecord
PRODUCT_COPY_FILES += \
    frameworks/av/media/libeffects/data/audio_effects.conf:system/etc/audio_effects.conf

# Wi-Fi (BCM4339, backported brcmfmac + stock MC74 firmware from vendor)
PRODUCT_PACKAGES += \
    wpa_supplicant \
    wpa_cli \
    hostapd \
    libwpa_client
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/wifi/wpa_supplicant.conf:system/etc/wifi/wpa_supplicant.conf \
    $(LOCAL_PATH)/wifi/wpa_supplicant_overlay.conf:system/etc/wifi/wpa_supplicant_overlay.conf \
    $(LOCAL_PATH)/modules/compat.ko:system/lib/modules/compat.ko \
    $(LOCAL_PATH)/modules/cfg80211.ko:system/lib/modules/cfg80211.ko \
    $(LOCAL_PATH)/modules/brcmutil.ko:system/lib/modules/brcmutil.ko \
    $(LOCAL_PATH)/modules/brcmfmac.ko:system/lib/modules/brcmfmac.ko \
    frameworks/native/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml
PRODUCT_PROPERTY_OVERRIDES += \
    wifi.interface=wlan0 \
    wifi.supplicant_scan_interval=15

# Audio: built-in speaker + mic (kernel MC74_CAPH ALSA card, AOSP usbaudio-based HAL)
PRODUCT_PACKAGES += \
    audio.primary.capri \
    audio.r_submix.default \
    tinyplay \
    tinycap \
    tinymix
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/audio/audio_policy.conf:system/etc/audio_policy.conf

# USB gadget re-plug after a failed enumeration
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/rootdir/mc74-usbcheck.sh:system/etc/mc74-usbcheck.sh

# Bluetooth (BCM4339, stock patch file from vendor)
PRODUCT_PACKAGES += \
    Bluetooth \
    libbt-vendor \
    bluetooth.default \
    audio.a2dp.default
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/bluetooth/bt_vendor.conf:system/etc/bluetooth/bt_vendor.conf \
    frameworks/native/data/etc/android.hardware.bluetooth.xml:system/etc/permissions/android.hardware.bluetooth.xml \
    frameworks/native/data/etc/android.hardware.bluetooth_le.xml:system/etc/permissions/android.hardware.bluetooth_le.xml

# Media codecs (software)
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/media/media_codecs.xml:system/etc/media_codecs.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_audio.xml:system/etc/media_codecs_google_audio.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_telephony.xml:system/etc/media_codecs_google_telephony.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_video_le.xml:system/etc/media_codecs_google_video_le.xml

# Ambient light sensor (CM3232) for auto-brightness
PRODUCT_PACKAGES += \
    sensors.capri
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.sensor.light.xml:system/etc/permissions/android.hardware.sensor.light.xml
