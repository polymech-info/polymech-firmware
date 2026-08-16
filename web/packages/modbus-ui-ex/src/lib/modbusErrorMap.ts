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

  // eModbus Specific Communication Errors - Assuming custom codes, e.g., starting from 200
  // NOTE: These codes might differ. Adjust as needed.
  200: "Timeout",
  201: "Invalid Server Response",
  202: "CRC Error", // RTU
  203: "Function Code Mismatch",
  204: "Server ID Mismatch",
  205: "Packet Length Error",
  206: "Parameter Count Error",
  207: "Parameter Limit Error",
  208: "eModbus Request Queue Full",
  209: "Illegal IP or Port", // TCP
  210: "IP Connection Failed", // TCP
  211: "TCP Header Mismatch", // TCP
  212: "Empty Message Received",
  213: "ASCII Frame Error", // ASCII
  214: "ASCII LRC Error", // ASCII
  215: "ASCII Invalid Character", // ASCII
  216: "Broadcast Error",

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