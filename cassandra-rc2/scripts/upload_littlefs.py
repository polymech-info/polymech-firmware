#!/usr/bin/env python3
import subprocess
import sys
import os
import argparse
from shutil import which
import datetime
import traceback

class Tee(object):
    """A file-like object that writes to multiple streams."""
    def __init__(self, *files):
        self.files = files
    def write(self, obj):
        for f in self.files:
            f.write(obj)
            f.flush()
    def flush(self):
        for f in self.files:
            f.flush()

def find_tool_in_bundle(tool_name):
    """Find a tool executable, checking the PyInstaller bundle root first."""
    if getattr(sys, 'frozen', False) and hasattr(sys, '_MEIPASS'):
        bundled_path = os.path.join(sys._MEIPASS, tool_name)
        if os.path.exists(bundled_path):
            return bundled_path
    
    script_path = os.path.join('scripts', tool_name)
    if os.path.exists(script_path): return script_path
        
    in_path = which(tool_name)
    if in_path: return in_path

    return None

def find_serial_port():
    """Auto-detect ESP32 serial port. Returns None if not found."""
    try:
        import serial.tools.list_ports
        ports = serial.tools.list_ports.comports()
        for port in ports:
            if 'USB' in port.description or 'Serial' in port.description:
                print(f"✅ Auto-detected serial port: {port.device}")
                return port.device
        print("🔶 Warning: Could not auto-detect serial port. Letting esptool try.")
    except (ImportError, Exception):
        print("🔶 Warning: pyserial not found or failed. Letting esptool try.")
    return None # Let esptool handle it

def build_littlefs(data_dir, output_file, size="1572864"):
    """Build LittleFS image from data directory using mklittlefs.exe."""
    mklittlefs_exe = find_tool_in_bundle('mklittlefs.exe')
    if not mklittlefs_exe:
        print("❌ mklittlefs.exe not found. Please place it in the 'scripts/' directory.")
        return False
    
    if not os.path.isdir(data_dir):
        print(f"❌ Source data directory '{data_dir}' not found.")
        return False

    cmd = [mklittlefs_exe, "-c", data_dir, "-s", size, "-p", "256", "-b", "4096", output_file]
    print(f"Building LittleFS image... (using {mklittlefs_exe})")
    try:
        result = subprocess.run(cmd, check=True, capture_output=True, text=True, encoding='utf-8')
        print(f"✅ LittleFS image created: {output_file}")
        return True
    except subprocess.CalledProcessError as e:
        print(f"❌ Failed to build LittleFS: {e}\n{e.stderr}")
        return False

def run_esptool(port, esptool_args, trace=False):
    """Finds and runs esptool.exe, adding port and trace flag if specified."""
    esptool_exe = find_tool_in_bundle('esptool.exe')
    if not esptool_exe:
        print("❌ esptool.exe not found. Please place it in the 'scripts/' directory.")
        return False
        
    cmd = [esptool_exe]
    if port:
        cmd.extend(["--port", port])
    if trace:
        cmd.append("--trace")
    cmd.extend(esptool_args)
    
    print(f"Running command: {' '.join(cmd)}")
    try:
        result = subprocess.run(cmd, check=True, capture_output=True, text=True, encoding='utf-8')
        if result.stdout: print(result.stdout)
        if result.stderr: print(result.stderr)
        return True
    except subprocess.CalledProcessError as e:
        print(f"❌ esptool command failed with exit code {e.returncode}:")
        print(f"   STDOUT: {e.stdout}")
        print(f"   STDERR: {e.stderr}")
        return False

def upload_firmware(port, bootloader, partitions, firmware, trace=False):
    """Upload firmware, bootloader, and partitions using esptool.exe."""
    args = [
        "--chip", "esp32s3", "--baud", "921600",
        "--before", "default_reset", "--after", "hard_reset", "write_flash",
        "-z", "--flash_mode", "dio", "--flash_freq", "80m", "--flash_size", "8MB",
    ]
    flash_files = []
    if bootloader: flash_files.extend(["0x0", bootloader])
    if partitions: flash_files.extend(["0x8000", partitions])
    if firmware: flash_files.extend(["0x10000", firmware])

    if not run_esptool(port, args + flash_files, trace=trace):
        return False
        
    print("✅ Firmware upload successful!")
    return True

