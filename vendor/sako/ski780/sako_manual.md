### P0 Standard Function Parameters (Page 13)

| parameter   | description              | parameter group   | range                                                                                                              | default   |
| :---------- | :----------------------- | :---------------- | :----------------------------------------------------------------------------------------------------------------- | :-------- |
| P0-01       | Motor control mode       | P0                | 0: Sensorless flux vector control (SFVC)<br>2: V/F contro                                                          | 2         |
| P0-02       | Command source selection | P0                | 0: Operation panel control (LED off)<br>1: Terminal control (LED on)<br>2: Communication control (LED blinking) | 0         |


### Function Parameters (Page 14)

| parameter   | description                                                    | parameter group   | range                                                                                                                                                                                                                                                                                          | default   |
|:------------|:---------------------------------------------------------------|:------------------|:-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|:----------|
| P0-03       | Main frequency source X selection                              | P0                | 0: Digital setting (preset frequency PO-08, press UP/DOWN to modify, non-retentive at power failure) <br> 1: Digital setting (preset frequency PO-08, press UP/DOWN to modify, retentive at power failure) <br> 2: AI1 <br> 3: Panel potentiometer <br> 4: External panel potentiometer <br> 5: HDI pulse setting (DI5) <br> 6: Multi-command <br> 7: Simple PLC <br> 8: PID <br> 9: Communication setting | 3         |
| P0-04       | Auxiliary frequency source Y selection                         | P0                | The same as P0-03 (Main frequency source X selection)                                                                                                                                                                                                                                            | 0         |
| P0-05       | Selection of Y range of auxiliary frequency source in superposition | P0                | 0: Relative to maximum frequency <br> 1: Relative to main frequency X                                                                                                                                                                                                                           | 0         |
| P0-06       | Selection of Y range of auxiliary frequency source in superposition | P0                | 0% ~ 150%                                                                                                                                                                                                                                                                                      | 100%      |




### P0 Standard Function Parameters - Page 15

| parameter                             | description                                                                                                                                                                                                        | parameter group                 | range                                                                                   | default   |
| :------------------------------------ | :----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | :------------------------------ | :-------------------------------------------------------------------------------------- | :-------- |
| P0-07 Frequency source superposition selection | Unit's digit (Frequency source selection)<br>0: Main frequency source X<br>1: X and Y operation (operation relationship determined by ten's digit)<br>2: Switchover between X and Y<br>3: Switchover between X and "X and Y operation"<br>4: Switchover between Y and "X and Y operation"<br>Ten's digit (X and Y operation relationship)<br>0: X+Y<br>1: X-Y<br>2: Maximum<br>3: Minimum | P0 Standard Function Parameters | Unit's digit: 0-4<br>Ten's digit: 0-3                                               | 00        |
| P0-08 Preset frequency                | Preset frequency setting                                                                                                                                                                                           | P0 Standard Function Parameters | 0.00Hz ~ maximum frequency (P0-10)                                                      | 50.00Hz   |
| P0-09 Rotation direction              | 0: Same direction<br>1: Reverse direction                                                                                                                                                                         | P0 Standard Function Parameters | 0, 1                                                                                    | 0         |
| P0-10 Maximum frequency               | Maximum frequency setting                                                                                                                                                                                          | P0 Standard Function Parameters | 5.00Hz ~ 500.00Hz                                                                       | 50.00Hz   |
| P0-11 Source of frequency upper limit | 0: Set by P0-12<br>1: AI1<br>2: AI2 local potentiometer<br>3: AI3 panel potentiometer external keyboard potentiometer<br>4: HDI pulse setting<br>5: Communication setting                                            | P0 Standard Function Parameters | 0-5                                                                                     | 0         |
| P0-12 Frequency upper limit           | Frequency upper limit setting                                                                                                                                                                                      | P0 Standard Function Parameters | Frequency lower limit (P0-14) to maximum frequency (P0-10)                              | 50.00Hz   |






### Columns

- parameter
- description
- if provided, parameter group
- range
- default


### P0 Standard Function Parameters (Page 16)

| parameter | description | parameter group | range | default |
|---|---|---|---|---|
| P0-13 | Frequency upper limit offset | P0 | 0.00Hz ~ maximum frequency P0-10 | 0.00Hz |
| P0-14 | Frequency lower limit | P0 | 0.00Hz ~ frequency upper limit P0-12 | 0.00Hz |
| P0-15 | Carrier frequency | P0 | 2.0kHz ~ 8.0kHz | Model dependent |
| P0-16 | Carrier frequency adjustment with temperature | P0 | 0: No, 1: Yes | 1 |
| P0-17 | Acceleration time 1 | P0 | 0.00s ~ 650.00s [P0-19=2]<br>0.0s ~ 6500.0s [P0-19=1]<br>0s ~ 65000s [P0-19=0] | Model dependent |
| P0-18 | Deceleration time 1 | P0 | 0.00s ~ 650.00s [P0-19=2]<br>0.0s ~ 6500.0s [P0-19=1]<br>0s ~ 65000s [P0-19=0] | Model dependent |
| P0-19 | Acceleration/Deceleration time unit | P0 | 0: 1s, 1: 0.1s, 2: 0.01s | 1 |
| P0-21 | Frequency offset of auxiliary frequency source for X and Y operation | P0 | 0.00Hz ~ maximum frequency P0-10 | 0.00Hz |
| P0-22 | Frequency reference resolution | P0 | 2: 0.01Hz | 2 |
| P0-23 | Retentive of digital setting frequency upon power failure | P0 | 0: Not retentive, 1: Retentive | 0 |




### Columns

- parameter
- description
- if provided, parameter group
- range
- default

### P0 Standard Function Parameters - Page 17

| parameter | description                                           | parameter group | range                                                                                                                                                                                                                                                                                                                                                                          | default   |
|-----------|-------------------------------------------------------|-----------------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|-----------|
| P0-25     | Acceleration/ Deceleration time base frequency        | P0              | 0: Maximum frequency (P0-10)<br>1: Set frequency<br>2: 100 Hz                                                                                                                                                                                                                                                                                                                  | 0         |
| P0-26     | Base frequency for UP/ DOWN modification during running | P0              | 0: Running frequency<br>1: Set frequency                                                                                                                                                                                                                                                                                                                                      | 0         |
| P0-27     | Binding command source to frequency source            | P0              | Unit's digit (Binding operation panel command to frequency source)<br>0: No binding<br>1: Frequency source by digital setting<br>2: AI1<br>3: AI2<br>4: Panel potentiometer external keyboard potentiometer<br>5: HDI Pulse setting (DI5)<br>6: Multi-command<br>7: Simple PLC<br>8: PID<br>9: Communication setting<br>Ten's digit (Binding terminal command to frequency source)<br>Hundred's digit (Binding communication command to frequency source) | 0000      |





### P1 Motor Parameters (Page 18)

| parameter | description              | parameter group   | range                           | default         |
| :-------- | :----------------------- | :---------------- | :------------------------------ | :-------------- |
| P1-00     | Motor type selection     | P1 Motor Parameters | 0: Common asynchronous motor<br>2: Permanent magnetic synchronous motor | 0               |
| P1-01     | Rated motor power        | P1 Motor Parameters | 0.1kW ~ 1000.0kW                | Model dependent |
| P1-02     | Rated motor voltage      | P1 Motor Parameters | 1V ~ 2000V                      | Model dependent |
| P1-03     | Rated motor current      | P1 Motor Parameters | 0.01A ~ 10.00A (AC drive power <= 2.2kW) | Model dependent |
| P1-04     | Rated motor frequency    | P1 Motor Parameters | 0.01Hz ~ maximum frequency      | Model dependent |
| P1-05     | Rated motor rotational speed | P1 Motor Parameters | 1rpm ~ 65535rpm                 | Model dependent |
| P1-10     | No-load current (asynchronous motor) | P1 Motor Parameters | 0.01A ~ P1-03                   | Model dependent |
| P1-37     | Auto-tuning selection    | P1 Motor Parameters | 0: No auto-tuning<br>1. Asynchronous motor static auto-tuning<br>2: Asynchronous motor complete auto-tuning | 0               |




### P2 Vector Control Parameters - Page 19

| parameter | description | parameter group | range | default |
|---|---|---|---|---|
| P2-00 | Speed loop proportional gain 1 | P2 | 1 ~ 100 | 30 |
| P2-01 | Speed loop integral time 1 | P2 | 0.01s ~ 10.00s | 0.50s |
| P2-02 | Switchover frequency 1 | P2 | 0.00 ~ P2-05 | 5.00Hz |
| P2-03 | Speed loop proportional gain 2 | P2 | 1 ~ 100 | 20 |
| P2-04 | Speed loop integral time 2 | P2 | 0.01s ~ 10.00s | 1.00s |
| P2-05 | Switchover frequency 2 | P2 | P2-02 ~ maximum output frequency | 10.00Hz |
| P2-06 | Vector control slip gain | P2 | 50% ~ 200% | 100% |
| P2-07 | Time constant of speed loop filter | P2 | 0.000s ~ 1.000s | 0.050s |
| P2-09 | Torque upper limit source in speed control mode | P2 | 0: Function code setting at P2-10 <br> 1: AI1 <br> 2: AI2 <br> 3: Panel potentiometer external keyboard potentiometer <br> 4: HDI Pulse setting <br> 5: Communication setting <br> 6: MIN(AI1, AI2) <br> 7: MAX(AI1, AI2) <br> 1-7 The full range of options corresponds to P2-10 | 0 |





### Columns

- parameter
- description
- if provided, parameter group
- range
- default
- Dont comment or explain, just return Markdown
- insert new lines before and after headers
- insert a descriptive chapter name, header level 3 with the page number

### P2 Vector Control Parameters (Page 20)

| parameter | description | parameter group | range | default |
|---|---|---|---|---|
| P2-10 | Digital setting of torque upper limit in speed control mode | P2 | 0.0%~200.0% | 150.0% |
| P2-13 | Excitation adjustment proportional gain | P2 | 0~60000 | 2000 |
| P2-14 | Excitation adjustment integral gain | P2 | 0~60000 | 1300 |
| P2-15 | Torque adjustment proportional gain | P2 | 0~60000 | 2000 |
| P2-16 | Torque adjustment integral gain | P2 | 0~60000 | 1300 |
| P2-17 | Speed loop integral property | P2 | Unit's digit: integral separation <br> 0: Disabled <br> 1: Enabled | 0 |
| P2-20 | Maximum output voltage coefficient | P2 | 100%~110% | 105% |
| P2-21 | Maximum torque coefficient in weak magnetic field | P2 | 50%~200% | 100% |





