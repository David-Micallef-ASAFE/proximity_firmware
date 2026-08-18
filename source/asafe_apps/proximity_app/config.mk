# Boards compatible with this app 
TARGET_BOARDS := pca10040 asMBT02 asE9 asC10
#
# Network default settings configuration
#

# If this section is removed, node has to be configured in
# a different way
default_network_address ?= 0xFFC72C
default_network_channel ?= 37
# For running this application with network encryption, the following key definitions may be uncommented and
# filled with random data (exactly 16 bytes each). Also `allow_insecure_key_injection` needs to be set
# to 'yes'. Note that the keys end up as plaintext in device flash with this mechanism. Not for
# production use!
#
# For production use, use a secure provisioning method or refer to the app_setup library in the SDK.

#default_network_cipher_key ?= 0x??,0x??,0x??,0x??,0x??,0x??,0x??,0x??,0x??,0x??,0x??,0x??,0x??,0x??,0x??,0x??
#default_network_authen_key ?= 0x??,0x??,0x??,0x??,0x??,0x??,0x??,0x??,0x??,0x??,0x??,0x??,0x??,0x??,0x??,0x??
#allow_insecure_key_injection=yes

#
# App specific configuration
#

# Define a specific application area_id - This was moved into the specific config
#app_specific_area_id=0x83744C

# Use Shared Data
SHARED_DATA=yes

#Printing and Debugging
APP_PRINTING=no

# Use App Scheduler and configure it
APP_SCHEDULER=yes
APP_SCHEDULER_TASKS=8

# Enable Local Provisioning library
LOCAL_PROVISIONING=yes
STACK_STATE_LIB=yes

##Storage
APP_SECURE_STORAGE=yes

# App version
app_major=2
app_minor=0
app_maintenance=0
app_development=0



#Tags
# -include $(APP_SRCS_PATH)/app_config/tag/Minew/MBT02.mk
# -include $(APP_SRCS_PATH)/app_config/tag/Minew/E9.mk
-include $(APP_SRCS_PATH)/app_config/tag/Minew/C10.mk