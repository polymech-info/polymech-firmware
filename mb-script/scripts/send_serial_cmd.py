import serial
import time
import sys
import serial.tools.list_ports

# --- Configuration ---
# Try to find the port automatically, default to COM12 if not found
DEFAULT_PORT = "COM12" 
BAUD_RATE = 115200      # Updated to 115200 baud to match successful run
RESPONSE_READ_TIMEOUT = 5 # Seconds to wait for response lines

def find_esp_port():
    ports = serial.tools.list_ports.comports()
    for port, desc, hwid in sorted(ports):
        # Look for common ESP32 VID/PID or descriptions
        if "CP210x" in desc or "USB Serial Device" in desc or "CH340" in desc or "SER=Serial" in hwid or "VID:PID=10C4:EA60" in hwid:
            print(f"Found potential ESP device: {port} ({desc})")
            return port
    print(f"Could not automatically find ESP device, defaulting to {DEFAULT_PORT}")
    return DEFAULT_PORT

def send_receive(port, baud, command_to_send, read_timeout):
    """Connects, sends a command, reads the response, and closes."""
    ser = None # Initialize ser to None
    try:
        print(f"Attempting to connect to {port} at {baud} baud...")
        ser = serial.Serial(port, baud, timeout=1) # Initial timeout for connection
        print("Connected. Waiting for board to initialize...")
        time.sleep(2.5) # Give time for potential reset after connect
        ser.reset_input_buffer() # Clear any initial garbage data
        
        print(f"Sending command: {command_to_send}")
        if not command_to_send.endswith('\n'):
            command_to_send += '\n'
            
        ser.write(command_to_send.encode('utf-8'))
        ser.flush() # Wait for data to be written
        print("Command sent. Waiting for response...")

        # Read response
        lines = []
        start_time = time.time()
        while time.time() - start_time < read_timeout:
            if ser.in_waiting > 0:
                try:
                    line = ser.readline().decode('utf-8', errors='ignore').strip()
                    if line:
                        print(f"Received: {line}")
                        lines.append(line)
                except Exception as read_err:
                    print(f"Error reading line: {read_err}")
                    # Continue trying to read other lines
            else:
                time.sleep(0.05) # Avoid busy-waiting

        if not lines:
            print("No response received within the timeout.")
        
        return "\n".join(lines)

    except serial.SerialException as e:
        print(f"Serial Error: {e}")
        return None
    except Exception as e:
        print(f"An unexpected error occurred: {e}")
        return None
    finally:
        if ser and ser.is_open:
            print("Closing serial port.")
            ser.close()

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python send_serial_cmd.py "<command_to_send>" [port]")
        sys.exit(1)
        
    command = sys.argv[1]
    
    # Use provided port or find automatically
    serial_port = DEFAULT_PORT
    if len(sys.argv) > 2:
         serial_port = sys.argv[2]
         print(f"Using specified port: {serial_port}")
    else:
         serial_port = find_esp_port()
         
    send_receive(serial_port, BAUD_RATE, command, RESPONSE_READ_TIMEOUT) 