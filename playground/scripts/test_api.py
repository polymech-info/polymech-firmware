#!/usr/bin/env python3
"""
Test script for the ESP32 Modbus REST API
This script tests all endpoints of the REST API and reports the results.
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
        self.api_url = f"{base_url}/api/v1"
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
        
        # Test system info endpoint
        self.test_system_info()
        print("-" * 80)
        
        # Test coil endpoints
        self.test_coils_list()
        print("-" * 80)
        self.test_coil_get(30)  # Test relay coil 0
        print("-" * 80)
        self.test_coil_toggle(30)  # Toggle relay coil 0
        print("-" * 80)
        
        # Test register endpoints
        self.test_registers_list()
        print("-" * 80)
        self.test_register_get(20)  # Test battle counter register
        print("-" * 80)
        self.test_register_update(20, 42)  # Update battle counter register
        print("-" * 80)
        
        # Test relay test endpoint
        self.test_relay_test()
        print("=" * 80)
        
        # Print summary
        print(f"\nTest Summary: {self.success_count} passed, {self.fail_count} failed")
        
        return self.fail_count == 0
    
    def test_system_info(self):
        """Test the system info endpoint"""
        self.print_info("Testing GET /system/info")
        try:
            response = requests.get(f"{self.api_url}/system/info", timeout=5)
            if response.status_code == 200:
                data = response.json()
                self.print_response(response)
                if 'version' in data and 'board' in data and 'uptime' in data:
                    self.print_success("System info endpoint returned valid data")
                else:
                    self.print_fail("System info endpoint returned incomplete data")
            else:
                self.print_fail(f"System info endpoint returned status code {response.status_code}")
        except Exception as e:
            self.print_fail("Failed to connect to system info endpoint", str(e))
    
    def test_coils_list(self):
        """Test the coils list endpoint"""
        self.print_info("Testing GET /coils")
        try:
            response = requests.get(f"{self.api_url}/coils?start=0&count=10", timeout=5)
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
        self.print_info(f"Testing GET /coils?address={address}")
        try:
            response = requests.get(f"{self.api_url}/coils", params={"address": address}, timeout=5)
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
    
    def test_coil_toggle(self, address):
        """Test toggling a coil"""
        self.print_info(f"Testing POST /coils/{address}")
        
        # First, get the current value
        try:
            get_response = requests.get(f"{self.api_url}/coils", params={"address": address}, timeout=5)
            if get_response.status_code == 200:
                current_value = get_response.json().get('value', False)
                new_value = not current_value
                
                # Now toggle the value
                try:
                    self.print_info(f"Setting coil {address} to {new_value}")
                    post_response = requests.post(
                        f"{self.api_url}/coils/{address}",
                        json={"value": new_value},
                        timeout=5
                    )
                    if post_response.status_code == 200:
                        data = post_response.json()
                        self.print_response(post_response)
                        if ('success' in data and data['success'] and 
                            'address' in data and data['address'] == address and
                            'value' in data and data['value'] == new_value):
                            self.print_success(f"Successfully toggled coil {address}")
                            
                            # Toggle back to original state to be nice
                            time.sleep(1)
                            requests.post(
                                f"{self.api_url}/coils/{address}",
                                json={"value": current_value},
                                timeout=5
                            )
                            self.print_info(f"Reset coil {address} to original state")
                        else:
                            self.print_fail(f"Failed to toggle coil {address}")
                    else:
                        self.print_fail(f"Coil toggle endpoint returned status code {post_response.status_code}")
                except Exception as e:
                    self.print_fail(f"Failed to toggle coil {address}", str(e))
            else:
                self.print_fail(f"Failed to get current coil state: {get_response.status_code}")
        except Exception as e:
            self.print_fail(f"Failed to get current coil state", str(e))
    
    def test_registers_list(self):
        """Test the registers list endpoint"""
        self.print_info("Testing GET /registers")
        try:
            response = requests.get(f"{self.api_url}/registers?start=0&count=10", timeout=5)
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
        self.print_info(f"Testing GET /registers?address={address}")
        try:
            response = requests.get(f"{self.api_url}/registers", params={"address": address}, timeout=5)
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
    
    def test_register_update(self, address, new_value):
        """Test updating a register"""
        self.print_info(f"Testing POST /registers/{address}")
        
        # First, get the current value
        try:
            get_response = requests.get(f"{self.api_url}/registers", params={"address": address}, timeout=5)
            if get_response.status_code == 200:
                current_value = get_response.json().get('value', 0)
                
                # Now update the value
                try:
                    self.print_info(f"Setting register {address} to {new_value}")
                    post_response = requests.post(
                        f"{self.api_url}/registers/{address}",
                        params={"value": new_value},
                        timeout=5
                    )
                    if post_response.status_code == 200:
                        data = post_response.json()
                        self.print_response(post_response)
                        if ('success' in data and data['success'] and 
                            'address' in data and data['address'] == address and
                            'value' in data and data['value'] == new_value):
                            self.print_success(f"Successfully updated register {address}")
                            
                            # Restore original value to be nice
                            time.sleep(1)
                            requests.post(
                                f"{self.api_url}/registers/{address}",
                                params={"value": current_value},
                                timeout=5
                            )
                            self.print_info(f"Reset register {address} to original value")
                        else:
                            self.print_fail(f"Failed to update register {address}")
                    else:
                        self.print_fail(f"Register update endpoint returned status code {post_response.status_code}")
                except Exception as e:
                    self.print_fail(f"Failed to update register {address}", str(e))
            else:
                self.print_fail(f"Failed to get current register value: {get_response.status_code}")
        except Exception as e:
            self.print_fail(f"Failed to get current register value", str(e))
    
    def test_relay_test(self):
        """Test the relay test endpoint"""
        self.print_info("Testing POST /relay/test")
        try:
            response = requests.post(f"{self.api_url}/relay/test", timeout=5)
            if response.status_code == 200:
                data = response.json()
                self.print_response(response)
                if 'success' in data and 'message' in data:
                    self.print_success("Relay test endpoint returned valid data")
                else:
                    self.print_fail("Relay test endpoint returned incomplete data")
            else:
                self.print_fail(f"Relay test endpoint returned status code {response.status_code}")
        except Exception as e:
            self.print_fail("Failed to connect to relay test endpoint", str(e))

def main():
    parser = argparse.ArgumentParser(description='Test the ESP32 Modbus REST API')
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
    
    tester = ApiTester(base_url)
    success = tester.run_tests()
    
    # Return non-zero exit code if any tests failed
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main() 