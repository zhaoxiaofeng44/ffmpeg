LOCAL_PATH := $(call my-dir)/..
include $(CLEAR_VARS)

LOCAL_MODULE := sqlite3_static
LOCAL_MODULE_FILENAME := libsqlite3

LOCAL_SRC_FILES := $(LOCAL_PATH)/cplusplus/sqlite3.c

LOCAL_C_INCLUDES := $(LOCAL_PATH)/cplusplus 
                  
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/cplusplus 

include $(BUILD_STATIC_LIBRARY)