### P3 V/F Control Parameters - Page 21

| parameter | description                     | parameter group           | range                                           | default         |
| :-------- | :------------------------------ | :------------------------ | :---------------------------------------------- | :-------------- |
| P3-00     | VF curve setting                | P3 V/F Control Parameters | 0: Linear V/F<br>1: Multi-point V/F<br>2: Square V/F<br>3: 1.2 power V/F<br>4: 1.4 power V/F<br>6: 1.6 power V/F<br>8: 1.8 power V/F<br>9: Reserved<br>10: V/F complete separation<br>11: V/F half separation | 0               |
| P3-01     | Torque boost                    | P3 V/F Control Parameters | 0.0% (Automatic torque boost)<br>0.1% ~ 30.0% | Model dependent |
| P3-02     | Cut-off frequency of torque boost | P3 V/F Control Parameters | 0.00Hz ~ maximum frequency                  | 50.00Hz         |
| P3-03     | Multi-point V/F frequency 1     | P3 V/F Control Parameters | 0.00Hz ~ P3-05                                | 0.00Hz          |
| P3-04     | Multi-point V/F voltage 1       | P3 V/F Control Parameters | 0.0% ~ 100.0%                                 | 0.0%            |
| P3-05     | Multi-point V/F frequency 2     | P3 V/F Control Parameters | P3-03 ~ P3-07                                 | 0.00Hz          |
| P3-06     | Multi-point V/F voltage 2       | P3 V/F Control Parameters | 0.0% ~ 100.0%                                 | 0.0%            |
| P3-07     | Multi-point V/F frequency 3     | P3 V/F Control Parameters | P3-05 ~ rated motor frequency (P1-04)         | 0.00Hz          |
| P3-08     | Multi-point V/F voltage 3       | P3 V/F Control Parameters | 0.0% ~ 100.0%                                 | 0.0%            |
| P3-09     | V/F slip compensation gain      | P3 V/F Control Parameters | 0.0% ~ 200.0%                                 | 0.0%            |
| P3-10     | VF over-excitation gain         | P3 V/F Control Parameters | 0 ~ 200                                         | 64              |
| P3-11     | VF oscillation suppression gain | P3 V/F Control Parameters | 0 ~ 100                                         | Model dependent |




### P4 Input Terminals - Page 22

| parameter   | description                | parameter group   | range                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          | default   |
| :---------- | :------------------------- | :---------------- | :------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | :-------- |
| P4-00       | DI1 terminal function selection | P4 Input Terminals | 0: No function<br>1: Forward RUN (FWD) or RUN<br>2: Reverse RUN (REV) or RUN direction<br>3: Three-line control<br>4: Forward JOG (FJOG)<br>5: Reverse JOG (RJOG)<br>6: Terminal UP<br>7: Terminal DOWN<br>8: Coast to stop<br>9: Fault reset (RESET)<br>10: RUN pause                                                                                                                                                                                                                                          | 1         |
| P4-01       | DI2 terminal function selection | P4 Input Terminals | 11: Normally open (NO) input of external fault<br>12: Multi-reference terminal 1<br>13: Multi-reference terminal 2<br>14: Multi-reference terminal 3<br>15: Multi-reference terminal 4<br>16: Terminal 1 for acceleration/ deceleration time selection<br>17: Terminal 2 for acceleration/ deceleration time selection<br>18: Frequency source switchover<br>19: UP and DOWN setting clear (terminal, operation panel)<br>20: Command source switchover terminal 1<br>21: Acceleration/Deceleration prohibited | 2         |
| P4-02       | DI3 terminal function selection | P4 Input Terminals | 11: Normally open (NO) input of external fault<br>12: Multi-reference terminal 1<br>13: Multi-reference terminal 2<br>14: Multi-reference terminal 3<br>15: Multi-reference terminal 4<br>16: Terminal 1 for acceleration/ deceleration time selection<br>17: Terminal 2 for acceleration/ deceleration time selection<br>18: Frequency source switchover<br>19: UP and DOWN setting clear (terminal, operation panel)<br>20: Command source switchover terminal 1<br>21: Acceleration/Deceleration prohibited | 4         |
| P4-03       | DI4 terminal function selection | P4 Input Terminals | 22: PID pause<br>23: PLC status reset<br>24: Swing pause<br>25: Counter input<br>26: Counter reset                                                                                                                                                                                                                                                                                                                                                                                                    | 9         |





### P4 Input Terminals (Page 23)

| parameter | description                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         | parameter group   | range                                  | default   |
| :-------- | :----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | :---------------- | :------------------------------------- | :-------- |
| P4-04     | D15 terminal function selection<br>27: Length count input<br>28: Length reset<br>29: Torque control prohibited<br>30: Pulse input (enabled only for DIS)<br>31: Reserved<br>32: Immediate DC braking<br>33: Normally closed (NC) input of external fault<br>34: Frequency modification enable<br>35: Reverse PID action direction<br>36: External STOP terminal 1<br>37: Command source switchover terminal 2<br>38: PID integral pause<br>39: Switchover between main frequency source X and preset frequency<br>40: Switchover between auxiliary frequency source Y and preset frequency<br>41: Reserved<br>42: Reserved<br>43: PID parameter switchover<br>44: User-defined fault 1<br>45: User-defined fault 2<br>46: Speed control/Torque control switchover<br>47: Emergency stop<br>48: External STOP terminal 2<br>49: Deceleration DC braking<br>50: Clear the current running time<br>51-59: Reserved | P4 Input Terminals | 27 - 59 (See description for options) | 12        |
| P4-10     | DI filter time                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    | P4 Input Terminals | 0.000s ~ 1.000s                        | 0.01s     |





### P4 Input Terminals (Page 24)

| parameter | description                                         | parameter group   | range                                                          | default   |
| :-------- | :-------------------------------------------------- | :---------------- | :------------------------------------------------------------- | :-------- |
| P4-11     | Terminal command mode                               | P4 Input Terminals | 0: Two-line mode 1<br>1: Two-line mode 2<br>2: Three-line mode 1 | 0         |
| P4-12     | Terminal UP/DOWN rate                             | P4 Input Terminals | 0.001Hz/s ~ 65.535Hz/s                                         | 1.00Hz/s  |
| P4-13     | AI curve 1 minimum input                          | P4 Input Terminals | 0.00V ~ P4-15                                                  | 0.00V     |
| P4-14     | Corresponding setting of AI curve 1 minimum input | P4 Input Terminals | -100.0% ~ +100.0%                                              | 0.0%      |
| P4-15     | AI curve 1 maximum input                          | P4 Input Terminals | P4-13 ~ +10.00V                                                | 10.00V    |
| P4-16     | Corresponding setting of AI curve 1 maximum input | P4 Input Terminals | -100.0% ~ +100.0%                                              | 100.0%    |
| P4-17     | AI1 filter time                                     | P4 Input Terminals | 0.00s ~ 10.00s                                                 | 0.10s     |
| P4-18     | AI curve 2 minimum input                          | P4 Input Terminals | 0.00V ~ P4-20                                                  | 0.00V     |
| P4-19     | Corresponding setting of AI curve 2 minimum input | P4 Input Terminals | -100.0% ~ +100.0%                                              | 0.0%      |
| P4-20     | AI curve 2 maximum input                          | P4 Input Terminals | P4-18 ~ +10.00V                                                | 10.00V    |
| P4-21     | Corresponding setting of AI curve 2 maximum input | P4 Input Terminals | -100.0% ~ +100.0%                                              | 10.00V    |
| P4-22     | AI2 filter time                                     | P4 Input Terminals | 0.00s ~ 10.00s                                                 | 0.10s     |
| P4-23     | AI curve 3 minimum input                          | P4 Input Terminals | -10.00V ~ P4-25                                                | -10.00V   |
| P4-24     | Corresponding setting of AI curve 3 minimum input | P4 Input Terminals | -100.0% ~ +100.0%                                              | -100.0%   |
| P4-25     | AI curve 3 maximum input                          | P4 Input Terminals | P4-23 ~ +10.00V                                                | 10.00V    |





### Function Codes (Page 25)

| parameter             | description                                                                                                                                                                                                                                                                                       | parameter group   | range             | default  |
| :-------------------- | :------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | :---------------- | :---------------- | :------- |
| P4-26                 | Corresponding setting of AI curve 3 maximum input                                                                                                                                                                                                                                                    | P4 Input Terminals | -100.0% ~ +100.0% | 100.0%   |
| P4-27                 | Panel potentiometer filter time                                                                                                                                                                                                                                                                 | P4 Input Terminals | 0.00s ~ 10.00s    | 0.10s    |
| P4-28                 | HDI Pulse minimum input                                                                                                                                                                                                                                                                          | P4 Input Terminals | 0.00kHz ~ P4-30   | 0.00kHz  |
| P4-29                 | Corresponding setting of HDI minimum input                                                                                                                                                                                                                                                         | P4 Input Terminals | -100.0% ~ 100.0%  | 0.0%     |
| P4-30                 | HDI maximum input                                                                                                                                                                                                                                                                                | P4 Input Terminals | P4-28 ~ 100.00kHz | 50.00kHz |
| P4-31                 | Corresponding setting of HDI pulse maximum Input                                                                                                                                                                                                                                                  | P4 Input Terminals | -100.0% ~ 100.0%  | 100.0%   |
| P4-32                 | HDI filter time                                                                                                                                                                                                                                                                                  | P4 Input Terminals | 0.00s ~ 10.00s    | 0.10s    |
| P4-33                 | AI curve selection: Unit's digit (AI1 curve selection) Curve 1 (2 points, see P4-13 to F4-16) Curve 2 (2 points, see P4-18 to F4-21) Curve 3 (2 points, see P4-23 to F4-26) Curve 4 (4 points, see A6-00 to A6-07) Curve 5 (4 points, see A6-08 to A6-15) Ten's digit (AI2 curve selection) Curve 1 to curve 5 (same as AI1) Hundred's digit (AI3 curve selection) Curve 1 to curve 5 (same as AI1) | P4 Input Terminals |                   | 321      |





### P4 Input Terminals - Page 26

