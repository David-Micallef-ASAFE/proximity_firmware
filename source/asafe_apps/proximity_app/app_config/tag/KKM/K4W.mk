#This is what internally (RedLore) is known as the Find it tag 3
#Minew MBT02 - Asset Tag SR - With Retrieval
# Shares 0xA20003 with the E9 by mistake, but deployed devices use it so it must stay.
# OTAP does not clash: the full area ID includes HW_VARIANT_ID, so K4W (nrf52833) = 0xA2000309
# and E9 (nrf52832) = 0xA2000303. Distinguish them in reporting via asafe_device_type.
# Changing this would require a bootloader updater OTAP on all deployed K4Ws.
app_specific_area_id=0xA20003
#A2 = ASAFE, 0003 = K4W

#ASAFE Device Type - This was brought over from RedLore.  Might be reviewed with ASAFE
asafe_device_type=33

#Accelerometer (We enable the I2C subsytem too)
APP_LIS3DH=yes
APP_I2C=yes
HAL_I2C=yes

#Button
HAL_BUTTON=yes
APP_BUTTON=yes

#LED
APP_LED=yes

#UWB
HAL_SPI=yes
APP_SPI=yes
APP_UWB=yes
HAL_GPIO=yes

#Wirepas Role - //Subnode Low Energy - see wms_settings.h
asafe_wirepas_role=0x02 

config_nfct_pins_as_gpios=yes


#Proximity App Settings
AS_RTLS_ROLE=tag
AS_RTLS_MODE=hr

#Positioning Mode (only relevant for SR) - wirepas or ble
AS_RTLS_POSITIONING_MODE=wirepas     

AS_RTLS_DEFAULT_UPDATE_RATE_SECONDS=86400
AS_RTLS_DEFAULT_UPDATE_RATE_MOTION_SECONDS=10

#Manufacturing
APP_MANUFACTURING=yes


