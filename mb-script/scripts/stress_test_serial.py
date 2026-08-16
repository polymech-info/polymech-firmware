# scripts/stress_test_serial.py
import serial
import time
import sys
import serial.tools.list_ports
import argparse

# --- Configuration ---
DEFAULT_PORT = "COM12"
BAUD_RATE = 115200
RESPONSE_READ_TIMEOUT = 2 # Seconds to wait for response lines (reduced for faster testing)
CONNECT_DELAY = 2.0 # Seconds to wait after connecting
# 125cmds/sec - 8ms delay

def find_esp_port():
    """Tries to automatically find the ESP device port."""
    ports = serial.tools.list_ports.comports()
    for port, desc, hwid in sorted(ports):
        # Look for common ESP32 VID/PID or descriptions
        if "CP210x" in desc or "USB Serial Device" in desc or "CH340" in desc or "SER=Serial" in hwid or "VID:PID=10C4:EA60" in hwid:
            print(f"Found potential ESP device: {port} ({desc})")
            return port
    print(f"Could not automatically find ESP device, defaulting to {DEFAULT_PORT}")
    return DEFAULT_PORT

def send_receive(port, baud, command_to_send, read_timeout, connect_delay):
    """Connects, sends a single command, reads the response, and closes."""
    ser = None
    all_lines = []
    print("-" * 20)
    try:
        # print(f"Attempting to connect to {port} at {baud} baud...")
        ser = serial.Serial(port, baud, timeout=1)
        # print(f"Connected. Waiting {connect_delay}s for board...")
        time.sleep(connect_delay)
        ser.reset_input_buffer()

        print(f"Sending command: {command_to_send}")
        if not command_to_send.endswith('\\n'):
            command_to_send += '\\n'

        ser.write(command_to_send.encode('utf-8'))
        ser.flush()
        # print("Command sent. Waiting for response...")

        # Read response
        start_time = time.time()
        while time.time() - start_time < read_timeout:
            if ser.in_waiting > 0:
                try:
                    line = ser.readline().decode('utf-8', errors='ignore').strip()
                    if line:
                        print(f"Received: {line}")
                        all_lines.append(line)
                        # Reset start time if we get data, maybe? Or just fixed timeout?
                        # start_time = time.time() # Uncomment to reset timeout on receiving data
                except Exception as read_err:
                    print(f"Error reading line: {read_err}")
            else:
                # Only sleep if nothing is waiting
                time.sleep(0.02) # Short sleep to avoid busy-waiting

        if not all_lines:
            print("No response received within the timeout.")

        return "\\n".join(all_lines)

    except serial.SerialException as e:
        print(f"Serial Error: {e}")
        return None
    except Exception as e:
        print(f"An unexpected error occurred: {e}")
        return None
    finally:
        if ser and ser.is_open:
            # print("Closing serial port.")
            ser.close()
        print("-" * 20)


def run_malformed_tests(port, baud, read_timeout, connect_delay):
    """Runs a predefined sequence of malformed tests."""
    print("--- Running Malformed Command Tests ---")
    malformed_commands = [
        "<<1;2;64;list:1:0",         # Missing end bracket
        "1;2;64;list:1:0>>",         # Missing start bracket
        "<<1;2;64>>",                # Missing payload section
        "<<1;2;64;list:1:0",         # Missing final >
        "<<1;2;64;list:1:",          # Incomplete payload
        "<<abc;def;ghi;list:1:0>>",  # Non-numeric header parts
        "<<1;2;64;very_long_payload_string_that_might_exceed_buffers_if_not_handled_well_xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx>>", # Long payload
        "<<>>",                      # Empty
        "",                          # Totally empty string
        "Just some random text",     # Not matching format
        "<<1;2;64;list:1:0>><<1;2;64;list:1:0>>", # Two commands concatenated
    ]
    for cmd in malformed_commands:
        send_receive(port, baud, cmd, read_timeout, connect_delay)
        time.sleep(0.2) # Small delay between malformed tests
    print("--- Malformed Command Tests Finished ---")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Send serial commands to ESP device, with stress testing options.")
    parser.add_argument("command", nargs='?', default=None, help="The command string to send (e.g., '<<1;2;64;list:1:0>>'). Required unless --malformed is used.")
    parser.add_argument("-p", "--port", default=None, help=f"Serial port name. Defaults to auto-detect or {DEFAULT_PORT}.")
    parser.add_argument("-n", "--count", type=int, default=1, help="Number of times to send the command.")
    parser.add_argument("-d", "--delay", type=int, default=50, help="Delay in milliseconds between sending commands.")
    parser.add_argument("--malformed", action="store_true", help="Run a sequence of malformed command tests instead of sending the specified command.")
    parser.add_argument("--timeout", type=float, default=RESPONSE_READ_TIMEOUT, help="Timeout in seconds to wait for response lines.")
    parser.add_argument("--connect-delay", type=float, default=CONNECT_DELAY, help="Delay in seconds after connecting before sending.")


    args = parser.parse_args()

    if not args.malformed and args.command is None:
        parser.error("the following arguments are required: command (unless --malformed is specified)")

    # Determine port
    serial_port = args.port if args.port else find_esp_port()

    if args.malformed:
        run_malformed_tests(serial_port, BAUD_RATE, args.timeout, args.connect_delay)
    else:
        print(f"--- Sending command '{args.command}' {args.count} times with {args.delay}ms delay ---")
        for i in range(args.count):
            print(f"Sending command #{i+1}/{args.count}")
            send_receive(serial_port, BAUD_RATE, args.command, args.timeout, args.connect_delay)
            if args.count > 1 and i < args.count - 1:
                time.sleep(args.delay / 1000.0)
        print(f"--- Finished sending command {args.count} times ---")
