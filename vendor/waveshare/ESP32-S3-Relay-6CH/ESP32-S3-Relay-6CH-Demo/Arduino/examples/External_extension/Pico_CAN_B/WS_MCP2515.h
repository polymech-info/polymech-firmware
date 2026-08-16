#ifndef __WS_MCP2515_H_
#define __WS_MCP2515_H_
#include <SPI.h>
#include <mcp_can.h>

#define SPI_CLK_PIN  47
#define SPI_MISO_PIN 15
#define SPI_MOSI_PIN 48
#define MCP2515_CS0_PIN  16
#define MCP2515_CS1_PIN  1
#define MCP2515_CS_PIN  MCP2515_CS0_PIN

#define CAN_ID 0x123                                                // ID of the data sent
#define CAN_LEN 8                                                   // The length of the data sent

uint8_t receiveCANData(long unsigned int* canId, uint8_t* data);             // Parameters: Saves a pointer to the ID, array of data
uint8_t sendCANData(long unsigned int canId, uint8_t len, uint8_t* data);     // Parameters: ID to be sent, length to be sent, data to be sent
void MCP2515_Init();                                                // Example Initialize MCP2515
void MCP2515_Loop();
#endif  
	 
