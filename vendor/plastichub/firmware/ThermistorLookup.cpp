#include "ThermistorLookup.h"

#define PGM_RD_W(x)   (short)pgm_read_word(&x)

// Derived from RepRap FiveD extruder::getTemperature()
// For hot end temperature measurement.
extern float analog2temp(int raw) {
  if(heater_ttbl_map != NULL)
  {
    float celsius = 0;
    uint8_t i;
    short (*tt)[][2] = (short (*)[][2])(heater_ttbl_map/*[e]*/);

    //for (i=1; i<60; i++)
    for (i=1; i<54; i++)
    {
      if (PGM_RD_W((*tt)[i][0]) > raw)
      {
        celsius = PGM_RD_W((*tt)[i-1][1]) + 
          (raw - PGM_RD_W((*tt)[i-1][0])) * 
          (float)(PGM_RD_W((*tt)[i][1]) - PGM_RD_W((*tt)[i-1][1])) /
          (float)(PGM_RD_W((*tt)[i][0]) - PGM_RD_W((*tt)[i-1][0]));
        break;
      }
    }

    // Overflow: Set to last value in the table
    //if (i == heater_ttbllen_map[e]) celsius = PGM_RD_W((*tt)[i-1][1]);

    return celsius;
  }
}

