#ifndef IFTTT_WEBHOOK_H
#define IFTTT_WEBHOOK_H

#include <Arduino.h>
#include "config-ifttt.h"

#if defined(ESP32) || defined(ESP8266)
#include <WiFi.h>
#include <HTTPClient.h>
#endif

class IFTTTWebhook
{
public:
    IFTTTWebhook() {}

    void triggerEvent(const String& value1) {
        #if defined(ESP32) || defined(ESP8266)
        if (WiFi.status() == WL_CONNECTED) {
            HTTPClient http;
            
            String url = "https://maker.ifttt.com/trigger/" + String(IFTTT_EVENT_NAME) + "/with/key/" + String(IFTTT_WEBHOOK_KEY);
            
            http.begin(url);
            http.addHeader("Content-Type", "application/json");
            
            String payload = "{\"value1\":\"" + value1 + "\"}";
            
            http.POST(payload);
            http.end();
        }
        #endif
    }
};

#endif // IFTTT_WEBHOOK_H 