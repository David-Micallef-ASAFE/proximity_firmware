#!/bin/sh
# Build and provision-parameterise firmware for every device in createFirmware.conf
#
# Usage:
#   ./createFirmware.sh                 # build every [device] in the conf file
#   ./createFirmware.sh asK4W asC3W     # build only the listed devices
#   ./createFirmware.sh asK4W flash     # build one device, then flash it
#                                       # (flash needs exactly one device)
#   ./createFirmware.sh asK4W clean=no  # KEY=value overrides any conf setting
#
# Settings are read from createFirmware.conf next to this script
# (override with: FW_CONF=/path/to/file ./createFirmware.sh ...).

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
CONF_FILE="${FW_CONF:-$SCRIPT_DIR/createFirmware.conf}"

cd "$SCRIPT_DIR" || exit 1

usage() {
    echo "Usage: $0 [device ...]"
    echo "       $0 <device> flash"
    echo "       Add KEY=value (e.g. clean=no) to override a conf setting"
    if [ -f "$CONF_FILE" ]; then
        echo "Devices in $CONF_FILE:"
        list_devices | sed 's/^/  /'
    fi
    exit 1
}

# ---------------------------------------------------------------------------
# Config file parsing (same format as jig.conf)
# ---------------------------------------------------------------------------

# Print every [section] name in the config file
list_configs() {
    awk '{ sub(/\r$/, "") }
         /^[ \t]*\[.*\][ \t]*$/ {
             s = $0; gsub(/^[ \t]*\[[ \t]*|[ \t]*\][ \t]*$/, "", s); print s
         }' "$CONF_FILE"
}

# Every section except [global]
list_devices() {
    list_configs | grep -vx "global"
}

