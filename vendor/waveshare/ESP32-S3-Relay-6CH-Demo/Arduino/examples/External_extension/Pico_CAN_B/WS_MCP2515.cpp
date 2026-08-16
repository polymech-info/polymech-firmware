#include "WS_MCP2515.h"

SPIClass spi;                     
MCP_CAN CAN(&spi, MCP2515_CS_PIN);    

uint8_t buf[100] = {0};
uint8_t Flag_Init = 1;
long unsigned int canId = 0;
uint32_t Count = 0;

uint8_t receiveCANData(long unsigned int* canId, uint8_t* data) {
  if (CAN.checkReceive() == CAN_MSGAVAIL) {                 // Check whether CAN data is received
    uint8_t len = 0;
    if (CAN.readMsgBuf(canId, &len, data) == CAN_OK) {     
      printf("CAN ID: 0x%x  Data: ",*canId);
      for(int i = 0; i < len; i++) {
        printf("%x  ", buf[i]);
      }
      printf(" \r\n");
      return len;
    }
  }
  return 0;
}
uint8_t sendCANData(long unsigned int canId, uint8_t len, uint8_t* data) {
  if(CAN.sendMsgBuf(canId, CAN_LEN, data) == CAN_OK) {  
    printf("CAN message sent successfully!\r\n");
    return CAN_OK;
  } else {
    printf("Failed to send CAN message!\r\n");
    return 0;
  }
}
void MCP2515_Init() {

  spi.begin(SPI_CLK_PIN,SPI_MISO_PIN,SPI_MOSI_PIN);

  // Example Initialize the MCP2515 device
  if (CAN.begin(MCP_ANY, CAN_125KBPS, MCP_16MHZ) == CAN_OK){
    printf("MCP2515 Initialized Successfully!\r\n");
    CAN.setMode(MCP_NORMAL);                                    // Set MCP2515 to normal mode
    printf("MCP2515 set to Normal Mode!\r\n");
    Flag_Init = 1;
  }
  else{
    Flag_Init = 0;
    printf("Error Initializing MCP2515...\r\n");
  }
}

void MCP2515_Loop() { 
  if(Flag_Init){
    uint8_t len = receiveCANData(&canId, buf);
    if(len)
      printf("Received data: %d bytes of data were received\r\n\r\n",len);
    Count++;
    if(Count==70000)
    {
      Count = 0;
      unsigned char data[CAN_LEN] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};
      sendCANData(CAN_ID,CAN_LEN,data);    // Sending CAN messages
    }
  }
  else{
    Count++;
    if(Count==600000){
      Count = 0;
      printf("Error Initializing MCP2515...\r\n");
      MCP2515_Init();
    }
  }
}