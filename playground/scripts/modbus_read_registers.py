import argparse
import json
import os
import logging
import time
from pymodbus.client import ModbusTcpClient
from pymodbus.exceptions import ConnectionException
from pymodbus.pdu import ExceptionResponse

# --- Configuration ---
MODBUS_PORT = 502
# REGISTER_ADDRESS = 0   # Removed default, now comes from args
# REGISTER_COUNT = 10    # Removed default, now comes from args
OUTPUT_DIR = "tmp"
OUTPUT_FILE = os.path.join(OUTPUT_DIR, "mbr-test.json") # Different output file
LOG_LEVEL = logging.INFO

# --- Modbus Exception Code Mapping ---
MODBUS_EXCEPTIONS = {
    1: "Illegal Function",
    2: "Illegal Data Address",
    3: "Illegal Data Value",
    4: "Slave Device Failure",
    5: "Acknowledge",
    6: "Slave Device Busy",
    7: "Negative Acknowledge",
    8: "Memory Parity Error",
    10: "Gateway Path Unavailable",
    11: "Gateway Target Device Failed to Respond",
}

# --- Setup Logging ---
logging.basicConfig(level=LOG_LEVEL, format='%(asctime)s - %(levelname)s - %(message)s')

# --- Argument Parsing ---
parser = argparse.ArgumentParser(description='Connect to Modbus TCP server and read holding registers.')
parser.add_argument('--ip-address', type=str, default='192.168.1.250', \
                    help='IP address of the Modbus TCP server (ESP32), defaults to 192.168.1.250')
parser.add_argument('--address', type=int, required=True, \
                    help='Starting holding register address to read.')
parser.add_argument('--count', type=int, default=1, \
                    help='Number of holding registers to read (default: 1).')
args = parser.parse_args()

# Use arguments for address and count
REGISTER_ADDRESS = args.address
REGISTER_COUNT = args.count

# --- Main Script ---
client = ModbusTcpClient(args.ip_address, port=MODBUS_PORT)
connection_success = False
results = None
read_success = False

# Create output directory
try:
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    logging.info(f"Ensured output directory exists: {OUTPUT_DIR}")
except OSError as e:
    logging.error(f"Failed to create output directory {OUTPUT_DIR}: {e}")
    exit(1)

try:
    logging.info(f"Attempting to connect to Modbus TCP server at {args.ip_address}:{MODBUS_PORT}...")
    connection_success = client.connect()

    if connection_success:
        logging.info("Connection successful.")

        logging.info(f"Reading {REGISTER_COUNT} holding registers starting from address {REGISTER_ADDRESS}...")
        try:
            # Read Holding Registers (Function Code 03)
            response = client.read_holding_registers(address=REGISTER_ADDRESS, count=REGISTER_COUNT)

            if not response.isError():
                # Access the list of register values
                register_values = response.registers
                # Ensure we have the expected number of registers
                if len(register_values) >= REGISTER_COUNT:
                    register_values = register_values[:REGISTER_COUNT]
                    logging.info(f"Successfully read {len(register_values)} register values.")
                    results = register_values
                    read_success = True

                    # Print the results to the console
                    print("\n--- Read Holding Register Values ---")
                    for i, value in enumerate(register_values):
                        print(f"Register {REGISTER_ADDRESS + i}: {value}")
                    print("----------------------------------\n")
                else:
                    logging.error(f"Read failed: Expected {REGISTER_COUNT} registers, but received {len(register_values)}.")
                    logging.debug(f"Response: {response}")

            else:
                # Handle Modbus logical errors
                error_code = getattr(response, 'exception_code', None)
                error_message = MODBUS_EXCEPTIONS.get(error_code, f"Unknown error code {error_code}")
                logging.error(f"Modbus error reading registers: Code {error_code} - {error_message}. Response: {response}")

        except ConnectionException as ce:
            # Handle connection errors during read
            logging.error(f"Connection error during read: {ce}")
        except Exception as e:
            # Handle other unexpected errors during read attempt
            logging.error(f"An unexpected error occurred during read: {e}")

    else:
        logging.error(f"Failed to connect to the Modbus TCP server at {args.ip_address}:{MODBUS_PORT}.")

except Exception as e:
    # Catch errors during initial connection or loop setup
    logging.error(f"An unexpected error occurred: {e}")

finally:
    if client.is_socket_open():
        client.close()
        logging.info("Modbus connection closed.")

# Write results to JSON file only if read was successful
if read_success and results is not None:
    try:
        with open(OUTPUT_FILE, 'w') as f:
            json.dump(results, f, indent=4)
        logging.info(f"Register results successfully written to {OUTPUT_FILE}")
    except IOError as e:
        logging.error(f"Failed to write results to {OUTPUT_FILE}: {e}")
else:
    if connection_success and not read_success:
         logging.warning("Read operation failed, JSON file not written.")
    elif not connection_success:
         logging.warning("Connection failed, JSON file not written.")

# Exit with error code if connection or read failed
if not connection_success or not read_success:
    exit(1) # Indicate failure 