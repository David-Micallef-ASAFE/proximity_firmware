#!/bin/sh
nrfjprog.exe -f NRF52 --snr 683888222 --recover
nrfjprog.exe -f NRF52 --snr 683888222 --eraseall
nrfjprog.exe -f NRF52 --snr 683888222 --program build/pca10056/dualmcu_app/final_image_dualmcu_app.hex
nrfjprog.exe -f NRF52 --snr 683888222 --pinreset
nrfjprog.exe -f NRF52 --snr 683888222 -r