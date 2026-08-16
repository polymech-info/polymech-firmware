#!/usr/bin/env python3
"""
Multi-Client Modbus Test - Tests how many concurrent Modbus clients the ESP32 can handle
"""
import argparse
import json
import os
import logging
import time
import threading
import statistics
from datetime import datetime
from multiprocessing import Process, Queue, Value
from pymodbus.client import ModbusTcpClient
from pymodbus.exceptions import ConnectionException

# --- Configuration ---
MODBUS_PORT = 502
MB_BATTLE_COUNTER_REG = 20   # Counter register address
MB_BATTLE_TIMESTAMP_REG = 21 # Timestamp register address
MB_CLIENT_COUNT_REG = 22     # Current number of connected clients
MB_CLIENT_MAX_REG = 23       # Maximum number of clients seen
MB_CLIENT_TOTAL_REG = 24     # Total client connections since start
OUTPUT_DIR = "tmp"
OUTPUT_FILE = os.path.join(OUTPUT_DIR, "multi-client-test.json")
LOG_LEVEL = logging.INFO

# --- Setup Logging ---
logging.basicConfig(level=LOG_LEVEL, format='%(asctime)s - %(levelname)s - %(message)s')

# --- Argument Parsing ---
parser = argparse.ArgumentParser(description='Multi-Client Modbus Test - Test concurrent client connections')
parser.add_argument('--ip-address', type=str, default='192.168.1.250', 
                    help='IP address of the Modbus TCP server (ESP32), defaults to 192.168.1.250')
parser.add_argument('--clients', type=int, default=4,
                    help='Number of concurrent clients to test (default: 4)')
parser.add_argument('--operations', type=int, default=100,
                    help='Number of operations per client (default: 100)')
parser.add_argument('--delay', type=float, default=0.01,
                    help='Delay in seconds between client operations (default: 0.01)')
parser.add_argument('--sequential', action='store_true',
                    help='Run sequential client test')
args = parser.parse_args()

# Create output directory
os.makedirs(OUTPUT_DIR, exist_ok=True)

