#!/bin/sh
nrfjprog.exe -f NRF52 --snr 1050314265 --recover
nrfjprog.exe -f NRF52 --snr 1050314265 --eraseall
nrfjprog.exe -f NRF52 --snr 1050314265 --program build/pca10040/provisioning_proxy/final_image_provisioning_proxy.hex
nrfjprog.exe -f NRF52 --snr 1050314265 --pinreset
nrfjprog.exe -f NRF52 --snr 1050314265 -r