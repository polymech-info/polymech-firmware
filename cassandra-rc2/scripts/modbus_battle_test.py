#!/usr/bin/env python3
"""
Modbus Battle Test - Tests how quickly Modbus registers can be updated
"""
import argparse
import json
import os
import logging
import time
import statistics
from datetime import datetime
from pymodbus.client import ModbusTcpClient
from pymodbus.exceptions import ConnectionException
from pymodbus.pdu import ExceptionResponse

# --- Configuration ---
MODBUS_PORT = 502
MB_BATTLE_COUNTER_REG = 20   # Counter register address
MB_BATTLE_TIMESTAMP_REG = 21 # Timestamp register address
OUTPUT_DIR = "tmp"
OUTPUT_FILE = os.path.join(OUTPUT_DIR, "modbus_battle.json")
LOG_LEVEL = logging.INFO
RETRY_COUNT = 3             # Number of retries for connection errors
RETRY_DELAY = 0.1           # Delay between retries in seconds

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
parser = argparse.ArgumentParser(description='Modbus Battle Test - Test how quickly Modbus registers can be updated')
parser.add_argument('--ip-address', type=str, default='192.168.1.250', 
                    help='IP address of the Modbus TCP server (ESP32), defaults to 192.168.1.250')
parser.add_argument('--count', type=int, default=1000,
                    help='Number of update iterations to perform')
parser.add_argument('--delay', type=float, default=0.0,
                    help='Delay in seconds between updates (0 for max speed)')
parser.add_argument('--values', type=str, default='increment',
                    choices=['increment', 'random', 'alternating'],
                    help='Values to write: increment (1,2,3...), random, or alternating (0,1,0,1...)')
parser.add_argument('--retries', type=int, default=RETRY_COUNT,
                    help=f'Number of retries for connection errors (default: {RETRY_COUNT})')
parser.add_argument('--retry-delay', type=float, default=RETRY_DELAY,
                    help=f'Delay between retries in seconds (default: {RETRY_DELAY})')
parser.add_argument('--log-level', type=str, default='INFO',
                    choices=['DEBUG', 'INFO', 'WARNING', 'ERROR', 'CRITICAL'],
                    help='Set logging level (default: INFO)')
args = parser.parse_args()

# Set log level from command line
if args.log_level:
    numeric_level = getattr(logging, args.log_level.upper(), None)
    if isinstance(numeric_level, int):
        logging.getLogger().setLevel(numeric_level)

# Create output directory
try:
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    logging.info(f"Ensured output directory exists: {OUTPUT_DIR}")
except OSError as e:
    logging.error(f"Failed to create output directory {OUTPUT_DIR}: {e}")
    exit(1)

def get_next_value(current_value, mode, iteration):
    """Returns the next value to write based on the specified mode"""
    if mode == 'increment':
        return current_value + 1
    elif mode == 'random':
        import random
        return random.randint(1, 65535)
    elif mode == 'alternating':
        return (iteration % 2)  # Returns 0 or 1
    return current_value + 1  # Default to increment

def write_with_retry(client, address, value, retries=RETRY_COUNT, retry_delay=RETRY_DELAY):
    """Write a register value with retry logic for connection errors"""
    connection_reset = False
    for attempt in range(retries + 1):
        try:
            # If we had a connection reset on previous attempt, reconnect first
            if connection_reset and not client.is_socket_open():
                logging.debug(f"Reconnecting after connection reset (attempt {attempt+1})")
                client.connect()
                # Give the server time to register the new connection
                time.sleep(retry_delay * 2)
                
            response = client.write_register(address, value)
            return response, None
        except ConnectionError as e:
            # Connection reset errors are common when other clients disconnect
            connection_reset = True
            if "reset by peer" in str(e).lower() or "connection reset" in str(e).lower():
                logging.debug(f"Connection reset detected, likely another client disconnected")
            
            if attempt < retries:
                # Use exponential backoff for connection issues
                wait_time = retry_delay * (attempt + 1)
                logging.debug(f"Retry {attempt+1}/{retries} after write error: {e} (waiting {wait_time:.2f}s)")
                time.sleep(wait_time)
            else:
                return None, e
        except Exception as e:
            if attempt < retries:
                # Use exponential backoff for connection issues
                wait_time = retry_delay * (attempt + 1)
                logging.debug(f"Retry {attempt+1}/{retries} after write error: {e} (waiting {wait_time:.2f}s)")
                time.sleep(wait_time)
            else:
                return None, e
    return None, Exception("Maximum retries exceeded")

