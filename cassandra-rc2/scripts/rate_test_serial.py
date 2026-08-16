# scripts/rate_test_serial.py
import serial
import time
import sys
import argparse
import serial.tools.list_ports
import json
import os
from datetime import datetime

# --- Configuration ---
DEFAULT_PORT = "COM12"
BAUD_RATE = 115200
INIT_DELAY = 2.5  # Match original script's 2.5s delay after connection
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

def test_command_rate(port, command, min_delay_ms, max_delay_ms, step_ms, num_commands, report_file=None):
    """Tests sending commands at different rates to find the maximum reliable rate."""
    ser = None
    current_delay = max_delay_ms  # Start with the slowest rate (most likely to work)
    
    # Prepare report data
    report_data = {
        "timestamp": datetime.now().isoformat(),
        "command": command,
        "port": port,
        "baud_rate": BAUD_RATE,
        "parameters": {
            "min_delay_ms": min_delay_ms,
            "max_delay_ms": max_delay_ms,
            "step_ms": step_ms,
            "commands_per_test": num_commands
        },
        "tests": [],
        "result": {
            "max_reliable_rate_cmds_per_sec": None,
            "min_reliable_delay_ms": None,
            "success": False
        }
    }
    
    try:
        print(f"Opening connection to {port} at {BAUD_RATE} baud...")
        ser = serial.Serial(port, BAUD_RATE, timeout=1)
        print(f"Connected. Waiting {INIT_DELAY}s for initialization...")
        time.sleep(INIT_DELAY)  # Initial delay
        ser.reset_input_buffer()
        
        # First test with maximum delay to establish baseline
        print(f"\n=== Testing with {current_delay}ms delay (baseline) ===")
        success, test_data = run_command_sequence(ser, command, current_delay, num_commands)
        report_data["tests"].append({
            "delay_ms": current_delay,
            "success": success,
            "data": test_data
        })
        
        if not success:
            print("Failed even with maximum delay! Device might not be responding correctly.")
            return report_data
            
        # Binary search to find the threshold
        low = min_delay_ms
        high = max_delay_ms
        last_success_delay = max_delay_ms
        
        while high - low > step_ms:
            mid = (low + high) // 2
            print(f"\n=== Testing with {mid}ms delay ===")
            success, test_data = run_command_sequence(ser, command, mid, num_commands)
            report_data["tests"].append({
                "delay_ms": mid,
                "success": success,
                "data": test_data
            })
            
            if success:
                high = mid  # Try with a shorter delay
                last_success_delay = mid
            else:
                low = mid  # Need a longer delay
        
        # Final confirmation of the threshold
        print(f"\n=== FINAL TEST with {last_success_delay}ms delay ===")
        success, test_data = run_command_sequence(ser, command, last_success_delay, num_commands)
        report_data["tests"].append({
            "delay_ms": last_success_delay,
            "success": success,
            "data": test_data,
            "is_final_test": True
        })
        
        if success:
            rate = 1000.0 / last_success_delay
            print(f"\n✅ Maximum reliable rate: approx. {rate:.2f} commands per second ({last_success_delay}ms delay)")
            report_data["result"]["success"] = True
            report_data["result"]["max_reliable_rate_cmds_per_sec"] = rate
            report_data["result"]["min_reliable_delay_ms"] = last_success_delay
        else:
            print(f"\n⚠️ Results inconsistent. Try again with different parameters.")
            report_data["result"]["success"] = False
            
    except Exception as e:
        print(f"Error: {e}")
        report_data["error"] = str(e)
    finally:
        if ser and ser.is_open:
            print("Closing serial port.")
            ser.close()
        
        # Save report if requested
        if report_file:
            # Ensure directory exists
            os.makedirs(os.path.dirname(os.path.abspath(report_file)), exist_ok=True)
            
            with open(report_file, 'w') as f:
                json.dump(report_data, f, indent=2)
            print(f"Report saved to {report_file}")
            
        return report_data