| parameter                              | description                                                                                                                                                                                               | parameter group    | range          | default |
| :------------------------------------- | :-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | :----------------- | :------------- | :------ |
| P4-34 Setting for AI less than minimum input | Unit's digit (Setting for AI1 less than minimum input) 0: Minimum value 1: 0.0% <br> Ten's digit (Setting for AI2 less than minimum input) 0, 1 (same as AI1) <br> Hundred's digit (Setting for AI3 less than minimum input) 0, 1 (same as AI1) | P4 Input Terminals |                | 000     |
| P4-35 DI1 delay time                   | Time delay for DI1 activation                                                                                                                                                                             | P4 Input Terminals | 0.0s ~ 3600.0s | 0.0s    |
| P4-36 DI2 delay time                   | Time delay for DI2 activation                                                                                                                                                                             | P4 Input Terminals | 0.0s ~ 3600.0s | 0.0s    |
| P4-37 DI3 delay time                   | Time delay for DI3 activation                                                                                                                                                                             | P4 Input Terminals | 0.0s ~ 3600.0s | 0.0s    |
| P4-38 DI valid mode selection 1        | 0: High level valid <br> 1: Low level valid <br> Unit’s digit (DI1 valid mode) <br> Ten’s digit (DI2 valid mode) <br> Hundred’s digit (DI3 valid mode) <br> Thousand’s digit (DI4 valid mode) <br> Ten thousand’s digit (DI5 valid mode) | P4 Input Terminals |                | 00000   |
| P4-39 AI1 input voltage/ current selection | 0: Voltage input <br> 1: Current input                                                                                                                                                                   | P4 Input Terminals | 0 / 1          | 0       |




### Columns

- parameter
- description
- if provided, parameter group
- range
- default

### P5 Output Terminals - Page 27

| parameter   | description                    | parameter group      | range                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     | default   |
| :---------- | :----------------------------- | :------------------- | :---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | :-------- |
| P5-00       | FM terminal output mode        | P5 Output Terminals  | 0: Pulse output (FMP) <br> 1: Switch signal output (FMR)                                                                                                                                                                                                                                                                                                                                                                                                                                   | 0         |
| P5-01       | FMR output function selection  | P5 Output Terminals  | 0: No output <br> 1: AC drive running <br> 2: Fault output (stop) <br> 3: Frequency-level detection FDT1 output <br> 4: Frequency reached <br> 5: Zero-speed running (no output at stop) <br> 6: Motor overload pre-warning <br> 7: AC drive overload pre-warning <br> 8: Set count value reached <br> 9: Designated count value reached <br> 10: Length reached <br> 11: PLC cycle complete <br> 12: Accumulative running time reached <br> 13: Frequency limited <br> 14: Torque limited <br> 15: Ready for RUN <br> 16: AI1 > AI2 <br> 17: Frequency upper limit reached <br> 18: Frequency lower limit reached (operation related) <br> 19: Undervoltage state output <br> 20: Communication setting | 2         |





### P5 Output Terminals - Page 28

| parameter | description             | parameter group      | range                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       | default |
| :-------- | :---------------------- | :------------------- | :------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | :------ |
| P5-02     | Relay function (TA/TB/TC) | P5 Output Terminals | 21: Reserved <br> 22: Reserved <br> 23: Zero-speed running 2 (having output at stop) <br> 24: Accumulative power-on time reached <br> 25: Frequency level detection FDT2 output <br> 26: Frequency 1 reached output <br> 27: Frequency 2 reached output <br> 28: Current 1 reached output <br> 29: Current 2 reached output <br> 30: Timing reached output <br> 31: All input limit exceeded <br> 32: Load becoming 0 <br> 33: Reverse running <br> 34: Zero current state <br> 35: IGBT temperature reached <br> 36: Current limit exceeded <br> 37: Frequency lower limit reached [having output at stop] <br> 38: Alarm output <br> 39: Motor overheat warning <br> 40: Current running time reached <br> 41: Fault output (There is no output if it is the coast to stop fault and undervoltage occurs.) | 0       |





### P5 Output Terminals (Page 29)

| parameter | description | parameter group | range | default |
|---|---|---|---|---|
| P5-06 | FMP output function selection <br> 0: Running frequency <br> 1: Set frequency <br> 2: Output current <br> 3: Output torque (absolute value) <br> 4: Output power <br> 5: Output voltage <br> 6: HDI input (100.0% corresponds 100.0kHz) <br> 7: AI1 <br> 8: AI2 <br> 11: Count value <br> 12: Communication setting <br> 13: Motor rotational speed | P5 | 0 - 13 | 0 |
| P5-07 | AD1 output function selection <br> 14: Output current (100.0% corresponds 1000.0A) <br> 15: Output voltage (100.0% corresponds 1000.0V) <br> 16: Output torque (actual value) | P5 | 14 - 16 | 0 |
| P5-09 | Maximum FMP output frequency | P5 | 0.01kHz ~ 100.00kHz | 50.00kHz |
| P5-10 | AD1 offset coefficient | P5 | -100.0% ~ +100.0% | 0.0% |
| P5-11 | AO1 gain | P5 | -10.00 ~ +10.00 | 1.00 |
| P5-17 | FMR output delay time | P5 | 0.0s ~ 3600.0s | 0.0s |
| P5-18 | Relay 1 output delay time | P5 | 0.0s ~ 3600.0s | 0.0s |
| P5-19 | Relay 2 output delay time | P5 | 0.0s ~ 3600.0s | 0.0s |





### P6 Start/Stop Control Parameters (Page 30)

| parameter   | description                                                | parameter group      | range              | default   |
| :---------- | :--------------------------------------------------------- | :------------------- | :----------------- | :-------- |
| P6-00       | Start mode                                                 | P6 Start/Stop Control | 0: Direct start<br>1: Rotational speed tracking restart<br>2: Pre-excited start (asynchronous motor) | 0         |
| P6-01       | Rotational speed tracking mode                           | P6 Start/Stop Control | 0: From frequency at stop<br>1: From power frequency<br>2: From maximum frequency | 0         |
| P6-02       | Rotational speed tracking speed                          | P6 Start/Stop Control | 1 ~ 100            | 20        |
| P6-03       | Startup frequency                                        | P6 Start/Stop Control | 0.00Hz ~ 10.00Hz   | 0.00Hz    |
| P6-04       | Startup frequency holding time                           | P6 Start/Stop Control | 0.0s ~ 100.0s      | 0.0s      |
| P6-05       | Startup DC braking current/ Pre-excited current            | P6 Start/Stop Control | 0% ~ 100%          | 0%        |
| P6-06       | Startup DC braking time/ Pre-excited time                  | P6 Start/Stop Control | 0.0s ~ 100.0s      | 0.0s      |





### Function Codes (Page 31)

| parameter   | description                                 | parameter group     | range                                 | default   |
| :---------- | :------------------------------------------ | :------------------ | :------------------------------------ | :-------- |
| P6-07       | Acceleration/Deceleration mode              | P6 Start/Stop Control | 0: Linear acceleration/ deceleration<br>1: Static S-curve<br>2: Dynamic S-curve | 0         |
| P6-08       | Time proportion of S-curve start segment    | P6 Start/Stop Control | 0.0% ~ (100%-P6-09)                   | 30.0%     |
| P6-09       | Time proportion of S-curve end segment      | P6 Start/Stop Control | 0.0% ~ (100%-P6-08)                   | 30.0%     |
| P6-10       | Stop mode                                   | P6 Start/Stop Control | 0: Decelerate to stop<br>1: Coast to stop | 0         |
| P6-11       | Initial frequency of stop DC braking        | P6 Start/Stop Control | 0.00Hz ~ maximum frequency              | 0.00Hz    |
| P6-12       | Waiting time of stop DC braking             | P6 Start/Stop Control | 0.0s ~ 100.0s                           | 0.0s      |
| P6-13       | Stop DC braking current                     | P6 Start/Stop Control | 0% ~ 100%                               | 0%        |
| P6-14       | Stop DC braking time                        | P6 Start/Stop Control | 0.0s ~ 100.0s                           | 0.0s      |
| P6-15       | Brake use ratio                             | P6 Start/Stop Control | 0% ~ 100%                               | 100%      |





### P7 Operation Panel and Display - Page 32

| parameter | description              | parameter group              | range                                                                                                                                                                                                                                                                                                                                                                                               | default |
| :-------- | :----------------------- | :--------------------------- | :-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | :------ |
| PT-01     | MF.K Key function selection | P7 Operation Panel and Display | 0: MF.K key disabled<br>1: Switchover between operation panel control and remote command control (terminal or communication)<br>2: Switchover between forward rotation and reverse rotation<br>3: Forward JOG<br>4: Reverse JOG                                                                                                                                                                       | 0       |
| PT-02     | STOP/RESET key function   | P7 Operation Panel and Display | 0: STOP/RESET key enabled only in operation panel control<br>1: STOP/RESET key enabled in any operation mode                                                                                                                                                                                                                                                                                       | 1       |
| PT-03     | LED display running parameters 1 | P7 Operation Panel and Display | 0000-FFFF<br>Bit00: Running frequency 1 (Hz)<br>Bit01: Set Frequency (Hz)<br>Bit02: Bus voltage (V)<br>Bit03: Output voltage (V)<br>Bit04: Output current (A)<br>Bit05: Output power (kW)<br>Bit06: Output torque (%)<br>Bit07: DI input status<br>Bit08: DO output status<br>Bit09: AI1 voltage (V)<br>Bit10: AI2 voltage (V)<br>Bit11: Panel potentiometer voltage (V)<br>Bit12: Count value<br>Bit13: Length value<br>Bit14: Load speed display<br>Bit15: PID setting | 1F      |





### P7 Operation Panel and Display - Page 33

| parameter   | description                         | parameter group               | range                                                                                                                                                                                                                                                                                                                                                                                                                            | default   |
| :---------- | :---------------------------------- | :---------------------------- | :------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | :-------- |
| PT-04       | LED display running parameters 2    | P7 Operation Panel and Display | 0000-FFFF<br>Bit00: PID feedback<br>Bit01: PLC stage<br>Bit02: HDI setting frequency (kHz)<br>Bit03: Running frequency 2 (Hz)<br>Bit04: Remaining running time<br>Bit05: AI1 voltage before correction (V)<br>Bit06: AI2<br>Bit07: Panel potentiometer voltage before correction (V)<br>Bit08: Linear speed<br>Bit09: Current power-on time (Hour)<br>Bit10: Current running time (Min)<br>Bit11: HDI setting frequency (Hz)<br>Bit12: Communication setting value<br>Bit13: Encoder feedback speed (Hz)<br>Bit14: Main frequency X display (Hz)<br>Bit15: Auxiliary frequency Y display (Hz) | 0         |
| PT-05       | LED display stop parameters         | P7 Operation Panel and Display | 0000-FFFF<br>Bit00: Set frequency (Hz)<br>Bit01: Bus voltage (V)<br>Bit02: DI input status<br>Bit03: DO output status<br>Bit04: AI1 voltage (V)<br>Bit05: AI2 voltage (V)<br>Bit06: Potentiometer voltage (V)<br>Bit07: Count value<br>Bit08: Length value<br>Bit09: PLC stage<br>Bit10: Load speed<br>Bit11: PID setting<br>Bit12: HDI setting frequency (kHz)                                                               | 33        |






