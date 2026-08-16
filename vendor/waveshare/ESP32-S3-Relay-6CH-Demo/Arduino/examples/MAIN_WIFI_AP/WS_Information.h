#ifndef _WS_INFORMATION_H_
#define _WS_INFORMATION_H_

#define Extension_Enable      1                   // Whether to extend the connection to external devices   1:Expansion device Modbus RTU Relay    0:No extend
#define RTC_Enable            0                   // Whether WIFI is enabled  .1:ture    0:false

// The name and password of the WiFi access point
#define APSSID       "ESP32-S3-Relay-6CH"
#define APPSK        "waveshare"

// !!! Timing switch relay, this operation must be connected to the Pico-RTC-DS3231 with the correct time setting
// RTC_Enable must be set to 1. If no change is required, set RTC_Enable to 0
#define RTC_OPEN_Time_Hour    8              // Time when the timing switch is enabled
#define RTC_OPEN_Time_Min     6              // Time when the timing switch is enabled
#define RTC_Closs_Time_Hour   16              // Time when the timing switch is off
#define RTC_Closs_Time_Min    30              // Time when the timing switch is off

#endif