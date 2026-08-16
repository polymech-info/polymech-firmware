#ifndef WIFI_NETWORK_SETTINGS_H
#define WIFI_NETWORK_SETTINGS_H

#include <Arduino.h>      // Should be first for core Arduino types
#include <IPAddress.h>    // For IPAddress
#include <WString.h>      // Explicitly include for String, though Arduino.h often covers it
#include <ArduinoJson.h>  // For JSON functionality
#include <ArduinoLog.h>   // For logging macros

#include "config.h"      // For WIFI_SSID, WIFI_PASSWORD, AP_SSID, AP_PASSWORD, ENABLE_AP_STA
#include "enums.h"       // For E_OK
#include "Logger.h"      // For Log object (if Log is a custom object wrapper)

// Struct to hold all WiFi Network Settings
struct WiFiNetworkSettings {
    // STA Configuration
    String sta_ssid;
    String sta_password;
    IPAddress sta_local_IP;
    IPAddress sta_gateway;
    IPAddress sta_subnet;
    IPAddress sta_primary_dns;
    IPAddress sta_secondary_dns;

    // AP Configuration (for AP_STA mode)
    String ap_ssid;
    String ap_password;
    IPAddress ap_config_ip;
    IPAddress ap_config_gateway;
    IPAddress ap_config_subnet;

    WiFiNetworkSettings() {
        // Initialize STA settings with defaults from config.h values
        sta_ssid = WIFI_SSID;
        sta_password = WIFI_PASSWORD;
        sta_local_IP = IPAddress(192, 168, 1, 250);    // Direct initialization
        sta_gateway = IPAddress(192, 168, 1, 1);      // Direct initialization
        sta_subnet = IPAddress(255, 255, 0, 0);        // Direct initialization
        sta_primary_dns = IPAddress(8, 8, 8, 8);        // Direct initialization
        sta_secondary_dns = IPAddress(8, 8, 4, 4);      // Direct initialization

#ifdef ENABLE_AP_STA
        ap_ssid = AP_SSID;
        ap_password = AP_PASSWORD;
        ap_config_ip = IPAddress(192, 168, 4, 1);       // Direct initialization
        ap_config_gateway = IPAddress(192, 168, 4, 1);   // Direct initialization
        ap_config_subnet = IPAddress(255, 255, 255, 240); // Direct initialization (using the last attempted /28 value)
#else
        ap_ssid = "";
        ap_password = "";
        // IPAddress members will be default constructed (0.0.0.0)
#endif
    }

    void print() const {
        Log.infoln("--- WiFiNetworkSettings Dump ---");
        Log.infoln("STA SSID: %s", sta_ssid.c_str());
        // Note: STA password is not logged for security.
        Log.infoln("STA IP: %s", sta_local_IP.toString().c_str());
        Log.infoln("STA Gateway: %s", sta_gateway.toString().c_str());
        Log.infoln("STA Subnet: %s", sta_subnet.toString().c_str());
        Log.infoln("STA DNS1: %s", sta_primary_dns.toString().c_str());
        Log.infoln("STA DNS2: %s", sta_secondary_dns.toString().c_str());

#if defined(ENABLE_AP_STA)
        Log.infoln("AP SSID: %s", ap_ssid.c_str());
        // Note: AP password is not logged for security.
        Log.infoln("AP IP: %s", ap_config_ip.toString().c_str());
        Log.infoln("AP Gateway: %s", ap_config_gateway.toString().c_str());
        Log.infoln("AP Subnet: %s", ap_config_subnet.toString().c_str());
#else
        Log.infoln("AP_STA mode not enabled, AP settings not actively used by setupNetwork.");
#endif
        Log.infoln("--- End WiFiNetworkSettings Dump ---");
    }

