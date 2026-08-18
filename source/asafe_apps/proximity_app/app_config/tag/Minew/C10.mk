#This is what internally (RedLore) is known as the Find it tag 3
#C10 - Badge Tag SR
app_specific_area_id=0xA20002
#A2 = ASAFE, 0002 = C10

#ASAFE Device Type - This was brought over from RedLore.  Might be reviewed with ASAFE
asafe_device_type=31

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