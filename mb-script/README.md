# Polymech Cassandra Firmware

This repository contains the firmware for the Polymech Cassandra project, running on an ESP32-S3.

## Overview

The firmware manages various hardware components and provides multiple interfaces for control and monitoring:

*   **Serial Interface:** For debugging, testing, and direct command execution.
*   **Modbus TCP Server:** Allows interaction with components using the Modbus protocol over WiFi.
*   **REST API & Web UI:** Provides a web-based interface (status page, API documentation) and a RESTful API for interacting with the system over WiFi.

## Architecture

The firmware is built upon a component-based architecture:

*   **`App` / `PHApp`:** The main application class (`src/PHApp.h`).
*   **`Component`:** Base class (`src/Component.h`) for all functional units.
*   **`Bridge`:** (`src/Bridge.h`) Facilitates serial command dispatch.
*   **`SerialMessage`:** (`src/SerialMessage.h`) Handles serial communication.
*   **`ModbusManager`:** (`src/ModbusManager.h`) Manages the Modbus TCP server.
*   **`RESTServer`:** (`src/RestServer.h`) Implements the web server and REST API.
*   **Details:** See [docs/components.md](./docs/components.md) for how components are structured, configured, and added.

## Interfaces

### 1. Serial Communication

A command-line interface is available over the USB serial port used for programming and monitoring.

*   **Purpose:** Debugging, direct method calls on components.
*   **Command Format:** `<<component_id;call_type;flags;payload:arg1:arg2...>>`
*   **Examples & Details:** See [docs/serial.md](./docs/serial.md)
*   **Sending Commands:** Use `npm run send -- "<command_string>"` or a standard serial terminal.

### 2. Modbus TCP

The device runs a Modbus TCP server (slave/responder) on port 502.

*   **Implementation:** Managed by `ModbusManager` using the `eModbus` library.
*   **Component Interaction:** Components supporting Modbus (like `Relay` or `PHApp` itself) implement `readNetworkValue` and `writeNetworkValue` methods. They are registered with the `ModbusManager` along with their corresponding Modbus addresses.
*   **Configuration:** Modbus addresses are defined in `src/config-modbus.h`.
*   **Design Details:** See [docs/design-network.md](./docs/design-network.md)
*   **Testing:** Use the `python scripts/modbus_*.py` scripts directly (npm script argument passing has issues):
    ```bash
    # Read Coil 51
    python scripts/modbus_read_coils.py --address 51 --ip-address <DEVICE_IP>
    # Write Coil 51 ON
    python scripts/modbus_write_coil.py --address 51 --value 1 --ip-address <DEVICE_IP>
    # Read Register 20
    python scripts/modbus_read_registers.py --address 20 --ip-address <DEVICE_IP>
    # Write Register 20
    python scripts/modbus_write_register.py --address 20 --value 123 --ip-address <DEVICE_IP>
    ```

### 3. Web Interface & REST API

A web server runs on port 80, providing:

*   **Status Page:** `http://<DEVICE_IP>/`
*   **Swagger UI:** `http://<DEVICE_IP>/api-docs` (Interactive API documentation)
*   **REST API:** `http://<DEVICE_IP>/api/v1/...`
*   **Working Endpoints:**
    *   `GET /api/v1/system/info`: Device status.
    *   `GET /api/v1/coils`: List of registered coils.
    *   `GET /api/v1/registers`: List of registered registers.
    *   `GET /api/v1/coils?address=<addr>`: Read a specific registered coil.
    *   `GET /api/v1/registers?address=<addr>`: Read a specific registered register.
    *   `POST /api/v1/relay/test`: Trigger internal relay test.
*   **Non-Functional Endpoints (Known Issue):**
    *   `POST /coils/{address}`: Returns 404.
    *   `POST /registers/{address}`: Returns 404.
*   **Details & Examples:** See [docs/web.md](./docs/web.md)
*   **Testing:** Use `npm run test-api-ip` (note that some tests related to POST endpoints will fail).

## Building and Development

See [firmware/.cursor/rules](./.cursor/rules) for a summary of common workflow commands.

**Key `npm` Scripts:**

*   `npm run build`: Compile the firmware.
*   `npm run upload`: Upload the firmware to the device.
*   `npm run clean`: Clean build artifacts.
*   `npm run monitor`: Open the serial monitor.
*   `npm run build-web`: Generate web assets (from `swagger.yaml`, `front/`) and build firmware.
*   `npm run send -- "<command>"`: Send a serial command.
*   `npm run test-api-ip`: Run the REST API test script against the device IP.
*   *(Modbus tests need direct python execution, see Modbus section above)*

## Configuration

*   **Hardware Pins, Features:** `src/config.h`, `src/config_adv.h`
*   **WiFi Credentials:** `src/config_secrets.h` (ensure this file exists and is populated)
*   **Modbus Addresses:** `src/config-modbus.h`
*   **Component IDs:** `src/enums.h` (enum `COMPONENT_KEY`) 