# --- Client Process Function ---
def client_process(client_id, ip_address, operations, delay, results_queue, stop_flag):
    """Run a client process that connects and performs operations"""
    client = None
    operations_count = 0
    successful_operations = 0
    failed_operations = 0
    connection_attempts = 0
    successful_connections = 0
    failed_connections = 0
    write_times = []
    read_times = []
    round_trip_times = []
    
    try:
        logging.info(f"Client {client_id}: Starting")
        
        # Keep trying to connect and perform operations until we reach the total
        while operations_count < operations and not stop_flag.value:
            # Connect if not connected
            if client is None or not client.is_socket_open():
                connection_attempts += 1
                try:
                    client = ModbusTcpClient(ip_address, port=MODBUS_PORT)
                    connected = client.connect()
                    if connected:
                        successful_connections += 1
                        logging.info(f"Client {client_id}: Connected")
                    else:
                        failed_connections += 1
                        logging.warning(f"Client {client_id}: Connection failed")
                        time.sleep(1)  # Wait a bit before retrying
                        continue
                except Exception as e:
                    failed_connections += 1
                    logging.error(f"Client {client_id}: Connection error: {e}")
                    time.sleep(1)  # Wait a bit before retrying
                    continue
            
            # Perform a read-write-read operation
            try:
                operations_count += 1
                
                # Read current counter value
                read_start = time.time()
                read_response = client.read_holding_registers(address=MB_BATTLE_COUNTER_REG, count=1)
                read_end = time.time()
                read_time = (read_end - read_start) * 1000  # Convert to ms
                
                if read_response.isError():
                    failed_operations += 1
                    logging.error(f"Client {client_id}: Read error: {read_response}")
                    continue
                
                current_value = read_response.registers[0]
                
                # Write incremented value
                write_start = time.time()
                write_response = client.write_register(address=MB_BATTLE_COUNTER_REG, value=current_value+1)
                write_end = time.time()
                write_time = (write_end - write_start) * 1000  # Convert to ms
                
                if write_response.isError():
                    failed_operations += 1
                    logging.error(f"Client {client_id}: Write error: {write_response}")
                    continue
                
                # Verify the write
                verify_start = time.time()
                verify_response = client.read_holding_registers(address=MB_BATTLE_COUNTER_REG, count=1)
                verify_end = time.time()
                verify_time = (verify_end - verify_start) * 1000  # Convert to ms
                
                round_trip_time = (verify_end - read_start) * 1000  # Full operation time in ms
                
                if verify_response.isError():
                    failed_operations += 1
                    logging.error(f"Client {client_id}: Verification error: {verify_response}")
                    continue
                    
                new_value = verify_response.registers[0]
                if new_value != current_value + 1:
                    logging.warning(f"Client {client_id}: Value mismatch: expected {current_value+1}, got {new_value}")
                
                # Success - record times
                successful_operations += 1
                write_times.append(write_time)
                read_times.append((read_time + verify_time) / 2)  # Average of both reads
                round_trip_times.append(round_trip_time)
                
                # Progress reporting
                if operations_count % 10 == 0:
                    logging.info(f"Client {client_id}: Completed {operations_count}/{operations} operations")
                
                # Add delay between operations
                if delay > 0:
                    time.sleep(delay)
                    
            except ConnectionException as ce:
                failed_operations += 1
                logging.error(f"Client {client_id}: Connection error during operation: {ce}")
                client = None  # Will try to reconnect
            except Exception as e:
                failed_operations += 1
                logging.error(f"Client {client_id}: Operation error: {e}")
                # Continue with next operation
        
        # Clean up
        if client and client.is_socket_open():
            client.close()
            
        # Report results
        results = {
            "client_id": client_id,
            "operations": {
                "total": operations_count,
                "successful": successful_operations,
                "failed": failed_operations
            },
            "connections": {
                "attempts": connection_attempts,
                "successful": successful_connections,
                "failed": failed_connections
            },
            "times_ms": {
                "write": {
                    "min": min(write_times) if write_times else None,
                    "max": max(write_times) if write_times else None,
                    "mean": statistics.mean(write_times) if write_times else None,
                    "median": statistics.median(write_times) if write_times else None
                },
                "read": {
                    "min": min(read_times) if read_times else None,
                    "max": max(read_times) if read_times else None,
                    "mean": statistics.mean(read_times) if read_times else None,
                    "median": statistics.median(read_times) if read_times else None
                },
                "round_trip": {
                    "min": min(round_trip_times) if round_trip_times else None,
                    "max": max(round_trip_times) if round_trip_times else None,
                    "mean": statistics.mean(round_trip_times) if round_trip_times else None,
                    "median": statistics.median(round_trip_times) if round_trip_times else None
                }
            },
            "all_times": {
                "write": write_times,
                "read": read_times,
                "round_trip": round_trip_times
            }
        }
        results_queue.put(results)
        logging.info(f"Client {client_id}: Finished - {successful_operations}/{operations_count} successful operations")
        
    except Exception as e:
        logging.error(f"Client {client_id}: Unexpected error: {e}")
        results_queue.put({
            "client_id": client_id,
            "error": str(e),
            "operations": {
                "total": operations_count,
                "successful": successful_operations,
                "failed": failed_operations
            }
        })
    finally:
        if client and client.is_socket_open():
            client.close()

