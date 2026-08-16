#!/usr/bin/env python3
"""
Test script for the ESP32 Modbus SimpleWebServer API
This script tests the actual working endpoints.
"""

import requests
import json
import argparse
import time
import sys
from colorama import init, Fore, Style

# Initialize colorama for colored terminal output
init()

class ApiTester:
    def __init__(self, base_url):
        self.base_url = base_url
        self.api_url = f"{base_url}/api/v1"  # Add v1 prefix
        self.success_count = 0
        self.fail_count = 0
        
    def print_success(self, message):
        print(f"{Fore.GREEN}✓ SUCCESS: {message}{Style.RESET_ALL}")
        self.success_count += 1
        
    def print_fail(self, message, error=None):
        print(f"{Fore.RED}✗ FAIL: {message}{Style.RESET_ALL}")
        if error:
            print(f"  {Fore.YELLOW}Error: {error}{Style.RESET_ALL}")
        self.fail_count += 1
    
    def print_info(self, message):
        print(f"{Fore.BLUE}ℹ INFO: {message}{Style.RESET_ALL}")
    
    def print_response(self, response):
        try:
            formatted_json = json.dumps(response.json(), indent=2)
            print(f"{Fore.CYAN}Response: {formatted_json}{Style.RESET_ALL}")
        except:
            print(f"{Fore.CYAN}Response: {response.text}{Style.RESET_ALL}")
    
    def run_tests(self):
        """Run all API tests"""
        self.print_info(f"Testing API at {self.api_url}")
        print("=" * 80)
        
        # Test coil endpoints - update to V1
        self.test_coils_list()  # GET /api/v1/coils
        print("-" * 80)
        self.test_coil_get(21)  # GET /api/v1/coils?address=21
        print("-" * 80)
        self.test_coil_set(51)  # Test toggle coil via POST /api/v1/coils/51?value=...
        print("-" * 80)
        
        # Test register endpoints - update to V1
        self.test_registers_list()  # GET /api/v1/registers
        print("-" * 80)
        self.test_register_get(20)  # GET /api/v1/registers?address=20
        print("-" * 80)
        self.test_register_set(20, 42)  # POST /api/v1/registers/20?value=42
        print("=" * 80)
        
        # Print summary
        print(f"\nTest Summary: {self.success_count} passed, {self.fail_count} failed")
        
        return self.fail_count == 0
    
    def test_coils_list(self):
        """Test the coils list endpoint"""
        self.print_info("Testing GET /v1/coils")
        try:
            # Updated URL, remove unused query params
            response = requests.get(f"{self.api_url}/coils", timeout=5)
            if response.status_code == 200:
                data = response.json()
                self.print_response(response)
                if 'coils' in data and isinstance(data['coils'], list):
                    self.print_success("Coils endpoint returned valid data")
                else:
                    self.print_fail("Coils endpoint returned invalid data")
            else:
                self.print_fail(f"Coils endpoint returned status code {response.status_code}")
        except Exception as e:
            self.print_fail("Failed to connect to coils endpoint", str(e))
    
    def test_coil_get(self, address):
        """Test getting a specific coil"""
        self.print_info(f"Testing GET /v1/coils?address={address}")
        try:
            # Updated URL and use query parameter
            response = requests.get(f"{self.api_url}/coils", params={'address': address}, timeout=5)
            if response.status_code == 200:
                data = response.json()
                self.print_response(response)
                if 'address' in data and 'value' in data:
                    self.print_success(f"Coil {address} endpoint returned valid data")
                else:
                    self.print_fail(f"Coil {address} endpoint returned incomplete data")
            else:
                self.print_fail(f"Coil {address} endpoint returned status code {response.status_code}")
        except Exception as e:
            self.print_fail(f"Failed to connect to coil {address} endpoint", str(e))
    
    # Renamed from test_coil_toggle to test_coil_set as per swagger
    def test_coil_set(self, address):
        """Test setting a coil"""
        self.print_info(f"Testing POST /v1/coils/{address}")
        
        # First, get the current value
        try:
            # Updated URL and use query parameter
            get_response = requests.get(f"{self.api_url}/coils", params={'address': address}, timeout=5)
            if get_response.status_code == 200:
                current_value = get_response.json().get('value', False)
                new_value = not current_value
                
                # Now set the value
                try:
                    self.print_info(f"Setting coil {address} to {new_value}")
                    # Updated URL structure and use query param for value
                    post_response = requests.post(
                        f"{self.api_url}/coils/{address}",
                        params={'value': new_value},
                        timeout=5
                    )
                    if post_response.status_code == 200:
                        data = post_response.json()
                        self.print_response(post_response)
                        # Check response format according to swagger
                        if ('success' in data and data['success'] and 
                            'address' in data and data['address'] == address and
                            'value' in data and data['value'] == new_value):
                            self.print_success(f"Successfully set coil {address}")
                            
                            # Restore original state to be nice
                            time.sleep(1)
                            requests.post(
                                f"{self.api_url}/coils/{address}",
                                params={'value': current_value},
                                timeout=5
                            )
                            self.print_info(f"Reset coil {address} to original state")
                        else:
                            self.print_fail(f"Failed to set coil {address}, unexpected response format")
                    else:
                        self.print_fail(f"Coil set endpoint returned status code {post_response.status_code}")
                except Exception as e:
                    self.print_fail(f"Failed to set coil {address}", str(e))
            else:
                self.print_fail(f"Failed to get current coil state: {get_response.status_code}")
        except Exception as e:
            self.print_fail(f"Failed to get current coil state", str(e))
    
    def test_registers_list(self):
        """Test the registers list endpoint"""
        self.print_info("Testing GET /v1/registers")
        try:
            # Updated URL, remove unused query params
            response = requests.get(f"{self.api_url}/registers", timeout=5)
            if response.status_code == 200:
                data = response.json()
                self.print_response(response)
                if 'registers' in data and isinstance(data['registers'], list):
                    self.print_success("Registers endpoint returned valid data")
                else:
                    self.print_fail("Registers endpoint returned invalid data")
            else:
                self.print_fail(f"Registers endpoint returned status code {response.status_code}")
        except Exception as e:
            self.print_fail("Failed to connect to registers endpoint", str(e))
    
    def test_register_get(self, address):
        """Test getting a specific register"""
        self.print_info(f"Testing GET /v1/registers?address={address}")
        try:
            # Updated URL and use query parameter
            response = requests.get(f"{self.api_url}/registers", params={'address': address}, timeout=5)
            if response.status_code == 200:
                data = response.json()
                self.print_response(response)
                if 'address' in data and 'value' in data:
                    self.print_success(f"Register {address} endpoint returned valid data")
                else:
                    self.print_fail(f"Register {address} endpoint returned incomplete data")
            else:
                self.print_fail(f"Register {address} endpoint returned status code {response.status_code}")
        except Exception as e:
            self.print_fail(f"Failed to connect to register {address} endpoint", str(e))
    
    # Renamed from test_register_update to test_register_set
    def test_register_set(self, address, new_value):
        """Test setting a register"""
        self.print_info(f"Testing POST /v1/registers/{address}")
        
        # First, get the current value
        try:
            # Updated URL and use query parameter
            get_response = requests.get(f"{self.api_url}/registers", params={'address': address}, timeout=5)
            if get_response.status_code == 200:
                current_value = get_response.json().get('value', 0)
                
                # Now set the value
                try:
                    self.print_info(f"Setting register {address} to {new_value}")
                    # Updated URL structure and use query param for value
                    post_response = requests.post(
                        f"{self.api_url}/registers/{address}",
                        params={'value': new_value},
                        timeout=5
                    )
                    if post_response.status_code == 200:
                        data = post_response.json()
                        self.print_response(post_response)
                        # Check response format according to swagger
                        if ('success' in data and data['success'] and 
                            'address' in data and data['address'] == address and
                            'value' in data and data['value'] == new_value):
                            self.print_success(f"Successfully set register {address}")
                            
                            # Restore original value to be nice
                            time.sleep(1)
                            requests.post(
                                f"{self.api_url}/registers/{address}",
                                params={'value': current_value},
                                timeout=5
                            )
                            self.print_info(f"Reset register {address} to original value")
                        else:
                            self.print_fail(f"Failed to set register {address}, unexpected response format")
                    else:
                        self.print_fail(f"Register set endpoint returned status code {post_response.status_code}")
                except Exception as e:
                    self.print_fail(f"Failed to set register {address}", str(e))
            else:
                self.print_fail(f"Failed to get current register value: {get_response.status_code}")
        except Exception as e:
            self.print_fail(f"Failed to get current register value", str(e))

def main():
    parser = argparse.ArgumentParser(description='Test the ESP32 Modbus SimpleWebServer API')
    parser.add_argument('--host', type=str, default='192.168.1.250',
                      help='Hostname or IP address of the ESP32 device (default: 192.168.1.250)')
    parser.add_argument('--port', type=int, default=80,
                      help='Port number (default: 80)')
    parser.add_argument('--protocol', type=str, default='http',
                      choices=['http', 'https'],
                      help='Protocol to use (default: http)')
    
    args = parser.parse_args()
    
    base_url = f"{args.protocol}://{args.host}"
    if args.port != 80:
        base_url += f":{args.port}"
    
    tester = ApiTester(base_url)
    success = tester.run_tests()
    
    # Return non-zero exit code if any tests failed
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main() 