def upload_littlefs(port, littlefs_path, address="6750208", trace=False):
    """Upload LittleFS partition using esptool.exe."""
    args = [
        "--chip", "esp32s3", "--baud", "921600",
        "--before", "default_reset", "--after", "hard_reset", "write_flash",
        "-z", "--flash_mode", "dio", "--flash_freq", "80m", "--flash_size", "8MB",
        address, littlefs_path
    ]
    if not run_esptool(port, args, trace=trace):
        return False
        
    print("✅ LittleFS upload successful!")
    return True

def get_binary_path(file_name):
    """Get path for a binary, checking relative to the executable first."""
    if os.path.exists(file_name): return file_name
    dist_path = os.path.join('dist', file_name)
    if os.path.exists(dist_path): return dist_path
    return None


def find_network_config():
    """Find network.json relative to the executable or script."""
    # If frozen (exe), base_path is the dir of the exe.
    base_path = "."
    if getattr(sys, 'frozen', False):
         base_path = os.path.dirname(sys.executable)
    
    # Priority 1: data/assets/network.json (User hint: assets in data)
    path_1 = os.path.join(base_path, 'data', 'assets', 'network.json')
    if os.path.exists(path_1): return path_1

    # Priority 2: data/network.json (Based on cp command in scripts)
    path_2 = os.path.join(base_path, 'data', 'network.json')
    if os.path.exists(path_2): return path_2

    # Priority 3: assets/network.json (Original instruction)
    path_3 = os.path.join(base_path, 'assets', 'network.json')
    if os.path.exists(path_3): return path_3
    
    return None

def get_target_host(config_path):
    """Read sta_local_ip from network.json."""
    import json
    try:
        with open(config_path, 'r', encoding='utf-8') as f:
            data = json.load(f)
            return data.get('sta_local_ip')
    except Exception as e:
        print(f"❌ Failed to read network config: {e}")
        return None

def run_pm_cli(args):
    """Run pm-fw-cli.exe with given arguments."""
    cli_name = "pm-fw-cli.exe"
    # Assume it's in the same directory as the uploader exe/script
    base_path = "."
    if getattr(sys, 'frozen', False):
         base_path = os.path.dirname(sys.executable)
         
    cli_path = os.path.join(base_path, cli_name)
    
    # Check if we are in dev mode (script) and it might be elsewhere? 
    # For now, strictly follow "same dir" or PATH
    if not os.path.exists(cli_path):
        found = which(cli_name)
        if found:
            cli_path = found
        else:
            print(f"❌ {cli_name} not found in {base_path} or PATH.")
            return False

    cmd = [cli_path] + args
    print(f"Running: {' '.join(cmd)}")
    try:
        # Capture output so we can print it (and thus log it via Tee)
        result = subprocess.run(cmd, check=True, capture_output=True, text=True, encoding='utf-8')
        if result.stdout: print(result.stdout)
        if result.stderr: print(result.stderr)
        return True
    except subprocess.CalledProcessError as e:
        print(f"❌ Command failed with exit code {e.returncode}")
        if e.stdout: print(f"STDOUT: {e.stdout}")
        if e.stderr: print(f"STDERR: {e.stderr}")
        return False