### P7 Operation Panel and Display - Page 34


| parameter | description                             | parameter group               | range                                                                                                                                                                                                | default |
| :-------- | :-------------------------------------- | :---------------------------- | :--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | :------ |
| P7-06     | Load speed display coefficient          | P7 Operation Panel and Display | 0.0001 ~ 6.5000                                                                                                                                                                                      | 1.0000  |
| P7-07     | Heatsink temperature of AC drive IGBT     | P7 Operation Panel and Display | 0°C ~ 120°C                                                                                                                                                                                          | -       |
| P7-09     | Accumulative running time               | P7 Operation Panel and Display | 0h ~ 65535h                                                                                                                                                                                          | -       |
| P7-12     | Number of decimal places for load speed display | P7 Operation Panel and Display | Unit' digit: U0-14 decimal number <br> 0: 0 decimal place <br> 1: 1 decimal place <br> 2: 2 decimal places <br> 3: 3 decimal places <br> Ten' digit: U0-19/UD-29 decimal number <br> 0: 0 decimal place <br> 1: 1 decimal place | 21      |
| P7-13     | Accumulative power-on time              | P7 Operation Panel and Display | 0 ~ 65535 h                                                                                                                                                                                          | -       |
| P7-14     | Accumulative power consumption          | P7 Operation Panel and Display | 0 ~ 65535 kwh                                                                                                                                                                                        | -       |


## Error Codes


Skipping Number Page : Nothing found




### P8 Auxiliary Functions - Page 35

| parameter   | description                     | parameter group   | range                            | default         |
| :---------- | :------------------------------ | :---------------- | :------------------------------- | :-------------- |
| P8-00       | JOG running frequency         | P8                | 0.00Hz ~ maximum frequency       | 2.00Hz          |
| P8-01       | JOG acceleration time         | P8                | 0.0s ~ 6500.0s                   | 20.0s           |
| P8-02       | JOG deceleration time         | P8                | 0.0s ~ 6500.0s                   | 20.0s           |
| P8-03       | Acceleration time 2           | P8                | 0.0s ~ 6500.0s                   | Model dependent |
| P8-04       | Deceleration time 2           | P8                | 0.0s ~ 6500.0s                   | Model dependent |
| P8-05       | Acceleration time 3           | P8                | 0.0s ~ 6500.0s                   | Model dependent |
| P8-06       | Deceleration time 3           | P8                | 0.0s ~ 6500.0s                   | Model dependent |
| P8-07       | Acceleration time 4           | P8                | 0.0s ~ 6500.0s                   | Model dependent |
| P8-08       | Deceleration time 4           | P8                | 0.0s ~ 6500.0s                   | Model dependent |
| P8-09       | Jump frequency 1              | P8                | 0.00Hz ~ maximum frequency       | 0.00Hz          |
| P8-10       | Jump frequency 2              | P8                | 0.00Hz ~ maximum frequency       | 0.00Hz          |
| P8-11       | Frequency jump amplitude      | P8                | 0.00Hz ~ maximum frequency       | 0.01Hz          |
| P8-12       | Forward/Reverse rotation dead-zone time | P8                | 0.0s ~ 3000.0s                   | 0.0s            |
| P8-13       | Reverse control                 | P8                | 0: Enabled 1: Disabled           | 0               |





### P8 Auxiliary Functions (Page 36)

### Columns

| parameter | description                                                                    | parameter group | range                                                  | default |
| :-------- | :----------------------------------------------------------------------------- | :-------------- | :----------------------------------------------------- | :------ |
| P8-14     | Running mode when set frequency lower than frequency lower limit                 | P8              | 0: Run at frequency lower limit 1: Stop 2: Run at zero speed | 0       |
| P8-15     | Droop control                                                                  | P8              | 0.00Hz ~ 10.00Hz                                       | 0.00Hz  |
| P8-16     | Accumulative power-on time threshold                                           | P8              | 0h ~ 65000h                                            | 0h      |
| P8-17     | Accumulative running time threshold                                            | P8              | 0h ~ 65000h                                            | 0h      |
| P8-18     | Startup protection                                                             | P8              | 0: No 1: Yes                                           | 0       |
| P8-19     | Frequency detection value (FDTL)                                              | P8              | 0.00Hz ~ maximum frequency                             | 50.00Hz |
| P8-20     | Frequency detection hysteresis (FDT hysteresis 1)                             | P8              | 0.0% ~ 100.0%(FDT1 electrical level)                  | 5.0%    |
| P8-21     | Detection range of frequency reached                                            | P8              | 0.0% ~ 100.0%(maximum frequency)                     | 0.0%    |
| P8-22     | Jump frequency during acceleration/deceleration                                | P8              | 0: Disabled 1: Enabled                                 | 0       |
| P8-25     | Frequency switchover point between acceleration time 1 and acceleration time 2 | P8              | 0.00Hz ~ maximum frequency                             | 0.00Hz  |
| P8-26     | Frequency switchover point between deceleration time 1 and deceleration time 2 | P8              | 0.00Hz ~ maximum frequency                             | 0.00Hz  |
| P8-27     | Terminal JOG preferred                                                         | P8              | 0: Disabled 1: Enabled                                 | 0       |





### P8 Auxiliary Functions (Page 37)

| parameter | description                                       | parameter group       | range                                                                 | default   |
| :-------- | :------------------------------------------------ | :-------------------- | :-------------------------------------------------------------------- | :-------- |
| P8-28     | Frequency detection value (FDT2)                  | P8 Auxiliary Functions | 0.00Hz ~ maximum frequency                                            | 50.00Hz   |
| P8-29     | Frequency detection hysteresis (FDT hysteresis 2) | P8 Auxiliary Functions | 0.0% ~ 100.0% (FDT2 electrical level)                                 | 5.0%      |
| P8-30     | Any frequency reaching detection value 1          | P8 Auxiliary Functions | 0.00Hz ~ maximum frequency                                            | 50.00Hz   |
| P8-31     | Any frequency reaching detection amplitude 1      | P8 Auxiliary Functions | 0.0% ~ 100.0% (maximum frequency)                                     | 0.0%      |
| P8-32     | Any frequency reaching detection value 2          | P8 Auxiliary Functions | 0.00Hz ~ maximum frequency                                            | 50.00Hz   |
| P8-33     | Any frequency reaching detection amplitude 2      | P8 Auxiliary Functions | 0.0% ~ 100.0% (maximum frequency)                                     | 5.0%      |
| P8-34     | Zero current detection level                      | P8 Auxiliary Functions | 0.0% ~ 300.0% rated motor current                                     | 5.0%      |
| P8-35     | Zero current detection delay time                 | P8 Auxiliary Functions | 0.01s ~ 600.00s                                                       | 0.10s     |
| P8-36     | Output overcurrent threshold                      | P8 Auxiliary Functions | 0.0% (no detection)<br>0.1% - 300.0% (rated motor current)            | 200.0%    |
| P8-37     | Output overcurrent detection delay time           | P8 Auxiliary Functions | 0.00s ~ 600.00s                                                       | 0.00s     |
| P8-38     | Any current reaching 1                            | P8 Auxiliary Functions | 0.0% - 300.0% (rated motor current)                                   | 100.0%    |
| P8-39     | Any current reaching 1 amplitude                  | P8 Auxiliary Functions | 0.0% ~ 300.0% (rated motor current)                                   | 0.0%      |
| P8-40     | Any current reaching 2                            | P8 Auxiliary Functions | 0.0% ~ 300.0% (rated motor current)                                   | 100.0%    |





### P8 Auxiliary Functions page 38

| parameter | description                        | parameter group       | range                                                                                           | default   |
| :-------- | :--------------------------------- | :-------------------- | :---------------------------------------------------------------------------------------------- | :-------- |
| P8-41     | Any current reaching 2 amplitude   | P8 Auxiliary Functions | 0.0% ~ 300.0% (rated motor current)                                                             | 0.0%      |
| P8-42     | Timing function                    | P8 Auxiliary Functions | 0: Disabled 1: Enabled                                                                          | 0         |
| P8-43     | Timing duration source             | P8 Auxiliary Functions | 0: P8-44 1: AI1 2: AI2 3: Panel potentiometer (analog input corresponds to the value of F8-44) | 0         |
| P8-44     | Timing duration                    | P8 Auxiliary Functions | 0.0Min ~ 6500.0Min                                                                              | 0.0Min    |
| P8-45     | AI1 input voltage lower limit      | P8 Auxiliary Functions | 0.00V ~ P8-46                                                                                   | 3.10V     |
| P8-46     | AI1 input voltage upper limit      | P8 Auxiliary Functions | P8-45 ~ 10.00V                                                                                  | 6.80V     |
| P8-47     | IGBT temperature threshold         | P8 Auxiliary Functions | 0°C ~ 100°C                                                                                     | 75°C      |
| P8-49     | Wakeup frequency                   | P8 Auxiliary Functions | Dormant frequency (P8-51) ~ maximum frequency (P0-10)                                         | 0.00Hz    |
| P8-50     | Wakeup delay time                  | P8 Auxiliary Functions | 0.0s ~ 6500.0s                                                                                  | 0.0s      |
| P8-51     | Dormant frequency                  | P8 Auxiliary Functions | 0.00Hz ~ wakeup frequency (P8-49)                                                               | 0.00Hz    |
| P8-52     | Dormant delay time                 | P8 Auxiliary Functions | 0.0s ~ 6500.0s                                                                                  | 0.0s      |
| P8-53     | Current running time reached       | P8 Auxiliary Functions | 0.0 ~ 6500.0 min                                                                                | 0.0Min    |
| P8-54     | Output power correction coefficient | P8 Auxiliary Functions | 0.00% ~ 200.0%                                                                                  | 100.0%    |


### P9 Fault and Protection Parameters - Page 39

