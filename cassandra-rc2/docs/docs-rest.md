# REST API Documentation

This document provides an overview of the RESTful API and WebSocket interface for interacting with the device.

## Authentication

The API does not currently require authentication.

## Base URL

All API endpoints are relative to the device's IP address. For example, if the device IP is `192.168.1.100`, the full URL for `/api/v1/system/info` would be `http://192.168.1.100/api/v1/system/info`.

---

## REST API Endpoints

### System Endpoints

#### Get System Information

- **Endpoint:** `GET /api/v1/system/info`
- **Description:** Retrieves general system information like firmware version, uptime, and memory usage.
- **Response:**
  ```json
  {
    "version": "3ce112f",
    "board": "esp32dev",
    "uptime": 12345,
    "timestamp": 12345678,
    "freeHeapKb": 150.5,
    "maxFreeBlockKb": 100.2,
    "cpuTicks": 123456789,
    "loopDurationMs": 500,
    "cpuLoadPercent": null
  }
  ```
- **cURL Example:**
  ```bash
  curl http://<DEVICE_IP>/api/v1/system/info
  ```
- **Node.js Example:**
  ```javascript
  const fetch = require('node-fetch');

  async function getSystemInfo() {
    try {
      const response = await fetch('http://<DEVICE_IP>/api/v1/system/info');
      const data = await response.json();
      console.log(data);
    } catch (error) {
      console.error('Error fetching system info:', error);
    }
  }

  getSystemInfo();
  ```

#### Get System Logs

- **Endpoint:** `GET /api/v1/system/logs`
- **Description:** Retrieves the system logs.
- **Query Parameters:**
  - `level` (optional): The minimum log level to retrieve. Can be one of `none`, `error`, `warning`, `notice`, `trace`, `verbose`. Defaults to `verbose`.
- **Response:** An array of log strings.
  ```json
  [
    "I: System initialized.",
    "W: Network connection weak."
  ]
  ```
- **cURL Example:**
  ```bash
  curl http://<DEVICE_IP>/api/v1/system/logs?level=warning
  ```
- **Node.js Example:**
  ```javascript
  const fetch = require('node-fetch');

  async function getLogs() {
    try {
      const response = await fetch('http://<DEVICE_IP>/api/v1/system/logs?level=warning');
      const data = await response.json();
      console.log(data);
    } catch (error) {
      console.error('Error fetching logs:', error);
    }
  }

  getLogs();
  ```

#### Get/Set Log Level

- **Endpoint:**
  - `GET /api/v1/system/log-level`
  - `GET /api/v1/system/log-level?level=<level>`
- **Description:**
  - `GET` without parameters retrieves the current log level.
  - `GET` with the `level` parameter sets a new log level.
- **Query Parameters:**
  - `level` (for setting): The new log level. One of `none`, `error`, `info`, `warning`, `notice`, `trace`, `verbose`.
- **Response (Get):**
  ```json
  {
    "success": true,
    "level": "verbose"
  }
  ```
- **Response (Set):**
  ```json
  {
    "success": true,
    "level": "warning"
  }
  ```
- **cURL Example (Get):**
  ```bash
  curl http://<DEVICE_IP>/api/v1/system/log-level
  ```
- **cURL Example (Set):**
  ```bash
  curl http://<DEVICE_IP>/api/v1/system/log-level?level=warning
  ```
