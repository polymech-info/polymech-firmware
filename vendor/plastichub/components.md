## Controllino - Mega

- [Unit Testing](https://docs.platformio.org/en/latest/plus/unit-testing.html?utm_medium=piohome&utm_source=platformio)
- Semiphores : emulated, via ASM noops
- Memory: seems stable with modbus over ethernet

### Omron MX2

- [Programmer](./firmware/OmronMx2.cpp#L460)

### Omron - E5DC

### Calibration

- [TC calibration](https://www.tcdirect.co.uk/product_2_133_21#133/21/1)

# Extrusion Firmware

### Communication modes

- [x] Custom Serial over RS485 (Extruza, 3dtreehouse)
- [x] Standard Serial (Lydia-v4.5 - touchscreen/RPI)
- [-] Pure RS485 - Slave|Master (In conjunction with injection addon)
- [-] Bluetooth
  - [-] App template
- [ ] TCP 
  - [ ] Modbus (rev 1, base abstract/odel)
  - [ ] ProfiBus (rev 2)
  - [ ] DeviceNet (rev 1)
- [ ] Debug Serial (Always)

### Protocol

- [ ] Internal: add duplex for RS485 feedback
- [-] Add command queue lifecycle / state response
- [-] Handshake
- [- Checksum
- [ ] Broadcast (for bluetooth as well)
- [-] User space / reserved

### Controls

- [ ] Simple cycle control via logic/analog interface
- [ ] Controllino - Mini/Uno version (http://www.kuehlschrankdichtung.de/ | PVC temp and cycle time profile storage) 

### Modbus testing strategy for vendors

- [ ] 1. test basics after unboxing
- [x] 2. programmer test
- [ ] 3. stress/flood test
- [ ] 4. EMI near (scope)
- [ ] 5. EMI long / cable test (serial debugging with scope)

### PID / TC related

- [-] determine barrel empty / full ratio/constant impact on PID readings
- [-] determine actual PID values, independent via calibration devices (see [./firmware/components](./firmware/components) for more)
- [ ] ramp times, window keeping
- [ ] test PVC/PFA [TC cables](https://www.tcdirect.co.uk/product_2_270_1)


## Components

- [ ] Add HMI column
- [ ] Add register range
- [ ] Add wire labels
- [ ] Universal relay bank with bridge caps (with NO/NC masking) for fans, at the extruder and object | each register needs a corrosponding slot for speed control


| Component                    	| ID        	| Qty 	| Volt 	| VA       	| Circuit 	| Powersource 	| Source File 	|
|------------------------------	|-----------	|-----	|------	|----------	|---------	|-------------	|-------------	|
| VFD                          	| VDF       	|     	|      	| -        	| Motor   	| Extern      	|             	|
| PID                          	| PID 1...  	| 3   	| 24   	| 1.5W     	| Digital 	| 24V-I       	|             	|
| Controllino - Master         	| CM        	| 1   	| 24   	| 5W       	| Digital 	| 24V-I       	|             	|
| Fans                         	| FAN       	| 6   	| 12   	| ?        	| Cooling 	| 12V-I       	|             	|
| Height - Sensor              	| SHEIGHT   	| 2   	| 24   	| 100mA    	| Sensor  	| 24V         	|             	|
| SSR                          	| SSR 1...  	| 4   	| 220  	| 100-300W 	| Heating 	| Extern      	|             	|
| Thermocouple - Extruder      	| TCE 1...  	| 4   	|      	|          	|         	|             	|             	|
| Hopper-Selonid               	| AUXH 1... 	| 2   	| 12   	| ?        	| Aux     	| 12V-II      	|             	|
| Motor-RPM Feedback           	| SRPM 1..2 	| 2   	| 12   	| ?        	| Sensor  	| 12V-I       	|             	|
| Power - Circuit - Contactors 	| CPO 1...  	| 3   	| 220V 	| -        	| Switch  	| Extern      	|             	|
| Thermo - Couple - Motor      	| TCM       	|     	|      	|          	|         	|             	|             	|
| Audio - Alarm                	| AA 1...    	| 2    	| 24V  	| ?        	| Feedback	| 24V-II       	|             	|

### Modbus design notes

- each issued read/write has a corrosponding state flag register, eg: issued, proccessing, processed, error code, failure
- use coils only
- when in duplex, feedback channel provides command queue id with state flag, polling should be avoided
- avoid multi bytes/long/strings
- each category shall provide additional user register space



