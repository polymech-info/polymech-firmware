#ifndef POS3_H
#define POS3_H

#include <Bounce2.h>
#include "../enums.h"

class Pos3
{
public:
   Pos3(int _upPin, int _downPin) : upPin(_upPin), downPin(_downPin)
   {
   }

   int setup()
   {

      this->debouncerUp = Bounce();
      this->debouncerUp.attach(this->upPin, INPUT_PULLUP);
      this->debouncerUp.interval(25);

      this->debouncerDown = Bounce();
      this->debouncerDown.attach(this->downPin, INPUT_PULLUP);
      this->debouncerDown.interval(25);
      return 0;
   }
   int loop()
   {
      int newDirection = this->read();

      if (newDirection != this->switch_pos)
      {
         this->last_switch = this->switch_pos;
      }
      this->switch_pos = newDirection;

      return this->switch_pos;
   }

   int last_switch = -1; // Track last switch position
   int switch_pos = -1;  // Current switch position

protected:
   int upPin;
   int downPin;

   Bounce debouncerUp;
   Bounce debouncerDown;

private:
   int read()
   {

      this->debouncerUp.update();
      this->debouncerDown.update();

      bool up = this->debouncerUp.read() == 0 ? true : false;
      bool down = this->debouncerDown.read() == 0 ? true : false;

      int newDirection = 0;

      if (up)
      {
         newDirection = POS3_DIRECTION::UP;
      }
      if (down)
      {
         newDirection = POS3_DIRECTION::DOWN;
      }
      if (!up && !down)
      {
         newDirection = POS3_DIRECTION::MIDDLE;
      }
      if (up && down)
      {
         newDirection = POS3_DIRECTION::INVALID;
      }
      return newDirection;
   }
};

#endif
