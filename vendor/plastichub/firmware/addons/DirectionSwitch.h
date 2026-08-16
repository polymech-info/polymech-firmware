#ifndef DIRECTION_SWITCH_H
#define DIRECTION_SWITCH_H

#include "../config.h"
#include "../components/3pos.h"
#include "../Addon.h"
#include <Streaming.h>
#include "../common/macros.h"

class DirectionSwitch : public Addon {
  public:
    Pos3 dir_switch;
    DirectionSwitch () : 
      dir_switch(DIR_SWITCH_UP_PIN, DIR_SWITCH_DOWN_PIN),
      Addon(DIRECTION_SWITCH_STR,DIRECTION_SWITCH){}
    
    void debug(Stream* stream){
      //*stream << this->name << ":" << 
      //  SPACE(dir_switch.switch_pos) << SPACE(dir_switch.last_switch);
    }
    void info(Stream* stream){
      //*stream << this->name << "\n\t : " SPACE("Up Pin:" << DIR_SWITCH_UP_PIN) << SPACE("\t | Down Pin :" << DIR_SWITCH_DOWN_PIN);
    }
    short setup(){
      dir_switch.setup();
      return dir_switch.loop();
    }
    short loop(){
      return dir_switch.loop();
    }
};

#endif
