from machine import Pin
import time
import ws_gpio

gpio = ws_gpio.GPIO()

class Relay():
    def __init__(self):
        self.Relay_Flag = [False]*6  # Relay current status flag
        
    # Data Analysis
    def Relay_Analysis(self, buf, Mode_Flag):
        if Mode_Flag == 2:
            print("Bluetooth Data :")
        elif Mode_Flag == 3:
            print("WIFI Data :")
        else:
            print("RS485 Data :")

        if buf[0] == ord('1'):
            gpio.GPIO_PIN_CH1.value(not gpio.GPIO_PIN_CH1.value())
            self.Relay_Flag[0] = not self.Relay_Flag[0]
            gpio.Buzzer_PWM(100)
            if self.Relay_Flag[0]:
                print("|***  Relay CH1 on  ***|")
            else:
                print("|***  Relay CH1 off ***|")
        elif buf[0] == ord('2'):
            gpio.GPIO_PIN_CH2.value(not gpio.GPIO_PIN_CH2.value())
            self.Relay_Flag[1] = not self.Relay_Flag[1]
            gpio.Buzzer_PWM(100)
            if self.Relay_Flag[1]:
                print("|***  Relay CH2 on  ***|")
            else:
                print("|***  Relay CH2 off ***|")
        elif buf[0] == ord('3'):
            gpio.GPIO_PIN_CH3.value(not gpio.GPIO_PIN_CH3.value())
            self.Relay_Flag[2] = not self.Relay_Flag[2]
            gpio.Buzzer_PWM(100)
            if self.Relay_Flag[2]:
                print("|***  Relay CH3 on  ***|")
            else:
                print("|***  Relay CH3 off ***|")
        elif buf[0] == ord('4'):
            gpio.GPIO_PIN_CH4.value(not gpio.GPIO_PIN_CH4.value())
            self.Relay_Flag[3] = not self.Relay_Flag[3]
            gpio.Buzzer_PWM(100)
            if self.Relay_Flag[3]:
                print("|***  Relay CH4 on  ***|")
            else:
                print("|***  Relay CH4 off ***|")
        elif buf[0] == ord('5'):
            gpio.GPIO_PIN_CH5.value(not gpio.GPIO_PIN_CH5.value())
            self.Relay_Flag[4] = not self.Relay_Flag[4]
            gpio.Buzzer_PWM(100)
            if self.Relay_Flag[4]:
                print("|***  Relay CH5 on  ***|")
            else:
                print("|***  Relay CH5 off ***|")
        elif buf[0] == ord('6'):
            gpio.GPIO_PIN_CH6.value(not gpio.GPIO_PIN_CH6.value())
            self.Relay_Flag[5] = not self.Relay_Flag[5]
            gpio.Buzzer_PWM(100)
            if self.Relay_Flag[5]:
                print("|***  Relay CH6 on  ***|")
            else:
                print("|***  Relay CH6 off ***|")
        elif buf[0] == ord('7'):
            gpio.GPIO_PIN_CH1.value(1)
            gpio.GPIO_PIN_CH2.value(1)
            gpio.GPIO_PIN_CH3.value(1)
            gpio.GPIO_PIN_CH4.value(1)
            gpio.GPIO_PIN_CH5.value(1)
            gpio.GPIO_PIN_CH6.value(1)
            for i in range(len(self.Relay_Flag)):
                self.Relay_Flag[i] = True
            print("|***  Relay ALL on  ***|")
            gpio.Buzzer_PWM(300)
        elif buf[0] == ord('8'):
            gpio.GPIO_PIN_CH1.value(0)
            gpio.GPIO_PIN_CH2.value(0)
            gpio.GPIO_PIN_CH3.value(0)
            gpio.GPIO_PIN_CH4.value(0)
            gpio.GPIO_PIN_CH5.value(0)
            gpio.GPIO_PIN_CH6.value(0)
            for i in range(len(self.Relay_Flag)):
                self.Relay_Flag[i] = False
            print("|***  Relay ALL off ***|")
            gpio.Buzzer_PWM(100)
            gpio.wait_ms(100)
            gpio.Buzzer_PWM(100)
        else:
            print("Note : Non-instruction data was received !")
            

            




