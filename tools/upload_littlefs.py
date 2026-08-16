#!/usr/bin/env python3
import subprocess
import sys
import os
import argparse

def find_serial_port():
    """Auto-detect ESP32 serial port"""
    try:
        import serial.tools.list_ports
        ports = serial.tools.list_ports.comports()
        for port in ports:
            if 'USB' in port.description or 'Serial' in port.description:
                return port.device
    except ImportError:
        pass
    return "COM17"  # fallback

def build_littlefs(data_dir, output_file, size="1572864"):
    """Build LittleFS image from data directory"""
    
    # Try to find mklittlefs tool
    mklittlefs_paths = [
        "mklittlefs.exe",
        os.path.expanduser("~/.platformio/packages/tool-mklittlefs/mklittlefs.exe"),
        "C:/Users/%USERNAME%/.platformio/packages/tool-mklittlefs/mklittlefs.exe"
    ]
    
    mklittlefs = None
    for path in mklittlefs_paths:
        if os.path.exists(path):
            mklittlefs = path
            break
    
    if not mklittlefs:
        print("mklittlefs.exe not found. Install via: pip install platformio")
        print("Or download from: https://github.com/earlephilhower/mklittlefs/releases")
        return False
    
    cmd = [mklittlefs, "-c", data_dir, "-s", size, "-p", "256", "-b", "4096", output_file]
    
    print(f"Building LittleFS image...")
    print(f"Command: {' '.join(cmd)}")
    
    try:
        subprocess.run(cmd, check=True)
        print(f"LittleFS image created: {output_file}")
        return True
    except subprocess.CalledProcessError as e:
        print(f"Failed to build LittleFS: {e}")
        return False

def upload_littlefs(port, littlefs_path, address="6750208"):
    """Upload LittleFS partition using esptool (matches PlatformIO exactly)"""
    
    cmd = [
        "python", "-m", "esptool",
        "--chip", "esp32s3", 
        "--port", port,
        "--baud", "115200",
        "--before", "default_reset",
        "--after", "hard_reset",
        "write_flash", "-z",
        "--flash_mode", "dio",
        "--flash_freq", "80m",
        "--flash_size", "8MB",
        address, littlefs_path
    ]
    
    print(f"Uploading LittleFS to {port} at address {address}...")
    print(f"Command: {' '.join(cmd)}")
    
    try:
        result = subprocess.run(cmd, check=True, capture_output=True, text=True)
        print("LittleFS upload successful!")
        return True
    except subprocess.CalledProcessError as e:
        print(f"Upload failed: {e}")
        print(f"Error output: {e.stderr}")
        return False

def find_platformio_littlefs(env="waveshare"):
    """Find PlatformIO-built LittleFS image"""
    pio_littlefs = f".pio/build/{env}/littlefs.bin"
    if os.path.exists(pio_littlefs):
        return pio_littlefs
    return None

def main():
    parser = argparse.ArgumentParser(description='Build and upload LittleFS partition')
    parser.add_argument('--port', '-p', help='Serial port (auto-detect if not specified)')
    parser.add_argument('--data-dir', '-d', default='data', help='Data directory (default: data)')
    parser.add_argument('--address', '-a', default='6750208', help='Flash address (default: 6750208)')
    parser.add_argument('--size', '-s', default='1572864', help='Partition size (default: 1572864)')
    parser.add_argument('--image', '-i', help='Use existing LittleFS image file')
    parser.add_argument('--env', '-e', default='waveshare', help='PlatformIO environment (default: waveshare)')
    parser.add_argument('--build-only', action='store_true', help='Only build image, do not upload')
    parser.add_argument('--auto', action='store_true', help='Auto-detect PlatformIO-built LittleFS image')
    
    args = parser.parse_args()
    
    port = args.port or find_serial_port()
    
    # Auto-detect PlatformIO-built LittleFS or use specified image
    if args.auto or (not args.image and not args.data_dir):
        pio_littlefs = find_platformio_littlefs(args.env)
        if pio_littlefs:
            littlefs_image = pio_littlefs
            print(f"✅ Using PlatformIO-built LittleFS: {littlefs_image}")
        else:
            print(f"❌ No PlatformIO LittleFS found for environment '{args.env}'")
            print(f"Run: npm run web:uploadfs (or pio run -t uploadfs -e {args.env})")
            sys.exit(1)
    elif args.image:
        littlefs_image = args.image
        if not os.path.exists(littlefs_image):
            print(f"❌ LittleFS image not found: {littlefs_image}")
            sys.exit(1)
    else:
        # Build from data directory
        littlefs_image = "littlefs.bin"
        if not os.path.exists(args.data_dir):
            print(f"❌ Data directory not found: {args.data_dir}")
            sys.exit(1)
        
        if not build_littlefs(args.data_dir, littlefs_image, args.size):
            sys.exit(1)
    
    if args.build_only:
        print(f"LittleFS image built: {littlefs_image}")
        sys.exit(0)
    
    # Upload LittleFS
    success = upload_littlefs(port, littlefs_image, args.address)
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main() 