| parameter | description                                               | parameter group        | range                  | default   |
| :-------- | :-------------------------------------------------------- | :--------------------- | :--------------------- | :-------- |
| P9-00     | Motor overload protection selection                       | P9 Fault and Protection | 0: Disabled 1: Enabled | 1         |
| P9-01     | Motor overload protection gain                            | P9 Fault and Protection | 0.20 ~ 10.00           | 1.00      |
| P9-02     | Motor overload warning coefficient                      | P9 Fault and Protection | 50% ~ 100%             | 80%       |
| P9-03     | Overvoltage stall gain                                    | P9 Fault and Protection | 0 ~ 100                | 0         |
| P9-04     | Overvoltage stall protective voltage                    | P9 Fault and Protection | 650 ~ 780V             | 760V      |
| P9-05     | Overcurrent stall gain                                    | P9 Fault and Protection | 0 ~ 100                | 20        |
| P9-06     | Overcurrent




### Columns

- parameter
- description
- if provided, parameter group
- range
- default

### P9 Fault and Protection Parameters Page 40

| parameter   | description               | parameter group         | range                               | default   |
| :---------- | :------------------------ | :---------------------- | :---------------------------------- | :-------- |
| P9-14       | 1st fault type            | P9 Fault and Protection | Displays fault code (See Error Codes) | -         |
| P9-15       | 2nd fault type            | P9 Fault and Protection | Displays fault code (See Error Codes) | -         |
| P9-16       | 3rd (latest) fault type | P9 Fault and Protection | Displays fault code (See Error Codes) | -         |

## Error Codes

### Columns

- code
- description
- if provided, parameter group

### P9 Fault and Protection Codes Page 40

| code   | description                             | parameter group         |
| :----- | :-------------------------------------- | :---------------------- |
| 0      | No fault                                | P9 Fault and Protection |
| 1      | Reserved                                | P9 Fault and Protection |
| 2      | Overcurrent during acceleration         | P9 Fault and Protection |
| 3      | Overcurrent during deceleration         | P9 Fault and Protection |
| 4      | Overcurrent at constant speed           | P9 Fault and Protection |
| 5      | Overvoltage during acceleration         | P9 Fault and Protection |
| 6      | Overvoltage during deceleration         | P9 Fault and Protection |
| 7      | Overvoltage at constant speed           | P9 Fault and Protection |
| 8      | Buffer resistance overload              | P9 Fault and Protection |
| 9      | Undervoltage                            | P9 Fault and Protection |
| 10     | AC drive overload                       | P9 Fault and Protection |
| 11     | Motor overload                          | P9 Fault and Protection |
| 12     | Power input phase loss                  | P9 Fault and Protection |
| 13     | Power output phase loss                 | P9 Fault and Protection |
| 14     | IGBT overheat                           | P9 Fault and Protection |
| 15     | External equipment fault                | P9 Fault and Protection |
| 16     | Communication fault                     | P9 Fault and Protection |
| 17     | Contactor fault                         | P9 Fault and Protection |
| 18     | Current detection fault                 | P9 Fault and Protection |
| 19     | Motor auto-tuning fault                 | P9 Fault and Protection |
| 21     | EEPROM read-write fault                 | P9 Fault and Protection |
| 22     | AC drive hardware fault                 | P9 Fault and Protection |
| 23     | Short circuit to ground                 | P9 Fault and Protection |
| 24     | Reserved                                | P9 Fault and Protection |
| 25     | Reserved                                | P9 Fault and Protection |
| 26     | Accumulative running time reached       | P9 Fault and Protection |
| 27     | User-defined fault 1                    | P9 Fault and Protection |
| 28     | User-defined fault 2                    | P9 Fault and Protection |
| 29     | Accumulative power-on time reached      | P9 Fault and Protection |
| 30     | Load becoming 0                         | P9 Fault and Protection |
| 31     | PID feedback lost during running        | P9 Fault and Protection |
| 40     | Current limit fault                     | P9 Fault and Protection |
| 41     | Motor switchover fault during running   | P9 Fault and Protection |
| 42     | Too large speed deviation               | P9 Fault and Protection |
| 43     | Motor over-speed                        | P9 Fault and Protection |






### P9 Fault and Protection page 41

| parameter | description                             | parameter group           | range   | default   |
| :-------- | :-------------------------------------- | :------------------------ | :------ | :-------- |
| P9-17     | Frequency upon 3<sup>rd</sup> fault      | P9 Fault and Protection | -       | -         |
| P9-18     | Current upon 3<sup>rd</sup> fault        | P9 Fault and Protection | -       | -         |
| P9-19     | Bus voltage upon 3<sup>rd</sup> fault    | P9 Fault and Protection | -       | -         |
| P9-20     | Input terminal status upon 3<sup>rd</sup> fault | P9 Fault and Protection | -       | -         |
| P9-21     | Output terminal status upon 3<sup>rd</sup> fault| P9 Fault and Protection | -       | -         |
| P9-22     | AC drive status upon 3<sup>rd</sup> fault  | P9 Fault and Protection | -       | -         |
| P9-23     | Power-on time upon 3<sup>rd</sup> fault  | P9 Fault and Protection | -       | -         |
| P9-24     | Running time upon 3<sup>rd</sup> fault   | P9 Fault and Protection | -       | -         |
| P9-27     | Frequency upon 2<sup>nd</sup> fault     | P9 Fault and Protection | -       | -         |
| P9-28     | Current upon 2<sup>nd</sup> fault       | P9 Fault and Protection | -       | -         |
| P9-29     | Bus voltage upon 2<sup>nd</sup> fault   | P9 Fault and Protection | -       | -         |
| P9-30     | Input terminal status upon 2<sup>nd</sup> fault| P9 Fault and Protection | -       | -         |
| P9-31     | Output terminal status upon 2<sup>nd</sup> fault| P9 Fault and Protection | -       | -         |
| P9-32     | AC drive status upon 2<sup>nd</sup> fault  | P9 Fault and Protection | -       | -         |
| P9-33     | Power on time upon 2<sup>nd</sup> fault   | P9 Fault and Protection | -       | -         |
| P9-34     | Running time upon 2<sup>nd</sup> fault   | P9 Fault and Protection | -       | -         |
| P9-37     | Frequency upon 1<sup>st</sup> fault     | P9 Fault and Protection | -       | -         |
| P9-38     | Current upon 1<sup>st</sup> fault        | P9 Fault and Protection | -       | -         |





### Columns

- `parameter`: The function code identifier.
- `description`: The name or description of the parameter.
- `parameter group`: The group the parameter belongs to (if specified).
- `range`: The allowed setting range for the parameter.
- `default`: The default value of the parameter.

### P9 Fault and Protection - Page 42

| parameter   | description                             | parameter group         | range                                                                                                                                                                                                                                                                                                                                                                               | default   |
| :---------- | :-------------------------------------- | :---------------------- | :---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | :-------- |
| P9-39       | Bus voltage upon 1st fault              | P9 Fault and Protection | —                                                                                                                                                                                                                                                                                                                                                                                   | —         |
| P9-40       | Input terminal status upon 1st fault    | P9 Fault and Protection | —                                                                                                                                                                                                                                                                                                                                                                                   | —         |
| P9-41       | Output terminal status upon 1st fault   | P9 Fault and Protection | —                                                                                                                                                                                                                                                                                                                                                                                   | —         |
| P9-42       | AC drive status upon 1st fault          | P9 Fault and Protection | —                                                                                                                                                                                                                                                                                                                                                                                   | —         |
| P9-43       | Power-on time upon 1st fault            | P9 Fault and Protection | —                                                                                                                                                                                                                                                                                                                                                                                   | —         |
| P9-44       | Running time upon 1st fault             | P9 Fault and Protection | —                                                                                                                                                                                                                                                                                                                                                                                   | —         |
| P9-47       | Fault protection action selection 1 | P9 Fault and Protection | Unit's digit (Motor overload, 11) <br> 0: Coast to stop <br> 1: Stop according to the stop mode <br> 2: Continue to run <br> Ten's digit (Power input phase loss, 12) <br> Same as unit's digit <br> Hundred's digit (Power output phase loss, 13) <br> Same as unit's digit <br> Thousand's digit (External equipment fault, 15) <br> Same as unit's digit <br> Ten thousand's digit (Communication fault, 16) <br> Same as unit's digit | 00000     |





### Columns

- parameter
- description
- if provided, parameter group
- range
- default

### P9 Fault and Protection (Page 43)

| parameter | description | parameter group | range | default |
|---|---|---|---|---|
| P9-54 | Frequency selection for continuing to run upon fault | P9 Fault and Protection | 0: Current running frequency <br> 1: Set frequency <br> 2: Frequency upper limit <br> 3: Frequency lower limit <br> 4: Backup frequency upon abnormality | 00000 |
| P9-55 | Backup frequency upon abnormality | P9 Fault and Protection | 0.0% ~ 100.0% <br> (100.0% corresponds to maximum frequency P0-10) | 100.0% |
| P9-59 | Action selection at instantaneous power failure | P9 Fault and Protection | 0: Invalid <br> 1: Bus voltage constant control <br> 2: Decelerate to stop | 0 |
| P9-60 | Action pause judging voltage at instantaneous power failure | P9 Fault and Protection | 80% ~ 100.0% | 85.0% |
| P9-61 | Voltage rally judging time at instantaneous power failure | P9 Fault and Protection | 0.5s | 0.5s |
| P9-62 | Action judging bus voltage at instantaneous power failure | P9 Fault and Protection | 80% ~ 100.0% | 80.0% |
| P9-63 | Protection upon load becoming 0 | P9 Fault and Protection | 0: Disabled <br> 1: Enabled | 0 |
| P9-64 | Detection level of load becoming 0 | P9 Fault and Protection | 0.0 ~ 100.0% | 10.0% |
| P9-65 | Detection time of load becoming 0 | P9 Fault and Protection | 0.0 ~ 60.0s | 1.0s |





### PA PID Function Parameters (Page 44)

