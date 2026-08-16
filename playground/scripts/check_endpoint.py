#!/usr/bin/env python3
"""
Simple script to check a specific REST API endpoint
"""

import requests
import json
import sys

def main():
    if len(sys.argv) < 3:
        print("Usage: python check_endpoint.py <url> <method>")
        print("Example: python check_endpoint.py http://192.168.1.250/api/v1/coils/30 GET")
        sys.exit(1)
    
    url = sys.argv[1]
    method = sys.argv[2].upper()
    
    print(f"Checking {method} {url}")
    
    try:
        if method == "GET":
            response = requests.get(url, timeout=5)
        elif method == "POST":
            data = {'value': True}
            response = requests.post(url, json=data, timeout=5)
        else:
            print(f"Unsupported method: {method}")
            sys.exit(1)
        
        print(f"Status code: {response.status_code}")
        if response.status_code != 404:
            try:
                print(f"Response: {json.dumps(response.json(), indent=2)}")
            except:
                print(f"Response: {response.text}")
    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    main() 