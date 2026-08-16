#!/usr/bin/env python
import serial
import argparse
import sys
import time
import signal
import threading
import select
import os
import datetime
import serial.tools.list_ports
import re  # Import regex module

# ANSI escape codes for colors
COLORS = {
    'F': '\033[91m',  # Red for Fatal
    'E': '\033[91m',  # Red for Error
    'W': '\033[93m',  # Yellow for Warning
    'I': '\033[94m',  # Blue for Info/Notice
    'N': '\033[94m',  # Blue for Info/Notice
    'T': '\033[96m',  # Cyan for Trace
    'V': '\033[95m',  # Magenta for Verbose
    'RST': '\033[0m'  # Reset color
}

# Global flag to control threads
running = True

# Global log file handler
log_file = None

# Global variable to store the timestamp of the last printed message
last_print_time = None

# --- Configuration ---
# Try to find the port automatically, default to COM12 if not found
DEFAULT_PORT = "COM12"
BAUD_RATE = 115200      # Updated to 115200 baud to match firmware
STOP_CHARACTER = '\x03' # Ctrl+C

def ensure_tmp_dir():
    """Make sure the tmp directory exists"""
    if not os.path.exists('./tmp'):
        os.makedirs('./tmp')
        print("Created tmp directory for session logs")

def find_esp_port():
    ports = serial.tools.list_ports.comports()
    for port, desc, hwid in sorted(ports):
        # Look for common ESP32 VID/PID or descriptions
        # Added more specific PIDs commonly found on ESP32-S3 dev boards
        if ("CP210x" in desc or 
            "USB Serial Device" in desc or 
            "CH340" in desc or 
            "SER=Serial" in hwid or 
            "VID:PID=10C4:EA60" in hwid or # CP210x
            "VID:PID=303A:1001" in hwid): # ESP32-S3 built-in USB-CDC
            print(f"Found potential ESP device: {port} ({desc}) [{hwid}]")
            return port
    print(f"Could not automatically find ESP device.")
    return None # Return None instead of default port

def open_log_file(port, baudrate, retries):
    """Open a new log file with timestamp in filename"""
    ensure_tmp_dir()
    timestamp = datetime.datetime.now().strftime('%H_%M_%S')
    log_path = f'./tmp/session-{timestamp}.md'
    
    # Create and open the log file
    log = open(log_path, 'w', encoding='utf-8')
    
    # Write header
    log.write(f"# Serial Communication Session Log\n\n")
    log.write(f"Started at: {datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n\n")
    log.write(f"## Configuration\n")
    log.write(f"- Port: {port}\n")
    log.write(f"- Baudrate: {baudrate}\n")
    log.write(f"- Retries: {retries}\n\n")
    log.write(f"## Communication Log\n\n")
    log.flush()
    
    print(f"Logging session to: {log_path}")
    return log, log_path

def format_component_names(message):
    """Makes component names bold in the markdown log using regex"""
    # Regex to find words starting with a capital letter, possibly followed by more caps/lowercase
    # Looks for words after ": " or at the start of the line, common for component names
    pattern = r'(?<=:\s)([A-Z][a-zA-Z0-9_]+)|(^[A-Z][a-zA-Z0-9_]+)'
    
    def replace_match(match):
        # Group 1 is for matches after ": ", Group 2 is for matches at the start
        name = match.group(1) or match.group(2)
        # Avoid bolding common single uppercase letters used as prefixes (F, E, W, I, N, T, V)
        if name and len(name) > 1:
            return f'**{name}**'
        return name if name else '' # Return original if no suitable name found

    return re.sub(pattern, replace_match, message)

def log_message(message, is_user_input=False):
    """Log a message to the log file"""
    global log_file
    if log_file and not log_file.closed:
        timestamp = datetime.datetime.now().strftime('%H:%M:%S.%f')[:-3]
        
        if is_user_input:
            log_file.write(f"**[{timestamp}] USER INPUT >** `{message}`\n\n")
        else:
            # Format component names before writing
            formatted_message = format_component_names(message)
            # Remove 'DEVICE >' prefix and write formatted message
            log_file.write(f"**[{timestamp}]** {formatted_message}\n\n")
        log_file.flush()

