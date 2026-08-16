#ifndef OMRON_MX2_H
#define OMRON_MX2_H

// Omron MX2 Registers
#define MX2_STATE 0x0003        // (2 bytes) Status of the inverter
#define MX2_STATUS 0x004        // (2 bytes) Status of the inverter
#define MX2_TARGET_FR 0x0001    // (4 bytes) Source (reference) of the frequency reference (0,01 [Hz])
#define MX2_ACCEL_TIME 0x1103   // (4 bytes) Acceleration time (cm compressor) in 0.01 sec
#define MX2_DEACCEL_TIME 0x1105 // (4 bytes) Braking time (cm compressor) in 0.01 sec

#define MX2_CURRENT_FR 0x1001 // (4 bytes) Output frequency control (0,01 [Hz])
#define MX2_AMPERAGE 0x1003   // (2 bytes) Output current monitoring (0,01 [A])
#define MX2_VOLTAGE 0x1011    // (2 bytes) Monitoring output voltage 0.1 [V]
#define MX2_POWER 0x1012      // (2 bytes) Power control 0.1 [kW]
#define MX2_POWER_HOUR 0x1013 // (4 bytes) Watt-hour control 0.1 [kW / h]
#define MX2_HOUR 0x1015       // (4 bytes) Control of operating time in the "Run" mode 1 [h]
#define MX2_HOUR1 0x1017      // (4 bytes) Monitoring of running hours with power on 1 [h]
#define MX2_TEMP 0x1019       // (2 bytes) Radiator temperature control (0.1 degree) -200 ... 1500
#define MX2_VOLTAGE_DC 0x1026 // (2 bytes) DC voltage control (PN) 0.1 [V]
#define MX2_NUM_ERR 0x0011    // (2 bytes) Trip counter 0 ... 65530
#define MX2_ERROR1 0x0012     // (20 bytes) Description 1 trip the remaining 5 lie sequentially behind the first address error are calculated MX2_ERROR1 + i * 0x0a
#define MX2_INIT_DEF 0x1357   // (2 bytes) Set the initialization mode to 0 (nothing), 1 (clearing the shutdown history), 2 (clearing the shutdown history and initializing data), 4 (clearing the shutdown history, initializing data and the program EzSQ)
#define MX2_INIT_RUN 0x13b7   // (2 bytes) Initialization start 0 (off), 1 (on)

#define MX2_SOURCE_FR 0x1201  // (2 bytes) Frequency reference source
#define MX2_SOURCE_CMD 0x1202 // (2 bytes) Command source
#define MX2_BASE_FR 0x1203    // (2 bytes) Main frequency 300 ... "maximum frequency" 0.1 Hz
#define MX2_MAX_FR 0x1204     // (2 bytes) Maximum frequency 300 ... 4000 (10000) 0.1 Hz
#define MX2_DC_BRAKING 0x1245 // (2 bytes) Enable DC Braking
#define MX2_STOP_MODE 0x134e  // (2 bytes) Choosing a stop method B091 = 01
#define MX2_MODE 0x13ae       // (2 bytes) IF mode selection b171 = 03

