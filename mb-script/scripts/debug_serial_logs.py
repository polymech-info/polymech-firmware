import requests
import argparse
import json
import time

def fetch_logs(host):
    """Fetches logs from the device's REST API."""
    url = f"http://{host}/api/v1/system/logs"
    try:
        response = requests.get(url, timeout=10) # Add a timeout
        response.raise_for_status() # Raise an exception for bad status codes (4xx or 5xx)
        return response.json()
    except requests.exceptions.ConnectionError:
        print(f"Error: Could not connect to {url}. Is the device running and on the network?")
    except requests.exceptions.Timeout:
        print(f"Error: Request timed out connecting to {url}.")
    except requests.exceptions.HTTPError as http_err:
        print(f"HTTP error occurred: {http_err} - {response.status_code} {response.reason}")
        try:
            # Try to print the error message from the API if available
            error_details = response.json()
            print(f"API Error Details: {error_details}")
        except json.JSONDecodeError:
            print(f"Could not parse error response: {response.text}")
    except requests.exceptions.RequestException as err:
        print(f"An unexpected error occurred: {err}")
    except json.JSONDecodeError:
        print(f"Error: Could not decode JSON response from {url}.")
        print(f"Raw response: {response.text[:200]}...") # Print beginning of raw response
    return None

def main():
    parser = argparse.ArgumentParser(description='Fetch and print logs from the device REST API.')
    parser.add_argument('--host', default='modbus-esp32.local', help='Hostname or IP address of the device (default: modbus-esp32.local)')
    args = parser.parse_args()

    print(f"Attempting to fetch logs from {args.host}...")
    logs = fetch_logs(args.host)

    if logs is not None:
        if isinstance(logs, list):
            if not logs:
                print("No logs received from the device.")
            else:
                print("--- Received Logs ---")
                for line in logs:
                    print(line)
                print("---------------------")
        else:
            print("Error: Received unexpected data format (expected a JSON list).")
            print(f"Received: {logs}")

if __name__ == "__main__":
    main() 