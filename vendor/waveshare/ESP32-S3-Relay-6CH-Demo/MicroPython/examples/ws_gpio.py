from machine import Pin, PWM
from neopixel import NeoPixel
import time

_BUZZER_PIN = Pin(21, Pin.OUT)     # Define buzzer pin and frequency

_Frequency = 1000
_buzzer_pwm = PWM(_BUZZER_PIN)
_buzzer_pwm.freq(_Frequency)
_buzzer_pwm.duty(0)                # Set initial duty cycle to 0

class GPIO():
    def __init__(self):
        #################################################################################
        # Public Variable
        #################################################################################
        # Define GPIO pins
        self.GPIO_PIN_CH1 = Pin(1, Pin.OUT)
        self.GPIO_PIN_CH2 = Pin(2, Pin.OUT)
        self.GPIO_PIN_CH3 = Pin(41, Pin.OUT)
        self.GPIO_PIN_CH4 = Pin(42, Pin.OUT)
        self.GPIO_PIN_CH5 = Pin(45, Pin.OUT)
        self.GPIO_PIN_CH6 = Pin(46, Pin.OUT)

        #################################################################################
        # Private Variable
        #################################################################################
        self._GPIO_PIN_RGB = Pin(38, Pin.OUT)
        
        self._RGB = NeoPixel(self._GPIO_PIN_RGB,1)   # Use the number of pins and lamp beads

        self._Dutyfactor = 200
    def wait_ms(self, Time):
        start_time = time.ticks_ms()
        while time.ticks_diff(time.ticks_ms(), start_time) < Time:
            # You can add other tasks here that will run during the wait
            pass
    # I/O functions
    def digitalToggle(self, pin):
        pin.value(not pin.value())  # Toggle the state of the pin

    def RGB_Light(self, red_val, green_val, blue_val):
        # Assume neopixelWrite is a function that writes RGB values to an RGB LED
        self._RGB[0] = (red_val, green_val, blue_val)
        self._RGB.write()# RGB color adjustment

    def Buzzer_PWM(self, Time):
        _buzzer_pwm.duty(self._Dutyfactor)  # Set duty cycle
        self.wait_ms(Time)
        _buzzer_pwm.duty(0)            # Turn off PWM