# get_setting KEY -> prints value from [$CONFIG], else from [global].
# Returns non-zero if the key is in neither.
get_setting() {
    # Command-line KEY=value overrides win over the conf file
    eval "_is_ovr=\${OVR_$1+x}"
    if [ -n "$_is_ovr" ]; then
        eval "printf '%s\n' \"\$OVR_$1\""
        return 0
    fi
    awk -v sec="$CONFIG" -v key="$1" '
        function trim(s) { gsub(/^[ \t]+|[ \t]+$/, "", s); return s }
        {
            sub(/\r$/, "")                       # tolerate Windows line endings
            line = trim($0)
            if (line == "" || line ~ /^[#;]/) next
            if (line ~ /^\[.*\]$/) { cur = trim(substr(line, 2, length(line) - 2)); next }
            eq = index(line, "="); if (!eq) next
            k = trim(substr(line, 1, eq - 1))
            v = trim(substr(line, eq + 1))
            if (v ~ /^".*"$/ || v ~ /^\047.*\047$/) v = substr(v, 2, length(v) - 2)
            if (k != key) next
            if (cur == sec)           { found = 1; sval = v }
            else if (cur == "global") { gfound = 1; gval = v }
        }
        END {
            if (found)  { print sval; exit 0 }
            if (gfound) { print gval; exit 0 }
            exit 1
        }' "$CONF_FILE"
}

# Load a required setting into a shell variable, or fail
require_setting() {
    _val=$(get_setting "$1") || {
        echo "Setting '$1' not found in [$CONFIG] or [global] ($CONF_FILE)"
        return 1
    }
    eval "$1=\$_val"
}

# Read VAR=value from a board config.mk (handles =, :=, ?=)
board_var() {
    awk -v key="$2" '
        { sub(/\r$/, "") }
        /^[ \t]*#/ { next }
        {
            line = $0
            if (match(line, /[:?]?=/) == 0) next
            k = substr(line, 1, RSTART - 1); gsub(/[ \t]/, "", k)
            if (k != key) next
            v = substr(line, RSTART + RLENGTH); gsub(/^[ \t]+|[ \t]+$/, "", v)
            print v; exit
        }' "board/$1/config.mk"
}

# Warn if APP_PRINTING=yes in the app's config.mk and ask the user to continue.
# Returns non-zero if the user declines.
check_app_printing() {
    app_cfg=$(find source -path "*/$1/config.mk" 2>/dev/null | head -n 1)
    if [ -z "$app_cfg" ]; then
        echo "Warning: config.mk for app '$1' not found under source/ - skipping APP_PRINTING check"
        return 0
    fi

    # Last uncommented APP_PRINTING assignment wins (handles =, :=, ?=)
    printing=$(awk '
        { sub(/\r$/, "") }
        /^[ \t]*APP_PRINTING[ \t]*[:?]?=/ {
            v = $0; sub(/^[^=]*=[ \t]*/, "", v); sub(/[ \t]*(#.*)?$/, "", v); val = v
        }
        END { print val }' "$app_cfg")

    [ "$printing" = "yes" ] || return 0

    echo "******************************************************************"
    echo "WARNING: APP_PRINTING=yes in $app_cfg"
    echo "         Debug printing will be enabled in the '$1' firmware."
    echo "******************************************************************"
    printf "Continue anyway? [y/N] "
    if ! read -r answer < /dev/tty; then
        echo
        echo "No terminal available to confirm - aborting"
        return 1
    fi
    case "$answer" in
        y|Y|yes|YES) return 0 ;;
        *)           return 1 ;;
    esac
}

# ---------------------------------------------------------------------------
# Build one device
# ---------------------------------------------------------------------------

build_device() {
    CONFIG="$1"
    echo "=================================================================="
    echo "=== Building [$CONFIG]"
    echo "=================================================================="

    if [ ! -f "board/$CONFIG/config.mk" ]; then
        echo "Unknown target_board: $CONFIG (no board/$CONFIG/config.mk)"
        return 1
    fi

    APP_NAME=$(get_setting APP_NAME) || APP_NAME=proximity_app
    CLEAN=$(get_setting CLEAN) || CLEAN=no

    for key in PROVISION_METHOD PROVISION_RETRIES PROVISION_TIMEOUT \
               PROVISION_KEY_AUTH PROVISION_KEY_ENC PROVISION_UID NODE_ROLE; do
        require_setting "$key" || return 1
    done

    # Device: explicit override, else MCU + MCU_SUB from the board config
    if ! DEVICE=$(get_setting DEVICE); then
        mcu=$(board_var "$CONFIG" MCU)
        mcu_sub=$(board_var "$CONFIG" MCU_SUB)
        if [ -z "$mcu" ]; then
            echo "MCU not found in board/$CONFIG/config.mk"
            return 1
        fi
        DEVICE="$mcu$mcu_sub"
    fi

    build_dir="build/$CONFIG/$APP_NAME"
    hex="$build_dir/final_image_${APP_NAME}.hex"
    hex_params="$build_dir/final_image_${APP_NAME}_with_params.hex"

    PROV_ARGS="-pm $PROVISION_METHOD -pr $PROVISION_RETRIES -pt $PROVISION_TIMEOUT \
-pauth $PROVISION_KEY_AUTH -penc $PROVISION_KEY_ENC -pid $PROVISION_UID -r $NODE_ROLE"

    echo "  APP_NAME  = $APP_NAME"
    echo "  DEVICE    = $DEVICE"
    echo "  NODE_ROLE = $NODE_ROLE"
    echo "  PROV_ARGS = $PROV_ARGS"
    echo

    if [ "$CLEAN" = "yes" ]; then
        make clean target_board="$CONFIG" app_name="$APP_NAME" || return 1
    fi

    make target_board="$CONFIG" app_name="$APP_NAME" || return 1

    if [ ! -f "$hex" ]; then
        echo "Build output not found: $hex"
        return 1
    fi

    rm -f "$hex_params"
    # PROV_ARGS is intentionally unquoted so it splits into separate arguments
    python3 tools/app_setup.py --hexfile "$hex" --device "$DEVICE" $PROV_ARGS || return 1

    if [ ! -f "$hex_params" ]; then
        echo "app_setup output not found: $hex_params"
        return 1
    fi
    echo "--> $hex_params"
}

# ---------------------------------------------------------------------------
# Packaging (archive + markdown report)
# ---------------------------------------------------------------------------

# Query make once for the current board/app; make_var then reads from it
load_make_vars() {
    MAKE_VARS=$(make --no-print-directory -s -f makefile -f "$PRINT_VARS_MK" \
        target_board="$CONFIG" app_name="$APP_NAME" __print_vars 2>/dev/null)
}

# Print a variable as resolved by make (call load_make_vars first)
make_var() {
    printf '%s\n' "$MAKE_VARS" | sed -n "s/^$1=//p"
}

# Copy a successfully built device's outputs into the staging folder and
# add its row to the report. Uses variables set by build_device.
package_device() {
    dest="$STAGE_DIR/$CONFIG"
    mkdir -p "$dest" || return 1
    cp "$hex_params" "$dest/" || return 1

    otaps=$(ls "$build_dir"/*.otap 2>/dev/null)
    if [ -z "$otaps" ]; then
        echo "No OTAP files found in $build_dir"
        return 1
    fi
    cp $otaps "$dest/" || return 1

    load_make_vars
    dev_type=$(make_var asafe_device_type)
    area_id=$(make_var app_specific_area_id)
    printing=$(make_var APP_PRINTING)
    version=$(sed -n 's/^app_version=//p' "$build_dir/version.txt" 2>/dev/null)

    printf '| %s | %s | %s | %s | %s | %s | %s | %s |\n' \
        "$CONFIG" "${dev_type:-?}" "${area_id:-?}" "${printing:-no}" \
        "$PROVISION_UID" "$NODE_ROLE" "${version:-?}" "$DEVICE" >> "$ROWS_FILE"
    echo "--> packaged into $CONFIG/"

    # Queue this device's app for the combined OTAP
    if [ "$(get_setting COMBINED_OTAP)" = "yes" ]; then
        app_area_id=$(make_var app_area_id)     # app_specific_area_id + HW_VARIANT_ID
        app_hex="$build_dir/$APP_NAME.hex"
        if [ -z "$version" ] || [ -z "$app_area_id" ] || [ ! -f "$app_hex" ]; then
            echo "Cannot add to combined OTAP (version='$version' app_area_id='$app_area_id' hex='$app_hex')"
            return 1
        fi
        echo "$version:$app_area_id:$app_hex" >> "$COMBINED_SPECS_FILE"
        printf '| %s | %s | %s | `%s` |\n' \
            "$CONFIG" "$version" "$app_area_id" "$app_hex" >> "$COMBINED_ROWS_FILE"
        echo "--> queued for combined OTAP ($version:$app_area_id)"
    fi
}

# Build out/combined_app.otap from the queued apps, move it into the archive
# root and record its size. Does nothing if no device has COMBINED_OTAP = yes.
create_combined_otap() {
    [ -s "$COMBINED_SPECS_FILE" ] || return 0

    CONFIG=global
    keys_file=$(get_setting BOOTLOADER_KEYS_FILE) || keys_file=./custom_bootloader_keys.ini
    if [ ! -f "$keys_file" ]; then
        echo "Bootloader keys file not found: $keys_file"
        return 1
    fi

    otap_out="$OUT_DIR/combined_app.otap"
    echo "=================================================================="
    echo "=== Creating combined OTAP"
    echo "=================================================================="
    # Specs are newline separated and contain no spaces: split unquoted
    python3 tools/genscratchpad.py --configfile="$keys_file" "$otap_out" \
        $(cat "$COMBINED_SPECS_FILE") || return 1

    COMBINED_SIZE=$(wc -c < "$otap_out" | tr -d ' ')
    cp "$otap_out" "$STAGE_DIR/" && rm -f "$otap_out" || return 1
    echo "--> combined_app.otap ($COMBINED_SIZE bytes)"
}

# Print one line per asafe_device_type / app_specific_area_id value that is
# shared by more than one device in this run (empty output = all unique)
find_duplicates() {
    awk -F'|' '
        function trim(s) { gsub(/^[ \t]+|[ \t]+$/, "", s); return s }
        {
            dev = trim($2); type = trim($3); area = trim($4)
            if (type != "?") { n["asafe_device_type " type]++;    d["asafe_device_type " type]    = d["asafe_device_type " type] ", " dev }
            if (area != "?") { n["app_specific_area_id " area]++; d["app_specific_area_id " area] = d["app_specific_area_id " area] ", " dev }
        }
        END {
            for (k in n) if (n[k] > 1) print k " is used by: " substr(d[k], 3)
        }' "$ROWS_FILE" | sort
}

write_report() {
    sha=$(git rev-parse --short HEAD 2>/dev/null)
    dirty=$(git status --porcelain 2>/dev/null | head -n 1)
    {
        echo "# Firmware build OUT_$DATETIME"
        echo
        echo "- Built: $BUILT_AT"
        echo "- Git commit: ${sha:-unknown}${dirty:+ (uncommitted changes present)}"
        echo "- Config: $(basename "$CONF_FILE")"
        echo "- Archive: OUT_$DATETIME.zip"
        echo
        if [ -n "$DUPLICATES" ]; then
            echo "## ERROR: duplicate device identifiers"
            echo
            echo "asafe_device_type and app_specific_area_id must be unique across all devices."
            echo "The following values are shared by more than one device:"
            echo
            printf '%s\n' "$DUPLICATES" | sed 's/^/- **/; s/ is used by:/** is used by:/'
            echo
        fi
        echo "| Device | asafe_device_type | app_specific_area_id | APP_PRINTING | Provisioning UID | Node role | App version | MCU |"
        echo "|---|---|---|---|---|---|---|---|"
        cat "$ROWS_FILE"
        if [ -n "$COMBINED_SIZE" ]; then
            echo
            echo "## Combined OTAP"
            echo
            echo "\`combined_app.otap\` (archive root) - **size: $COMBINED_SIZE bytes ($(( (COMBINED_SIZE + 1023) / 1024 )) KB)**"
            echo
            echo "| Device | App version | app_area_id | App hex |"
            echo "|---|---|---|---|"
            cat "$COMBINED_ROWS_FILE"
        fi
        if [ -n "$FAILED" ]; then
            echo
            echo "**Failed (not in archive):**$FAILED"
        fi
        if [ -n "$SKIPPED" ]; then
            echo
            echo "**Skipped (SKIP_BUILD = yes, not in archive):**$SKIPPED"
        fi
        echo
        echo "Each device folder in the archive contains \`final_image_<app>_with_params.hex\` and the \`*.otap\` files."
    } > "$REPORT_MD"
}

# Write manifest.json (same information as the markdown report) into the
# archive root. Built from the same row files used for the report.
write_manifest() {
    sha=$(git rev-parse --short HEAD 2>/dev/null)
    dirty=$(git status --porcelain 2>/dev/null | head -n 1)
    M_NAME="OUT_$DATETIME" M_BUILT="$BUILT_AT" M_SHA="$sha" M_DIRTY="$dirty" \
    M_CONF="$(basename "$CONF_FILE")" M_SIZE="$COMBINED_SIZE" \
    M_FAILED="$FAILED" M_SKIPPED="$SKIPPED" M_DUPS="$DUPLICATES" \
    python3 -c '
import json, os, sys

def rows(path):
    out = []
    with open(path) as f:
        for line in f:
            cells = [c.strip().strip("`") for c in line.strip().strip("|").split("|")]
            if len(cells) > 1:
                out.append(cells)
    return out

def num(v):
    return int(v) if v.isdigit() else v

e = os.environ
devices = [{
    "device": c[0], "asafe_device_type": num(c[1]), "app_specific_area_id": c[2],
    "app_printing": c[3] == "yes", "provisioning_uid": c[4], "node_role": num(c[5]),
    "app_version": c[6], "mcu": c[7],
    "files": sorted(os.listdir(os.path.join(sys.argv[3], c[0]))),
} for c in rows(sys.argv[1])]

manifest = {
    "name": e["M_NAME"],
    "built": e["M_BUILT"],
    "git_commit": e["M_SHA"] or None,
    "git_uncommitted_changes": bool(e["M_DIRTY"]),
    "config": e["M_CONF"],
    "archive": e["M_NAME"] + ".zip",
    "errors": {"duplicate_identifiers": [l for l in e["M_DUPS"].splitlines() if l]},
    "devices": devices,
    "combined_otap": None,
    "failed": e["M_FAILED"].split(),
    "skipped": e["M_SKIPPED"].split(),
}
if e["M_SIZE"]:
    manifest["combined_otap"] = {
        "file": "combined_app.otap",
        "size_bytes": int(e["M_SIZE"]),
        "devices": [{"device": c[0], "app_version": c[1], "app_area_id": c[2], "app_hex": c[3]}
                    for c in rows(sys.argv[2])],
    }

with open(os.path.join(sys.argv[3], "manifest.json"), "w") as f:
    json.dump(manifest, f, indent=2)
    f.write("\n")
' "$ROWS_FILE" "$COMBINED_ROWS_FILE" "$STAGE_DIR"
}

create_archive() {
    python3 -c '
import os, sys, zipfile
src, dst = sys.argv[1], sys.argv[2]
with zipfile.ZipFile(dst, "w", zipfile.ZIP_DEFLATED) as z:
    for root, dirs, files in os.walk(src):
        dirs.sort()
        for f in sorted(files):
            p = os.path.join(root, f)
            z.write(p, os.path.relpath(p, src))
' "$STAGE_DIR" "$ARCHIVE"
}

# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

if [ ! -f "$CONF_FILE" ]; then
    echo "Config file not found: $CONF_FILE"
    exit 1
fi

case "$1" in -h|--help) usage ;; esac

# Split arguments into devices and the optional "flash" action
ACTION=""
DEVICES=""
for arg in "$@"; do
    case "$arg" in
        flash)
            ACTION="flash" ;;
        *=*)
            # KEY=value overrides a conf setting (key is case-insensitive)
            key=$(printf '%s' "${arg%%=*}" | tr 'a-z' 'A-Z')
            case "$key" in
                ''|[0-9]*|*[!A-Z0-9_]*) echo "Invalid setting name: ${arg%%=*}"; usage ;;
            esac
            eval "OVR_$key=\${arg#*=}"
            echo "Override: $key = ${arg#*=}" ;;
        *)
            list_devices | grep -qx "$arg" || { echo "Device not in $CONF_FILE: $arg"; usage; }
            DEVICES="$DEVICES $arg" ;;
    esac
done

if [ "$ACTION" = "flash" ]; then
    set -- $DEVICES
    if [ $# -ne 1 ]; then
        echo "flash requires exactly one device (got $#), e.g. $0 asK4W flash"
        usage
    fi
    CONFIG="$1"
    require_setting FLASHING_SERIAL || exit 1
fi

# No devices named: build every device except those with SKIP_BUILD = yes
# (a device named explicitly on the command line is always built)
SKIPPED=""
if [ -z "$DEVICES" ]; then
    for d in $(list_devices); do
        CONFIG="$d"
        if [ "$(get_setting SKIP_BUILD)" = "yes" ]; then
            SKIPPED="$SKIPPED $d"
        else
            DEVICES="$DEVICES $d"
        fi
    done
    [ -n "$SKIPPED" ] && echo "Skipping (SKIP_BUILD = yes):$SKIPPED"
fi
[ -z "$DEVICES" ] && { echo "No devices to build in $CONF_FILE"; exit 1; }

# Check APP_PRINTING once per distinct app before building anything
CHECKED_APPS=" "
for d in $DEVICES; do
    CONFIG="$d"
    app=$(get_setting APP_NAME) || app=proximity_app
    case "$CHECKED_APPS" in *" $app "*) continue ;; esac
    CHECKED_APPS="$CHECKED_APPS$app "
    check_app_printing "$app" || { echo "Aborted by user."; exit 1; }
done

# Output locations
DATETIME=$(date +%Y%m%d_%H%M%S)
OUT_DIR="$SCRIPT_DIR/out"
ARCHIVE="$OUT_DIR/OUT_$DATETIME.zip"
REPORT_MD="$OUT_DIR/OUT_$DATETIME.md"
mkdir -p "$OUT_DIR" || exit 1
STAGE_DIR=$(mktemp -d) || exit 1
ROWS_FILE=$(mktemp) || exit 1
PRINT_VARS_MK=$(mktemp) || exit 1
COMBINED_SPECS_FILE=$(mktemp) || exit 1
COMBINED_ROWS_FILE=$(mktemp) || exit 1
COMBINED_SIZE=""
trap 'rm -rf "$STAGE_DIR" "$ROWS_FILE" "$PRINT_VARS_MK" "$COMBINED_SPECS_FILE" "$COMBINED_ROWS_FILE"' EXIT
printf '%s\n' '.PHONY: __print_vars' '__print_vars:' \
    '	@echo "asafe_device_type=$(asafe_device_type)"' \
    '	@echo "app_specific_area_id=$(app_specific_area_id)"' \
    '	@echo "app_area_id=$(app_area_id)"' \
    '	@echo "APP_PRINTING=$(APP_PRINTING)"' > "$PRINT_VARS_MK"

OK=""
FAILED=""
for d in $DEVICES; do
    if build_device "$d" && package_device; then
        OK="$OK $d"
    else
        echo "!!! [$d] FAILED"
        FAILED="$FAILED $d"
    fi
    echo
done

DUPLICATES=""
if [ -n "$OK" ]; then
    DUPLICATES=$(find_duplicates)

    # Remove previous outputs so only this run's files remain in out/
    if [ -d "$OUT_DIR" ] && [ "$(basename "$OUT_DIR")" = "out" ]; then
        find "$OUT_DIR" -mindepth 1 -delete
    fi

    if ! create_combined_otap; then
        echo "!!! Combined OTAP FAILED"
        FAILED="$FAILED (combined_otap)"
    fi
    echo

    BUILT_AT=$(date '+%Y-%m-%d %H:%M:%S')
    write_report
    if ! write_manifest; then
        echo "!!! Failed to write manifest.json"
        FAILED="$FAILED (manifest)"
    fi
    if create_archive; then
        echo "Archive: $ARCHIVE"
        echo "Report:  $REPORT_MD"
    else
        echo "!!! Failed to create archive $ARCHIVE"
        FAILED="$FAILED (archive)"
    fi
    echo
fi

echo "=================================================================="
if [ -n "$DUPLICATES" ]; then
    echo "******************************************************************"
    echo "WARNING: asafe_device_type / app_specific_area_id must be unique,"
    echo "         but multiple devices share the same value:"
    printf '%s\n' "$DUPLICATES" | sed 's/^/           /'
    echo "******************************************************************"
fi
[ -n "$OK" ]      && echo "Succeeded:$OK"
[ -n "$SKIPPED" ] && echo "Skipped:  $SKIPPED"
if [ -n "$FAILED" ]; then
    echo "Failed:   $FAILED"
    exit 1
fi
echo "Done. All devices built OK"

if [ "$ACTION" = "flash" ]; then
    # $DEVICES is a single device here; hex_params was set by build_device
    flash_hex="$hex_params"
    echo
    echo "=================================================================="
    echo "=== Flashing [$DEVICES] (serial $FLASHING_SERIAL)"
    echo "===   $flash_hex"
    echo "=================================================================="
    if nrfutil.exe device recover --serial-number "$FLASHING_SERIAL" &&
       nrfutil.exe device program --serial-number "$FLASHING_SERIAL" --firmware "$flash_hex" &&
       nrfutil.exe device reset   --serial-number "$FLASHING_SERIAL"; then
        echo "Done. Flash OK"
    else
        echo "!!! Flash FAILED"
        exit 1
    fi
fi
