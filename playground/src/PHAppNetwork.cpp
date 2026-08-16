#include <Arduino.h>
#include <macros.h>
#include <Component.h>
#include <enums.h>
#include <ArduinoLog.h>
#include "./PHApp.h"
#include <ESPmDNS.h> 
#include <LittleFS.h>    
#include <ArduinoJson.h> 

#include "./config.h"
#include "./config_adv.h"
#include "./config-modbus.h"
#include "./features.h"

#ifdef ENABLE_PROCESS_PROFILE
#include "profiles/PlotBase.h"
#include "profiles/SignalPlot.h"
#include "profiles/TemperatureProfile.h"
#endif

#include <modbus/ModbusTCP.h>
#include <modbus/ModbusTypes.h>

#ifdef ENABLE_WIFI
#include <WiFi.h>
#ifdef ENABLE_WEBSERVER
#include "components/RestServer.h"
#endif
#endif

short PHApp::loadNetworkSettings() {
    Log.infoln(F("PHApp::loadNetworkSettings() - Attempting to load network configuration from LittleFS..."));
    if (!LittleFS.begin(true)) { // Ensure LittleFS is mounted (true formats if necessary)
        Log.errorln(F("PHApp::loadNetworkSettings() - Failed to mount LittleFS. Cannot load network configuration."));
        wifiSettings.print(); // Print defaults before returning
        return E_FATAL; // Use E_FATAL for critical FS failure
    }

    File configFile = LittleFS.open(NETWORK_CONFIG_FILENAME, "r");
    if (!configFile) {
        Log.warningln(F("PHApp::loadNetworkSettings() - Failed to open network config file: %s. Using default settings."), NETWORK_CONFIG_FILENAME);
        LittleFS.end(); // Close LittleFS
        wifiSettings.print(); // Print defaults before returning
        return E_NOT_FOUND; // Indicates file wasn't found, defaults will be used.
    }

    Log.infoln(F("PHApp::loadNetworkSettings() - Opened network config file: %s"), NETWORK_CONFIG_FILENAME);

    JsonDocument doc; // Using JsonDocument for automatic memory management

    DeserializationError error = deserializeJson(doc, configFile);
    configFile.close(); // Close the file as soon as possible

    if (error) {
        Log.errorln(F("PHApp::loadNetworkSettings() - Failed to parse network config JSON: %s. Using default settings."), error.c_str());
        LittleFS.end(); // Close LittleFS
        wifiSettings.print(); // Print defaults before returning
        return E_INVALID_PARAMETER; // Indicates a parsing error, defaults will be used.
    }

    JsonObject root = doc.as<JsonObject>();
    if (root.isNull()) {
        Log.errorln(F("PHApp::loadNetworkSettings() - Network config JSON root is not an object. Using default settings."));
        LittleFS.end();
        wifiSettings.print(); // Print defaults before returning
        return E_INVALID_PARAMETER;
    }
    
    Log.infoln(F("PHApp::loadNetworkSettings() - Successfully parsed network config file. Applying settings..."));
    short loadResult = wifiSettings.loadSettings(root); // Call the existing method in WiFiNetworkSettings

    LittleFS.end(); // Ensure LittleFS is closed after operations

    if (loadResult == E_OK) {
        Log.infoln(F("PHApp::loadNetworkSettings() - Network settings loaded successfully from %s."), NETWORK_CONFIG_FILENAME);
    } else {
        Log.warningln(F("PHApp::loadNetworkSettings() - Issues applying parsed network settings. Some defaults may still be in use."));
    }
    wifiSettings.print(); // Print settings after attempting to load them
    return loadResult; 
}

