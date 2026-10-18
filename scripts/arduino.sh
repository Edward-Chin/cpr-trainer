#!/bin/bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
source "$ROOT/arduino.env"

# Command-line environment overrides are intentionally not used: arduino.env
# is the single source of settings for both Zed tasks and terminal commands.
if [[ -n "$ARDUINO_CLI" ]]; then
    CLI="$ARDUINO_CLI"
elif command -v arduino-cli >/dev/null 2>&1; then
    CLI="$(command -v arduino-cli)"
else
    CLI="/Applications/Arduino IDE.app/Contents/Resources/app/lib/backend/resources/arduino-cli"
fi
if [[ ! -x "$CLI" ]]; then
    echo "Arduino CLI not found. Set ARDUINO_CLI in arduino.env to its executable path." >&2
    exit 1
fi

need_board() {
    if [[ -z "$ARDUINO_FQBN" ]]; then
        echo "Set ARDUINO_FQBN in arduino.env to the exact board identifier first." >&2
        echo "Search with: bash scripts/arduino.sh cli board listall Uno" >&2
        exit 1
    fi
}

need_port() {
    if [[ -z "$ARDUINO_PORT" ]]; then
        echo "Set ARDUINO_PORT in arduino.env first. Run the connected-boards task." >&2
        exit 1
    fi
    if [[ "$ARDUINO_PORT" == /dev/* && ! -e "$ARDUINO_PORT" ]]; then
        echo "Port does not exist: $ARDUINO_PORT. Reconnect and list boards again." >&2
        exit 1
    fi
}

compile() {
    need_board
    local sketch_name
    sketch_name="$(basename "$ARDUINO_SKETCH")"
    if [[ ! -f "$ARDUINO_SKETCH/$sketch_name.ino" ]]; then
        echo "Missing sketch: $ARDUINO_SKETCH/$sketch_name.ino" >&2
        exit 1
    fi
    mkdir -p "$ROOT/.arduino"
    if ! mkdir "$ROOT/.arduino/build.lock" 2>/dev/null; then
        echo "Another build/upload may be running. Stop it before trying again." >&2
        echo "If it was force-terminated, remove the empty .arduino/build.lock directory." >&2
        exit 1
    fi
    trap 'rmdir "$ROOT/.arduino/build.lock"' EXIT
    # Use the known-working clean path for both build and upload. This trades
    # incremental-build speed for avoiding the reported cached-build stall.
    echo "Compiling $ARDUINO_SKETCH for $ARDUINO_FQBN (clean build)..."
    "$CLI" compile --clean --fqbn "$ARDUINO_FQBN" --warnings all \
        --build-path "$ROOT/.arduino/build" "$@" "$ARDUINO_SKETCH"
    echo "Compilation finished."
}

monitor() {
    need_board
    need_port
    echo "Serial monitor: $ARDUINO_PORT at $ARDUINO_BAUD baud. Ctrl+C stops it."
    echo "Stop this monitor before uploading or opening another monitor."
    "$CLI" monitor --fqbn "$ARDUINO_FQBN" --port "$ARDUINO_PORT" \
        --config "baudrate=$ARDUINO_BAUD" --timestamp
}

case "${1:-help}" in
    doctor)
        printf 'CLI: %s\nSketch: %s\nBoard: %s\nPort: %s\nBaud: %s\n' \
            "$CLI" "$ARDUINO_SKETCH" "${ARDUINO_FQBN:-NOT SET}" \
            "${ARDUINO_PORT:-NOT SET}" "$ARDUINO_BAUD"
        "$CLI" version
        "$CLI" core list
        ;;
    boards) "$CLI" board list ;;
    board-details)
        need_board
        "$CLI" board details --fqbn "$ARDUINO_FQBN"
        ;;
    build|rebuild) compile ;;
    upload)
        need_board
        need_port
        echo "Building, then uploading to $ARDUINO_PORT. Close any serial monitor first."
        compile
        echo "Uploading to $ARDUINO_PORT..."
        "$CLI" upload --fqbn "$ARDUINO_FQBN" --port "$ARDUINO_PORT" \
            --input-dir "$ROOT/.arduino/build" --verify --verbose "$ARDUINO_SKETCH"
        echo "Upload and verification finished."
        ;;
    monitor) monitor ;;
    monitor-log)
        need_board
        need_port
        mkdir -p "$ROOT/logs"
        LOG_PATH="$ROOT/logs/serial-$(date +%Y%m%d-%H%M%S)-$$.log"
        echo "Saving serial session to $LOG_PATH"
        monitor 2>&1 | tee "$LOG_PATH"
        ;;
    capture-distance)
        need_board
        need_port
        "$CLI" monitor --fqbn "$ARDUINO_FQBN" --port "$ARDUINO_PORT" \
            --config "baudrate=$ARDUINO_BAUD" --quiet | python3 "$ROOT/scripts/capture-distance.py"
        ;;
    cli)
        shift
        exec "$CLI" "$@"
        ;;
    help)
        echo "Usage: bash scripts/arduino.sh {doctor|boards|board-details|build|rebuild|upload|monitor|monitor-log|capture-distance|cli ...}"
        ;;
    *) echo "Unknown command: $1" >&2; exit 2 ;;
esac
