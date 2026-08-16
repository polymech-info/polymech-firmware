#ifndef _WS_INFORMATION_H_
#define _WS_INFORMATION_H_

#define Extension_Enable      1                   // Whether to extend the connection to external devices   1:Expansion device Modbus RTU Relay    0:No extend
#define RTC_Enable            0                   // Whether WIFI is enabled  .1:ture    0:false

// The name and password of the WiFi access point
#define STASSID       "Waveshare-TL-WR886N_2L"
#define STAPSK        "waveshare0755"

// Details about devices on the Waveshare cloud
#define MQTT_Server   "mqtt.waveshare.cloud"
#define MQTT_Port     1883
#define MQTT_ID       "46ce3503"
#define MQTT_Pub      "Pub/59/45/46ce3503"
#define MQTT_Sub      "Sub/59/45/46ce3503"


// !!! Timing switch relay, this operation must be connected to the Pico-RTC-DS3231 with the correct time setting,
// RTC_Enable must be set to 1. If no change is required, set RTC_Enable to 0
#define RTC_OPEN_Time_Hour    18              // Time when the timing switch is enabled
#define RTC_OPEN_Time_Min     33              // Time when the timing switch is enabled
#define RTC_Closs_Time_Hour   18              // Time when the timing switch is off
#define RTC_Closs_Time_Min    34              // Time when the timing switch is off

#endif