    short loadSettings(JsonObject& doc) {        
        Log.infoln("WiFiNetworkSettings::load - Loading WiFi settings from JSON...");
        IPAddress tempIp;

        // STA Settings
        JsonVariant sta_ssid_val = doc["sta_ssid"];
        if (sta_ssid_val.is<const char*>()) {
            sta_ssid = sta_ssid_val.as<String>();
            Log.infoln("Loaded sta_ssid: %s", sta_ssid.c_str());
        } else if (!sta_ssid_val.isNull()) {
            Log.warningln("Value for 'sta_ssid' is not a string.");
        }

        JsonVariant sta_password_val = doc["sta_password"];
        if (sta_password_val.is<const char*>()) {
            sta_password = sta_password_val.as<String>();
            // Avoid logging password: Log.infoln("Loaded sta_password");
        } else if (!sta_password_val.isNull()) {
            Log.warningln("Value for 'sta_password' is not a string.");
        }

        JsonVariant sta_local_ip_val = doc["sta_local_ip"];
        if (sta_local_ip_val.is<const char*>()) {
            if (tempIp.fromString(sta_local_ip_val.as<const char*>())) {
                sta_local_IP = tempIp;
                Log.infoln("Loaded sta_local_IP: %s", sta_local_IP.toString().c_str());
            } else {
                Log.warningln("Failed to parse IP from string for 'sta_local_ip': %s", sta_local_ip_val.as<const char*>());
            }
        } else if (!sta_local_ip_val.isNull()) {
            Log.warningln("Value for 'sta_local_ip' is not a string, cannot parse as IP.");
        }

        JsonVariant sta_gateway_val = doc["sta_gateway"];
        if (sta_gateway_val.is<const char*>()) {
            if (tempIp.fromString(sta_gateway_val.as<const char*>())) {
                sta_gateway = tempIp;
                Log.infoln("Loaded sta_gateway: %s", sta_gateway.toString().c_str());
            } else {
                Log.warningln("Failed to parse IP from string for 'sta_gateway': %s", sta_gateway_val.as<const char*>());
            }
        } else if (!sta_gateway_val.isNull()) {
            Log.warningln("Value for 'sta_gateway' is not a string, cannot parse as IP.");
        }

        JsonVariant sta_subnet_val = doc["sta_subnet"];
        if (sta_subnet_val.is<const char*>()) {
            if (tempIp.fromString(sta_subnet_val.as<const char*>())) {
                sta_subnet = tempIp;
                Log.infoln("Loaded sta_subnet: %s", sta_subnet.toString().c_str());
            } else {
                Log.warningln("Failed to parse IP from string for 'sta_subnet': %s", sta_subnet_val.as<const char*>());
            }
        } else if (!sta_subnet_val.isNull()) {
            Log.warningln("Value for 'sta_subnet' is not a string, cannot parse as IP.");
        }

        JsonVariant sta_primary_dns_val = doc["sta_primary_dns"];
        if (sta_primary_dns_val.is<const char*>()) {
            if (tempIp.fromString(sta_primary_dns_val.as<const char*>())) {
                sta_primary_dns = tempIp;
                Log.infoln("Loaded sta_primary_dns: %s", sta_primary_dns.toString().c_str());
            } else {
                Log.warningln("Failed to parse IP from string for 'sta_primary_dns': %s", sta_primary_dns_val.as<const char*>());
            }
        } else if (!sta_primary_dns_val.isNull()) {
            Log.warningln("Value for 'sta_primary_dns' is not a string, cannot parse as IP.");
        }

        JsonVariant sta_secondary_dns_val = doc["sta_secondary_dns"];
        if (sta_secondary_dns_val.is<const char*>()) {
            if (tempIp.fromString(sta_secondary_dns_val.as<const char*>())) {
                sta_secondary_dns = tempIp;
                Log.infoln("Loaded sta_secondary_dns: %s", sta_secondary_dns.toString().c_str());
            } else {
                Log.warningln("Failed to parse IP from string for 'sta_secondary_dns': %s", sta_secondary_dns_val.as<const char*>());
            }
        } else if (!sta_secondary_dns_val.isNull()) {
            Log.warningln("Value for 'sta_secondary_dns' is not a string, cannot parse as IP.");
        }

#ifdef ENABLE_AP_STA
        // AP Settings
        JsonVariant ap_ssid_val = doc["ap_ssid"];
        if (ap_ssid_val.is<const char*>()) {
            ap_ssid = ap_ssid_val.as<String>();
            Log.infoln("Loaded ap_ssid: %s", ap_ssid.c_str());
        } else if (!ap_ssid_val.isNull()) {
            Log.warningln("Value for 'ap_ssid' is not a string.");
        }

        JsonVariant ap_password_val = doc["ap_password"];
        if (ap_password_val.is<const char*>()) {
            ap_password = ap_password_val.as<String>();
            // Avoid logging password: Log.infoln("Loaded ap_password");
        } else if (!ap_password_val.isNull()) {
            Log.warningln("Value for 'ap_password' is not a string.");
        }

        JsonVariant ap_config_ip_val = doc["ap_config_ip"];
        if (ap_config_ip_val.is<const char*>()) {
            if (tempIp.fromString(ap_config_ip_val.as<const char*>())) {
                ap_config_ip = tempIp;
                Log.infoln("Loaded ap_config_ip: %s", ap_config_ip.toString().c_str());
            } else {
                Log.warningln("Failed to parse IP from string for 'ap_config_ip': %s", ap_config_ip_val.as<const char*>());
            }
        } else if (!ap_config_ip_val.isNull()) {
            Log.warningln("Value for 'ap_config_ip' is not a string, cannot parse as IP.");
        }

        JsonVariant ap_config_gateway_val = doc["ap_config_gateway"];
        if (ap_config_gateway_val.is<const char*>()) {
            if (tempIp.fromString(ap_config_gateway_val.as<const char*>())) {
                ap_config_gateway = tempIp;
                Log.infoln("Loaded ap_config_gateway: %s", ap_config_gateway.toString().c_str());
            } else {
                Log.warningln("Failed to parse IP from string for 'ap_config_gateway': %s", ap_config_gateway_val.as<const char*>());
            }
        } else if (!ap_config_gateway_val.isNull()) {
            Log.warningln("Value for 'ap_config_gateway' is not a string, cannot parse as IP.");
        }

        JsonVariant ap_config_subnet_val = doc["ap_config_subnet"];
        if (ap_config_subnet_val.is<const char*>()) {
            if (tempIp.fromString(ap_config_subnet_val.as<const char*>())) {
                ap_config_subnet = tempIp;
                Log.infoln("Loaded ap_config_subnet: %s", ap_config_subnet.toString().c_str());
            } else {
                Log.warningln("Failed to parse IP from string for 'ap_config_subnet': %s", ap_config_subnet_val.as<const char*>());
            }
        } else if (!ap_config_subnet_val.isNull()) {
            Log.warningln("Value for 'ap_config_subnet' is not a string, cannot parse as IP.");
        }
#endif
        Log.infoln("WiFiNetworkSettings::load - Finished loading WiFi settings.");
        return E_OK;
    }

