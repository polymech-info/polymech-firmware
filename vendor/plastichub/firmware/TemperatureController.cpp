#include "TemperatureController.h"

#ifdef HAS_TC

short TemperatureController::setup()
{
  return E_OK;
}

short TemperatureController::debug(Stream *stream)
{
  *stream << this->name << "\n\t : ";
  return E_OK;
}

short TemperatureController::info(Stream *stream)
{
  *stream << this->name << "\n\t : ";
  return E_OK;
}

short TemperatureController::loop()
{
  updateTCP();
  return E_OK;
}

void TemperatureController::updateTCP()
{
  modbus->mb->R[MB_W_TC_STATE] = _state;
}

double PIDInput[2] = {0, 0};
double PIDOutput[2] = {0, 0};
double PIDSetpoint[2] = {0, 0};

double PIDKp[2]; // = DEFAULT_PID_KP;
double PIDKi[2]; // = DEFAULT_PID_KI;
double PIDKd[2]; // = DEFAULT_PID_KD;

double relayPin[2] = HEATER_PIN_RELAY;
double thermPin[2] = TEMP_PIN;

unsigned int targetTempHeater[PLASTIC_ID_TOTAL][2]; // = {DEFAULT_TEMP_PET, DEFAULT_TEMP_HDPE, DEFAULT_TEMP_V, DEFAULT_TEMP_LDPE, DEFAULT_TEMP_PP, DEFAULT_TEMP_PS, DEFAULT_TEMP_PLA};

unsigned long windowStartTime[2];

Oversample *Thermistor[2];

PID PIDHeater[2] = {{&PIDInput[0], &PIDOutput[0], &PIDSetpoint[0], PIDKp[0], PIDKi[0], PIDKd[0], DIRECT},
                    {&PIDInput[1], &PIDOutput[1], &PIDSetpoint[1], PIDKp[1], PIDKi[1], PIDKd[1], DIRECT}};

extern void initHeater()
{
  for (int i = 0; i < 2; i++)
  {
    PIDHeater[i].SetMode(MANUAL);
    PIDHeater[i].SetOutputLimits(0, SOFT_PWM_WINDOW_SIZE);
    pinMode(relayPin[i], OUTPUT);
    Thermistor[i] = new Oversample(thermPin[i], 14);
    printPIDSetting(i);
  }
  pinMode(FAN_PIN, OUTPUT);
}

extern void updateTemp()
{
  for (int i = 0; i < 2; i++)
  {
    PIDInput[i] = analog2temp(Thermistor[i]->readDecimated());
    if ((int)PIDInput[i] < HEATER_MIN_TEMP || (int)PIDOutput[i] > HEATER_MAX_TEMP)
    {
      offHeater();
    }
  }
}

extern void updatePID()
{
  for (int i = 0; i < 2; i++)
  {
    PIDHeater[i].Compute();
    if ((int)PIDSetpoint[i] <= 0)
    {
      PIDOutput[i] = 0; // Override PID
      PIDHeater[i].SetMode(MANUAL);
    }
  }
}

extern void manageHeaterSoftPWM()
{
  unsigned long now = millis();

  for (int i = 0; i < 2; i++)
  {
    if (now - windowStartTime[i] > SOFT_PWM_WINDOW_SIZE)
    {
      windowStartTime[i] = now;
    }
    if (PIDOutput[i] > now - windowStartTime[i])
    {
      digitalWrite(relayPin[i], HIGH);
    }
    else
    {
      digitalWrite(relayPin[i], LOW);
    }
  }
}

