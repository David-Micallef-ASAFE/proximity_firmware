#!/bin/sh
nrfutil.exe device recover --serial-number 261002294
nrfutil.exe  device program --serial-number 261002294 --firmware build/asAnchorHR/proximity_app/final_image_proximity_app_with_params.hex
nrfutil.exe device reset --serial-number 261002294
