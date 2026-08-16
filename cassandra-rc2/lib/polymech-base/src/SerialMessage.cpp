#include <Vector.h>
#include <ArduinoLog.h>

#include "Bridge.h"

#include <macros.h>
#include <constants.h>
#include <xtypes.h>

#include "SerialMessage.h"
#include "CommandMessage.h"

#ifndef SERIAL_COMMAND_PARSE_INTERVAL
#define SERIAL_COMMAND_PARSE_INTERVAL 50
#endif

// #define DEBUG_SERIAL_MESSAGES

#ifdef DEBUG_SERIAL_MESSAGES
#define _DEBUG_MESSAGE_HANDLING(format, ...) Log.verboseln(format, ##__VA_ARGS__)
#else
#define _DEBUG_MESSAGE_HANDLING(format, ...)
#endif

void printStringAsHex(const char *str)
{
    Serial.print(" :: ");
    for (int i = 0; str[i] != '\0'; i++)
    {
        Serial.print("0x");
        if (str[i] < 0x10)
        {
            Serial.print("0");
        }
        Serial.print(str[i], HEX);
        Serial.print(" ");
    }
    Serial.println(" :: ");
}


short SerialMessage::setup()
{
    return E_OK;
}

short SerialMessage::debug()
{
    return E_OK;
}
String SerialMessage::readStringFromSerial()
{
    String message;
    while (stream.available())
    {
        message = stream.readString();
        message.trim();
    }
    return message;
}

CommandMessage *SerialMessage::read()
{
    String message = readStringFromSerial();
    if (!message.length())
    {
        return NULL;
    }
    _DEBUG_MESSAGE_HANDLING("SerialMessage::read: message: %s", message.c_str());
    // Use the member 'msg' object directly
    if (!this->msg.matches(message.c_str()))
    {
        _DEBUG_MESSAGE_HANDLING("SerialMessage::read : Invalid message - no match : %s", message.c_str());
        return NULL;
    }

    bool validMessage = this->msg.parse(message);
    if (!validMessage)
    {
        _DEBUG_MESSAGE_HANDLING("SerialMessage::read : invalid message - incomplete : %s", message.c_str());
        return NULL;
    }
    // Return address of the member object
    return &this->msg;
}

short SerialMessage::loop()
{
    if (now - lastRead < SERIAL_COMMAND_PARSE_INTERVAL)
    {
        return E_OK;
    }
    lastRead = now;
    // Use the pointer returned by read()
    CommandMessage *parsedMsg = read(); 
    if (parsedMsg)
    {
        _DEBUG_MESSAGE_HANDLING("SerialMessage::loop:received message %d :: %s", parsedMsg->id, parsedMsg->payload.c_str());
        if (owner)
        {
            // Pass the pointer to the parsed message (which is &this->msg)
            owner->onMessage(parsedMsg->id, parsedMsg->verb, parsedMsg->flags, parsedMsg->payload, this);
        }
        else
        {
            _DEBUG_MESSAGE_HANDLING("SerialMessage::loop: have no owner");
        }
    }
    return E_OK;
}