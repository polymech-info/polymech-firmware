#ifndef STATUS_H
#define STATUS_H

static millis_t status_blink_TS = 0;
static bool doBlink = false;
static bool last_blink = true;
static millis_t blink_start_ts;
static millis_t max_blink_time = HOUR_MS; // stop blinking in an hour

// This is using currently 2 LEDs : red & green, running at 220V via relay.

static void status_setup() { }

static void status_loop(){

 if (millis() - status_blink_TS > 1000 ) {
   status_blink_TS = millis();
   last_blink = !last_blink;
   if(doBlink){
     //analogWrite(STATUS_ERROR_PIN, last_blink ? RELAY_ON : RELAY_OFF);
   }
   if(millis() - status_blink_TS > max_blink_time ){
     doBlink = false;
   }
 }
}

static void status_blink(bool blink){
  if(!doBlink && blink){
    blink_start_ts = millis();
  }
  doBlink = blink;
}

static void setStatusAllOn(){
  if(doBlink){
    return;
  }
    digitalWrite(STATUS_POWER_PIN, HIGH);
}

static void setStatusAllOff(){
  if(doBlink){
    return;
  }
    digitalWrite(STATUS_POWER_PIN, LOW);
}

static void setStatus(bool error) {
  /*
  if(doBlink){
    return;
  }
  if (error) {
    analogWrite(STATUS_ERROR_PIN, RELAY_ON);
    analogWrite(STATUS_OK_PIN, RELAY_OFF);
  } else {
    analogWrite(STATUS_OK_PIN, RELAY_ON);
    analogWrite(STATUS_ERROR_PIN, RELAY_OFF);
  }
  */
}

#endif
