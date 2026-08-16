## Overview

Family : KC868-A6 | [Official](https://shop.kincony.com/products/kc868-a16-arduino-esp32-16-channel-gpio-module)

———————————

### Resources

schematic: https://www.kincony.com/download/KC868-A16-schematic.pdf
ESP32 I/O pin define: https://www.kincony.com/forum/showthread.php?tid=1666
config yaml file for ESPHome: https://www.kincony.com/forum/showthread.php?tid=1628
tasmota firmware: https://www.kincony.com/forum/showthread.php?tid=1715
arduino demo source code: https://www.kincony.com/forum/forumdisplay.php?fid=25

KCS v2 firmware online guide: https://www.kincony.com/esp32-kcsv2-firmware.html

KCS v2 function explanation video tour: https://youtu.be/-DexVHAxPUM

KC868-A16 professional hardware review by Russian

KC868-AK “KCS” firmware download
How to use “KCS” firmware online guide
How to use “KCS” firmware YouTube video tour
KC868-A16 introduction YouTube video tour

## GPIO

#define ANALOG_A1  36
#define ANALOG_A2  34
#define ANALOG_A3  35
#define ANALOG_A4  39

IIC SDA:4
IIC SCL:5

Relay_IIC_address 0x24
Relay_IIC_address 0x25

Input_IIC_address 0x21
Input_IIC_address 0x22

DS18B20/DHT11/DHT21/LED strip -1: 32
DS18B20/DHT11/DHT21/LED strip -2: 33
DS18B20/DHT11/DHT21/LED strip -2: 14


RF433MHz wireless receiver: 2
RF433MHz wireless sender: 15

Ethernet (LAN8720) I/O define:

#define ETH_ADDR        0
#define ETH_POWER_PIN  -1
#define ETH_MDC_PIN    23
#define ETH_MDIO_PIN  18
#define ETH_TYPE      ETH_PHY_LAN8720
#define ETH_CLK_MODE  ETH_CLOCK_GPIO17_OUT

RS485:
RXD:16
TXD:13
