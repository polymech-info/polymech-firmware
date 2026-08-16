# LLM Development Workflow Commands

This document outlines the common `npm` commands used during development and testing of the Cassandra firmware, categorized by their function.

## Application Architecture and Adding New Components

The firmware application is primarily managed by the `PHApp` class ([`src/PHApp.h`](../src/PHApp.h), [`src/PHApp.cpp`](../src/PHApp.cpp)). It acts as the central coordinator for various hardware interactions and communication protocols.

**Key Concepts:**

*   **`PHApp`**: The main application class inheriting from `App`. It initializes and manages all other functional units (Components).
*   **`Component`**: A base class ([`src/Component.h`](../src/Component.h)) for modular hardware abstractions (e.g., relays, sensors, potentiometers). Each specific component type (like `Relay`, `POT`) inherits from `Component`.
*   **`ModbusManager`**: Handles Modbus communication ([`src/ModbusManager.h`](../src/ModbusManager.h)). `PHApp` creates and owns the `ModbusManager` instance.
*   **Component Registration**: Components that need to expose data via Modbus are registered with the `ModbusManager` instance within `PHApp`. This typically happens in `PHApp::setup()` or `PHApp::setupModbus()`.
*   **Network Values**: Components implement `readNetworkValue(address)` and `writeNetworkValue(address, value)` methods. The `ModbusManager` calls these methods when Modbus requests target the addresses registered for that component.

**Steps to Add a New Component:**

1.  **Define Modbus Addresses:** Add necessary Modbus register/coil addresses for your new component in [`src/config-modbus.h`](../src/config-modbus.h).
2.  **Create Component Class:**
    *   Create new header (`.h`) and source (`.cpp`) files for your component (e.g., [`src/MyNewSensor.h`](../src/MyNewSensor.h), [`src/MyNewSensor.cpp`](../src/MyNewSensor.cpp)).
    *   In the header, define a class that inherits from `Component`.
    *   Include necessary headers (e.g., `Component.h`).
    *   Declare constructor, `setup()`, `loop()`, `readNetworkValue()`, `writeNetworkValue()`, and any other required methods.
3.  **Implement Component Logic:**
    *   In the source file (`.cpp`), implement the methods declared in the header.
    *   `setup()`: Initialize hardware, pins, etc.
    *   `loop()`: Read sensor values, update internal state, etc.
    *   `readNetworkValue()`: Return the component's state based on the requested Modbus `address`.
    *   `writeNetworkValue()`: Update the component's state based on the requested Modbus `address` and `value`. Return `E_OK` on success or an error code (from [`enums.h`](../src/enums.h)) on failure.
4.  **Integrate into `PHApp`:**
    *   In `PHApp.h`, include the header for your new component.
    *   In `PHApp.h`, declare a pointer to your new component class as a member variable (e.g., `MyNewSensor* mySensor;`).
    *   In `PHApp::setup()`:
        *   Instantiate your component (e.g., `mySensor = new MyNewSensor(/* constructor args */);`).
        *   Call your component's `setup()` method (e.g., `mySensor->setup();`).
        *   If the component uses Modbus, register it with the `ModbusManager`: `modbusManager->registerComponent(mySensor, MY_SENSOR_START_ADDRESS, MY_SENSOR_REGISTER_COUNT);` (using addresses from `config-modbus.h`).
5.  **Cleanup:** In the `PHApp` destructor (`~PHApp()`), delete the component instance (`delete mySensor;`).
6.  **Build:** Add your new `.cpp` file to the build system if necessary (PlatformIO typically picks up source files in `src` automatically). Run `npm run build` to compile.

## Key Packages and Libraries

This project relies on several key software packages and libraries:

*   **PlatformIO**: The core build system and development ecosystem used to manage the toolchain, libraries, building, and uploading for the ESP32.
*   **Arduino Framework (for ESP32)**: Provides a simplified C++ API for interacting with the ESP32 hardware (GPIO, Serial, WiFi, etc.). It runs on top of ESP-IDF.
*   **ESP-IDF (Espressif IoT Development Framework)**: The official low-level development framework from Espressif, providing the drivers and core system functionalities.
*   **npm**: Used as a task runner to execute build, test, and utility scripts defined in `package.json`.
*   **Python**: Used for various helper scripts (located in the `scripts/` directory) for testing, sending commands, and interacting with the device over serial, Modbus, or HTTP.
*   **C++ Libraries (managed by PlatformIO):**
    *   **`ArduinoJson`**: For efficient JSON serialization and deserialization, used heavily in the REST API.
    *   **`ESPAsyncWebServer` / `AsyncTCP`**: Libraries providing asynchronous handling of HTTP requests and TCP connections, forming the basis of the REST API and WebSocket communication.
    *   **`eModbus`**: Handles Modbus RTU/TCP client and server communication.
    *   **`ArduinoLog`**: Provides flexible logging capabilities, including log levels and output redirection (used here for both Serial and the internal log buffer).
    *   **`LittleFS`**: A lightweight filesystem used to store web assets (HTML, CSS, etc.) on the ESP32's flash memory.
    *   **`WiFi` / `ESPmDNS`**: Standard Arduino/ESP32 libraries for network connectivity and service discovery.
    *   **`Vector`**: A C++ standard library container used for dynamic arrays (like the log buffer).

