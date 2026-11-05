#!/bin/sh
make
python3 tools/app_setup.py --hexfile build/asK4W/proximity_app/final_image_proximity_app.hex --device nrf52833 -pm 1 -pr 5 -pt 60 -pauth e8570800393a587118860f155f1c8e22 -penc 25d2bdedb2a55682e0daf2b9db3bf0cf -pid 35B3F081 -r 2
./programK4W.sh