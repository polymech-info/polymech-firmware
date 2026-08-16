#ifndef NETWORK_VALUE_PB_H
#define NETWORK_VALUE_PB_H

#include "pb_encode.h"
#include "modbus/ModbusTypes.h"
#include <type_traits>
#include <Arduino.h> // For millis()

class NV_Protobuf {
protected:
    // This is a feature class, and its methods will be called from NetworkValue.
    // It is stateless.

public:
    void init_feature() {}
    void clear_feature() {}
    void setup_feature() {}
    void loop_feature() {}

    void debug_pb_feature() const {
        Serial.println("[NV_Debug] ==> NV_Protobuf::debug_pb_feature() CALLED <==");
    }

    void info_feature() const {
         Log.traceln(F("  [Feature: Protobuf] Enabled"));
    }

    template <typename T>
    bool encode(pb_ostream_t *stream, const MB_Registers& regInfo, const T& value) const {
        // Field 1: address (uint32)
        if (!pb_encode_tag(stream, PB_WT_VARINT, 1)) return false;
        if (!pb_encode_varint(stream, regInfo.startAddress)) return false;

        // Field 2: timestamp (uint64)
        if (!pb_encode_tag(stream, PB_WT_VARINT, 2)) return false;
        if (!pb_encode_varint(stream, millis())) return false;

        // oneof value
        return encode_value(stream, value);
    }

private:
    // For bool
    bool encode_value(pb_ostream_t *stream, const bool& value) const {
        if (!pb_encode_tag(stream, PB_WT_VARINT, 4)) return false; // field 4
        return pb_encode_varint(stream, value);
    }

    // For integers (signed and unsigned) and enums
    template <typename T>
    typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value, bool>::type
    encode_value(pb_ostream_t *stream, const T& value) const {
        if (!pb_encode_tag(stream, PB_WT_VARINT, 3)) return false; // field 3
        return pb_encode_svarint(stream, static_cast<int64_t>(value));
    }

    // For enums
    template <typename T>
    typename std::enable_if<std::is_enum<T>::value, bool>::type
    encode_value(pb_ostream_t *stream, const T& value) const {
        if (!pb_encode_tag(stream, PB_WT_VARINT, 3)) return false; // field 3, same as integers
        return pb_encode_svarint(stream, static_cast<int64_t>(value));
    }
    
    // For floats/doubles
    template <typename T>
    typename std::enable_if<std::is_floating_point<T>::value, bool>::type
    encode_value(pb_ostream_t *stream, const T& value) const {
        float float_val = static_cast<float>(value);
        if (!pb_encode_tag(stream, PB_WT_32BIT, 5)) return false; // field 5
        return pb_encode_fixed32(stream, &float_val);
    }

    // For std::array
    template <typename T, size_t N>
    bool encode_value(pb_ostream_t *stream, const std::array<T, N>& value) const {
        if (!pb_encode_tag(stream, PB_WT_STRING, 6)) return false; // field 6
        return pb_encode_string(stream, (pb_byte_t*)value.data(), value.size() * sizeof(T));
    }

    // For std::string
    bool encode_value(pb_ostream_t *stream, const std::string& value) const {
        if (!pb_encode_tag(stream, PB_WT_STRING, 6)) return false; // field 6
        return pb_encode_string(stream, (const pb_byte_t*)value.c_str(), value.length());
    }
};

#endif // NETWORK_VALUE_PB_H 