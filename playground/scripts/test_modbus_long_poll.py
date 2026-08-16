import argparse
import logging
import time
import sys
import os
import json
import random
from pymodbus.client import ModbusTcpClient
from pymodbus.exceptions import ConnectionException, ModbusIOException
from pymodbus.pdu import ExceptionResponse

# --- Configuration ---
DEFAULT_MODBUS_PORT = 502
MIN_DELAY_MS = 30
MAX_DELAY_MS = 500
DELAY_STEP_MS = 5 # How much to change delay each cycle
DEFAULT_POLL_DELAY_MS = MIN_DELAY_MS # Start at min delay
DEFAULT_COIL_ADDRESS = 0
DEFAULT_COIL_COUNT = 10
DEFAULT_REGISTER_ADDRESS = 0
DEFAULT_REGISTER_COUNT = 10
LOG_LEVEL = logging.INFO
OUTPUT_DIR = "tmp"
OUTPUT_FILE = os.path.join(OUTPUT_DIR, "long-poll.json")
MAX_ERRORS = 1000
WRITE_PROBABILITY = 0.1 # 10% chance to write each cycle (per type)

# --- Modbus Exception Code Mapping ---
# (Same mapping as in modbus_read_registers.py can be used if needed)
MODBUS_EXCEPTIONS = {
    1: "Illegal Function",
    2: "Illegal Data Address",
    3: "Illegal Data Value",
    4: "Slave Device Failure",
    5: "Acknowledge",
    6: "Slave Device Busy",
    # ... add others if necessary
}

# --- Setup Logging ---
# Configure logging to output to stdout
logging.basicConfig(
    level=LOG_LEVEL,
    format='%(asctime)s - %(levelname)s - %(message)s',
    stream=sys.stdout # Ensure logs go to console
)

# --- Argument Parsing ---
parser = argparse.ArgumentParser(description='Continuously poll Modbus TCP server for coils and registers.')
parser.add_argument('--ip-address', type=str, default='192.168.1.250',
                    help='IP address of the Modbus TCP server (e.g., ESP32). Defaults to 192.168.1.250')
parser.add_argument('--port', type=int, default=DEFAULT_MODBUS_PORT,
                    help=f'Port of the Modbus TCP server. Defaults to {DEFAULT_MODBUS_PORT}')
parser.add_argument('--delay', type=float, default=DEFAULT_POLL_DELAY_MS / 1000.0,
                    help=f'Initial polling delay in seconds. Will vary between {MIN_DELAY_MS/1000.0}s and {MAX_DELAY_MS/1000.0}s. Defaults to {DEFAULT_POLL_DELAY_MS / 1000.0}s')
parser.add_argument('--coil-addr', type=int, default=DEFAULT_COIL_ADDRESS,
                    help=f'Starting address for reading coils. Defaults to {DEFAULT_COIL_ADDRESS}')
parser.add_argument('--coil-count', type=int, default=DEFAULT_COIL_COUNT,
                    help=f'Number of coils to read. Defaults to {DEFAULT_COIL_COUNT}')
parser.add_argument('--reg-addr', type=int, default=DEFAULT_REGISTER_ADDRESS,
                    help=f'Starting address for reading holding registers. Defaults to {DEFAULT_REGISTER_ADDRESS}')
parser.add_argument('--reg-count', type=int, default=DEFAULT_REGISTER_COUNT,
                    help=f'Number of holding registers to read. Defaults to {DEFAULT_REGISTER_COUNT}')

args = parser.parse_args()

# Validate initial delay
if not (MIN_DELAY_MS / 1000.0 <= args.delay <= MAX_DELAY_MS / 1000.0):
    logging.error(f"Error: Initial polling delay must be between {MIN_DELAY_MS/1000.0}s and {MAX_DELAY_MS/1000.0}s. Provided: {args.delay}s")
    sys.exit(1)

# poll_delay_sec = args.delay # Replaced by dynamic delay

