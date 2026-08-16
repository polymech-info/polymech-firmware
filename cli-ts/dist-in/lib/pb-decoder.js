import pkg from 'protobufjs';
const { Reader } = pkg;
/**
 * Decodes a NetworkValue update from a Protobuf binary buffer.
 * This is a manual implementation based on the schema defined in the documentation,
 * as we are not using .proto files on the client.
 * @param buffer The binary buffer received from the WebSocket.
 * @returns A structured object with the decoded data.
 */
export function decodeNetworkValue(buffer) {
    const reader = Reader.create(buffer);
    const result = {};
    let value = null;
    while (reader.pos < reader.len) {
        const tag = reader.uint32();
        // The field number is the upper 3 bits of the tag.
        const fieldNumber = tag >>> 3;
        // The wire type is the lower 3 bits of the tag.
        const wireType = tag & 7;
        switch (fieldNumber) {
            case 1: // address
                result.address = reader.uint32();
                break;
            case 2: // timestamp
                // Timestamps are uint64. JS numbers can lose precision for very large values,
                // but for millis() this is safe for a very long time.
                result.timestamp = reader.uint64();
                break;
            case 3: // sint_value
                value = reader.sint64();
                break;
            case 4: // bool_value
                value = reader.bool();
                break;
            case 5: // float_value
                value = reader.float();
                break;
            case 6: // bytes_value
                const bytes = Buffer.from(reader.bytes());
                // Assuming bytes are an array of 32-bit signed integers (4 bytes each)
                if (bytes.length > 0 && bytes.length % 4 === 0) {
                    const numbers = [];
                    for (let i = 0; i < bytes.length; i += 4) {
                        numbers.push(bytes.readInt32LE(i));
                    }
                    value = numbers;
                }
                else {
                    // If not a multiple of 4, treat as a raw buffer
                    value = bytes;
                }
                break;
            default:
                // Skip unknown fields to maintain forward compatibility.
                reader.skipType(wireType);
                break;
        }
    }
    result.value = value;
    return result;
}
//# sourceMappingURL=pb-decoder.js.map