## Example Scenarios

Here are some common development workflows and the minimal sequence of commands to execute them:

**Scenario 1: Making a change to core C++ code (e.g., in `PHApp.cpp` or a `Component`)**

1.  Make your code changes.
2.  Compile and upload the new firmware:
    ```bash
    npm run upload
    ```
3.  Monitor the device for logs/output:
    ```bash
    npm run build:monitor 
    ```
    (Press Ctrl+C to exit the monitor)

**Scenario 2: Adding a new Component (`MyNewSensor`)**

1.  Follow the steps in "Steps to Add a New Component" above (define addresses, create class, implement logic, integrate into `PHApp`, cleanup).
2.  Compile and upload the new firmware:
    ```bash
    npm run upload
    ```
3.  Test the new component (e.g., using Modbus or serial commands):
    ```bash
    # Example: Read a holding register associated with the new component
    npm run modbus:read:holding -- --address <YOUR_NEW_COMPONENT_ADDRESS>
    
    # Monitor serial output
    npm run build:monitor
    ```

**Scenario 3: Changing the web UI (e.g., editing `front/index.html` or `front/styles.css`)**

1.  Make your changes to the files in the `front/` directory.
2.  Rebuild the web assets and the firmware:
    ```bash
    npm run web:build
    ```
3.  Upload the updated filesystem image:
    ```bash
    npm run web:uploadfs
    ```
4.  Upload the firmware (optional, but good practice if `web:build` included firmware changes):
    ```bash
    npm run upload 
    ```
5.  Access the web UI in your browser (e.g., `http://modbus-esp32.local`) and hard refresh (Ctrl+Shift+R or Cmd+Shift+R) to see changes.

**Scenario 4: Testing a specific Modbus register**

1.  Read the register:
    ```bash
    npm run modbus:read:holding -- --address <REGISTER_ADDRESS>
    ```
2.  Write to the register:
    ```bash
    npm run modbus:write:holding -- --address <REGISTER_ADDRESS> --value <NEW_VALUE>
    ```
3.  Read again to verify:
    ```bash
    npm run modbus:read:holding -- --address <REGISTER_ADDRESS>
    ```

**Scenario 5: Debugging with logs**

1.  Ensure the device is running and connected to the network.
2.  Fetch logs via the REST API:
    ```bash
    npm run debug:serial
    ```
3.  Alternatively, view live serial output:
    ```bash
    npm run build:monitor
    ```

## Core Code / Build

Commands related to basic compilation, cleaning, and uploading the main firmware code.

*   `npm run build`: Compiles the firmware using PlatformIO (`pio run`).
*   `npm run build:clean`: Cleans the build artifacts (`pio run -t clean`).
*   `npm run upload`: Compiles and uploads the firmware via the default upload method (`pio run -t upload`).
*   `npm run build:run`: A full cycle: compiles, uploads, and opens the serial monitor (`pio run && pio run -t upload && pio device monitor`).

## Web Assets

Commands related to generating, building, and uploading the web interface assets (HTML, CSS, Swagger).

*   `npm run web:gen-swagger`: Generates the `swagger_content.h` header file from `swagger.yaml`.
*   `npm run web:build`: Generates web assets (Swagger header, HTML/CSS headers) and then runs the main firmware build (`npm run build`).
*   `npm run web:uploadfs`: Uploads the LittleFS filesystem image containing web assets (`pio run -t uploadfs`). Use after `web:build` to update the UI.

## Modbus

Commands specifically for interacting with or testing Modbus functionality.

*   `npm run modbus:read:coil`: Reads Modbus coils.
*   `npm run modbus:read:holding`: Reads Modbus holding registers.
*   `npm run modbus:write:coil`: Writes to a Modbus coil.
*   `npm run modbus:write:holding`: Writes to a Modbus holding register.
*   `npm run test:modbus:counter:read`: Reads the battle counter value via Modbus.
*   `npm run test:modbus:counter:watch`: Continuously reads the battle counter value via Modbus.
*   `npm run test:modbus:counter:reset`: Resets the battle counter via Modbus.
*   `npm run test:modbus:counter:increment`: Increments the battle counter via Modbus.

## Serial Communication

