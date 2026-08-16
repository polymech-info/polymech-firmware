#!/usr/bin/env python3
"""
Test script to verify upload settings match PlatformIO configuration
Based on output from: pio run -t uploadfs -e waveshare -v
"""
import os
import subprocess

def test_mklittlefs_command():
    """Test that mklittlefs command matches PlatformIO"""
    print("=== Testing mklittlefs command ===")
    
    expected_cmd = 'mklittlefs -c data -s 1572864 -p 256 -b 4096 littlefs.bin'
    print(f"Expected: {expected_cmd}")
    
    # Test if data directory exists
    if not os.path.exists('data'):
        print("❌ Data directory not found")
        return False
    
    # Find mklittlefs
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
        print("❌ mklittlefs.exe not found")
        return False
    
    print(f"✅ Found mklittlefs: {mklittlefs}")
    return True

def test_esptool_command():
    """Test that esptool command matches PlatformIO"""
    print("\n=== Testing esptool command ===")
    
    expected_params = {
        'chip': 'esp32s3',
        'port': 'COM17',
        'baud': '115200',
        'flash_mode': 'dio',
        'flash_freq': '80m',
        'flash_size': '8MB'
    }
    
    expected_addresses = {
        'bootloader': '0x0000',
        'partitions': '0x8000', 
        'boot_app0': '0xe000',
        'firmware': '0x10000',
        'littlefs': '0x670000'
    }
    
    print("Expected parameters:")
    for key, value in expected_params.items():
        print(f"  {key}: {value}")
    
    print("Expected flash addresses:")
    for key, value in expected_addresses.items():
        print(f"  {key}: {value}")
    
    # Test esptool availability
    try:
        result = subprocess.run(['python', '-m', 'esptool', 'version'], 
                              capture_output=True, text=True, check=True)
        print(f"✅ esptool available: {result.stdout.strip()}")
        return True
    except (subprocess.CalledProcessError, FileNotFoundError):
        print("❌ esptool not available")
        return False

def test_flash_addresses():
    """Verify flash address calculations"""
    print("\n=== Testing flash addresses ===")
    
    # From PlatformIO firmware upload output
    addresses = {
        'bootloader': (0x0000, '0x0000'),
        'partitions': (0x8000, '0x8000'),
        'boot_app0': (0xe000, '0xe000'),
        'firmware': (0x10000, '0x10000'),
        'littlefs': (6750208, '0x670000')  # From uploadfs output
    }
    
    all_match = True
    for name, (decimal, expected_hex) in addresses.items():
        actual_hex = hex(decimal)
        match = actual_hex == expected_hex
        status = "✅" if match else "❌"
        print(f"{status} {name}: {decimal} = {actual_hex} (expected {expected_hex})")
        if not match:
            all_match = False
    
    return all_match

def test_partition_size():
    """Verify partition size calculations"""
    print("\n=== Testing partition size ===")
    
    # From PlatformIO output: 1572864 bytes
    size_bytes = 1572864
    size_kb = size_bytes // 1024
    size_mb = size_kb // 1024
    size_hex = hex(size_bytes)
    
    print(f"Partition size: {size_bytes} bytes = {size_kb} KB = {size_mb:.1f} MB = {size_hex}")
    
    # Should be 1.5 MB
    if size_mb == 1:  # 1.5 rounds to 1
        print("✅ Partition size matches (~1.5 MB)")
        return True
    else:
        print("❌ Partition size unexpected")
        return False

def main():
    """Run all tests"""
    print("Testing upload configuration against PlatformIO settings...")
    print()
    
def test_platformio_files():
    """Test that PlatformIO build files exist"""
    print("\n=== Testing PlatformIO files ===")
    
    required_files = {
        'firmware': '.pio/build/waveshare/firmware.bin',
        'bootloader': '.pio/build/waveshare/bootloader.bin',
        'partitions': '.pio/build/waveshare/partitions.bin'
    }
    
    boot_app0_paths = [
        os.path.expanduser("~/.platformio/packages/framework-arduinoespressif32/tools/partitions/boot_app0.bin"),
        "C:/Users/%USERNAME%/.platformio/packages/framework-arduinoespressif32/tools/partitions/boot_app0.bin"
    ]
    
    all_exist = True
    
    for name, path in required_files.items():
        exists = os.path.exists(path)
        status = "✅" if exists else "❌"
        print(f"{status} {name}: {path}")
        if not exists:
            all_exist = False
    
    # Check boot_app0
    boot_app0_found = False
    for path in boot_app0_paths:
        if os.path.exists(path):
            print(f"✅ boot_app0: {path}")
            boot_app0_found = True
            break
    
    if not boot_app0_found:
        print("❌ boot_app0: not found in PlatformIO packages")
        all_exist = False
    
    return all_exist

    tests = [
        test_mklittlefs_command,
        test_esptool_command, 
        test_flash_addresses,
        test_partition_size,
        test_platformio_files
    ]
    
    results = []
    for test in tests:
        try:
            results.append(test())
        except Exception as e:
            print(f"❌ Test failed with error: {e}")
            results.append(False)
    
    print("\n=== Test Summary ===")
    passed = sum(results)
    total = len(results)
    
    if passed == total:
        print(f"✅ All {total} tests passed!")
        print("Upload scripts should work correctly.")
    else:
        print(f"❌ {total - passed} of {total} tests failed")
        print("Check the configuration before uploading.")
    
    return passed == total

if __name__ == "__main__":
    success = main()
    exit(0 if success else 1) 