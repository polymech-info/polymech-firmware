# Web Interface and REST API

This document describes the web interfaces provided by the firmware.

## Static Web UI

The device serves a static web interface (built using React, located in `../web-basic/dist` during development) from its root URL (`/`).

-   **URL:** `http://<device-ip>/`
-   **Functionality:** Provides a user interface for monitoring and potentially controlling the device. All dynamic data is fetched via the REST API described below.

## REST API

The device exposes a RESTful API for programmatic interaction under the `/api/v1/` path.

The API specification follows the OpenAPI 3.0 standard and can be viewed:

-   **Live Docs (Swagger UI):** `http://<device-ip>/api-docs` (Serves Swagger UI interface)
-   **Specification File:** `http://<device-ip>/api/swagger.yaml`

### Endpoints

#### System

*   **`GET /api/v1/system/info`**
    *   **Description:** Retrieves general system information, including firmware version, board name, uptime, and a list of components registered with the Modbus manager.
    *   **Response:** JSON object containing system details and an array of component objects (`id`, `name`, `startAddress`, `count`).

*   **`GET /api/v1/system/logs`**
    *   **Description:** Retrieves recent system log messages captured in an internal buffer.
    *   **Response:** JSON array of strings, with each string being a log line (oldest first, up to 300 lines).

#### Coils

*   **`GET /api/v1/coils`**
    *   **Description:** Retrieves the status of all registered Modbus coils.
    *   **Response:** JSON object `{ "coils": [ { "address": <int>, "value": <bool> }, ... ] }`.
*   **`GET /api/v1/coils?address=<addr>`**
    *   **Description:** Retrieves the status of a specific Modbus coil by its address.
    *   **Response:** JSON object `{ "address": <int>, "value": <bool> }` or `404 Not Found`.
*   **`POST /api/v1/coils/<addr>?value=<0|1|true|false>`**
    *   **Description:** Sets the state of a specific Modbus coil.
    *   **Response:** JSON object `{ "success": true, "address": <int>, "value": <bool> }` or error status (`400`, `404`).

#### Registers

*   **`GET /api/v1/registers`**
    *   **Description:** Retrieves the value of all registered Modbus holding registers.
    *   **Response:** JSON object `{ "registers": [ { "address": <int>, "value": <int> }, ... ] }`.
*   **`GET /api/v1/registers?address=<addr>`**
    *   **Description:** Retrieves the value of a specific Modbus holding register by its address.
    *   **Response:** JSON object `{ "address": <int>, "value": <int> }` or `404 Not Found`.
*   **`POST /api/v1/registers/<addr>?value=<number>`**
    *   **Description:** Sets the value of a specific Modbus holding register.
    *   **Response:** JSON object `{ "success": true, "address": <int>, "value": <int> }` or error status (`400`, `404`).


## Notes

*   The REST API relies on the `ModbusManager` to interact with underlying components. Ensure `ModbusManager` is initialized correctly.
*   The endpoints for writing individual coils/registers (`POST /coils/{addr}`, `POST /registers/{addr}`) are currently non-functional due to suspected routing issues in the web server library.
*   Error handling is basic; more specific HTTP status codes could be implemented based on Modbus exception codes or internal errors. 