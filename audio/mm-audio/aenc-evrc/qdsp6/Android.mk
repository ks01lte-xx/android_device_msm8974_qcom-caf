ifneq ($(BUILD_TINY_ANDROID),true)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

# ---------------------------------------------------------------------------------
#                 Common definitons
# ---------------------------------------------------------------------------------

libOmxEvrcEnc-def := -g -O3
libOmxEvrcEnc-def += -DQC_MODIFIED
libOmxEvrcEnc-def += -D_ANDROID_
libOmxEvrcEnc-def += -D_ENABLE_QC_MSG_LOG_
libOmxEvrcEnc-def += -DVERBOSE
libOmxEvrcEnc-def += -D_DEBUG
ifeq ($(strip $(TARGET_USES_QCOM_MM_AUDIO)),true)
libOmxEvrcEnc-def += -DAUDIOV2
endif

libOmxEvrcEnc-def += -Wno-sign-conversion -Wno-shorten-64-to-32 -Wno-self-assign -Wno-parentheses-equality -Wno-format -Wno-sign-compare -Wno-tautological-compare -Wno-shorten-64-to-32 -Wno-unused-local-typedef
# ---------------------------------------------------------------------------------
#             Make the Shared library (libOmxEvrcEnc)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)

libOmxEvrcEnc-inc       := $(LOCAL_PATH)/inc

LOCAL_MODULE            := libOmxEvrcEnc
LOCAL_MODULE_TAGS       := optional
LOCAL_VENDOR_MODULE     := true
LOCAL_CFLAGS            := $(libOmxEvrcEnc-def)
LOCAL_CFLAGS            += -Wno-error
LOCAL_C_INCLUDES        := $(libOmxEvrcEnc-inc)
LOCAL_SHARED_LIBRARIES  := libutils liblog
LOCAL_SRC_FILES         := src/aenc_svr.c
LOCAL_SRC_FILES         += src/omx_evrc_aenc.cpp

LOCAL_HEADER_LIBRARIES  := generated_kernel_headers media_headers

include $(BUILD_SHARED_LIBRARY)

endif

# ---------------------------------------------------------------------------------
#                     END
# ---------------------------------------------------------------------------------

