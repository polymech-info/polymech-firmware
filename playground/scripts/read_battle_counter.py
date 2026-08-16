#!/usr/bin/env python3
"""
Read Battle Counter - Simple script to read the battle counter from the ESP32
"""
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
MB_BATTLE_COUNTER_REG = 20  # Counter register address
MB_BATTLE_TIMESTAMP_REG = 21  # Timestamp register address
OUTPUT_DIR = "tmp"
OUTPUT_FILE = os.path.join(OUTPUT_DIR, "battle-counter.json")
LOG_LEVEL = logging.INFO

# --- Setup Logging ---
logging.basicConfig(level=LOG_LEVEL, format='%(asctime)s - %(levelname)s - %(message)s')

# --- Argument Parsing ---
parser = argparse.ArgumentParser(description='Read the battle counter value from the ESP32')
parser.add_argument('--ip-address', type=str, default='192.168.1.250', 
                    help='IP address of the Modbus TCP server (ESP32), defaults to 192.168.1.250')
parser.add_argument('--increment', action='store_true',
                    help='Increment the counter after reading it')
parser.add_argument('--reset', action='store_true',
                    help='Reset the counter to 0')
parser.add_argument('--watch', action='store_true',
                    help='Watch the counter continuously')
parser.add_argument('--interval', type=float, default=1.0,
                    help='Interval in seconds between watch updates (default: 1.0)')
args = parser.parse_args()

# Create output directory
os.makedirs(OUTPUT_DIR, exist_ok=True)

def read_counter(client):
    """Read the battle counter and timestamp from the ESP32"""
    try:
        # Read the counter value at register 20
        response = client.read_holding_registers(address=MB_BATTLE_COUNTER_REG, count=2)
        if not response.isError() and len(response.registers) >= 2:
            counter = response.registers[0]
            timestamp = response.registers[1]
            return counter, timestamp
        else:
            logging.error(f"Error reading counter: {response}")
            return None, None
    except Exception as e:
        logging.error(f"Exception reading counter: {e}")
        return None, None

def increment_counter(client):
    """Increment the battle counter on the ESP32"""
    try:
        # Read current value
        counter, _ = read_counter(client)
        if counter is not None:
            # Increment by 1
            response = client.write_register(address=MB_BATTLE_COUNTER_REG, value=counter+1)
            if not response.isError():
                logging.info(f"Counter incremented from {counter} to {counter+1}")
                return True
            else:
                logging.error(f"Error incrementing counter: {response}")
                return False
        return False
    except Exception as e:
        logging.error(f"Exception incrementing counter: {e}")
        return False

def reset_counter(client):
    """Reset the battle counter to 0"""
    try:
        response = client.write_register(address=MB_BATTLE_COUNTER_REG, value=0)
        if not response.isError():
            logging.info("Counter reset to 0")
            return True
        else:
            logging.error(f"Error resetting counter: {response}")
            return False
    except Exception as e:
        logging.error(f"Exception resetting counter: {e}")
        return False

def watch_counter(client, interval):
    """Watch the counter continuously"""
    last_counter = None
    try:
        print("\nWatching battle counter (Ctrl+C to stop)...")
        print("------------------------------------------------")
        print("| Counter | Timestamp | Changes/sec | Changes |")
        print("------------------------------------------------")
        
        start_time = time.time()
        start_counter = None
        
        while True:
            counter, timestamp = read_counter(client)
            
            if counter is not None:
                if start_counter is None:
                    start_counter = counter
                
                elapsed = time.time() - start_time
                changes = counter - start_counter if start_counter is not None else 0
                rate = changes / elapsed if elapsed > 0 else 0
                
                # Only print if the counter has changed
                if last_counter != counter:
                    print(f"| {counter:7d} | {timestamp:9d} | {rate:11.2f} | {changes:7d} |")
                    last_counter = counter
            
            time.sleep(interval)
            
    except KeyboardInterrupt:
        print("\nStopped watching counter.")

def main():
    client = ModbusTcpClient(args.ip_address, port=MODBUS_PORT)
    
    try:
        logging.info(f"Connecting to Modbus TCP server at {args.ip_address}:{MODBUS_PORT}...")
        connection_success = client.connect()
        
        if connection_success:
            logging.info("Connection successful")
            
            if args.reset:
                reset_counter(client)
                time.sleep(0.1)  # Small delay
            
            if args.watch:
                watch_counter(client, args.interval)
            else:
                counter, timestamp = read_counter(client)
                
                if counter is not None:
                    print(f"\nBattle Counter: {counter}")
                    print(f"Timestamp: {timestamp}")
                    
                    # Save to JSON file
                    with open(OUTPUT_FILE, 'w') as f:
                        json.dump({
                            "timestamp": time.time(),
                            "counter": counter,
                            "modbus_timestamp": timestamp
                        }, f, indent=2)
                    logging.info(f"Counter value saved to {OUTPUT_FILE}")
                
                if args.increment:
                    increment_counter(client)
                    time.sleep(0.1)  # Small delay
                    counter, timestamp = read_counter(client)
                    if counter is not None:
                        print(f"\nAfter increment:")
                        print(f"Battle Counter: {counter}")
                        print(f"Timestamp: {timestamp}")
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