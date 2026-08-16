#include "WS_UART.h"

uint32_t Simulated_time=0;                      // Analog time counting
uint8_t Port_Data[20] = "hello World";          // Data storage area
void setup() {
  // put your setup code here, to run once:
  Extension_Init();
}

void loop() {
  // put your main code here, to run repeatedly:
  Extension_Loop();
  Simulated_time++;
  if(Simulated_time ==10000){
    Simulated_time = 0;
    SetData2(Port_Data,12);
    SetData3(Port_Data,12);
  }
}
