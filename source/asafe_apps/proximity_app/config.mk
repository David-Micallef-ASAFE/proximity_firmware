# Boards compatible with this app 
TARGET_BOARDS := pca10040 asMBT02 asE9 asC10 asK4W asC3W asJIG asAnchorHR asAssetTagHRWithRetrieval asAssetTagSRLarge 
#
# Network default settings configuration
#

# If this section is removed, node has to be configured in
# a different way
default_network_address ?= 11259375
default_network_channel ?= 37
# For running this application with network encryption, the following key definitions may be uncommented and
# filled with random data (exactly 16 bytes each). Also `allow_insecure_key_injection` needs to be set
# to 'yes'. Note that the keys end up as plaintext in device flash with this mechanism. Not for
# production use!
#
# For production use, use a secure provisioning method or refer to the app_setup library in the SDK.

default_network_cipher_key ?= 0xaa,0xbb,0xaa,0xbb,0xaa,0xbb,0xaa,0xbb,0xaa,0xbb,0xaa,0xbb,0xaa,0xbb,0xaa,0xbb
default_network_authen_key ?= 0xcc,0xdd,0xcc,0xdd,0xcc,0xdd,0xcc,0xdd,0xcc,0xdd,0xcc,0xdd,0xcc,0xdd,0xcc,0xdd
allow_insecure_key_injection=yes

#
# App specific configuration
#

# Define a specific application area_id - This was moved into the specific config
#app_specific_area_id=0x83744C

# Hal Voltage is used accross all devices
HAL_VOLTAGE=yes

# Use Shared Data
SHARED_DATA=yes

#Printing and Debugging
APP_PRINTING=no

# Use App Scheduler and configure it
APP_SCHEDULER=yes
APP_SCHEDULER_TASKS=10

# Enable Local Provisioning library
LOCAL_PROVISIONING=yes
STACK_STATE_LIB=yes

# Enable App Setup to safely load the keys
APP_SETUP=yes

# HAL_HW_DELAY=yes


#Configurations - Note that below we assume one configuration per board
#Minew MBT02
ifeq ($(target_board),asMBT02)
-include $(APP_SRCS_PATH)/app_config/tag/Minew/MBT02.mk
endif
#Minew E9
ifeq ($(target_board),asE9)
-include $(APP_SRCS_PATH)/app_config/tag/Minew/E9.mk
endif
#Minew C10
ifeq ($(target_board),asC10)
-include $(APP_SRCS_PATH)/app_config/tag/Minew/C10.mk
endif
#KKM K4W
ifeq ($(target_board),asK4W)
-include $(APP_SRCS_PATH)/app_config/tag/KKM/K4W.mk
endif
#KKM C3W
ifeq ($(target_board),asC3W)
#Since we use this device for both roles we enable only one of these
-include $(APP_SRCS_PATH)/app_config/tag/KKM/C3W.mk
# -include $(APP_SRCS_PATH)/app_config/anchor/KKM/C3W.mk
endif
#ASAFE JIG
ifeq ($(target_board),asJIG)
-include $(APP_SRCS_PATH)/app_config/anchor/ASAFE/JIG.mk
# -include $(APP_SRCS_PATH)/app_config/tag/ASAFE/JIG.mk
endif
#ASAFE DK BOARD - TESTING ONLY
ifeq ($(target_board),pca10040)
-include $(APP_SRCS_PATH)/app_config/anchor/NORDIC/NRF52832DK.mk
# -include $(APP_SRCS_PATH)/app_config/tag/ASAFE/JIG.mk
endif

#ASAFE AnchorHR
ifeq ($(target_board),asAnchorHR)
-include $(APP_SRCS_PATH)/app_config/anchor/ASAFE/anchorHR.mk
# -include $(APP_SRCS_PATH)/app_config/tag/ASAFE/JIG.mk
endif

#ASAFE Asset Tag HR With Retrieval
ifeq ($(target_board),asAssetTagHRWithRetrieval)
-include $(APP_SRCS_PATH)/app_config/tag/ASAFE/assetTagHRWithRetrieval.mk
endif

#ASAFE Asset Tag SR Large
ifeq ($(target_board),asAssetTagSRLarge)
-include $(APP_SRCS_PATH)/app_config/tag/ASAFE/assetTagSRLarge.mk
endif



#Versions Launched
#1.0.1.0 - Initial Version used during order of devices from Minew and KKM for proximity demo.  App simply launches the device into low power, provisions the device but has no further capabilities.  App is able to send status update
#1.0.1.2 - Used as a version to test OTAP.  This version is flashed to the device and 1.3 is then the OTAPed version.
#1.0.1.3 - Used internally for testing and development.
#1.0.1.4 - Release version for ASAFE.  Manufactured Devices and OTAPed devices (KKM and MINEW) received this version in Nov 2026

# App version
app_major=1
app_minor=0
app_maintenance=1
app_development=4
