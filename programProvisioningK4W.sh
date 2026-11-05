#!/bin/sh
nrfjprog.exe -f NRF52 --snr 801040825 --recover
nrfjprog.exe -f NRF52 --snr 801040825 --eraseall
nrfjprog.exe -f NRF52 --snr 801040825 --program build/asK4W/local_provisioning/final_image_local_provisioning.hex
nrfjprog.exe -f NRF52 --snr 801040825 --pinreset
nrfjprog.exe -f NRF52 --snr 801040825 -r