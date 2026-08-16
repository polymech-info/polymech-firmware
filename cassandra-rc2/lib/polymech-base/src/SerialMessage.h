#ifndef SERIAL_MESSAGE_H
#define SERIAL_MESSAGE_H

#include <Vector.h>
#include <ArduinoLog.h>
#include <Arduino.h> // Add for Stream, String etc. if not implicit

#include <xtypes.h>
#include <Component.h>
#include <CommandMessage.h>
#include "config.h"

#ifndef SERIAL_RX_BUFFER_SIZE
#define SERIAL_RX_BUFFER_SIZE 256
#endif

class CommandMessage;
/**
 * @class SerialMessage
 * @brief Represents a serial message component.
 * 
 * The SerialMessage class is a component that handles serial communication.
 * It provides methods for reading, parsing, and handling command messages.
 */
class SerialMessage : public Component
{
public:
    
    /**
     * @brief Constructs a SerialMessage object.
     * 
     * @param _stream The serial stream used for communication.
     * @param _owner The owner component of this SerialMessage.
     */
    SerialMessage(Stream &_stream, Component *_owner) : Component("SerialMessage", COMPONENT_KEY_MB_SERIAL, Component::COMPONENT_DEFAULT, _owner),
                                                        stream(_stream),
                                                        _rxBufferIdx(0) // Initialize buffer index
    {
        lastRead = 0;
        _rxBuffer[0] = '\0'; // Initialize buffer (Use single quotes for single char)
    }

    /**
     * @brief Reads a command message from the serial stream.
     * 
     * @return A pointer to the CommandMessage object read from the stream.
     */
    CommandMessage *read(); // Removed - logic moved to loop()

    /**
     * @brief Parses a string into a CommandMessage object.
     * 
     * @param string The string to be parsed.
     * @return A pointer to the parsed CommandMessage object.
     */
    CommandMessage *parse(char *string); // Removed unused method

    /**
     * @brief Executes the main loop of the SerialMessage component.
     * 
     * @return A status code indicating the result of the loop execution.
     */
    short loop();
    /**
     * @brief Performs the setup operations for the SerialMessage component.
     * 
     * @return A status code indicating the result of the setup.
     */
    short setup();

    /**
     * @brief Performs debug operations for the SerialMessage component.
     * @return A status code indicating the result of the debug operation.
     */
    short debug();

private:
    // Private members and methods
    // Vector<CommandMessage *> messages; // Removed unused vector
    millis_t lastRead;
    char _rxBuffer[SERIAL_RX_BUFFER_SIZE]; // Receive buffer
    uint8_t _rxBufferIdx;                  // Current index in buffer
    bool _serialConnected = false;         // Track connection status

protected:
    Stream &stream;
    CommandMessage msg; // Removed - Will create local instance in loop
    String readStringFromSerial(); // Removed - logic moved to loop()
    char _delimiter = '\n'; // Message delimiter (Use single quotes)
};

#endif