#ifndef VFD_H
#define VFD_H

#include <Streaming.h>
#include "./Addon.h"
#include "./enums.h"
#include "./config.h"

#ifdef HAS_STATES
#include <ArduinoJson.h>
#endif

class VFD : public Addon
{
public:

    enum DIRECTION
    {
        FORWARD = 1,
        STOP = 0,
        REVERSE = 2
    };

    VFD() : Addon(VFD_STR, VFD_CONTROL),
            direction(STOP){};

    void rev(short nop)
    {
        update(DIRECTION::REVERSE);
    }

    void fwd(short nop)
    {
        update(DIRECTION::FORWARD);
    }

    short setup()
    {
        pinMode(FWD_PIN, OUTPUT);
        pinMode(REV_PIN, OUTPUT);
        
        stop();
    }
    
    short stop(short nop = 0)
    {
        update(DIRECTION::STOP);
    }
    
    void speed(int aValue)
    {
    }

    virtual void debug(Stream *stream)
    {
       // *stream << this->name << ":" << SPACE(direction);
    }

    virtual void info(Stream *stream)
    {
      // *stream << this->name << "\n\t" << SPACE(": FWD PIN " << FWD_PIN << " | REV PIN " << REV_PIN);
    }

    uchar direction;
    uchar lastDirection;
    millis_t dt;

#ifdef HAS_STATES
    String state()
    {
        const int capacity = JSON_OBJECT_SIZE(2);
        StaticJsonDocument<capacity> doc;
        doc["0"] = id;
        doc["1"] = direction;
        return doc.as<String>();
    }
#endif
private:
    void update(uchar newDirection)
    {
        if (direction != newDirection)
        {
            dt = now;
            lastDirection = direction;
            direction = newDirection;
            switch (direction)
            {
                case DIRECTION::FORWARD:
                {
                    digitalWrite(REV_PIN, LOW);
                    digitalWrite(FWD_PIN, HIGH);
                    break;
                }
                case DIRECTION::REVERSE:
                {
                    digitalWrite(FWD_PIN, LOW);
                    digitalWrite(REV_PIN, HIGH);
                    break;
                }
                case DIRECTION::STOP:
                {
                    digitalWrite(FWD_PIN, LOW);
                    digitalWrite(REV_PIN, LOW);
                }
            }
        }
    }
};

#endif
