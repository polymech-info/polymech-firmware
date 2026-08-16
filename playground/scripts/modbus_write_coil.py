import argparse
import os
import logging
from pymodbus.client import ModbusTcpClient
from pymodbus.exceptions import ConnectionException
from pymodbus.pdu import ExceptionResponse

# --- Configuration ---
MODBUS_PORT = 502
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
parser = argparse.ArgumentParser(description='Connect to Modbus TCP server and write a single coil.')
parser.add_argument('--ip-address', type=str, default='192.168.1.250', \
                    help='IP address of the Modbus TCP server, defaults to 192.168.1.250')
parser.add_argument('--address', type=int, required=True, \
                    help='Coil address to write.')
parser.add_argument('--value', type=int, required=True, choices=[0, 1], \
                    help='Value to write to the coil (0 for OFF, 1 for ON).')
args = parser.parse_args()

# --- Main Script ---
client = ModbusTcpClient(args.ip_address, port=MODBUS_PORT)
connection_success = False
write_success = False

try:
    logging.info(f"Attempting to connect to Modbus TCP server at {args.ip_address}:{MODBUS_PORT}...")
    connection_success = client.connect()

    if connection_success:
        logging.info("Connection successful.")

        coil_value = bool(args.value)
        logging.info(f"Writing coil {args.address} to value {coil_value} ({args.value})...")
        try:
            # Write Single Coil (Function Code 05)
            response = client.write_coil(address=args.address, value=coil_value)

            # --- Add extra debugging ---
            logging.debug(f"Raw write response: {response}")
            if response is None:
                logging.error("Modbus write coil received None response (timeout or connection issue?)")
            # --- End extra debugging ---

            if response is not None and not response.isError():
                logging.info(f"Successfully wrote coil {args.address} to {coil_value}.")
                write_success = True
                # Print confirmation
                print(f"\n--- Wrote Coil {args.address} = {coil_value} ---\n")
            else:
                # Handle Modbus logical errors
                error_code = getattr(response, 'exception_code', None)
                error_message = MODBUS_EXCEPTIONS.get(error_code, f"Unknown error code {error_code}")
                logging.error(f"Modbus error writing coil: Code {error_code} - {error_message}. Response: {response}")

        except ConnectionException as ce:
            logging.error(f"Connection error during write: {ce}")
        except Exception as e:
            logging.error(f"An unexpected error occurred during write: {e}")

    else:
        logging.error(f"Failed to connect to the Modbus TCP server at {args.ip_address}:{MODBUS_PORT}.")

except Exception as e:
    logging.error(f"An unexpected error occurred: {e}")

finally:
    if client.is_socket_open():
        client.close()
        logging.info("Modbus connection closed.")

# Exit with error code if connection or write failed
if not connection_success or not write_success:
    exit(1) # Indicate failure 