def read_with_retry(client, address, count, retries=RETRY_COUNT, retry_delay=RETRY_DELAY):
    """Read register(s) with retry logic for connection errors"""
    connection_reset = False
    for attempt in range(retries + 1):
        try:
            # If we had a connection reset on previous attempt, reconnect first
            if connection_reset and not client.is_socket_open():
                logging.debug(f"Reconnecting after connection reset (attempt {attempt+1})")
                client.connect()
                # Give the server time to register the new connection
                time.sleep(retry_delay * 2)
                
            response = client.read_holding_registers(address=address, count=count)
            return response, None
        except ConnectionError as e:
            # Connection reset errors are common when other clients disconnect
            connection_reset = True
            if "reset by peer" in str(e).lower() or "connection reset" in str(e).lower():
                logging.debug(f"Connection reset detected, likely another client disconnected")
            
            if attempt < retries:
                # Use exponential backoff for connection issues
                wait_time = retry_delay * (attempt + 1)
                logging.debug(f"Retry {attempt+1}/{retries} after read error: {e} (waiting {wait_time:.2f}s)")
                time.sleep(wait_time)
            else:
                return None, e
        except Exception as e:
            if attempt < retries:
                # Use exponential backoff for connection issues
                wait_time = retry_delay * (attempt + 1)
                logging.debug(f"Retry {attempt+1}/{retries} after read error: {e} (waiting {wait_time:.2f}s)")
                time.sleep(wait_time)
            else:
                return None, e
    return None, Exception("Maximum retries exceeded")

def run_battle_test(client, count, delay, value_mode):
    """Performs the Modbus battle test with timing metrics"""
    write_times = []
    verify_times = []
    round_trip_times = []
    success_count = 0
    current_value = 0
    retry_count = args.retries
    retry_delay = args.retry_delay
    reconnect_count = 0
    
    # First reset the counter to 0
    try:
        client.write_register(MB_BATTLE_COUNTER_REG, 0)
        time.sleep(0.1)  # Small delay to ensure reset takes effect
    except Exception as e:
        logging.error(f"Failed to reset counter: {e}")
        return None
        
    logging.info(f"Starting Modbus battle test with {count} iterations, {delay}s delay, and {value_mode} values")
    logging.info(f"Using {retry_count} retries with {retry_delay}s delay between retries")
    
    test_start_time = time.time()
    
    for i in range(count):
        next_value = get_next_value(current_value, value_mode, i)
        iteration_start = time.time()
        
        # Check connection state
        if not client.is_socket_open():
            logging.warning(f"Connection lost at iteration {i}, attempting to reconnect...")
            try:
                client.connect()
                reconnect_count += 1
                logging.info(f"Successfully reconnected (reconnect #{reconnect_count})")
                time.sleep(retry_delay * 2)  # Give server time to setup connection
            except Exception as ce:
                logging.error(f"Failed to reconnect: {ce}")
        
        # Write the value and measure time
        write_start = time.time()
        write_success = False
        
        response, write_error = write_with_retry(client, MB_BATTLE_COUNTER_REG, next_value, 
                                              retry_count, retry_delay)
        
        if write_error:
            logging.error(f"Exception during write at iteration {i}: {write_error}")
        elif response and not response.isError():
            write_success = True
            current_value = next_value
        else:
            error_msg = MODBUS_EXCEPTIONS.get(response.exception_code, "Unknown") if response else "No response"
            logging.error(f"Write error at iteration {i}: {error_msg}")
        
        write_end = time.time()
        write_time = (write_end - write_start) * 1000  # Convert to ms
        write_times.append(write_time)
        
        # Verify the value was written correctly
        verify_start = time.time()
        verify_success = False
        
        if write_success:
            read_response, read_error = read_with_retry(client, MB_BATTLE_COUNTER_REG, 1,
                                                     retry_count, retry_delay)
            
            if read_error:
                logging.error(f"Exception during verification at iteration {i}: {read_error}")
            elif read_response and not read_response.isError() and len(read_response.registers) > 0:
                read_value = read_response.registers[0]
                if read_value == next_value:
                    verify_success = True
                else:
                    logging.warning(f"Verification mismatch at iteration {i}: Expected {next_value}, got {read_value}")
            else:
                error_msg = MODBUS_EXCEPTIONS.get(read_response.exception_code, "Unknown") if read_response else "No response"
                logging.error(f"Read error at iteration {i}: {error_msg}")
        
        verify_end = time.time()
        verify_time = (verify_end - verify_start) * 1000  # Convert to ms
        verify_times.append(verify_time)
        
        # Calculate round-trip time
        round_trip_time = (verify_end - write_start) * 1000  # Convert to ms
        round_trip_times.append(round_trip_time)
        
        # Count successful operations
        if write_success and verify_success:
            success_count += 1
            
        # Show progress
        if i % 10 == 0 or i == count - 1:
            elapsed = time.time() - test_start_time
            percent_complete = (i+1) / count * 100
            estimated_total = elapsed / (i+1) * count if i > 0 else 0
            remaining = estimated_total - elapsed if estimated_total > 0 else 0
            
            logging.info(f"Progress: {i+1}/{count} iterations ({percent_complete:.1f}%), " + 
                         f"{success_count} successful, " +
                         f"ETA: {remaining:.1f}s remaining")
            
        # Apply delay if specified
        if delay > 0:
            time.sleep(delay)
            
    # Calculate statistics
    success_rate = (success_count / count) * 100 if count > 0 else 0
    test_elapsed_time = time.time() - test_start_time
    
    results = {
        "timestamp": datetime.now().isoformat(),
        "parameters": {
            "ip_address": args.ip_address,
            "count": count,
            "delay": delay,
            "value_mode": value_mode,
            "retries": retry_count,
            "retry_delay": retry_delay
        },
        "results": {
            "success_count": success_count,
            "success_rate": success_rate,
            "reconnect_count": reconnect_count,
            "total_elapsed_seconds": test_elapsed_time,
            "write_times_ms": {
                "min": min(write_times) if write_times else None,
                "max": max(write_times) if write_times else None,
                "mean": statistics.mean(write_times) if write_times else None,
                "median": statistics.median(write_times) if write_times else None,
                "percentile_95": statistics.quantiles(write_times, n=20)[18] if len(write_times) >= 20 else None
            },
            "verify_times_ms": {
                "min": min(verify_times) if verify_times else None,
                "max": max(verify_times) if verify_times else None,
                "mean": statistics.mean(verify_times) if verify_times else None,
                "median": statistics.median(verify_times) if verify_times else None,
                "percentile_95": statistics.quantiles(verify_times, n=20)[18] if len(verify_times) >= 20 else None
            },
            "round_trip_times_ms": {
                "min": min(round_trip_times) if round_trip_times else None,
                "max": max(round_trip_times) if round_trip_times else None,
                "mean": statistics.mean(round_trip_times) if round_trip_times else None,
                "median": statistics.median(round_trip_times) if round_trip_times else None,
                "percentile_95": statistics.quantiles(round_trip_times, n=20)[18] if len(round_trip_times) >= 20 else None
            },
            "max_update_rate_per_second": 1000 / statistics.mean(round_trip_times) if round_trip_times else None
        },
        "all_data": {
            "write_times": write_times,
            "verify_times": verify_times,
            "round_trip_times": round_trip_times
        }
    }
    
    return results

