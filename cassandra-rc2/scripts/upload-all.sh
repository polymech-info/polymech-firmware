#!/bin/bash

# ESP32 Firmware & LittleFS Upload Script
# Usage: ./scripts/upload-all.sh [--env ENV] [--port PORT] [--fs]

# Default values
ENV="waveshare"
PORT=""
UPLOAD_FS=true
FIRMWARE_ONLY=false
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
        -f|--firmware-only)
            FIRMWARE_ONLY=true
            UPLOAD_FS=false
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
    echo "ESP32 Complete Upload Script"
    echo ""
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -e, --env ENV         PlatformIO environment (default: waveshare)"
    echo "  -p, --port PORT       Serial port (default: auto-detect)"
    echo "  -f, --firmware-only   Upload firmware only (skip LittleFS)"
    echo "  -h, --help            Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0                              # Upload firmware + LittleFS (default)"
    echo "  $0 --firmware-only              # Upload firmware only"
    echo "  $0 --env waveshare-release      # Upload release build + LittleFS"
    echo "  $0 --port COM17                 # Upload both to specific port"
    echo "  $0 --env waveshare-release --port COM17  # All options"
    echo ""
    exit 0
fi

echo "=== ESP32 Complete Upload Tool ==="
echo "Environment: $ENV"
echo "Port: ${PORT:-auto-detect}"
if [ "$FIRMWARE_ONLY" = true ]; then
    echo "Upload: Firmware only"
else
    echo "Upload: Firmware + LittleFS (complete)"
fi
echo ""

# Build file paths
BUILD_DIR=".pio/build/$ENV"
FIRMWARE="$BUILD_DIR/firmware.bin"
BOOTLOADER="$BUILD_DIR/bootloader.bin"
PARTITIONS="$BUILD_DIR/partitions.bin"

# Check if build files exist
if [ ! -f "$FIRMWARE" ]; then
    echo "❌ Firmware not found: $FIRMWARE"
    echo "Run: npm run build (or pio run -e $ENV)"
    exit 1
fi

echo "✅ Found firmware files in $BUILD_DIR"

# Build command arguments
CMD_ARGS=(
    "scripts/upload_firmware.py"
    "--firmware" "$FIRMWARE"
)

# Add optional files if they exist
if [ -f "$BOOTLOADER" ]; then
    CMD_ARGS+=("--bootloader" "$BOOTLOADER")
fi

if [ -f "$PARTITIONS" ]; then
    CMD_ARGS+=("--partitions" "$PARTITIONS")
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
    echo "✅ Firmware upload completed successfully!"
    
    # Upload LittleFS if requested
    if [ "$UPLOAD_FS" = true ]; then
        echo ""
        echo "=== Uploading LittleFS ==="
        
        # Build LittleFS command arguments
        FS_CMD_ARGS=(
            "scripts/upload_littlefs.py"
            "--auto"
            "--env" "$ENV"
        )
        
        # Add port if specified
        if [ -n "$PORT" ]; then
            FS_CMD_ARGS+=("--port" "$PORT")
        fi
        
        echo "Command: python ${FS_CMD_ARGS[*]}"
        echo ""
        
        python "${FS_CMD_ARGS[@]}"
        
        if [ $? -eq 0 ]; then
            echo ""
            echo "✅ LittleFS upload completed successfully!"
            echo "🎉 All uploads completed!"
        else
            echo ""
            echo "❌ LittleFS upload failed!"
            exit 1
        fi
    fi
else
    echo ""
    echo "❌ Firmware upload failed!"
    exit 1
fi
