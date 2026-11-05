#!/bin/sh
nrfjprog.exe -f NRF52 --snr 801040825 --recover
nrfjprog.exe -f NRF52 --snr 801040825 --eraseall
nrfjprog.exe -f NRF52 --snr 801040825 --program build/asC3W/proximity_app/final_image_proximity_app_with_params.hex
nrfjprog.exe -f NRF52 --snr 801040825 --pinreset
nrfjprog.exe -f NRF52 --snr 801040825 -r