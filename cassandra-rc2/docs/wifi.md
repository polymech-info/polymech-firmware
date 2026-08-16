## WiFi Setup Guide

This guide will walk you through connecting your device to your local WiFi network.

### Understanding WiFi Modes

*   **Access Point (AP) Mode**: In this mode, the device creates its own WiFi network. You'll connect to this network initially to configure the device.
*   **Station (STA) Mode**: In this mode, the device connects to an existing WiFi network, like your home or office WiFi. This allows you to access the device from other computers or devices on your local network.

### Setup Procedure

1.  **Connect to the Device's AP:**
    *   Power on your device.
    *   On your computer or smartphone, search for available WiFi networks.
    *   Connect to the network named "**PolyMechAP**" (or a similar name related to your device). The default password for this network is `poly1234`.

2.  **Access the Web Interface:**
    *   Once connected to the device's AP, open a web browser (like Chrome, Firefox, or Safari).
    *   In the address bar, type [`192.168.4.1`](http://192.168.4.1) and press Enter. This should open the device's Network Settings page, as shown in the image.

3.  **Configure Station (STA) Mode:**
    *   On the Network Settings page, find the **Station (STA) Mode** section.
    *   **STA SSID**: Enter the name of your home or office WiFi network.
    *   **STA Password**: Enter the password for your home or office WiFi network. Leave this blank if you don't want to change a previously saved password.
    *   **STA IP Address, Gateway, Subnet Mask, and DNS**: You **must** manually configure these IP settings for the device to connect to your local network. The controller does not automatically obtain an IP address (via DHCP) in Station Mode.
        *   **Why these settings are required:** To ensure the device can communicate on your local network, it needs a unique IP address, and it needs to know how to reach other devices (Gateway) and the internet (DNS), all within the structure of your network (Subnet Mask).
        *   **How to determine the correct IP settings (Gateway, Subnet, DNS & choosing an STA IP Address):**
            *   The **STA Gateway**, **STA Subnet Mask**, and **STA Primary/Secondary DNS** values will typically be the same as your computer or smartphone's current network settings when connected to your local WiFi.
            *   For the **STA IP Address**, you need to choose an IP that is within your network's range but unused and ideally outside the range your router might use for other devices that *do* get IPs automatically (DHCP pool). This avoids IP address conflicts. A common strategy is to pick an IP address that is similar to your gateway but with a different last number (e.g., if your gateway is `192.168.1.1`, you could try `192.168.1.200`, ensuring `.200` is not in use and not reserved for automatic assignment by your router).

            *   **Finding your current network settings (to determine Gateway, Subnet, DNS):**
                *   **On Windows:**
                    1.  Open Command Prompt (search for `cmd`).
                    2.  Type `ipconfig` and press Enter.
                    3.  Look for your active WiFi connection. You'll find the `IPv4 Address` (your computer's current IP), `Subnet Mask`, and `Default Gateway`. For DNS servers, you might need to scroll down or they might be the same as the Gateway.
                *   **On Android (steps may vary slightly by version/manufacturer):**
                    1.  Go to Settings > Wi-Fi.
                    2.  Tap on the connected WiFi network (often a gear icon or by pressing and holding the network name).
                    3.  You should see details like IP address, Gateway, Subnet Mask, and DNS.
                *   **On iOS (iPhone/iPad):**
                    1.  Go to Settings > Wi-Fi.
                    2.  Tap the "i" icon next to your connected WiFi network.
                    3.  You'll see IP Address, Subnet Mask, Router (this is your Gateway), and DNS information under "Configure DNS".

            *   **Example:** If your Windows `ipconfig` shows:
                *   IPv4 Address: `192.168.1.105`
                *   Subnet Mask: `255.255.255.0`
                *   Default Gateway: `192.168.1.1`
                *   DNS Servers: `192.168.1.1` (or others like `8.8.8.8`)

                You could set your device to:
                *   STA IP Address: `192.168.1.200` (chosen to be unique and outside router's DHCP range)
                *   STA Subnet Mask: `255.255.255.0` (from your network info)
                *   STA Gateway: `192.168.1.1` (from your network info)
                *   STA Primary DNS: `192.168.1.1` (or `8.8.8.8`, from your network info)
                *   STA Secondary DNS: `8.8.4.4` (optional, can be another DNS or your Gateway)

    *   The **Access Point (AP) Mode** settings below usually don't need to be changed unless you have a specific reason.

4.  **Save Network Settings:**
    *   After entering your WiFi details, click the "**Save Network Settings**" button at the bottom of the page.

5.  **Reset the Device:**
    *   **IMPORTANT**: After saving the settings, you **must** click the **"Reset" button in the web interface**. This will restart the device and apply the new WiFi settings.

6.  **Connect to Your Local Network:**
    *   After the device restarts, it should automatically connect to the WiFi network you configured in STA mode.
    *   Your computer or smartphone should also reconnect to your regular WiFi network.
    *   You should now be able to access your device by typing its new IP address (either the one you assigned or the one assigned automatically by your router) into your web browser. You can usually find this IP address by checking the list of connected devices in your router's administration page.

### Troubleshooting

*   **Can't connect to the device's AP ("PolyMechAP")?**
    *   Ensure the device is powered on.
    *   Make sure you are close enough to the device.
    *   Try restarting the device and your computer/smartphone.
*   **Network Settings page doesn't load at `192.168.4.1`?**
    *   Double-check you are connected to the "PolyMechAP" WiFi network.
    *   Try a different web browser.
    *   Ensure no other device on your network is using the IP address `192.168.4.1`.
*   **Device doesn't connect to your local WiFi after reset?**
    *   Double-check the STA SSID and STA Password you entered are correct. Passwords are case-sensitive.
    *   Ensure your WiFi router is working correctly.
    *   Try moving the device closer to your WiFi router.
    *   If you manually set an IP address, ensure it's valid for your network and not already in use. Try leaving it blank to get an IP automatically.
    *   You may need to repeat the setup process by connecting to the device's AP again. The device should revert to AP mode if it fails to connect to the STA network after a few attempts.

If you continue to experience issues, please consult the full user manual or contact support.