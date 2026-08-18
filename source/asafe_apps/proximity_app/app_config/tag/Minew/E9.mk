#This is what internally (RedLore) is known as the Find it tag 3
#Minew MBT02 - Asset Tag SR - With Retrieval
app_specific_area_id=0xA20003
#A2 = ASAFE, 0003 = E9

#ASAFE Device Type - This was brought over from RedLore.  Might be reviewed with ASAFE
asafe_device_type=32

#Accelerometer (We enable the I2C subsytem too)
APP_LIS3DH=yes
APP_I2C=yes
HAL_I2C=yes

#Buzzer
APP_BUZZER=no

#Button
HAL_BUTTON=yes
APP_BUTTON=yes

#LED
APP_LED=yes

#Wirepas Role - //Subnode Low Energy - see wms_settings.h
asafe_wirepas_role=0x02 