Commands for interacting with the device via the serial port.

*   `npm run serial:send-cmd`: Sends a specific, hardcoded `printRegisters` command via serial.
*   `npm run serial:send`: Sends a generic message defined in `scripts/send_message.py` via serial.
*   `npm run test:serial:counter:increment`: Sends a command to increment the test counter via serial.
*   `npm run test:serial:counter:reset`: Sends a command to reset the test counter via serial.
*   `npm run test:serial:counter:get`: Sends a command to get the test counter value via serial.
*   `npm run test:serial:client-stats:reset`: Resets Modbus client statistics via serial command.
*   `npm run test:serial:client-stats:get`: Gets Modbus client statistics via serial command.

## REST API

Commands for testing the REST API endpoints.

*   `npm run test:api`: Runs a general REST API test suite.
*   `npm run test:api:ip`: Runs the general API test suite against a specific IP.
*   `npm run test:api:working`: Runs a basic "is the API working" test.
*   `npm run test:api:system-info`: Tests the `/api/v1/system/info` endpoint.
*   `npm run test:api:system-info:ip`: Tests the system info endpoint against a specific IP.
*   `npm run test:api:logs`: Tests the `/api/v1/system/logs` endpoint.
*   `npm run test:api:logs:ip`: Tests the logs endpoint against a specific IP.

## Performance / Stress Testing

Commands explicitly designed for performance or stress testing.

*   `npm run test:serial:rate`: Tests the rate of serial command processing.
*   `npm run test:serial:stress`: Performs a stress test with varying delays for serial commands.
*   `npm run test:modbus:battle`: Runs a Modbus read/write battle test.
*   `npm run test:modbus:battle:quick`: Runs a shorter version of the battle test.
*   `npm run test:modbus:battle:full`: Runs a longer, more intensive version of the battle test.
*   `npm run test:modbus:multi-client`: Tests handling multiple concurrent Modbus clients.
*   `npm run test:modbus:multi-client:low`: Runs the multi-client test with fewer clients/operations.
*   `npm run test:modbus:multi-client:high`: Runs the multi-client test with more clients/operations.
*   `npm run test:modbus:client-sequential`: Tests sequential connections from multiple Modbus clients.
*   `npm run test:modbus:client-sequential:max`: Tests sequential connections with the maximum configured clients.
*   `npm run test:modbus:longpoll`: Runs a long polling test for Modbus.

## Debugging / Monitoring

Commands useful for monitoring the device and diagnosing issues.

*   `npm run build:monitor`: Opens the PlatformIO serial monitor (`pio device monitor`).
*   `npm run debug:serial`: Fetches and prints the buffered logs from the device's REST API (`/api/v1/system/logs`).
*   `npm run test:modbus:counter:watch`: Continuously reads the battle counter value via Modbus.

## Documentation

Commands related to generating documentation.

*   `npm run docs:generate`: Generates code documentation using Doxygen.

## General

*   `npm test`: Placeholder script, currently does nothing specific (`echo "Error: no test specified" && exit 1`). 
*   

6.  **Build:** Add your new `.cpp` file to the build system if necessary (PlatformIO typically picks up source files in `src` automatically). Run `npm run build` to compile.

## Key Packages and Libraries

This project relies on several key software packages and libraries:

*   **PlatformIO**: The core build system and development ecosystem used to manage the toolchain, libraries, building, and uploading for the ESP32.
*   **Arduino Framework (for ESP32)**: Provides a simplified C++ API for interacting with the ESP32 hardware (GPIO, Serial, WiFi, etc.). It runs on top of ESP-IDF.
*   **ESP-IDF (Espressif IoT Development Framework)**: The official low-level development framework from Espressif, providing the drivers and core system functionalities.
*   **npm**: Used as a task runner to execute build, test, and utility scripts defined in `package.json`.
*   **Python**: Used for various helper scripts (located in the `scripts/` directory) for testing, sending commands, and interacting with the device over serial, Modbus, or HTTP.
*   **C++ Libraries (managed by PlatformIO):**
    *   **`ArduinoJson`**: For efficient JSON serialization and deserialization, used heavily in the REST API.
    *   **`eModbus`**: Handles Modbus RTU/TCP client and server communication.
    *   **`ArduinoLog`**: Provides flexible logging capabilities, including log levels and output redirection (used here for both Serial and the internal log buffer).
    *   **`LittleFS`**: A lightweight filesystem used to store web assets (HTML, CSS, etc.) on the ESP32's flash memory.
    *   **`WiFi` / `ESPmDNS`**: Standard Arduino/ESP32 libraries for network connectivity and service discovery.
    *   **`Vector`**: A C++ standard library container used for dynamic arrays (like the log buffer).

## Example Scenarios
// ... existing code ...
