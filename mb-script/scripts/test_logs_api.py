#!/usr/bin/env python3
"""
Test script for the ESP32 Modbus REST API /system/logs and /system/log-level endpoints.
"""

import requests
import json
import argparse
import sys
from colorama import init, Fore, Style

# Initialize colorama
init()

# Valid log levels
VALID_LOG_LEVELS = ["none", "error", "warning", "notice", "trace", "verbose"]

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
        # Attempt to pretty-print JSON if possible
        data = response.json()
        if isinstance(data, list):
            print(f"{Fore.CYAN}Response (JSON Array, {len(data)} items):{Style.RESET_ALL}")
            # Print first few and last few lines if too long
            limit = 15 
            if len(data) > 2 * limit:
                for i in range(limit):
                    print(f"  {data[i]}")
                print(f"  ... ({len(data) - 2 * limit} more lines) ...")
                for i in range(len(data) - limit, len(data)):
                     print(f"  {data[i]}")
            else:
                 for line in data:
                    print(f"  {line}")
        else:
            formatted_json = json.dumps(data, indent=2)
            print(f"{Fore.CYAN}Response (JSON Object): {formatted_json}{Style.RESET_ALL}")
            
    except Exception as e:
        print(f"{Fore.CYAN}Response (raw): {response.text[:500]}...{Style.RESET_ALL}") # Limit raw output
        print(f"{Fore.YELLOW}  Could not parse JSON: {e}{Style.RESET_ALL}")

def test_get_logs(base_url):
    api_url = f"{base_url}/api/v1/system/logs"
    fail_count = 0

    # Test GET logs without level parameter
    print_info(f"Testing GET {api_url}")
    try:
        response = requests.get(api_url, timeout=10)
        print_response(response)

        if response.status_code == 200:
            try:
                data = response.json()
                if isinstance(data, list):
                    print_success(f"Logs endpoint returned a list with {len(data)} items.")
                    # Optional: Check if items are strings
                    if data and not isinstance(data[0], str):
                        print_fail("Log items do not appear to be strings.")
                        fail_count += 1
                else:
                    print_fail("Response is not a JSON list as expected.")
                    fail_count += 1

            except json.JSONDecodeError as e:
                print_fail("Response is not valid JSON.", str(e))
                fail_count += 1
            except Exception as e:
                print_fail("Error processing JSON response.", str(e))
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

    # Test GET logs with level parameter
    print_info(f"Testing GET {api_url} with level parameter")
    for level in VALID_LOG_LEVELS:
        print_info(f"Testing GET {api_url}?level={level}")
        try:
            response = requests.get(f"{api_url}?level={level}", timeout=10)
            print_response(response)

            if response.status_code == 200:
                try:
                    data = response.json()
                    if isinstance(data, list):
                        print_success(f"Logs endpoint with level={level} returned a list with {len(data)} items.")
                        # Optional: Check if items are strings
                        if data and not isinstance(data[0], str):
                            print_fail("Log items do not appear to be strings.")
                            fail_count += 1
                    else:
                        print_fail("Response is not a JSON list as expected.")
                        fail_count += 1

                except json.JSONDecodeError as e:
                    print_fail("Response is not valid JSON.", str(e))
                    fail_count += 1
                except Exception as e:
                    print_fail("Error processing JSON response.", str(e))
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

    # Test GET logs with invalid level parameter
    print_info(f"Testing GET {api_url} with invalid level parameter")
    try:
        response = requests.get(f"{api_url}?level=invalid", timeout=10)
        print_response(response)

        if response.status_code == 400:
            print_success("Server correctly rejected invalid log level")
        else:
            print_fail(f"Server did not reject invalid log level (got {response.status_code}, expected 400)")
            fail_count += 1

    except requests.exceptions.RequestException as e:
        print_fail("Failed to connect to the endpoint.", str(e))
        fail_count += 1
    except Exception as e:
        print_fail("An unexpected error occurred.", str(e))
        fail_count += 1
    
    return fail_count