# --- Statistics Tracking ---
stats = {
    "poll_cycles": 0,
    "connection_success": 0,
    "connection_failure": 0,
    "coil_read_success": 0,
    "coil_read_error": 0,
    "register_read_success": 0,
    "register_read_error": 0,
    "coil_write_success": 0,
    "coil_write_error": 0,
    "register_write_success": 0,
    "register_write_error": 0,
    "last_coil_values": None,
    "last_register_values": None
}

def write_stats():
    """Writes the current statistics to the JSON file."""
    try:
        os.makedirs(OUTPUT_DIR, exist_ok=True)
        with open(OUTPUT_FILE, 'w') as f:
            json.dump(stats, f, indent=4)
        logging.debug(f"Statistics successfully written to {OUTPUT_FILE}")
    except IOError as e:
        logging.error(f"Failed to write statistics to {OUTPUT_FILE}: {e}")
    except Exception as e:
        logging.error(f"An unexpected error occurred during statistics write: {e}")

# --- Main Script ---
client = ModbusTcpClient(args.ip_address, port=args.port)

logging.info(f"Starting Modbus long poll test:")
logging.info(f"  Target: {args.ip_address}:{args.port}")
logging.info(f"  Polling Delay: Varies {MIN_DELAY_MS/1000.0:.3f}s - {MAX_DELAY_MS/1000.0:.3f}s (Initial: {args.delay:.3f}s)")
logging.info(f"  Coils: Addr={args.coil_addr}, Count={args.coil_count}")
logging.info(f"  Registers: Addr={args.reg_addr}, Count={args.reg_count}")
logging.info(f"  Max Errors: {MAX_ERRORS}")
logging.info(f"  Write Probability: {WRITE_PROBABILITY*100}%")
logging.info("Press Ctrl+C to stop.")

# --- Dynamic Delay State ---
current_delay_sec = args.delay
delay_direction = 1 # 1 for increasing, -1 for decreasing
min_delay_sec = MIN_DELAY_MS / 1000.0
max_delay_sec = MAX_DELAY_MS / 1000.0
delay_step_sec = DELAY_STEP_MS / 1000.0