short PHApp::saveNetworkSettings(JsonObject& doc) {
    Log.infoln(F("PHApp::saveNetworkSettings() - Attempting to save network configuration to LittleFS..."));

    if (!LittleFS.begin(true)) { // Ensure LittleFS is mounted
        Log.errorln(F("PHApp::saveNetworkSettings() - Failed to mount LittleFS. Cannot save network configuration."));
        return E_FATAL; // Or a more specific LittleFS error
    }

    File configFile = LittleFS.open(NETWORK_CONFIG_FILENAME, "w"); // Open for writing, creates if not exists, truncates if exists
    if (!configFile) {
        Log.errorln(F("PHApp::saveNetworkSettings() - Failed to open network config file '%s' for writing."), NETWORK_CONFIG_FILENAME);
        LittleFS.end(); // Close LittleFS
        return E_FATAL; // Replaced E_FS_ERROR with E_FATAL
    }

    Log.infoln(F("PHApp::saveNetworkSettings() - Opened/created network config file: %s for writing."), NETWORK_CONFIG_FILENAME);

    size_t bytesWritten = serializeJson(doc, configFile);
    configFile.close(); // Close the file as soon as possible

    if (bytesWritten > 0) {
        Log.infoln(F("PHApp::saveNetworkSettings() - Successfully wrote %d bytes to %s."), bytesWritten, NETWORK_CONFIG_FILENAME);
    } else {
        Log.errorln(F("PHApp::saveNetworkSettings() - Failed to serialize JSON to file or wrote 0 bytes to %s."), NETWORK_CONFIG_FILENAME);
        LittleFS.end(); // Close LittleFS
        // Attempt to remove the (potentially empty or corrupted) file if serialization failed.
        if (LittleFS.exists(NETWORK_CONFIG_FILENAME)) {
            LittleFS.remove(NETWORK_CONFIG_FILENAME);
        }
        return E_INVALID_PARAMETER; // Or a more specific serialization error
    }

    LittleFS.end(); // Ensure LittleFS is closed after operations
    Log.infoln(F("PHApp::saveNetworkSettings() - Network settings saved successfully to %s."), NETWORK_CONFIG_FILENAME);
    // Optionally, after saving, you might want to immediately reload and apply these settings:
    // loadNetworkSettings(); 
    // Or, signal that a restart is needed for settings to take full effect if they are only read at boot.
    return E_OK;
}

short PHApp::setupNetwork()
{
  loadNetworkSettings(); // Load settings from LittleFS first
  bool sta_connected = false;
  bool ap_started = false;

#if defined(ENABLE_AP_STA)
  WiFi.mode(WIFI_AP_STA);
  Log.infoln("Setting up AP_STA with SSID: %s", wifiSettings.ap_ssid.c_str());
  if (!WiFi.softAPConfig(wifiSettings.ap_config_ip, wifiSettings.ap_config_gateway, wifiSettings.ap_config_subnet))
  {
    Log.errorln("AP Failed to configure");
  }
  else
  {
    if (!WiFi.softAP(wifiSettings.ap_ssid.c_str(), wifiSettings.ap_password.c_str()))
    {
      Log.errorln("AP Failed to start");
    }
    else
    {
      Log.infoln("AP IP address: %s", WiFi.softAPIP().toString().c_str());
      ap_started = true;
    }
  }

  // Configure Station (STA) part
  Log.infoln("Configuring STA for AP_STA mode...");
  
  if (!WiFi.config(wifiSettings.sta_local_IP, wifiSettings.sta_gateway, wifiSettings.sta_subnet, wifiSettings.sta_primary_dns, wifiSettings.sta_secondary_dns))
  {
    Log.errorln("STA (for AP_STA) Failed to configure");
  }
  WiFi.begin(wifiSettings.sta_ssid.c_str(), wifiSettings.sta_password.c_str());
  Log.infoln("Attempting to connect to STA WiFi: %s", wifiSettings.sta_ssid.c_str());

  int connect_timeout_ms = 30000;
  unsigned long start_time = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - start_time < connect_timeout_ms))
  {
    delay(100);
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Log.infoln("STA IP address (AP_STA mode): %s", WiFi.localIP().toString().c_str());
    sta_connected = true;
  }
  else
  {
    Log.warningln("STA (for AP_STA) connection failed or timed out. AP is still active.");
  }