| parameter | description                  | parameter group   | range                                                                                                                                   | default   |
| :-------- | :--------------------------- | :---------------- | :-------------------------------------------------------------------------------------------------------------------------------------- | :-------- |
| PA-00     | PID setting source           | PA PID Function | 0: PA-01<br>1: AI1<br>2: AI2<br>3: Panel potentiometer<br>4: HDI Pulse setting (DI5)<br>5: Communication setting<br>6: Multi-reference | 0         |
| PA-01     | PID digital setting          | PA PID Function | 0.0% ~ 100.0%                                                                                                                           | 50.0%     |
| PA-02     | PID feedback source          | PA PID Function | 0: AI1<br>1: AI2<br>2: Panel potentiometer<br>3: AI1 - AI2<br>4: HDI Pulse setting (DI5)<br>5: Communication setting<br>6: AI1 + AI2<br>7: MAX (\|AI1\|, \|AI2\|)<br>8: MIN (\|AI1\|, \|AI2\|) | 0         |
| PA-03     | PID action direction         | PA PID Function | 0: Forward action<br>1: Reverse action                                                                                               | 0         |
| PA-04     | PID setting feedback range | PA PID Function | 0 ~ 65535                                                                                                                               | 1000      |
| PA-05     | Proportional gain Kp1      | PA PID Function | 0.0 ~ 100.0                                                                                                                             | 20.0      |
| PA-06     | Integral time Ti1          | PA PID Function | 0.01s ~ 10.00s                                                                                                                          | 2.00s     |
| PA-07     | Differential time Td1      | PA PID Function | 0.000s ~ 10.000s                                                                                                                        | 0.000s    |




### Columns

| parameter   | description                             | parameter group   | range                 | default   |
| :---------- | :-------------------------------------- | :---------------- | :-------------------- | :-------- |
| PA-08       | Cut-off frequency of PID reverse rotation | PA PID Function   | 0.00 ~ maximum frequency | 2.00Hz    |
| PA-09       | PID deviation limit                     | PA PID Function   | 0.0% ~ 100.0%         | 0.0%      |
| PA-10       | PID differential limit                  | PA PID Function   | 0.00% ~ 100.00%       | 0.10%     |
| PA-11       | PID setting change time                 | PA PID Function   | 0.00 ~ 650.00s        | 0.00s     |
| PA-12       | PID feedback filter time                | PA PID Function   | 0.00 ~ 60.00s         | 0.00s     |
| PA-13       | PID output filter time                  | PA PID Function   | 0.00 ~ 60.00s         | 0.00s     |
| PA-15       | Proportional gain Kp2                   | PA PID Function   | 0.0 ~ 100.0           | 20.0      |
| PA-16       | Integral time Ti2                       | PA PID Function   | 0.01s ~ 10.00s        | 2.00s     |
| PA-17       | Differential time Td2                   | PA PID Function   | 0.000s ~ 10.000s      | 0.000s    |
| PA-18       | PID parameter switchover condition      | PA PID Function   | 0: No switchover<br>1: Switchover via DI<br>2: Automatic switchover based on deviation<br>3: Automatic switchover based on running frequency | 0         |
| PA-19       | PID parameter switchover deviation 1    | PA PID Function   | 0.0% ~ PA-20          | 20.0%     |
| PA-20       | PID parameter switchover deviation 2    | PA PID Function   | PA-19 ~ 100.0%        | 80.0%     |

### PA PID Function Parameters Page 45





### Columns

- parameter
- description
- if provided, parameter group
- range
- default

### PA PID Function Parameters (Page 46)

| parameter   | description                                                                                                                                                                                   | parameter group   | range                                                               | default   |
| :---------- | :-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | :---------------- | :------------------------------------------------------------------ | :-------- |
| PA-21       | PID initial value                                                                                                                                                                             | PA                | 0.0% ~ 100.0%                                                       | 0.0%      |
| PA-22       | PID initial value holding time                                                                                                                                                                | PA                | 0.00 ~ 650.00%                                                      | 0.00%     |
| PA-23       | Maximum deviation between two PID outputs in forward direction                                                                                                                                | PA                | 0.00% ~ 100.00%                                                     | 1.00%     |
| PA-24       | Maximum deviation between two PID outputs in reverse direction                                                                                                                                | PA                | 0.00% ~ 100.00%                                                     | 1.00%     |
| PA-25       | PID integral property. Unit's digit (Integral separated): 0: Invalid, 1: Valid. Ten's digit (Whether to stop integral operation when the output reaches the limit): 0: Continue, 1: Stop. | PA                | Unit's digit (0, 1), Ten's digit (0, 1)                             | 00        |
| PA-26       | Detection value of PID feedback loss                                                                                                                                                          | PA                | 0.0%: Not judging feedback loss; 0.1% ~ 100.0%                   | 0.0%      |
| PA-27       | Detection time of PID feedback loss                                                                                                                                                           | PA                | 0.0s ~ 20.0s                                                        | 0.0s      |
| PA-28       | PID operation at stop                                                                                                                                                                         | PA                | 0: No PID operation at stop, 1: PID operation at stop                 | 0         |





### PB Swing Frequency, Fixed Length and Count Parameters (Page 47)

| parameter   | description                   | parameter group   | range                             | default   |
| :---------- | :---------------------------- | :---------------- | :-------------------------------- | :-------- |
| PB-00       | Swing frequency setting mode  | PB                | 0: Relative to the central frequency<br>1: Relative to the maximum frequency | 0         |
| PB-01       | Swing frequency amplitude     | PB                | 0.0% ~ 100.0%                     | 0.0%      |
| PB-02       | Jump frequency amplitude      | PB                | 0.0% ~ 50.0%                      | 0.0%      |
| PB-03       | Swing frequency cycle         | PB                | 0.1s ~ 3000.0s                    | 10.0s     |
| PB-04       | Triangular wave rising time coefficient | PB                | 0.1% ~ 100.0%                     | 50.0%     |
| PB-05       | Set length                    | PB                | 0m ~ 65535m                       | 1000m     |
| PB-06       | Actual length                 | PB                | 0m ~ 65535m                       | 0m        |
| PB-07       | Number of pulses per meter    | PB                | 0.1 ~ 6553.5                      | 100.0     |
| PB-08       | Set count value               | PB                | 1 ~ 65535                         | 1000      |
| PB-09       | Designated count value        | PB                | 1 ~ 65535                         | 1000      |





### PC Multi-Reference and Simple PLC Function Page 48

| parameter       | description                                                                                                     | range              | default   |
| :-------------- | :-------------------------------------------------------------------------------------------------------------- | :----------------- | :-------- |
| PC-00 Reference 0 | Reference 0                                                                                                     | -100.0% ~ 100.0%   | 0.0%      |
| PC-01 Reference 1 | Reference 1                                                                                                     | -100.0% ~ 100.0%   | 0.0%      |
| PC-02 Reference 2 | Reference 2                                                                                                     | -100.0% ~ 100.0%   | 0.0%      |
| PC-03 Reference 3 | Reference 3                                                                                                     | -100.0% ~ 100.0%   | 0.0%      |
| PC-04 Reference 4 | Reference 4                                                                                                     | -100.0% ~ 100.0%   | 0.0%      |
| PC-05 Reference 5 | Reference 5                                                                                                     | -100.0% ~ 100.0%   | 0.0%      |
| PC-06 Reference 6 | Reference 6                                                                                                     | -100.0% ~ 100.0%   | 0.0%      |
| PC-07 Reference 7 | Reference 7                                                                                                     | -100.0% ~ 100.0%   | 0.0%      |
| PC-08 Reference 8 | Reference 8                                                                                                     | -100.0% ~ 100.0%   | 0.0%      |
| PC-09 Reference 9 | Reference 9                                                                                                     | -100.0% ~ 100.0%   | 0.0%      |
| PC-10 Reference 10 | Reference 10                                                                                                    | -100.0% ~ 100.0%   | 0.0%      |
| PC-11 Reference 11 | Reference 11                                                                                                    | -100.0% ~ 100.0%   | 0.0%      |
| PC-12 Reference 12 | Reference 12                                                                                                    | -100.0% ~ 100.0%   | 0.0%      |
| PC-13 Reference 13 | Reference 13                                                                                                    | -100.0% ~ 100.0%   | 0.0%      |
| PC-14 Reference 14 | Reference 14                                                                                                    | -100.0% ~ 100.0%   | 0.0%      |
| PC-15 Reference 15 | Reference 15                                                                                                    | -100.0% ~ 100.0%   | 0.0%      |
| PC-16 Simple PLC running mode | Simple PLC running mode: 0: Stop after the AC drive runs one cycle, 1: Keep final values..., 2: Repeat after... | 0 / 1 / 2          | 0         |





### PC Multi-Reference and Simple PLC Function Parameters (Page 49)

| parameter | description                                           | parameter group                           | range                                  | default |
| :-------- | :---------------------------------------------------- | :---------------------------------------- | :------------------------------------- | :------ |
| PC-17     | Simple PLC retentive selection                         | PC Multi-Reference and Simple PLC Function | Unit's digit: 0: No, 1: Yes<br>Ten's digit: 0: No, 1: Yes | 00      |
| PC-18     | Running time of simple PLC reference 0                 | PC Multi-Reference and Simple PLC Function | 0.0s/h ~ 6553.5s/h                     | 0.0s/h  |
| PC-19     | Acceleration/deceleration time of simple PLC reference 0 | PC Multi-Reference and Simple PLC Function | 0 ~ 3                                  | 0       |
| PC-20     | Running time of simple PLC reference 1                 | PC Multi-Reference and Simple PLC Function | 0.0s/h ~ 6553.5s/h                     | 0.0s/h  |
| PC-21     | Acceleration/deceleration time of simple PLC reference 1 | PC Multi-Reference and Simple PLC Function | 0 ~ 3                                  | 0       |
| PC-22     | Running time of simple PLC reference 2                 | PC Multi-Reference and Simple PLC Function | 0.0s/h ~ 6553.5s/h                     | 0.0s/h  |
| PC-23     | Acceleration/deceleration time of simple PLC reference 2 | PC Multi-Reference and Simple PLC Function | 0 ~ 3                                  | 0       |
| PC-24     | Running time of simple PLC reference 3                 | PC Multi-Reference and Simple PLC Function | 0.0s/h ~ 6553.5s/h                     | 0.0s/h  |
| PC-25     | Acceleration/deceleration time of simple PLC reference 3 | PC Multi-Reference and Simple PLC Function | 0 ~ 3                                  | 0       |
| PC-26     | Running time of simple PLC reference 4                 | PC Multi-Reference and Simple PLC Function | 0.0s/h ~ 6553.5s/h                     | 0.0s/h  |
| PC-27     | Acceleration/deceleration time of simple PLC reference 4 | PC Multi-Reference and Simple PLC Function | 0 ~ 3                                  | 0       |





### Columns

