#include "WS_MQTT.h"

// The name and password of the WiFi access point
const char* ssid = STASSID;
const char* password = STAPSK;
// Details about devices on the Waveshare cloud
const char* mqtt_server = MQTT_Server;
int PORT = MQTT_Port;
const char* ID = MQTT_ID;   // Defining device ID
char pub[] = MQTT_Pub;      // MQTT release topic
char sub[] = MQTT_Sub;      // MQTT subscribe to topics


WiFiClient espClient;       //MQTT initializes the contents
PubSubClient client(espClient);

StaticJsonDocument<400> sendJson;
StaticJsonDocument<400> readJson;
unsigned long lastUpdateTime = 0;
char msg[MSG_BUFFER_SIZE];
bool WIFI_Connection = 0;
char ipStr[16];

const unsigned long updateInterval = 5000;


// MQTT subscribes to callback functions for processing received messages
void callback(char* topic, byte* payload, unsigned int length) {  
  uint8_t CH_Flag = 0;
  String inputString;
  for (int i = 0; i < length; i++) {
    inputString += (char)payload[i];
  }
  printf("%s\r\n",inputString.c_str());                                     // Format of data sent back by the server {"data":{"CH1":1}}
  int dataBegin = inputString.indexOf("\"data\"");                          // Finds if "data" is present in the string (quotes also)
  if (dataBegin == -1) {
    printf("Missing 'data' field in JSON. - MQTT\r\n");                     
    return;
  }
  int CH_Begin = -1;      
  if (inputString.indexOf("\"CH1\"", dataBegin) != -1){                     
    CH_Flag = 1;
    CH_Begin = inputString.indexOf("\"CH1\"", dataBegin);
  }
  else if (inputString.indexOf("\"CH2\"", dataBegin) != -1){             
    CH_Flag = 2;
    CH_Begin = inputString.indexOf("\"CH2\"", dataBegin);
  }  
  else if (inputString.indexOf("\"CH3\"", dataBegin) != -1){             
    CH_Flag = 3;
    CH_Begin = inputString.indexOf("\"CH3\"", dataBegin);
  }  
  else if (inputString.indexOf("\"CH4\"", dataBegin) != -1){             
    CH_Flag = 4;
    CH_Begin = inputString.indexOf("\"CH4\"", dataBegin);
  }  
  else if (inputString.indexOf("\"CH5\"", dataBegin) != -1){             
    CH_Flag = 5;
    CH_Begin = inputString.indexOf("\"CH5\"", dataBegin);
  }   
  else if (inputString.indexOf("\"CH6\"", dataBegin) != -1){             
    CH_Flag = 6;
    CH_Begin = inputString.indexOf("\"CH6\"", dataBegin);
  }   
  else if (inputString.indexOf("\"ALL\"", dataBegin) != -1){             
    CH_Flag = 7;
    CH_Begin = inputString.indexOf("\"ALL\"", dataBegin);
  }   
  else{
    printf("Note : Non-instruction data was received - MQTT!\r\n");
    CH_Flag = 0;
    return;
  }
  int valueBegin = inputString.indexOf(':', CH_Begin);
  int valueEnd = inputString.indexOf('}', valueBegin);
  if (valueBegin != -1 && valueEnd != -1) {
    if(CH_Flag != 0)
    {
      String ValueStr = inputString.substring(valueBegin + 1, valueEnd);
      int Value = ValueStr.toInt();
      if(CH_Flag < 7){
        if(Value == 1 && Relay_Flag[CH_Flag - 1] == 0){
          uint8_t Data[1]={CH_Flag+48};
          Relay_Analysis(Data,MQTT_Mode);
        }
        else if(Value == 0 && Relay_Flag[CH_Flag - 1] == 1){
          uint8_t Data[1]={CH_Flag+48};
          Relay_Analysis(Data,MQTT_Mode);
        }
      }
      else if(CH_Flag == 7){
        if(Value == 1 && ((Relay_Flag[0] & Relay_Flag[1] & Relay_Flag[2] & Relay_Flag[3] & Relay_Flag[4] & Relay_Flag[5]) == 0)){
          uint8_t Data[1]={7+48};
          Relay_Analysis(Data,MQTT_Mode);
        }
        else if(Value == 0 && ((Relay_Flag[0] | Relay_Flag[1] | Relay_Flag[2] | Relay_Flag[3] | Relay_Flag[4] | Relay_Flag[5] )== 1)){
          uint8_t Data[1]={8+48};
          Relay_Analysis(Data,MQTT_Mode);
        }
      }
    }
  }
}

