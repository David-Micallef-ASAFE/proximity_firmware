#This is what internally (RedLore) is known as the Find it tag 3
#Minew MBT02 - Asset Tag SR - With Retrieval
app_specific_area_id=0xA21000
#A2 = ASAFE, 0100 = Anchor Jig

#ASAFE Device Type - This was brought over from RedLore.  Might be reviewed with ASAFE
asafe_device_type=36

#Button
HAL_BUTTON=yes
APP_BUTTON=yes

#LED
APP_LED=yes

#UWB
HAL_SPI=no
APP_SPI=no
APP_UWB=no

HAL_GPIO=yes

#Wirepas Role - //Subnode Low Energy - see wms_settings.h
asafe_wirepas_role=0x02 

config_nfct_pins_as_gpios=no


#Proximity App Settings
AS_RTLS_ROLE=anchor
AS_RTLS_MODE=hr

#Positioning Mode (only relevant for SR) - wirepas or ble
AS_RTLS_POSITIONING_MODE=wirepas     

AS_RTLS_DEFAULT_UPDATE_RATE_SECONDS=86400
AS_RTLS_DEFAULT_UPDATE_RATE_MOTION_SECONDS=86400

#Manufacturing
APP_MANUFACTURING=yes
APP_ASAFE_NFC=yes

#Logic Analysis for UWB
APP_LOGIC_DEBUG=no


