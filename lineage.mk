# Inherit from the common Open Source product configuration (no telephony)
$(call inherit-product, $(SRC_TARGET_DIR)/product/core_minimal.mk)

# Inherit device configuration
$(call inherit-product, device/meraki/mc74/device_mc74.mk)

# Inherit some common Lineage stuff (tablet-style, no phone apps)
$(call inherit-product, vendor/cm/config/common_mini_tablet_wifionly.mk)

PRODUCT_NAME := lineage_mc74
PRODUCT_DEVICE := mc74
PRODUCT_BRAND := Meraki
PRODUCT_MANUFACTURER := Cisco Meraki
PRODUCT_MODEL := MC74
PRODUCT_CHARACTERISTICS := tablet,nosdcard
