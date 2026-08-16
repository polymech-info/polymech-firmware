# Wi-Fi Network Settings

This document explains the settings you can configure for the device's Wi-Fi connection. These settings allow the device to connect to your existing Wi-Fi network (as a "Station") and optionally to create its own Wi-Fi network (as an "Access Point"). Properly configuring these is essential for network communication, like accessing the web interface or other network services the device might offer.

These settings are typically saved in a configuration file on the device.

## 1. Station Mode (STA) - Connecting to Your Wi-Fi

"Station Mode" means the device will act like your phone or computer and connect to an existing Wi-Fi network (e.g., your home or office Wi-Fi).

*   **1.1. Wi-Fi Network Name (SSID)** (`sta_ssid`):
    *   **What it is**: This is the name of the Wi-Fi network you want the device to connect to. You should see this name when you search for Wi-Fi networks on your phone or computer.
    *   **Example**: `MyHomeWiFi`, `OfficeGuestNetwork`

*   **1.2. Wi-Fi Password** (`sta_password`):
    *   **What it is**: The password for the Wi-Fi network specified in `sta_ssid`.
    *   **Security Note**: This password is sensitive. It's stored on the device to allow automatic connection. For security reasons, it might not be displayed in logs or some parts of the interface after being set.

*   **1.3. Device IP Address** (`sta_local_IP`):
    *   **What it is**: If you want to assign a specific IP address to this device on your network (instead of letting the router assign one automatically via DHCP), you can set it here. An IP address is like a unique street address for devices on a network.
    *   **Format**: Typically four numbers separated by dots (e.g., `192.168.1.250`).
    *   **Note**: If you set this, make sure the IP address is valid for your network and not already used by another device. If unsure, it's often best to leave this to be assigned automatically by your router (which usually means leaving it blank or set to `0.0.0.0` if the device supports DHCP for static IPs, or relying on DHCP if static IP is not configured).

*   **1.4. Gateway IP Address** (`sta_gateway`):
    *   **What it is**: This is usually the IP address of your Wi-Fi router. It's the "gate" through which the device accesses other networks, including the internet.
    *   **Format**: Four numbers separated by dots (e.g., `192.168.1.1`).
    *   **Note**: This is needed if you are setting a static IP address for the device.

*   **1.5. Subnet Mask** (`sta_subnet`):
    *   **What it is**: This helps define the size of your local network. It works with the IP address and Gateway to manage network traffic.
    *   **Format**: Four numbers separated by dots (e.g., `255.255.255.0` or `255.255.0.0`).
    *   **Note**: This is also usually needed if you are setting a static IP address.

*   **1.6. Primary DNS Server** (`sta_primary_dns`):
    *   **What it is**: The Domain Name System (DNS) server translates human-readable website names (like `www.google.com`) into IP addresses that computers use. This is the main DNS server the device will use.
    *   **Format**: Four numbers separated by dots (e.g., `8.8.8.8` for Google's public DNS).
    *   **Note**: Often, your router provides this automatically. You might set this if you want to use a specific DNS server.

*   **1.7. Secondary DNS Server** (`sta_secondary_dns`):
    *   **What it is**: A backup DNS server to use if the Primary DNS Server isn't reachable.
    *   **Format**: Four numbers separated by dots (e.g., `8.8.4.4` for Google's secondary public DNS).

## 2. Access Point (AP) Mode - Creating Its Own Wi-Fi Network

"Access Point Mode" allows the device to create its own Wi-Fi network. Other devices (like your phone) can then connect directly to this device's Wi-Fi. This is often used for initial configuration or if no other Wi-Fi network is available. This section is relevant if the `ENABLE_AP_STA` option is active in the device's firmware.

*   **2.1. Device's Network Name (SSID)** (`ap_ssid`):
    *   **What it is**: If the device creates its own Wi-Fi network, this is the name that network will broadcast. This is the name you'll look for on your phone/computer to connect to the device directly.
    *   **Example**: `DeviceSetupWiFi`, `MyMachineAP`

*   **2.2. Device's Network Password** (`ap_password`):
    *   **What it is**: The password required to connect to the Wi-Fi network created by the device (specified in `ap_ssid`).
    *   **Security Note**: Choose a secure password if you use this feature regularly.

*   **2.3. Device's AP IP Address** (`ap_config_ip`):
    *   **What it is**: When the device is an Access Point, this will be its IP address on the network it creates. Devices connecting to it will use this IP to communicate with it.
    *   **Format**: Four numbers separated by dots (e.g., `192.168.4.1`).

*   **2.4. AP Gateway IP Address** (`ap_config_gateway`):
    *   **What it is**: For the network created by the device, this is its gateway address. Often, it's the same as the `ap_config_ip`.
    *   **Format**: Four numbers separated by dots (e.g., `192.168.4.1`).

*   **2.5. AP Subnet Mask** (`ap_config_subnet`):
    *   **What it is**: Defines the network size for the Wi-Fi network created by the device.
    *   **Format**: Four numbers separated by dots (e.g., `255.255.255.0`).

---

**Important Notes:**
*   The names in `backticks` (like `sta_ssid`) are the technical names for these settings used in the configuration files or by developers.
*   **Static vs. DHCP**: For STA mode, if you leave IP address, gateway, subnet, and DNS fields blank or as `0.0.0.0` (if supported), the device will likely try to get network settings automatically from your router using DHCP. This is often the easiest approach for most users. Manually setting these (static IP) gives you more control but requires understanding your network's configuration.
*   **AP_STA Mode**: Some devices can operate in `AP_STA` mode, meaning they are connected to your Wi-Fi (STA) *and* simultaneously provide their own Access Point (AP). The AP settings are used if this combined mode is enabled.
*   **Saving Settings**: Changes to these settings usually need to be saved on the device and may require a restart of the device or its network components to take effect.
