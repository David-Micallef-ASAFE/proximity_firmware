##!/bin/sh
nrfjprog.exe -f NRF52 --snr 801040820 --recover
nrfjprog.exe -f NRF52 --snr 801040820 --eraseall
nrfjprog.exe -f NRF52 --snr 801040820 --program build/asMBT02/proximity_app/final_image_proximity_app_with_params.hex
nrfjprog.exe -f NRF52 --snr 801040820 --pinreset
nrfjprog.exe -f NRF52 --snr 801040820 -r