def signal_handler(sig, frame):
    global running, log_file
    print("\nExiting serial monitor...")
    running = False
    
    # Close log file if open
    if log_file and not log_file.closed:
        log_file.write(f"\n## Session End\n\nEnded at: {datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
        log_file.flush()
        log_file.close()
        print(f"Log file closed.")
    
    # Allow time for threads to exit gracefully
    time.sleep(0.5)
    sys.exit(0)

def input_thread(ser):
    """Thread to read user input and send it to serial port"""
    global running
    print("Input mode enabled. Type commands and press Enter to send.")
    print("Enter 'exit' or 'quit' to exit the program.")
    
    try:
        while running:
            # Simple input approach that works on all platforms
            user_input = input()
            
            # Skip empty lines
            if not user_input.strip():
                continue
                
            if user_input.lower() in ('exit', 'quit'):
                print("Exit command received.")
                log_message(user_input, is_user_input=True)
                running = False
                break
            
            # Special commands
            if user_input.startswith('!'):
                log_message(user_input, is_user_input=True)
                handle_special_command(user_input, ser)
                continue
            
            # Log the user input
            log_message(user_input, is_user_input=True)
            
            # Add newline to input if not present
            if not user_input.endswith('\n'):
                user_input += '\n'
            
            # Send to serial port
            print(f"---- Sending: {repr(user_input)} ----")
            ser.write(user_input.encode('utf-8'))
            ser.flush()
    except Exception as e:
        print(f"Input thread error: {e}")
    finally:
        print("Input thread exiting")

def handle_special_command(cmd, ser):
    """Handle special commands that start with '!'"""
    # Strip the '!' prefix
    cmd = cmd[1:].strip()
    
    if cmd.startswith('send_hex '):
        # Format: !send_hex 01 02 03 FF
        try:
            hex_values = cmd[9:].split()
            bytes_to_send = bytes([int(x, 16) for x in hex_values])
            print(f"Sending hex bytes: {' '.join(hex_values)}")
            ser.write(bytes_to_send)
            ser.flush()
        except Exception as e:
            print(f"Error sending hex: {e}")
    
    elif cmd.startswith('baudrate '):
        # Format: !baudrate 115200
        try:
            new_baudrate = int(cmd[9:])
            print(f"Changing baudrate to {new_baudrate}")
            ser.baudrate = new_baudrate
        except Exception as e:
            print(f"Error changing baudrate: {e}")
    
    elif cmd == 'help':
        print("\nSpecial commands:")
        print("  !send_hex XX XX XX - Send hex bytes")
        print("  !baudrate XXXX - Change baudrate")
        print("  !help - Show this help\n")
    
    else:
        print(f"Unknown special command: {cmd}")
        print("Type !help for available commands")

def print_colored_output(line):
    """Prints the line to console, adding color to the log level prefix if found and prepending time delta."""
    global last_print_time
    current_time = datetime.datetime.now()
    time_delta_str = ""

    if last_print_time:
        delta = current_time - last_print_time
        # Calculate minutes, seconds, milliseconds
        total_seconds = delta.total_seconds()
        minutes = int(total_seconds // 60)
        seconds = int(total_seconds % 60)
        milliseconds = delta.microseconds // 1000
        # Format as mm:ss:fff
        time_delta_str = "{:02}:{:02}:{:03} ".format(minutes, seconds, milliseconds)

    if len(line) > 1 and line[1] == ':' and line[0] in COLORS:
        level_char = line[0]
        color_code = COLORS.get(level_char, '') # Get color for the level
        rest_of_line = line[1:]
        print(f"{time_delta_str}{color_code}{level_char}{COLORS['RST']}{rest_of_line}")
    else:
        # Print normally if no recognized log level prefix, but still include time delta
        print(f"{time_delta_str}{line}")

    last_print_time = current_time

def monitor_serial(port, baudrate, max_retries=3, initial_command=None, exit_after_command=False):
    global running, log_file
    retry_count = 0
    
    while retry_count < max_retries and running:
        try:
            print(f"Attempt {retry_count + 1}/{max_retries}: Opening serial port {port} at {baudrate} baud (8N1)...")
            ser = serial.Serial(
                port=port,
                baudrate=baudrate,
                bytesize=serial.EIGHTBITS,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE,
                timeout=0.1
            )
            
            print(f"Serial port {port} opened successfully. Monitoring for messages...")
            print("Type commands and press Enter to send them to the device.")
            print("Special commands start with '!' (Type !help for info)")
            print("Press Ctrl+C to exit")
            print("-" * 60)
            
            # Start input thread for sending commands
            input_handler = threading.Thread(target=input_thread, args=(ser,))
            input_handler.daemon = True
            input_handler.start()
            
            # Send initial command if provided
            if initial_command:
                print(f"Waiting 4 second before sending command...")
                time.sleep(4) # Ensure at least 1000ms delay
                print(f"Sending initial command: {initial_command}")
                if not initial_command.endswith('\n'):
                    initial_command += '\n'
                try:
                    ser.write(initial_command.encode('utf-8'))
                    ser.flush()
                    time.sleep(0.2) # Small delay after sending command
                except serial.SerialException as e:
                    print(f"Error sending initial command: {e}")
                except Exception as e:
                    print(f"An unexpected error occurred sending initial command: {e}")
            
            # Check if we should exit after sending the command
            if exit_after_command:
                print("Command sent. Waiting 3 seconds for response before exiting...")
                log_message("Exiting after sending initial command.")
                
                # Read for 3 seconds before exiting
                exit_start_time = time.time()
                while time.time() - exit_start_time < 3.0:
                    if ser.in_waiting > 0:
                        try:
                            line = ser.readline()
                            if line: 
                                try:
                                    decoded = line.decode('utf-8').rstrip()
                                    print(decoded) # Print response received during wait
                                    log_message(decoded)
                                except UnicodeDecodeError:
                                    hex_str = ' '.join([f'{b:02x}' for b in line])
                                    print(f"HEX: {hex_str}")
                                    log_message(f"HEX: {hex_str}")
                        except Exception as e:
                            print(f"Error reading during exit wait: {e}")
                            log_message(f"ERROR during exit wait: {str(e)}")
                    time.sleep(0.01) # Short sleep to prevent busy-waiting

                running = False # Signal threads to stop AFTER the wait/read period
                # The main loop below will be skipped, and finally block will close port
            
            # Main loop for reading from serial
            while running:
                if ser.in_waiting > 0:
                    try:
                        line = ser.readline()
                        if line:  # Only process non-empty lines
                            try:
                                # Try to decode as UTF-8 first
                                decoded = line.decode('utf-8').rstrip()
                                # Print with color formatting
                                print_colored_output(decoded)
                                # Log the response (without color codes)
                                log_message(decoded)
                            except UnicodeDecodeError:
                                # If that fails, print hex values
                                hex_str = ' '.join([f'{b:02x}' for b in line])
                                print(f"HEX: {hex_str}")
                                # Log the hex response
                                log_message(f"HEX: {hex_str}")
                    except Exception as e:
                        print(f"Error reading from serial: {e}")
                        log_message(f"ERROR: {str(e)}")
                else:
                    # Small sleep to prevent CPU hogging
                    time.sleep(0.01)
                    
        except serial.SerialException as e:
            retry_count += 1
            error_msg = f"Connection failed: {e}"
            print(error_msg)
            log_message(error_msg)
            
            if retry_count >= max_retries or not running:
                final_error = f"Error opening serial port after {retry_count} attempts: {e}"
                print(final_error)
                log_message(final_error)
                running = False
                sys.exit(1)
            else:
                retry_msg = f"Retrying in 2 seconds... (Attempt {retry_count + 1}/{max_retries})"
                print(retry_msg)
                log_message(retry_msg)
                time.sleep(2)  # Wait before retrying
        except KeyboardInterrupt:
            print("\nMonitor stopped by user")
            log_message("Monitor stopped by user")
            running = False
        finally:
            # This will only execute if we break out of the inner while loop
            if 'ser' in locals() and ser.is_open:
                ser.close()
                close_msg = f"Serial port {port} closed"
                print(close_msg)
                log_message(close_msg)

def main():
    global running, log_file, args
    
    # Register the signal handler for a clean exit with Ctrl+C
    signal.signal(signal.SIGINT, signal_handler)
    
    parser = argparse.ArgumentParser(description='Serial monitor with optional initial command.')
    parser.add_argument('port', nargs='?', default=None, help='Serial port name (e.g., COM3 or /dev/ttyUSB0). If omitted, attempts to find ESP device.')
    parser.add_argument('-c', '--command', type=str, default=None, help='Initial command to send upon connection.')
    parser.add_argument('--baudrate', '-b', type=int, default=BAUD_RATE, help='Baudrate (default: 115200)')
    parser.add_argument('--retries', '-r', type=int, default=3, help='Number of connection retry attempts (default: 3)')
    parser.add_argument('-x', '--exit-after-command', action='store_true', help='Exit immediately after sending the initial command specified with -c.')
    
    args = parser.parse_args()
    
    # Determine port
    serial_port = args.port
    if not serial_port:
        print("No port specified, attempting to find ESP device...")
        serial_port = find_esp_port()
        if not serial_port:
            print(f"Could not find ESP device. Please specify the port manually. Exiting.")
            sys.exit(1)
    else:
        print(f"Using specified port: {serial_port}")

    # Open log file
    log_file, log_path = open_log_file(serial_port, args.baudrate, args.retries)
    
    running = True
    try:
        monitor_serial(serial_port, args.baudrate, args.retries, args.command, args.exit_after_command)
    except Exception as e:
        print(f"Unexpected error: {e}")
    finally:
        running = False
        if log_file:
            log_file.close()
            print(f"Log saved to: {log_path}")
        print("Serial monitor stopped.")

if __name__ == "__main__":
    main() 