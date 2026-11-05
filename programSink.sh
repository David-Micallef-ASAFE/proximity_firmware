#!/bin/sh
nrfjprog.exe -f NRF52 --snr 801040824 --recover
nrfjprog.exe -f NRF52 --snr 801040824 --eraseall
nrfjprog.exe -f NRF52 --snr 801040824 --program build/pca10059/dualmcu_app/final_image_dualmcu_app.hex
nrfjprog.exe -f NRF52 --snr 801040824 --pinreset
nrfjprog.exe -f NRF52 --snr 801040824 -r