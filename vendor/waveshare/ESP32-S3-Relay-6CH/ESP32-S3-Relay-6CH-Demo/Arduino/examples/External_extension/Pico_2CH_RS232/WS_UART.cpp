#include "WS_UART.h"
#include <algorithm>

HardwareSerial lidarSerial2(2);  // Using serial port 1
HardwareSerial lidarSerial3(0);  // Using serial port 1

uint8_t buf2[20] = {0};          // Data storage area
uint8_t buf3[20] = {0};          // Data storage area

void SetData2(uint8_t* data, size_t length) {
  lidarSerial2.write(data, length);                          // Send data from the RS485
}
void SetData3(uint8_t* data, size_t length) {
  lidarSerial3.write(data, length);                          // Send data from the RS485
}
void Extension_Init()
{
  // Serial.begin(115200);                                  // Initializing serial port
  lidarSerial2.begin(115200, SERIAL_8N1, RXD2, TXD2);        // Initializing serial port
  lidarSerial3.begin(115200, SERIAL_8N1, RXD3, TXD3);        // Initializing serial port
}

void Extension_Loop()
{
  uint8_t Receive_Flag2 = 0,Receive_Flag3 = 0;       // Receiving mark
  Receive_Flag2 = lidarSerial2.available();
  Receive_Flag3 = lidarSerial3.available();
  if (Receive_Flag2 > 0) {
    lidarSerial2.readBytes(buf2, Receive_Flag2);              // The Receive_Flag length is read
    printf("Channel 0: %s\r\n",buf2);
    Receive_Flag2=0;
    memset(buf2,0, sizeof(buf2));
  } 
  if (Receive_Flag3 > 0) {
    lidarSerial3.readBytes(buf3, Receive_Flag3);              // The Receive_Flag length is read
    printf("Channel 1: %s\r\n",buf3);
    Receive_Flag3=0;
    memset(buf3,0, sizeof(buf3));
  }
}