- **Node.js Example (Set):**
  ```javascript
  const fetch = require('node-fetch');

  async function setLogLevel(level) {
    try {
      const response = await fetch(`http://<DEVICE_IP>/api/v1/system/log-level?level=${level}`);
      const data = await response.json();
      console.log(data);
    } catch (error) {
      console.error('Error setting log level:', error);
    }
  }

  setLogLevel('warning');
  ```

#### List Filesystem

- **Endpoint:** `GET /api/v1/fs/list`
- **Description:** Lists files and directories in the root of the device's filesystem (LittleFS).
- **Response:**
  ```json
  [
    {
      "name": "/index.html",
      "type": "file",
      "size": 1234
    },
    {
      "name": "/config",
      "type": "directory",
      "size": 0
    }
  ]
  ```
- **cURL Example:**
  ```bash
  curl http://<DEVICE_IP>/api/v1/fs/list
  ```

### Modbus Endpoints

These endpoints provide access to Modbus data (coils and registers).

#### Get All Coils

- **Endpoint:** `GET /api/v1/coils`
- **Description:** Retrieves the state of all mapped coils.
- **Response:**
  ```json
  {
    "coils": [
      {
        "address": 1,
        "value": 1,
        "name": "Motor_On",
        "component": "Motor",
        "id": 101,
        "type": 1,
        "flags": 0,
        "group": "Motors"
      }
    ]
  }
  ```
- **cURL Example:**
  ```bash
  curl http://<DEVICE_IP>/api/v1/coils
  ```

#### Get Single Coil

- **Endpoint:** `GET /api/v1/coils?address=<address>`
- **Description:** Retrieves the state of a single coil by its address.
- **Query Parameters:**
  - `address`: The address of the coil.
- **Response:**
  ```json
  {
    "address": 1,
    "value": true
  }
  ```
- **cURL Example:**
  ```bash
  curl http://<DEVICE_IP>/api/v1/coils?address=1
  ```

#### Set Single Coil

- **Endpoint:** `POST /api/v1/coils/<address>?value=<value>`
- **Description:** Sets the state of a single coil.
- **URL Parameters:**
  - `address`: The address of the coil.
- **Query Parameters:**
  - `value`: The new state of the coil (`1`/`true` or `0`/`false`).
- **Response:**
  ```json
  {
    "success": true,
    "address": 51,
    "value": true
  }
  ```
- **cURL Example:**
  ```bash
  curl -X POST "http://<DEVICE_IP>/api/v1/coils/51?value=true"
  ```
- **Node.js Example:**
  ```javascript
  const fetch = require('node-fetch');

  async function setCoil(address, value) {
    try {
      const response = await fetch(`http://<DEVICE_IP>/api/v1/coils/${address}?value=${value}`, {
        method: 'POST'
      });
      const data = await response.json();
      console.log(data);
    } catch (error) {
      console.error(`Error setting coil ${address}:`, error);
    }
  }

  setCoil(51, true);
  ```

#### Get All Registers

- **Endpoint:** `GET /api/v1/registers`
- **Description:** Retrieves the value of all mapped registers.
- **Query Parameters (optional):**
  - `page`: The page number to retrieve (0-indexed). Defaults to 0.
  - `pageSize`: The number of registers per page. Defaults to 20.
- **Response (with pagination):**
  ```json
  {
    "registers": [
      {
        "error": 0,
        "address": 100,
        "value": 1234,
        "name": "Temperature_Setpoint",
        "component": "Heater",
        "id": 201,
        "type": 3,
        "slaveId": 1,
        "flags": 0,
        "group": "Heating"
      }
    ],
    "meta": {
      "page": 0,
      "pageSize": 20,
      "totalRegisters": 256,
      "totalPages": 13
    }
  }
  ```
- **cURL Example (with pagination):**
  ```bash
  curl http://<DEVICE_IP>/api/v1/registers?page=1&pageSize=50
  ```
  ```bash
  curl http://<DEVICE_IP>/api/v1/registers
  ```

#### Get Single Register

- **Endpoint:** `GET /api/v1/registers?address=<address>`
- **Description:** Retrieves the value of a single register by its address.
- **Query Parameters:**
  - `address`: The address of the register.
- **Response:**
  ```json
  {
    "address": 100,
    "value": 1234
  }
  ```
- **cURL Example:**
  ```bash
  curl http://<DEVICE_IP>/api/v1/registers?address=100
  ```

#### Set Single Register

- **Endpoint:** `POST /api/v1/registers/<address>?value=<value>`
- **Description:** Sets the value of a single holding register.
- **URL Parameters:**
  - `address`: The address of the register.
- **Query Parameters:**
  - `value`: The new integer value for the register (16-bit signed).
- **Response:**
  ```json
  {
    "success": true,
    "address": 20,
    "value": 42
  }
  ```
- **cURL Example:**
  ```bash
  curl -X POST "http://<DEVICE_IP>/api/v1/registers/20?value=42"
  ```
- **Node.js Example:**
  ```javascript
  const fetch = require('node-fetch');

  async function setRegister(address, value) {
    try {
      const response = await fetch(`http://<DEVICE_IP>/api/v1/registers/${address}?value=${value}`, {
        method: 'POST'
      });
      const data = await response.json();
      console.log(data);
    } catch (error) {
      console.error(`Error setting register ${address}:`, error);
    }
  }

  setRegister(20, 42);
  ```

#### Get Modbus Address Mappings

- **Endpoint:** `GET /api/v1/mappings`
- **Description:** Retrieves the list of all Modbus address mappings.
- **cURL Example:**
  ```bash
  curl http://<DEVICE_IP>/api/v1/mappings
  ```

---

## WebSocket API

For real-time data streaming and more complex interactions, a WebSocket interface is provided.

- **Endpoint:** `ws://<DEVICE_IP>/ws`

### Connection

A client can connect to the WebSocket endpoint to receive real-time updates and send commands.

**Node.js Example:**
```javascript
const WebSocket = require('ws');

const ws = new WebSocket('ws://<DEVICE_IP>/ws');

ws.on('open', function open() {
  console.log('Connected to WebSocket');
  // Send a command to get system info
  ws.send(JSON.stringify({ command: 'get_sysinfo' }));
});

ws.on('message', function incoming(data) {
  console.log('Received:', JSON.parse(data));
});

ws.on('close', function close() {
  console.log('Disconnected from WebSocket');
});

ws.on('error', function error(err) {
  console.error('WebSocket error:', err);
});
```

### Commands

Commands are sent as JSON strings.

- `get_sysinfo`: Get system information.
- `get_logs`: Get log history.
- `get_coils`: Get all coil values.
- `get_registers`: Get all register values. Supports paging.
- `write_register`: Write to a register. Payload: `{ "command": "write_register", "address": 100, "value": 1234 }`
- `write_coil`: Write to a coil. Payload: `{ "command": "write_coil", "address": 50, "value": true }`

### Register Paging via WebSocket

When requesting all registers via the `get_registers` command, the response can be paginated to handle a large number of registers efficiently.

**Request Payload:**
```json
{
  "command": "get_registers",
  "page": 0,
  "pageSize": 50
}
```

- `page` (optional): The page number to retrieve (0-indexed). Defaults to 0.
- `pageSize` (optional): The number of registers per page. Defaults to 20.

**Response Payload:**

The response is a JSON object containing the data for the requested page and metadata about the pagination.

```json
{
  "type": "registers",
  "data": [
    {
      "error": 0,
      "address": 100,
      "value": 1234,
      "name": "Register_100",
      "component": "CompA",
      "id": 1,
      "type": 3,
      "slaveId": 1,
      "flags": 1,
      "group": "Group1"
    }
    // ... up to pageSize registers
  ],
  "meta": {
    "page": 0,
    "pageSize": 50,
    "totalRegisters": 256,
    "totalPages": 6
  }
}
```

- `type`: The type of the message (`registers`).
- `data`: An array of register objects for the current page.
- `meta`: An object containing pagination details:
  - `page`: The current page number.
  - `pageSize`: The number of items per page.
  - `totalRegisters`: The total number of registers available.
  - `totalPages`: The total number of pages.