def test_log_level(base_url):
    api_url = f"{base_url}/api/v1/system/log-level"
    fail_count = 0

    # Test GET log level
    print_info(f"Testing GET {api_url}")
    try:
        response = requests.get(api_url, timeout=10)
        print_response(response)

        if response.status_code == 200:
            try:
                data = response.json()
                if "level" in data and data["level"] in VALID_LOG_LEVELS:
                    print_success(f"Current log level is '{data['level']}'")
                    initial_level = data["level"]
                else:
                    print_fail("Response missing 'level' field or invalid level value")
                    fail_count += 1
            except json.JSONDecodeError as e:
                print_fail("Response is not valid JSON.", str(e))
                fail_count += 1
        else:
            print_fail(f"GET endpoint returned status code {response.status_code}")
            fail_count += 1

        # Test PUT log level - try each valid level
        print_info(f"Testing GET {api_url} to set different levels")
        for level in VALID_LOG_LEVELS:
            print_info(f"Setting log level to '{level}'")
            try:
                response = requests.get(f"{api_url}?level={level}", timeout=10)
                print_response(response)

                if response.status_code == 200:
                    try:
                        data = response.json()
                        if "success" in data and data["success"] and "level" in data and data["level"] == level:
                            print_success(f"Successfully set log level to '{level}'")
                        else:
                            print_fail(f"Failed to set log level to '{level}'")
                            fail_count += 1
                    except json.JSONDecodeError as e:
                        print_fail("Response is not valid JSON.", str(e))
                        fail_count += 1
                else:
                    print_fail(f"GET endpoint returned status code {response.status_code}")
                    fail_count += 1

            except requests.exceptions.RequestException as e:
                print_fail(f"Failed to set log level to '{level}'.", str(e))
                fail_count += 1

        # Test invalid log level
        print_info("Testing GET with invalid log level")
        try:
            response = requests.get(f"{api_url}?level=invalid", timeout=10)
            print_response(response)

            if response.status_code == 400:
                print_success("Server correctly rejected invalid log level")
            else:
                print_fail(f"Server did not reject invalid log level (got {response.status_code}, expected 400)")
                fail_count += 1
        except requests.exceptions.RequestException as e:
            print_fail("Failed to test invalid log level.", str(e))
            fail_count += 1

        # Restore initial log level
        print_info(f"Restoring initial log level '{initial_level}'")
        try:
            response = requests.get(f"{api_url}?level={initial_level}", timeout=10)
            if response.status_code == 200:
                print_success(f"Restored log level to '{initial_level}'")
            else:
                print_fail(f"Failed to restore initial log level")
                fail_count += 1
        except requests.exceptions.RequestException as e:
            print_fail("Failed to restore initial log level.", str(e))
            fail_count += 1

    except requests.exceptions.RequestException as e:
        print_fail("Failed to connect to the endpoint.", str(e))
        fail_count += 1
    except Exception as e:
         print_fail("An unexpected error occurred.", str(e))
         fail_count += 1

    return fail_count

def main():
    parser = argparse.ArgumentParser(description='Test the ESP32 Modbus REST API /system/logs and /system/log-level endpoints')
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

    fail_count = 0

    # Test logs endpoint
    print("-" * 80)
    print_info("Testing system logs endpoint")
    fail_count += test_get_logs(base_url)

    # Test log level endpoint
    print("-" * 80)
    print_info("Testing log level endpoint")
    fail_count += test_log_level(base_url)

    print("-" * 80)
    if fail_count == 0:
        print(f"{Fore.GREEN}✓ All tests passed!{Style.RESET_ALL}")
    else:
        print(f"{Fore.RED}✗ {fail_count} check(s) failed.{Style.RESET_ALL}")

    sys.exit(fail_count)

if __name__ == "__main__":
    main() 