// Setting the inverter for a specific compressor Registers Hxxx Permanent magnet motor (PM motor)
#define MX2_b171 0x13ae // b171 Inverter selection b171 read / write 0 (off), 1 (IM mode), 2 (high frequency mode), 3 (PM mode) = 03
#define MX2_b180 0x13b7 // b180 Initialization trigger = 01
#define MX2_H102 0x1571 // H102 Setting the PM engine code 00 (standard Omron data) 01 (auto-tuning data) = 1
#define MX2_H103 0x1572 // H103 PM engine power (0.1 / 0.2 / 0.4 / 0.55 / 0.75 / 1.1 / 1.5 / 2.2 / 3.0 / 3, 7 / 4.0 / 5.5 / 7.5 / 11.0 / 15.0 / 18.5) = 7
#define MX2_H104 0x1573 // H104 Setting the number of poles of the PM motor = 4
#define MX2_H105 0x1574 // H105 Rated current of the PM motor = 1000 (this is 11A)
#define MX2_H106 0x1575 // H106 PM motor constant R From 0.001 to 65.535 Ohms = 0.55
#define MX2_H107 0x1576 // H107 PM Engine Ld Constant From 0.01 to 655.35 mH = 2.31
#define MX2_H108 0x1577 // H108 Lq constant of PM engine From 0.01 to 655.35 mH = 2.7
#define MX2_H109 0x1578 // H109 Ke Engine Constant PM-motor 0.0001 ... 6.5535 Vmax ./ (rad / s) = 750 must be selected it affects consumption and noise
#define MX2_H110 0x1579 // (4 bytes) H110 PM motor constant J From 0.001 to 9999,000 kg / m² = 0.01
#define MX2_H111 0x157B // H111 Auto tuning constant R From 0.001 to 65.535 Ohms
#define MX2_H112 0x157C // H112 Auto-tuning constant Ld From 0.01 to 655.35 mH
#define MX2_H113 0x157D // H113 Auto tuning constant Lq From 0.01 to 655.35 mH
#define MX2_H116 0x1581 // H116 The response of the PM motor at a speed of 1 ... 1000 = 100 (default)
#define MX2_H117 0x1582 // H117 Starting current of the PM motor From 20.00 to 100.00% = 70 (default)
#define MX2_H118 0x1583 // H118 Starting time of the PM motor 0.01 ... 60.00 s = 1 (default)
#define MX2_H119 0x1584 // H119 Engine PM stabilization constant From 0 to 120% s = 100
#define MX2_H121 0x1586 // H121 Minimum frequency of the PM motor From 0.0 to 25.5% = 0
#define MX2_H122 0x1587 // H122 Idling current PM motor From 0.00 to 100.00% = 50 (default)
#define MX2_H123 0x1588 // H123 Choice of PM engine start method 00 (off) 01 (on) = 0 (default)
#define MX2_H131 0x158A // H131 Estimation of the initial position of the rotor of the PM motor: standby time 0 V 0 ... 255 = 10 (default)
#define MX2_H132 0x158B // H132 Assessment of the initial position of the rotor of the PM motor: waiting time for determination 0 ... 255 = 10 (default)
#define MX2_H133 0x158C // H133 Assessment of the initial position of the rotor of the PM motor: determination time 0 ... 255 = 30 (default)
#define MX2_H134 0x158D // H134 Assessment of the initial position of the rotor of the PM motor: voltage gain 0 ... 200 = 100 (default)
#define MX2_C001 0x1401 // C001 Input function [1] 0 (FW: go forward) = 0
#define MX2_C004 0x1404 // C004 Input function [4] 18 (RS: reset) = 18
#define MX2_C005 0x1405 // C005 Input function [5] [also input “PTC”] = 19 PTC Thermistor with positive TCS for thermal protection (only C005)
#define MX2_C026 0x1404 // C026 Relay output function 5 (AL: error signal) = 05
#define MX2_b091 0x135E // b091 Choice of stopping method 0 (braking to a complete stop), 1 (coasting stop) = 1
#define MX2_b021 0x1316 // b021 Operating mode with overload limitation 0 (off), 1 (enabled during acceleration and rotation at a constant speed),          \
                        //       2 (enabled during rotation at a constant speed), 3 (enabled during acceleration and rotation at a constant speed [increase \
                        //       speed in generator mode]) = 1
#define MX2_b022 0x1317 // b022 Overload restriction level 200 ... 2000 (0.1%) =
#define MX2_b023 0x1318 // b023 Braking time with overload limitation (0.1 sec) = 10
#define MX2_F002 0x1103 // (4 bytes) F002 Acceleration time (1) Standard, default acceleration, range from 0.001 to 3600 s (0.01 sec) = 20 * 100
#define MX2_F003 0x1105 // (4 bytes) F003 Deceleration time (1) Standard, default acceleration, range from 0.001 to 3600 s (0.01 sec) = 20 * 100
#define MX2_A001 0x1201 // A001 Frequency reference source 00 ... Potent. on external panels 01 ... Control terminals 02 ... Setting parameter F001 \
                        //       03 ... Input via ModBus network 04 ... Add. card 06 ... Entrance imp. after 07 ... via EzSQ 10 ... Result of arithmetic operation = 03
#define MX2_A002 0x1202 // A002 Source of the “Run” command 01 .. Control terminals 02 ... “Run” key on the keypad or digital panel 03 ... Input via the ModBus network 04 ... Add. card = 01
#define MX2_A003 0x1203 // A003 Main frequency Can be set in the range from 30 Hz to the maximum frequency (A004) (0.1 Hz) = 120 * 10
#define MX2_A004 0x1204 // A004 Maximum frequency Can be set in the range from the fundamental frequency to 400 Hz (0.1 Hz) = 120 * 10

// Omron MX2 Bits
#define MX2_START 0x0000     // (bit) Run command 1: Run, 0: Stop (valid with A002 = 03)
#define MX2_SET_DIR 0x0001   // (bit) Command of direction of rotation 1: Reverse rotation, 0: Rotation in the forward direction (valid with A002 = 03)
#define MX2_RESET 0x0004     // (bit) Reset emergency shutdown (RS) 1: Reset
#define MX2_READY 0x0011     // (bit) Ready IF 1: Ready, 0: Not ready
#define MX2_DIRECTION 0x0010 // (bit) Direction of rotation 1: Reverse rotation, 0: Rotation in the forward direction (deadlock with "d003")

#define TEST_NUMBER 1234 // Verification code for function 0x08

#endif