def main():
    # Reset the counter before starting the test
    try:
        reset_client = ModbusTcpClient(args.ip_address, port=MODBUS_PORT)
        if reset_client.connect():
            reset_client.write_register(address=MB_BATTLE_COUNTER_REG, value=0)
            reset_client.write_register(address=MB_CLIENT_COUNT_REG, value=0)
            reset_client.write_register(address=MB_CLIENT_MAX_REG, value=0)
            reset_client.write_register(address=MB_CLIENT_TOTAL_REG, value=0)
            reset_client.close()
            logging.info("Reset counter and client statistics")
        else:
            logging.error("Failed to connect for reset")
    except Exception as e:
        logging.error(f"Error during reset: {e}")
    
    # Add a sequential client test instead of parallel processing
    if args.clients > 0 and args.operations == 1:
        run_sequential_test(args.ip_address, args.clients, args.delay)
        return
    
    # Create shared objects for normal parallel test
    results_queue = Queue()
    stop_flag = Value('i', 0)
    
    # Start client processes
    processes = []
    start_time = time.time()
    
    try:
        logging.info(f"Starting {args.clients} client processes...")
        for i in range(args.clients):
            p = Process(target=client_process, 
                       args=(i+1, args.ip_address, args.operations, args.delay, results_queue, stop_flag))
            processes.append(p)
            p.start()
            # Small delay between starting clients to avoid connection race
            time.sleep(0.1)
        
        # Wait for all processes to complete
        for p in processes:
            p.join()
            
    except KeyboardInterrupt:
        logging.info("Test interrupted by user")
        stop_flag.value = 1
        for p in processes:
            if p.is_alive():
                p.join(timeout=1)
                if p.is_alive():
                    p.terminate()
    
    # Calculate total test time
    total_time = time.time() - start_time
    
    # Collect results
    results = []
    while not results_queue.empty():
        results.append(results_queue.get())
        
    # Get final client statistics
    client_stats = {
        "count": 0,
        "max": 0,
        "total": 0
    }
    
    try:
        stats_client = ModbusTcpClient(args.ip_address, port=MODBUS_PORT)
        if stats_client.connect():
            response = stats_client.read_holding_registers(address=MB_CLIENT_COUNT_REG, count=3)
            if not response.isError():
                client_stats["count"] = response.registers[0]
                client_stats["max"] = response.registers[1]
                client_stats["total"] = response.registers[2]
            stats_client.close()
    except Exception as e:
        logging.error(f"Error getting client stats: {e}")
        
    # Aggregate statistics
    total_operations = sum(r["operations"]["total"] for r in results)
    successful_operations = sum(r["operations"]["successful"] for r in results if "operations" in r and "successful" in r["operations"])
    failed_operations = sum(r["operations"]["failed"] for r in results if "operations" in r and "failed" in r["operations"])
    
    # Calculate operations per second
    ops_per_second = successful_operations / total_time if total_time > 0 else 0
    
    # Prepare summary
    summary = {
        "timestamp": datetime.now().isoformat(),
        "test_parameters": {
            "ip_address": args.ip_address,
            "clients": args.clients,
            "operations_per_client": args.operations,
            "delay": args.delay
        },
        "results": {
            "total_time_seconds": total_time,
            "operations": {
                "total": total_operations,
                "successful": successful_operations,
                "failed": failed_operations,
                "success_rate": (successful_operations / total_operations * 100) if total_operations > 0 else 0
            },
            "performance": {
                "operations_per_second": ops_per_second,
                "operations_per_second_per_client": ops_per_second / args.clients if args.clients > 0 else 0
            },
            "client_stats": client_stats
        },
        "client_details": results
    }
    
    # Save results to file
    with open(OUTPUT_FILE, 'w') as f:
        json.dump(summary, f, indent=2)
    
    # Print summary
    print("\n--- Multi-Client Modbus Test Results ---")
    print(f"Clients: {args.clients}")
    print(f"Operations per client: {args.operations}")
    print(f"Total test time: {total_time:.2f} seconds")
    print(f"Total operations: {total_operations}")
    print(f"Successful operations: {successful_operations}")
    print(f"Failed operations: {failed_operations}")
    print(f"Success rate: {summary['results']['operations']['success_rate']:.2f}%")
    print(f"Operations per second (total): {ops_per_second:.2f}")
    print(f"Operations per second (per client): {summary['results']['performance']['operations_per_second_per_client']:.2f}")
    print(f"Server client stats - current: {client_stats['count']}, max: {client_stats['max']}, total: {client_stats['total']}")
    print("--------------------------------------\n")
    print(f"Full results saved to {OUTPUT_FILE}")