def run_command_sequence(ser, command, delay_ms, count):
    """Runs a sequence of commands with specified delay between them."""
    expected_responses = ["V: Bridge::onMessage", "V: PHApp::list", "V: Called method:"]
    all_success = True
    test_data = []
    
    for i in range(count):
        print(f"Sending command #{i+1}/{count} with {delay_ms}ms delay")
        success, response_data = send_and_verify(ser, command, expected_responses)
        test_data.append({
            "command_index": i+1,
            "success": success,
            "responses": response_data
        })
        
        all_success = all_success and success
        
        if not success:
            print(f"❌ Failed at attempt #{i+1} - stopping sequence")
            return False, test_data
            
        if i < count - 1:  # Don't sleep after the last command
            time.sleep(delay_ms / 1000.0)
    
    return all_success, test_data

def send_and_verify(ser, command, expected_responses):
    """Sends a command and verifies the response contains expected strings."""
    # Send command
    print(f"  Sending: {command}")
    send_time = time.time()
    if not command.endswith('\n'):
        command += '\n'
    ser.write(command.encode('utf-8'))
    ser.flush()
    
    # Read response with timeout
    start = time.time()
    timeout = 2.0  # 2 seconds to receive a response
    buffer = ""
    found_responses = [False] * len(expected_responses)
    response_data = {
        "send_time": send_time,
        "response_time": None,
        "total_response_time_ms": None,
        "lines": [],
        "expected_responses": {response: False for response in expected_responses},
        "all_found": False,
        "echo_detected": False
    }
    
    while time.time() - start < timeout:
        if ser.in_waiting > 0:
            data = ser.readline().decode('utf-8', errors='ignore').strip()
            if data:
                receive_time = time.time()
                print(f"  Received: {data}")
                buffer += data + "\n"
                response_data["lines"].append({
                    "text": data,
                    "time": receive_time,
                    "delay_ms": round((receive_time - send_time) * 1000, 2)
                })
                
                # First response time
                if response_data["response_time"] is None:
                    response_data["response_time"] = receive_time
                    response_data["total_response_time_ms"] = round((receive_time - send_time) * 1000, 2)
                
                # Check if this line contains any of our expected responses
                for i, expected in enumerate(expected_responses):
                    if expected in data:
                        found_responses[i] = True
                        response_data["expected_responses"][expected] = True
                
                # If we've found all expected responses, we can stop early
                if all(found_responses):
                    print("  ✅ All expected responses received")
                    response_data["all_found"] = True
                    return True, response_data
        else:
            time.sleep(0.01)  # Short sleep to avoid busy waiting
    
    # Check if we've seen all expected responses
    all_found = all(found_responses)
    response_data["all_found"] = all_found
    
    if not all_found:
        print("  ❌ Not all expected responses received")
        for i, expected in enumerate(expected_responses):
            if not found_responses[i]:
                print(f"    Missing: {expected}")
                
        # Look for echo pattern
        if command.strip('\n') in buffer:
            print("  ⚠️ Command echo detected - device might be in echo mode or not handling commands")
            response_data["echo_detected"] = True
    
    return all_found, response_data

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Test maximum command rate for ESP device.")
    parser.add_argument("command", help="The command to send (e.g., '<<1;2;64;list:1:0>>')")
    parser.add_argument("--port", help=f"Serial port (default: auto-detect or {DEFAULT_PORT})")
    parser.add_argument("--min-delay", type=int, default=5, help="Minimum delay between commands (ms)")
    parser.add_argument("--max-delay", type=int, default=1000, help="Maximum delay between commands (ms)")
    parser.add_argument("--step", type=int, default=5, help="Step size for delay adjustment (ms)")
    parser.add_argument("--count", type=int, default=3, help="Number of commands to send in each test sequence")
    parser.add_argument("--report", help="Path to save JSON report (default: ./tmp/battle.json)")
    
    args = parser.parse_args()
    
    # Determine port
    serial_port = args.port if args.port else find_esp_port()
    
    # Set default report path if not specified
    report_path = args.report if args.report else "./tmp/battle.json"
    
    # Run the rate test
    test_command_rate(
        serial_port,
        args.command,
        args.min_delay,
        args.max_delay,
        args.step,
        args.count,
        report_path
    ) 