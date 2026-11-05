#!/bin/sh
echo "Programming JIG Anchor 1"
py.exe "C:\Working\AS_DW3000_JIG\Multiplex_client\multiplex_client.py" OUT 0
nrfutil.exe device recover --serial-number 261002294
nrfutil.exe  device program --serial-number 261002294 --firmware build/asJIG/proximity_app/final_image_proximity_app.hex
nrfutil.exe device reset --serial-number 261002294
echo "Programming JIG Anchor 2"
py.exe "C:\Working\AS_DW3000_JIG\Multiplex_client\multiplex_client.py" OUT 1
nrfutil.exe device recover --serial-number 261002294
nrfutil.exe  device program --serial-number 261002294 --firmware build/asJIG/proximity_app/final_image_proximity_app.hex
nrfutil.exe device reset --serial-number 261002294
echo "Programming JIG Anchor 3"
py.exe "C:\Working\AS_DW3000_JIG\Multiplex_client\multiplex_client.py" OUT 2
nrfutil.exe device recover --serial-number 261002294
nrfutil.exe  device program --serial-number 261002294 --firmware build/asJIG/proximity_app/final_image_proximity_app.hex
nrfutil.exe device reset --serial-number 261002294
echo "Programming JIG Anchor 4"
py.exe "C:\Working\AS_DW3000_JIG\Multiplex_client\multiplex_client.py" OUT 3
nrfutil.exe device recover --serial-number 261002294
nrfutil.exe  device program --serial-number 261002294 --firmware build/asJIG/proximity_app/final_image_proximity_app.hex
nrfutil.exe device reset --serial-number 261002294
echo "Programming JIG Anchor 5"
py.exe "C:\Working\AS_DW3000_JIG\Multiplex_client\multiplex_client.py" OUT 4
nrfutil.exe device recover --serial-number 261002294
nrfutil.exe  device program --serial-number 261002294 --firmware build/asJIG/proximity_app/final_image_proximity_app.hex
nrfutil.exe device reset --serial-number 261002294
echo "Programming JIG Anchor 6"
py.exe "C:\Working\AS_DW3000_JIG\Multiplex_client\multiplex_client.py" OUT 5
nrfutil.exe device recover --serial-number 261002294
nrfutil.exe  device program --serial-number 261002294 --firmware build/asJIG/proximity_app/final_image_proximity_app.hex
nrfutil.exe device reset --serial-number 261002294
echo "Programming JIG Anchor 7"
py.exe "C:\Working\AS_DW3000_JIG\Multiplex_client\multiplex_client.py" OUT 6
nrfutil.exe device recover --serial-number 261002294
nrfutil.exe  device program --serial-number 261002294 --firmware build/asJIG/proximity_app/final_image_proximity_app.hex
nrfutil.exe device reset --serial-number 261002294
echo "Programming JIG Anchor 8"
py.exe "C:\Working\AS_DW3000_JIG\Multiplex_client\multiplex_client.py" OUT 7
nrfutil.exe device recover --serial-number 261002294
nrfutil.exe  device program --serial-number 261002294 --firmware build/asJIG/proximity_app/final_image_proximity_app.hex
nrfutil.exe device reset --serial-number 261002294