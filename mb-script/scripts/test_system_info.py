#!/usr/bin/env python3
"""
Test script for the ESP32 Modbus REST API /system/info endpoint.
"""

import requests
import json
import argparse
import sys
from colorama import init, Fore, Style

# Initialize colorama
init()

def print_success(message):
    print(f"{Fore.GREEN}✓ SUCCESS: {message}{Style.RESET_ALL}")

def print_fail(message, error=None):
    print(f"{Fore.RED}✗ FAIL: {message}{Style.RESET_ALL}")
    if error:
        print(f"  {Fore.YELLOW}Error: {error}{Style.RESET_ALL}")

def print_info(message):
    print(f"{Fore.BLUE}ℹ INFO: {message}{Style.RESET_ALL}")

def print_response(response):
    try:
        formatted_json = json.dumps(response.json(), indent=2)
        print(f"{Fore.CYAN}Response: {formatted_json}{Style.RESET_ALL}")
    except Exception as e:
        print(f"{Fore.CYAN}Response (raw): {response.text}{Style.RESET_ALL}")
        print(f"{Fore.YELLOW}  Could not parse JSON: {e}{Style.RESET_ALL}")

def main():
    parser = argparse.ArgumentParser(description='Test the ESP32 Modbus REST API /system/info endpoint')
    parser.add_argument('--host', type=str, default='modbus-esp32.local',
                      help='Hostname or IP address of the ESP32 device (default: modbus-esp32.local)')
    parser.add_argument('--port', type=int, default=80,
                      help='Port number (default: 80)')
    parser.add_argument('--protocol', type=str, default='http',
                      choices=['http', 'https'],
                      help='Protocol to use (default: http)')

    args = parser.parse_args()

    base_url = f"{args.protocol}://{args.host}"
    if args.port != 80:
        base_url += f":{args.port}"

    api_url = f"{base_url}/api/v1/system/info"
    fail_count = 0

    print_info(f"Testing GET {api_url}")
    try:
        response = requests.get(api_url, timeout=10) # Increased timeout slightly
        print_response(response)

        if response.status_code == 200:
            try:
                data = response.json()
                # Basic checks
                if not all(k in data for k in ["version", "board", "uptime", "timestamp"]):
                    print_fail("System info missing required base fields.")
                    fail_count += 1
                
                # Check components array
                if "components" in data:
                    if isinstance(data["components"], list):
                        print_success("System info includes 'components' array.")
                        if not data["components"]:
                             print_info("Component array is present but empty (no Modbus components registered?).")
                        else:
                            # Check structure of the first component if array is not empty
                            first_comp = data["components"][0]
                            if not all(k in first_comp for k in ["id", "name", "startAddress", "count"]):
                                print_fail("First component in array has incorrect structure.")
                                fail_count += 1
                            else:
                                print_success("Component structure looks correct.")
                    else:
                        print_fail("'components' field is not a list.")
                        fail_count += 1
                else:
                    # This might be acceptable if ModbusManager isn't active
                    print_info("System info does not include 'components' array (ModbusManager inactive or no components?).")

                # If all checks passed so far
                if fail_count == 0:
                     print_success("System info response structure is valid.")

            except json.JSONDecodeError as e:
                print_fail("Response is not valid JSON.", str(e))
                fail_count += 1
        else:
            print_fail(f"Endpoint returned status code {response.status_code}")
            fail_count += 1

    except requests.exceptions.RequestException as e:
        print_fail("Failed to connect to the endpoint.", str(e))
        fail_count += 1
    except Exception as e:
         print_fail("An unexpected error occurred.", str(e))
         fail_count += 1

    print("-" * 80)
    if fail_count == 0:
        print(f"{Fore.GREEN}✓ All checks passed!{Style.RESET_ALL}")
    else:
        print(f"{Fore.RED}✗ {fail_count} check(s) failed.{Style.RESET_ALL}")

    sys.exit(fail_count)

if __name__ == "__main__":
    main() 