import ws_serial       # 导入WS_Serial库
from ws_gpio import *   # 导入WS_Serial库
import ws_relay
import ws_bluetooth 
if __name__ == "__main__":
    try:
        Bluetooth = ws_bluetooth.Bluetooth()
        Serial = ws_serial.Serial()
        Relay = ws_relay.Relay()
        # UART
        Serial.Serial_Init()
        # Bluetooth
        Bluetooth.Bluetooth_Init()

        # Complete the setup
        print("Connect to the Bluetooth network named \"ESP32-S3-Relay-6CH\" and send data to control the device through the debugging tool")

        while True:
            # RS485 Receive Data
            Serial.Serial_Loop()
    except KeyboardInterrupt:
        print("Abnormal interrupt, program exit")