    JsonDocument toJSON() const {
        JsonDocument doc; // Using dynamic allocation for ArduinoJson v6+

        doc["sta_ssid"] = sta_ssid;
        // doc["sta_password"] = sta_password; // Omit password for security
        doc["sta_local_ip"] = sta_local_IP.toString();
        doc["sta_gateway"] = sta_gateway.toString();
        doc["sta_subnet"] = sta_subnet.toString();
        doc["sta_primary_dns"] = sta_primary_dns.toString();
        doc["sta_secondary_dns"] = sta_secondary_dns.toString();

#ifdef ENABLE_AP_STA
        doc["ap_ssid"] = ap_ssid;
        // doc["ap_password"] = ap_password; // Omit password for security
        doc["ap_config_ip"] = ap_config_ip.toString();
        doc["ap_config_gateway"] = ap_config_gateway.toString();
        doc["ap_config_subnet"] = ap_config_subnet.toString();
#else
        // Consistently represent AP settings even if not enabled
        doc["ap_ssid"] = ""; // Or consider omitting if not defined by ENABLE_AP_STA
        // doc["ap_password"] = "";
        doc["ap_config_ip"] = "0.0.0.0";
        doc["ap_config_gateway"] = "0.0.0.0";
        doc["ap_config_subnet"] = "0.0.0.0";
#endif
        return doc;
    }
};

#endif // WIFI_NETWORK_SETTINGS_H 