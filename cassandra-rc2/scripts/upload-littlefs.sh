#!/bin/bash

# ESP32 LittleFS Upload Script
# Usage: ./scripts/upload-littlefs.sh [--env ENV] [--port PORT] [--auto]

# Default values
ENV="waveshare"
PORT=""
AUTO=false
HELP=false

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -e|--env)
            ENV="$2"
            shift 2
            ;;
        -p|--port)
            PORT="$2"
            shift 2
            ;;
        -a|--auto)
            AUTO=true
            shift
            ;;
        -h|--help)
            HELP=true
            shift
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Show help
if [ "$HELP" = true ]; then
    echo "ESP32 LittleFS Upload Script"
    echo ""
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -e, --env ENV     PlatformIO environment (default: waveshare)"
    echo "  -p, --port PORT   Serial port (default: auto-detect)"
    echo "  -a, --auto        Use PlatformIO-built LittleFS image"
    echo "  -h, --help        Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0 --auto                       # Upload PlatformIO-built LittleFS"
    echo "  $0 --auto --env waveshare       # Upload from specific environment"
    echo "  $0 --auto --port COM17          # Upload to specific port"
    echo ""
    exit 0
fi

echo "=== ESP32 LittleFS Upload ==="
echo "Environment: $ENV"
echo "Port: ${PORT:-auto-detect}"
echo ""

# Build command arguments
CMD_ARGS=(
    "scripts/upload_littlefs.py"
    "--env" "$ENV"
)

# Add auto flag
if [ "$AUTO" = true ]; then
    CMD_ARGS+=("--auto")
fi

# Add port if specified
if [ -n "$PORT" ]; then
    CMD_ARGS+=("--port" "$PORT")
fi

# Execute upload
echo "Command: python ${CMD_ARGS[*]}"
echo ""

python "${CMD_ARGS[@]}"

if [ $? -eq 0 ]; then
    echo ""
    echo "✅ LittleFS upload completed successfully!"
else
    echo ""
    echo "❌ LittleFS upload failed!"
    exit 1
fi 