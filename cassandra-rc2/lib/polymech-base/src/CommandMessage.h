#ifndef COMMAND_MESSAGE_H
#define COMMAND_MESSAGE_H

#include <Vector.h>
#include <ArduinoLog.h>
#include <StringUtils.h>
#include "./enums.h"

//#define DEBUG_MESSAGES_PARSE

#ifdef DEBUG_MESSAGES_PARSE
#define DEBUG_MESSAGES_PARSER(format, ...) Log.verboseln(format, ##__VA_ARGS__)
#else
#define DEBUG_MESSAGES_PARSER(format, ...)
#endif

#define MESSAGE_TOKENS 4

class CommandMessage
{
public:
    short id;
    E_CALLS verb;
    E_MessageFlags flags;
    String payload;
    millis_t ts;

    static constexpr char *START_STR = C_STR("<<");
    static constexpr char *END_STR = C_STR(">>");
    static constexpr char *DELIMITER = C_STR(";");

    static constexpr int START_LENGTH = 2;
    static constexpr int END_LENGTH = 2;

    CommandMessage(short _id, E_CALLS _verb, E_MessageFlags _flags) : id(_id),
                                                                      verb(_verb),
                                                                      flags(_flags)
    {
    }

    // Add default constructor
    CommandMessage() : id(0),
                       verb(E_CALLS::EC_NONE),
                       flags(E_MessageFlags::E_MF_NONE) {}

    void clear()
    {
        id = 0;
        verb = E_CALLS::EC_NONE;
        flags = E_MessageFlags::E_MF_NONE;
        ts = 0;
    }

    bool matches(String string)
    {
        return string.startsWith(CommandMessage::START_STR) && string.endsWith(CommandMessage::END_STR);
    }

    bool parse(String message)
    {
        clear();
        String data = message.substring(CommandMessage::START_LENGTH, message.length() - CommandMessage::END_LENGTH);
        char dataBuffer[data.length() + 1];
        strcpy(dataBuffer, data.c_str());
        const char *strings[MESSAGE_TOKENS];
        char *ptr = NULL;
        byte index = 0;
        ptr = strtok(dataBuffer, CC_STR(CommandMessage::DELIMITER)); 
        while (ptr != NULL && index < MESSAGE_TOKENS) 
        {
            strings[index] = ptr;
            index++;
            ptr = strtok(NULL, CC_STR(CommandMessage::DELIMITER));
        }

        if (index < MESSAGE_TOKENS)
        {
            DEBUG_MESSAGES_PARSER("CommandMessage::parse: invalid message - incomplete - got %d tokens", index);
            return false;
        }

        id = atoi(strings[0]);
        verb = (E_CALLS)convertTo<int>(strings[1]);
        flags = (E_MessageFlags)convertTo<long int>(strings[2]);
        payload = String(strings[3]);
        ts = millis();
        DEBUG_MESSAGES_PARSER("CommandMessage::parse: id: %d, verb: %d, flags: %d, payload: %s", id, verb, flags, payload.c_str()); 
        return true;
    }
};

#endif
