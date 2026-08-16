import requests
import json
import os
import sys
import argparse

# Default device identifier (mDNS or IP)
DEFAULT_DEVICE_ID = "modbus-esp32.local"

# API endpoint
API_ENDPOINT = "/api/v1/system/boot-metrics"

# Output directory
OUTPUT_DIR = "./tmp"
OUTPUT_FILE = "boot_metrics.json"

def get_device_url(device_id):
    """Constructs the full URL for the API endpoint."""
    if not device_id.startswith(("http://", "https://")):
        device_id = f"http://{device_id}"
    return f"{device_id}{API_ENDPOINT}"

def fetch_and_save_metrics(device_url):
    """Fetches boot and current metrics from the device and saves them to a JSON file."""
    try:
        print(f"Attempting to fetch metrics from: {device_url}")
        response = requests.get(device_url, timeout=10) # 10 second timeout
        response.raise_for_status() # Raise an exception for bad status codes (4xx or 5xx)

        metrics_data = response.json()
        print("Successfully fetched metrics:")
        # Print initial and current metrics
        print(json.dumps(metrics_data, indent=2))

        # Ensure the output directory exists
        os.makedirs(OUTPUT_DIR, exist_ok=True)

        output_path = os.path.join(OUTPUT_DIR, OUTPUT_FILE)

        # Save the data to a JSON file
        with open(output_path, 'w') as f:
            json.dump(metrics_data, f, indent=4)

        print(f"Metrics saved to: {output_path}")

    except requests.exceptions.ConnectionError as e:
        print(f"Error: Could not connect to the device at {device_url}.", file=sys.stderr)
        print(f"Details: {e}", file=sys.stderr)
        sys.exit(1)
    except requests.exceptions.Timeout:
        print(f"Error: Request timed out while connecting to {device_url}.", file=sys.stderr)
        sys.exit(1)
    except requests.exceptions.RequestException as e:
        print(f"Error: An error occurred during the request to {device_url}.", file=sys.stderr)
        print(f"Details: {e}", file=sys.stderr)
        sys.exit(1)
    except json.JSONDecodeError:
        print(f"Error: Failed to decode JSON response from {device_url}.", file=sys.stderr)
        print(f"Received content: {response.text}", file=sys.stderr)
        sys.exit(1)
    except OSError as e:
        print(f"Error: Could not create directory or write file.", file=sys.stderr)
        print(f"Details: {e}", file=sys.stderr)
        sys.exit(1)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Fetch boot and current metrics from ESP32 device via REST API.")
    parser.add_argument(
        "--ip",
        type=str,
        default=DEFAULT_DEVICE_ID,
        help=f"IP address or mDNS name of the device (default: {DEFAULT_DEVICE_ID})"
    )
    args = parser.parse_args()

    url = get_device_url(args.ip)
    fetch_and_save_metrics(url) 