# --- Main Script ---
def main():
    client = ModbusTcpClient(args.ip_address, port=MODBUS_PORT)
    connection_success = False
    results = None
    
    try:
        logging.info(f"Connecting to Modbus TCP server at {args.ip_address}:{MODBUS_PORT}...")
        connection_success = client.connect()
        
        if connection_success:
            logging.info("Connection successful")
            results = run_battle_test(client, args.count, args.delay, args.values)
            if results:
                # Print summary to console
                print("\n--- Modbus Battle Test Results ---")
                print(f"Iterations: {args.count}")
                print(f"Success rate: {results['results']['success_rate']:.2f}%")
                print(f"Reconnections: {results['results']['reconnect_count']}")
                print(f"Total time: {results['results']['total_elapsed_seconds']:.2f} seconds")
                print(f"Write time (avg): {results['results']['write_times_ms']['mean']:.2f} ms")
                print(f"Verify time (avg): {results['results']['verify_times_ms']['mean']:.2f} ms")
                print(f"Round-trip time (avg): {results['results']['round_trip_times_ms']['mean']:.2f} ms")
                print(f"Maximum update rate: {results['results']['max_update_rate_per_second']:.2f} updates/second")
                
                # Print additional connection stats
                if results['results']['reconnect_count'] > 0:
                    print("\nConnection Information:")
                    print(f"  Reconnection events: {results['results']['reconnect_count']}")
                    print(f"  Average time between reconnects: {results['results']['total_elapsed_seconds']/results['results']['reconnect_count']:.2f}s")
                
                print("--------------------------------\n")
                
                # Save results to file
                with open(OUTPUT_FILE, 'w') as f:
                    json.dump(results, f, indent=2)
                logging.info(f"Results saved to {OUTPUT_FILE}")
        else:
            logging.error(f"Failed to connect to Modbus TCP server at {args.ip_address}:{MODBUS_PORT}")
            
    except Exception as e:
        logging.error(f"An error occurred: {e}")
    finally:
        if client.is_socket_open():
            client.close()
            logging.info("Modbus connection closed")

if __name__ == "__main__":
    main() 