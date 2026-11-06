#!/bin/sh
nrfutil.exe device recover --serial-number 1050385570
nrfutil.exe  device program --serial-number 1050385570 --firmware build/pca10040/local_provisioning/final_image_local_provisioning.hex
nrfutil.exe device reset --serial-number 1050385570
