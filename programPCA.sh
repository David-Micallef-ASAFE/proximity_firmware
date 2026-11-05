#!/bin/sh
nrfjprog.exe -f NRF52 --snr 1050385570 --recover
nrfjprog.exe -f NRF52 --snr 1050385570 --eraseall
nrfjprog.exe -f NRF52 --snr 1050385570 --program build/pca10040/dualmcu_app/final_image_dualmcu_app.hex
nrfjprog.exe -f NRF52 --snr 1050385570 --pinreset
nrfjprog.exe -f NRF52 --snr 1050385570 -r