void setup_wifi() {
  uint8_t Count = 0;
  printf("Connecting to ");
  printf("%s\r\n",ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Count++;
    delay(500);
    printf(".\r\n");
    if(Count % 2 == 0 && Count != 0){                 
      RGB_Light(60, 0, 0); 
      delay(1000);
      RGB_Light(0, 0, 0); 
    }
    if(Count % 10 == 0){                                // 10 attempts failed to connect, cancel the connection, try again
      WiFi.disconnect();
      delay(100);
      WiFi.mode(WIFI_OFF);
      delay(100);
      WiFi.mode(WIFI_STA);
      delay(100);
      WiFi.begin(ssid, password);
    }
    if(Count > 22){                                     // connection fail
      break;
    } 
  }
  delay(100);
  if(Count < 23){
    WIFI_Connection = 1;
    RGB_Light(0, 60, 0); 
    delay(1000);
    RGB_Light(0, 0, 0);    
    // IPAddress myIP = WiFi.localIP();
    // printf("AP IP address: ");
    // sprintf(ipStr, "%d.%d.%d.%d", myIP[0], myIP[1], myIP[2], myIP[3]);
    // printf("%s\r\n", ipStr);
    printf("WIFI connection is successful, relay control can be performed via Waveshare cloud.\r\n");
  }
  else{
    WIFI_Connection = 0;
    printf("WIFI connection fails, you can use the Bluetooth debugging Assistant to control the device.\r\n");
    RGB_Light(60, 0, 0); 
  }  
}

// Reconnect to the MQTT server
void reconnect() {
  uint8_t Count = 0;
  while (!client.connected()) {
    Count++;    
    if (client.connect(ID)) {
      client.subscribe(sub);
      printf("Waveshare Cloud connection is successful and now you can use all features.\r\n"); 
    } 
    else{
      delay(500);
      if(Count % 2 == 0 && Count != 0){ 
        printf("%d\r\n", client.state());               
        RGB_Light(60, 0, 60); 
        delay(1000);
        RGB_Light(0, 0, 0);
      } 
      if(Count % 10 == 0){                               // 10 attempts failed to connect, cancel the connection, try again
        client.disconnect();
        delay(100);
        client.setServer(mqtt_server, PORT);
        delay(100);
        client.setCallback(callback);
        delay(100);
      }
      if(Count > 32){                                   // connection fail
        Count = 0;
        printf("warning: Waveshare cloud connection fails. Currently, only Bluetooth control is available !!!\r\n"); 
      } 

    }
  }
}
// Send data in JSON format to MQTT server
void sendJsonData() {
  sendJson["ID"] = ID;
  String pubres;
  serializeJson(sendJson, pubres);
  int str_len = pubres.length() + 1;
  char char_array[str_len];
  pubres.toCharArray(char_array, str_len);
  client.publish(pub, char_array);
}
void MQTT_Init()
{
  setup_wifi();
  if(WIFI_Connection == 1){
    client.setServer(mqtt_server, PORT);
    client.setCallback(callback);
  }
}

void MQTT_Loop()
{
  if(WIFI_Connection == 1){
    if (!client.connected()) {
      reconnect();
    }
    client.loop();
    // if (millis() - lastUpdateTime > updateInterval) {  // Periodic data reporting
    //   sendJsonData();
    //   lastUpdateTime = millis();
    // }
  }
}
