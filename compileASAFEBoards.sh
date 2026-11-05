echo "Cleaning all boards"
make clean target_board=asMBT02
make clean target_board=asE9
make clean target_board=asC10
make clean target_board=asK4W
make clean target_board=asC3W

echo "Compiling for Minew MBT02 - ASAFE Asset Tag SR with Retrieval (Find it tag 3)"
make target_board=asMBT02
python3 tools/app_setup.py --hexfile build/asMBT02/proximity_app/final_image_proximity_app.hex --device nrf52832 -pm 1 -pr 5 -pt 60 -pauth e8570800393a587118860f155f1c8e22 -penc 25d2bdedb2a55682e0daf2b9db3bf0cf -pid 35B3F081 -r 2

echo "Compiling for Minew E9 - ASAFE Asset Tag SR (Puck Tag)"
make target_board=asE9
python3 tools/app_setup.py --hexfile build/asE9/proximity_app/final_image_proximity_app.hex --device nrf52832 -pm 1 -pr 5 -pt 60 -pauth e8570800393a587118860f155f1c8e22 -penc 25d2bdedb2a55682e0daf2b9db3bf0cf -pid 35B3F081 -r 2

echo "Compiling for Minew C10 - ASAFE People Tag SR (Badge Tag)"
make target_board=asC10
python3 tools/app_setup.py --hexfile build/asC10/proximity_app/final_image_proximity_app.hex --device nrf52832 -pm 1 -pr 5 -pt 60 -pauth e8570800393a587118860f155f1c8e22 -penc 25d2bdedb2a55682e0daf2b9db3bf0cf -pid 35B3F081 -r 2

echo "Compiling for KKM K4W - ASAFE Asset Tag HR (Puck Tag HR)"
make target_board=asK4W
python3 tools/app_setup.py --hexfile build/asK4W/proximity_app/final_image_proximity_app.hex --device nrf52833 -pm 1 -pr 5 -pt 60 -pauth e8570800393a587118860f155f1c8e22 -penc 25d2bdedb2a55682e0daf2b9db3bf0cf -pid 35B3F081 -r 2

echo "Compiling for KKM C3W - ASAFE People Tag HR (Badge Tag HR)"
make target_board=asC3W
python3 tools/app_setup.py --hexfile build/asC3W/proximity_app/final_image_proximity_app.hex --device nrf52833 -pm 1 -pr 5 -pt 60 -pauth e8570800393a587118860f155f1c8e22 -penc 25d2bdedb2a55682e0daf2b9db3bf0cf -pid 35B3F081 -r 2

