#ifndef RESET_H
#define RESET_H

// This module uses currently a normally closed momentary button.

static millis_t sw_reset_TS = 0;

static void reset_setup()
{
  pinMode(RESET_PIN, INPUT_PULLUP);    
  sw_reset_TS = millis();
}

static void reset_loop()
{
  if (millis() - sw_reset_TS > RESET_INTERVAL) {
    #if RESET_NC == true
    //  globals.isReset = digitalRead(RESET_PIN);
    #else
    //  globals.isReset = !digitalRead(RESET_PIN);
    #endif

    sw_reset_TS = millis();
  //  if(globals.isReset && DEBUG){
  //    Serial.println("reset");
  //  }
  } 
}

#endif
