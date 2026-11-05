# You can add more sources here and libraries if needed
SRCS +=
INCLUDES +=
LIBS +=
LDFLAGS += 
LDLIBS +=-lm

#Includes
INCLUDES += -I$(SRCS_PATH)inc
INCLUDES += -I$(SRCS_PATH)drivers/lis3dh
INCLUDES += -I$(SRCS_PATH)drivers/i2c
INCLUDES += -I$(SRCS_PATH)drivers/led
INCLUDES += -I$(SRCS_PATH)drivers/buzzer
INCLUDES += -I$(SRCS_PATH)drivers/uwb
INCLUDES += -I$(SRCS_PATH)drivers/uwb/dw3000
INCLUDES += -I$(SRCS_PATH)drivers/uwb/port
INCLUDES += -I$(SRCS_PATH)drivers/uwb/probe_interface/
INCLUDES += -I$(SRCS_PATH)drivers/uwb/qmath
INCLUDES += -I$(SRCS_PATH)drivers/spi
INCLUDES += -I$(SRCS_PATH)drivers/gpio
INCLUDES += -I$(SRCS_PATH)uwb/inc
INCLUDES += -I$(SRCS_PATH)lib

# INCLUDES += -I$(SRCS_PATH)drivers/uwb
# INCLUDES += -I$(SRCS_PATH)drivers/uwb/decadriver
# INCLUDES += -I$(SRCS_PATH)drivers/uwb/decadriver/dw3000
# INCLUDES += -I$(SRCS_PATH)drivers/uwb/platform
# INCLUDES += -I$(SRCS_PATH)drivers/uwb/platform/nrf-sdk
# INCLUDES += -I$(SRCS_PATH)drivers/uwb_lite


#Device Type
CFLAGS += -DASAFE_DEVICE_TYPE=$(asafe_device_type)

#Application area id
CFLAGS += -DAPP_AREA_ID=$(app_specific_area_id)

#ASAFE Wirepas Role
CFLAGS += -DASAFE_WIREPAS_ROLE=$(asafe_wirepas_role)

# Example:
# SRCS += $(APP_SRCS_PATH)my_driver.c
# LIBS += my_lib.a
# include math.h lib
# LDLIBS += -lm

#ASAFE Provisioning Manager
SRCS += $(SRCS_PATH)as_provisioning.c

#ASAFE Status
SRCS += $(SRCS_PATH)as_status.c

#ASAFE Wirepas Connector
SRCS += $(SRCS_PATH)as_wirepas.c

#ASAFE Device <-> Cloud CBOR Protocol
SRCS += $(SRCS_PATH)as_cbor.c
SRCS += $(SRCS_PATH)as_protocol.c

#ASAFE GPIO Connector
SRCS += $(SRCS_PATH)drivers/gpio/as_gpio.c

#ASAFE Storage
SRCS += $(SRCS_PATH)as_storage.c

#ASAFE Proximity
SRCS += $(SRCS_PATH)as_proximity.c

#ASAFE ble
SRCS += $(SRCS_PATH)as_ble.c

#ASAFE ble bare-metal advertiser (Wirepas stack must be stopped)
SRCS += $(SRCS_PATH)as_bleBareMetal.c

#APP_LIS3DH - Accelerometer
ifeq ($(APP_LIS3DH),yes)
CFLAGS += -DAPP_LIS3DH
SRCS += $(SRCS_PATH)drivers/lis3dh/lis3dh.c
SRCS += $(SRCS_PATH)drivers/lis3dh/as_lis3dh.c
endif

#APP_I2C
ifeq ($(APP_I2C),yes)
CFLAGS += -DAPP_I2C
SRCS += $(SRCS_PATH)drivers/i2c/as_i2c.c
endif

#APP_LED
ifeq ($(APP_LED),yes)
CFLAGS += -DAPP_LED
SRCS += $(SRCS_PATH)drivers/led/as_led.c
endif

#APP_BUZZER
ifeq ($(APP_BUZZER),yes)
CFLAGS += -DAPP_BUZZER
SRCS += $(SRCS_PATH)drivers/buzzer/as_buzzer.c
endif


#APP_SPI
ifeq ($(APP_SPI),yes)
CFLAGS += -DAPP_SPI
SRCS += $(SRCS_PATH)drivers/spi/as_spi.c
endif

