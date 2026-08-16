
### Modbus Registers

| type                   | description                | address (Hex) | address (dec) | parameter group |
| :--------------------- | :------------------------- | :------------ | :------------ | :-------------- |
| write holding register | P0-16 functional parameters | F010H         | 61456         | P0              |
| write holding register | AC-08 functional parameters | AC08H         | 44040         | AC              |




### Modbus Registers (Page 1)

| type                       | description                                 | address in Hex   | address in decimal   | parameter group   |
| :------------------------- | :------------------------------------------ | :--------------- | :------------------- | :---------------- |
| Read Holding Register (03) | U-Group monitoring parameter U0-11 (Example) | 700BH            | 28683                | U-Group (U0-11)   |
| Read Holding Register (03) | Frequency converter fault code              | 8000H            | 32768                | F9-14 (context)   |
| Read Holding Register (03) | Frequency converter operating status        | 3000H            | 12288                |                   |


### Control Parameters Modbus Registers

| type                  | description                                       | address (Hex) | address (dec) | parameter group   |
| :-------------------- | :------------------------------------------------ | :------------ | :------------ | :---------------- |
| write holding register | Communication Command (1: fwd, 2: rev, 3: stop) | 3000H         | 12288         | communication   |
| write holding register | Control Command (1: fwd, 2: rev, 3: fwd jog)    | 2000H         | 8192          | Control command |


### Modbus Registers

| type                  | description                                                        | address Hex | address decimal | parameter group               |
| :-------------------- | :----------------------------------------------------------------- | :---------- | :-------------- | :------------------------------ |
| write holding register | Communication setting value (Frequency/torque/PID sources, etc.) | 1000H       | 4096            | Communication setting value     |
| write holding register | Digital output terminal control (BIT0: DO1 output control)         | 2001H       | 8193            | Digital output terminal control |


### Output Control Registers

| type                       | description                         | address (Hex) | address (decimal) | parameter group                                             |
| :------------------------- | :---------------------------------- | :------------ | :---------------- | :---------------------------------------------------------- |
| Write Holding Register (FC06/16) | Analog output AO1                 | 2002H         | 8194              | Analog output AO1, AO2, high-speed pulse output FMP control |
| Write Holding Register (FC06/16) | Analog output AO2                 | 2003H         | 8195              | Analog output AO1, AO2, high-speed pulse output FMP control |
| Write Holding Register (FC06/16) | High-speed pulse output FMP control | 2004H         | 8196              | Analog output AO1, AO2, high-speed pulse output FMP control |


### Parameter Initialization Registers (Page 1)


| type                 | description                                                                                                                                       | address (Hex) | address (decimal) | parameter group   |
| :------------------- | :------------------------------------------------------------------------------------------------------------------------------------------------ | :------------ | :---------------- | :---------------- |
| Write Holding Reg. | User password verification                                                                                                                        | 1F00H         | 7936              | Parameter Control |
| Write Holding Reg. | Parameter initialization command (1: Restore factory, 2: Clear record info, 4: Restore user backup, 5: Backup user current parameters) | 1F01H         | 7937              | Parameter Control |










### Modbus Register Mapping (Page 1)


| type                   | description                        | address in Hex  | address in decimal | parameter group |
| :--------------------- | :--------------------------------- | :-------------- | :----------------- | :-------------- |
| Read Holding Register  | Read P0-PE Group Parameters        | 0xF000 - 0xFEFF | 61440 - 65279      | P0 ~ PE Group   |
| Write Holding Register | Write P0-PE Group Parameters (RAM) | 0x0000 - 0x0EFF | 0 - 2303           | P0 ~ PE Group   |
| Read Holding Register  | Read A0-AC Group Parameters        | 0xA000 - 0xACFF | 40960 - 44287      | A0 ~ AC Group   |
| Write Holding Register | Write A0-AC Group Parameters (RAM) | 0x4000 - 0x4CFF | 16384 - 19711      | A0 ~ AC Group   |
| Read Holding Register  | Read U0 Group Parameters           | 0x7000 - 0x70FF | 28672 - 28927      | U0 Group        |


### Modbus Registers

| type                | description                                     | address in Hex | address in decimal | parameter group   |
| :------------------ | :---------------------------------------------- | :------------- | :----------------- | :---------------- |
| Write Holding (06/16) | Functional Code P3-12 (Write-only RAM)          | 030C           | 780                | P group           |
| Write Holding (06/16) | Functional Code A0-05 (Write-only RAM)          | 4005           | 16389              | A group           |
| Write Holding (06/16) | Data input from host computer (2 decimal points) | 1000           | 4096               | Not specified     |


### Modbus Registers

