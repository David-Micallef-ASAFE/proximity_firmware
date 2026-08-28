#This is what internally (RedLore) is known as the Find it tag 3
#Minew MBT02 - Asset Tag SR - With Retrieval
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



