export const modbusErrorMap: Record<number, string> = {
  // Standard Modbus Exceptions
  0: "Success",
  1: "Illegal Function",
  2: "Illegal Data Address",
  3: "Illegal Data Value",
  4: "Server Device Failure",
  5: "Acknowledge",
  6: "Server Device Busy",
  7: "Negative Acknowledge",
  8: "Memory Parity Error",
  10: "Gateway Path Unavailable", // Assuming 0x0A
  11: "Gateway Target Device Failed to Respond", // Assuming 0x0B

  // Internal Operation/Queue Errors - Assuming custom codes starting from a base, e.g., 100
  // NOTE: These codes might differ in the actual implementation. Adjust as needed.
  100: "Operation Not Ready",
  101: "ModbusRTU Operation Queue Full",
  102: "eModbus Client Queue Full",
  103: "Operation Execution Failed",
  104: "Invalid Parameter",
  105: "Operation Retrying",
  106: "Max Retries Exceeded",

  // eModbus Specific Communication Errors
  224: "Timeout", // 0xE0
  225: "Invalid Server Response", // 0xE1
  226: "CRC Error", // 0xE2, RTU only
  227: "Function Code Mismatch", // 0xE3
  228: "Server ID Mismatch", // 0xE4
  229: "Packet Length Error", // 0xE5
  230: "Parameter Count Error", // 0xE6
  231: "Parameter Limit Error", // 0xE7
  232: "eModbus Request Queue Full", // 0xE8, eModbus client queue
  233: "Illegal IP or Port", // 0xE9
  234: "IP Connection Failed", // 0xEA
  235: "TCP Header Mismatch", // 0xEB
  236: "Empty Message Received", // 0xEC
  237: "ASCII Frame Error", // 0xED
  238: "ASCII LRC Error", // 0xEE
  239: "ASCII Invalid Character", // 0xEF
  240: "Broadcast Error", // 0xF0
  255: "Undefined Error", // 0xFF, Other communication error

  // Undefined/Default
  [-1]: "Undefined or Unknown Error", // Placeholder for default/unknown
};

export const getModbusErrorDescription = (errorCode: number): string => {
  return modbusErrorMap[errorCode] || modbusErrorMap[-1]; // Return specific description or the default unknown error
};

// Example usage:
// import { getModbusErrorDescription } from './modbusErrorMap';
// const description = getModbusErrorDescription(3); // "Illegal Data Value"
// const unknownDesc = getModbusErrorDescription(999); // "Undefined or Unknown Error" 