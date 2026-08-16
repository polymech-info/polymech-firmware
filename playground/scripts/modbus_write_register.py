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
parser = argparse.ArgumentParser(description='Connect to Modbus TCP server and write a single holding register.')
parser.add_argument('--ip-address', type=str, default='192.168.1.250', \
                    help='IP address of the Modbus TCP server, defaults to 192.168.1.250')
parser.add_argument('--address', type=int, required=True, \
                    help='Holding register address to write.')
# Make --value optional, but capture remaining args
parser.add_argument('--value', type=int, required=False, \
                    help='Value to write to the register (integer).')
parser.add_argument('remaining_args', nargs=argparse.REMAINDER)
args = parser.parse_args()

# --- Determine the value to write ---
register_value = None
if args.value is not None:
    register_value = args.value
elif len(args.remaining_args) == 1:
    # If --value wasn't provided, but exactly one remaining arg exists, assume it's the value
    try:
        register_value = int(args.remaining_args[0])
        logging.debug(f"Using positional argument {register_value} as the value.")
    except ValueError:
        parser.error(f"Invalid integer value provided: {args.remaining_args[0]}")
else:
    # If --value is missing and remaining_args doesn't have exactly one item
    parser.error("The --value argument is required (or provide a single integer value after the address).")

# --- Main Script ---
client = ModbusTcpClient(args.ip_address, port=MODBUS_PORT)
connection_success = False
write_success = False

try:
    logging.info(f"Attempting to connect to Modbus TCP server at {args.ip_address}:{MODBUS_PORT}...")
    connection_success = client.connect()

    if connection_success:
        logging.info("Connection successful.")

        # register_value = args.value # Removed: value determined above
        logging.info(f"Writing register {args.address} to value {register_value}...")
        try:
            # Write Single Register (Function Code 06)
            response = client.write_register(address=args.address, value=register_value)

            if not response.isError():
                logging.info(f"Successfully wrote register {args.address} to {register_value}.")
                write_success = True
                # Print confirmation
                print(f"\n--- Wrote Register {args.address} = {register_value} ---\n")
            else:
                # Handle Modbus logical errors
                error_code = getattr(response, 'exception_code', None)
                error_message = MODBUS_EXCEPTIONS.get(error_code, f"Unknown error code {error_code}")
                logging.error(f"Modbus error writing register: Code {error_code} - {error_message}. Response: {response}")

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