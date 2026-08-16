#include "IRSensor.h"
#include "config.h"
#include <Streaming.h>

IRSensor::IRSensor()
{
  this->highTS = 0;
  this->lowTS = 0;
  this->dt = 0;
  this->now = 0;
  this->ir_moving = 0;
}

void IRSensor::debug(Stream &stream)
{
#ifdef IR_SPEED
  stream
      << "IR-MOVING : " << ir_moving << "IR-SPEED: " << (int)this->velocity;
#else
  stream << "IR-MOVING : " << (int)this->ir_moving;
#endif
}

void IRSensor::count()
{
#ifdef IR_SPEED
  if (digitalRead(IR_PIN) && (micros() - this->debounce > IR_INTERVAL) && digitalRead(IR_PIN))
  {
    // Check again that the encoder sends a good signal and then check that the time is greater than 1000 microseconds and check again that the signal is correct.
    this->debounce = micros(); // Store the time to verify that we do not count the rebound in the signal.
    pulses++;
  }
#endif
}

short IRSensor::setup()
{
#ifdef IR_SPEED
  pinMode(IR_PIN, INPUT);
  attachInterrupt(0, ir_count, RISING); // Configuration of interrupt 0, where it is connected.
  this->pulses = 0;
  this->rpm = 0;
  this->timeold = 0;
#endif
}

short IRSensor::loop()
{
#ifdef IR_SPEED
  if (millis() - this->timeold >= IR_INTERVAL)
  {
    noInterrupts();                                                             // Don't process interrupts during calculations // We disconnect the interrupt so it doesn't act in this part of the program.
    this->rpm = (minute / this->pulsesperturn) / (millis() - timeold) * pulses; // Calculate the revolutions per minute
    this->velocity = this->rpm * 3.1416 * this->wheel_diameter * 60 / 1000000;  // Speed ​​calculation in [Km / h]
    timeold = millis();                                                         // We store the current time.
    this->pulses = 0;                                                           // Initialize the pulses.
    this->interrupts();                                                         // Restart the interrupt processing // Reiniciamos la interrupción
  }
#endif

  this->ir_value = digitalRead(IR_PIN);
  this->now = millis();
  if (this->ir_value == HIGH)
  {
    this->highTS = this->now;
  }
  else
  {
    this->lowTS = this->now;
  }

  if (this->highTS <= this->lowTS)
  {
    this->dt = this->lowTS - this->highTS;
  }
  else
  {
    this->dt = this->highTS - this->lowTS;
  }

  this->dt = abs(this->dt);

  if (this->dt > IR_TIMEOUT)
  {
    this->ir_moving = 0;
  }
  else
  {
    this->ir_moving = 1;
  }
}

short IRSensor::ok()
{
  return this->ir_moving == 1;
}