#!/bin/sh
nrfutil.exe device recover --serial-number 1050314265
nrfutil.exe  device program --serial-number 1050314265 --firmware build/pca10040/proximity_app/final_image_proximity_app_with_params.hex
nrfutil.exe device reset --serial-number 1050314265
