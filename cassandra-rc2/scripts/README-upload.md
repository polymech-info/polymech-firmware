# ESP32-S3 Upload Scripts (Windows)

Upload firmware and LittleFS without PlatformIO dependency.

## Prerequisites

Install required tools:
```bash
# Install esptool
pip install esptool pyserial

# Optional: Download mklittlefs.exe for LittleFS support
# From: https://github.com/earlephilhower/mklittlefs/releases
```

## Quick Start

### 1. Complete Upload (Default - Recommended)
```bash
# Build everything first
npm run build
npm run web:build-dist && npm run web:sync && npm run web:uploadfs

# Upload firmware + LittleFS (default behavior)
scripts/upload-all.sh            # Linux/macOS
scripts\upload-all.bat           # Windows
```

### 2. Firmware Only Upload
```bash
# Build firmware first (requires PlatformIO)
npm run build

# Upload firmware only (skip LittleFS)
scripts/upload-all.sh --firmware-only      # Linux/macOS
scripts\upload-all.bat --firmware-only     # Windows
```

### 3. Upload LittleFS Only  
```bash
# Upload LittleFS filesystem only
scripts/upload-littlefs.sh --auto    # Linux/macOS
scripts\upload-littlefs.bat --auto   # Windows
```

## Advanced Usage

### Complete Upload (Main Scripts)
```bash
# Upload everything (firmware + LittleFS) - DEFAULT
scripts/upload-all.sh

# Upload firmware only (skip LittleFS)
scripts/upload-all.sh --firmware-only

# Specify environment
scripts/upload-all.sh --env waveshare-release

# Specify port
scripts/upload-all.sh --port COM17

# All options
scripts/upload-all.sh --env waveshare-release --port COM17

# Windows equivalent
scripts\upload-all.bat --env waveshare-release --port COM17
```

### Upload Firmware (Python - Advanced)
```bash
# Auto-detect all files (recommended)
python scripts/upload_firmware.py --auto

# With specific environment
python scripts/upload_firmware.py --auto --env waveshare-release

# With specific port
python scripts/upload_firmware.py --port COM17 --auto

# Manual file specification
python scripts/upload_firmware.py \
  --firmware .pio/build/waveshare/firmware.bin \
  --bootloader .pio/build/waveshare/bootloader.bin \
  --partitions .pio/build/waveshare/partitions.bin \
  --boot-app0 ~/.platformio/packages/framework-arduinoespressif32/tools/partitions/boot_app0.bin
```

### Upload LittleFS (Standalone Scripts)
```bash
# Upload PlatformIO-built LittleFS (recommended)
scripts/upload-littlefs.sh --auto
scripts\upload-littlefs.bat --auto

# Specify environment
scripts/upload-littlefs.sh --auto --env waveshare-release

# Specify port
scripts/upload-littlefs.sh --auto --port COM17
```

### Upload LittleFS (Python - Advanced)
```bash
# Auto-detect PlatformIO-built LittleFS
python scripts/upload_littlefs.py --auto

# Build and upload from data/ directory
python scripts/upload_littlefs.py --data-dir data

# Upload existing image
python scripts/upload_littlefs.py --image littlefs.bin

# Build only (no upload)
python scripts/upload_littlefs.py --data-dir data --build-only

# Custom flash address and size
python scripts/upload_littlefs.py \
  --data-dir data \
  --address 6750208 \
  --size 1572864
```

## Alternative Options

### Option 1: ESP32 Flash Download Tools (GUI)
- Download: https://www.espressif.com/en/support/download/other-tools
- GUI-based flashing tool from Espressif
- No Python required

### Option 2: espflash (Rust-based)
```bash
# Install via cargo
cargo install espflash

# Upload firmware
espflash flash --chip esp32s3 firmware.bin

# Upload filesystem
espflash write-bin 0x670000 littlefs.bin
```

### Option 3: Custom esptool commands
```bash
# Erase flash
python -m esptool --chip esp32s3 --port COM17 erase_flash

# Upload complete firmware (matches PlatformIO)
python -m esptool --chip esp32s3 --port COM17 --baud 115200 \
  write_flash -z --flash_mode dio --flash_freq 80m --flash_size 8MB \
  0x0000 bootloader.bin \
  0x8000 partitions.bin \
  0xe000 boot_app0.bin \
  0x10000 firmware.bin

# Upload LittleFS  
python -m esptool --chip esp32s3 --port COM17 --baud 115200 \
  write_flash -z --flash_mode dio --flash_freq 80m --flash_size 8MB \
  0x670000 littlefs.bin
```

## Flash Memory Map (ESP32-S3)

| Address   | Size      | Content        |
|-----------|-----------|----------------|
| 0x0000    | 0x8000    | Bootloader     |
| 0x8000    | 0x6000    | Partition Table|
| 0xe000    | 0x2000    | boot_app0      |
| 0x10000   | 0x660000  | Firmware       |
| 0x670000  | 0x180000  | LittleFS       |

## Troubleshooting

### Port Detection Issues
```bash
# List available ports
python -c "import serial.tools.list_ports; [print(f'{p.device}: {p.description}') for p in serial.tools.list_ports.comports()]"
```

### Permission Issues
- Run as Administrator if needed
- Close other serial monitors (Arduino IDE, PlatformIO)

### mklittlefs.exe Not Found
- Install PlatformIO: `pip install platformio` 
- Or download manually from GitHub releases
- Place in PATH or scripts/ directory 