| parameter | description                             | if provided, parameter group   | range                    | default   |
| :-------- | :-------------------------------------- | :----------------------------- | :----------------------- | :-------- |
| PC-28     | Running time of simple PLC reference 5  |                                | 0.0s[h] ~ 6553.5s[h]     | 0.0s(h)   |
| PC-29     | Acceleration/deceleration time of simple PLC reference 5 |                                | 0~3                      | 0         |
| PC-30     | Running time of simple PLC reference 6  |                                | 0.0s[h] ~ 6553.5s[h]     | 0.0s(h)   |
| PC-31     | Acceleration/deceleration time of simple PLC reference 6 |                                | 0~3                      | 0         |
| PC-32     | Running time of simple PLC reference 7  |                                | 0.0s[h] ~ 6553.5s[h]     | 0.0s(h)   |
| PC-33     | Acceleration/deceleration time of simple PLC reference 7 |                                | 0~3                      | 0         |
| PC-34     | Running time of simple PLC reference 8  |                                | 0.0s[h] ~ 6553.5s[h]     | 0.0s(h)   |
| PC-35     | Acceleration/deceleration time of simple PLC reference 8 |                                | 0~3                      | 0         |
| PC-36     | Running time of simple PLC reference 9  |                                | 0.0s[h] ~ 6553.5s[h]     | 0.0s(h)   |
| PC-37     | Acceleration/deceleration time of simple PLC reference 9 |                                | 0~3                      | 0         |
| PC-38     | Running time of simple PLC reference 10 |                                | 0.0s[h] ~ 6553.5s[h]     | 0.0s(h)   |
| PC-39     | Acceleration/deceleration time of simple PLC reference 10|                                | 0~3                      | 0         |
| PC-40     | Running time of simple PLC reference 11 |                                | 0.0s[h] ~ 6553.5s[h]     | 0.0s(h)   |
| PC-41     | Acceleration/deceleration time of simple PLC reference 11|                                | 0~3                      | 0         |

### PC Multi-Reference and Simple PLC Function - Page 50

## Error Codes

Skipping Number Page : Nothing found





### Columns

-   parameter
-   description
-   if provided, parameter group
-   range
-   default

### PC Multi-Reference and Simple PLC Function (Page 51)

| parameter | description                                                   | range                                                                                                                                                                                                               | default   |
| :-------- | :------------------------------------------------------------ | :------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | :-------- |
| PC-42     | Running time of simple PLC reference 12                       | 0.0s(h) ~ 6553.5s(h)                                                                                                                                                                                                | 0.0s(h)   |
| PC-43     | Acceleration/deceleration time of simple PLC reference 12     | 0 ~ 3                                                                                                                                                                                                               | 0         |
| PC-44     | Running time of simple PLC reference 13                       | 0.0s(h) ~ 6553.5s(h)                                                                                                                                                                                                | 0.0s(h)   |
| PC-45     | Acceleration/deceleration time of simple PLC reference 13     | 0 ~ 3                                                                                                                                                                                                               | 0         |
| PC-46     | Running time of simple PLC reference 14                       | 0.0s(h) ~ 6553.5s(h)                                                                                                                                                                                                | 0.0s(h)   |
| PC-47     | Acceleration/deceleration time of simple PLC reference 14     | 0 ~ 3                                                                                                                                                                                                               | 0         |
| PC-48     | Running time of simple PLC reference 15                       | 0.0s(h) ~ 6553.5s(h)                                                                                                                                                                                                | 0.0s(h)   |
| PC-49     | Acceleration/deceleration time of simple PLC reference 15     | 0 ~ 3                                                                                                                                                                                                               | 0         |
| PC-50     | Time unit of simple PLC running                               | 0: second 1: hour                                                                                                                                                                                                   | 0         |
| PC-51     | Reference 0 source                                            | 0: Set by PC-00<br>1: AI1<br>2: Panel potentiometer<br>3: External panel potentiometer<br>4: HDI pulse setting<br>5: PID<br>6: Set by preset frequency (PD-08), modified via UP/DOWN | 0         |





### Columns

- parameter
- description
- if provided, parameter group
- range
- default

### PD Communication Parameters on Page 52

| parameter | description | parameter group            | range                                                                                                                                                                                                                             | default   |
| :-------- | :---------- | :------------------------- | :-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | :-------- |
| PD-00     | Baud rate   | PD Communication Parameters | Unit's digit: MODBUS<br>0: 300BPS<br>1: 600BPS<br>2: 1200BPS<br>3: 2400BPS<br>4: 4800BPS<br>5: 9600BPS<br>6: 19200BPS<br>7: 38400BPS<br>8: 57600BPS<br>9: 115200BPS<br>Ten's digit: PROFIBUS-DP<br>0: 115200 BPs<br>1: 208300 BPs<br>2: 256000 BPs<br>3: 512000 Bps<br>Hundred's digit (reserved)<br>Thousand's digit: CANlink<br>0: 20<br>1: 50<br>2: 100<br>3: 125<br>4: 250<br>5: 500<br>6: 1M | 6005      |





### PD Communication Parameters (Page 53)

| parameter                                                | description                                                                                                                                                                          | parameter group             | range                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             | default   |
| :------------------------------------------------------- | :----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | :-------------------------- | :---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | :-------- |
| PD-01: MODBUS data format                                | Selects the data format for MODBUS communication. 0: No check, data format <8,N,2> 1: Even parity check, data format <8,E,1> 2: Odd Parity check, data format <8,O,1> 3: No check, data format <8,N,1> Valid for Modbus | PD Communication Parameters | 0: No check, data format <8,N,2><br>1: Even parity check, data format <8,E,1><br>2: Odd Parity check, data format <8,O,1><br>3: No check, data format <8,N,1>                                                                                                                                                                                                                                                                                                                                                                                                                                                                | 0         |
| PD-02: Local address                                     | Sets the local address for the device. 0: Broadcast address 1 ~ 247: Specific address                                                                                                | PD Communication Parameters | 0: Broadcast address<br>1 ~ 247                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   | 1         |
| PD-03: MODBUS response delay                             | Sets the delay before the device responds to a MODBUS request.                                                                                                                       | PD Communication Parameters | 0 ~ 20ms                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          | 2         |
| PD-04: Communication timeout                             | Sets the communication timeout duration. 0.0: invalid 0.1 ~ 60.0s                                                                                                                    | PD Communication Parameters | 0.0: invalid<br>0.1 ~ 60.0s                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       | 0.0       |
| PD-05: Modbus protocol selection and PROFIBUS-DP data format | Selects the communication protocol and data format. Unit's digit: Modbus protocol (0: Non-standard, 1: Standard) Ten's digit: PROFIBUS-DP data format (0: PPO1, 1: PPO2, 2: PPO3, 3: PPO5) | PD Communication Parameters | Unit's digit: Modbus protocol<br> 0: Non-standard Modbus protocol<br> 1: Standard Modbus protocol<br>Ten's digit: PROFIBUS-DP data format<br> 0: PPO1 format<br> 1: PPO2 format<br> 2: PPO3 format<br> 3: PPO5 format                                                                                                                                                                                                                                                                                                                                                                                                     | 30        |
| PD-06: Communication reading current resolution          | Sets the resolution for reading current values via communication. 0: 0.01A 1: 0.1A                                                                                                     | PD Communication Parameters | 0: 0.01A<br>1: 0.1A                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       | 0         |





### PP Function Code Management : Page 54

### Columns

- parameter
- description
- if provided, parameter group
- range
- default

| parameter | description                                                                                                                                                                                          | parameter group                | range      | default |
| :-------- | :--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | :----------------------------- | :--------- | :------ |
| PP-00     | User password                                                                                                                                                                                        |                                | 0~65535    | 0       |
| PP-01     | Restore default settings <br> 0: No operation <br> 01: Restore factory settings except motor parameters <br> 02: Clear records                                                                          |                                | 0, 1, 2    | 0       |
| PP-02     | AC drive parameter display property <br> Unit's digit (Group U display selection): 0: Not display, 1: Display <br> Ten's digit (Group A display selection): 0: Not display, 1: Display                | Group U, Group A               | (see desc) | 11      |
| PP-03     | Individualized parameter display property <br> Unit's digit (User-defined parameter display selection): 0: Not display, 1: Display <br> Ten's digit (User-modified parameter display selection): 0: Not display, 1: Display | User-defined, User-modified | (see desc) | 00      |
| PP-04     | Parameter modification property <br> 0: Modifiable <br> 1: Not modifiable                                                                                                                             |                                | 0, 1       | 0       |





### AD Torque Control Parameters - Page 55

| parameter | description                              | parameter group   | range                                                                                                                                                                                                                            | default   |
| :-------- | :--------------------------------------- | :---------------- | :------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | :-------- |
| A0-00     | Speed/Torque control selection           |                   | 0: Speed control <br> 1: Torque control                                                                                                                                                                                       | 0         |
| A0-01     | Torque setting source in torque control  |                   | 0: Digital setting (A0-03) <br> 1: AI1 <br> 2: AI2 <br> 3: Panel potentiometer <br> 4: HDI pulse setting (DI5) <br> 5: Communication setting <br> 6: MIN(AI1, AI2) <br> 7: MAX(AI1, AI2) <br> Full range of values 1-7 corresponds to the digital setting of A0-03. | 0         |
| A0-03     | Torque digital setting in torque control |                   | -200.0% ~ 200.0%                                                                                                                                                                                                                 | 150.0%    |
| A0-05     | Forward maximum frequency in torque control |                | 0.00Hz ~ maximum frequency                                                                                                                                                                                                     | 50.00Hz   |
| A0-06     | Reverse maximum frequency in torque control |                | 0.00Hz ~ maximum frequency                                                                                                                                                                                                     | 50.00Hz   |
| A0-07     | Acceleration time in torque control    |                   | 0.00s ~ 65000s                                                                                                                                                                                                                 | 0.00s     |
| A0-08     | Deceleration time in torque control    |                   | 0.00s ~ 65000s                                                                                                                                                                                                                 | 0.00s     |





### AS Control Optimization Parameters (Page 56)

