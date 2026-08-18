# You can add more sources here and libraries if needed
SRCS +=
INCLUDES +=
LIBS +=
LDFLAGS +=
LDLIBS +=

#Includes
INCLUDES += -I$(SRCS_PATH)inc
INCLUDES += -I$(SRCS_PATH)drivers/lis3dh
INCLUDES += -I$(SRCS_PATH)drivers/i2c
INCLUDES += -I$(SRCS_PATH)drivers/led
INCLUDES += -I$(SRCS_PATH)drivers/buzzer


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