#APP_MANUFACTURING
ifeq ($(APP_MANUFACTURING),yes)
CFLAGS += -DAPP_MANUFACTURING
SRCS += $(SRCS_PATH)as_manufacturing.c
endif

#Allow NFC to act as a way to enable Manufacturing and to switch between inventory and run
ifeq ($(APP_ASAFE_NFC),yes)
CFLAGS += -DAPP_ASAFE_NFC
SRCS += $(SRCS_PATH)as_nfc.c
endif


#APP_BUTTON
ifeq ($(APP_BUTTON),yes)
CFLAGS += -DAPP_BUTTON
SRCS += $(SRCS_PATH)as_button.c
endif


#APP_UWB
ifeq ($(APP_UWB),yes)
CFLAGS += -DAPP_UWB
SRCS += $(SRCS_PATH)drivers/uwb/port/port_platform.c
SRCS += $(SRCS_PATH)drivers/uwb/deca_compat.c
# SRCS += $(SRCS_PATH)drivers/uwb/deca_interface.c
SRCS += $(SRCS_PATH)drivers/uwb/deca_rsl.c
SRCS += $(SRCS_PATH)drivers/uwb/dw3000/dw3000_device.c
SRCS += $(SRCS_PATH)drivers/uwb/probe_interface/deca_probe_interface.c
#QMATH - Used by DW driver
SRCS += $(SRCS_PATH)drivers/uwb/qmath/qmath.c
#ASAFE UWB Connector
SRCS += $(SRCS_PATH)/uwb/as_uwb.c
SRCS += $(SRCS_PATH)/uwb/as_uwbTimer.c
SRCS += $(SRCS_PATH)/uwb/as_ranging.c
SRCS += $(SRCS_PATH)/uwb/as_nbor.c
endif

# #APP_UWB
# ifeq ($(APP_UWB),yes)
# CFLAGS += -DAPP_UWB
# SRCS += $(SRCS_PATH)drivers/uwb_lite/as_dw3000.c
# endif




#CONFIG NFC PINS AS GPIO
ifeq ($(config_nfct_pins_as_gpios),yes)
CFLAGS += -DCONFIG_NFCT_PINS_AS_GPIOS
endif

#LOGIC DEBUG - USED IN UWB FOR TIMING ANALYSIS
ifeq ($(APP_LOGIC_DEBUG),yes)
CFLAGS += -DAPP_LOGIC_DEBUG
endif


#PROXIMITY APP SETTINGS
$(if $(AS_RTLS_ROLE),,$(error AS_RTLS_ROLE is not defined))
$(if $(AS_RTLS_MODE),,$(error AS_RTLS_MODE is not defined))
$(if $(AS_RTLS_DEFAULT_UPDATE_RATE_SECONDS),,$(error AS_RTLS_DEFAULT_UPDATE_RATE_SECONDS is not defined))
$(if $(AS_RTLS_POSITIONING_MODE),,$(error AS_RTLS_POSITIONING_MODE is not defined))

#ROLE
ifeq ($(AS_RTLS_ROLE),tag)
CFLAGS += -DAS_RTLS_ROLE_TAG
else ifeq ($(AS_RTLS_ROLE),anchor)
CFLAGS += -DAS_RTLS_ROLE_ANCHOR
endif

#MODE
ifeq ($(AS_RTLS_MODE),sr)
CFLAGS += -DAS_RTLS_MODE_SR
else ifeq ($(AS_RTLS_MODE),hr)
CFLAGS += -DAS_RTLS_MODE_HR
endif

#UPDATE RATE
CFLAGS += -DAS_RTLS_DEFAULT_UPDATE_RATE_SECONDS=$(AS_RTLS_DEFAULT_UPDATE_RATE_SECONDS)
#UPDATE RATE MOTION
CFLAGS += -DAS_RTLS_DEFAULT_UPDATE_RATE_MOTION_SECONDS=$(AS_RTLS_DEFAULT_UPDATE_RATE_MOTION_SECONDS)

#POSITIONING MODE
ifeq ($(AS_RTLS_POSITIONING_MODE),wirepas)
CFLAGS += -DAS_RTLS_POSITIONING_MODE_WIREPAS
else ifeq ($(AS_RTLS_POSITIONING_MODE),ble)
CFLAGS += -DAS_RTLS_POSITIONING_MODE_BLE
endif
