import bluetooth
import ws_relay
import ws_serial
import ws_information   
import ws_gpio 
import time
class Bluetooth():
    def __init__(self):
        #################################################################################
        # Private Variable
        #################################################################################
        self._SERVICE_UUID = '4fafc201-1fb5-459e-8fcc-c5c9c331914b'                    # UUID of the server
        self._RX_CHARACTERISTIC_UUID = 'beb5483e-36e1-4688-b7f5-ea07361b26a8'          # UUID of the characteristic Tx
        self._TX_CHARACTERISTIC_UUID = 'beb5484a-36e1-4688-b7f5-ea07361b26a8'          # UUID of the characteristic Rx
        self._BLEDeviceName = "ESP32 S3 Relay 6CH"

        self._Bluetooth_Mode = 2  # Used to distinguish data sources
        self._TxCharacteristic = 0
        self._RxCharacteristic = 0
        self._ble=0
        ###########################################################################################################################################################################################################################################
        self.Relay = ws_relay.Relay()
        self.Information = ws_information.Information()
        self.Serial = ws_serial.Serial()
        self.GPIO = ws_gpio.GPIO()
    def connected(self):
        print("Device connected");

    def disconnected(self):
        print("Device disconnected")
        self.advertiser(self._BLEDeviceName)                                                  # Re-broadcast so that the device can query

    def ble_irq(self, event, data):
        _rxValue = ""
        if event == 1:
            self.connected()
        elif event == 2:
            self.disconnected()
        elif event == 3:  
            _rxValue = self._ble.gatts_read(self._RxCharacteristic)               # 调用 gatts_read 方法读取接收缓冲区中的数据，并将其存储在 buffer 变量中
            #print(_rxValue)
            if len(_rxValue) == 1:
                print(_rxValue)  # Print output
                valueBytes = bytearray(_rxValue, 'utf-8')  # Convert value to bytes
                self.Relay.Relay_Analysis(valueBytes, self._Bluetooth_Mode)  # pilot relay
            elif len(_rxValue) == 2:
                if self.Information.Extension_Enable == 1:
                    print(_rxValue)  # Print output
                    valueBytes = bytearray(_rxValue, 'utf-8')  # Convert value to bytes
                    if valueBytes[0] == 0x06:  # Instruction check correct
                        self.Serial.RS485_Analysis(valueBytes)  # Control external relay
                    else:
                        print("Note1 : Non-instruction data was received - Bluetooth !")
                else:
                    print("Note2 : Non-instruction data was received - Bluetooth !")
            else:
                print("Note3: Non-instruction data was received - Bluetooth !")
            _rxValue = ""

    def register(self):    
        services = (
            (
                bluetooth.UUID(self._SERVICE_UUID),
                (
                    (bluetooth.UUID(self._TX_CHARACTERISTIC_UUID), bluetooth.FLAG_READ),
                    (bluetooth.UUID(self._RX_CHARACTERISTIC_UUID), bluetooth.FLAG_WRITE),
                )
            ),
        )
        ((TxCharacteristic, RxCharacteristic,), ) = self._ble.gatts_register_services(services)
        return (TxCharacteristic, RxCharacteristic)

    def send(self, data):
        self._ble.gatts_write(self._TxCharacteristic, data)

    def advertiser(self, name):
        print(name)
        name = bytes(name, 'UTF-8')
        adv_data = bytearray('\x02\x01\x02', 'UTF-8') + bytearray((len(name) + 1, 0x09), 'UTF-8') + name
        self._ble.gap_advertise(100, adv_data)
        #print(adv_data)
        #print("\r\n")
    def Bluetooth_Init(self):
        self._ble = bluetooth.BLE()
        self._ble.active(True)
        self._ble.config(gap_name=self._BLEDeviceName)
        self._ble.irq(self.ble_irq)
        self._TxCharacteristic, self._RxCharacteristic = self.register()
        self.advertiser(self._BLEDeviceName)
        self.GPIO.RGB_Light(0, 0, 60) 
        time.sleep(1000 / 1000)       # Convert Time to seconds
        self.GPIO.RGB_Light(0, 0, 0) 
        print("Now you can read it in your phone!\r\n")




