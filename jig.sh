#!/bin/sh
# Jig programming helper
#
# Usage:
#   ./jig.sh flash all        # recover + program + reset all 8 anchors
#   ./jig.sh flash 3          # flash anchor 3 only
#   ./jig.sh flash 1 4 7      # flash several anchors
#   ./jig.sh reset all        # reset all 8 anchors
#   ./jig.sh reset 4          # reset anchor 4 only

MUX_CLIENT="C:\Working\AS_DW3000_JIG\Multiplex_client\multiplex_client.py"
SERIAL="261002294"
FIRMWARE="build/asJIG/proximity_app/final_image_proximity_app.hex"
NUM_CHANNELS=8

usage() {
    echo "Usage: $0 {flash|reset} {all|1-$NUM_CHANNELS [more channels...]}"
    exit 1
}

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

[ $# -lt 2 ] && usage

ACTION="$1"
shift

case "$ACTION" in
    flash|reset) ;;
    *) echo "Unknown action: $ACTION"; usage ;;
esac

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