| type                    | description                                                   | address in Hex   | address in decimal   |
| :---------------------- | :------------------------------------------------------------ | :--------------- | :------------------- |
| Write Holding Reg (06/16) | Communication setting value (decimal system) -10000 - 10000 | 1000H            | 4096                 |
| Read Holding Reg (03)   | Running frequency                                             | 1001H            | 4097                 |
| Read Holding Reg (03)   | Busbar voltage                                                | 1002H            | 4098                 |
| Read Holding Reg (03)   | Output voltage                                                | 1003H            | 4099                 |
| Read Holding Reg (03)   | Output current                                                | 1004H            | 4100                 |
| Read Holding Reg (03)   | Output power                                                  | 1005H            | 4101                 |
| Read Holding Reg (03)   | Output torque                                                 | 1006H            | 4102                 |
| Read Holding Reg (03)   | Running speed                                                 | 1007H            | 4103                 |
| Write Holding Reg (06/16) | PID set                                                       | 1010H            | 4112                 |
| Read Holding Reg (03)   | PID feedback                                                  | 1011H            | 4113                 |
| Read Holding Reg (03)   | PLC process                                                   | 1012H            | 4114                 |
| Read Holding Reg (03)   | Pulse input frequency, unit: 0.01kHz                          | 1013H            | 4115                 |
| Read Holding Reg (03)   | Feedback speed, unit 0.1Hz                                    | 1014H            | 4116                 |
| Read Holding Reg (03)   | Remaining run time                                            | 1015H            | 4117                 |
| Read Holding Reg (03)   | AI1 Pre-correction voltage                                    | 1016H            | 4118                 |
| Read Holding Reg (03)   | AI2 Pre-correction voltage                                    | 1017H            | 4119                 |



### Modbus Registers

| type                        | description                     | address in Hex | address in decimal | parameter group   |
| :-------------------------- | :------------------------------ | :------------- | :----------------- | :---------------- |
| read holding register (FC03)| DI input sign                   | 1008H          | 4104               |                   |
| read holding register (FC03)| DO output sign                  | 1009H          | 4105               |                   |
| read holding register (FC03)| AI1 voltage                     | 100AH          | 4106               |                   |
| read holding register (FC03)| AI2 voltage                     | 100BH          | 4107               |                   |
| read holding register (FC03)| AI3 voltage                     | 100CH          | 4108               |                   |
| read holding register (FC03)| Count value input               | 100DH          | 4109               |                   |
| read holding register (FC03)| Length value input              | 100EH          | 4110               |                   |
| read holding register (FC03)| Loading speed                   | 100FH          | 4111               |                   |
| read holding register (FC03)| AI3 Pre-correction voltage      | 1018H          | 4120               |                   |
| read holding register (FC03)| Linear speed                    | 1019H          | 4121               |                   |
| read holding register (FC03)| Current power on time           | 101AH          | 4122               |                   |
| read holding register (FC03)| Current running time            | 101BH          | 4123               |                   |
| read holding register (FC03)| Pulse input frequency, unit: 1Hz| 101CH          | 4124               |                   |
| read holding register (FC03)| Communication set value         | 101DH          | 4125               |                   |
| read holding register (FC03)| Actual feedback speed           | 101EH          | 4126               |                   |
| read holding register (FC03)| Main frequency X display        | 101FH          | 4127               |                   |
| read holding register (FC03)| Auxiliary frequency Y display   | 1020H          | 4128               |                   |



### Modbus Registers


| type | description             | address in Hex | address in decimal | parameter group |
| :--- | :---------------------- | :------------- | :----------------- | :-------------- |
| FC16 | Input Control Command   | 2000H          | 8192               |                 |
| FC03 | Frequency Conv. Status | 3000H          | 12288              |                 |



### Modbus Registers


| type          | description                     | address in Hex | address in decimal | parameter group                 |
| :------------ | :------------------------------ | :------------- | :----------------- | :------------------------------ |
| FC03          | Parameter lock password check   | 1F00           | 7936               | Parameter lock password check   |
| FC06 / FC16   | Digital output terminal control | 2001           | 8193               | Digital output terminal control |


### Modbus Registers

| type         | description                             | address in Hex | address in decimal | parameter group   |
|:-------------|:----------------------------------------|:---------------|:-------------------|:------------------|
| FC 06/16     | Analog output AO1 control (write only)  | 2002H          | 8194               |                   |
| FC 06/16     | Analog output AO2 control (write only)  | 2003H          | 8195               |                   |
| FC 06/16     | Pulse output control (write only)       | 2004H          | 8196               |                   |
| FC 03/04 (?) | Fault description of frequency converter | 8000H          | 32768              | Fault information |



### Modbus Registers

| type                  | description | address in Hex | address in decimal | parameter group |
| :-------------------- | :---------- | :------------- | :----------------- | :-------------- |
| Write Holding Register | Baud rate   | Fd00           | 64768              | FD              |





### Modbus Register Examples

| type (FC) | description                                              | address (Hex) | address (decimal) | parameter group   |
| :-------- | :------------------------------------------------------- | :------------ | :---------------- | :---------------- |
| 06        | Communication command channel selection (Value 2)        | F002          | 61442             | P002              |
| 06        | Main frequency source selection (Value 9)                | F003          | 61443             | P003              |
| 06        | Run Command (Start = Value 1)                            | 2000          | 8192              | Run Control       |
| 06        | Set Running Frequency (e.g., 10Hz = Value 2000h?)        | 1000          | 4096              | Frequency Setting |
| 06        | Run Command (Stop = Value 6)                             | 2000          | 8192              | Run Control       |
