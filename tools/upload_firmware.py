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

def upload_firmware(port, firmware_path, bootloader_path=None, partitions_path=None, boot_app0_path=None):
    """Upload firmware using esptool"""
    
    # ESP32-S3 flash addresses (from PlatformIO)
    bootloader_addr = "0x0000"
    partitions_addr = "0x8000" 
    boot_app0_addr = "0xe000"
    firmware_addr = "0x10000"
    
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
        "--flash_size", "8MB"
    ]
    
    # Add bootloader if provided
    if bootloader_path and os.path.exists(bootloader_path):
        cmd.extend([bootloader_addr, bootloader_path])
    
    # Add partitions if provided  
    if partitions_path and os.path.exists(partitions_path):
        cmd.extend([partitions_addr, partitions_path])
    
    # Add boot_app0 if provided
    if boot_app0_path and os.path.exists(boot_app0_path):
        cmd.extend([boot_app0_addr, boot_app0_path])
    
    # Add firmware
    cmd.extend([firmware_addr, firmware_path])
    
    print(f"Uploading firmware to {port}...")
    print(f"Command: {' '.join(cmd)}")
    
    try:
        result = subprocess.run(cmd, check=True, capture_output=True, text=True)
        print("Upload successful!")
        return True
    except subprocess.CalledProcessError as e:
        print(f"Upload failed: {e}")
        print(f"Error output: {e.stderr}")
        return False

def find_platformio_files(build_dir=".pio/build/waveshare"):
    """Find PlatformIO build files automatically"""
    files = {}
    
    # Firmware
    firmware_path = os.path.join(build_dir, "firmware.bin")
    if os.path.exists(firmware_path):
        files['firmware'] = firmware_path
    
    # Bootloader
    bootloader_path = os.path.join(build_dir, "bootloader.bin")
    if os.path.exists(bootloader_path):
        files['bootloader'] = bootloader_path
    
    # Partitions
    partitions_path = os.path.join(build_dir, "partitions.bin")
    if os.path.exists(partitions_path):
        files['partitions'] = partitions_path
    
    # boot_app0 (framework file)
    boot_app0_paths = [
        os.path.expanduser("~/.platformio/packages/framework-arduinoespressif32/tools/partitions/boot_app0.bin"),
        "C:/Users/%USERNAME%/.platformio/packages/framework-arduinoespressif32/tools/partitions/boot_app0.bin"
    ]
    
    for path in boot_app0_paths:
        if os.path.exists(path):
            files['boot_app0'] = path
            break
    
    return files

def main():
    parser = argparse.ArgumentParser(description='Upload firmware via esptool')
    parser.add_argument('--port', '-p', help='Serial port (auto-detect if not specified)')
    parser.add_argument('--firmware', '-f', help='Firmware binary path (auto-detect from PlatformIO if not specified)')
    parser.add_argument('--bootloader', '-b', help='Bootloader binary path (auto-detect from PlatformIO if not specified)')
    parser.add_argument('--partitions', '-pt', help='Partitions binary path (auto-detect from PlatformIO if not specified)')
    parser.add_argument('--boot-app0', '-ba', help='boot_app0 binary path (auto-detect from PlatformIO if not specified)')
    parser.add_argument('--auto', '-a', action='store_true', help='Auto-detect all files from PlatformIO build')
    parser.add_argument('--env', '-e', default='waveshare', help='PlatformIO environment (default: waveshare)')
    
    args = parser.parse_args()
    
    port = args.port or find_serial_port()
    
    # Auto-detect files if requested or if firmware not specified
    if args.auto or not args.firmware:
        build_dir = f".pio/build/{args.env}"
        print(f"Auto-detecting PlatformIO build files from {build_dir}...")
        pio_files = find_platformio_files(build_dir)
        
        firmware_path = args.firmware or pio_files.get('firmware')
        bootloader_path = args.bootloader or pio_files.get('bootloader')
        partitions_path = args.partitions or pio_files.get('partitions')
        boot_app0_path = args.boot_app0 or pio_files.get('boot_app0')
        
        print(f"Found files:")
        print(f"  Firmware:    {firmware_path}")
        print(f"  Bootloader:  {bootloader_path}")
        print(f"  Partitions:  {partitions_path}")
        print(f"  boot_app0:   {boot_app0_path}")
        print()
    else:
        firmware_path = args.firmware
        bootloader_path = args.bootloader
        partitions_path = args.partitions
        boot_app0_path = args.boot_app0
    
    if not firmware_path or not os.path.exists(firmware_path):
        print(f"Firmware file not found: {firmware_path}")
        print("Run: npm run build")
        sys.exit(1)
    
    success = upload_firmware(port, firmware_path, bootloader_path, partitions_path, boot_app0_path)
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main() 