extern void AT(double temp, int hotend, int ncycles, bool set_result /*=false*/)
{
  double input = 0.0;
  int cycles = 0;
  bool heating = true;

  unsigned long temp_ms = millis(), t1 = temp_ms, t2 = temp_ms;
  long t_high = 0, t_low = 0;
  unsigned long ms = millis();

  long bias, d;
  double Ku, Tu;
  double workKp = 0, workKi = 0, workKd = 0;
  double max = 0, min = 10000;

#ifdef ENABLE_SERIAL
  Serial.println("AT");
#endif

  offHeater(); // switch off all heaters.

  PIDOutput[hotend] = bias = d = SOFT_PWM_WINDOW_SIZE >> 1;

  bool wait_for_heatup = true;

  // PID Tuning loop
  while (wait_for_heatup)
  {

    ms = millis();

    updateTemp();
    input = PIDInput[hotend];

    if (max < input)
      max = input;
    if (min > input)
      min = input;

    if (heating && input > temp)
    {
      if (ms > t2 + 5000UL)
      {
        heating = false;
        PIDOutput[hotend] = (bias - d) >> 1;
        t1 = ms;
        t_high = t1 - t2;
        max = temp;
      }
    }

    if (!heating && input < temp)
    {
      if (ms > t1 + 5000UL)
      {
        heating = true;
        t2 = ms;
        t_low = t2 - t1;
        if (cycles > 0)
        {
          long max_pow = SOFT_PWM_WINDOW_SIZE;
          bias += (d * (t_high - t_low)) / (t_low + t_high);
          bias = constrain(bias, 20, max_pow - 20);
          d = (bias > max_pow / 2) ? max_pow - 1 - bias : bias;

          bias = SOFT_PWM_WINDOW_SIZE >> 1;
          d = SOFT_PWM_WINDOW_SIZE >> 1; // Shek: hard-code fixed bias and d because the autocalculation resulted in thermal runaway (the power of "turning off" causes continuous heat up).

#ifdef ENABLE_SERIAL
          Serial.print(F(SERIAL_AT_BIAS));
          Serial.print(bias);
          Serial.println();
          Serial.print(F(SERIAL_AT_D));
          Serial.print(d);
          Serial.println();
          Serial.print(F(SERIAL_AT_MIN));
          Serial.print(min);
          Serial.println();
          Serial.print(F(SERIAL_AT_MAX));
          Serial.print(max);
          Serial.println();
#endif

          if (cycles > 2)
          {
            Ku = (4.0 * d) / (M_PI * (max - min) * 0.5);
            Tu = ((double)(t_low + t_high) * 0.001);
            workKp = 0.6 * Ku;
            workKi = 2 * workKp / Tu;
            workKd = workKp * Tu * 0.125;
#ifdef ENABLE_SERIAL
            Serial.print(F(SERIAL_AT_KU));
            Serial.println(Ku);
            Serial.print(F(SERIAL_AT_TU));
            Serial.println(Tu);
            Serial.println(F(SERIAL_AT_CLASSIC_PID));
            Serial.print(F(SERIAL_AT_KP));
            Serial.println(workKp);
            Serial.print(F(SERIAL_AT_KI));
            Serial.println(workKi);
            Serial.print(F(SERIAL_AT_KD));
            Serial.println(workKd);
#endif
          }
        }
        PIDOutput[hotend] = (bias + d) >> 1;
        cycles++;
        min = temp;
      }
    }
#define MAX_OVERSHOOT_PID_AUTOTUNE 20
    if (input > temp + MAX_OVERSHOOT_PID_AUTOTUNE)
    {
#ifdef ENABLE_SERIAL
      Serial.println(F(SERIAL_AT_OVERSHOOT));
#endif
      return;
    }
    // Every 2 seconds...
    if (ms > temp_ms + 2000UL)
    {
#ifdef ENABLE_SERIAL
      Serial.print("T");
      Serial.print(hotend);
      Serial.print(": ");
      Serial.print(input);
      Serial.print(" @: ");
      Serial.println(PIDOutput[hotend]);
      /*char tbuf[6], outbuf[19];
      ftoa(tbuf,input,2);
      sprintf_P(outbuf, PSTR(SERIAL_AT_TEMP_OUT), hotend, tbuf, (int) PIDOutput[hotend]);
      Serial.println(outbuf);*/
#endif
      temp_ms = ms;
    }
    // every 2 seconds
    // Over 10 minutes?
    if (((ms - t1) + (ms - t2)) > (10L * 60L * 1000L * 10L))
    {
#ifdef ENABLE_SERIAL
      Serial.println(F(SERIAL_AT_TIMEOUT));
#endif
      return;
    }
    if (cycles > ncycles)
    {
#ifdef ENABLE_SERIAL
      Serial.println(F(SERIAL_AT_FINISH));
#endif
      if (set_result)
      {
        PIDKp[hotend] = workKp;
        PIDKi[hotend] = workKi;
        PIDKd[hotend] = workKd;
        PIDHeater[hotend].SetTunings(PIDKp[hotend], PIDKi[hotend], PIDKd[hotend]);
        printPIDSetting(hotend);
      }
      // update PID etc.
      return;
    }
    // lcd_update();
    if (!wait_for_heatup)
    {
      offHeater();
    }
    else
    {
      manageHeaterSoftPWM();
    }
  }
}

extern void setFan(bool state)
{
  digitalWrite(FAN_PIN, state);
}

extern void offHeater()
{
  for (int i = 0; i < 2; i++)
  {
    PIDSetpoint[i] = 0;
    PIDHeater[i].SetMode(MANUAL);
  }
#ifdef ENABLE_LCD
  statusLCD(F(STATUS_READY));
  logoLCD(-1);
#endif
#ifdef ENABLE_SERIAL
  Serial.println(SERIAL_OFF_ALL_HEATER);
#endif
}

extern void printPIDSetting(int idxHeater)
{
#ifdef ENABLE_SERIAL
  Serial.print(F(SERIAL_PRINT_PID));
  Serial.print(idxHeater);
  Serial.print(F(":"));
  Serial.print(PIDHeater[idxHeater].GetKp(), 2);
  Serial.print(F(SERIAL_PRINT_KP));
  Serial.print(PIDHeater[idxHeater].GetKi(), 2);
  Serial.print(F(SERIAL_PRINT_KI));
  Serial.print(PIDHeater[idxHeater].GetKd(), 2);
  Serial.print(F(SERIAL_PRINT_KD));
  Serial.println();
#endif
}

extern void setHeaterTemp(int idxHeater, int newSetpoint)
{
  PIDSetpoint[idxHeater] = newSetpoint;
  PIDHeater[idxHeater].SetMode((newSetpoint <= 0) ? MANUAL : AUTOMATIC);
  // char serialbuffer[45];
  // sprintf_P(serialbuffer, PSTR(SERIAL_SET_HEATER_TO_TEMP), idxHeater, newSetpoint);
  //#ifdef ENABLE_SERIAL
  //   Serial.println(serialbuffer);
  //#endif
}

#endif