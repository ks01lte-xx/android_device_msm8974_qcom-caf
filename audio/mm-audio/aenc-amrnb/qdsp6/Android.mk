ifneq ($(BUILD_TINY_ANDROID),true)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

# ---------------------------------------------------------------------------------
#                 Common definitons
# ---------------------------------------------------------------------------------

libOmxAmrEnc-def := -g -O3
libOmxAmrEnc-def += -DQC_MODIFIED
libOmxAmrEnc-def += -D_ANDROID_
libOmxAmrEnc-def += -D_ENABLE_QC_MSG_LOG_
libOmxAmrEnc-def += -DVERBOSE
libOmxAmrEnc-def += -D_DEBUG
ifeq ($(strip $(TARGET_USES_QCOM_MM_AUDIO)),true)
libOmxAmrEnc-def += -DAUDIOV2
endif

libOmxAmrEnc-def += -Wno-sign-conversion -Wno-shorten-64-to-32 -Wno-self-assign -Wno-parentheses-equality -Wno-format -Wno-sign-compare -Wno-tautological-compare -Wno-shorten-64-to-32 -Wno-unused-local-typedef
# ---------------------------------------------------------------------------------
#             Make the Shared library (libOmxAmrEnc)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)

libOmxAmrEnc-inc       := $(LOCAL_PATH)/inc

LOCAL_MODULE            := libOmxAmrEnc
LOCAL_MODULE_TAGS       := optional
LOCAL_VENDOR_MODULE     := true
LOCAL_CFLAGS            := $(libOmxAmrEnc-def)
LOCAL_CFLAGS            += -Wno-error
LOCAL_C_INCLUDES        := $(libOmxAmrEnc-inc)
LOCAL_SHARED_LIBRARIES  := libutils liblog
LOCAL_SRC_FILES         := src/aenc_svr.c
LOCAL_SRC_FILES         += src/omx_amr_aenc.cpp

LOCAL_HEADER_LIBRARIES  := generated_kernel_headers media_headers

include $(BUILD_SHARED_LIBRARY)

endif

# ---------------------------------------------------------------------------------
#                     END
# ---------------------------------------------------------------------------------

