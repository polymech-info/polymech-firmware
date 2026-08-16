#ifndef _WS_UART_H_
#define _WS_UART_H_
#include <HardwareSerial.h>       // Reference the ESP32 built-in serial port library

#define TXD2 43                 //The TXD of UART1 corresponds to GPIO
#define RXD2 44                 //The RXD of UART1 corresponds to GPIO

#define TXD3 15                 //The TXD of UART1 corresponds to GPIO
#define RXD3 16                 //The RXD of UART1 corresponds to GPIO




void SetData2(uint8_t* data, size_t length);   // Send data from the Port 0
void SetData3(uint8_t* data, size_t length);   // Send data from the Port 2

void Extension_Init();         // Example Initialize the system serial port
void Extension_Loop();         // Read serial data and print

#endif