def run_sequential_test(ip_address, max_clients, delay_between_clients):
    """Test how many clients the server can handle by connecting them one by one"""
    logging.info(f"Starting sequential client test with up to {max_clients} clients...")
    
    # Array to hold all clients
    clients = []
    clients_connected = 0
    max_connected = 0
    client_data = {}
    
    try:
        # Try to connect clients one by one
        for i in range(1, max_clients + 1):
            logging.info(f"Connecting client {i}...")
            
            try:
                # Create and connect a new client
                client = ModbusTcpClient(ip_address, port=MODBUS_PORT)
                connected = client.connect()
                
                if connected:
                    clients.append(client)
                    clients_connected += 1
                    max_connected = max(max_connected, clients_connected)
                    
                    # Read counter to confirm connection is working
                    response = client.read_holding_registers(address=MB_BATTLE_COUNTER_REG, count=1)
                    if not response.isError():
                        value = response.registers[0]
                        logging.info(f"Client {i} connected successfully. Read value: {value}")
                    else:
                        logging.warning(f"Client {i} connected but read failed: {response}")
                    
                    # Wait between connections
                    time.sleep(delay_between_clients)
                else:
                    logging.error(f"Client {i} failed to connect")
                    break
                    
            except Exception as e:
                logging.error(f"Error connecting client {i}: {e}")
                break
        
        # Get stats
        if clients_connected > 0:
            # Try to read with the first client
            try:
                response = clients[0].read_holding_registers(address=MB_CLIENT_COUNT_REG, count=3)
                if not response.isError():
                    client_data["count"] = response.registers[0]
                    client_data["max"] = response.registers[1]
                    client_data["total"] = response.registers[2]
            except Exception as e:
                logging.error(f"Error reading client stats: {e}")
        
        # Hold connections for a moment
        logging.info(f"Successfully connected {clients_connected} clients. Waiting 5 seconds...")
        time.sleep(5)
        
    except KeyboardInterrupt:
        logging.info("Test interrupted by user")
    finally:
        # Close all clients
        for i, client in enumerate(clients):
            if client.is_socket_open():
                try:
                    client.close()
                    logging.info(f"Disconnected client {i+1}")
                except:
                    pass
    
    # Report results
    print("\n--- Sequential Client Test Results ---")
    print(f"Maximum clients attempted: {max_clients}")
    print(f"Clients successfully connected: {clients_connected}")
    print(f"Success rate: {(clients_connected / max_clients * 100) if max_clients > 0 else 0:.2f}%")
    
    if client_data:
        print(f"Server reported - clients: {client_data.get('count', 'N/A')}, max: {client_data.get('max', 'N/A')}, total: {client_data.get('total', 'N/A')}")
    
    print("--------------------------------------\n")
    
    # Save results
    results = {
        "timestamp": datetime.now().isoformat(),
        "test_type": "sequential",
        "parameters": {
            "ip_address": ip_address,
            "max_clients": max_clients,
            "delay": delay_between_clients
        },
        "results": {
            "clients_connected": clients_connected,
            "success_rate": (clients_connected / max_clients * 100) if max_clients > 0 else 0,
            "client_stats": client_data
        }
    }
    
    with open(OUTPUT_FILE, 'w') as f:
        json.dump(results, f, indent=2)
    
    logging.info(f"Results saved to {OUTPUT_FILE}")

if __name__ == "__main__":
    # Override for sequential test
    if args.sequential:
        args.operations = 1
    
    main() 