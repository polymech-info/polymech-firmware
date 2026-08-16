#ifndef CONFIG_NETWORK_H
#define CONFIG_NETWORK_H

////////////////////////////////////////////////////////////////
//
// Modbus Mirror (master->slave replication)
//

#ifdef ENABLE_MODBUS_MIRROR
#define MODBUS_MIRROR_SERVER_IP {192, 168, 1, 250}
#define MODBUS_MIRROR_SERVER_PORT 502
#define MODBUS_MIRROR_RECONNECT_INTERVAL_MS 5000
#define MODBUS_MIRROR_MAX_RECONNECT_TIME_MS (5 * 60 * 1000)
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Network
//

#define NETWORK_CONFIG_FILENAME "/network.json"
// Static IP Addresses for STA mode (and STA part of AP_STA)
static IPAddress local_IP(192, 168, 1, 250);
static IPAddress gateway(192, 168, 1, 1);
static IPAddress subnet(255, 255, 0, 0);
static IPAddress primaryDNS(8, 8, 8, 8);
static IPAddress secondaryDNS(8, 8, 4, 4);

#define WIFI_SSID "Livebox6-EBCD"
#define WIFI_PASSWORD "c4RK35h4PZNS"

// AP_STA Mode Configuration
// To enable AP_STA mode, define ENABLE_AP_STA.
// If ENABLE_AP_STA is defined, the device will act as both an Access Point
// and a WiFi client (Station) simultaneously.
// The AP will have its own SSID and IP configuration.
// The STA part will connect to the WiFi network defined by WIFI_SSID and WIFI_PASSWORD.

#define ENABLE_AP_STA // Uncomment to enable AP_STA mode
#ifdef ENABLE_AP_STA
#define AP_SSID "PolyMechAP"
#define AP_PASSWORD "poly1234" // Password must be at least 8 characters
static IPAddress ap_local_IP(192, 168, 4, 1);
static IPAddress ap_gateway(192, 168, 4, 1); // Typically the same as ap_local_IP for the AP
static IPAddress ap_subnet(255, 255, 255, 240); // Changed to .240 (/28 subnet)
#endif


#endif