def main():
    # When running as a bundled exe, the CWD may not be the exe's directory.
    # Change CWD to the exe's directory to ensure relative paths work.
    if getattr(sys, 'frozen', False):
        os.chdir(os.path.dirname(sys.executable))

    parser = argparse.ArgumentParser(description='Upload firmware and/or LittleFS to ESP32.')
    parser.add_argument('--port', '-p', help='Specify serial port (overrides auto-detect)')
    parser.add_argument('--firmware-only', action='store_true', help='Only upload firmware')
    parser.add_argument('--littlefs-only', action='store_true', help='Only upload LittleFS')
    parser.add_argument('--trace', '-t', action='store_true', help='Enable trace-level output for esptool')
    parser.add_argument('--data-dir', '-d', default='data', help='Data directory for LittleFS')
    parser.add_argument('--address', '-a', default='6750208', help='LittleFS flash address')
    parser.add_argument('--size', '-s', default='1572864', help='LittleFS partition size')
    parser.add_argument('--image', '-i', help='Path to existing LittleFS image file')
    parser.add_argument('--no-restore', action='store_true', help='Disable the full Backup -> Update -> Restore -> Reset flow (which is on by default)')
    
    args = parser.parse_args()
    
    # --- 1. BACKUP (Default unless --no-restore is set) ---
    target_host_ip = None
    should_restore = not args.no_restore
    
    if should_restore:
        print("\n=== PHASE 1: BACKUP ===")
        net_conf = find_network_config()
        if not net_conf:
            print("❌ Could not find assets/network.json. Cannot determine target host.")
            return 1
        
        target_host_ip = get_target_host(net_conf)
        if not target_host_ip:
            print("❌ Could not read 'sta_local_ip' from network config.")
            return 1
            
        print(f"ℹ️ Target Host: {target_host_ip}")
        
        # pm-fw-cli.exe dump --targethost=http://<IP> --directory=./backup
        backup_args = ["dump", f"--targethost=http://{target_host_ip}", "--directory=./backup"]
        if not run_pm_cli(backup_args):
            print("🔶 Warning: Backup failed. Proceeding with update anyway...")
        else:
            print("✅ Backup completed successfully.")

    # --- 2. FIRMWARE/FS UPDATE ---
    print("\n=== PHASE 2: FIRMWARE UPDATE ===")
    port = args.port or find_serial_port()
    
    # Determine what to upload
    do_firmware = not args.littlefs_only
    do_littlefs = not args.firmware_only

    if do_firmware:
        print("--- Locating Firmware Files ---")
        firmware = get_binary_path('firmware.bin')
        if firmware:
            print(f"✅ Firmware found: {firmware}")
            bootloader = get_binary_path('bootloader.bin')
            partitions = get_binary_path('partitions.bin')
            if not upload_firmware(port, bootloader, partitions, firmware, trace=args.trace):
                return 1
        else:
            print("🔶 Warning: firmware.bin not found. Skipping firmware upload.")

    if do_littlefs:
        print("\n--- Starting LittleFS Upload ---")
        littlefs_image = args.image
        if not littlefs_image:
            littlefs_image = "littlefs.bin"
            if not build_littlefs(args.data_dir, littlefs_image, args.size):
                return 1
        
        if not upload_littlefs(port, littlefs_image, args.address, trace=args.trace):
            return 1

    # --- 3. RESTORE (Only if enabled) ---
    if should_restore and target_host_ip:
        print("\n=== PHASE 3: RESTORE ===")
        # pm-fw-cli.exe restore --directory=./backup --targethost=http://<IP>
        restore_args = ["restore", "--directory=./backup", f"--targethost=http://{target_host_ip}"]
        if not run_pm_cli(restore_args):
             print("🔶 Warning: Restore failed. Proceeding with reset anyway...")
        else:
             print("✅ Restore completed successfully.")

        # --- 4. RESET ---
        print("\n=== PHASE 4: DEVICE RESET ===")
        # pm-fw-cli.exe mb --fn 6 --reg 100 --value 1 --host <IP>
        # Note: mb command arguments from previously viewed mb.ts: 
        # handler(argv: { host: string; port: number; fn: number; reg: number; value?: number; ... })
        reset_args = ["mb", "--fn", "6", "--reg", "100", "--value", "1", "--host", target_host_ip]
        if run_pm_cli(reset_args):
            print("✅ Reset command sent.")
        else:
            print("❌ Failed to send reset command.")

    print("\n🎉 All operations completed!")
    return 0

if __name__ == "__main__":
    exit_code = 0
    

    # Determine log file path. Place it next to the executable if bundled.
    if getattr(sys, 'frozen', False):
        base_path = os.path.dirname(sys.executable)
    else:
        # When running as a script, place it in the current working directory.
        base_path = os.getcwd()
    log_filename = os.path.join(base_path, "cassandra-uploader.log")

    # Open log file and keep a reference to original stdout/stderr
    log_file = open(log_filename, 'w', encoding='utf-8')
    original_stdout = sys.stdout
    original_stderr = sys.stderr
    
    # Redirect stdout and stderr to tee object
    sys.stdout = Tee(original_stdout, log_file)
    sys.stderr = Tee(original_stderr, log_file)

    try:
        print(f"--- Log started at {datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')} ---")
        print(f"Log file: {log_filename}\n")
        
        # If main() returns None, it becomes 0.
        exit_code = main() or 0
    except Exception as e:
        print(f"❌ An unexpected error occurred: {e}")
        traceback.print_exc(file=sys.stderr)
        exit_code = 1
    finally:
        print(f"\n--- Log finished at {datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')} ---")
        
        # Restore original stdout/stderr
        sys.stdout = original_stdout
        sys.stderr = original_stderr
        log_file.close()

        # When running as a bundled executable, pause for user input.
        # This message will go to the original stdout, not the log file.
        if getattr(sys, 'frozen', False):
            print("\n---", file=original_stdout) # Use original stdout
            # input() uses the original stdin/stdout by default after redirection is restored
            input("Press Enter to close this window...")
        
        sys.exit(exit_code)