#elif defined(ENABLE_WIFI) // STA mode only
  Log.infoln("Configuring WiFi in STA mode...");
  if (!WiFi.config(wifiSettings.sta_local_IP, wifiSettings.sta_gateway, wifiSettings.sta_subnet, wifiSettings.sta_primary_dns, wifiSettings.sta_secondary_dns))
  {
    Log.errorln("STA Failed to configure");
  }
  WiFi.begin(wifiSettings.sta_ssid.c_str(), wifiSettings.sta_password.c_str());
  int connect_timeout_ms = 30000;
  unsigned long start_time = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - start_time < connect_timeout_ms))
  {
    delay(100);
  }
  if (WiFi.status() == WL_CONNECTED)
  {
    Log.infoln("IP address: %s", WiFi.localIP().toString().c_str());
    sta_connected = true;
  }
  else
  {
    Log.errorln("WiFi connection timed out!");
    // return E_WIFI_CONNECTION_FAILED; // Keep network setup going if AP might work or for mDNS on AP
  }
#endif

  // Initialize mDNS
  // It should be started if either STA is connected or AP is successfully started.
  if (sta_connected || ap_started) {
    const char* mdns_hostname = "polymech-cassandra";
    if (MDNS.begin(mdns_hostname)) {
      Log.infoln("mDNS responder started. Hostname: %s", mdns_hostname);
      MDNS.addService("http", "tcp", 80);
      Log.infoln("mDNS service _http._tcp.local on port 80 advertised.");
      Log.infoln("Access the web server at: http://%s.local", mdns_hostname);
    } else {
      Log.errorln("Error starting mDNS responder!");
    }
  } else {
    Log.warningln("Neither STA connected nor AP started. mDNS will not be initialized.");
  }

#ifdef ENABLE_MODBUS_TCP
  setNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS);
  setupModbus();
#else
  modbusManager = nullptr;
#endif

#if defined(ENABLE_WEBSERVER) && defined(ENABLE_MODBUS_TCP)

  if (modbusManager) // Check Modbus dependency first
  {
    IPAddress webserverIP = IPAddress(0,0,0,0);
    bool canStartWebServer = false;

#if defined(ENABLE_AP_STA)
    webserverIP = WiFi.softAPIP(); // IP of the AP interface
    if (webserverIP && webserverIP != IPAddress(0,0,0,0)) {
        Log.infoln("AP_STA mode: Web server will use AP IP: %s", webserverIP.toString().c_str());
        canStartWebServer = true;
    } else {
        Log.errorln("AP_STA mode: Soft AP IP is invalid or not yet available. Cannot determine IP for web server on AP.");
    }
    // Log STA IP for informational purposes if connected
    if (WiFi.status() == WL_CONNECTED) {
        Log.infoln("AP_STA mode: STA interface is also connected with IP: %s", WiFi.localIP().toString().c_str());
        Log.infoln("             External clients (on STA network) might try http://%s", WiFi.localIP().toString().c_str());
    }
#elif defined(ENABLE_WIFI) // STA mode only
    if (WiFi.status() == WL_CONNECTED) {
        webserverIP = WiFi.localIP();
        Log.infoln("STA mode: Web server will use STA IP: %s", webserverIP.toString().c_str());
        canStartWebServer = true;
    } else {
        Log.errorln("STA mode: WiFi not connected. Cannot start web server.");
    }
#else
    // This case should not be hit if ENABLE_WEBSERVER implies one of the WiFi modes for IP-based server.
    Log.warningln("WebServer enabled, but no WiFi mode (AP_STA or STA) is configured to provide an IP address.");
#endif
    if (canStartWebServer) {
        webServer = new RESTServer(webserverIP, 80, modbusManager, this);
        components.push_back(webServer);
        Log.infoln("RESTServer initialized.");
        Log.infoln("Clients connected to the ESP32 (e.g., via AP) should try accessing the server at: http://%s", webserverIP.toString().c_str());
    } else {
        Log.errorln("Cannot initialize RESTServer: No suitable IP address available from current WiFi configuration.");
        webServer = nullptr;
    }
  }
  else
  {
    Log.errorln("Cannot initialize RESTServer: ModbusTCP is null! Ensure Modbus is setup first.");
    webServer = nullptr;
    return E_DEPENDENCY_NOT_MET;
  }
#elif defined(ENABLE_WEBSERVER) && !defined(ENABLE_MODBUS_TCP)
  Log.warningln("WebServer enabled but Modbus TCP is not. RESTServer initialization might be incomplete.");
  webServer = nullptr; // Keep it null if it relies on ModbusTCP
#endif
  return E_OK;
}