#!/bin/sh
# Jig programming helper
#
# Usage:
#   ./jig.sh [config_name] [app_setup] [{flash|reset} {all|1-8 [more channels...]}]
#
# Examples:
#   ./jig.sh flash all                  # uses [global] settings
#   ./jig.sh proximity flash 3          # uses [proximity], falling back to [global]
#   ./jig.sh proximity flash 1 4 7
#   ./jig.sh proximity reset all
#   ./jig.sh proximity app_setup flash all   # run app_setup once, then flash
#   ./jig.sh proximity app_setup             # run app_setup only
#
# Settings are read from jig.conf next to this script
# (override with: JIG_CONF=/path/to/file ./jig.sh ...).

SERIAL="261002294"
NUM_CHANNELS=8

SCRIPT_DIR=$(dirname "$0")
CONF_FILE="${JIG_CONF:-$SCRIPT_DIR/jig.conf}"

usage() {
    echo "Usage: $0 [config_name] [app_setup] [{flash|reset} {all|1-$NUM_CHANNELS [more channels...]}]"
    if [ -f "$CONF_FILE" ]; then
        echo "Configurations in $CONF_FILE:"
        list_configs | sed 's/^/  /'
    fi
    exit 1
}

# ---------------------------------------------------------------------------
# Config file parsing
# ---------------------------------------------------------------------------

# Print every [section] name in the config file
list_configs() {
    awk '{ sub(/\r$/, "") }
         /^[ \t]*\[.*\][ \t]*$/ {
             s = $0; gsub(/^[ \t]*\[[ \t]*|[ \t]*\][ \t]*$/, "", s); print s
         }' "$CONF_FILE"
}

# Return success if a [section] exists
config_exists() {
    list_configs | grep -qx "$1"
}

# get_setting KEY -> prints value from [$CONFIG], else from [global].
# Returns non-zero if the key is in neither.
get_setting() {
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

# Load a required setting into a shell variable, or abort
require_setting() {
    _val=$(get_setting "$1") || {
        echo "Setting '$1' not found in [$CONFIG] or [global] ($CONF_FILE)"
        exit 1
    }
    eval "$1=\$_val"
}

# ---------------------------------------------------------------------------
# Jig actions
# ---------------------------------------------------------------------------

# Switch the multiplexer to anchor N (1-based -> mux output 0-based)
select_channel() {
    py.exe "$MUX_CLIENT" OUT $(( $1 - 1 ))
}

flash_anchor() {
    echo "=== Programming JIG Anchor $1 ==="
    select_channel "$1" &&
    nrfutil.exe device recover --serial-number "$SERIAL" &&
    nrfutil.exe device program --serial-number "$SERIAL" --firmware "$FIRMWARE" &&
    nrfutil.exe device reset   --serial-number "$SERIAL"
}

reset_anchor() {
    echo "=== Resetting JIG Anchor $1 ==="
    select_channel "$1" &&
    nrfutil.exe device reset --serial-number "$SERIAL"
}

# Generate the special firmware (run once, before flashing)
app_setup() {
    echo "=== Running app_setup ==="
    python3 tools/app_setup.py \
        --hexfile "$FIRMWARE_APP_SETUP" \
        --device  "$DEVICE" \
        -pm 1 \
        -pr    "$PROVISION_RETRIES" \
        -pt    "$PROVISION_TIMEOUT" \
        -pauth "$PROVISION_KEY_AUTH" \
        -penc  "$PROVISION_KEY_ENC" \
        -pid   "$PROVISION_UID" \
        -r     "$NODE_ROLE"
}

# ---------------------------------------------------------------------------
# Argument handling
# ---------------------------------------------------------------------------

if [ ! -f "$CONF_FILE" ]; then
    echo "Config file not found: $CONF_FILE"
    exit 1
fi

# Config name is optional: if the first argument is an action, use [global]
case "$1" in
    flash|reset|app_setup) CONFIG="global" ;;
    "")                    usage ;;
    *)                     CONFIG="$1"; shift ;;
esac

if ! config_exists "$CONFIG"; then
    echo "Unknown configuration: $CONFIG"
    usage
fi

# Optional app_setup step
DO_APP_SETUP=0
if [ "$1" = "app_setup" ]; then
    DO_APP_SETUP=1
    shift
fi

# Optional flash/reset action (required unless app_setup was given)
ACTION=""
if [ $# -gt 0 ]; then
    ACTION="$1"
    shift
    case "$ACTION" in
        flash|reset) ;;
        *) echo "Unknown action: $ACTION"; usage ;;
    esac
    [ $# -lt 1 ] && { echo "No channels given for $ACTION"; usage; }
elif [ $DO_APP_SETUP -eq 0 ]; then
    usage
fi

# Load settings ([config] first, then [global])
if [ $DO_APP_SETUP -eq 1 ]; then
    for key in FIRMWARE_APP_SETUP DEVICE PROVISION_RETRIES PROVISION_TIMEOUT \
               PROVISION_KEY_AUTH PROVISION_KEY_ENC PROVISION_UID NODE_ROLE; do
        require_setting "$key"
    done
    if [ ! -f "$FIRMWARE_APP_SETUP" ]; then
        echo "app_setup firmware not found: $FIRMWARE_APP_SETUP"
        exit 1
    fi
fi
[ -n "$ACTION" ] && require_setting MUX_CLIENT
[ "$ACTION" = "flash" ] && require_setting FIRMWARE

# Build the channel list
if [ "$1" = "all" ]; then
    CHANNELS=$(seq 1 $NUM_CHANNELS)
else
    CHANNELS="$*"
fi

# Validate channels before touching any hardware
for ch in $CHANNELS; do
    case "$ch" in
        ''|*[!0-9]*) echo "Invalid channel: $ch"; usage ;;
    esac
    if [ "$ch" -lt 1 ] || [ "$ch" -gt $NUM_CHANNELS ]; then
        echo "Channel out of range (1-$NUM_CHANNELS): $ch"
        usage
    fi
done

echo "Configuration: [$CONFIG]"
if [ $DO_APP_SETUP -eq 1 ]; then
    echo "  FIRMWARE_APP_SETUP = $FIRMWARE_APP_SETUP"
    echo "  DEVICE             = $DEVICE"
    echo "  PROVISION_UID      = $PROVISION_UID"
    echo "  NODE_ROLE          = $NODE_ROLE"
fi
[ -n "$ACTION" ] && echo "  MUX_CLIENT         = $MUX_CLIENT"
[ "$ACTION" = "flash" ] && echo "  FIRMWARE           = $FIRMWARE"
echo

# app_setup runs once; abort before touching the jig if it fails
if [ $DO_APP_SETUP -eq 1 ]; then
    if ! app_setup; then
        echo "!!! app_setup FAILED - aborting"
        exit 1
    fi
    echo
fi

[ -z "$ACTION" ] && { echo "Done. app_setup OK"; exit 0; }

# Checked after app_setup, in case app_setup produces this file
if [ "$ACTION" = "flash" ] && [ ! -f "$FIRMWARE" ]; then
    echo "Firmware not found: $FIRMWARE"
    exit 1
fi

FAILED=""
for ch in $CHANNELS; do
    if ! ${ACTION}_anchor "$ch"; then
        echo "!!! Anchor $ch: $ACTION FAILED"
        FAILED="$FAILED $ch"
    fi
done

echo
if [ -n "$FAILED" ]; then
    echo "Done with errors. Failed anchors:$FAILED"
    exit 1
fi
echo "Done. All selected anchors: $ACTION OK"