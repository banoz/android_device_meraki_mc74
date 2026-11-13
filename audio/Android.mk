# MC74 primary audio HAL: the AOSP usbaudio HAL (tinyalsa + alsa_utils)
# bound to the built-in "MC74" ALSA card (speaker playback, mic capture).
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := audio.primary.capri
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_SRC_FILES := audio_hw.c
LOCAL_C_INCLUDES += \
	external/tinyalsa/include \
	$(call include-path-for, audio-utils) \
	$(call include-path-for, alsa-utils)
LOCAL_SHARED_LIBRARIES := liblog libcutils libtinyalsa libaudioutils libalsautils
LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS := -Wno-unused-parameter
include $(BUILD_SHARED_LIBRARY)
