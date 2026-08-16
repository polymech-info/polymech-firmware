#ifndef STATUSLIGHT_H
#define STATUSLIGHT_H

class StatusLight
{
public:
  StatusLight(short _pin) : pin(_pin)
  {
    doBlink = false;
    status_blink_TS = 0;
    last_blink = !digitalRead(pin);
  }

  short setup()
  {
  }

  void setBlink(bool blink)
  {
    doBlink = blink;
  }

  void on()
  {
    digitalWrite(pin, HIGH);
  }

  void off()
  {
    digitalWrite(pin, LOW);
  }

  short loop()
  {
    if (doBlink)
    {
      if (millis() - status_blink_TS > 1000)
      {
        status_blink_TS = millis();
        last_blink = !last_blink;
        digitalWrite(pin, last_blink);
      }
    }
  }

  void status_loop()
  {
  }

  void status_blink(bool blink)
  {
    if (!doBlink && blink)
    {
      blink_start_ts = millis();
    }
    doBlink = blink;
  }

  short pin;
  millis_t status_blink_TS;
  bool doBlink;
  bool last_blink;
  millis_t blink_start_ts;
  millis_t max_blink_time; // stop blinking in an hour
};

#endif
