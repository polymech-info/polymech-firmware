from machine import UART  # 导入UART库，用于处理串口通讯
import machine
import ws_relay


class Serial():
    def __init__(self):
        #################################################################################
        # Private Variable
        #################################################################################
        self._TXD1 = 17
        self._RXD1 = 18
        self._RS485_Mode = 1  # Used to distinguish data sources
        self._Extension_CH1 = 1  # Expansion Channel 1
        self._Extension_CH2 = 2  # Expansion Channel 2
        self._Extension_CH3 = 3  # Expansion Channel 3
        self._Extension_CH4 = 4  # Expansion Channel 4
        self._Extension_CH5 = 5  # Expansion Channel 5
        self._Extension_CH6 = 6  # Expansion Channel 6
        self._Extension_CH7 = 7  # Expansion Channel 7
        self._Extension_CH8 = 8  # Expansion Channel 8
        self._Extension_ALL_ON = 9  # Expansion ALL ON
        self._Extension_ALL_OFF = 10  # Expansion ALL OFF

        self._data = (
            bytes([0x06, 0x05, 0x00, 0x01, 0x55, 0x00, 0xA2, 0xED]),  # ESP32-S3-Relay-6CH CH1 Toggle
            bytes([0x06, 0x05, 0x00, 0x02, 0x55, 0x00, 0x52, 0xED]),  # ESP32-S3-Relay-6CH CH2 Toggle
            bytes([0x06, 0x05, 0x00, 0x03, 0x55, 0x00, 0x03, 0x2D]),  # ESP32-S3-Relay-6CH CH3 Toggle
            bytes([0x06, 0x05, 0x00, 0x04, 0x55, 0x00, 0xB2, 0xEC]),  # ESP32-S3-Relay-6CH CH4 Toggle
            bytes([0x06, 0x05, 0x00, 0x05, 0x55, 0x00, 0xE3, 0x2C]),  # ESP32-S3-Relay-6CH CH5 Toggle
            bytes([0x06, 0x05, 0x00, 0x06, 0x55, 0x00, 0x13, 0x2C]),  # ESP32-S3-Relay-6CH CH6 Toggle
            bytes([0x06, 0x05, 0x00, 0xFF, 0xFF, 0x00, 0xBD, 0xBD]),  # ESP32-S3-Relay-6CH ALL ON
            bytes([0x06, 0x05, 0x00, 0xFF, 0x00, 0x00, 0xFC, 0x4D]),  # ESP32-S3-Relay-6CH ALL OFF
        )
        self._Send_data = (
            bytes([0x01, 0x05, 0x00, 0x00, 0x55, 0x00, 0xF2, 0x9A]),  # Modbus RTU Relay CH1 Toggle
            bytes([0x01, 0x05, 0x00, 0x01, 0x55, 0x00, 0xA3, 0x5A]),  # Modbus RTU Relay CH2 Toggle
            bytes([0x01, 0x05, 0x00, 0x02, 0x55, 0x00, 0x53, 0x5A]),  # Modbus RTU Relay CH3 Toggle
            bytes([0x01, 0x05, 0x00, 0x03, 0x55, 0x00, 0x02, 0x9A]),  # Modbus RTU Relay CH4 Toggle
            bytes([0x01, 0x05, 0x00, 0x04, 0x55, 0x00, 0xB3, 0x5B]),  # Modbus RTU Relay CH5 Toggle
            bytes([0x01, 0x05, 0x00, 0x05, 0x55, 0x00, 0xE2, 0x9B]),  # Modbus RTU Relay CH6 Toggle
            bytes([0x01, 0x05, 0x00, 0x06, 0x55, 0x00, 0x12, 0x9B]),  # Modbus RTU Relay CH7 Toggle
            bytes([0x01, 0x05, 0x00, 0x07, 0x55, 0x00, 0x43, 0x5B]),  # Modbus RTU Relay CH8 Toggle
            bytes([0x01, 0x05, 0x00, 0xFF, 0xFF, 0x00, 0xBC, 0x0A]),  # Modbus RTU Relay ALL ON
            bytes([0x01, 0x05, 0x00, 0xFF, 0x00, 0x00, 0xFD, 0xFA]),  # Modbus RTU Relay ALL OFF

        )

        ###########################################################################################################################################################################################################################################
        self._lidarSerial = machine.UART(1)  # Using serial port 1
        self.Relay = ws_relay.Relay()
            
    def SetData(self, data, length):
        self._lidarSerial.write(data)  # Send data from the RS485

    def ReadData(self, buf, length):
        if self._lidarSerial.any() >= length:
            self._lidarSerial.readinto(buf, length)
            print("Received data:", " ".join("{:02X}".format(byte) for byte in buf))
            # Add a receiving data handler
            buf.clear()

    def RS485_Analysis(self, buf):
        if buf[1] == self._Extension_CH1:
            self.SetData(self._Send_data[0], len(self._Send_data[0])) 
            print("|***  Toggle expansion channel 1 ***|")
        elif buf[1] == self._Extension_CH2:
            self.SetData(self._Send_data[1], len(self._Send_data[1])) 
            print("|***  Toggle expansion channel 2 ***|")
        elif buf[1] == self._Extension_CH3:
            self.SetData(self._Send_data[2], len(self._Send_data[2])) 
            print("|***  Toggle expansion channel 3 ***|")
        elif buf[1] == self._Extension_CH4:
            self.SetData(self._Send_data[3], len(self._Send_data[3])) 
            print("|***  Toggle expansion channel 4 ***|")
        elif buf[1] == self._Extension_CH5:
            self.SetData(self._Send_data[4], len(self._Send_data[4])) 
            print("|***  Toggle expansion channel 5 ***|")
        elif buf[1] == self._Extension_CH6:
            self.SetData(self._Send_data[5], len(self._Send_data[5])) 
            print("|***  Toggle expansion channel 6 ***|")
        elif buf[1] == self._Extension_CH7:
            self.SetData(self._Send_data[6], len(self._Send_data[6])) 
            print("|***  Toggle expansion channel 7 ***|")
        elif buf[1] == self._Extension_CH8:
            self.SetData(self._Send_data[7], len(self._Send_data[7])) 
            print("|***  Toggle expansion channel 8 ***|")
        elif buf[1] == self._Extension_ALL_ON:
            self.SetData(self._Send_data[8], len(self._Send_data[8])) 
            print("|***  Enable all extension channels ***|")
        elif buf[1] == self._Extension_ALL_OFF:
            self.SetData(self._Send_data[9], len(self._Send_data[9])) 
            print("|***  Close all expansion channels ***|")
        else:
            print("Note : Non-control external device instructions !")

    def Serial_Init(self):
        self._lidarSerial.init(9600, bits=8, parity=None, stop=1, rx = self._RXD1, tx = self._TXD1)


    def Serial_Loop(self):
        buf = bytearray(20)    # Data storage area
        Receive_Flag = self._lidarSerial.any()
        if Receive_Flag > 0:
            buf = bytearray(Receive_Flag)       # Create a bytearray to store the data
            self._lidarSerial.readinto(buf)           # Read the data into the bytearray
            if Receive_Flag == 8:
                i = 0
                for i in range(8):
                    if buf[:8] == self._data[i][:8]:          # Compare two bytearrays
                        buf[0] = i+1+48
                        self.Relay.Relay_Analysis(buf, self._RS485_Mode)
                        break
                if i > 7:
                    print("Note : Non-instruction data was received - RS485 !")
            else:
                print("Note : Non-instruction data was received - RS485 !")



