#include "serial.h"
#include <Streaming.h>
#include <Vector.h>
#include "bridge.h"
#include "common/macros.h"

#define START_STR "<<"
#define END_STR ">>"
#define START_LENGTH 2
#define END_LENGTH 2
#define SPLIT_STR ";"

static PPSerial::Message *_messages[10];

short PPSerial::setup()
{
    messages.setStorage(_messages);
}

short PPSerial::loop()
{
}
PPSerial::Message *PPSerial::parse(const char *string)
{
    Message *msg = new Message(1, 2, 0, 3);
    const char *strings[5];
    char *ptr = NULL;
    byte index = 0;

    ptr = strtok(string, SPLIT_STR);

    while (ptr != NULL && index < 5)
    {

        strings[index] = ptr;
        index++;
        ptr = strtok(NULL, SPLIT_STR);
    }

    msg->id = atoi(strings[0]);
    msg->verb = atoi(strings[1]);
    msg->flags = atoi(strings[2]);
    msg->flags = SBI(msg->flags, Bridge::NEW);
    msg->version = atoi(strings[3]);
    msg->payload = strings[4];
    return msg;
}
void PPSerial::readMessages()
{
    while (stream.available())
    {
        String message = stream.readString();
        message.trim();

        if (message.startsWith(START_STR) && message.endsWith(END_STR))
        {
            String data = message.substring(START_LENGTH, message.length() - END_LENGTH);
            char *ptr = NULL;
            byte index = 0;
            ptr = strtok(data.c_str(), '\n');
            while (ptr != NULL && index < 5)
            {
                if (strlen(ptr) > 4)
                {
                    Message *msg = parse(String(ptr).c_str());
                    if (msg != NULL)
                    {
                        messages.push_back(msg);
                    }
                }
                index++;
                ptr = strtok(NULL, '\n');
            }
        }
    }
}

PPSerial::Message *PPSerial::read()
{
    /*
    readMessages();
    Message *m = messages.at(0);
    if (m != NULL && messages.size())
    {
        messages.remove(0);
        return m;
    }
    return NULL;
*/
    while (stream.available())
    {
        String message = stream.readString();
        message.trim();

        if (message.startsWith(START_STR) && message.endsWith(END_STR))
        {

            msg->payload = NULL;

            String data = message.substring(START_LENGTH, message.length() - END_LENGTH);
            const char *strings[5];
            char *ptr = NULL;
            byte index = 0;
            ptr = strtok(data.c_str(), SPLIT_STR);
            while (ptr != NULL && index < 5)
            {

                strings[index] = ptr;
                index++;
                ptr = strtok(NULL, SPLIT_STR);
            }

            msg->id = atoi(strings[0]);
            msg->verb = atoi(strings[1]);
            msg->flags = atoi(strings[2]);
            msg->version = atoi(strings[3]);
            msg->payload = strings[4];
            return msg;
        }
        else
        {
            Serial.print(message);
            Serial.println(" : invalid message");
        }
    }
    return NULL;
}