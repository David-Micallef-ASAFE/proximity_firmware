#!/bin/sh
nrfjprog.exe -f NRF52 --snr 801040825 --recover
nrfjprog.exe -f NRF52 --snr 801040825 --eraseall
nrfjprog.exe -f NRF52 --snr 801040825 --program build/asK4W/minimal_app/final_image_minimal_app_with_params_123456.hex
nrfjprog.exe -f NRF52 --snr 801040825 --pinreset
nrfjprog.exe -f NRF52 --snr 801040825 -r