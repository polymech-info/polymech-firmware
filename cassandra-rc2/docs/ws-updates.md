# WebSocket Compact Update Format Proposal

## 1. Problem Statement

The current WebSocket update mechanism uses verbose JSON payloads for frequent events.
**Example Payload:**

```json
{"type":"register_update","data":{"slaveId":13,"address":1235,"fc":3,"count":1,"id":633,"value":166}}
```

**Issues:**

- High network overhead (~100 bytes/update).
- JSON parsing/serializing CPU cost on ESP32.

## 2. Proposed Solution: Hybrid Binary Protocol

Use **WebSocket Binary Frames** for high-frequency updates while maintaining **Text Frames** for JSON (backward compatibility).

### Key Features

- **Efficiency:** ~10-12 bytes per update (vs ~100 bytes).
- **Performance:** Direct memory replication (C++ struct) -> DataView (JS).
- **Compatibility:** Legacy clients ignore binary frames; new clients handle both.

## 3. Binary Specification (Little Endian)

### Packet Structure

`[Type (1B)] [Payload (Variable)]`

| Hex | Type | Description |
| :--- | :--- | :--- |
| `0x01` | `COIL_UPDATE` | Single Coil Update |
| `0x02` | `REGISTER_UPDATE` | Single Register Update |

### 3.1 Packet 0x01: COIL_UPDATE (10 Bytes)

| Offset | Field | Type | Description |
| :--- | :--- | :--- | :--- |
| 0 | `Type` | `UInt8` | `0x01` |
| 1 | `SlaveId` | `UInt8` | Modbus Slave ID |
| 2 | `Fc` | `UInt8` | Function Code (e.g. 5) |
| 3 | `Value` | `UInt8` | 0 or 1 |
| 4 | `ComponentId` | `UInt16` | Internal ID |
| 6 | `Address` | `UInt16` | Modbus Address |
| 8 | `Count` | `UInt16` | Count (usually 1) |

### 3.2 Packet 0x02: REGISTER_UPDATE (12 Bytes)

| Offset | Field | Type | Description |
| :--- | :--- | :--- | :--- |
| 0 | `Type` | `UInt8` | `0x02` |
| 1 | `SlaveId` | `UInt8` | Modbus Slave ID |
| 2 | `Fc` | `UInt8` | Function Code (e.g. 3) |
| 3 | `Reserved` | `UInt8` | Padding |
| 4 | `ComponentId` | `UInt16` | Internal ID |
| 6 | `Address` | `UInt16` | Modbus Address |
| 8 | `Count` | `UInt16` | Count (usually 1) |
| 10 | `Value` | `UInt16` | Register Value |

## 4. Implementation

### Server (C++)

Define packed structs:

```cpp
#pragma pack(push, 1)
struct BinaryRegisterUpdate {
    uint8_t type = 0x02;
    uint8_t slaveId;
    uint8_t fc;
    uint8_t reserved;
    uint16_t componentId;
    uint16_t address;
    uint16_t count;
    uint16_t value;
};
#pragma pack(pop)
```

Send using `ws.binaryAll((uint8_t*)&update, sizeof(update));`.

### Client (TypeScript)

Handle binary frames:

```typescript
ws.binaryType = 'arraybuffer';
ws.onmessage = (event) => {
  if (event.data instanceof ArrayBuffer) {
    handleBinary(new DataView(event.data));
  } else {
    // legacy JSON
  }
};

function handleBinary(view: DataView) {
  const type = view.getUint8(0);
  if (type === 0x02) {
    const value = view.getUint16(10, true); // Little Endian
    // Dispatch update
  }
}
```

## 5. Alternative: Compact Text

If binary is not feasible, use delimiter-separated strings:
`RU|13|3|633|1235|1|166`

- **Pros:** Human readable.
- **Cons:** Larger, string parsing overhead.