try:
    while True:
        stats["poll_cycles"] += 1
        logging.debug(f"--- Poll Cycle {stats['poll_cycles']} ---")
        cycle_connection_error = False # Track connection error specifically for this cycle

        try:
            if not client.is_socket_open():
                logging.info(f"Attempting to connect to {args.ip_address}:{args.port}...")
                connection_success = client.connect()
                if not connection_success:
                    logging.error("Connection failed. Retrying next cycle.")
                    stats["connection_failure"] += 1
                    cycle_connection_error = True
                    # No time.sleep here, handled at the end of the loop
                    # continue # Don't continue, let it write stats and sleep
                else:
                    logging.info("Connection successful.")
                    stats["connection_success"] += 1

            # Only attempt reads if connection is presumably okay for this cycle
            if not cycle_connection_error and client.is_socket_open():
                # --- Read Coils ---
                logging.debug(f"Reading {args.coil_count} coils from address {args.coil_addr}...")
                try:
                    coil_response = client.read_coils(address=args.coil_addr, count=args.coil_count)

                    if coil_response.isError():
                        stats["coil_read_error"] += 1
                        stats["last_coil_values"] = "Error"
                        if isinstance(coil_response, ExceptionResponse):
                            error_code = coil_response.exception_code
                            error_message = MODBUS_EXCEPTIONS.get(error_code, f"Unknown error code {error_code}")
                            logging.error(f"Modbus error reading coils: Code {error_code} - {error_message}. Response: {coil_response}")
                        else:
                             logging.error(f"Failed to read coils. Response: {coil_response}")
                    elif isinstance(coil_response, ModbusIOException):
                        stats["coil_read_error"] += 1
                        stats["last_coil_values"] = "IOError"
                        logging.error(f"Modbus IO exception reading coils: {coil_response}")
                        client.close() # Close socket on IO error
                    else:
                        stats["coil_read_success"] += 1
                        coil_values = coil_response.bits[:args.coil_count] # Ensure correct count
                        stats["last_coil_values"] = coil_values
                        logging.info(f"Successfully read {len(coil_values)} coils: {coil_values}")

                except ConnectionException as ce:
                    stats["coil_read_error"] += 1 # Count as coil error and connection failure
                    stats["connection_failure"] += 1
                    stats["last_coil_values"] = "ConnectionError"
                    logging.error(f"Connection error during coil read: {ce}. Closing connection.")
                    if client.is_socket_open(): client.close()
                except Exception as e:
                     stats["coil_read_error"] += 1
                     stats["last_coil_values"] = f"Exception: {e}"
                     logging.error(f"Unexpected error during coil read: {e}")

                # --- Read Holding Registers ---
                if client.is_socket_open(): # Check connection again before next read
                    logging.debug(f"Reading {args.reg_count} holding registers from address {args.reg_addr}...")
                    try:
                        register_response = client.read_holding_registers(address=args.reg_addr, count=args.reg_count)

                        if register_response.isError():
                            stats["register_read_error"] += 1
                            stats["last_register_values"] = "Error"
                            if isinstance(register_response, ExceptionResponse):
                                error_code = register_response.exception_code
                                error_message = MODBUS_EXCEPTIONS.get(error_code, f"Unknown error code {error_code}")
                                logging.error(f"Modbus error reading registers: Code {error_code} - {error_message}. Response: {register_response}")
                            else:
                                logging.error(f"Failed to read registers. Response: {register_response}")
                        elif isinstance(register_response, ModbusIOException):
                            stats["register_read_error"] += 1
                            stats["last_register_values"] = "IOError"
                            logging.error(f"Modbus IO exception reading registers: {register_response}")
                            client.close() # Close socket on IO error
                        else:
                            stats["register_read_success"] += 1
                            register_values = register_response.registers[:args.reg_count] # Ensure correct count
                            stats["last_register_values"] = register_values
                            logging.info(f"Successfully read {len(register_values)} registers: {register_values}")

                    except ConnectionException as ce:
                        stats["register_read_error"] += 1 # Count as register error and connection failure
                        stats["connection_failure"] += 1
                        stats["last_register_values"] = "ConnectionError"
                        logging.error(f"Connection error during register read: {ce}. Closing connection.")
                        if client.is_socket_open(): client.close()
                    except Exception as e:
                        stats["register_read_error"] += 1
                        stats["last_register_values"] = f"Exception: {e}"
                        logging.error(f"Unexpected error during register read: {e}")

                # --- Random Writes ---
                # Write Coil?
                if random.random() < WRITE_PROBABILITY:
                    write_addr = random.randint(args.coil_addr, args.coil_addr + args.coil_count - 1)
                    write_val = random.choice([True, False])
                    logging.debug(f"Attempting to write Coil {write_addr} = {write_val}")
                    try:
                        write_response = client.write_coil(write_addr, write_val)
                        if write_response.isError():
                            stats["coil_write_error"] += 1
                            if isinstance(write_response, ExceptionResponse):
                                error_code = write_response.exception_code
                                error_message = MODBUS_EXCEPTIONS.get(error_code, f"Unknown error code {error_code}")
                                logging.error(f"Modbus error writing coil {write_addr}: Code {error_code} - {error_message}. Response: {write_response}")
                            else:
                                logging.error(f"Failed to write coil {write_addr}. Response: {write_response}")
                        elif isinstance(write_response, ModbusIOException):
                             stats["coil_write_error"] += 1
                             logging.error(f"Modbus IO exception writing coil {write_addr}: {write_response}")
                             client.close()
                        else:
                            stats["coil_write_success"] += 1
                            logging.info(f"Successfully wrote Coil {write_addr} = {write_val}")
                    except ConnectionException as ce:
                        stats["coil_write_error"] += 1
                        stats["connection_failure"] += 1 # Also count as connection failure
                        logging.error(f"Connection error during coil write to {write_addr}: {ce}. Closing connection.")
                        if client.is_socket_open(): client.close()
                    except Exception as e:
                        stats["coil_write_error"] += 1
                        logging.error(f"Unexpected error during coil write to {write_addr}: {e}")

                # Write Register?
                if client.is_socket_open() and random.random() < WRITE_PROBABILITY:
                    write_addr = random.randint(args.reg_addr, args.reg_addr + args.reg_count - 1)
                    write_val = random.randint(0, 65535)
                    logging.debug(f"Attempting to write Register {write_addr} = {write_val}")
                    try:
                        write_response = client.write_register(write_addr, write_val)
                        if write_response.isError():
                            stats["register_write_error"] += 1
                            if isinstance(write_response, ExceptionResponse):
                                error_code = write_response.exception_code
                                error_message = MODBUS_EXCEPTIONS.get(error_code, f"Unknown error code {error_code}")
                                logging.error(f"Modbus error writing register {write_addr}: Code {error_code} - {error_message}. Response: {write_response}")
                            else:
                                logging.error(f"Failed to write register {write_addr}. Response: {write_response}")
                        elif isinstance(write_response, ModbusIOException):
                             stats["register_write_error"] += 1
                             logging.error(f"Modbus IO exception writing register {write_addr}: {write_response}")
                             client.close()
                        else:
                            stats["register_write_success"] += 1
                            logging.info(f"Successfully wrote Register {write_addr} = {write_val}")
                    except ConnectionException as ce:
                        stats["register_write_error"] += 1
                        stats["connection_failure"] += 1 # Also count as connection failure
                        logging.error(f"Connection error during register write to {write_addr}: {ce}. Closing connection.")
                        if client.is_socket_open(): client.close()
                    except Exception as e:
                        stats["register_write_error"] += 1
                        logging.error(f"Unexpected error during register write to {write_addr}: {e}")

        except ConnectionException as ce:
            # Handle connection errors during the cycle's connection attempt
            stats["connection_failure"] += 1 # Ensure this is counted if connect() fails
            logging.error(f"Connection error during connect attempt: {ce}")
            if client.is_socket_open():
                 client.close() # Ensure closed if error occurred after partial connect
        except Exception as e:
            # Catch-all for unexpected errors in the main loop
            logging.error(f"An unexpected error occurred in poll cycle {stats['poll_cycles']}: {e}")
            if client.is_socket_open():
                 client.close() # Attempt to clean up connection

        # --- Write stats for this cycle ---
        write_stats()

        # --- Check for error limit ---
        total_errors = stats["connection_failure"] + stats["coil_read_error"] + stats["register_read_error"] + \
                       stats["coil_write_error"] + stats["register_write_error"]
        if total_errors >= MAX_ERRORS:
            logging.error(f"Stopping test: Reached maximum error limit of {MAX_ERRORS} (Total errors: {total_errors}).")
            break # Exit the while loop

        # --- Wait for next poll cycle --- Adjust Delay ---
        sleep_time = current_delay_sec
        logging.debug(f"Waiting {sleep_time:.3f}s before next poll...")
        time.sleep(sleep_time)

        # Update delay for next cycle
        current_delay_sec += delay_step_sec * delay_direction
        if current_delay_sec >= max_delay_sec:
            current_delay_sec = max_delay_sec
            delay_direction = -1
            logging.debug("Reached max delay, reversing direction.")
        elif current_delay_sec <= min_delay_sec:
            current_delay_sec = min_delay_sec
            delay_direction = 1
            logging.debug("Reached min delay, reversing direction.")

except KeyboardInterrupt:
    logging.info("\nCtrl+C received. Stopping the poll test.")
except Exception as e:
    logging.error(f"An critical error occurred: {e}")
finally:
    logging.info("Performing final actions...")
    if client.is_socket_open():
        client.close()
        logging.info("Modbus connection closed.")

    # --- Write final statistics ---
    logging.info("Writing final statistics...")
    write_stats() # Ensure stats are written on exit

    logging.info(f"Test finished after {stats['poll_cycles']} poll cycles.")
    logging.info(f"Final Stats: {stats}") # Log final stats to console too
    sys.exit(0) 