| parameter                                   | description                             | parameter group             | range                                                                 | default         |
| :------------------------------------------ | :-------------------------------------- | :-------------------------- | :-------------------------------------------------------------------- | :-------------- |
| AS-00 DPWM switchover frequency upper limit | DPWM switchover frequency upper limit | AS Control Optimization | 5.00Hz ~ maximum frequency                                            | 8.00Hz          |
| AS-01 PWM modulation mode                   | PWM modulation mode                   | AS Control Optimization | 0: Asynchronous modulation <br> 1: Synchronous modulation               | 0               |
| AS-02 Dead zone compensation mode selection | Dead zone compensation mode selection | AS Control Optimization | 0: No compensation <br> 1: Compensation mode 1                          | 1               |
| AS-03 Random PWM depth                      | Random PWM depth                      | AS Control Optimization | 0: Random PWM invalid <br> 1 ~ 10: PWM carrier frequency random depth | 0               |
| AS-04 Rapid current limit                   | Rapid current limit                   | AS Control Optimization | 0: Disabled <br> 1: Enabled                                             | 1               |
| AS-05 Current detection compensation        | Current detection compensation        | AS Control Optimization | 0 ~ 100                                                               | 5               |
| AS-06 Undervoltage threshold                | Undervoltage threshold                | AS Control Optimization | 210 ~ 420V                                                            | 350V            |
| AS-07 SVC optimization mode selection       | SVC optimization mode selection       | AS Control Optimization | 1: Optimization mode 1 <br> 2: Optimization mode 2                     | 1               |
| AS-08 Dead-zone time adjustment             | Dead-zone time adjustment             | AS Control Optimization | 100% ~ 200%                                                           | 150%            |
| AS-09 Overvoltage threshold                 | Overvoltage threshold                 | AS Control Optimization | 200.0V ~ 2500.0V                                                      | Model dependant |




### U0 Monitoring Parameters - Page 57

| parameter | description                                     | parameter group         | range   | default          |
| :-------- | :---------------------------------------------- | :---------------------- | :------ | :--------------- |
| U0-00     | Running frequency (Hz)                          | U0 Monitoring Parameters |         | 0.01Hz           |
| U0-01     | Set frequency (Hz)                              | U0 Monitoring Parameters |         | 0.01Hz           |
| U0-02     | Bus voltage (V)                                 | U0 Monitoring Parameters |         | 0.1V             |
| U0-03     | Output voltage (V)                              | U0 Monitoring Parameters |         | 1V               |
| U0-04     | Output current (A)                              | U0 Monitoring Parameters |         | 0.01A            |
| U0-05     | Output power (kW)                               | U0 Monitoring Parameters |         | 0.1kW            |
| U0-06     | Output torque (%)                               | U0 Monitoring Parameters |         | 0.1%             |
| U0-07     | DI input state                                  | U0 Monitoring Parameters |         | 1                |
| U0-08     | DO output state                                 | U0 Monitoring Parameters |         | 1                |
| U0-09     | AI1 voltage (V)                                 | U0 Monitoring Parameters |         | 0.01V            |
| U0-10     | AI2 voltage (V)/current (mA)                    | U0 Monitoring Parameters |         | 0.01V/0.01mA     |
| U0-11     | Panel potentiometer voltage (V)                 | U0 Monitoring Parameters |         | 0.01V            |
| U0-12     | Count value                                     | U0 Monitoring Parameters |         | 1                |
| U0-13     | Length value                                    | U0 Monitoring Parameters |         | 1                |
| U0-14     | Load speed display                              | U0 Monitoring Parameters |         | 1                |
| U0-15     | PID setting                                     | U0 Monitoring Parameters |         | 1                |
| U0-16     | PID feedback                                    | U0 Monitoring Parameters |         | 1                |
| U0-17     | PLC stage                                       | U0 Monitoring Parameters |         | 1                |
| U0-18     | HDI input pulse frequency (Hz)                  | U0 Monitoring Parameters |         | 0.01kHz          |
| U0-19     | Feedback speed (Hz)                             | U0 Monitoring Parameters |         | 0.01Hz           |
| U0-20     | Remaining running time                          | U0 Monitoring Parameters |         | 0.1Min           |
| U0-21     | AI1 voltage before correction                   | U0 Monitoring Parameters |         | 0.001V           |
| U0-22     | AI2 voltage (V)/current (mA) before correction | U0 Monitoring Parameters |         | 0.001V/0.01mA    |
| U0-23     | Panel potentiometer voltage before correction   | U0 Monitoring Parameters |         | 0.001V           |





### U0 Monitoring Parameters (Page 58)

| parameter   | description                                        | parameter group         | range   | default   |
| :---------- | :------------------------------------------------- | :---------------------- | :------ | :-------- |
| U0-24       | Linear speed                                       | U0 Monitoring Parameters |         | 1m/Min    |
| U0-25       | Accumulative power-on time                         | U0 Monitoring Parameters |         | 1Min      |
| U0-26       | Accumulative running time                          | U0 Monitoring Parameters |         | 0.1Min    |
| U0-27       | HDI pulse input frequency                          | U0 Monitoring Parameters |         | 1Hz       |
| U0-28       | Communication setting value                        | U0 Monitoring Parameters |         | 0.01%     |
| U0-30       | Main frequency X                                   | U0 Monitoring Parameters |         | 0.01Hz    |
| U0-31       | Auxiliary frequency Y                              | U0 Monitoring Parameters |         | 0.01Hz    |
| U0-32       | Viewing any register address value                 | U0 Monitoring Parameters |         | 1         |
| U0-35       | Target torque (%)                                  | U0 Monitoring Parameters |         | 0.1%      |
| U0-36       | Rotation position                                  | U0 Monitoring Parameters |         | 1         |
| U0-37       | Power factor angle                                 | U0 Monitoring Parameters |         | 0.1°      |
| U0-39       | Target voltage upon V/F separation                 | U0 Monitoring Parameters |         | 1V        |
| U0-40       | Output voltage upon V/F separation                 | U0 Monitoring Parameters |         | 1V        |
| U0-41       | DI state visual display                            | U0 Monitoring Parameters |         | 1         |
| U0-42       | DO state visual display                            | U0 Monitoring Parameters |         | 1         |
| U0-43       | DI function state visual display 1 (function 01-40) | U0 Monitoring Parameters |         | 1         |
| U0-44       | DI function state visual display 2 (function 41-80) | U0 Monitoring Parameters |         | 1         |
| U0-45       | Fault information                                  | U0 Monitoring Parameters |         | 1         |
| U0-59       | Current set frequency(%)                           | U0 Monitoring Parameters |         | 0.01%     |
| U0-60       | Current running frequency(%)                       | U0 Monitoring Parameters |         | 0.01%     |
| U0-61       | AC drive running state                             | U0 Monitoring Parameters |         | 1         |
| U0-62       | Current fault code                                 | U0 Monitoring Parameters |         | 1         |
| U0-65       | Torque upper limit                                 | U0 Monitoring Parameters |         | 0.1%      |




Skipping Number Page : Nothing found

## Error Codes

### Troubleshooting List - Page 59

| code   | description           | parameter group   |
| :----- | :-------------------- | :---------------- |
| Err01  | Inverter Unit Protection |                   |






### Columns

- parameter
- description
- if provided, parameter group
- range
- default


### Page 60 - No Parameters Found


## Error Codes


### Columns

- code
- description
- if provided, parameter group


### Troubleshooting List - Page 60

| code  | description                       |
| :---- | :-------------------------------- |
| Err02 | Overcurrent during acceleration   |
| Err03 | Overcurrent during deceleration |
| Err04 | Overcurrent at constant speed   |


Skipping Number Page : Nothing found

## Error Codes

### Troubleshooting List Page 61

| code | description | if provided, parameter group |
|---|---|---|
| Err05 | Overvoltage during acceleration |  |
| Err06 | Overvoltage during deceleration |  |
| Err07 | Overvoltage at constant speed |  |
| Err08 | Control power supply fault |  |
| Err09 | Undervoltage |  |



## Error Codes

### Troubleshooting List - Page 62

### Columns

- code
- description

| code   | description                                                                                                                                                              |
| :----- | :----------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Err10  | **AC drive overload**: <br> 1: The load is too heavy or lockedrotor occurs on the motor.<br> 2: The AC drive model is of too small power class                              |
| Err11  | **Motor overload**: <br> 1: P9-01 is set improperly.<br> 2: The load is too heavy or lockedrotor occurs on the motor.<br> 3: The AC drive model is of too small power class |
| Err12  | **Power input phase loss**: <br> 1: The three-phase power input is abnormal.<br> 2: The drive board is faulty.<br> 3: The lightening board is faulty.<br> 4: The main control board is faulty. |
| Err13  | **Power output phase loss**: <br> 1: The cable connecting the AC drive and the motor is faulty.<br> 2: The AC drive's three-phase outputs are unbalanced when the motor is running.<br> 3: The drive board is faulty.<br> 4: The IGBT is faulty. |
| Err14  | **IGBT overheat**: <br> 1: The ambient temperature is too high.<br> 2: The air filter is blocked.<br> 3: The fan is damaged.<br> 4: The thermally sensitive resistor of the IGBT is damaged.<br> 5: The AC drive IGBT is damaged. |
| Err15  | **External equipment fault**: <br> 1: External fault signal is input via DI.<br> 2: External fault signal is input via virtual I/O.                                           |


Skipping Number Page : Nothing found

## Error Codes

### Troubleshooting List Page 63

| code | description |
|---|---|
| Err16 | Communication fault |
| Err17 | Contactor fault |
| Err18 | Current detection fault |
| Err19 | Motor auto-tuning fault |
| Err21 | EEPROM readwrite fault |
| Err22 | AC drive hardware fault |
| Err23 | Short circuit to ground |
| Err26 | Accumulative running time reached |
| Err27 | User-defined fault 1 |


## Error Codes

### Troubleshooting List Page 64

| code  | description                           |
| :---- | :------------------------------------ |
| Err28 | User-defined fault 2                  |
| Err29 | Accumulative power-on time reached  |
| Err30 | Load becoming 0                     |
| Err31 | PID feedback lost during running      |
| Err40 | Pulse-by-pulse current limit fault    |
| Err41 | Motor switchover fault during running |
| Err45 | Motor overheat                        |
| Err51 | Initial position fault                |


Skipping Number Page : Nothing found


Skipping Number Page : Nothing found

## Error Codes

### Troubleshooting page 66

| code                      | description                                                | parameter group   |
| :------------------------ | :--------------------------------------------------------- | :---------------- |
| Err14 (IGBT overheat)   | fault is reported frequently                               |                   |
| Motor does not rotate   | The motor does not rotate after the AC drive runs            |                   |
| Overcurrent/Overvoltage | The AC drive reports overcurrent and overvoltage frequently |                   |
| No display                | No display upon power-on                                   |                   |
