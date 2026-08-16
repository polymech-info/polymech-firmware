#ifndef _WS_MQTT_H_
#define _WS_MQTT_H_

#include <ArduinoJson.h>
// #include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <WebServer.h> 
#include <WiFiClientSecure.h>
#include "WS_GPIO.h"
#include "WS_Information.h"

#define MQTT_Mode            3
#define WIFI_Mode            3


extern PubSubClient client;
extern WiFiClient espClient;

extern WebServer server;

extern bool Relay_Flag[6];       // Relay current status flag
extern void Relay_Analysis(uint8_t *buf,uint8_t Mode_Flag);


/************************************************** Web *********************************************/
void handleRoot();
void handleGetData();
void handleSwitch(int ledNumber);

void handleSwitch1();
void handleSwitch2();
void handleSwitch3();
void handleSwitch4();
void handleSwitch5();
void handleSwitch6();
void handleSwitch7();
void handleSwitch8();

/************************************************** MQTT *********************************************/
void callback(char* topic, byte* payload, unsigned int length);   // MQTT subscribes to callback functions for processing received messages
void setup_wifi();
void reconnect();                                                 // Reconnect to the MQTT server
void sendJsonData();                                              // Send data in JSON format to MQTT server
void MQTT_Init();
void MQTT_Loop();

#endif