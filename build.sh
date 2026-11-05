#!/bin/bash
# Build, provision and flash the firmware for the target_board set in config.mk.
#
# Usage: ./build.sh [<app_name>] [<serial>] [clean] [make] [app_setup] [flash]
#   <app_name> - app to build/flash (any folder under source/), default: app_name in config.mk
#   <serial>   - programmer serial number (digits only), default: 261002294
#   clean      - make clean for the current target_board
#   make       - build, then inject the provisioning params (implies app_setup)
#   app_setup  - only inject the provisioning params into the existing build (tools/app_setup.py)
#   flash      - flash final_image_<app>_with_params.hex, or final_image_<app>.hex
#                if there is no up to date _with_params image
#
# Steps always run in the order clean -> make -> app_setup -> flash, whatever the argument order.
# Any failure stops the script before the next step.
#
# Overrides (environment variables):
#   target_board=asK4W ./build.sh make flash   - build for another board without editing config.mk

set -e
cd "$(dirname "$0")"

# Provisioning parameters passed to tools/app_setup.py
PROV_ARGS="-pm 1 -pr 5 -pt 60 -pauth e8570800393a587118860f155f1c8e22 -penc 25d2bdedb2a55682e0daf2b9db3bf0cf -pid 35B3F081 -r 2"
# PROV_ARGS="-pm 1 -pr 5 -pt 60 -pauth 774b3078e7b71421d9ebdc5fe40ffcfe   -penc 74316bf19e3b499ac618fe9abd690995 -pid 2A89C376 -r 2"

# Default programmer serial number
SERIAL=261002294

# Read a "name=value" setting from a makefile fragment, ignoring commented lines
read_setting() {
    sed -n -E "s/^[[:space:]]*$1[[:space:]]*[:?]?=[[:space:]]*([^[:space:]#]*).*/\1/p" "$2" | tail -n 1
}

# Same lookup as APP_POSSIBLE_FOLDER in makefile_common.mk
is_app() {
    compgen -G "source/*/$1/" > /dev/null || [ -d "source/$1" ]
}

if [ $# -eq 0 ]; then
    sed -n '2,17p' "$0" | sed 's/^# \{0,1\}//'
    exit 1
fi

do_clean=0; do_make=0; do_app_setup=0; do_flash=0
for arg in "$@"; do
    case "$arg" in
        clean) do_clean=1 ;;
        make)  do_make=1; do_app_setup=1 ;;
        app_setup) do_app_setup=1 ;;
        flash) do_flash=1 ;;
        *[!0-9]*|'')
            if is_app "$arg"; then
                app_name=$arg
            else
                echo "Error: unknown argument '$arg' (expected an app name under source/, a programmer serial number, clean, make, app_setup or flash)" >&2
                exit 1
            fi
            ;;
        *) SERIAL=$arg ;;
    esac
done

target_board=${target_board:-$(read_setting target_board config.mk)}
app_name=${app_name:-$(read_setting app_name config.mk)}

if [ -z "$target_board" ] || [ ! -f "board/$target_board/config.mk" ]; then
    echo "Error: target_board '$target_board' not found under board/" >&2
    exit 1
fi

mcu=$(read_setting MCU "board/$target_board/config.mk")
mcu_sub=$(read_setting MCU_SUB "board/$target_board/config.mk")
device="$mcu$mcu_sub"

build_dir="build/$target_board/$app_name"
hex="$build_dir/final_image_${app_name}.hex"
hex_params="$build_dir/final_image_${app_name}_with_params.hex"

echo "== Board: $target_board  App: $app_name  Device: $device =="

if [ $do_clean -eq 1 ]; then
    echo "== make clean =="
    make clean target_board="$target_board" app_name="$app_name"
fi

if [ $do_make -eq 1 ]; then
    echo "== make =="
    make target_board="$target_board" app_name="$app_name"
fi

if [ $do_app_setup -eq 1 ]; then
    if [ ! -f "$hex" ]; then
        echo "Error: $hex not found, run './build.sh $app_name make' first" >&2
        exit 1
    fi
    echo "== app_setup ($device) =="
    python3 tools/app_setup.py --hexfile "$hex" --device "$device" $PROV_ARGS
fi

if [ $do_flash -eq 1 ]; then
    # Prefer the provisioned image, unless it is older than the last build
    if [ -f "$hex_params" ] && ! [ "$hex" -nt "$hex_params" ]; then
        flash_hex=$hex_params
    elif [ -f "$hex" ]; then
        flash_hex=$hex
    else
        echo "Error: $hex not found, run './build.sh $app_name make' first" >&2
        exit 1
    fi
    echo "== flash $flash_hex (serial $SERIAL) =="
    nrfutil.exe device recover --serial-number "$SERIAL"
    nrfutil.exe device program --serial-number "$SERIAL" --firmware "$flash_hex"
    nrfutil.exe device reset --serial-number "$SERIAL"
fi

echo "== Done =="
