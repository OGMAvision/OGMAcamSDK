LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)  
LOCAL_MODULE := ogmacam
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libogmacam.so
include $(PREBUILT_SHARED_LIBRARY)