#ifndef PPSERIAL_H
#define PPSERIAL_H
#include <Streaming.h>
#include "Addon.h"

// Message struct:  << id,verb,flags,version,payload >>
// Example :<<1;2;0;1;Power:on:1>>

class PPSerial : public Addon
{

public:
    PPSerial(Stream &_stream) : stream(_stream),
                                Addon("Serial", SERIAL_BRIDGE),
                                msg(new Message(0, 0, 0, 0))
    {
        flags = 0;
    }

    struct Message
    {
    public:
        int id;
        uchar verb;
        int flags;
        uchar version;
        char *payload;
        millis_t ts;
        Message(int _id, int _verb, int _flags, int _version) : id(_id),
                                                                verb(_verb),
                                                                flags(_flags),
                                                                version(_version) {}
    };

    short loop();
    Message *read();
    Message *msg;
    Message *parse(const char *string);
    Vector<Message *> messages;

    void readMessages();
    short setup();

protected:
    Stream &stream;
};
#endif