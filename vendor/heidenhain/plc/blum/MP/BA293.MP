             ;Machine Parameters iTNC 530 MC 6xxx
             ;CC 422/CC 424(B)/CC 61xx
             ;Id Nr: 606 420-04, 606 422-04, 606 424-04, 606 425-04
             ;Date of information: 2014-04-02
             ;
             ;==============================================================
             ;Code numbers:
             ;==============================================================
             ; Edit machine parameters  ...................... 95148
             ; Short form of machine parameters .............. 123
             ; Edit/Test PLC program ......................... 807667
             ; Offset compensation ........................... 75368
             ; Unprotect PGM ................................. 86357
             ; Digital scope 4 channels ...................... 688379
             ; Erase RAM, total (M0..M999, B0..B127) ......... 531210
             ; Write system data FN17,31,32................... 555343
             ; Reset PLC counter ............................. 857282
             ; Monitor network ............................... NETMONI
             ; Set up network ................................ NET123
             ; DSP monitor ................................... DSP123
             ; Scope before first CE ......................... DSP123
             ; Check log book (LSV-2 off/on) .. .............. LOGBOOK
             ; Check log book from RAM (LSV-2 off/om) ........ LOGBOOK1
             ; Select machine kinematic ...................... KINEMATIC
             ; Test emergency stop machine hardware .......... FAILTEST
             ; Execute NC Software Update .................... SETUP
             ; Systeminformations TNC:\version.a ............. VERSION
             ; Display active options ........................ SIK
             ; Set Genaral Key ............................... 65535
             ;
             ;==============================================================
             ; Special keys and functions
             ;==============================================================
             ; Entering special characters via the TNC keyboard:
             ; <Form Feed>   = <Shift> + <Ret>
             ; <|>           = <Shift> + <!>
             ; <~>           = <Shift> + <^>
             ; <Esc>         = <Shift> + <#>
             ;
             ; Switch between CAPITAL and lowercase:
             ; <Shift> + <Space>
             ;
             ;==============================================================
             ;Contents
             ;==============================================================
             ;MP        0 ..  999   Measuring systems and machine axes
             ;MP     1000 .. 1399   Positioning / Controlling
             ;MP     1400 .. 1699   Operation with velocity feedforward control
             ;MP     1700 .. 1899   Operation with following error (lag)
             ;MP     2000 .. 2999   Parameters for the PWM interface (DSP)
             ;MP     3000 .. 3999   Spindle control (1st spindle)
             ;MP     4000 .. 4999   Integrated PLC
             ;MP     5000 .. 5999   Serial interfaces
             ;MP     6000 .. 6199   Touch probe
             ;MP     6500 .. 6599   Tool touch probe for tool measurement
             ;MP     7100 .. 7199   Tapping
             ;MP     7200 .. 7399   Programming and display
             ;MP     7400 .. 7599   Machining and program run
             ;MP     7600 .. 7699   Hardware
             ;MP    13000 ..13999   Spindle control (2nd spindle)
             ;
             ;==============================================================
             ;Input format
             ;==============================================================
             ;Leading character % = binary input
             ;    e.g.:  MP 10 : % 000111111
             ;                 :   987654321 axis
             ;Leading character $ = hexadecimal input
             ;    e.g.:  MP 4310.0  : $800F
             ;                         1000 0000 0000 1111
             ;                        -32753
             ;==============================================================
             ;MP 10 .. 999
             ;Measuring systems and machine axes
             ;==============================================================
             ;MP10 Active axes
             ;Input: %987654321 bit-encoded
             ;.............987654ZYX <- axis
             ;221111111111----------
             ;1098765432109876543210 <- bit #
MP 10      : %0000000000000000000111
             ;--------------------------------------------------------------
             ;MP12 Axes in demo mode
             ;Input: %987654321 bit-encoded
             ;       0 = Demo mode inactive
             ;       1 = Demo mode active
             ;.............987654ZYX <- axis
             ;221111111111----------
             ;1098765432109876543210 <- bit #
MP 12      : %0000000000000000000000
             ;--------------------------------------------------------------
             ;MP20 Check the measuring system signals for the axes
             ;Input: %987654321 bit-encoded
             ;.............987654ZYX <- axis
             ;221111111111----------
             ;1098765432109876543210 <- bit #
MP 20.0    : %0000000000000000000111;Absolute position of distance-coded
             ;                       reference marks
             ;                   000 <- without scale (MP20.0)
             ;                   111 <- with scale (MP20.0)
MP 20.1    : %1111111111111111111111;Amplitude
MP 20.2    : %1111111111111111111111;Edge separation
             ;--------------------------------------------------------------
             ;MP21 Check the measuring system signals for the spindle
             ;Input: %yx bit-encoded
             ;       x = 1st spindle
             ;       y = 2nd spindle
MP 21.0    : %00        ;Absolute position of distance-coded reference marks
MP 21.1    : %11        ;Amplitude
MP 21.2    : %11        ;Edge separation
             ;--------------------------------------------------------------
             ;MP100 Assignment of axis designation
             ;Input: WvUcBaZyX
             ; X = NC axes (capital letters)
             ; x = PLC axes (lowercase letters)
             ; - = Non-displayed axes
             ;11111111---------
             ;76543210987654321 <- axis
MP 100.0   : -------------CAZYX;Traverse range 1
MP 100.1   : -------------CAZYX;Traverse range 2
MP 100.2   : -------------CAZYX;Traverse range 3
             ;--------------------------------------------------------------
             ;MP108 Assignment of axis to controller basic-board
             ;Input: 0..4 = Number of controller basic-board in HSCI system
             ;              Only order of controller basic-boards in the
             ;              HSCI system affect on address 0..4
             ;              1. controller basic-board gets adr. 0
MP 108.0   : 0
MP 108.1   : 0
MP 108.2   : 0
MP 108.3   : 0
MP 108.4   : 0
MP 108.5   : 0
MP 108.6   : 0
MP 108.7   : 0
MP 108.8   : 0
MP 108.9   : 0
MP 108.10  : 0
MP 108.11  : 0
MP 108.12  : 0
MP 108.13  : 0
             ;--------------------------------------------------------------
             ;MP109 Assignment of spindles to controller basic-board
             ;Input: 0..4 = Number of controller basic-board in HSCI system
             ;              Only order of controller basic-boards in the
             ;              HSCI system affect on address 0..4
             ;              1. controller basic-board gets adr. 0
MP 109.0   : 0
MP 109.1   : 0
             ;--------------------------------------------------------------
             ;MP110 Assign position encoder inputs to axes
             ;Input:   0        = No position encoder input
             ;         1..6     = Position encoder input X1..X6
             ;         35..38   = Position encoder input X35..X38
             ;         201..214 = Position encoder input X201..X214
MP 110.0   : 203        ;w/ scale=203, w/o scale=0
MP 110.1   : 202        ;w/ scale=204, w/o scale=0
MP 110.2   : 204        ;w/ scale=205, w/o scale=0
MP 110.3   : 0
MP 110.4   : 0
MP 110.5   : 0
MP 110.6   : 0
MP 110.7   : 0
MP 110.8   : 0
MP 110.9   : 0
MP 110.10  : 0
MP 110.11  : 0
MP 110.12  : 0
MP 110.13  : 0
             ;--------------------------------------------------------------
             ;MP111 Assign position encoder inputs for spindles
             ;Input:   0        = No position encoder input
             ;         1..6     = Position encoder input X1..X6
             ;         35..38   = Position encoder input X35..X38
             ;         201..214 = Position encoder input X201..X214
MP 111.0   : 0          ;1st spindle
MP 111.1   : 0          ;2nd spindle
             ;--------------------------------------------------------------
             ;MP113 Assign speed encoder inputs for spindles
             ;Input:   0      = No speed encoder
             ;         15..20 = Speed encoder input X15..X20
             ;         80..85 = Speed encoder input X80..X85
MP 113.0   : 20         ;1st spindle
MP 113.1   : 0          ;2nd spindle
             ;--------------------------------------------------------------
             ;MP115.0 Definition of position encoder inputs 1 Vpp or 11 uA
             ;Input: %X38..X35 X6..X1 bit-encoded
             ;       0 = 1 Vpp
             ;       1 = 11 uA
             ;-XXXX
             ;-3333XXXXXX
             ;-8765654321 <- socket #
MP 115.0   : %00000000000
             ;MP115.1 Reserved
             ;-XXXX
             ;-3333XXXXXX
             ;-8765654321
MP 115.1   : %00000000000
             ;MP115.2 Low or high input frequency of position encoder inputs
             ;Input: For 1 Vpp: 0 = 27 kHz
             ;                  1 = 400 kHz
             ;       For 11 uA: 0 = 27 kHz
             ;                  1 = 140 kHz
             ;-XXXX
             ;-3333XXXXXX
             ;-8765654321 <- socket #
MP 115.2   : %00000000000
             ;--------------------------------------------------------------
             ;MP118 Definition of position encoder input for axis
             ;      (only CC 6xxx)
             ;Input: %xxxx
             ;Bit0     Position encoder input 1 Vpp
             ;         0 = 1 Vpp
             ;         1 = reserved
             ;Bit1     Reserved
             ;Bit2     Input frequency of position encoder input
             ;         For 1 Vpp: 0 = 27 kHz
             ;                    1 = 400 kHz
             ;Bit3     Position encoder input analog or digital
             ;         0 = analog encoder signal (1 Vpp)
             ;         1 = digital encoder signal (EnDat 2.2)
             ;Bit4     Only on controls with Functional Safety FS:
             ;         Safety functions of SKERN only via
             ;         position encoder (EnDat 2.2 FS)
             ;         0 = SKERN uses speed and position encoder
             ;         1 = SKERN uses only safe position encoder
             ;Bit5     Only on controls with Functional Safety FS:
             ;         Safety functions of SKERN only via
             ;         speed encoder (EnDat 2.2 FS)
             ;         0 = SKERN uses speed and position encoder
             ;         1 = SKERN uses only safe speed encoder
             ;6543210 <- bit #
MP 118.0   : %0000000   ;w/ LC115=%001000, w/o LC115=%000000
MP 118.1   : %0000000   ;w/ LC115=%001000, w/o LC115=%000000
MP 118.2   : %0000000   ;w/ LC115=%001000, w/o LC115=%000000
MP 118.3   : %0000000
MP 118.4   : %0000000
MP 118.5   : %0000000
MP 118.6   : %0000000
MP 118.7   : %0000000
MP 118.8   : %0000000
MP 118.9   : %0000000
MP 118.10  : %0000000
MP 118.11  : %0000000
MP 118.12  : %0000000
MP 118.13  : %0000000
             ;--------------------------------------------------------------
             ;MP119 Definition of position encoder input for spindles
             ;        (only CC 6xxx)
             ;Input: %xxxx
             ;Bit0     Position encoder input 1 Vpp
             ;         0 = 1 Vpp
             ;         1 = reserved
             ;Bit1     Reserved
             ;Bit2     Input frequency of position encoder input
             ;         For 1 Vpp: 0 = 27 kHz
             ;                    1 = 400 kHz
             ;Bit3     Position encoder input analog or digital
             ;         0 = analog encoder signal (1 Vpp)
             ;         1 = digital encoder signal (EnDat 2.2)
             ;Bit4     Only on controls with Functional Safety FS:
             ;         Safety functions of SKERN only via
             ;         position encoder (EnDat 2.2 FS)
             ;         0 = SKERN uses speed and position encoder
             ;         1 = SKERN uses only safe position encoder
             ;Bit5     Only on controls with Functional Safety FS:
             ;         Safety functions of SKERN only via
             ;         speed encoder (EnDat 2.2 FS)
             ;         0 = SKERN uses speed and position encoder
             ;         1 = SKERN uses only safe speed encoder
             ;6543210 <- bit #
MP 119.0   : %0000000
MP 119.1   : %0000000
             ;--------------------------------------------------------------
             ;MP120 Assign the nominal speed value outputs to the axes
             ;Note: Assignment is permanently defined(.0 -> x1)
             ;Input: 0      = Not a servo-controlled axis
             ;       1..8   = Analog outputs X66, X67 (CMA)
             ;       51..64 = Digital outputs X51..X64
             ;                for CC with HEIDENHAIN inverters
             ;       80..85 = Digital outputs X80..X85 (for UEC)
MP 120.0   : 53
MP 120.1   : 52
MP 120.2   : 54
MP 120.3   : 0          ;51
MP 120.4   : 0
MP 120.5   : 0
MP 120.6   : 0
MP 120.7   : 0
MP 120.8   : 0
MP 120.9   : 0
MP 120.10  : 0
MP 120.11  : 0
MP 120.12  : 0
MP 120.13  : 0
             ;--------------------------------------------------------------
             ;MP121 Assign the nominal speed value outputs for spindles
             ;Input: 0      = Not a servo-controlled spindle
             ;       1..8   = Analog outputs X66, X67 (CMA)
             ;       51..64 = Digital outputs X51..X64
             ;                for CC with HEIDENHAIN inverters
             ;       80..85 = Digital outputs X80..X85 (for UEC)
MP 121.0   : 56         ;1st spindle
MP 121.1   : 0          ;2nd spindle
             ;--------------------------------------------------------------
             ;MP130 Y index of the machine parameters MP2xxx.y for the axes
             ;Input: 0 to 17
MP 130.0   : 0
MP 130.1   : 1
MP 130.2   : 2
MP 130.3   : 3
MP 130.4   : 4
MP 130.5   : 5
MP 130.6   : 6
MP 130.7   : 7
MP 130.8   : 8
MP 130.9   : 9
MP 130.10  : 10
MP 130.11  : 11
MP 130.12  : 11
MP 130.13  : 11
             ;--------------------------------------------------------------
             ;MP131 Y index of the machine parameters MP2xxx.y for the
             ;      spindle in operating mode 0
             ;Input: 0 to 17
MP 131.0   : 12         ;Spindle 1
MP 131.1   : 13         ;Spindle 2
             ;--------------------------------------------------------------
             ;MP132 Y index of the machine parameters MP2xxx.y for the
             ;      spindle in operating mode 1
             ;Input: 0 to 17
MP 132.0   : 14         ;Spindle 1
MP 132.1   : 15         ;Spindle 2
             ;--------------------------------------------------------------
             ;MP210 Counting direction of position encoder signals
             ;Input: %987654321 bit-encoded
             ;       0 = positive
             ;       1 = negative
             ;.............987654ZYX <- axis
             ;221111111111----------
             ;1098765432109876543210 <- bit #
MP 210     : %0000000000000000000000
             ;                  0100 <- without scale (MP210)
             ;                  ?000 <- with scale (MP210)
             ;--------------------------------------------------------------
             ;The signal period is automatically calculated by the TNC:
             ;Signal period = MP331 / MP332
             ;MP331 = Distance for the counting pulses from MP332
             ;Input: 0 to 99 999.9999 [mm/degrees]
MP 331.0   : 20.00000   ;w/ LS187C/LC183/GOP=20, w/o scale=16
MP 331.1   : 20.00000   ;w/ LS187C/LC183/GOP=20, w/o scale=16
MP 331.2   : 20.00000   ;w/ LS187C/LC183/GOP=20, w/o scale=12
MP 331.3   : 2.00000
MP 331.4   : 10.00000
MP 331.5   : 10.00000
MP 331.6   : 10.00000
MP 331.7   : 10.00000
MP 331.8   : 10.00000
MP 331.9   : 10.00000
MP 331.10  : 10.00000
MP 331.11  : 10.00000
MP 331.12  : 10.00000
MP 331.13  : 10.00000
             ;--------------------------------------------------------------
             ;MP332 Number of counting pulses in the distance from MP331
             ;Input: 1 to 16 777 215
MP 332.0   : 1000.00000 ;w/ LS187C/LC183/GOP=1000, w/o scale=2048
MP 332.1   : 1000.00000 ;w/ LS187C/LC183/GOP=1000, w/o scale=2048
MP 332.2   : 1000.00000 ;w/ LS187C/LC183/GOP=1000, w/o scale=2048
MP 332.3   : 2048.00000
MP 332.4   : 2048.00000
MP 332.5   : 2048.00000
MP 332.6   : 2048.00000
MP 332.7   : 2048.00000
MP 332.8   : 2048.00000
MP 332.9   : 2048.00000
MP 332.10  : 2048.00000
MP 332.11  : 2048.00000
MP 332.12  : 2048.00000
MP 332.13  : 2048.00000
             ;--------------------------------------------------------------
             ;MP334 Number of grating periods between the zero pulses
             ;of distance-coded encoders
             ;Input: 0 to 65535
             ;       0 = 1000
MP 334.0   : 0          ;LS187C/GOP=0, LB382C/LOP=2000
MP 334.1   : 0          ;LS187C/GOP=0, LB382C/LOP=2000
MP 334.2   : 0          ;LS187C/GOP=0, LB382C/LOP=2000
MP 334.3   : 0
MP 334.4   : 0
MP 334.5   : 0
MP 334.6   : 0
MP 334.7   : 0
MP 334.8   : 0
MP 334.9   : 0
MP 334.10  : 0
MP 334.11  : 0
MP 334.12  : 0
MP 334.13  : 0
             ;--------------------------------------------------------------
             ;MP340 External interpolation for square-wave signals
             ;Input: 2 to 99
             ;       0, 1 = no external interpolation
MP 340.0   : 0
MP 340.1   : 0
MP 340.2   : 0
MP 340.3   : 0
MP 340.4   : 0
MP 340.5   : 0
MP 340.6   : 0
MP 340.7   : 0
MP 340.8   : 0
MP 340.9   : 0
MP 340.10  : 0
MP 340.11  : 0
MP 340.12  : 0
MP 340.13  : 0
             ;--------------------------------------------------------------
             ;MP410 Axis identification for axis keys IV, V and VI
             ;      (VI only with HR 5xx)
             ;Input: A/B/C/U/V/W/T with SW-Option #44
MP 410.3   : A          ;IV
MP 410.4   : C          ;V
MP 410.5   :            ;VI
             ;--------------------------------------------------------------
             ;MP420 Hirth coupling
             ;Input: 0 = No Hirth coupling
             ;       1 = Hirth coupling active
MP 420.0   : 0
MP 420.1   : 0
MP 420.2   : 0
MP 420.3   : 0
MP 420.4   : 0
MP 420.5   : 0
MP 420.6   : 0
MP 420.7   : 0
MP 420.8   : 0
MP 420.9   : 0
MP 420.10  : 0
MP 420.11  : 0
MP 420.12  : 0
MP 420.13  : 0
             ;--------------------------------------------------------------
             ;MP430 Default increment for Hirth coupling
             ;Input: 0.0000 to 180.0000 [degrees]
MP 430.0   : 0
MP 430.1   : 0
MP 430.2   : 0
MP 430.3   : 0
MP 430.4   : 0
MP 430.5   : 0
MP 430.6   : 0
MP 430.7   : 0
MP 430.8   : 0
MP 430.9   : 0
MP 430.10  : 0
MP 430.11  : 0
MP 430.12  : 0
MP 430.13  : 0
             ;==============================================================
             ;Some machine parameters in the range MP500 to MP670 only have
             ;a function on MC 422 units with ID 364 176-xx and 364 178-xx.
             ;These are identified with: (only MC 422)
             ;
             ;All other machine parameters in the range MP500 to MP699
             ;only have a function if it is a system with Functional
             ;Safety (FS) on an HSCI system.
             ;--------------------------------------------------------------
             ;SMP500 Axes: Remove axes from safe operation
             ;       If an axis is defined as safe, then 0 cannot
             ;       be entered in the corresponding MP110.x and
             ;             MP 112.x parameters (only MC 422)
             ;Input: 0 = Safe axis
             ;       1 = Axis not safe
             ;.............987654ZYX <- axis
             ;221111111111----------
             ;1098765432109876543210 <- bit #
MP 500     : %0000000000000000000000
             ;--------------------------------------------------------------
             ;SMP501 Spindle: Remove the spindle from safe operation
             ;       If an axis is defined as safe, then 0 cannot
             ;       be entered in the corresponding MP111 and MP 113
             ;       parameters (only MC 422)
             ;Input: 0 = Spindle safe
             ;       1 = Spindle not safe
             ;10 <- bit #
MP 501     : %00
             ;--------------------------------------------------------------
             ;SMP511 Time until the safety self-test.
             ;       This must occur within 72 h.
             ;Input: 1 to 4320 [min]
MP 511     : 72
             ;--------------------------------------------------------------
             ;SMP520 Axes: Default time for stopping at the current limit
             ;       Stop1 for transitioning to a safe stop (ts1).
             ;       (only MC 422)
             ;Input: 0.000 to 50 000 [s]
MP 520     : 0.5
             ;--------------------------------------------------------------
             ;SMP521 Spindle: Default time for stopping at the current limit
             ;       Stop1 for transitioning to a safe stop (ts1).
             ;       (only MC 422)
             ;Input: 0.000 to 50 000 [s]
MP 521     : 2
             ;--------------------------------------------------------------
             ;SMP523 Spindle: Prescribed time for stopping axis group A
             ;       with stop3(only MC 422)
             ;Input: 0.000 to 2 000 [s]
MP 523     : 2
             ;--------------------------------------------------------------
             ;SMP524 Spindle: Prescribed time for stopping axis group S
             ;       with stop3. Upon standstill of the axes, the limit of
             ;       current is braked at. (only MC 422)
             ;Input: 0.000 to 10.000 [s]
MP 524     : 2
             ;--------------------------------------------------------------
             ;SMP525 Axes:
             ;       Default time for bringing axes to a standstill on an
             ;       axis-specific basis on the emergency braking ramp for
             ;       SS1 reaction
             ;Input: 0.000 to 10.000 [s]
MP 525.0   : 1
MP 525.1   : 1
MP 525.2   : 1
MP 525.3   : 1
MP 525.4   : 1
MP 525.5   : 1
MP 525.6   : 1
MP 525.7   : 1
MP 525.8   : 1
MP 525.9   : 1
MP 525.10  : 1
MP 525.11  : 1
MP 525.12  : 1
MP 525.13  : 1
             ;--------------------------------------------------------------
             ;SMP526 Spindles:
             ;       Default time for bringing spindles to a standstill
             ;       on an axis-specific basis on the emergency braking
             ;       ramp for SS1 reaction
             ;Input: 0.000 to 10.000 [s]
MP 526.0   : 1
MP 526.1   : 1
             ;--------------------------------------------------------------
             ;SMP527 Axes:
             ;       Default time for bringing axes to a controlled
             ;       standstill on an axis-specific basis for SS2 reaction
             ;Input: 0.000 to 10.000 [s]
MP 527.0   : 1
MP 527.1   : 1
MP 527.2   : 1
MP 527.3   : 1
MP 527.4   : 1
MP 527.5   : 1
MP 527.6   : 1
MP 527.7   : 1
MP 527.8   : 1
MP 527.9   : 1
MP 527.10  : 1
MP 527.11  : 1
MP 527.12  : 1
MP 527.13  : 1
             ;--------------------------------------------------------------
             ;SMP528 Spindles:
             ;       Default time for bringing spindles to a controlled
             ;       standstill on an axis-specific basis for SS2 reaction
             ;Input: 0.000 to 10.000 [s]
MP 528.0   : 1
MP 528.1   : 1
             ;--------------------------------------------------------------
             ;SMP529 Time window during which the permissive key must
             ;       be pressed in operating mode 4 (only MC 422)
             ;Input: 0 to 10 [min]
MP 529     : 3
             ;--------------------------------------------------------------
             ;SMP530 Delay until dv/dt monitoring becomes active
             ;Input: 0.000 to 10.000 [s]
MP 530.0   : 0.03
MP 530.1   : 0.03
MP 530.2   : 0.03
MP 530.3   : 0.03
MP 530.4   : 0.03
MP 530.5   : 0.03
MP 530.6   : 0.03
MP 530.7   : 0.03
MP 530.8   : 0.03
MP 530.9   : 0.03
MP 530.10  : 0.03
MP 530.11  : 0.03
MP 530.12  : 0.03
MP 530.13  : 0.03
             ;--------------------------------------------------------------
             ;SMP535 Run-times of the timers (max. 16) for SPLC program
             ;Input: 0.0 to 1000000.0 [s]
MP 535.0   : 0
MP 535.1   : 0
MP 535.2   : 0
MP 535.3   : 0
MP 535.4   : 0
MP 535.5   : 0
MP 535.6   : 0
MP 535.7   : 0
MP 535.8   : 0
MP 535.9   : 0
MP 535.10  : 0
MP 535.11  : 0
MP 535.12  : 0
MP 535.13  : 0
MP 535.14  : 0
MP 535.15  : 0
             ;--------------------------------------------------------------
             ;SMP540 Axes:
             ;       Limit value for "Safely limited speed - SLS" in
             ;       operating mode 3 (SOM 3)
             ;Input: 0 to 5000 [mm/min or °/min]
MP 540.0   : 2000
MP 540.1   : 2000
MP 540.2   : 2000
MP 540.3   : 2000
MP 540.4   : 2000
MP 540.5   : 2000
MP 540.6   : 2000
MP 540.7   : 2000
MP 540.8   : 2000
MP 540.9   : 2000
MP 540.10  : 2000
MP 540.11  : 2000
MP 540.12  : 2000
MP 540.13  : 2000
             ;--------------------------------------------------------------
             ;SMP541 Spindle:
             ;       Limit value for "Safely limited speed - SLS" in
             ;       operating mode 3 (SOM 3)
             ;Input: 0 to 16000 [rpm]
MP 541.0   : 500
MP 541.1   : 500
             ;--------------------------------------------------------------
             ;SMP545 Axes:
             ;       Limit value for standstill monitoring in SOS state
             ;Input: 0.001 to 30 000 [mm or °]
MP 545.0   : 0.5
MP 545.1   : 0.5
MP 545.2   : 0.5
MP 545.3   : 0.5
MP 545.4   : 0.5
MP 545.5   : 0.5
MP 545.6   : 0.5
MP 545.7   : 0.5
MP 545.8   : 0.5
MP 545.9   : 0.5
MP 545.10  : 0.5
MP 545.11  : 0.5
MP 545.12  : 0.5
MP 545.13  : 0.5
             ;--------------------------------------------------------------
             ;SMP547 reserved
MP 547.0   : 2
MP 547.1   : 2
MP 547.2   : 2
MP 547.3   : 2
MP 547.4   : 2
MP 547.5   : 2
MP 547.6   : 2
MP 547.7   : 2
MP 547.8   : 2
MP 547.9   : 2
MP 547.10  : 2
MP 547.11  : 2
MP 547.12  : 2
MP 547.13  : 2
             ;--------------------------------------------------------------
             ;SMP549 Axis-specific configuration, if axis group is in
             ;       SOS or STO after SS2 reaction
             ;Input: 0 = Spindle axis groups in STO after SS2
             ;           axis axis groups in SOS after SS2
             ;       1 = Axis group in STO after SS2
             ;       2 = Axis group in SOS after SS2
MP 549.0   : 0
MP 549.1   : 0
MP 549.2   : 0
MP 549.3   : 0
MP 549.4   : 0
MP 549.5   : 0
MP 549.6   : 0
MP 549.7   : 0
             ;--------------------------------------------------------------
             ;SMP550 Axes (only with HSCI system):
             ;       Axis-specific limit value for maximum permissible
             ;       path upon SS2 reaction
             ;Input: 0.0010 to 30.0000 [mm or °]
             ;
             ;MP550 without HSCI system:
             ;      Maximum deviation of an axis in axis group T
             ;      after release of permissive button
             ;Input: 0,0010 to 100,0000 [mm]
MP 550.0   : 0.5
MP 550.1   : 0.5
MP 550.2   : 0.5
MP 550.3   : 0.5
MP 550.4   : 0.5
MP 550.5   : 0.5
MP 550.6   : 0.5
MP 550.7   : 0.5
MP 550.8   : 0.5
MP 550.9   : 0.5
MP 550.10  : 0.5
MP 550.11  : 0.5
MP 550.12  : 0.5
MP 550.13  : 0.5
             ;--------------------------------------------------------------
             ;SMP551 Spindle:
             ;       Limit value for "Safely limited speed - SLS" in
             ;       operating mode 4 (SOM 4)
             ;       MP560 bit 0 = 1 and code number
             ;Input: 0 to 16000 [rpm]
MP 551.0   : 500
MP 551.1   : 500
             ;--------------------------------------------------------------
             ;SMP552 Axes:
             ;       Limit value for "Safely limited speed - SLS" in
             ;       operating mode 4 (SOM 4)
             ;       MP560 bit 0 = 1 and code number
             ;Input: 0 to 5000 [mm/min] or [degrees/min]
MP 552.0   : 0
MP 552.1   : 0
MP 552.2   : 0
MP 552.3   : 0
MP 552.4   : 0
MP 552.5   : 0
MP 552.6   : 0
MP 552.7   : 0
MP 552.8   : 0
MP 552.9   : 0
MP 552.10  : 0
MP 552.11  : 0
MP 552.12  : 0
MP 552.13  : 0
             ;--------------------------------------------------------------
             ;SMP555 Display mode for rotary axes and PLC auxiliary axes
             ;Input: 0.0000 to 99.999 9999 [degrees]
             ;        0 = No modulo
             ;            Software limit switches active
             ;       >0 = Modulo value for display
             ;            Software limit switches inactive
MP 555.0   : 0
MP 555.1   : 0
MP 555.2   : 0
MP 555.3   : 0
MP 555.4   : 0
MP 555.5   : 0
MP 555.6   : 0
MP 555.7   : 0
MP 555.8   : 0
MP 555.9   : 0
MP 555.10  : 0
MP 555.11  : 0
MP 555.12  : 0
MP 555.13  : 0
             ;--------------------------------------------------------------
             ;SMP560 Additional functions
             ;Input: %xxxxxxxxx
             ;Bit 0    Enabling of operating mode 4 (SOM 4)
             ;         (Permissive button only for spindle or NC start
             ;         and separately selectable spindle speed and
             ;         axis feed rates)
             ;         0 = SOM 4 not allowed
             ;         1 = SOM 4 allowed
             ;Bit 1    Protective doors during safety self-test
             ;         0 = Protective doors need to be closed
             ;         1 = Protective doors do not need to be closed
             ;Bit 2    After external stop, S = 0 and closing of the
             ;         protective doors  (only MC 422)
             ;         0 = NC start only via keystroke
             ;         1 = NC start via keystroke or PLC
             ;Bit 3    Test of brake activation
             ;         0 = Active
             ;         1 = Inactive
             ;Bit 4    Reaction upon opening the protective doors,
             ;         when the time for the safety self-test has passed
             ;         (only MC 422)
             ;         0 = SS2 reaction
             ;         1 = No SS2 reaction
             ;Bit 5    Measurement of current of all drives before
             ;         testing pulse-deletion with the switch-off test
             ;         (only MC 422)
             ;         0 = Measurement of the current will be performed
             ;         1 = No measurement of the current
             ;Bit 6    Test of switching off specific axes via PWM
             ;         output STO.B.x.
             ;         0 = Perform test via STO.B.x for specific axes
             ;         1 = Switch off and test all axes together
             ;Bit 7    Permissive button of the handwheel in
             ;         operating mode 4 (SOM 4)(only MC 422)
             ;         0 = Permissive button only required for spindle
             ;             or NC start
             ;         1 = Permissive button also required for
             ;             maintaining the motion
             ;Bit 8    Wiring of -STO.A.G and -STOS.A.G to safety
             ;         relays in power modules
             ;         0 = Wiring exists
             ;         1 = Wiring does not exist
             ;Bit 9    Interpolating motions in handwheel operation
             ;         in operating modes 3 and 4 (SOM 3, SOM 4)
             ;         (only MC 422)
             ;         0 = Only one axis may be moved at a time
             ;111111----------
             ;5432109876543210 <- bit #
MP 560     : %0000000000000000
             ;--------------------------------------------------------------
             ;SMP561 Enable operating modes (only MC 422)
             ;Input:   0 = Do not enable operating mode 4
             ;         1 = Enable operating mode 4
MP 561     : 0
             ;--------------------------------------------------------------
             ;SMP562 Function of the keys on the machine operating
             ;       panel (only MC 422)
             ;Input: %xx
             ;Bit 0    Activation of the keys on the machine operating
             ;         panel
             ;         0 = Via machine key
             ;         1 = Via permissive key
             ;Bit 1    Axis motions with permissive key
             ;         0 = Releasing the permissive button leads to
             ;             standstill of the axes after 3 s into SBH state
             ;         1 = Releasing the permissive key immediately
             ;             leads to Stop3
             ;10 <- bit #
MP 562     : %00
             ;--------------------------------------------------------------
             ;SMP563 Release tool holder (only MC 422)
             ;Input:   0 = Do not release tool holder with WZS.L.x
             ;         1 = Release tool holder with WZS.L.x
MP 563     : 0
             ;--------------------------------------------------------------
             ;SMP570 Standstill monitoring in SBH state (safe controlled
             ;       stop) inactive (only MC 422).
             ;Input: 0 = Active (bit-coded ...IV Z Y X)
             ;       1 = Inactive (bit-coded ...IV Z Y X)
MP 570     : 0
             ;--------------------------------------------------------------
             ;SMP580 Numbers of inputs of machine operating panel
             ;      (only MC 422)
             ;Input: 128 to 152
MP 580.0   : 0          ;Direction key X+
MP 580.1   : 0          ;Direction key Y+
MP 580.2   : 0          ;Direction key Z+
MP 580.3   : 0          ;Direction key IV+
             ;-------------------------------------------------------------
             ;SMP581 Numbers of inputs of machine operating panel
             ;      (only MC 422)
             ;Input: 128 to 152
MP 581.0   : 0          ;Direction key X-
MP 581.1   : 0          ;Direction key Y-
MP 581.2   : 0          ;Direction key Z-
MP 581.3   : 0          ;Direction key IV-
             ;-------------------------------------------------------------
             ;SMP582 Numbers of inputs of machine operating panel
             ;      (only MC 422)
             ;Input: 128 to 152
MP 582.0   : 0          ; Spindle start
MP 582.1   : 0          ; Spindle jog mode
MP 582.2   : 0          ; Spindle stop
MP 582.3   : 0          ; Permissive button
MP 582.4   : 0          ; Unclamp tool holder
MP 582.5   : 0          ; NC start
MP 582.6   : 0          ; NC stop
MP 582.7   : 0          ; Coolant on
MP 582.8   : 0          ; Unlock door
MP 582.9   : 0          ; Rapid traverse
MP 582.10  : 0          ; PLC emergency stop
MP 582.11  : 0          ; Spindle left
MP 582.12  : 0          ; Spindle right
MP 582.13  : 0          ; NC stop and spindle stop
             ;--------------------------------------------------------------
             ;SMP583 Numbers of handwheel inputs
             ;      (only MC 422)
             ;Input: 173 to 175
MP 583.0   : 0          ;Spindle start
MP 583.1   : 0          ;Spindle jog mode
MP 583.2   : 0          ;Spindle stop
MP 583.3   : 0          ;NC start
MP 583.4   : 0          ;NC stop
             ;--------------------------------------------------------------
             ;SMP585 Inverted PLC inputs of channel A. Maximum 8.
             ;Input: PLC operand address
             ;       -1 = no inverted PLC inputs
MP 585.0   : +0
MP 585.1   : +0
MP 585.2   : +0
MP 585.3   : +0
MP 585.4   : +0
MP 585.5   : +0
MP 585.6   : +0
MP 585.7   : +0
             ;--------------------------------------------------------------
             ;SMP586 Inverted PLC inputs of channel B. Maximum 8.
             ;Input: PLC operand address
             ;       -1 = no inverted PLC inputs
MP 586.0   : +0
MP 586.1   : +0
MP 586.2   : +0
MP 586.3   : +0
MP 586.4   : +0
MP 586.5   : +0
MP 586.6   : +0
MP 586.7   : +0
             ;--------------------------------------------------------------
             ;SMP587 PLC inputs tested in the cyclic minute test.
             ;       Power must be supplied via -TEST.A and -TEST.B.
             ;       Maximum 16.
             ;Input: PLC operand address
MP 587.0   : +0
MP 587.1   : +0
MP 587.2   : +0
MP 587.3   : +0
MP 587.4   : +0
MP 587.5   : +0
MP 587.6   : +0
MP 587.7   : +0
MP 587.8   : +0
MP 587.9   : +0
MP 587.10  : +0
MP 587.11  : +0
MP 587.12  : +0
MP 587.13  : +0
MP 587.14  : +0
MP 587.15  : +0
             ;--------------------------------------------------------------
             ;SMP590 Axes:
             ;       Limit value for "Safely limited speed - SLS" in
             ;       operating mode 2 (SOM 2)
             ;Input: 0 to 2000 [mm/min] or [degrees/min]
MP 590.0   : 2000
MP 590.1   : 2000
MP 590.2   : 2000
MP 590.3   : 2000
MP 590.4   : 2000
MP 590.5   : 2000
MP 590.6   : 2000
MP 590.7   : 2000
MP 590.8   : 2000
MP 590.9   : 2000
MP 590.10  : 2000
MP 590.11  : 2000
MP 590.12  : 2000
MP 590.13  : 2000
             ;--------------------------------------------------------------
             ;SMP591 Spindle:
             ;       Limit value for "Safely limited speed - SLS" in
             ;       operating mode 2 (SOM 2)
             ;Input: 0 to 16000 [rpm]
MP 591.0   : 500
MP 591.1   : 500
             ;--------------------------------------------------------------
             ;SMP600 Axes:
             ;       Assignment of axes to axis groups
             ;Input: 0 to 7
             ;       -1 = Not a safe axis
MP 600.0   : +0
MP 600.1   : +0
MP 600.2   : +0
MP 600.3   : +0
MP 600.4   : +0
MP 600.5   : +0
MP 600.6   : +0
MP 600.7   : +0
MP 600.8   : +0
MP 600.9   : +0
MP 600.10  : +0
MP 600.11  : +0
MP 600.12  : +0
MP 600.13  : +0
             ;--------------------------------------------------------------
             ;SMP601 Spindles:
             ;       Assignment of spindles to axis groups
             ;Input: 0 to 7
             ;       -1 = Not a safe spindle
MP 601.0   : +1
MP 601.1   : +1
             ;--------------------------------------------------------------
             ;SMP610 Definition of braking sequence
             ;       Definition of axis groups that must be at standstill
             ;       before the axis group itself is braked
             ;Input: %xxxxxxxx
             ;       1 = Prior braking of axis group required
             ;76543210 <- bit #
MP 610.0   : %00000000
MP 610.1   : %00000000
MP 610.2   : %00000000
MP 610.3   : %00000000
MP 610.4   : %00000000
MP 610.5   : %00000000
MP 610.6   : %00000000
MP 610.7   : %00000000
             ;--------------------------------------------------------------
             ;SMP630 Maximum permissible speed deviation in % for
             ;       actual/nominal monitoring with speed values for axis
             ;Input: 0 to 20 [%] of the current nominal speed
MP 630.0   : 1
MP 630.1   : 1
MP 630.2   : 1
MP 630.3   : 1
MP 630.4   : 1
MP 630.5   : 1
MP 630.6   : 1
MP 630.7   : 1
MP 630.8   : 1
MP 630.9   : 1
MP 630.10  : 1
MP 630.11  : 1
MP 630.12  : 1
MP 630.13  : 1
             ;--------------------------------------------------------------
             ;SMP631 Maximum permissible speed deviation in % for
             ;       actual/nominal monitoring with speed values for spindle
             ;Input: 0 to 20 [%] of the current nominal speed
MP 631.0   : 1
MP 631.1   : 1
             ;--------------------------------------------------------------
             ;SMP632 Delay time for
             ;       actual/nominal monitoring with speed values for axis
             ;Input: 0 to 60 [s] of the current nominal speed
MP 632.0   : 10
MP 632.1   : 10
MP 632.2   : 10
MP 632.3   : 10
MP 632.4   : 10
MP 632.5   : 10
MP 632.6   : 10
MP 632.7   : 10
MP 632.8   : 10
MP 632.9   : 10
MP 632.10  : 10
MP 632.11  : 10
MP 632.12  : 10
MP 632.13  : 10
             ;--------------------------------------------------------------
             ;SMP633 Delay time for
             ;       actual/nominal monitoring with speed values for spindle
             ;Input: 0 to 60 [s] of the current nominal speed
MP 633.0   : 10
MP 633.1   : 10
             ;--------------------------------------------------------------
             ;SMP640 Axes: Meaning for CC: (only MC 422)
             ;       Maximum position deviation between MCU and CCU
             ;       during operation
             ;       Meaning for MC:
             ;       Maximum permissible deviation for actual/nominal
             ;       value monitoring during operation
             ;Input:  0 to 30 [mm or °]
MP 640.0   : 0          ;X axis
MP 640.1   : 0          ;Y axis
MP 640.2   : 0          ;Z axis
MP 640.3   : 0          ;Axis 4
             ;--------------------------------------------------------------
             ;SMP641 Maximum permissible position deviation for
             ;       actual/nominal monitoring with position values
             ;Input: 0 to 30 000 [mm] or [°]
MP 641.0   : 1
MP 641.1   : 1
MP 641.2   : 1
MP 641.3   : 1
MP 641.4   : 1
MP 641.5   : 1
MP 641.6   : 1
MP 641.7   : 1
MP 641.8   : 1
MP 641.9   : 1
MP 641.10  : 1
MP 641.11  : 1
MP 641.12  : 1
MP 641.13  : 1
             ;--------------------------------------------------------------
             ;SMP642 Axes: Maximum position deviation between MC and CC
             ;       when traversing the reference mark;
             ;       Maximum position deviation to the marking when
             ;       testing the axes
             ;Input: 0 to 30 000 [mm] or [°]
MP 642.0   : 1
MP 642.1   : 1
MP 642.2   : 1
MP 642.3   : 1
MP 642.4   : 1
MP 642.5   : 1
MP 642.6   : 1
MP 642.7   : 1
MP 642.8   : 1
MP 642.9   : 1
MP 642.10  : 1
MP 642.11  : 1
MP 642.12  : 1
MP 642.13  : 1
             ;--------------------------------------------------------------
             ;SMP646 Axes: Position at which the operator can test the
             ;       agreement of the actual position with the position
             ;       values used internally (position of mark)
             ;Input: -99 999.999 to +99 999.9999 [mm] or [°]
MP 646.0   : +0
MP 646.1   : +0
MP 646.2   : +0
MP 646.3   : +0
MP 646.4   : +0
MP 646.5   : +0
MP 646.6   : +0
MP 646.7   : +0
MP 646.8   : +0
MP 646.9   : +0
MP 646.10  : +0
MP 646.11  : +0
MP 646.12  : +0
MP 646.13  : +0
             ;--------------------------------------------------------------
             ;SMP650 Positive absolute position limit values relative to
             ;       the machine datum
             ;Input: -99 999.9999 to +99 999.9999 [mm] or [°]
MP 650.0   : +99999
MP 650.1   : +99999
MP 650.2   : +99999
MP 650.3   : +99999
MP 650.4   : +99999
MP 650.5   : +99999
MP 650.6   : +99999
MP 650.7   : +99999
MP 650.8   : +99999
MP 650.9   : +99999
MP 650.10  : +99999
MP 650.11  : +99999
MP 650.12  : +99999
MP 650.13  : +99999
             ;--------------------------------------------------------------
             ;SMP670 Negative absolute position limit values relative to
             ;       the machine datum
             ;Input: -99 999.9999 to +99 999.9999 [mm] or [°]
MP 670.0   : -99999
MP 670.1   : -99999
MP 670.2   : -99999
MP 670.3   : -99999
MP 670.4   : -99999
MP 670.5   : -99999
MP 670.6   : -99999
MP 670.7   : -99999
MP 670.8   : -99999
MP 670.9   : -99999
MP 670.10  : -99999
MP 670.11  : -99999
MP 670.12  : -99999
MP 670.13  : -99999
             ;--------------------------------------------------------------
             ;SMP691.0 Reserved
MP 691.0   : 0
             ;--------------------------------------------------------------
             ;SMP691.1 Reserved
MP 691.1   : 0
             ;--------------------------------------------------------------
             ;SMP691.2 Reserved
MP 691.2   : 0
             ;--------------------------------------------------------------
             ;SMP693 Version of the SPLC-API used to create the SPLC program
             ;Input: Numerical value of the API version
             ;         e.g. 100 for version 1.00
MP 693     : 0
             ;--------------------------------------------------------------
             ;MP708 Dynamic backlash compensation
             ;Input:  0 = Inactive
             ;        0.0001 to 1.0000 [mm]
MP 708.0   : 0
MP 708.1   : 0
MP 708.2   : 0
MP 708.3   : 0
MP 708.4   : 0
MP 708.5   : 0
MP 708.6   : 0
MP 708.7   : 0
MP 708.8   : 0
MP 708.9   : 0
MP 708.10  : 0
MP 708.11  : 0
MP 708.12  : 0
MP 708.13  : 0
             ;--------------------------------------------------------------
             ;MP709 Filter for backlash compensation
             ;Input: 0 = no filter function for MP710
             ;       +1 to +1000 [ms] = time constant for MP710
MP 709.0   : 0
MP 709.1   : 0
MP 709.2   : 0
MP 709.3   : 0
MP 709.4   : 0
MP 709.5   : 0
MP 709.6   : 0
MP 709.7   : 0
MP 709.8   : 0
MP 709.9   : 0
MP 709.10  : 0
MP 709.11  : 0
MP 709.12  : 0
MP 709.13  : 0
             ;--------------------------------------------------------------
             ;MP710 Backlash compensation
             ;Input: -9.9999 to +9.9999 [mm] or [degrees]
MP 710.0   : +0
MP 710.1   : +0
MP 710.2   : +0
MP 710.3   : +0
MP 710.4   : +0
MP 710.5   : +0
MP 710.6   : +0
MP 710.7   : +0
MP 710.8   : +0
MP 710.9   : +0
MP 710.10  : +0
MP 710.11  : +0
MP 710.12  : +0
MP 710.13  : +0
             ;--------------------------------------------------------------
             ;MP711 Height of reversal spikes during rotation
             ;for analog axes (MP2610.x - MP2620.x for digital axes)
             ;Input: -1.0000 to +1.0000 [mm]
MP 711.0   : +0
MP 711.1   : +0
MP 711.2   : +0
MP 711.3   : +0
MP 711.4   : +0
MP 711.5   : +0
MP 711.6   : +0
MP 711.7   : +0
MP 711.8   : +0
MP 711.9   : +0
MP 711.10  : +0
MP 711.11  : +0
MP 711.12  : +0
MP 711.13  : +0
             ;--------------------------------------------------------------
             ;MP712 Compensation value per control loop cycle time
             ;for analog axes (MP2610.x - MP2620.x for digital axes)
             ;Input: 0.000000 to 99.999999 [mm]
MP 712.0   : 0
MP 712.1   : 0
MP 712.2   : 0
MP 712.3   : 0
MP 712.4   : 0
MP 712.5   : 0
MP 712.6   : 0
MP 712.7   : 0
MP 712.8   : 0
MP 712.9   : 0
MP 712.10  : 0
MP 712.11  : 0
MP 712.12  : 0
MP 712.13  : 0
             ;--------------------------------------------------------------
             ;MP715 Height of reversal spikes during rotation with M105
             ;for analog axes (MP2610.x - MP2620.x for digital axes)
             ;Input: -1.0000 to +1.0000 [mm]
MP 715.0   : +0
MP 715.1   : +0
MP 715.2   : +0
MP 715.3   : +0
MP 715.4   : +0
MP 715.5   : +0
MP 715.6   : +0
MP 715.7   : +0
MP 715.8   : +0
MP 715.9   : +0
MP 715.10  : +0
MP 715.11  : +0
MP 715.12  : +0
MP 715.13  : +0
             ;--------------------------------------------------------------
             ;MP716 Compensation value per control loop cycle time with M105
             ;Input: 0.000000 to 99.999999 [mm]
MP 716.0   : 0
MP 716.1   : 0
MP 716.2   : 0
MP 716.3   : 0
MP 716.4   : 0
MP 716.5   : 0
MP 716.6   : 0
MP 716.7   : 0
MP 716.8   : 0
MP 716.9   : 0
MP 716.10  : 0
MP 716.11  : 0
MP 716.12  : 0
MP 716.13  : 0
             ;--------------------------------------------------------------
             ;MP720 Linear axis-error compensation
             ;for analog axes
             ;Input: -1.000 to +1.000 [mm/m]
MP 720.0   : +0
MP 720.1   : +0
MP 720.2   : +0
MP 720.3   : +0
MP 720.4   : +0
MP 720.5   : +0
MP 720.6   : +0
MP 720.7   : +0
MP 720.8   : +0
MP 720.9   : +0
MP 720.10  : +0
MP 720.11  : +0
MP 720.12  : +0
MP 720.13  : +0
             ;--------------------------------------------------------------
             ;MP730 Select the axis-error compensation
             ;Input: %987654321 bit-encoded
             ;       0 = Linear axis-error compensation
             ;       1 = Non-linear axis-error compensation
             ;.............987654ZYX <- axis
             ;221111111111----------
             ;1098765432109876543210 <- bit #
MP 730     : %1111111111111111111111
             ;--------------------------------------------------------------
             ;MP732 Non-linear axis-error compensation for angular axes
             ;      is mapped to range of traverse (if MP730 = 1).
             ;      Instead of 0..360 degree e.g. -90..+90 degree.
             ;Input: %987654321 bit-encoded
             ;       0 = Inactive (usual compensation)
             ;       1 = Active (mapping to range of traverse)
             ;.............987654ZYX <- axis
             ;221111111111----------
             ;1098765432109876543210 <- bit #
MP 732     : %0000000000000000000000
             ;--------------------------------------------------------------
             ;MP748 Dynamic reversal error compensation
             ;Input:  0 = Inactive
             ;        0.0001 to 1.0000 [mm]
MP 748.0   : 0
MP 748.1   : 0
MP 748.2   : 0
MP 748.3   : 0
MP 748.4   : 0
MP 748.5   : 0
MP 748.6   : 0
MP 748.7   : 0
MP 748.8   : 0
MP 748.9   : 0
MP 748.10  : 0
MP 748.11  : 0
MP 748.12  : 0
MP 748.13  : 0
             ;--------------------------------------------------------------
             ;MP750 Backlash compensation (distance)
             ;Input: -9.9999 to +9.9999 [mm] or [degrees]
MP 750.0   : +0
MP 750.1   : +0
MP 750.2   : +0
MP 750.3   : +0
MP 750.4   : +0
MP 750.5   : +0
MP 750.6   : +0
MP 750.7   : +0
MP 750.8   : +0
MP 750.9   : +0
MP 750.10  : +0
MP 750.11  : +0
MP 750.12  : +0
MP 750.13  : +0
             ;--------------------------------------------------------------
             ;MP752 Backlash compensation (time)
             ;Input: 0 to 1000 [ms]
MP 752.0   : 0
MP 752.1   : 0
MP 752.2   : 0
MP 752.3   : 0
MP 752.4   : 0
MP 752.5   : 0
MP 752.6   : 0
MP 752.7   : 0
MP 752.8   : 0
MP 752.9   : 0
MP 752.10  : 0
MP 752.11  : 0
MP 752.12  : 0
MP 752.13  : 0
             ;--------------------------------------------------------------
             ;MP810  Display mode for rotary axes and PLC auxiliary axes
             ;Input:   0.0000 to 99 999.9999 [degrees]
             ;         0 = Display +-99 999.9999
             ;             Software limit switches active
             ;       <>0 = Modulo value for display
             ;             Software limit switches inactive
MP 810.0   : 0
MP 810.1   : 0
MP 810.2   : 0
MP 810.3   : 360
MP 810.4   : 360
MP 810.5   : 0
MP 810.6   : 0
MP 810.7   : 0
MP 810.8   : 0
MP 810.9   : 0
MP 810.10  : 0
MP 810.11  : 0
MP 810.12  : 0
MP 810.13  : 0
             ;--------------------------------------------------------------
             ;MP812 Active software limit switches with modulo display, M94
             ;      and encoders with EnDat interface
             ;Input: %987654321 bit-encoded
             ;       0 = Software limit switch inactive
             ;       1 = Software limit switch active
             ;.............987654ZYX <- axis
             ;221111111111----------
             ;1098765432109876543210 <- bit #
MP 812     : %0000000000000000000000
             ;--------------------------------------------------------------
             ;MP850 Synchronized axes
             ;Input: 0 = Master axis
             ;       1 = Slave axis to axis 1
             ;       2 = Slave axis to axis 2
             ;       ...
             ;       9 = Slave axis to axis 9
MP 850.0   : 0          ;e.g. 0 = Master axis 1
MP 850.1   : 0          ;     0 = Master axis 2
MP 850.2   : 0          ;     0 = Master axis 3
MP 850.3   : 0          ;     1 = Axis 4 coupled with 1
MP 850.4   : 0          ;     2 = Axis 5 coupled with 2
MP 850.5   : 0
MP 850.6   : 0
MP 850.7   : 0
MP 850.8   : 0
MP 850.9   : 0
MP 850.10  : 0
MP 850.11  : 0
MP 850.12  : 0
MP 850.13  : 0
             ;--------------------------------------------------------------
             ;MP855 Synchronization monitoring
             ;Input: 0 to 100.0000 [mm]
             ;       0 = Monitoring inactive
MP 855.0   : 0
MP 855.1   : 0
MP 855.2   : 0
MP 855.3   : 0
MP 855.4   : 0
MP 855.5   : 0
MP 855.6   : 0
MP 855.7   : 0
MP 855.8   : 0
MP 855.9   : 0
MP 855.10  : 0
MP 855.11  : 0
MP 855.12  : 0
MP 855.13  : 0
             ;--------------------------------------------------------------
             ;MP860 Synchronous control
             ;Input: %xxx
             ;Bit 0   Datum for synchronous control
             ;        0 = Datum at position after switch-on
             ;        1 = Datum at reference marks (machine datum)
             ;Bit 1   Master-slave torque control
             ;        0 = No master-slave torque controlled axis
             ;        1 = Axis is torque slave axis
             ;Bit 2   Brake test of slave axis
             ;        0 = No automatic brake test of slave axis
             ;        1 = Brake test of slave axis is started automatically
             ;           with the brake test of the master axis
             ;210 <- bit #
MP 860.0   : %000
MP 860.1   : %000
MP 860.2   : %000
MP 860.3   : %000
MP 860.4   : %000
MP 860.5   : %000
MP 860.6   : %000
MP 860.7   : %000
MP 860.8   : %000
MP 860.9   : %000
MP 860.10  : %000
MP 860.11  : %000
MP 860.12  : %000
MP 860.13  : %000
             ;--------------------------------------------------------------
             ;MP91x + Ranges of traverse (activation via PLC)
             ;Input: -99999.9999 to +99999.9999 [mm] or [degrees]
             ;+Limit switch for 1st range of traverse
MP 910.0   : +1054      ;w/ scale=+1054, w/o scale=+968
MP 910.1   : +690       ;w/ scale=+690, w/o scale=+51
MP 910.2   : +650       ;w/ scale=+650, w/o scale=+49
MP 910.3   : +99999
MP 910.4   : +99999
MP 910.5   : +99999
MP 910.6   : +99999
MP 910.7   : +99999
MP 910.8   : +99999
MP 910.9   : +99999
MP 910.10  : +99999
MP 910.11  : +99999
MP 910.12  : +99999
MP 910.13  : +99999
             ;--------------------------------------------------------------
             ;+Limit switch for 2nd range of traverse
MP 911.0   : +1054      ;w/ scale=+1054, w/o scale=+968
MP 911.1   : +690       ;w/ scale=+690, w/o scale=+51
MP 911.2   : +650       ;w/ scale=+650, w/o scale=+49
MP 911.3   : +99999
MP 911.4   : +99999
MP 911.5   : +99999
MP 911.6   : +99999
MP 911.7   : +99999
MP 911.8   : +99999
MP 911.9   : +99999
MP 911.10  : +99999
MP 911.11  : +99999
MP 911.12  : +99999
MP 911.13  : +99999
             ;--------------------------------------------------------------
             ;+Limit switch for 3rd range of traverse
MP 912.0   : +1054      ;w/ scale=+1054, w/o scale=+968
MP 912.1   : +690       ;w/ scale=+690, w/o scale=+51
MP 912.2   : +650       ;w/ scale=+650, w/o scale=+49
MP 912.3   : +99999
MP 912.4   : +99999
MP 912.5   : +99999
MP 912.6   : +99999
MP 912.7   : +99999
MP 912.8   : +99999
MP 912.9   : +99999
MP 912.10  : +99999
MP 912.11  : +99999
MP 912.12  : +99999
MP 912.13  : +99999
             ;--------------------------------------------------------------
             ;MP92x - Ranges of traverse (activation via PLC)
             ;Input: -99999.9999 to +99999.9999 [mm] or [degrees]
             ;-Limit switch for 1st range of traverse
MP 920.0   : +34        ;w/ scale=+34, w/o scale=-52
MP 920.1   : +80        ;w/ scale=+80, w/o scale=-559
MP 920.2   : +40        ;w/ scale=+40, w/o scale=-561
MP 920.3   : -99999
MP 920.4   : -99999
MP 920.5   : -99999
MP 920.6   : -99999
MP 920.7   : -99999
MP 920.8   : -99999
MP 920.9   : -99999
MP 920.10  : -99999
MP 920.11  : -99999
MP 920.12  : -99999
MP 920.13  : -99999
             ;--------------------------------------------------------------
             ;-Limit switch for 2nd range of traverse
MP 921.0   : +34        ;w/ scale=+34, w/o scale=-52
MP 921.1   : +80        ;w/ scale=+80, w/o scale=-559
MP 921.2   : +40        ;w/ scale=+40, w/o scale=-561
MP 921.3   : -99999
MP 921.4   : -99999
MP 921.5   : -99999
MP 921.6   : -99999
MP 921.7   : -99999
MP 921.8   : -99999
MP 921.9   : -99999
MP 921.10  : -99999
MP 921.11  : -99999
MP 921.12  : -99999
MP 921.13  : -99999
             ;--------------------------------------------------------------
             ;-Limit switch for 3rd range of traverse
MP 922.0   : +34        ;w/ scale=+34, w/o scale=-52
MP 922.1   : +80        ;w/ scale=+80, w/o scale=-559
MP 922.2   : +40        ;w/ scale=+40, w/o scale=-561
MP 922.3   : -99999
MP 922.4   : -99999
MP 922.5   : -99999
MP 922.6   : -99999
MP 922.7   : -99999
MP 922.8   : -99999
MP 922.9   : -99999
MP 922.10  : -99999
MP 922.11  : -99999
MP 922.12  : -99999
MP 922.13  : -99999
             ;--------------------------------------------------------------
             ;MP950 Datum for positioning blocks with M92
             ;Input: -99 999.9999 to +99 999.9999 [mm] or [degrees]
             ;       Values with respect to the machine datum
MP 950.0   : +0
MP 950.1   : +0
MP 950.2   : +0
MP 950.3   : +0
MP 950.4   : +0
MP 950.5   : +0
MP 950.6   : +0
MP 950.7   : +0
MP 950.8   : +0
MP 950.9   : +0
MP 950.10  : +0
MP 950.11  : +0
MP 950.12  : +0
MP 950.13  : +0
             ;--------------------------------------------------------------
             ;MP951 Simulated tool-change position for
             ;Tool Call during a block scan
             ;Input: -99 999.9999 to +99 999.9999 [mm] or [degrees]
MP 951.0   : +0
MP 951.1   : +0
MP 951.2   : +637.207   ;same position as MP4210.14
MP 951.3   : +0
MP 951.4   : +0
MP 951.5   : +0
MP 951.6   : +0
MP 951.7   : +0
MP 951.8   : +0
MP 951.9   : +0
MP 951.10  : +0
MP 951.11  : +0
MP 951.12  : +0
MP 951.13  : +0
             ;--------------------------------------------------------------
             ;MP960 Machine datum
             ;Input: -1.79769313486E+308 to +1.79769313486E+308 [mm] or
             ;       [degrees]
             ;       Values with respect to scale reference point
MP 960.0   : +0.00000
MP 960.1   : +0.00000
MP 960.2   : +0.00000
MP 960.3   : +0.00000
MP 960.4   : +0.00000
MP 960.5   : +0.00000
MP 960.6   : +0.00000
MP 960.7   : +0.00000
MP 960.8   : +0.00000
MP 960.9   : +0.00000
MP 960.10  : +0.00000
MP 960.11  : +0.00000
MP 960.12  : +0.00000
MP 960.13  : +0.00000
             ;==============================================================
             ;MP 1000 .. 1399
             ;Positioning / Controlling
             ;==============================================================
             ;MP1010 Axis-specific rapid traverse
             ;Input: 10 to 1 000 000 [mm/min] or [degrees/min]
MP 1010.0  : 30000
MP 1010.1  : 30000
MP 1010.2  : 24000
MP 1010.3  : 4000
MP 1010.4  : 10000
MP 1010.5  : 10000
MP 1010.6  : 10000
MP 1010.7  : 10000
MP 1010.8  : 10000
MP 1010.9  : 10000
MP 1010.10 : 10000
MP 1010.11 : 10000
MP 1010.12 : 10000
MP 1010.13 : 10000
             ;--------------------------------------------------------------
             ;MP1011 Max. rapid traverse for path interpolation
             ;Input: 10 to 1 000 000 [mm/min] or [degrees/min]
MP 1011    : 300000
             ;--------------------------------------------------------------
             ;MP1012 Second axis-specific rapid traverse. Switching between
             ;       rapid traverse in MP1010 or MP1012 via function FN17.
             ;       With use of rapid traverse in MP1012 is
             ;       MP1011 inactive.
             ;Input: 10 to 1 000 000 [mm/min] or [degrees/min]
MP 1012.0  : 300000
MP 1012.1  : 300000
MP 1012.2  : 300000
MP 1012.3  : 300000
MP 1012.4  : 300000
MP 1012.5  : 300000
MP 1012.6  : 300000
MP 1012.7  : 300000
MP 1012.8  : 300000
MP 1012.9  : 300000
MP 1012.10 : 300000
MP 1012.11 : 300000
MP 1012.12 : 300000
MP 1012.13 : 300000
             ;--------------------------------------------------------------
             ;MP1020 Axis-specific manual feed rate
             ;Input: 10 to 300 000 [mm/min] or [degrees/min]
MP 1020.0  : 3000
MP 1020.1  : 3000
MP 1020.2  : 3000
MP 1020.3  : 1000
MP 1020.4  : 1000
MP 1020.5  : 1000
MP 1020.6  : 1000
MP 1020.7  : 1000
MP 1020.8  : 1000
MP 1020.9  : 1000
MP 1020.10 : 1000
MP 1020.11 : 1000
MP 1020.12 : 1000
MP 1020.13 : 1000
             ;--------------------------------------------------------------
             ;MP1030 Axis-specific positioning window
             ;Input: 0.0001 to 2.0000 [mm] or [degrees]
MP 1030.0  : 0.02
MP 1030.1  : 0.02
MP 1030.2  : 0.02
MP 1030.3  : 0.02
MP 1030.4  : 0.01
MP 1030.5  : 0.01
MP 1030.6  : 0.01
MP 1030.7  : 0.01
MP 1030.8  : 0.01
MP 1030.9  : 0.01
MP 1030.10 : 0.01
MP 1030.11 : 0.01
MP 1030.12 : 0.01
MP 1030.13 : 0.01
             ;--------------------------------------------------------------
             ;MP1040 Polarity of the nominal speed value for positive
             ;       direction of traverse
             ;Input: %987654321 bit-encoded
             ; 0 = Positive
             ; 1 = Negative
             ;.............987654ZYX <- axis
             ;221111111111----------
             ;1098765432109876543210 <- bit #
MP 1040    : %0000000000000000000100
             ;                  0100 <- without scale (MP1040)
             ;                  ?100 <- with scale (MP1040)
             ;--------------------------------------------------------------
             ;MP1050 Analog voltage for rapid traverse
             ;Input: 1.000 to 9.000 [V]
MP 1050.0  : 1
MP 1050.1  : 1
MP 1050.2  : 1
MP 1050.3  : 1
MP 1050.4  : 1
MP 1050.5  : 1
MP 1050.6  : 1
MP 1050.7  : 1
MP 1050.8  : 1
MP 1050.9  : 1
MP 1050.10 : 1
MP 1050.11 : 1
MP 1050.12 : 1
MP 1050.13 : 1
             ;--------------------------------------------------------------
             ;MP1054 Linear distance of one motor revolution
             ;Input: 0 to a maximum of 15 figures [mm]
             ;       Example: 123456798.012345 [mm]
             ;       Example: 123.456798012345 [mm]
MP 1054.0  : 16
MP 1054.1  : 16
MP 1054.2  : 12
MP 1054.3  : 2
MP 1054.4  : 10
MP 1054.5  : 10
MP 1054.6  : 10
MP 1054.7  : 10
MP 1054.8  : 10
MP 1054.9  : 10
MP 1054.10 : 10
MP 1054.11 : 10
MP 1054.12 : 10
MP 1054.13 : 10
             ;--------------------------------------------------------------
             ;MP1060 Axis-specific acceleration
             ;Input: 0.001 to 500 [m/s^2] or [1000 degree/s^2]
MP 1060.0  : 3
MP 1060.1  : 2.1
MP 1060.2  : 4
MP 1060.3  : 1.9
MP 1060.4  : 0.5
MP 1060.5  : 0.5
MP 1060.6  : 0.5
MP 1060.7  : 0.5
MP 1060.8  : 0.5
MP 1060.9  : 0.5
MP 1060.10 : 0.5
MP 1060.11 : 0.5
MP 1060.12 : 0.5
MP 1060.13 : 0.5
             ;--------------------------------------------------------------
             ;MP1061 Acceleration limitation for path interpolation
             ;Input: 0.001 to 500 [m/s^2]
             ;       MP1061.0 Path acceleration for machining feed rate
             ;       MP1061.1 Path acceleration for rapid traverse
MP 1061.0  : 10
MP 1061.1  : 10
             ;--------------------------------------------------------------
             ;MP1070 Radial acceleration
             ;Input: 0.001 to 500 [m/s^2]
MP 1070    : 1.47
             ;--------------------------------------------------------------
             ;MP1080 Analog drives: Integral-action factor
             ;Input: 0 to 65 535 (digital drives: 0)
MP 1080.0  : 0
MP 1080.1  : 0
MP 1080.2  : 0
MP 1080.3  : 0
MP 1080.4  : 0
MP 1080.5  : 0
MP 1080.6  : 0
MP 1080.7  : 0
MP 1080.8  : 0
MP 1080.9  : 0
MP 1080.10 : 0
MP 1080.11 : 0
MP 1080.12 : 0
MP 1080.13 : 0
             ;--------------------------------------------------------------
             ;MP1085 Max. permissible axis-specific jerk during machining
             ;       movements for the operating modes
             ;       "Program Run Full Sequence," "Program Run Single Block",
             ;       and "Positioning with Manual Data Input",
             ;       as long as feed rate not FMAX or
             ;       feed rate < value in MP1092.
             ;       (up to now for all axis in MP1090.0)
             ;            MP1060.x^2 * 60000
             ; MP1085.x >= ------------------
             ;               MP1010.x
             ;Input: 0.1 to 9999.9 [m/s^3]
MP 1085.0  : 30
MP 1085.1  : 30
MP 1085.2  : 30
MP 1085.3  : 30
MP 1085.4  : 10
MP 1085.5  : 10
MP 1085.6  : 10
MP 1085.7  : 10
MP 1085.8  : 10
MP 1085.9  : 10
MP 1085.10 : 10
MP 1085.11 : 10
MP 1085.12 : 10
MP 1085.13 : 10
             ;--------------------------------------------------------------
             ;MP1086 Max. permissible axis-specific jerk during machining
             ;       movements for the operating modes
             ;       "Program Run Full Sequence," "Program Run Single Block",
             ;       and "Positioning with Manual Data Input",
             ;       when feed rate at rapid traverse or
             ;       feed rate > value in MP1092.
             ;       (up to now for all axis in MP1090.1)
             ;            MP1060.x^2 * 60000
             ; MP1086.x >= ------------------
             ;               MP1010.x
             ;Input: 0.1 to 9999.9 [m/s^3]
             ;       0 = Inactive (even with FMAX value from MP1085.x)
MP 1086.0  : 30
MP 1086.1  : 30
MP 1086.2  : 30
MP 1086.3  : 30
MP 1086.4  : 10
MP 1086.5  : 10
MP 1086.6  : 10
MP 1086.7  : 10
MP 1086.8  : 10
MP 1086.9  : 10
MP 1086.10 : 10
MP 1086.11 : 10
MP 1086.12 : 10
MP 1086.13 : 10
             ;--------------------------------------------------------------
             ;MP1087 Axis-specific jerk limitation in the manual
             ;       mode of operation
             ;Input: 0.1 to 1000 [m/s^3]
MP 1087.0  : 20
MP 1087.1  : 20
MP 1087.2  : 20
MP 1087.3  : 20
MP 1087.4  : 10
MP 1087.5  : 10
MP 1087.6  : 10
MP 1087.7  : 10
MP 1087.8  : 10
MP 1087.9  : 10
MP 1087.10 : 10
MP 1087.11 : 10
MP 1087.12 : 10
MP 1087.13 : 10
             ;--------------------------------------------------------------
             ;MP1088 Axis-specific jerk limitation for non-filtert
             ;       movements
             ;       (PLC positioning, measuring cycles)
             ;Input: 0.1 to 9999.9 [m/s^3]
MP 1088.0  : 10
MP 1088.1  : 10
MP 1088.2  : 10
MP 1088.3  : 10
MP 1088.4  : 5
MP 1088.5  : 5
MP 1088.6  : 5
MP 1088.7  : 5
MP 1088.8  : 5
MP 1088.9  : 5
MP 1088.10 : 5
MP 1088.11 : 5
MP 1088.12 : 5
MP 1088.13 : 5
             ;--------------------------------------------------------------
             ;MP1089 Axis-specific jerk limitation during reference traverse
             ;Input: 0.1 to 1000 [m/s^3]
MP 1089.0  : 20
MP 1089.1  : 20
MP 1089.2  : 20
MP 1089.3  : 20
MP 1089.4  : 10
MP 1089.5  : 10
MP 1089.6  : 10
MP 1089.7  : 10
MP 1089.8  : 10
MP 1089.9  : 10
MP 1089.10 : 10
MP 1089.11 : 10
MP 1089.12 : 10
MP 1089.13 : 10
             ;--------------------------------------------------------------
             ;MP1090 Limitation of contouring jerk as a result from
             ;       MP1085.x or MP1086.x
             ;            MP1060^2 * 60000
             ; MP1090 >= ------------------
             ;               MP1010
             ;Input: 0.1 to 9999.9 [m/s^3]
             ;       0 = Inactive
MP 1090.0  : 0          ;Jerk limitation if a feed rate-value is programmed
MP 1090.1  : 0          ;Jerk limitation if feed rate FMAX is programmed
             ;--------------------------------------------------------------
             ;MP1092 Feed rate value to switch between jerk in MP1085.x or
             ;       MP1086.x
             ;       programmed feed rate < MP1092 --> MP1085.x active
             ;       programmed feed rate > MP1092 --> MP1086.x active
             ;       MP1092 = 0 --> MP1085.x during machining movements
             ;       without FMAX active and MP1086.x during machining
             ;       movements with FMAX active.
             ;Input: 0 to 300 000 [mm/min]
MP 1092    : 10000
             ;--------------------------------------------------------------
             ;MP1110 Standstill monitoring
             ;(Gross positioning error x D)
             ;Input: 0.0010 to 30.0000 [mm]
MP 1110.0  : 0.2
MP 1110.1  : 0.2
MP 1110.2  : 0.2
MP 1110.3  : 0.2
MP 1110.4  : 0.2
MP 1110.5  : 0.2
MP 1110.6  : 0.2
MP 1110.7  : 0.2
MP 1110.8  : 0.2
MP 1110.9  : 0.2
MP 1110.10 : 0.2
MP 1110.11 : 0.2
MP 1110.12 : 0.2
MP 1110.13 : 0.2
             ;--------------------------------------------------------------
             ;MP1120 Standstill monitoring for field angle determination
             ;Input: 0.0000 to 300.0000 [mm] or [°]
MP 1120.0  : 0
MP 1120.1  : 0
MP 1120.2  : 0
MP 1120.3  : 0
MP 1120.4  : 0
MP 1120.5  : 0
MP 1120.6  : 0
MP 1120.7  : 0
MP 1120.8  : 0
MP 1120.9  : 0
MP 1120.10 : 0
MP 1120.11 : 0
MP 1120.12 : 0
MP 1120.13 : 0
             ;--------------------------------------------------------------
             ;MP1140 Movement monitoring
             ;(Gross positioning error x C)
             ;Input: 0.030 to 10.000 [1000/min]
             ;       0.030 [1000/min] recommended
MP 1140.0  : 0.03
MP 1140.1  : 0.03
MP 1140.2  : 0.03
MP 1140.3  : 0.03
MP 1140.4  : 0.03
MP 1140.5  : 0.03
MP 1140.6  : 0.03
MP 1140.7  : 0.03
MP 1140.8  : 0.03
MP 1140.9  : 0.03
MP 1140.10 : 0.03
MP 1140.11 : 0.03
MP 1140.12 : 0.03
MP 1140.13 : 0.03
             ;--------------------------------------------------------------
             ;MP1144 Movement monitoring for position and speed
             ;(Gross positioning error x C)
             ;Input: 0 to 99 999.9999 [mm]
             ;       0 = No monitoring
MP 1144.0  : 0.5        ;w/ scale=0.5, w/o scale=0
MP 1144.1  : 0.5        ;w/ scale=0.5, w/o scale=0
MP 1144.2  : 0.5        ;w/ scale=0.5, w/o scale=0
MP 1144.3  : 0
MP 1144.4  : 0
MP 1144.5  : 0
MP 1144.6  : 0
MP 1144.7  : 0
MP 1144.8  : 0
MP 1144.9  : 0
MP 1144.10 : 0
MP 1144.11 : 0
MP 1144.12 : 0
MP 1144.13 : 0
             ;--------------------------------------------------------------
             ;MP1146 Position monitoring for EnDat, max. permissible
             ;       difference between stored switch-off position and
             ;       the position transferred via the EnDat interface
             ;Input: 0.0000 to 300.0000 [mm]
             ;       0 = No difference permitted
MP 1146.0  : 0.5        ;w/ LC scale=0.5, w/o scale=0
MP 1146.1  : 0.5        ;w/ LC scale=0.5, w/o scale=0
MP 1146.2  : 0.5        ;w/ LC scale=0.5, w/o scale=0
MP 1146.3  : 0
MP 1146.4  : 0
MP 1146.5  : 0
MP 1146.6  : 0
MP 1146.7  : 0
MP 1146.8  : 0
MP 1146.9  : 0
MP 1146.10 : 0
MP 1146.11 : 0
MP 1146.12 : 0
MP 1146.13 : 0
             ;--------------------------------------------------------------
             ;MP1147 Disable position capture for axes with EnDat encoder
             ;Input:   0 = Position can be captured
             ;         1 = Position capturing disabled. Error message and
             ;             emergency stop
             ;.............987654ZYX <- axis
             ;221111111111----------
             ;1098765432109876543210 <- bit #
MP 1147    : %0000000000000000000000
             ;--------------------------------------------------------------
             ;MP1150 Delay time
             ;Input: 0 to 65.535 [s]
             ;Switch off the residual voltage when the error message
             ; "positioning error" appears.
             ;  Recommended value: 0
MP 1150.0  : 0
             ;Time period for which the monitoring functions are to
             ;remain inactive after the fast PLC input defined in MP4130.0
             ;has been set. After expiration of this time, the monitoring
             ;functions are effective again.
             ;  Recommended input range: 0.2 s to 0.5 s
MP 1150.1  : 0.5
             ;Minimum time period for which the monitoring functions are to
             ;remain effective after expiration of the time from MP1150.1
             ;(e.g. if the input changes quickly).
MP 1150.2  : 0
             ;--------------------------------------------------------------
             ;MP1160 Distance for LIFTOFF
             ;Distance from tool to contour, which is taken by the LIFTOFF
             ;function (at emergency stop at DSP-faults)
             ;MP appears at CC 422 too, but has no function.
             ;Input 0 to 30.0000 [mm]
             ;Default: 0.1 [mm]
MP 1160    : 0.1
             ;--------------------------------------------------------------
             ;MP1200 Nominal position value filter
             ;Input: 0 = Single filter
             ;       1 = Double filter
             ;       2 = HSC filter
             ;       3 = "Advanced HSC filter"
MP 1200    : 1
             ;--------------------------------------------------------------
             ;MP1201 Nominal position value filter for modes: Manual,
             ;       Hand wheel, jog increment, reference marks
             ;Input: 0 = Single filter
             ;       1 = Double filter
MP 1201    : 0
             ;--------------------------------------------------------------
             ;MP1202 Cycle 32 3D milling - predefined tolerance
             ;Input: 0,0000 to 3,0000 [mm]
MP 1202.0  : 0.02       ;Corners with movement with machining feed rate
MP 1202.1  : 0.25       ;Corners with movement with rapid traverse
             ;--------------------------------------------------------------
             ;MP1205 Reducing contouring feed rate at the beginning of a
             ;       contour-element
             ;Input: 0 = Inactive (fast but less precision)
             ;       1 = Active (slow but more precision)
MP 1205    : 0
             ;--------------------------------------------------------------
             ;MP1210 Cut-off frequency for single filter
             ;Input: 0.0 to 166.0 [Hz]
MP 1210    : 30
             ;--------------------------------------------------------------
             ;MP1211 Cut-off frequency for double filter
             ;Input: 0.0 to 166.0 [Hz]
MP 1211    : 30
             ;--------------------------------------------------------------
             ;MP1212 Cut-off frequency for for HSC filter
             ;Input: 0.0 to 166.0 [Hz]
MP 1212    : 20
             ;--------------------------------------------------------------
             ;MP1213 Cut-off frequency for "advanced HSC filter"
             ;Input: 0,0 to 166,0 [Hz]
MP 1213    : 20
             ;--------------------------------------------------------------
             ;MP1218 Cut-off frequency for single filter for
             ;       rotary axis (A, B, C)
             ;Input: 0.0 to 166.0 [Hz]
             ;       0 = Triangle filter (default)
MP 1218.0  : 0
MP 1218.1  : 0
MP 1218.2  : 0
             ;--------------------------------------------------------------
             ;MP1222 Tolerance at changes of curvature for HSC filter
             ;Input: 0 = Ignore tolerance
             ;       1 = Do not ignore tolerance
MP 1222    : 0
             ;--------------------------------------------------------------
             ;MP1223 Tolerance at changes of curvature for
             ;       "advanced HSC filter"
             ;Input: 0 = Ignore tolerance
             ;       1 = Do not ignore tolerance
MP 1223    : 0
             ;--------------------------------------------------------------
             ;MP1230 Jerk at corners for single filter
             ;Input: 0.1 to 1000.0 [m/s^3]
MP 1230.0  : 30
MP 1230.1  : 30
MP 1230.2  : 30
MP 1230.3  : 30
MP 1230.4  : 10
MP 1230.5  : 10
MP 1230.6  : 10
MP 1230.7  : 10
MP 1230.8  : 10
MP 1230.9  : 10
MP 1230.10 : 10
MP 1230.11 : 10
MP 1230.12 : 10
MP 1230.13 : 10
             ;--------------------------------------------------------------
             ;MP1231 Jerk at corners for  double filter
             ;Input: 0.1 to 1000.0 [m/s^3]
MP 1231.0  : 30
MP 1231.1  : 30
MP 1231.2  : 30
MP 1231.3  : 30
MP 1231.4  : 10
MP 1231.5  : 10
MP 1231.6  : 10
MP 1231.7  : 10
MP 1231.8  : 10
MP 1231.9  : 10
MP 1231.10 : 10
MP 1231.11 : 10
MP 1231.12 : 10
MP 1231.13 : 10
             ;--------------------------------------------------------------
             ;MP1232 Jerk at corners for HSC filter
             ;Input: 0.1 to 1000.0 [m/s^3]
MP 1232.0  : 30
MP 1232.1  : 30
MP 1232.2  : 30
MP 1232.3  : 30
MP 1232.4  : 10
MP 1232.5  : 10
MP 1232.6  : 10
MP 1232.7  : 10
MP 1232.8  : 10
MP 1232.9  : 10
MP 1232.10 : 10
MP 1232.11 : 10
MP 1232.12 : 10
MP 1232.13 : 10
             ;--------------------------------------------------------------
             ;MP1233 Jerk at corners for "advanced HSC filter"
             ;Input: 0.1 to 1000.0 [m/s^3]
MP 1233.0  : 30
MP 1233.1  : 30
MP 1233.2  : 30
MP 1233.3  : 30
MP 1233.4  : 10
MP 1233.5  : 10
MP 1233.6  : 10
MP 1233.7  : 10
MP 1233.8  : 10
MP 1233.9  : 10
MP 1233.10 : 10
MP 1233.11 : 10
MP 1233.12 : 10
MP 1233.13 : 10
             ;--------------------------------------------------------------
             ;MP1240 Jerk at changes of curvature for single filter
             ;Input: 0.1 to 1000.0 [m/s^3]
MP 1240.0  : 1000
MP 1240.1  : 1000
MP 1240.2  : 1000
MP 1240.3  : 1000
MP 1240.4  : 10
MP 1240.5  : 10
MP 1240.6  : 10
MP 1240.7  : 10
MP 1240.8  : 10
MP 1240.9  : 10
MP 1240.10 : 10
MP 1240.11 : 10
MP 1240.12 : 10
MP 1240.13 : 10
             ;--------------------------------------------------------------
             ;MP1241 Jerk at changes of curvature for double filter
             ;Input: 0.1 to 1000.0 [m/s^3]
MP 1241.0  : 1000
MP 1241.1  : 1000
MP 1241.2  : 1000
MP 1241.3  : 1000
MP 1241.4  : 10
MP 1241.5  : 10
MP 1241.6  : 10
MP 1241.7  : 10
MP 1241.8  : 10
MP 1241.9  : 10
MP 1241.10 : 10
MP 1241.11 : 10
MP 1241.12 : 10
MP 1241.13 : 10
             ;--------------------------------------------------------------
             ;MP1242 Jerk at changes of curvature for HSC filter
             ;Input: 0.1 to 1000.0 [m/s^3]
MP 1242.0  : 30
MP 1242.1  : 30
MP 1242.2  : 30
MP 1242.3  : 30
MP 1242.4  : 10
MP 1242.5  : 10
MP 1242.6  : 10
MP 1242.7  : 10
MP 1242.8  : 10
MP 1242.9  : 10
MP 1242.10 : 10
MP 1242.11 : 10
MP 1242.12 : 10
MP 1242.13 : 10
             ;--------------------------------------------------------------
             ;MP1243 Jerk at changes of curvature for "advanced HSC filter"
             ;Input: 0.1 to 1000.0 [m/s^3]
MP 1243.0  : 30
MP 1243.1  : 30
MP 1243.2  : 30
MP 1243.3  : 30
MP 1243.4  : 10
MP 1243.5  : 10
MP 1243.6  : 10
MP 1243.7  : 10
MP 1243.8  : 10
MP 1243.9  : 10
MP 1243.10 : 10
MP 1243.11 : 10
MP 1243.12 : 10
MP 1243.13 : 10
             ;--------------------------------------------------------------
             ;MP1250 Factor for Jerk at corners/change of curvature at
             ;changes between rapid traverses
             ;Input: 0.0000 to +30.0000
MP 1250.0  : 1
MP 1250.1  : 1
MP 1250.2  : 1
MP 1250.3  : 1
MP 1250.4  : 1
MP 1250.5  : 1
MP 1250.6  : 1
MP 1250.7  : 1
MP 1250.8  : 1
MP 1250.9  : 1
MP 1250.10 : 1
MP 1250.11 : 1
MP 1250.12 : 1
MP 1250.13 : 1
             ;--------------------------------------------------------------
             ;MP1262 Minimum filter order for "HSC filter"
             ;Input: 0 to 31
MP 1262    : 31
             ;--------------------------------------------------------------
             ;MP1263 Minimum filter order for "Advanced HSC filter"
             ;Input: 0 to 31
MP 1263    : 31
             ;--------------------------------------------------------------
             ;MP1290 Maximum angle tolerance for
             ;"DCM - Dynamic Collison Monitoring"
             ;Input: 0.0000 to 3.0000 [°]
             ;Default: 3 [°]
MP 1290    : 3
             ;--------------------------------------------------------------
             ;MP1292 Allowance for collision monitored objects at active
             ;"DCM - Dynamic Collision Monitoring" (with M128 or TCPM)
             ;Input: 0 to 1000 [mm]
MP 1292    : 0
             ;--------------------------------------------------------------
             ;MP1294 Higher traversing speed with DCM
             ;       Only single axis movement allowed in Manual Operation
             ;       and El. Handwheel modes
             ;Input: 0 = Inactive
             ;       1 = Active
MP 1294    : 0
MP 1295    : 0
             ;--------------------------------------------------------------
             ;MP1320 Direction for traversing the reference marks
             ;Input: %987654321 bit-encoded
             ; 0 = Positive
             ; 1 = Negative
             ;.............987654ZYX <- axis
             ;221111111111----------
             ;1098765432109876543210 <- bit #
MP 1320    : %0000000000000000000001
             ;--------------------------------------------------------------
             ;MP1330 Velocity for traversing the reference marks
             ;Input: 80 to 1 000 000 [mm/min]
MP 1330.0  : 5000
MP 1330.1  : 5000
MP 1330.2  : 5000
MP 1330.3  : 2000
MP 1330.4  : 10000
MP 1330.5  : 10000
MP 1330.6  : 10000
MP 1330.7  : 10000
MP 1330.8  : 10000
MP 1330.9  : 10000
MP 1330.10 : 10000
MP 1330.11 : 10000
MP 1330.12 : 10000
MP 1330.13 : 10000
             ;--------------------------------------------------------------
             ;MP1331 Velocity for leaving the reference mark end position
             ;(only for rotary encoders MP1350=2)
             ;Input: 80 to 1 000 000 [mm/min]
MP 1331.0  : 500
MP 1331.1  : 500
MP 1331.2  : 500
MP 1331.3  : 500
MP 1331.4  : 500
MP 1331.5  : 500
MP 1331.6  : 500
MP 1331.7  : 500
MP 1331.8  : 500
MP 1331.9  : 500
MP 1331.10 : 500
MP 1331.11 : 500
MP 1331.12 : 500
MP 1331.13 : 500
             ;--------------------------------------------------------------
             ;MP1340 Sequence for traversing the reference points
             ;Input: 0     = No evaluation of the reference mark
             ;       1..14 = Axes 1..14
MP 1340.0  : 3
MP 1340.1  : 2
MP 1340.2  : 1
MP 1340.3  : 4
MP 1340.4  : 5
MP 1340.5  : 6
MP 1340.6  : 7
MP 1340.7  : 8
MP 1340.8  : 9
MP 1340.9  : 10
MP 1340.10 : 11
MP 1340.11 : 12
MP 1340.12 : 13
MP 1340.13 : 14
             ;--------------------------------------------------------------
             ;MP1350 Type of reference-mark traverse
             ;Input: 0 = Position encoder with distance-coded reference marks
             ;       1 = Position encoder with one reference mark
             ;       2 = Special sequence (linear measurement via ROD)
             ;       3 = Position encoder with distance-coded reference marks
             ;           (reference run outside of trip dog for reference end
             ;           position)
             ;       4 = Same as 3 except that two additional reference marks
             ;           are evaluated
             ;       5 = Encoder with EnDat interface
             ;       6 = Reference run via fast PLC input from MP1360.x
             ;       7 = Speed encoder with EnDat interface, connected
             ;           to the position and speed encoder input
             ;       8 = Encoder with EnDat2.2 interface and
             ;           incremental track
MP 1350.0  : 3          ;w/ LS/GOP scale=3, w/ LC scale/ROQ=5, w/o scale=2
MP 1350.1  : 3          ;w/ LS/GOP scale=3, w/ LC scale/ROQ=5, w/o scale=2
MP 1350.2  : 3          ;w/ LS/GOP scale=3, w/ LC scale/ROQ=5, w/o scale=2
MP 1350.3  : 2
MP 1350.4  : 2
MP 1350.5  : 1
MP 1350.6  : 1
MP 1350.7  : 1
MP 1350.8  : 1
MP 1350.9  : 1
MP 1350.10 : 1
MP 1350.11 : 1
MP 1350.12 : 1
MP 1350.13 : 1
             ;--------------------------------------------------------------
             ;MP1352 Axis for which the monitoring of the software limit
             ;       switches can be activated before the reference marks
             ;       are passed. (Last saved actual position,
             ;       activation via softkey)
             ;Input: 0 = Monitoring can not be activated
             ;       1 = Monitoring can be activated
             ;.............987654ZYX <- axis
             ;221111111111----------
             ;1098765432109876543210 <- bit #
MP 1352    : %0000000000000000011111
             ;--------------------------------------------------------------
             ;MP1355 Reference run
             ;Input: 0 = Reference run as defined in MP1350.x
             ;       1 = Reference run via EnDat interface
             ;           of speed encoder (distance from position
             ;           encoder in MP1356.x), repeated reference
             ;           run over position encoder at a later date
             ;.............987654ZYX <- axis
             ;221111111111----------
             ;1098765432109876543210 <- bit #
MP 1355    : %0000000000000000000000
             ;--------------------------------------------------------------
             ;MP1356 Distance between speed encoder and position encoder if
             ;       MP1355 = 1
             ;Input: -1.79769313486E+308 to +1.79769313486E+308 [mm] or [deg]
MP 1356.0  : +0.00000
MP 1356.1  : +0.00000
MP 1356.2  : +0.00000
MP 1356.3  : +0.00000
MP 1356.4  : +0.00000
MP 1356.5  : +0.00000
MP 1356.6  : +0.00000
MP 1356.7  : +0.00000
MP 1356.8  : +0.00000
MP 1356.9  : +0.00000
MP 1356.10 : +0.00000
MP 1356.11 : +0.00000
MP 1356.12 : +0.00000
MP 1356.13 : +0.00000
             ;--------------------------------------------------------------
             ;MP1357 W1032 at dual reference run
             ;Input: 0 = Reset, if the reference run via the EnDat
             ;           interface of the speed encoder is done
             ;       1 = Reset, if the reference mark of the position
             ;           encoder is crossed
             ;.............987654ZYX <- axis
             ;221111111111----------
             ;1098765432109876543210 <- bit #
MP 1357    : %0000000000000000000000
             ;--------------------------------------------------------------
             ;MP1358 Readout multiturn information
             ;Input: 0 = Do not readout multiturn information
             ;       1 = Readout multiturn information
             ;.............987654ZYX <- axis
             ;221111111111----------
             ;1098765432109876543210 <- bit #
MP 1358    : %0000000000000000000000
             ;--------------------------------------------------------------
             ;MP1360 Number of fast PLC input for reference run
             ;Input: 0    = No reference run via fast input
             ;       1..5 = Number of fast PLC input
MP 1360.0  : 0
MP 1360.1  : 0
MP 1360.2  : 0
MP 1360.3  : 0
MP 1360.4  : 0
MP 1360.5  : 0
MP 1360.6  : 0
MP 1360.7  : 0
MP 1360.8  : 0
MP 1360.9  : 0
MP 1360.10 : 0
MP 1360.11 : 0
MP 1360.12 : 0
MP 1360.13 : 0
             ;--------------------------------------------------------------
             ;MP1391.0 Velocity feedforward control in the operating modes
             ;       Manual and Handwheel
             ;Input: %xxxxxxxxxxxxxx
             ;       0 = Operation with following error (lag)
             ;       1 = Operation with velocity feedforward control
             ;.............987654ZYX <- axis
             ;221111111111----------
             ;1098765432109876543210 <- bit #
MP 1391.0  : %1111111111111111111111
             ;--------------------------------------------------------------
             ;MP1391.1 Acceleration feedforward in the operating modes
             ;       Manual and Handwheel
             ;Input: %xxxxxxxxxxxxxx
             ;         0 = Acceleration feedforward control inactive
             ;         1 = Acceleration feedforward control active
             ;.............987654ZYX <- axis
             ;221111111111----------
             ;1098765432109876543210 <- bit #
MP 1391.1  : %0000000000000000000000
             ;--------------------------------------------------------------
             ;MP1392 Velocity feedforward control in the operating modes
             ;       Positioning with Manual Data Input,
             ;       Program Run Single Block and Program Run Full Sequence
             ;Input: %xxxxxxxxxxxxxx
             ;       0 = Operation with following error (lag)
             ;       1 = Operation with velocity feedforward control
             ;.............987654ZYX <- axis
             ;221111111111----------
             ;1098765432109876543210 <- bit #
MP 1392    : %1111111111111111111111
             ;--------------------------------------------------------------
             ;MP1396 Feedback control with velocity semifeedforward
             ;Input: 0.001 to 0.999: Factor for velocity semifeedforward
             ;         1: Velocity feedforward control
MP 1396.0  : 1
MP 1396.1  : 1
MP 1396.2  : 1
MP 1396.3  : 1
MP 1396.4  : 1
MP 1396.5  : 1
MP 1396.6  : 1
MP 1396.7  : 1
MP 1396.8  : 1
MP 1396.9  : 1
MP 1396.10 : 1
MP 1396.11 : 1
MP 1396.12 : 1
MP 1396.13 : 1
             ;==============================================================
             ;MP 1400 .. 1699
             ;Operation with velocity feedforward control
             ;==============================================================
             ;MP1410 Position monitoring during operation with
             ;velocity feedforward control
             ;Erasable (positioning error x)
             ;Input: 0.0010 to 30.0000 [mm]
MP 1410.0  : 0.5
MP 1410.1  : 0.5
MP 1410.2  : 0.5
MP 1410.3  : 0.5
MP 1410.4  : 0.5
MP 1410.5  : 0.5
MP 1410.6  : 0.5
MP 1410.7  : 0.5
MP 1410.8  : 0.5
MP 1410.9  : 0.5
MP 1410.10 : 0.5
MP 1410.11 : 0.5
MP 1410.12 : 0.5
MP 1410.13 : 0.5
             ;--------------------------------------------------------------
             ;MP1420 Position monitoring during operation with
             ;velocity feedforward control
             ;EMERGENCY STOP (gross positioning error x A)
             ;Input: 0.0010 to 30.0000 [mm]
MP 1420.0  : 2
MP 1420.1  : 2
MP 1420.2  : 2
MP 1420.3  : 2
MP 1420.4  : 2
MP 1420.5  : 2
MP 1420.6  : 2
MP 1420.7  : 2
MP 1420.8  : 2
MP 1420.9  : 2
MP 1420.10 : 2
MP 1420.11 : 2
MP 1420.12 : 2
MP 1420.13 : 2
             ;--------------------------------------------------------------
             ;MP1510 kv factor for velocity feedforward control
             ;Input: 0.100 to 1000.000 [m/(min*mm)]
MP 1510.0  : 4
MP 1510.1  : 4
MP 1510.2  : 4
MP 1510.3  : 1
MP 1510.4  : 1
MP 1510.5  : 1
MP 1510.6  : 1
MP 1510.7  : 1
MP 1510.8  : 1
MP 1510.9  : 1
MP 1510.10 : 1
MP 1510.11 : 1
MP 1510.12 : 1
MP 1510.13 : 1
             ;--------------------------------------------------------------
             ;MP1511 Factor for static friction compensation
             ;Input: 0 to 16 777 215
             ;Static friction compensation during operation with
             ;velocity feedforward control
             ;Derivative component for low feed rates, time limitation
MP 1511.0  : 0
MP 1511.1  : 0
MP 1511.2  : 0
MP 1511.3  : 0
MP 1511.4  : 0
MP 1511.5  : 0
MP 1511.6  : 0
MP 1511.7  : 0
MP 1511.8  : 0
MP 1511.9  : 0
MP 1511.10 : 0
MP 1511.11 : 0
MP 1511.12 : 0
MP 1511.13 : 0
             ;--------------------------------------------------------------
             ;MP1512 Limitation of the amount of the static friction
             ;compensation
             ;(Limitation of derivative component of counting steps)
             ;Input: 0 to 16 777 215 [counting steps]
MP 1512.0  : 0
MP 1512.1  : 0
MP 1512.2  : 0
MP 1512.3  : 0
MP 1512.4  : 0
MP 1512.5  : 0
MP 1512.6  : 0
MP 1512.7  : 0
MP 1512.8  : 0
MP 1512.9  : 0
MP 1512.10 : 0
MP 1512.11 : 0
MP 1512.12 : 0
MP 1512.13 : 0
             ;--------------------------------------------------------------
             ;MP1513 Feed-rate limitation for static friction compensation
             ;Input: 0 to 300 000 [mm/min]
MP 1513.0  : 0
MP 1513.1  : 0
MP 1513.2  : 0
MP 1513.3  : 0
MP 1513.4  : 0
MP 1513.5  : 0
MP 1513.6  : 0
MP 1513.7  : 0
MP 1513.8  : 0
MP 1513.9  : 0
MP 1513.10 : 0
MP 1513.11 : 0
MP 1513.12 : 0
MP 1513.13 : 0
             ;--------------------------------------------------------------
             ;MP1515 kv factor for velocity feedforward control (M105)
             ;Input: 0.100 to 1000.000 [m/(min*mm)]
             ;Can be switched on/off via M105, M106
MP 1515.0  : 2.8
MP 1515.1  : 2.8
MP 1515.2  : 4
MP 1515.3  : 3
MP 1515.4  : 3
MP 1515.5  : 3
MP 1515.6  : 3
MP 1515.7  : 3
MP 1515.8  : 3
MP 1515.9  : 3
MP 1515.10 : 3
MP 1515.11 : 3
MP 1515.12 : 3
MP 1515.13 : 3
             ;--------------------------------------------------------------
             ;MP1516 kv factor for velocity semifeedforward control
             ;Input: 0.100 to 20.000 [(m/min)/mm]
MP 1516.0  : 1
MP 1516.1  : 1
MP 1516.2  : 1
MP 1516.3  : 1
MP 1516.4  : 1
MP 1516.5  : 1
MP 1516.6  : 1
MP 1516.7  : 1
MP 1516.8  : 1
MP 1516.9  : 1
MP 1516.10 : 1
MP 1516.11 : 1
MP 1516.12 : 1
MP 1516.13 : 1
             ;--------------------------------------------------------------
             ;MP1521 Transient response during acceleration and deceleration
             ;Input: 0 to 255 [ms]
MP 1521    : 0
             ;--------------------------------------------------------------
             ;MP1522 Speed smoothing
             ;Recommended only for machines natural frequency Fma <=25Hz
             ;initial value = 1/(2*Fma), increase in steps of 5ms while lag
             ;error is decreasing
             ;Input: 0 to 60 [ms]
MP 1522    : 15
             ;==============================================================
             ;MP 1700 .. 1899
             ;Operation with following error (servo lag)
             ;==============================================================
             ;MP1710 Position monitoring for operation with following error
             ;Erasable (positioning error x)
             ;Input: 0.0000 to 300.0000 [mm]
MP 1710.0  : 18
MP 1710.1  : 18
MP 1710.2  : 14.4
MP 1710.3  : 2.4
MP 1710.4  : 12
MP 1710.5  : 12
MP 1710.6  : 12
MP 1710.7  : 12
MP 1710.8  : 12
MP 1710.9  : 12
MP 1710.10 : 12
MP 1710.11 : 12
MP 1710.12 : 12
MP 1710.13 : 12
             ;--------------------------------------------------------------
             ;MP1720 Position monitoring for operation with following error
             ;EMERGENCY STOP (gross positioning error x A)
             ;Input: 0.0000 to 300.0000 [mm]
MP 1720.0  : 21
MP 1720.1  : 21
MP 1720.2  : 16.8
MP 1720.3  : 2.8
MP 1720.4  : 14
MP 1720.5  : 14
MP 1720.6  : 14
MP 1720.7  : 14
MP 1720.8  : 14
MP 1720.9  : 14
MP 1720.10 : 14
MP 1720.11 : 14
MP 1720.12 : 14
MP 1720.13 : 14
             ;--------------------------------------------------------------
             ;MP1810 kv factor for operation with following error
             ;Input: 0.100 to 20.000 [m/(min*mm)]
MP 1810.0  : 2
MP 1810.1  : 2
MP 1810.2  : 2
MP 1810.3  : 2
MP 1810.4  : 2
MP 1810.5  : 1
MP 1810.6  : 1
MP 1810.7  : 1
MP 1810.8  : 1
MP 1810.9  : 1
MP 1810.10 : 1
MP 1810.11 : 1
MP 1810.12 : 1
MP 1810.13 : 1
             ;--------------------------------------------------------------
             ;MP1815 kv factor for operation with following error
             ;Effective after M105
             ;Input: 0.100 to 20.000 [m/(min*mm)]
             ;Can be switched on/off via M105, M106
MP 1815.0  : 3
MP 1815.1  : 3
MP 1815.2  : 3
MP 1815.3  : 3
MP 1815.4  : 3
MP 1815.5  : 3
MP 1815.6  : 3
MP 1815.7  : 3
MP 1815.8  : 3
MP 1815.9  : 3
MP 1815.10 : 3
MP 1815.11 : 3
MP 1815.12 : 3
MP 1815.13 : 3
             ;--------------------------------------------------------------
             ;MP1820 Multiplication factor for the kv factor
             ;Following error mode: Multiplication factor for kv after
             ;characteristic curve kink point
             ;Input: 0.001 to 1.000
MP 1820.0  : 1
MP 1820.1  : 1
MP 1820.2  : 1
MP 1820.3  : 1
MP 1820.4  : 1
MP 1820.5  : 1
MP 1820.6  : 1
MP 1820.7  : 1
MP 1820.8  : 1
MP 1820.9  : 1
MP 1820.10 : 1
MP 1820.11 : 1
MP 1820.12 : 1
MP 1820.13 : 1
             ;--------------------------------------------------------------
             ;MP1830 Characteristic curve kink point
             ;Characteristic curve kink point (in % of rapid traverse)
             ;Input: 0.000 to 100.000 [%]
MP 1830.0  : 100
MP 1830.1  : 100
MP 1830.2  : 100
MP 1830.3  : 100
MP 1830.4  : 100
MP 1830.5  : 100
MP 1830.6  : 100
MP 1830.7  : 100
MP 1830.8  : 100
MP 1830.9  : 100
MP 1830.10 : 100
MP 1830.11 : 100
MP 1830.12 : 100
MP 1830.13 : 100
             ;==============================================================
             ;MP 2000 .. 2999
             ;Current control with PWM interface
             ;Integrated speed and current control
             ;
             ;==============================================================
             ;MP2000 Axis specific switching from single speed axis to
             ;       double speed axis (only CC 6xxx, SW-Option #49)
             ;Input: 0 = single speed axis
             ;       1 = double speed axis
MP 2000.0  : 0
MP 2000.1  : 0
MP 2000.2  : 0
MP 2000.3  : 0
MP 2000.4  : 0
MP 2000.5  : 0
MP 2000.6  : 0
MP 2000.7  : 0
MP 2000.8  : 0
MP 2000.9  : 0
MP 2000.10 : 0
MP 2000.11 : 0
MP 2000.12 : 0
MP 2000.13 : 0
MP 2000.14 : 0
MP 2000.15 : 0
             ;--------------------------------------------------------------
             ;MP2040 Axis groups for which drive release via
             ;X150 (PWM output X51..X56) / X151 (PWM output X57..X64)
             ;can be switched off
             ;W524 Bit0=1 - switch off via X150/X151 active
             ;Input: bit 0 - bit 31 coded (spindle on bit #15 & #31)
             ;-----------------111111---------
             ;S---------------S543210987654ZYX <- axis
             ;3322222222221111111111----------
             ;10987654321098765432109876543210 <- bit #
MP 2040.0  : %10000000000000001000000000011111;Axis group 1
MP 2040.1  : %00000000000000000000000000000000;Axis group 2
MP 2040.2  : %00000000000000000000000000000000;Axis group 3
MP 2040.3  : %00000000000000000000000000000000;Axis group 4
MP 2040.4  : %00000000000000000000000000000000;Axis group 5
MP 2040.5  : %00000000000000000000000000000000;Axis group 6
MP 2040.6  : %00000000000000000000000000000000;Axis group 7
MP 2040.7  : %00000000000000000000000000000000;Axis group 8
             ;--------------------------------------------------------------
             ;MP2100 Type of axis power modules
             ;Input:   Name from file <Motor.amp>
MP 2100.0  : HEIDENHAIN-UM121D
MP 2100.1  : HEIDENHAIN-UM121D
MP 2100.2  : HEIDENHAIN-UM112D
MP 2100.3  : HEIDENHAIN-UM111D
MP 2100.4  : HEIDENHAIN-UM111D
MP 2100.5  : 
MP 2100.6  : 
MP 2100.7  : 
MP 2100.8  : 
MP 2100.9  : 
MP 2100.10 : 
MP 2100.11 : 
MP 2100.12 : HEIDENHAIN-UM112D;1st spindle mode 0
MP 2100.13 : HEIDENHAIN-UM112D;2nd spindle mode 0
MP 2100.14 : HEIDENHAIN-UM112D;1st spindle mode 1
MP 2100.15 : HEIDENHAIN-UM112D;2nd spindle mode 1
             ;--------------------------------------------------------------
             ;MP2160 Field weakening for synchronous motors
             ;Input: 0 = Voltage-protection module not present
             ;       1 = Voltage-protection module present
             ;       2 = Limited field weakening without
             ;           voltage-protection module for EcoDyn motors
MP 2160.0  : 0
MP 2160.1  : 0
MP 2160.2  : 0
MP 2160.3  : 0
MP 2160.4  : 0
MP 2160.5  : 0
MP 2160.6  : 0
MP 2160.7  : 0
MP 2160.8  : 0
MP 2160.9  : 0
MP 2160.10 : 0
MP 2160.11 : 0
MP 2160.12 : 0
MP 2160.13 : 0
MP 2160.14 : 0
MP 2160.15 : 0
             ;--------------------------------------------------------------
             ;MP2170 Waiting period between switch-on of the drive and
             ;       the drive's readiness signal
             ;Input: 0 to 4.999 [s]
             ;       0 = 2 s
MP 2170    : 0
             ;--------------------------------------------------------------
             ;MP2172 Delay for removal of inverter ready signal after serious
             ;error messages (internal emergency stop) which are triggered
             ;by the control
             ;Input: 0 to 60 [s]
             ;       0 = 3 s (default value)
MP 2172    : 0
             ;--------------------------------------------------------------
             ;MP2173 Pulse switch-off of the power stage
             ;       Monitoring time for the braking process. If after
             ;       the time has expired the axes are still in motion,
             ;       the pulses of the power modules are switched off.
             ;       Time must be greater then the maximum eletrical
             ;       breaking time for the axis/spindel.
             ;       -Attention, refer to instructions in technical manual-
             ;Input: 0.200 to 100.000 [s]
             ;       0 = 3 s (default value)
MP 2173.0  : 0
MP 2173.1  : 0
MP 2173.2  : 0
MP 2173.3  : 0
MP 2173.4  : 0
MP 2173.5  : 0
MP 2173.6  : 0
MP 2173.7  : 0
MP 2173.8  : 0
MP 2173.9  : 0
MP 2173.10 : 0
MP 2173.11 : 0
MP 2173.12 : 0
MP 2173.13 : 0
MP 2173.14 : 0
MP 2173.15 : 0
             ;--------------------------------------------------------------
             ;MP2180 PWM frequency
             ;       On controller board only simple or double
             ;       PWM fundamental frequency
             ;       (e. g. 5000 Hz or 10000 Hz)
             ;Input: 0 = 5000 Hz
             ;       3200 to 3999 = 3333 Hz
             ;       4000 to 4999 = 4000 Hz
             ;       5000 to 6000 = 5000 Hz
             ;       6001 to 7999 = 6666 Hz
             ;       8000 to 9999 = 8000 Hz
             ;       10000 = 10000 Hz
MP 2180.0  : 4000
MP 2180.1  : 4000
MP 2180.2  : 4000
MP 2180.3  : 4000
MP 2180.4  : 4000
MP 2180.5  : 0
MP 2180.6  : 0
MP 2180.7  : 0
MP 2180.8  : 0
MP 2180.9  : 0
MP 2180.10 : 0
MP 2180.11 : 0
MP 2180.12 : 4000
MP 2180.13 : 4000
MP 2180.14 : 4000
MP 2180.15 : 4000
             ;--------------------------------------------------------------
             ;MP2182 Current controller cycle time
             ;       e.g. for spindle temperature problems,
             ;            Optimum utilization of the power stage current at
             ;            low spindle speeds
             ;Input: 0 = Standard case:
             ;           MP2180 = [3333 .. 5000] with single-speed axes
             ;           MP2180 = [3333 .. 10000] with double-speed axes
             ;           (current controller cycle time = 1 / (2 * MP2180))
             ;       1 = Special case 1, only CC424:
             ;           MP2180 = [6666 .. 10000] for high PWM frequencies
             ;           with single-speed axes
             ;           (current controller cycle time = 1 / MP2180)
             ;       2 = Special case 2, only CC424, CC61xx:
             ;           MP2180 = [3333 .. 5000] with speed-dependent
             ;           doubling (MP2186, MP2188) of the fundamental
             ;           PWM frequency from MP2180 with double-speed axes
             ;           (current controller cycle time = 1 / (4 * MP2180))
MP 2182.0  : 0
MP 2182.1  : 0
MP 2182.2  : 0
MP 2182.3  : 0
MP 2182.4  : 0
MP 2182.5  : 0
MP 2182.6  : 0
MP 2182.7  : 0
MP 2182.8  : 0
MP 2182.9  : 0
MP 2182.10 : 0
MP 2182.11 : 0
MP 2182.12 : 0
MP 2182.13 : 0
MP 2182.14 : 0
MP 2182.15 : 0
             ;--------------------------------------------------------------
             ;MP2186 Speed to switch from simple to double PWM fundamental
             ;       frequency. Use only with HEIDENHAIN power moduls.
             ;Input: 0 to 100000 [rpm]
MP 2186.0  : 0
MP 2186.1  : 0
MP 2186.2  : 0
MP 2186.3  : 0
MP 2186.4  : 0
MP 2186.5  : 0
MP 2186.6  : 0
MP 2186.7  : 0
MP 2186.8  : 0
MP 2186.9  : 0
MP 2186.10 : 0
MP 2186.11 : 0
MP 2186.12 : 0
MP 2186.13 : 0
MP 2186.14 : 0
MP 2186.15 : 0
             ;--------------------------------------------------------------
             ;MP2188 Speed to switch back from double to simple PWM
             ;       fundamental frequency. Necessary: MP2188.x < MP2186.x
             ;       Use only with HEIDENHAIN power moduls.
             ;Input: 0 to 100000 [rpm]
MP 2188.0  : 0
MP 2188.1  : 0
MP 2188.2  : 0
MP 2188.3  : 0
MP 2188.4  : 0
MP 2188.5  : 0
MP 2188.6  : 0
MP 2188.7  : 0
MP 2188.8  : 0
MP 2188.9  : 0
MP 2188.10 : 0
MP 2188.11 : 0
MP 2188.12 : 0
MP 2188.13 : 0
MP 2188.14 : 0
MP 2188.15 : 0
             ;--------------------------------------------------------------
             ;MP2190 DC-link voltage of inverter
             ;       HEIDENHAIN inverter regenerative: 650 V
             ;       HEIDENHAIN inverter non-regenerative: 565 V
             ;Input: 0 to 3000 [V]
             ;       * = Value of table SUPPLY.SPY
MP 2190.0  : *
MP 2190.1  : *
MP 2190.2  : *
MP 2190.3  : *
MP 2190.4  : *
             ;--------------------------------------------------------------
             ;MP2195 Suppress the error messages from the
             ;       HEIDENHAIN power supply units
             ;Input: %xxxxxxxx
             ;Bit 0   Reserved
             ;Bit 1   "-ERR.UZ.GR" signal
             ;        0 = Error message is not suppressed
             ;        1 = Error message is suppressed
             ;Bit 2   "-ERR.TEMP" signal
             ;        0 = Error message is not suppressed
             ;        1 = Error message is suppressed
             ;Bit 3   Reserved
             ;Bit 4   "-ERR.IZ.GR" signal
             ;        0 = Error message is not suppressed
             ;        1 = Error message is suppressed
             ;Bit 5   "RDY.PS" signal
             ;        0 = Error message is not suppressed
             ;        1 = Error message is suppressed
             ;Bit 6   "-ERR.ILEAK" signal
             ;        0 = Error message is not suppressed
             ;        1 = Error message is suppressed
             ;Bit 7   with CC 424: "-PF.PS.AC" signal
             ;        0 = Error message is not suppressed, LIFTOFF active
             ;        1 = Error message is suppressed, LIFTOFF inactive
             ;Bit 8   with CC 61xx: "-PF.PS.DC" signal
             ;        0 = Error message is not suppressed
             ;        1 = Error message is suppressed
             ;Bit 9   with CC 61xx: Monitoring of dc-link voltage
             ;        0 = Monitoring of dc-link voltage, LIFTOFF active
             ;        1 = Monitoring of dc-link voltage, LIFTOFF inactive
             ;Bit 10  with CC 61xx: Power monitoring of UV(R)
             ;        0 = Power monitoring active
             ;        1 = Power monitoring inactive
             ;1----------
             ;09876543210 <- bit #
MP 2195    : %00000000000
             ;--------------------------------------------------------------
             ;MP2196 Identifier of power supply unit in MP2198.x
             ;Input:   P, Q, R or T
MP 2196    : 
             ;--------------------------------------------------------------
             ;MP2198 Type of power supply unit
             ;Input:   Name from file <Supply.spy>
MP 2198.0  : UVR130D
MP 2198.1  : 
MP 2198.2  : 
MP 2198.3  : 
MP 2198.4  : 
             ;--------------------------------------------------------------
             ;MP2199 Assignment of axis to power supply unit (MP2198.x)
             ;Input:   0 (= MP2198.0) or 1 (= MP2198.1)
MP 2199.0  : 0
MP 2199.1  : 0
MP 2199.2  : 0
MP 2199.3  : 0
MP 2199.4  : 0
MP 2199.5  : 0
MP 2199.6  : 0
MP 2199.7  : 0
MP 2199.8  : 0
MP 2199.9  : 0
MP 2199.10 : 0
MP 2199.11 : 0
MP 2199.12 : 0
MP 2199.13 : 0
MP 2199.14 : 0
MP 2199.15 : 0
             ;--------------------------------------------------------------
             ;MP2200 Type of axis motors
             ;Input:   Name from file <Motor.mot>
MP 2200.0  : 1FK7100-5AF7
MP 2200.1  : 1FK7100-5AF7
MP 2200.2  : 1FK7103-5AF7
MP 2200.3  : 1FK7083-5AF7
MP 2200.4  : 1FK7083-5AF7
MP 2200.5  : 
MP 2200.6  : 
MP 2200.7  : 
MP 2200.8  : 
MP 2200.9  : 
MP 2200.10 : 
MP 2200.11 : 
MP 2200.12 : 1PH7133-2ND0
MP 2200.13 : 1PH7133-2ND0
MP 2200.14 : 1PH7133-2ND0
MP 2200.15 : 1PH7133-2ND0
             ;--------------------------------------------------------------
             ;MP2202 Overwrite "Line count" from the motor table
             ;Input: * = Input from the motor table active
             ;       0 = No speed encoder (volts-per-hertz control mode)
MP 2202.0  : *
MP 2202.1  : *
MP 2202.2  : *
MP 2202.3  : *
MP 2202.4  : *
MP 2202.5  : *
MP 2202.6  : *
MP 2202.7  : *
MP 2202.8  : *
MP 2202.9  : *
MP 2202.10 : *
MP 2202.11 : *
MP 2202.12 : *
MP 2202.13 : *
MP 2202.14 : *
MP 2202.15 : *
             ;--------------------------------------------------------------
             ;MP2204 Overwrite "Counting direction" from the motor table
             ;Input: * = Input from the motor table active
             ;       + = Positive
             ;       - = Negative
MP 2204.0  : *
MP 2204.1  : *
MP 2204.2  : *
MP 2204.3  : *
MP 2204.4  : *
MP 2204.5  : *
MP 2204.6  : *
MP 2204.7  : *
MP 2204.8  : *
MP 2204.9  : *
MP 2204.10 : *
MP 2204.11 : *
MP 2204.12 : *
MP 2204.13 : *
MP 2204.14 : *
MP 2204.15 : *
             ;--------------------------------------------------------------
             ;MP2206 Overwrite "Type of encoder" from the motor table
             ;Input: * = Input from the motor table active
             ;       0 = No speed encoder (volts-per-hertz control mode)
             ;       1 = Incremental rotary encoder with Z1 track
             ;       2 = (Aligned) absolute rotary encoder
             ;       3 = Absolute linear encoder
             ;       4 = reserved
             ;       5 = (Non-aligned) absolute rotary encoder
             ;       6 = Incremental rotary encoder without Z1 track
             ;       7 = (Non-aligned) incremental rotary encoder
             ;           with distance-coded reference marks
             ;       8 = (Non-aligned) linear encoder with
             ;           distance-coded reference marks
             ;       9 = (Aligned) rotary encoder with
             ;           EnDat 2.2 interface
             ;       10= (Non-aligned) rotary encoder with
             ;           EnDat 2.2 interface
             ;       11= Linear encoder with EnDat 2.2 interface
             ;       12= EnDat2.2 incremental, position measuring system
             ;       13 to 18: Reserved
             ;       19= EnDat2.2 incremental, speed measuring system
MP 2206.0  : *
MP 2206.1  : *
MP 2206.2  : *
MP 2206.3  : *
MP 2206.4  : *
MP 2206.5  : *
MP 2206.6  : *
MP 2206.7  : *
MP 2206.8  : *
MP 2206.9  : *
MP 2206.10 : *
MP 2206.11 : *
MP 2206.12 : *
MP 2206.13 : *
MP 2206.14 : *
MP 2206.15 : *
             ;--------------------------------------------------------------
             ;MP2208 Overwrite "Series reactor L in [uH]"
             ;       from the motor table
             ;Input: * = Input from the motor table active
             ;       Value Series reactor in [uH]
MP 2208.0  : *
MP 2208.1  : *
MP 2208.2  : *
MP 2208.3  : *
MP 2208.4  : *
MP 2208.5  : *
MP 2208.6  : *
MP 2208.7  : *
MP 2208.8  : *
MP 2208.9  : *
MP 2208.10 : *
MP 2208.11 : *
MP 2208.12 : *
MP 2208.13 : *
MP 2208.14 : *
MP 2208.15 : *
             ;--------------------------------------------------------------
             ;MP2209 Overwrite "Mass moment of inertia in [kg*m^2]"
             ;       from the motor table
             ;Input: * = Input from the motor table active
             ;       Value mass moment of inertia in [kg*m^2]
MP 2209.0  : *
MP 2209.1  : *
MP 2209.2  : *
MP 2209.3  : *
MP 2209.4  : *
MP 2209.5  : *
MP 2209.6  : *
MP 2209.7  : *
MP 2209.8  : *
MP 2209.9  : *
MP 2209.10 : *
MP 2209.11 : *
MP 2209.12 : *
MP 2209.13 : *
MP 2209.14 : *
MP 2209.15 : *
             ;--------------------------------------------------------------
             ;MP2210 Factor for reduction of the magnetisation current
             ;Input: 0 to 99 [%]
             ;       0 = Function inactive
             ;
MP 2210.0  : 0
MP 2210.1  : 0
MP 2210.2  : 0
MP 2210.3  : 0
MP 2210.4  : 0
MP 2210.5  : 0
MP 2210.6  : 0
MP 2210.7  : 0
MP 2210.8  : 0
MP 2210.9  : 0
MP 2210.10 : 0
MP 2210.11 : 0
MP 2210.12 : 0
MP 2210.13 : 0
MP 2210.14 : 0
MP 2210.15 : 0
             ;--------------------------------------------------------------
             ;MP2211 Reserved
MP 2211.0  : 0
MP 2211.1  : 0
MP 2211.2  : 0
MP 2211.3  : 0
MP 2211.4  : 0
MP 2211.5  : 0
MP 2211.6  : 0
MP 2211.7  : 0
MP 2211.8  : 0
MP 2211.9  : 0
MP 2211.10 : 0
MP 2211.11 : 0
MP 2211.12 : 0
MP 2211.13 : 0
MP 2211.14 : 0
MP 2211.15 : 0
             ;--------------------------------------------------------------
             ;MP2220 Monitoring functions
             ;Input: %xxxxxxxxxxxxxxxx
             ;Bit 0   Reference-mark monitoring
             ;        0 = Active
             ;        1 = Inactive
             ;Bit 1   Monitoring of rotational direction
             ;        0 = Active
             ;        1 = Inactive
             ;Bit 2   Power limiting of the spindle for -ERR.IZ.GR
             ;        (only for HEIDENHAIN inverters except UE 2xx)
             ;        0 = Inactive
             ;        1 = Active
             ;Bit 3   Vibrations are suppressed during switch-off of drive
             ;        0 = Active
             ;        1 = Inactive
             ;Bit 4   Monitoring temperature too high
             ;        0 = Active
             ;        1 = Inactive
             ;Bit 5   Monitoring temperature too low
             ;        0 = Active
             ;        1 = Inactive
             ;Bit 6   reserved
             ;Bit 7   Monitoring input frequency encoder
             ;        0 = Active
             ;        1 = Inactive
             ;Bit 8   Compensating the mechanical offset
             ;        by a step by step increasing of the kV
             ;        0 = Active
             ;        1 = Inactive
             ;Bit 9   Reserved
             ;Bit 10  Reserved
             ;Bit 11  Reserved
             ;Bit 12  Reserved
             ;Bit 13  Reserved
             ;Bit 14  Reserved
             ;Bit 15  Reserved
             ;111111----------
             ;5432109876543210 <- bit #
MP 2220.0  : %0000000000000000
MP 2220.1  : %0000000000000000
MP 2220.2  : %0000000000000000
MP 2220.3  : %0000000000000000
MP 2220.4  : %0000000000000000
MP 2220.5  : %0000000000000000
MP 2220.6  : %0000000000000000
MP 2220.7  : %0000000000000000
MP 2220.8  : %0000000000000000
MP 2220.9  : %0000000000000000
MP 2220.10 : %0000000000000000
MP 2220.11 : %0000000000000000
MP 2220.12 : %0000000000000000
MP 2220.13 : %0000000000000000
MP 2220.14 : %0000000000000000
MP 2220.15 : %0000000000000000
             ;--------------------------------------------------------------
             ;MP2221 reserved
             ;111111----------
             ;5432109876543210 <- bit #
MP 2221.0  : %0000000000000000
MP 2221.1  : %0000000000000000
MP 2221.2  : %0000000000000000
MP 2221.3  : %0000000000000000
MP 2221.4  : %0000000000000000
MP 2221.5  : %0000000000000000
MP 2221.6  : %0000000000000000
MP 2221.7  : %0000000000000000
MP 2221.8  : %0000000000000000
MP 2221.9  : %0000000000000000
MP 2221.10 : %0000000000000000
MP 2221.11 : %0000000000000000
MP 2221.12 : %0000000000000000
MP 2221.13 : %0000000000000000
MP 2221.14 : %0000000000000000
MP 2221.15 : %0000000000000000
             ;--------------------------------------------------------------
             ;MP2222 Special control functions
             ;Input: %xxxxxxxxxxxxxxxx
             ;Bit 0   Reserved
             ;Bit 1   Reserved
             ;Bit 2   Reserved
             ;Bit 3   Reserved
             ;Bit 4   Reserved
             ;Bit 5   Reserved
             ;Bit 6   Reserved
             ;Bit 7   Reserved
             ;Bit 8   Reserved
             ;Bit 9   Reserved
             ;Bit 10  CC 61xx:
             ;        Optimized function for wye/delta switchover
             ;        with asynchronous motors: activation of inverter and
             ;        current controller at the same time
             ;        0 = Inactive
             ;        1 = Active
             ;Bit 11  CC 61xx:
             ;        Special function for wye/delta switchover
             ;        with asynchronous motors: demagnetization of motor
             ;        at switch-off
             ;        0 = Inactive
             ;        1 = Active
             ;Bit 12  Reserved
             ;Bit 13  Reserved
             ;Bit 14  Reserved
             ;Bit 15  Reserved
             ;111111----------
             ;5432109876543210 <- bit #
MP 2222.0  : %0000000000000000
MP 2222.1  : %0000000000000000
MP 2222.2  : %0000000000000000
MP 2222.3  : %0000000000000000
MP 2222.4  : %0000000000000000
MP 2222.5  : %0000000000000000
MP 2222.6  : %0000000000000000
MP 2222.7  : %0000000000000000
MP 2222.8  : %0000000000000000
MP 2222.9  : %0000000000000000
MP 2222.10 : %0000000000000000
MP 2222.11 : %0000000000000000
MP 2222.12 : %0000000000000000
MP 2222.13 : %0000000000000000
MP 2222.14 : %0000000000000000
MP 2222.15 : %0000000000000000
             ;--------------------------------------------------------------
             ;MP2223 Special control functions
             ;Input: %xxxxxxxxxxxxxxxx
             ;Bit 0   Reserved
             ;Bit 1   CC 61xx:
             ;        Automatic including of holding current in MP2630
             ;        by using PLC module 9128 and 9158
             ;        0 = Inactive
             ;        1 = Active
             ;Bit 2   Reserved
             ;Bit 3   Reserved
             ;Bit 4   Reserved
             ;Bit 5   Reserved
             ;Bit 6   Reserved
             ;Bit 7   only CC 424, CC 61xx:
             ;        Readiness of the current controller is to be waited
             ;        for during a drive-on
             ;        0 = Wait for readiness of current controller
             ;        1 = Do not wait for readiness of current controller
             ;Bit 8   Reserved
             ;Bit 9   Reserved
             ;Bit 10  Reserved
             ;Bit 11  Reserved
             ;Bit 12  Reserved
             ;Bit 13  Reserved
             ;Bit 14  Reserved
             ;Bit 15  Reserved
             ;111111----------
             ;5432109876543210 <- bit #
MP 2223.0  : %0000000000000000
MP 2223.1  : %0000000000000000
MP 2223.2  : %0000000000000000
MP 2223.3  : %0000000000000000
MP 2223.4  : %0000000000000000
MP 2223.5  : %0000000000000000
MP 2223.6  : %0000000000000000
MP 2223.7  : %0000000000000000
MP 2223.8  : %0000000000000000
MP 2223.9  : %0000000000000000
MP 2223.10 : %0000000000000000
MP 2223.11 : %0000000000000000
MP 2223.12 : %0000000000000000
MP 2223.13 : %0000000000000000
MP 2223.14 : %0000000000000000
MP 2223.15 : %0000000000000000
             ;--------------------------------------------------------------
             ;MP2230 Motor brake test: Factor for rated current
             ;Input: 0 to 30
             ;       0 = do not carry out test; or motor without motor brake
MP 2230.0  : 0
MP 2230.1  : 0
MP 2230.2  : 1
MP 2230.3  : 0
MP 2230.4  : 0
MP 2230.5  : 0
MP 2230.6  : 0
MP 2230.7  : 0
MP 2230.8  : 0
MP 2230.9  : 0
MP 2230.10 : 0
MP 2230.11 : 0
MP 2230.12 : 0
MP 2230.13 : 0
MP 2230.14 : 0
MP 2230.15 : 0
             ;--------------------------------------------------------------
             ;MP2232 Motor brake test: Maximum permissible path
             ;Input: 0 to 10 [mm] or [degrees]
MP 2232.0  : 0.1
MP 2232.1  : 0.1
MP 2232.2  : 0.1
MP 2232.3  : 0.1
MP 2232.4  : 0.1
MP 2232.5  : 0.1
MP 2232.6  : 0.1
MP 2232.7  : 0.1
MP 2232.8  : 0.1
MP 2232.9  : 0.1
MP 2232.10 : 0.1
MP 2232.11 : 0.1
MP 2232.12 : 0.1
MP 2232.13 : 0.1
MP 2232.14 : 0.1
MP 2232.15 : 0.1
             ;--------------------------------------------------------------
             ;MP2236 Reserved
             ;Input: 10
MP 2236.0  : 10
MP 2236.1  : 10
MP 2236.2  : 10
MP 2236.3  : 10
MP 2236.4  : 10
MP 2236.5  : 10
MP 2236.6  : 10
MP 2236.7  : 10
MP 2236.8  : 10
MP 2236.9  : 10
MP 2236.10 : 10
MP 2236.11 : 10
MP 2236.12 : 10
MP 2236.13 : 10
MP 2236.14 : 10
MP 2236.15 : 10
             ;--------------------------------------------------------------
             ;MP2250 Determination of the field angle without movement of
             ;       the motor
             ;Input:   0 = like value 2
             ;         1 = reserved
             ;         2 = Method 2 (brake applied)
             ;         3 = Method 3 (like Method 2, but brake is not
             ;             applied)
             ;         4 = Method 4 (see Technical Manual)
MP 2250.0  : 0
MP 2250.1  : 0
MP 2250.2  : 0
MP 2250.3  : 0
MP 2250.4  : 0
MP 2250.5  : 0
MP 2250.6  : 0
MP 2250.7  : 0
MP 2250.8  : 0
MP 2250.9  : 0
MP 2250.10 : 0
MP 2250.11 : 0
MP 2250.12 : 0
MP 2250.13 : 0
MP 2250.14 : 0
MP 2250.15 : 0
             ;--------------------------------------------------------------
             ;MP2252 reserved
             ;Input: enter 0
MP 2252.0  : 0
MP 2252.1  : 0
MP 2252.2  : 0
MP 2252.3  : 0
MP 2252.4  : 0
MP 2252.5  : 0
MP 2252.6  : 0
MP 2252.7  : 0
MP 2252.8  : 0
MP 2252.9  : 0
MP 2252.10 : 0
MP 2252.11 : 0
MP 2252.12 : 0
MP 2252.13 : 0
MP 2252.14 : 0
MP 2252.15 : 0
             ;--------------------------------------------------------------
             ;MP2254 Determination of the field angle
             ;Input:   0 = Field angle is determined during operation,
             ;             softkey has no function
             ;             (without plausibility check)
             ;         1 = Only CC 422: Determination of the field angle via
             ;             softkey, movement of the motor is allowed
             ;         2 = Only CC 61xx: Determination of the field angle via
             ;             softkey, movement of the motor is allowed
             ;             (with plausibility check)
             ;         3 = like value 2, the drive does not need an enable
             ;             by the PLC
             ;             - Attention, refer to safety instructions in
             ;               technical manual -
MP 2254.0  : 0
MP 2254.1  : 0
MP 2254.2  : 0
MP 2254.3  : 0
MP 2254.4  : 0
MP 2254.5  : 0
MP 2254.6  : 0
MP 2254.7  : 0
MP 2254.8  : 0
MP 2254.9  : 0
MP 2254.10 : 0
MP 2254.11 : 0
MP 2254.12 : 0
MP 2254.13 : 0
MP 2254.14 : 0
MP 2254.15 : 0
             ;--------------------------------------------------------------
             ;MP2256 Determined field angle (after determination it is
             ;       entered by the iTNC)
             ;Input: 0 = Field angles not yet determined
MP 2256.0  : +0
MP 2256.1  : +0
MP 2256.2  : +0
MP 2256.3  : +0
MP 2256.4  : +0
MP 2256.5  : +0
MP 2256.6  : +0
MP 2256.7  : +0
MP 2256.8  : +0
MP 2256.9  : +0
MP 2256.10 : +0
MP 2256.11 : +0
MP 2256.12 : +0
MP 2256.13 : +0
MP 2256.14 : +0
MP 2256.15 : +0
             ;--------------------------------------------------------------
             ;MP2257 Control or encoder identification for
             ;       field angle from MP2256.x
             ;Input: $00000000 to $FFFFFFFF
MP 2257.0  : $00000000
MP 2257.1  : $00000000
MP 2257.2  : $00000000
MP 2257.3  : $00000000
MP 2257.4  : $00000000
MP 2257.5  : $00000000
MP 2257.6  : $00000000
MP 2257.7  : $00000000
MP 2257.8  : $00000000
MP 2257.9  : $00000000
MP 2257.10 : $00000000
MP 2257.11 : $00000000
MP 2257.12 : $00000000
MP 2257.13 : $00000000
MP 2257.14 : $00000000
MP 2257.15 : $00000000
             ;--------------------------------------------------------------
             ;MP2260 Reserved
             ;Input: [empty]
MP 2260.0  : 
MP 2260.1  : 
MP 2260.2  : 
MP 2260.3  : 
MP 2260.4  : 
MP 2260.5  : 
MP 2260.6  : 
MP 2260.7  : 
MP 2260.8  : 
MP 2260.9  : 
MP 2260.10 : 
MP 2260.11 : 
MP 2260.12 : 
MP 2260.13 : 
MP 2260.14 : 
MP 2260.15 : 
             ;--------------------------------------------------------------
             ;MP2261 Deactivate compensations
             ;       Bit 0: torque ripple compensation
             ;       Bit 1: gear error compensation
             ;Input: %0000000000000000
             ;       1 = compensation inactive
             ;111111----------
             ;5432109876543210 <- bit #
MP 2261.0  : %0000000000000000
MP 2261.1  : %0000000000000000
MP 2261.2  : %0000000000000000
MP 2261.3  : %0000000000000000
MP 2261.4  : %0000000000000000
MP 2261.5  : %0000000000000000
MP 2261.6  : %0000000000000000
MP 2261.7  : %0000000000000000
MP 2261.8  : %0000000000000000
MP 2261.9  : %0000000000000000
MP 2261.10 : %0000000000000000
MP 2261.11 : %0000000000000000
MP 2261.12 : %0000000000000000
MP 2261.13 : %0000000000000000
MP 2261.14 : %0000000000000000
MP 2261.15 : %0000000000000000
             ;--------------------------------------------------------------
             ;MP2302 Reference value for I2t monitoring of the motor
             ;Input: 0 to 1000.000 [factor]
             ;       0 = No monitoring (I2t power module active)
             ;     > 0 = Factor for rated current of the motor as
             ;           reference value
MP 2302.0  : 1
MP 2302.1  : 1
MP 2302.2  : 1
MP 2302.3  : 1
MP 2302.4  : 1
MP 2302.5  : 1
MP 2302.6  : 1
MP 2302.7  : 1
MP 2302.8  : 1
MP 2302.9  : 1
MP 2302.10 : 1
MP 2302.11 : 1
MP 2302.12 : 1          ;Y connection 1st spindle
MP 2302.13 : 1          ;Y connection 2nd spindle
MP 2302.14 : 1          ;D connection 1st spindle
MP 2302.15 : 1          ;D connection 2nd spindle
             ;--------------------------------------------------------------
             ;MP2304 Reference value for I2t monitoring of the power module
             ;Input: 0 to 1000.000 [factor]
             ;       0 = No monitoring
             ;     > 0 = Factor for rated current of the power module as
             ;           reference value
MP 2304.0  : 1
MP 2304.1  : 1
MP 2304.2  : 1
MP 2304.3  : 1
MP 2304.4  : 1
MP 2304.5  : 1
MP 2304.6  : 1
MP 2304.7  : 1
MP 2304.8  : 1
MP 2304.9  : 1
MP 2304.10 : 1
MP 2304.11 : 1
MP 2304.12 : 1
MP 2304.13 : 1
MP 2304.14 : 1
MP 2304.15 : 1
             ;--------------------------------------------------------------
             ;MP2308 Time between the braking signal and switch-off of the
             ;       controller (overlap time)
             ;Input: 0.001 to 5.000 [s]
             ;       0 = 0.200 s
MP 2308.0  : 0
MP 2308.1  : 0
MP 2308.2  : 0
MP 2308.3  : 0
MP 2308.4  : 0
MP 2308.5  : 0
MP 2308.6  : 0
MP 2308.7  : 0
MP 2308.8  : 0
MP 2308.9  : 0
MP 2308.10 : 0
MP 2308.11 : 0
MP 2308.12 : 0
MP 2308.13 : 0
MP 2308.14 : 0
MP 2308.15 : 0
             ;--------------------------------------------------------------
             ;MP2309 Time after switch-on of the controller in which the
             ;       parameters are adapted to active brakes
             ;Input: 0.001 to 5.000 [s]
             ;       0 = Function inactive
MP 2309.0  : 0
MP 2309.1  : 0
MP 2309.2  : 0
MP 2309.3  : 0
MP 2309.4  : 0
MP 2309.5  : 0
MP 2309.6  : 0
MP 2309.7  : 0
MP 2309.8  : 0
MP 2309.9  : 0
MP 2309.10 : 0
MP 2309.11 : 0
MP 2309.12 : 0
MP 2309.13 : 0
MP 2309.14 : 0
MP 2309.15 : 0
             ;--------------------------------------------------------------
             ;MP2312 Factor for utilization of the motors
             ;Input: 0 to 1000.000
             ;       0 or 1 = Rated current of motor as reference value
MP 2312.0  : 1
MP 2312.1  : 1
MP 2312.2  : 1
MP 2312.3  : 1
MP 2312.4  : 1
MP 2312.5  : 1
MP 2312.6  : 1
MP 2312.7  : 1
MP 2312.8  : 1
MP 2312.9  : 1
MP 2312.10 : 1
MP 2312.11 : 1
MP 2312.12 : 1
MP 2312.13 : 1
MP 2312.14 : 1
MP 2312.15 : 1
             ;--------------------------------------------------------------
             ;MP2390 Max. braking performance for deceleration in an EM STOP
             ;Input: 0.001 to 3000.000 [kW]
             ;       0 = Function inactive
MP 2390.0  : 0
MP 2390.1  : 0
MP 2390.2  : 0
MP 2390.3  : 0
MP 2390.4  : 0
MP 2390.5  : 0
MP 2390.6  : 0
MP 2390.7  : 0
MP 2390.8  : 0
MP 2390.9  : 0
MP 2390.10 : 0
MP 2390.11 : 0
MP 2390.12 : 0
MP 2390.13 : 0
MP 2390.14 : 0
MP 2390.15 : 0
             ;--------------------------------------------------------------
             ;MP2392 Maximum drive performance
             ;Input: 0.001 to 3000.000 [kW]
             ;       0 = Function inactive
MP 2392.0  : 0
MP 2392.1  : 0
MP 2392.2  : 0
MP 2392.3  : 0
MP 2392.4  : 0
MP 2392.5  : 0
MP 2392.6  : 0
MP 2392.7  : 0
MP 2392.8  : 0
MP 2392.9  : 0
MP 2392.10 : 0
MP 2392.11 : 0
MP 2392.12 : 0
MP 2392.13 : 0
MP 2392.14 : 0
MP 2392.15 : 0
             ;--------------------------------------------------------------
             ;MP2393 Maximum drive performance through PLC
             ;Input: 0.001 to 3000.000 [kW]
             ;       0 = Function inactive
MP 2393.0  : 0
MP 2393.1  : 0
MP 2393.2  : 0
MP 2393.3  : 0
MP 2393.4  : 0
MP 2393.5  : 0
MP 2393.6  : 0
MP 2393.7  : 0
MP 2393.8  : 0
MP 2393.9  : 0
MP 2393.10 : 0
MP 2393.11 : 0
MP 2393.12 : 0
MP 2393.13 : 0
MP 2393.14 : 0
MP 2393.15 : 0
             ;--------------------------------------------------------------
             ;MP2394 Maximum braking performance for deceleration in a
             ;power failure
             ;Input: 0.001 to 3000.000 [kW]
             ;       0 = Function inactive
MP 2394.0  : 0
MP 2394.1  : 0
MP 2394.2  : 0
MP 2394.3  : 0
MP 2394.4  : 0
MP 2394.5  : 0
MP 2394.6  : 0
MP 2394.7  : 0
MP 2394.8  : 0
MP 2394.9  : 0
MP 2394.10 : 0
MP 2394.11 : 0
MP 2394.12 : 0
MP 2394.13 : 0
MP 2394.14 : 0
MP 2394.15 : 0
             ;--------------------------------------------------------------
             ;MP2396 Maximum drive torque
             ;Input: 0.1 to 3000.0 [Nm]
             ;       0 = Function inactive
MP 2396.0  : 0
MP 2396.1  : 0
MP 2396.2  : 0
MP 2396.3  : 0
MP 2396.4  : 0
MP 2396.5  : 0
MP 2396.6  : 0
MP 2396.7  : 0
MP 2396.8  : 0
MP 2396.9  : 0
MP 2396.10 : 0
MP 2396.11 : 0
MP 2396.12 : 0
MP 2396.13 : 0
MP 2396.14 : 0
MP 2396.15 : 0
             ;--------------------------------------------------------------
             ;MP2420 Proportional factor for the current controller
             ;Input: 0 to 9999.99 [V/A]
             ;       * = automatic calculation of P-Factor
MP 2420.0  : 13.32
MP 2420.1  : 13.99
MP 2420.2  : 6.1
MP 2420.3  : 15
MP 2420.4  : 15
MP 2420.5  : 5
MP 2420.6  : 5
MP 2420.7  : 5
MP 2420.8  : 5
MP 2420.9  : 5
MP 2420.10 : 5
MP 2420.11 : 5
MP 2420.12 : 8.18       ;Y connection 1st spindle
MP 2420.13 : 8.18       ;Y connection 2nd spindle
MP 2420.14 : 8.18       ;D connection 1st spindle
MP 2420.15 : 8.18       ;D connection 2nd spindle
             ;--------------------------------------------------------------
             ;MP2430 Integral-action factor for current controller
             ;Input: 0 to 9 999 999 [Vs/A]
             ;       * = automatic calculation of I-Factor
MP 2430.0  : 3710
MP 2430.1  : 3710
MP 2430.2  : 1785
MP 2430.3  : 500
MP 2430.4  : 500
MP 2430.5  : 5000
MP 2430.6  : 5000
MP 2430.7  : 0
MP 2430.8  : 0
MP 2430.9  : 0
MP 2430.10 : 0
MP 2430.11 : 0
MP 2430.12 : 913        ;Y connection 1st spindle
MP 2430.13 : 913        ;Y connection 2nd spindle
MP 2430.14 : 913        ;D connection 1st spindle
MP 2430.15 : 913        ;D connection 2nd spindle
             ;--------------------------------------------------------------
             ;MP2440 Limit frequency of current controller
             ;       feed forward
             ;Input: 0.1 to 5000.0 [Hz]
             ;       0 = current controller feed forward inactive
MP 2440.0  : 0
MP 2440.1  : 0
MP 2440.2  : 0
MP 2440.3  : 0
MP 2440.4  : 0
MP 2440.5  : 0
MP 2440.6  : 0
MP 2440.7  : 0
MP 2440.8  : 0
MP 2440.9  : 0
MP 2440.10 : 0
MP 2440.11 : 0
MP 2440.12 : 0
MP 2440.13 : 0
MP 2440.14 : 0
MP 2440.15 : 0
             ;--------------------------------------------------------------
             ;MP2450 Compensation of dead time
             ;Input: 0 or 1
             ;       0 = compensation inactive
             ;       1 = compensation active
MP 2450.0  : 0
MP 2450.1  : 0
MP 2450.2  : 0
MP 2450.3  : 0
MP 2450.4  : 0
MP 2450.5  : 0
MP 2450.6  : 0
MP 2450.7  : 0
MP 2450.8  : 0
MP 2450.9  : 0
MP 2450.10 : 0
MP 2450.11 : 0
MP 2450.12 : 0
MP 2450.13 : 0
MP 2450.14 : 0
MP 2450.15 : 0
             ;--------------------------------------------------------------
             ;MP2500 Proportional factor of the speed controller
             ;Input: 0 to 1 000 000.000 [As]
MP 2500.0  : 20
MP 2500.1  : 20
MP 2500.2  : 40
MP 2500.3  : 2
MP 2500.4  : 1
MP 2500.5  : 1
MP 2500.6  : 1
MP 2500.7  : 1
MP 2500.8  : 1
MP 2500.9  : 1
MP 2500.10 : 1
MP 2500.11 : 1
MP 2500.12 : 30         ;Y connection 1st spindle
MP 2500.13 : 30         ;Y connection 2nd spindle
MP 2500.14 : 30         ;D connection 1st spindle
MP 2500.15 : 30         ;D connection 2nd spindle
             ;--------------------------------------------------------------
             ;MP2510 Integral factor of the speed controller
             ;Input: 0 to 100 000 000 [A]
MP 2510.0  : 2500
MP 2510.1  : 1750
MP 2510.2  : 5000
MP 2510.3  : 500
MP 2510.4  : 300
MP 2510.5  : 500
MP 2510.6  : 500
MP 2510.7  : 300
MP 2510.8  : 300
MP 2510.9  : 300
MP 2510.10 : 300
MP 2510.11 : 300
MP 2510.12 : 1500       ;Y connection 1st spindle
MP 2510.13 : 1500       ;Y connection 2nd spindle
MP 2510.14 : 1500       ;D connection 1st spindle
MP 2510.15 : 1500       ;D connection 2nd spindle
             ;--------------------------------------------------------------
             ;MP2512 Integral-action factor limitation of the speed
             ;       controller
             ;Input: 0 to 30 [s]
             ;       0 = Inactive (recommended)
MP 2512.0  : 0
MP 2512.1  : 0
MP 2512.2  : 0
MP 2512.3  : 0
MP 2512.4  : 0
MP 2512.5  : 0
MP 2512.6  : 0
MP 2512.7  : 0
MP 2512.8  : 0
MP 2512.9  : 0
MP 2512.10 : 0
MP 2512.11 : 0
MP 2512.12 : 0
MP 2512.13 : 0
MP 2512.14 : 0
MP 2512.15 : 0
             ;--------------------------------------------------------------
             ;MP2520 Differential factor for the speed controller of the axes
             ;Input: 0 to 1 [As^2]
             ;       0 = Inactive (recommended)
MP 2520.0  : 0
MP 2520.1  : 0
MP 2520.2  : 0
MP 2520.3  : 0
MP 2520.4  : 0
MP 2520.5  : 0
MP 2520.6  : 0
MP 2520.7  : 0
MP 2520.8  : 0
MP 2520.9  : 0
MP 2520.10 : 0
MP 2520.11 : 0
MP 2520.12 : 0
MP 2520.13 : 0
MP 2520.14 : 0
MP 2520.15 : 0
             ;--------------------------------------------------------------
             ;MP2542 Band-rejection filter damping, filter 1
             ;Input: 0 to 99.0 [dB]
MP 2542.0  : +11.1
MP 2542.1  : +11.1
MP 2542.2  : +11.6
MP 2542.3  : +0
MP 2542.4  : +0
MP 2542.5  : +0
MP 2542.6  : +0
MP 2542.7  : +0
MP 2542.8  : +0
MP 2542.9  : +0
MP 2542.10 : +0
MP 2542.11 : +0
MP 2542.12 : +0
MP 2542.13 : +0
MP 2542.14 : +0
MP 2542.15 : +0
             ;--------------------------------------------------------------
             ;MP2543 Band-rejection filter damping, filter 2
             ;Input: 0 to 99.0 [dB]
MP 2543.0  : +10
MP 2543.1  : +10
MP 2543.2  : +0
MP 2543.3  : +0
MP 2543.4  : +0
MP 2543.5  : +0
MP 2543.6  : +0
MP 2543.7  : +0
MP 2543.8  : +0
MP 2543.9  : +0
MP 2543.10 : +0
MP 2543.11 : +0
MP 2543.12 : +0
MP 2543.13 : +0
MP 2543.14 : +0
MP 2543.15 : +0
             ;--------------------------------------------------------------
             ;MP2544 Band-rejection filter damping, filter 3
             ;Input: 0 to 99.0 [dB]
MP 2544.0  : +0
MP 2544.1  : +0
MP 2544.2  : +0
MP 2544.3  : +0
MP 2544.4  : +0
MP 2544.5  : +0
MP 2544.6  : +0
MP 2544.7  : +0
MP 2544.8  : +0
MP 2544.9  : +0
MP 2544.10 : +0
MP 2544.11 : +0
MP 2544.12 : +0
MP 2544.13 : +0
MP 2544.14 : +0
MP 2544.15 : +0
             ;--------------------------------------------------------------
             ;MP2545 Band-rejection filter damping, filter 4
             ;Input: 0 to 99.0 [dB]
MP 2545.0  : +0
MP 2545.1  : +0
MP 2545.2  : +0
MP 2545.3  : +0
MP 2545.4  : +0
MP 2545.5  : +0
MP 2545.6  : +0
MP 2545.7  : +0
MP 2545.8  : +0
MP 2545.9  : +0
MP 2545.10 : +0
MP 2545.11 : +0
MP 2545.12 : +0
MP 2545.13 : +0
MP 2545.14 : +0
MP 2545.15 : +0
             ;--------------------------------------------------------------
             ;MP2546 Band-rejection filter damping, filter 5
             ;Input: 0 to 99.0 [dB]
MP 2546.0  : +0
MP 2546.1  : +0
MP 2546.2  : +0
MP 2546.3  : +0
MP 2546.4  : +0
MP 2546.5  : +0
MP 2546.6  : +0
MP 2546.7  : +0
MP 2546.8  : +0
MP 2546.9  : +0
MP 2546.10 : +0
MP 2546.11 : +0
MP 2546.12 : +0
MP 2546.13 : +0
MP 2546.14 : +0
MP 2546.15 : +0
             ;--------------------------------------------------------------
             ;MP2547 Band-rejection filter damping, filter 6
             ;Input: 0 to 99.0 [dB]
MP 2547.0  : +0
MP 2547.1  : +0
MP 2547.2  : +0
MP 2547.3  : +0
MP 2547.4  : +0
MP 2547.5  : +0
MP 2547.6  : +0
MP 2547.7  : +0
MP 2547.8  : +0
MP 2547.9  : +0
MP 2547.10 : +0
MP 2547.11 : +0
MP 2547.12 : +0
MP 2547.13 : +0
MP 2547.14 : +0
MP 2547.15 : +0
             ;--------------------------------------------------------------
             ;MP2552 Band-rejection filter center frequency, filter 1
             ;Input: 0 to 30 000.0 [Hz]
MP 2552.0  : 469
MP 2552.1  : 469
MP 2552.2  : 484
MP 2552.3  : 0
MP 2552.4  : 0
MP 2552.5  : 0
MP 2552.6  : 0
MP 2552.7  : 0
MP 2552.8  : 0
MP 2552.9  : 0
MP 2552.10 : 0
MP 2552.11 : 0
MP 2552.12 : 0
MP 2552.13 : 0
MP 2552.14 : 0
MP 2552.15 : 0
             ;--------------------------------------------------------------
             ;MP2553 Band-rejection filter center frequency, filter 2
             ;Input: 0 to 30 000.0 [Hz]
MP 2553.0  : 718
MP 2553.1  : 703
MP 2553.2  : 0
MP 2553.3  : 0
MP 2553.4  : 0
MP 2553.5  : 0
MP 2553.6  : 0
MP 2553.7  : 0
MP 2553.8  : 0
MP 2553.9  : 0
MP 2553.10 : 0
MP 2553.11 : 0
MP 2553.12 : 0
MP 2553.13 : 0
MP 2553.14 : 0
MP 2553.15 : 0
             ;--------------------------------------------------------------
             ;MP2554 Band-rejection filter center frequency, filter 3
             ;Input: 0 to 30 000.0 [Hz]
MP 2554.0  : 0
MP 2554.1  : 0
MP 2554.2  : 0
MP 2554.3  : 0
MP 2554.4  : 0
MP 2554.5  : 0
MP 2554.6  : 0
MP 2554.7  : 0
MP 2554.8  : 0
MP 2554.9  : 0
MP 2554.10 : 0
MP 2554.11 : 0
MP 2554.12 : 0
MP 2554.13 : 0
MP 2554.14 : 0
MP 2554.15 : 0
             ;--------------------------------------------------------------
             ;MP2555 Band-rejection filter center frequency, filter 4
             ;Input: 0 to 30 000.0 [Hz]
MP 2555.0  : 0
MP 2555.1  : 0
MP 2555.2  : 0
MP 2555.3  : 0
MP 2555.4  : 0
MP 2555.5  : 0
MP 2555.6  : 0
MP 2555.7  : 0
MP 2555.8  : 0
MP 2555.9  : 0
MP 2555.10 : 0
MP 2555.11 : 0
MP 2555.12 : 0
MP 2555.13 : 0
MP 2555.14 : 0
MP 2555.15 : 0
             ;--------------------------------------------------------------
             ;MP2556 Band-rejection filter center frequency, filter 5
             ;Input: 0 to 30 000.0 [Hz]
MP 2556.0  : 0
MP 2556.1  : 0
MP 2556.2  : 0
MP 2556.3  : 0
MP 2556.4  : 0
MP 2556.5  : 0
MP 2556.6  : 0
MP 2556.7  : 0
MP 2556.8  : 0
MP 2556.9  : 0
MP 2556.10 : 0
MP 2556.11 : 0
MP 2556.12 : 0
MP 2556.13 : 0
MP 2556.14 : 0
MP 2556.15 : 0
             ;--------------------------------------------------------------
             ;MP2557 Band-rejection filter center frequency, filter 6
             ;Input: 0 to 30 000.0 [Hz]
MP 2557.0  : 0
MP 2557.1  : 0
MP 2557.2  : 0
MP 2557.3  : 0
MP 2557.4  : 0
MP 2557.5  : 0
MP 2557.6  : 0
MP 2557.7  : 0
MP 2557.8  : 0
MP 2557.9  : 0
MP 2557.10 : 0
MP 2557.11 : 0
MP 2557.12 : 0
MP 2557.13 : 0
MP 2557.14 : 0
MP 2557.15 : 0
             ;--------------------------------------------------------------
             ;MP2560 Filter order of the low-pass filter
             ;Input: 0 to 20
             ;       Recommended input for high current noise of high
             ;       frequency spindles: 10 to 20
MP 2560.0  : 1
MP 2560.1  : 1
MP 2560.2  : 1
MP 2560.3  : 1
MP 2560.4  : 0
MP 2560.5  : 0
MP 2560.6  : 0
MP 2560.7  : 0
MP 2560.8  : 0
MP 2560.9  : 0
MP 2560.10 : 0
MP 2560.11 : 0
MP 2560.12 : 3
MP 2560.13 : 3
MP 2560.14 : 3
MP 2560.15 : 3
             ;--------------------------------------------------------------
             ;MP2561 Maximum motor encoder input frequency
             ;Input: %xxxxxxxxxxxxxxxx
             ;Bit 0   0 = 400 kHz
             ;        1 = 800 kHz
             ;111111----------
             ;5432109876543210 <- bit #
MP 2561.0  : %0000000000000000
MP 2561.1  : %0000000000000000
MP 2561.2  : %0000000000000000
MP 2561.3  : %0000000000000000
MP 2561.4  : %0000000000000000
MP 2561.5  : %0000000000000000
MP 2561.6  : %0000000000000000
MP 2561.7  : %0000000000000000
MP 2561.8  : %0000000000000000
MP 2561.9  : %0000000000000000
MP 2561.10 : %0000000000000000
MP 2561.11 : %0000000000000000
MP 2561.12 : %0000000000000000
MP 2561.13 : %0000000000000000
MP 2561.14 : %0000000000000000
MP 2561.15 : %0000000000000000
             ;--------------------------------------------------------------
             ;MP2562 Filter type for filter 1
             ;Input: 0 = No filter
             ;       1 = PT2 low-pass filter (speed controller)
             ;       2 = Band-rejection filter (speed controller)
             ;       3 = Phase lifting filter (speed controller)
             ;       11 = PT2 low-pass filter (position controller)
             ;       12 = Band-rejection filter (position controller)
             ;       13 = Phase lifting filter (position controller)
MP 2562.0  : 2
MP 2562.1  : 2
MP 2562.2  : 2
MP 2562.3  : 0
MP 2562.4  : 0
MP 2562.5  : 0
MP 2562.6  : 0
MP 2562.7  : 0
MP 2562.8  : 0
MP 2562.9  : 0
MP 2562.10 : 0
MP 2562.11 : 0
MP 2562.12 : 0
MP 2562.13 : 0
MP 2562.14 : 0
MP 2562.15 : 0
             ;--------------------------------------------------------------
             ;MP2563 Filter type for filter 2
             ;Input: 0 = No filter
             ;       1 = PT2 low-pass filter (speed controller)
             ;       2 = Band-rejection filter (speed controller)
             ;       3 = Phase lifting filter (speed controller)
             ;       11 = PT2 low-pass filter (position controller)
             ;       12 = Band-rejection filter (position controller)
             ;       13 = Phase lifting filter (position controller)
MP 2563.0  : 2
MP 2563.1  : 2
MP 2563.2  : 0
MP 2563.3  : 0
MP 2563.4  : 0
MP 2563.5  : 0
MP 2563.6  : 0
MP 2563.7  : 0
MP 2563.8  : 0
MP 2563.9  : 0
MP 2563.10 : 0
MP 2563.11 : 0
MP 2563.12 : 0
MP 2563.13 : 0
MP 2563.14 : 0
MP 2563.15 : 0
             ;--------------------------------------------------------------
             ;MP2564 Filter type for filter 3
             ;Input: 0 = No filter
             ;       1 = PT2 low-pass filter (speed controller)
             ;       2 = Band-rejection filter (speed controller)
             ;       3 = Phase lifting filter (speed controller)
             ;       11 = PT2 low-pass filter (position controller)
             ;       12 = Band-rejection filter (position controller)
             ;       13 = Phase lifting filter (position controller)
MP 2564.0  : 0
MP 2564.1  : 0
MP 2564.2  : 0
MP 2564.3  : 0
MP 2564.4  : 0
MP 2564.5  : 0
MP 2564.6  : 0
MP 2564.7  : 0
MP 2564.8  : 0
MP 2564.9  : 0
MP 2564.10 : 0
MP 2564.11 : 0
MP 2564.12 : 0
MP 2564.13 : 0
MP 2564.14 : 0
MP 2564.15 : 0
             ;--------------------------------------------------------------
             ;MP2565 Filter type for filter 4
             ;Input: 0 = No filter
             ;       1 = PT2 low-pass filter (speed controller)
             ;       2 = Band-rejection filter (speed controller)
             ;       3 = Phase lifting filter (speed controller)
             ;       11 = PT2 low-pass filter (position controller)
             ;       12 = Band-rejection filter (position controller)
             ;       13 = Phase lifting filter (position controller)
MP 2565.0  : 0
MP 2565.1  : 0
MP 2565.2  : 0
MP 2565.3  : 0
MP 2565.4  : 0
MP 2565.5  : 0
MP 2565.6  : 0
MP 2565.7  : 0
MP 2565.8  : 0
MP 2565.9  : 0
MP 2565.10 : 0
MP 2565.11 : 0
MP 2565.12 : 0
MP 2565.13 : 0
MP 2565.14 : 0
MP 2565.15 : 0
             ;--------------------------------------------------------------
             ;MP2566 Filter type for filter 5
             ;Input: 0 = No filter
             ;       1 = PT2 low-pass filter (speed controller)
             ;       2 = Band-rejection filter (speed controller)
             ;       3 = Phase lifting filter (speed controller)
             ;       11 = PT2 low-pass filter (position controller)
             ;       12 = Band-rejection filter (position controller)
             ;       13 = Phase lifting filter (position controller)
MP 2566.0  : 0
MP 2566.1  : 0
MP 2566.2  : 0
MP 2566.3  : 0
MP 2566.4  : 0
MP 2566.5  : 0
MP 2566.6  : 0
MP 2566.7  : 0
MP 2566.8  : 0
MP 2566.9  : 0
MP 2566.10 : 0
MP 2566.11 : 0
MP 2566.12 : 0
MP 2566.13 : 0
MP 2566.14 : 0
MP 2566.15 : 0
             ;--------------------------------------------------------------
             ;MP2567 Filter type for filter 6
             ;Input: 0 = No filter
             ;       1 = PT2 low-pass filter (speed controller)
             ;       2 = Band-rejection filter (speed controller)
             ;       3 = Phase lifting filter (speed controller)
             ;       11 = PT2 low-pass filter (position controller)
             ;       12 = Band-rejection filter (position controller)
             ;       13 = Phase lifting filter (position controller)
MP 2567.0  : 0
MP 2567.1  : 0
MP 2567.2  : 0
MP 2567.3  : 0
MP 2567.4  : 0
MP 2567.5  : 0
MP 2567.6  : 0
MP 2567.7  : 0
MP 2567.8  : 0
MP 2567.9  : 0
MP 2567.10 : 0
MP 2567.11 : 0
MP 2567.12 : 0
MP 2567.13 : 0
MP 2567.14 : 0
MP 2567.15 : 0
             ;--------------------------------------------------------------
             ;MP2572 Band-rejection filter bandwidth for filter 1
             ;Input: 0 to 30 000.0 [Hz]
MP 2572.0  : 469
MP 2572.1  : 469
MP 2572.2  : 484
MP 2572.3  : 0
MP 2572.4  : 0
MP 2572.5  : 0
MP 2572.6  : 0
MP 2572.7  : 0
MP 2572.8  : 0
MP 2572.9  : 0
MP 2572.10 : 0
MP 2572.11 : 0
MP 2572.12 : 0
MP 2572.13 : 0
MP 2572.14 : 0
MP 2572.15 : 0
             ;--------------------------------------------------------------
             ;MP2573 Band-rejection filter bandwidth for filter 2
             ;Input: 0 to 30 000.0 [Hz]
MP 2573.0  : 718
MP 2573.1  : 703
MP 2573.2  : 0
MP 2573.3  : 0
MP 2573.4  : 0
MP 2573.5  : 0
MP 2573.6  : 0
MP 2573.7  : 0
MP 2573.8  : 0
MP 2573.9  : 0
MP 2573.10 : 0
MP 2573.11 : 0
MP 2573.12 : 0
MP 2573.13 : 0
MP 2573.14 : 0
MP 2573.15 : 0
             ;--------------------------------------------------------------
             ;MP2574 Band-rejection filter bandwidth for filter 3
             ;Input: 0 to 30 000.0 [Hz]
MP 2574.0  : 0
MP 2574.1  : 0
MP 2574.2  : 0
MP 2574.3  : 0
MP 2574.4  : 0
MP 2574.5  : 0
MP 2574.6  : 0
MP 2574.7  : 0
MP 2574.8  : 0
MP 2574.9  : 0
MP 2574.10 : 0
MP 2574.11 : 0
MP 2574.12 : 0
MP 2574.13 : 0
MP 2574.14 : 0
MP 2574.15 : 0
             ;--------------------------------------------------------------
             ;MP2575 Band-rejection filter bandwidth for filter 4
             ;Input: 0 to 30 000.0 [Hz]
MP 2575.0  : 0
MP 2575.1  : 0
MP 2575.2  : 0
MP 2575.3  : 0
MP 2575.4  : 0
MP 2575.5  : 0
MP 2575.6  : 0
MP 2575.7  : 0
MP 2575.8  : 0
MP 2575.9  : 0
MP 2575.10 : 0
MP 2575.11 : 0
MP 2575.12 : 0
MP 2575.13 : 0
MP 2575.14 : 0
MP 2575.15 : 0
             ;--------------------------------------------------------------
             ;MP2576 Band-rejection filter bandwidth for filter 5
             ;Input: 0 to 30 000.0 [Hz]
MP 2576.0  : 0
MP 2576.1  : 0
MP 2576.2  : 0
MP 2576.3  : 0
MP 2576.4  : 0
MP 2576.5  : 0
MP 2576.6  : 0
MP 2576.7  : 0
MP 2576.8  : 0
MP 2576.9  : 0
MP 2576.10 : 0
MP 2576.11 : 0
MP 2576.12 : 0
MP 2576.13 : 0
MP 2576.14 : 0
MP 2576.15 : 0
             ;--------------------------------------------------------------
             ;MP2577 Band-rejection filter bandwidth for filter 6
             ;Input: 0 to 30 000.0 [Hz]
MP 2577.0  : 0
MP 2577.1  : 0
MP 2577.2  : 0
MP 2577.3  : 0
MP 2577.4  : 0
MP 2577.5  : 0
MP 2577.6  : 0
MP 2577.7  : 0
MP 2577.8  : 0
MP 2577.9  : 0
MP 2577.10 : 0
MP 2577.11 : 0
MP 2577.12 : 0
MP 2577.13 : 0
MP 2577.14 : 0
MP 2577.15 : 0
             ;--------------------------------------------------------------
             ;MP2590 Ramp for braking in an EMERGENCY STOP
             ;Input: 0.1 to 999.9 [rev/min/ms]
             ;                    (rpm is the motor speed)
             ;       0 = Function inactive
MP 2590.0  : 0
MP 2590.1  : 0
MP 2590.2  : 0
MP 2590.3  : 0
MP 2590.4  : 0
MP 2590.5  : 0
MP 2590.6  : 0
MP 2590.7  : 0
MP 2590.8  : 0
MP 2590.9  : 0
MP 2590.10 : 0
MP 2590.11 : 0
MP 2590.12 : 0
MP 2590.13 : 0
MP 2590.14 : 0
MP 2590.15 : 0
             ;--------------------------------------------------------------
             ;MP2600 Acceleration feedforward control
             ;Input: 0 to 1000 [A/(rev/s^2)]
MP 2600.0  : 0.0466
MP 2600.1  : 0.0636
MP 2600.2  : 0.088
MP 2600.3  : 0
MP 2600.4  : 0
MP 2600.5  : 0
MP 2600.6  : 0
MP 2600.7  : 0
MP 2600.8  : 0
MP 2600.9  : 0
MP 2600.10 : 0
MP 2600.11 : 0
MP 2600.12 : 0
MP 2600.13 : 0
MP 2600.14 : 0
MP 2600.15 : 0
             ;--------------------------------------------------------------
             ;MP2602 IPC time constant T 1
             ;Input: 0 to 1.0000 [s]
             ;       0: No IPC
MP 2602.0  : 0
MP 2602.1  : 0
MP 2602.2  : 0
MP 2602.3  : 0
MP 2602.4  : 0
MP 2602.5  : 0
MP 2602.6  : 0
MP 2602.7  : 0
MP 2602.8  : 0
MP 2602.9  : 0
MP 2602.10 : 0
MP 2602.11 : 0
MP 2602.12 : 0
MP 2602.13 : 0
MP 2602.14 : 0
MP 2602.15 : 0
             ;--------------------------------------------------------------
             ;MP2604 IPC time constant T 2
             ;Input: 0 to 1.0000 [s]
             ;       0: No IPC
MP 2604.0  : 0
MP 2604.1  : 0
MP 2604.2  : 0
MP 2604.3  : 0
MP 2604.4  : 0
MP 2604.5  : 0
MP 2604.6  : 0
MP 2604.7  : 0
MP 2604.8  : 0
MP 2604.9  : 0
MP 2604.10 : 0
MP 2604.11 : 0
MP 2604.12 : 0
MP 2604.13 : 0
MP 2604.14 : 0
MP 2604.15 : 0
             ;--------------------------------------------------------------
             ;MP2606 Following error during jerk
             ;Input 0.000 to 0.080
             ;      0 = Function inactive
MP 2606.0  : 0
MP 2606.1  : 0
MP 2606.2  : 0
MP 2606.3  : 0
MP 2606.4  : 0
MP 2606.5  : 0
MP 2606.6  : 0
MP 2606.7  : 0
MP 2606.8  : 0
MP 2606.9  : 0
MP 2606.10 : 0
MP 2606.11 : 0
MP 2606.12 : 0
MP 2606.13 : 0
MP 2606.14 : 0
MP 2606.15 : 0
             ;--------------------------------------------------------------
             ;MP2607 Damping factor for active damping of low-frequency
             ;       oscillations
             ;Input: 0 to 30.000
             ;       0: No damping
             ;       1.5: Typical damping factor
MP 2607.0  : 0
MP 2607.1  : 0
MP 2607.2  : 0
MP 2607.3  : 0
MP 2607.4  : 0
MP 2607.5  : 0
MP 2607.6  : 0
MP 2607.7  : 0
MP 2607.8  : 0
MP 2607.9  : 0
MP 2607.10 : 0
MP 2607.11 : 0
MP 2607.12 : 0
MP 2607.13 : 0
MP 2607.14 : 0
MP 2607.15 : 0
             ;--------------------------------------------------------------
             ;MP2608 Damping time constant for active damping
             ;       of low-frequency oscillations
             ;Input: 0 to 0.9999 [s]
             ;       0: No damping
             ;       0.005 to 0.02: Typical damping time constant
MP 2608.0  : 0
MP 2608.1  : 0
MP 2608.2  : 0
MP 2608.3  : 0
MP 2608.4  : 0
MP 2608.5  : 0
MP 2608.6  : 0
MP 2608.7  : 0
MP 2608.8  : 0
MP 2608.9  : 0
MP 2608.10 : 0
MP 2608.11 : 0
MP 2608.12 : 0
MP 2608.13 : 0
MP 2608.14 : 0
MP 2608.15 : 0
             ;--------------------------------------------------------------
             ;MP2609 Separation frequency for Crossover Position Filter
             ;       (CPF) for the passive damping of machine resonances,
             ;       can be used for drive motors with speed and position
             ;       encoder (2-encoder systems), (only with CC 61xx).
             ;Input: 0 to 300.0000 [Hz]
             ;       0: CPF inactive
             ;       10.0000 to 100.0000: Typical frequency range
MP 2609.0  : 0
MP 2609.1  : 0
MP 2609.2  : 0
MP 2609.3  : 0
MP 2609.4  : 0
MP 2609.5  : 0
MP 2609.6  : 0
MP 2609.7  : 0
MP 2609.8  : 0
MP 2609.9  : 0
MP 2609.10 : 0
MP 2609.11 : 0
MP 2609.12 : 0
MP 2609.13 : 0
MP 2609.14 : 0
MP 2609.15 : 0
             ;--------------------------------------------------------------
             ;MP2610 Friction compensation at approx. 10 rpm
             ;Input: 0 to 100 [A]
MP 2610.0  : 1.029
MP 2610.1  : 1.08
MP 2610.2  : 2.517
MP 2610.3  : 0
MP 2610.4  : 0
MP 2610.5  : 0
MP 2610.6  : 0
MP 2610.7  : 0
MP 2610.8  : 0
MP 2610.9  : 0
MP 2610.10 : 0
MP 2610.11 : 0
MP 2610.12 : 0
MP 2610.13 : 0
MP 2610.14 : 0
MP 2610.15 : 0
             ;--------------------------------------------------------------
             ;MP2612 Delay of friction compensation
             ;Input: 0 to 1 [s], if MP2614 = 0
             ;       0 to 1 [mm], if MP2614 > 0
MP 2612.0  : 0          ;0.005
MP 2612.1  : 0          ;0.001
MP 2612.2  : 0          ;0.004
MP 2612.3  : 0
MP 2612.4  : 0
MP 2612.5  : 0
MP 2612.6  : 0
MP 2612.7  : 0
MP 2612.8  : 0
MP 2612.9  : 0
MP 2612.10 : 0
MP 2612.11 : 0
MP 2612.12 : 0
MP 2612.13 : 0
MP 2612.14 : 0
MP 2612.15 : 0
             ;--------------------------------------------------------------
             ;MP2614 Dynamic friction compensation
             ;Input: 0 to 100.000 [mm]
MP 2614.0  : 0          ;0.007
MP 2614.1  : 0          ;0.006
MP 2614.2  : 0          ;0.005
MP 2614.3  : 0
MP 2614.4  : 0
MP 2614.5  : 0
MP 2614.6  : 0
MP 2614.7  : 0
MP 2614.8  : 0
MP 2614.9  : 0
MP 2614.10 : 0
MP 2614.11 : 0
MP 2614.12 : 0
MP 2614.13 : 0
MP 2614.14 : 0
MP 2614.15 : 0
             ;--------------------------------------------------------------
             ;MP2620 Friction compensation at rated speed
             ;Input: 0 to 100.000 [A]
MP 2620.0  : 7.606
MP 2620.1  : 7.752
MP 2620.2  : 12
MP 2620.3  : 0
MP 2620.4  : 0
MP 2620.5  : 0
MP 2620.6  : 0
MP 2620.7  : 0
MP 2620.8  : 0
MP 2620.9  : 0
MP 2620.10 : 0
MP 2620.11 : 0
MP 2620.12 : 0
MP 2620.13 : 0
MP 2620.14 : 0
MP 2620.15 : 0
             ;--------------------------------------------------------------
             ;MP2630 Holding current for vertical axis (weight compensation)
             ;Input: -100.000 to +100.000 [A]
MP 2630.0  : +0
MP 2630.1  : +0
MP 2630.2  : -7.257
MP 2630.3  : +0
MP 2630.4  : +0
MP 2630.5  : +0
MP 2630.6  : +0
MP 2630.7  : +0
MP 2630.8  : +0
MP 2630.9  : +0
MP 2630.10 : +0
MP 2630.11 : +0
MP 2630.12 : +0
MP 2630.13 : +0
MP 2630.14 : +0
MP 2630.15 : +0
             ;--------------------------------------------------------------
             ;MP2640 Torsion-Compensation between position encoder and
             ;       speed encoder
             ;Input: 0.001 to 100.000 [um/A]
             ;       0 = Inactive
MP 2640.0  : 0
MP 2640.1  : 0
MP 2640.2  : 0
MP 2640.3  : 0
MP 2640.4  : 0
MP 2640.5  : 0
MP 2640.6  : 0
MP 2640.7  : 0
MP 2640.8  : 0
MP 2640.9  : 0
MP 2640.10 : 0
MP 2640.11 : 0
MP 2640.12 : 0
MP 2640.13 : 0
MP 2640.14 : 0
MP 2640.15 : 0
             ;--------------------------------------------------------------
             ;MP2690 Limitation of compensation values for backlash,
             ;       static friction and reversal error
             ;Input: 0 to 1.0000 [s]
             ;       0 = Inactive
MP 2690.0  : 0
MP 2690.1  : 0
MP 2690.2  : 0
MP 2690.3  : 0
MP 2690.4  : 0
MP 2690.5  : 0
MP 2690.6  : 0
MP 2690.7  : 0
MP 2690.8  : 0
MP 2690.9  : 0
MP 2690.10 : 0
MP 2690.11 : 0
MP 2690.12 : 0
MP 2690.13 : 0
MP 2690.14 : 0
MP 2690.15 : 0
             ;--------------------------------------------------------------
             ;MP2700 Axis specific activation of additional
             ;       compensations (software options)
             ;Input: %xxxxxxxxxxxxxxxxxx
             ;Bit 0   CTC compensation (option 141)
             ;        0 = Inactive
             ;        1 = Active
             ;Bit 1   PAC compensation (option 142)
             ;        0 = Inactive
             ;        1 = Active
             ;Bit 2   LAC compensation (option 143)
             ;        0 = Inactive
             ;        1 = Active
             ;Bit 3   MAC compensation (Option 144)
             ;        0 = Inactive
             ;        1 = Active
             ;Bit 4   ACC compensation (Option 145)
             ;        0 = Inactive
             ;        1 = Active
             ;Bit 5   AVD compensation (Option 146)
             ;        0 = Inactive
             ;        1 = Active
             ;111111----------
             ;5432109876543210 <- bit #
MP 2700.0  : %0000000000000000
MP 2700.1  : %0000000000000000
MP 2700.2  : %0000000000000000
MP 2700.3  : %0000000000000000
MP 2700.4  : %0000000000000000
MP 2700.5  : %0000000000000000
MP 2700.6  : %0000000000000000
MP 2700.7  : %0000000000000000
MP 2700.8  : %0000000000000000
MP 2700.9  : %0000000000000000
MP 2700.10 : %0000000000000000
MP 2700.11 : %0000000000000000
MP 2700.12 : %0000000000000000
MP 2700.13 : %0000000000000000
MP 2700.14 : %0000000000000000
MP 2700.15 : %0000000000000000
             ;--------------------------------------------------------------
             ;MP2710 ACC: Selection of principal of function of the
             ;       automatic mode (Only valid if bit 4 is set in MP2700)
             ;Input: 0 = Robust standard mode with an average application
             ;           of a signal for damping the chatter
             ;           (recommended)
             ;       1 = Chatter damping is increased for the selected
             ;           axis; this can sometimes be advisable for one
             ;           or two axes (e.g. vertical axes) and results in
             ;           slightly reduced control accuracy
             ;           (following error increases slightly)
             ;       2 = Chatter damping is significantly increased for
             ;           the selected axis; this is usually advisable
             ;           for no more than one axis and results in
             ;           reduced control accuracy
             ;           (following error increases)
MP 2710.0  : 0
MP 2710.1  : 0
MP 2710.2  : 0
MP 2710.3  : 0
MP 2710.4  : 0
MP 2710.5  : 0
MP 2710.6  : 0
MP 2710.7  : 0
MP 2710.8  : 0
MP 2710.9  : 0
MP 2710.10 : 0
MP 2710.11 : 0
MP 2710.12 : 0
MP 2710.13 : 0
MP 2710.14 : 0
MP 2710.15 : 0
             ;--------------------------------------------------------------
             ;MP2720 Index assignment for AVD compensation files
             ;Input: 0 to 9
MP 2720.0  : 0
MP 2720.1  : 0
MP 2720.2  : 0
MP 2720.3  : 0
MP 2720.4  : 0
MP 2720.5  : 0
MP 2720.6  : 0
MP 2720.7  : 0
MP 2720.8  : 0
MP 2720.9  : 0
MP 2720.10 : 0
MP 2720.11 : 0
MP 2720.12 : 0
MP 2720.13 : 0
MP 2720.14 : 0
MP 2720.15 : 0
             ;--------------------------------------------------------------
             ;MP2800 reserved
             ;111111----------
             ;5432109876543210 <- bit #
MP 2800.0  : %0000000000000000
MP 2800.1  : %0000000000000000
MP 2800.2  : %0000000000000000
MP 2800.3  : %0000000000000000
MP 2800.4  : %0000000000000000
MP 2800.5  : %0000000000000000
MP 2800.6  : %0000000000000000
MP 2800.7  : %0000000000000000
MP 2800.8  : %0000000000000000
MP 2800.9  : %0000000000000000
MP 2800.10 : %0000000000000000
MP 2800.11 : %0000000000000000
MP 2800.12 : %0000000000000000
MP 2800.13 : %0000000000000000
MP 2800.14 : %0000000000000000
MP 2800.15 : %0000000000000000
             ;--------------------------------------------------------------
             ;MP2900 Tensioning torque between master and slave for
             ;       master-slave torque control
             ;Input: -100.00 to 100.00 [Nm]
             ;       (entry under slave axis)
MP 2900.0  : +0
MP 2900.1  : +0
MP 2900.2  : +0
MP 2900.3  : +0
MP 2900.4  : +0
MP 2900.5  : +0
MP 2900.6  : +0
MP 2900.7  : +0
MP 2900.8  : +0
MP 2900.9  : +0
MP 2900.10 : +0
MP 2900.11 : +0
MP 2900.12 : +0
MP 2900.13 : +0
MP 2900.14 : +0
MP 2900.15 : +0
             ;--------------------------------------------------------------
             ;MP2910 Gain for the torque controller of the master-slave
             ;       torque control
             ;Input: 0.001 to 9999.999 [1/(min*Nm)]
             ;       (entry under slave axis)
MP 2910.0  : 0
MP 2910.1  : 0
MP 2910.2  : 0
MP 2910.3  : 0
MP 2910.4  : 0
MP 2910.5  : 0
MP 2910.6  : 0
MP 2910.7  : 0
MP 2910.8  : 0
MP 2910.9  : 0
MP 2910.10 : 0
MP 2910.11 : 0
MP 2910.12 : 0
MP 2910.13 : 0
MP 2910.14 : 0
MP 2910.15 : 0
             ;--------------------------------------------------------------
             ;MP2912 Configuration of the master-slave
             ;       torque control
             ;Input: %xx
             ;Bit 0   Speed compensation configuration
             ;        0 = Speed compensation value applied to master
             ;            and slave
             ;        1 = Speed compensation value applied only to slave
             ;Bit 1   Feedforward of master-slave torque control
             ;        0 = Inactive
             ;        1 = Active
             ;10 <- bit #
MP 2912.0  : %00
MP 2912.1  : %00
MP 2912.2  : %00
MP 2912.3  : %00
MP 2912.4  : %00
MP 2912.5  : %00
MP 2912.6  : %00
MP 2912.7  : %00
MP 2912.8  : %00
MP 2912.9  : %00
MP 2912.10 : %00
MP 2912.11 : %00
MP 2912.12 : %00
MP 2912.13 : %00
MP 2912.14 : %00
MP 2912.15 : %00
             ;-------------------------------------------------------------
             ;MP2920 Reserved
             ;Input: 0
MP 2920.0  : 0
MP 2920.1  : 0
MP 2920.2  : 0
MP 2920.3  : 0
MP 2920.4  : 0
MP 2920.5  : 0
MP 2920.6  : 0
MP 2920.7  : 0
MP 2920.8  : 0
MP 2920.9  : 0
MP 2920.10 : 0
MP 2920.11 : 0
MP 2920.12 : 0
MP 2920.13 : 0
MP 2920.14 : 0
MP 2920.15 : 0
             ;-------------------------------------------------------------
             ;MP2990 Reserved
             ;Input: 0
MP 2990.0  : 0
MP 2990.1  : 0
MP 2990.2  : 0
MP 2990.3  : 0
MP 2990.4  : 0
MP 2990.5  : 0
             ;==============================================================
             ;MP 3000 .. 3999
             ;Spindle control (1st spindle)
             ;==============================================================
             ;MP3010 Output of spindle speed, oriented spindle stop
             ;Input:
             ; 0 = No output of spindle speed
             ; 1 = Encoded output of spindle speed only
             ;     if the speed changes
             ; 2 = Encoded output of spindle speed at each Tool Call
             ; 3 = Analog output of spindle speed, but
             ;     gear changing signal only if the gear range changes
             ; 4 = Analog output of spindle speed, but
             ;     gear changing signal at every Tool Call
             ; 5 = Analog output of spindle speed and no
             ;     gear changing signal
             ; 6 = Same as 3, but with closed-loop spindle for
             ;     orientation
             ; 7 = Same as 4, but with closed-loop spindle for
             ;     orientation
             ; 8 = Same as 5, but with closed-loop spindle for
             ;     orientation
MP 3010    : 8
             ;--------------------------------------------------------------
             ;MP3011 Function of analog output s if MP3010 < 3
             ;Input: 0 = No special function (0 volt)
             ;       1 = Voltage proportional to the current machining feed
             ;           rate (depending on MP3012)
             ;       2 = Voltage is defined via the PLC module 9130
             ;       3 = Voltage is defined through M functions
             ;           (M200 to M204)
MP 3011    : 0
             ;--------------------------------------------------------------
             ;MP3012 Feed rate from output of an analog voltage of 10 V
             ;(MP3011 = 1)
             ;Input: 0 to 300 000 [mm/min]
             ;Laser power control M200-M204
MP 3012    : 0          ;0 to 30 000 m/min = 10 volts
             ;--------------------------------------------------------------
             ;MP3013.0 to MP3013.11 Characteristic curve kink points for
             ;output of the analog voltage with M202
             ;Input: 10 to 300 000 [mm/min]
MP 3013.0  : 0
MP 3013.1  : 0
MP 3013.2  : 0
MP 3013.3  : 0
MP 3013.4  : 0
MP 3013.5  : 0
MP 3013.6  : 0
MP 3013.7  : 0
MP 3013.8  : 0
MP 3013.9  : 0
MP 3013.10 : 0
MP 3013.11 : 0
             ;--------------------------------------------------------------
             ;MP3014.0 to MP3014.11 Characteristic curve kink points for
             ;output of the analog voltage with M202
             ;Input: 0.000 to 9.999 [V]
MP 3014.0  : 0
MP 3014.1  : 0
MP 3014.2  : 0
MP 3014.3  : 0
MP 3014.4  : 0
MP 3014.5  : 0
MP 3014.6  : 0
MP 3014.7  : 0
MP 3014.8  : 0
MP 3014.9  : 0
MP 3014.10 : 0
MP 3014.11 : 0
             ;--------------------------------------------------------------
             ;MP3020 Definition of speed range of BCD spindle
             ;Input: 0 to 99 999
             ;Limitation of speed codes (no limitation with 01991)
             ;    1* increment (1..9)
             ;+  10* max. code (1..99)
             ;+1000* min. code (1..99)
MP 3020    : 1991
             ;--------------------------------------------------------------
             ;MP3030 Axis stop with a Tool Call in which only a
             ;spindle speed is transferred
             ;Input: %xx
             ;Bit 0   Axis stops with Tool Call
             ;        0 = Active
             ;        1 = Inactive
             ;Bit 1   Regulate to speed 0 when switching to another
             ;        gear range
             ;        0 = Active
             ;        1 = Inactive
MP 3030    : %00
             ;--------------------------------------------------------------
             ;MP3120 Speed 0 permitted
             ;Input: 0: S = 0 permitted
             ;       1: S = 0 not permitted
MP 3120    : 0
             ;--------------------------------------------------------------
             ;MP3130 Polarity of speed setpoint for the spindle
             ;Input: 0 = M03 positive voltage; M04 negative voltage
             ;       1 = M03 negative voltage; M04 positive voltage
             ;       2 = M03 and M04 positive voltage
             ;       3 = M03 and M04 negative voltage
MP 3130    : 1
             ;--------------------------------------------------------------
             ;MP3140 Counting direction of the encoder signals for the
             ;       spindle
             ;Input: 0 = Positive counting direction with M03
             ;       1 = Negative counting direction with M03
MP 3140    : 1
             ;--------------------------------------------------------------
             ;MP3142 Line count of the rotary encoder on the spindle
             ;Input: 100 to 100 000 [lines]
MP 3142    : 2048
             ;--------------------------------------------------------------
             ;MP3143 Mounting configuration of the spindle position encoder
             ;Input: 0 = Direct
             ;       1 = Transmission (reference pulse from input X30)
             ;       2 = Transmission (reference end position of input X30
             ;           and reference pulse from encoder, not possible if
             ;           transmission is located between ROD and spindle)
             ;       3 = Transmission (2nd reference pulse from input X30)
             ;       4 = Referencing via EnDat (not possible if
             ;           transmission is located between ROD and spindle)
             ;       5 = like input = 0, but evaluation of transmission
MP 3143    : 3
             ;--------------------------------------------------------------
             ;MP3210 S digital: Motor revolutions at rated speed
             ;Input: 0 to 100 [1000/min]
             ;
             ;       S analog : S analog voltage at rated speed
             ;Input: 0 to 9.999 [V]
MP 3210.0  : 7.5        ;1st gear range
MP 3210.1  : 0          ;2nd gear range
MP 3210.2  : 0          ;3rd gear range
MP 3210.3  : 0          ;4th gear range
MP 3210.4  : 0          ;5th gear range
MP 3210.5  : 0          ;6th gear range
MP 3210.6  : 0          ;7th gear range
MP 3210.7  : 0          ;8th gear range
             ;--------------------------------------------------------------
             ;MP3240.1 S digital: Minimum possible output speed of motor
             ;Input: 0 to 9.999 [1000/min]
             ;
             ;       S analog : Minimum possible S analog output voltage
             ;Input: 0 to 9.999 [V]
MP 3240.1  : 0.01
             ;--------------------------------------------------------------
             ;MP3240.2 S digital: Motor speed for gear changing
             ;Input: 0 to 9.999 [1000/min]
             ;
             ;       S analog : Jog voltage for gear changing
             ;Input: 0 to 9.999 [V]
MP 3240.2  : 0.005
             ;--------------------------------------------------------------
             ;MP3310 S override limitation
             ;Input: 0 to 100 [%]
MP 3310.0  : 150        ;Maximum
MP 3310.1  : 0          ;Minimum
             ;--------------------------------------------------------------
             ;MP3350 Maximum permissible overshoot of the spindle rpm for
             ;       monitoring the spindle rpm, as a percentage of the
             ;       nominal speed value
             ;Input: 0 to 100 [%]
MP 3350    : 0
             ;--------------------------------------------------------------
             ;MP3351 Entry of an absolute value for the permissible
             ;       overshoot of the rpm. This value is only used if the
             ;       absolute value resulting from MP3350 is less than the
             ;       the value entered for MP3351. (for low nominal speeds)
             ;Input: 0,001 to 100000,000 [rpm]
             ;       0 = Monitoring deactivated
MP 3351    : 0
             ;--------------------------------------------------------------
             ;MP3411 Ramp gradient of spindle
             ;Input: S digital: 0 to 1.999 [(1000/min)/ms]
             ;       S analog : 0 to 1.999 [V/ms]
MP 3411.0  : 0.004
MP 3411.1  : 0.001
MP 3411.2  : 0.001
MP 3411.3  : 0.001
MP 3411.4  : 0.001
MP 3411.5  : 0.001
MP 3411.6  : 0.001
MP 3411.7  : 0.001
             ;--------------------------------------------------------------
             ;MP3412 Multiplier for MP3411
             ;Input: 0.000 to 1.999
MP 3412.0  : 0.8        ;With M05
MP 3412.1  : 0.6        ;For oriented spindle stop
MP 3412.2  : 0.6        ;For tapping
MP 3412.3  : 0.6        ;For rigid tapping
             ;--------------------------------------------------------------
             ;MP3415 Spindle overshoot behavior
             ;Input: 0 to 1000 [ms]
MP 3415.0  : 300        ;With M03, M04, M05
MP 3415.1  : 500        ;For oriented spindle stop
MP 3415.2  : 100        ;For tapping
MP 3415.3  : 500        ;For rigid tapping
             ;--------------------------------------------------------------
             ;MP3420 Positioning window for the spindle
             ;Input: 0 to 360 [degrees]
MP 3420    : 0.5
             ;--------------------------------------------------------------
             ;MP3430 Deviation of the reference mark from the desired
             ;position (spindle preset)
             ;Input: 0 to 360 [degrees]
MP 3430    : 0
             ;--------------------------------------------------------------
             ;MP3440 kv factor for spindle orientation
             ;Input: 0.1 to 10 [1000/min]
MP 3440.0  : 1          ;1st gear range
MP 3440.1  : 1          ;2nd gear range
MP 3440.2  : 1          ;3rd gear range
MP 3440.3  : 1          ;4th gear range
MP 3440.4  : 1          ;5th gear range
MP 3440.5  : 1          ;6th gear range
MP 3440.6  : 1          ;7th gear range
MP 3440.7  : 1          ;8th gear range
             ;--------------------------------------------------------------
             ;MP3450 Spindle motor revolutions
             ;Input: 0 to 65535 [rpm]
             ;       0 no transmission
MP 3450.0  : 3          ;1st gear range
MP 3450.1  : 0          ;2nd gear range
MP 3450.2  : 0          ;3rd gear range
MP 3450.3  : 0          ;4th gear range
MP 3450.4  : 0          ;5th gear range
MP 3450.5  : 0          ;6th gear range
MP 3450.6  : 0          ;7th gear range
MP 3450.7  : 0          ;8th gear range
             ;--------------------------------------------------------------
             ;MP3451 Spindle revolutions
             ;Input: 0 to 65535 [rpm]
             ;       0 no transmission
MP 3451.0  : 4          ;1st gear range
MP 3451.1  : 0          ;2nd gear range
MP 3451.2  : 0          ;3rd gear range
MP 3451.3  : 0          ;4th gear range
MP 3451.4  : 0          ;5th gear range
MP 3451.5  : 0          ;6th gear range
MP 3451.6  : 0          ;7th gear range
MP 3451.7  : 0          ;8th gear range
             ;--------------------------------------------------------------
             ;MP3510 Rated speed for gear ranges
             ;Input: 0 to 99 999.999 [rpm]
MP 3510.0  : 10000      ;1st gear range
MP 3510.1  : 0          ;2nd gear range
MP 3510.2  : 0          ;3rd gear range
MP 3510.3  : 0          ;4th gear range
MP 3510.4  : 0          ;5th gear range
MP 3510.5  : 0          ;6th gear range
MP 3510.6  : 0          ;7th gear range
MP 3510.7  : 0          ;8th gear range
             ;--------------------------------------------------------------
             ;MP3515 Maximum spindle speed
             ;Input: 0 to 99 999.999 [rpm]
MP 3515.0  : 10000      ;1st gear range
MP 3515.1  : 0          ;2nd gear range
MP 3515.2  : 0          ;3rd gear range
MP 3515.3  : 0          ;4th gear range
MP 3515.4  : 0          ;5th gear range
MP 3515.5  : 0          ;6th gear range
MP 3515.6  : 0          ;7th gear range
MP 3515.7  : 0          ;8th gear range
             ;--------------------------------------------------------------
             ;MP3520.0 Speed activation through marker M4011
             ;Input: 0 to 99 999.999 [rpm]
MP 3520.0  : 100
             ;--------------------------------------------------------------
             ;MP3520.1 Spindle speed for oriented spindle stop
             ;Input: 0 to 99 999.999 [rpm]
MP 3520.1  : 500
             ;--------------------------------------------------------------
             ;MP3530 Increased spindle power for roughing
             ;Input: 0 = Inactive
             ;       1 = increased spindle power for roughing
MP 3530    : 0
             ;--------------------------------------------------------------
             ;MP3540 Allowed undershooting of the actual value of the
             ;       spindle speed. Defined via an percentage-factor of the
             ;       nominal value of the spindel speed.
             ;Input: 0 to 100 [%]
             ;       0 = Monitoring inactive
MP 3540    : 0
             ;--------------------------------------------------------------
             ;MP3542 Minimum value of the spindle speed at which the
             ;       monitoring from MP3540 becomes active.
             ;Input: 0.001 to 99 999.999 [rpm]
             ;       0 = Monitoring inactive
MP 3542    : 0
             ;--------------------------------------------------------------
             ;MP3550 Delay of Emergency-Stop reaction of the spindle
             ;Input: 0.001 to 0.100 [s]
             ;       0 = Delay inactive
MP 3550    : 0
             ;==============================================================
             ;MP 4000 .. 4999
             ;Integral PLC
             ;==============================================================
             ;MP4000 Options for the conditional compilation of the PLC
             ;       program
             ;Input: String
             ;
MP 4000.0  : 
             ;
             ;MP4000.1: 0, 4, 5, 6, 9, 10, 11, 15
             ; Function for nominal position as actual position of
             ; 4th/5th/6th axis
             ; 0: Function disable
             ; 4: Function enable for 4th axis
             ; 5: Function enable for 5th axis
             ; 6: Function enable for 6th axis
             ; 9: Function enable for 4th & 5th axes
             ;10: Function enable for 4th & 6th axes
             ;11: Function enable for 5th & 6th axes
             ;15: Function enable for 4th, 5th,& 6th axes
MP 4000.1  : 0
             ;
             ;MP4000.2: 0 - 9
             ; Safety mode
             ; MP4000.2 =                0 1 2 3 4 5 6 7 8 9
             ;
             ; Without safety mode       X   X X X X X X X
             ; Standard safety mode        X
             ; DMx safety mode                             X
MP 4000.2  : 1
             ;
             ;MP4000.3: 0 - 9
             ; Working area monitoring
             ; MP4000.3 =                            0 1 2 3 4 5 6 7 8 9
             ;
             ; Without working area monitoring       X     X X X X X X X
             ; Square rotary table protection          X
             ; Rectangular rotary table protection       X
MP 4000.3  : 0
             ;
             ;MP4000.4: 0 - 14
             ; Machine panel, handwheel panel, and additional panels
             ;                                                      1 1 1 1 1
             ; MP4000.4 =                      |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|
             ;                                 | | | | | | | | | | | | | | | |
             ; reserved                        | | |X| | | | | | |X|X| |X|X| |
             ;                                 | | | | | | | | | | | | | | | |
             ; ===machine panel===============================================
             ; - MB620                         |X| | |X| | | |X| | | | | | | |
             ; - MB720                         | |X| | |X| | | |X| | |X| | |X|
             ; - TE745                         | | | | | |X|X| | | | | | | | |
             ; - spindle start/stop            |X|X| | | | | | | | | |X| | | |
             ; - M03/M04/M05                   | | | |X|X|X|X|X|X| | | | | |X|
             ; - override knobs                |2|2| |2|2|3|2|2|2| | |2| | |2|
             ; - S% -> FMAX% override knob     | | | | | | | | | | | |X| | |X|
             ; - S+%/S100%/S-%                 | | | | | | | | | | | |X| | |X|
             ; ===handwheel panel=============================================
             ; - NC start/NC stop/S start      |X|X| | | | | | | | | |X| | | |
             ; - NC start/NC stop/S stop       | | | |X|X|X|X| |X| | | | | |X|
             ; ===additional panels===========================================
             ; - IPC 61x0 + TE6x0              | | | | | | | |X| | | | | | | |
             ; - wireless handwheel            | | | | | | | |X| | | | | | | |
             ; - additional MB620              | | | | | | | |X| | | | | | | |
             ; - additional NC START/STOP      | | | | | | | | |X| | | | | | |
             ; - door lock/safety confirmation | | | | | | | | |X| | | | | | |
             ; - light beam protection         | | | | | | | | |X| | | | | | |
             ; ===============================================================
MP 4000.4  : 4
             ;
             ;MP4000.5: 0 - 8
             ; Hydraulic System
             ; MP4000.5 =                             0|1|2|3|4|5|6|7|8
             ;                                         | | | | | | | |
             ; Without hydraulic system               X| | | | | | | |
             ; ========================================================
             ; Hydraulic motor 1
             ;   motor overload signal                 |X|X|X|X|X|X|X|X
             ;   oil pressure switch                   |X|X|X|X|X|X| |X
             ;   oil level switch                      |X|X| |X| | | |
             ;   oil filter sensor                     | | |X| | | | |
             ;   oil temperature sensor                | | | | | | | |
             ;   valve high/low pressure sensor        | | | |X| | | |
             ;   gravity axis counter-balancing        |X| | |X| | | |
             ;   motor on/off by oil pressure          | | |X| |X| | |
             ;   motor on/off by oil pressure & timer  | | | | | |X| |
             ; ========================================================
             ; Hydraulic motor 2
             ;   motor overload signal (NOTE1)         | | | | | | | |X
             ;   oil pressure switch                   | | | | | | | |X
             ;   oil level switch                      | | | | | | | |
             ;   gravity axis counter-balancing        | | | | | | | |X
             ;   gravity axis over pressure sensor     | | | | | | | |X
             ;   tool & axes clamping/unclamping       | | | | | | | |X
             ;   motor on/off by oil pressure          | | | | | | | |X
             ;
             ; NOTE1: Serial connected with hydraulic motor 1
MP 4000.5  : 0
             ;
             ;MP4000.6: 0 - 5
             ; Guard Locking Device
             ; (Only for TVMC/MB series with machining area)
             ; MP4000.6 =                     00 01 02 03 04 05
             ;
             ; Without guard locking device    X        X  X  X
             ; ELAN                               X
             ; EUCHNER                               X
MP 4000.6  : 0
             ;
             ;MP4000.7: 0 - 5
             ; Automatic Door System
             ; MP4000.7 =                     |0|1|2|3|4|5
             ;                                | | | | | |
             ; Without automatic door system  |X| |X|X|X|X
             ; TVMC                           | |X| | | |
             ;                                | | | | | |
             ; ============================================
             ; mechanical safety protection   | |X| | | |
             ; optical safety protection      | | | | | |
             ; ============================================
MP 4000.7  : 0
             ;
             ;MP4000.8: 0 - 11
             ; 1st Axis Lubrication System
             ;
             ; MP4000.8 =                   00 01 02 03 04 05 06 07 08 09 10 11
             ;
             ; Without lubrication system    X              X  X  X  X  X
             ; Oil lubrication (simple)         X     X
             ; Oil lubrication (pressure)          X     X
             ; Grease lubrication (pressure)                               X  X
             ;
             ; Pressure switch                     X     X                 X  X
             ; Level switch                        X     X
             ; For X, Y, and Z axes             X  X                       X
             ; For X axis only                        X  X                    X
MP 4000.8  : 2
MP 4000.9  : 
             ;
             ;MP4000.10: 0 - 299
             ; High-speed Spindle System
             ;
             ; rear bearing temperature monitoring -------------------+
             ; CYTEC CYCON control ---------------------------------+ |
             ; rotary joint cooling circulation control ----------+ | |
             ; tool clamped -> solenoid off --------------------+ | | |
             ; analog tool unclamped sensor ------------------+ | | | |
             ; analog tool clamped sensor ------------------+ | | | | |
             ; Grease/Oil-air lubrication (G/O) ----------+ | | | | | |
             ; Digital/Analog drive (D/A) --------------+ | | | | | | |
             ; Asynchronous/Synchronous motor (A/S) --+ | | | | | | | |
             ; MP4000.10 = ------------------------+  | | | | | | | | |
             ;                                     |  | | | | | | | | |
             ; Without high-speed spindle ....... 000|-|-|-|-|-|-|-|-|-|
             ; NEO (INDRAMAT 1MB160) ............ 001|A|D|G| | | | | | |
             ; NEO (INDRAMAT MBS162) ............ 019|S|D|G| | | | | | |
             ; NEO (INDRAMAT MBS162/MBS202) ..... 020|S|D|G|X|X| | | | |
             ; WEISS asynchronous motor (reserved)021|A| | | | | | | | |
             ; WEISS 2SP1204-1Hxxx -xxD or -xxH . 039|S|D|G| | | | | | |
             ; WEISS 2SP1204-1Hxxx -xxF or -xxG . 040|S|D|G|X|X| | | | |
             ; PERON SPEED TCV-6-SP/DIS-671-00 .. 041|A|D|G|X|X| | | | |
             ; PERON SPEED TCV-6-SP/DIS-2457-00 . 041|A|D|G|X|X| | | | |
             ; PERON SPEED TCV-3/DIS-1583 ....... 042|A|D|O| | | | | | |
             ; PERON SPEED TCV-230-SP/DIS-2375-00 043|A|D|G| | | | | | |
             ; PERON SPEED TCV-6-SP/DIS-2611-00 . 043|A|D|G| | | | | | |
             ; PERON SPEED TCV-320-SP/DIS-2652-00 051|A|D|G|X|X| | | |X|
             ; PERON SPEED TCV-238-SP/DIS-1198 .. 060|S|D|G| | | | | | |
             ; CYTEC CS/xxxx/021W/120F-xxxx ..... 061|A|D|G| | |X| |X| |
             ; CYTEC CS/xxxx/021W/120F-xxxx ..... 062|A|D|G|X| |X|X| | |
             ; CYTEC CS/xxxx/021W/180F-xxxx ..... 080|A|D|G|X| |X| | | |
             ; CYTEC CS/xxxx/027W/120F-xxxx ..... 080|S|D|G|X| |X| | | |
             ; CYTEC CS/xxxx/034W/100F-xxxx ..... 080|S|D|G|X| |X| | | |
             ; CYTEC CS/xxxx/041W/070F-xxxx ..... 080|S|D|G|X| |X| | | |
             ; CYTEC CS/xxxx/042W/075F-xxxx ..... 080|A|D|G|X| |X| | | |
             ; CYTEC CS/xxxx/050W/150F-xxxx ..... 080|A|D|G|X| |X| | | |
             ; CYTEC CS/xxxx/034W/070F-xxxx ..... 081|S|D|G| | |X| |X| |
             ; ROYAL RE4064 (1FE1083-4WN11) ..... 100|S|D|G| | | | | | |
             ; MICROLAB M3-LMS-CTS24 ............ 191|A|D|G| | | | | | |
             ; FAEMAT FA  160 C.U.V. ............ 296|A|D|G| | | | | | |
             ; FAEMAT FAI 250 C.U.V. ............ 298|A|D|O| | | | | | |
             ; FAEMAT FA  110 CUS ............... 299|A|A|O| | | | | | |
MP 4000.10 : 0
             ;
             ;MP4000.11: 0 - 7
             ; Coolant Through Spindle Tank System
             ;
             ; MP4000.11 =                            00 01 02 03 04 05 06 07
             ;
             ; Without CTS tank                        X        X
             ; CTS tank <= 20 bar (EUMACH)                X
             ; CTS tank <= 20 bar (EUMACH)                   X
             ; CTS tank > 20 bar  (LIS)/(CS)                       X
             ; CTS tank < 70 bar  (LIS)/(CS)                          X  X  X
             ;
             ; CTS level overflow sensor                              X  X  X
             ; CTS level high sensor                      X  X     X  X  X  X
             ; CTS level low sensor                       X  X     X  X  X  X
             ; CTS tank inlet pressure sensor (NOTE1)              X  X  X  X
             ; M08 tank to CTS tank motor                 X  X     X  X  X  X
             ; Coolant refilled while M07 (NOTE2)            X     X
             ; M08 high pressure motor on CTS tank                       X
             ; M08 normal pressure motor on CTS tank                        X
             ;
             ; NOTE1: When paper filter device is used, there will be no CTS
             ;        tank inlet pressure sensor.
             ; NOTE2: In this case, the overflow pipe position is lower than
             ;        the CTS level high sensor but higher than the CTS level
             ;        low sensor.  For other case, the overflow pipe position
             ;        is higher than the CTS level high sensor.
MP 4000.11 : 0
             ;
             ;MP4000.12: 0 - 10
             ; Additional Spindle Head
             ;
             ; V/H head: vertical/horizontal spindle head
             ;
             ; MP4000.12 =                  00 01 02 03 04 05 06 07 08 09 10
             ;
             ; Without additional S head     X        X  X        X  X
             ; ITRI automatic V/H head (45D)    X  X
             ; Gong Yang manual S head (90D)                X
             ; Emena manual S head (45D)                       X
             ; SEMPUCO manual S head (45D)                              X
             ; ITRI manual S head (45D)                                    X
             ;
             ; M03/M04 direction(NOTE1)         +  +        +  -        -  +
             ; tool clamped sensor(NOTE2)      (X)(X)                     (X)
             ; tool unclamped sensor(NOTE2)     X  X          (X)       X  X
             ; tool clamping valve(NOTE2)      (X)(X)         (X)      (X)(X)
             ; tool unclamping valve            X  X           X        X  X
             ; head coupling clamped sensor     X  X
             ; head coupling unclamped sensor   X  X
             ; head coupling clamping valve     X  X
             ; head coupling unclamping valve   X  X        X
             ;
             ; encoder on B axis of V/H head       X
             ;
             ; NOTE1:
             ;   +: M03/M04 direction same for with head and without head
             ;   -: M03/M04 direction reversed for with head and without head
             ; NOTE2:
             ;   (): sensor or valve may not existing
MP 4000.12 : 0
MP 4000.13 : 
MP 4000.14 : 
             ;
             ;MP4000.15: Thermal compensation function
             ; 0: BASIC PROGRAM linear thermal compensation with machine
             ;    parameters
             ; 1: PMC thermal compensation module (8-bit for data +
             ;    1-bit for +/- sign)
             ; 2: PMC thermal compensation module (11-bit for data +
             ;    1-bit for +/- sign)
             ; 5: BASIC PROGRAM thermal compensation table
MP 4000.15 : 0
MP 4000.16 : 
MP 4000.17 : 
MP 4000.18 : 
MP 4000.19 : 
MP 4000.20 : 
MP 4000.21 : 
MP 4000.22 : 
             ;
             ;MP4000.23: 0 - 11
             ; 2nd Axis Lubrication System
             ;
             ; MP4000.23 =                  00 01 02 03 04 05 06 07 08 09 10 11
             ;
             ; Without lubrication system    X        X  X  X  X  X  X  X
             ; Oil lubrication (simple)         X
             ; Oil lubrication (pressure)          X
             ; Grease lubrication (pressure)                               X  X
             ;
             ; Pressure switch                     X                       X  X
             ; Level switch                        X
             ; For Y axis only                  X  X                       X
             ; For Y and Z axes                                               X
MP 4000.23 : 0
             ;MP4000.24: 0 - 11
             ; 3rd Axis Lubrication System
             ;
             ; MP4000.24 =                  00 01 02 03 04 05 06 07 08 09 10 11
             ;
             ; Without lubrication system    X        X  X  X  X  X  X  X
             ; Oil lubrication (simple)         X
             ; Oil lubrication (pressure)          X
             ; Grease lubrication (pressure)                               X  X
             ;
             ; Pressure switch                     X                       X  X
             ; Level switch                        X
             ; For Z axis only                  X  X                       X
             ; For V axis (axis 7) only                                       X
MP 4000.24 : 0
MP 4000.25 : 
MP 4000.26 : 
MP 4000.27 : 
MP 4000.28 : 
             ;
             ;MP4000.29: 0 - 5
             ; OEM cycle
             ; MP4000.29 =                00 01 02 03 04 05
             ;
             ; Without OEM cycle           X     X  X  X  X
             ; OEM cycle 332                  X
MP 4000.29 : 0
             ;
             ;MP4000.30: 0 - 12
             ; Tool Measuring System
             ; MP4000.30 =                      00 01 02 ... 10 11 12
             ;
             ; HEIDENHAIN TT probe system        X            X
             ; BLUM laser system                    X            X
             ; RENISHAW laser system                   X            X
             ;
             ; out of machining traverse(NOTE1)               X  X  X
             ; protected by ATC door                          X  X  X
             ;
             ;NOTE1: Traverse range 1 (LIMIT 1) is for machining traverse,
             ;       traverse range 3 (LIMIT 3) is for tool measuring travese.
MP 4000.30 : 0
             ;
             ;MP4000.31: 0 - 10
             ; Soft Key Menu
             ;                                                        1 1
             ; MP4000.31 =                        0|1|2|3|4|5|6|7|8|9|0|1
             ; With tool loading SK----------------|-|-|-|-|-|-|-|-|-|-|-
             ; - standard                         X| | | | | | | | | | |
             ; - 3D head                           |X| | | | | | | | | |
             ; - TVMC                              | |X| | | | | | | | |
             ; - reserved                          | | |X| | | | | | | |
             ; - EBM with working area monitoring  | | | |X| | | | | | |
             ; - reserved                          | | | | |X| | | | | |
             ; Without tool loading SK-------------|-|-|-|-|-|-|-|-|-|-|-
             ; - standard                          | | | | | |X| | | | |
             ; - 3D head                           | | | | | | |X| | | |
             ; - TVMC/MB with partition            | | | | | | | |X| | |
             ; - DVM with additional manual head   | | | | | | | | |X| |
             ; - EBM with working area monitoring  | | | | | | | | | |X|
             ; - TVMC/MB with partition (dual TC)  | | | | | | | | | | |X
MP 4000.31 : 0
MP 4000.32 : 
MP 4000.33 : 
MP 4000.34 : 
MP 4000.35 : 
MP 4000.36 : 
MP 4000.37 : 
MP 4000.38 : 
MP 4000.39 : 
             ;
             ;MP4000.40: 0, 1, 2, 3, 23, 24, 25, 26
             ; Feed Motor Fan Overloaded Message
             ; 0: Without message
             ; 1: Message for X axis (1st overloaded signal)
             ; 2: Message for Y axis (1st overloaded signal)
             ; 3: Message for Z axis (1st overloaded signal)
             ;23: Message for X & Y axes (1st&2nd overloaded signals)
             ;24: Message for X & Z axes (1st&2nd overloaded signals)
             ;25: Message for Y & Z axes (1st&2nd overloaded signals)
             ;26: Message for X, Y, & Z axes (1st&2nd&3rd overloaded signals)
MP 4000.40 : 0
             ;
             ;MP4000.41: 0, 51, 52
             ; Message for axis location before traverse reference point
             ; Only for axis reference deceleration signal not at the end of
             ; traverse range
             ; Example: Tilting axis reference deceleration signal
             ; 0: Without message
             ;51: Message for 5th axis as A-axis
             ;52: Message for 5th axis as B-axis
MP 4000.41 : 0
MP 4000.42 : 
             ;
             ;MP4000.43: Custom kinematics configuration
             ;-1: Function inactive
MP 4000.43 : -1
             ;
             ;MP4000.44: 0/1
             ; 4th axis clamping mode on/off at power-on
             ; For Hirth coupling axis (MP420.4 = 1), MP4000.44 must be 0.
             ; 0: 4th axis clamping mode on at power-on (axis clamping)
             ; 1: 4th axis clamping mode off at power-on (axis unclamping)
MP 4000.44 : 0
             ;
             ;MP4000.45: 0/1
             ; 5th axis clamping mode on/off at power-on
             ; For Hirth coupling axis (MP420.5 = 1), MP4000.45 must be 0.
             ; 0: 5th axis clamping mode on at power-on (axis clamping)
             ; 1: 5th axis clamping mode off at power-on (axis unclamping)
MP 4000.45 : 0
             ;
             ;MP4000.46: 0/1
             ; 6th axis clamping mode on/off at power-on
             ; For Hirth coupling axis (MP420.6 = 1), MP4000.46 must be 0.
             ; 0: 6th axis clamping mode on at power-on (axis clamping)
             ; 1: 6th axis clamping mode off at power-on (axis unclamping)
MP 4000.46 : 0
MP 4000.47 : 
MP 4000.48 : 
MP 4000.49 : 
MP 4000.50 : 
MP 4000.51 : 
MP 4000.52 : 
MP 4000.53 : 
             ;
             ;MP4000.54:
             ; PLB62xx 4th Slot Assignment (MP4000.54)
             ; PLB6204 4th slot assignment
             ;   Device    |Empty   |PLD-H 08-16-00
             ;   ----------|--------|--------------
             ;   MP4000.54 |6204-4=0|6204-4=8I16O
             ;
             ; PLB6206 4th slot assignment
             ;   Device    |Empty   |PLD-H 08-16-00
             ;   ----------|--------|--------------
             ;   MP4000.54 |6206-4=0|6206-4=8I16O
             ;
             ; PLB6208 4th slot assignment
             ;   Device    |Empty   |PLD-H 08-16-00
             ;   ----------|--------|--------------
             ;   MP4000.54 |6208-4=0|6208-4=8I16O
MP 4000.54 : 6206-4=8I16O
             ;
             ;MP4000.55:
             ; PLB62xx 5th Slot Assignment (MP4000.55)
             ; PLB6204 5th slot assignment
             ;   Device    |None
             ;   ----------|----
             ;   MP4000.55 |BLANK (no character/number allowed)
             ;
             ; PLB6206 5th slot assignment
             ;   Device    |Empty   |PLD-H 16-08-00
             ;   ----------|--------|--------------
             ;   MP4000.55 |6206-5=0|6206-5=16I8O
             ;
             ; PLB6208 5th slot assignment
             ;   Device    |Empty   |PLD-H 16-08-00
             ;   ----------|--------|--------------
             ;   MP4000.55 |6208-5=0|6208-5=16I8O
MP 4000.55 : 6206-5=0
             ;
             ;MP4000.56:
             ; PLB62xx 6th Slot Assignment (MP4000.56)
             ; PLB6204 6th slot assignment
             ;   Device    |None
             ;   ----------|----
             ;   MP4000.56 |BLANK (no character/number allowed)
             ;
             ; PLB6206 6th slot assignment
             ;   Device    |Empty   |PLD-H 16-08-00|PLA-H 08-04-04
             ;   ----------|--------|--------------|--------------
             ;   MP4000.56 |6206-6=0|6206-6=16I8O  |6206-6=8I4O4PT
             ;
             ; PLB6208 6th slot assignment
             ;   Device    |Empty   |PLD-H 16-08-00
             ;   ----------|--------|--------------
             ;   MP4000.56 |6208-6=0|6208-6=16I8O
MP 4000.56 : 6206-6=0
             ;
             ;MP4000.57:
             ; PLB62xx 7th Slot Assignment (MP4000.57)
             ; PLB6204 7th slot assignment
             ;   Device    |None
             ;   ----------|----
             ;   MP4000.57 |BLANK (no character/number allowed)
             ;
             ; PLB6206 7th slot assignment
             ;   Device    |None
             ;   ----------|----
             ;   MP4000.57 |BLANK (no character/number allowed)
             ;
             ; PLB6208 7th slot assignment
             ;   Device    |Empty   |PLD-H 16-08-00
             ;   ----------|--------|--------------
             ;   MP4000.57 |6208-7=0|6208-7=16I8O
MP 4000.57 : 
             ;
             ;MP4000.58:
             ; PLB6208 8th Slot Assignment (MP4000.58)
             ; PLB6204 8th slot assignment
             ;   Device    |None
             ;   ----------|----
             ;   MP4000.58 |BLANK (no character/number allowed)
             ;
             ; PLB6206 8th slot assignment
             ;   Device    |None
             ;   ----------|----
             ;   MP4000.58 |BLANK (no character/number allowed)
             ;
             ; PLB6208 8th slot assignment
             ;   Device    |Empty   |PLD-H 16-08-00|PLA-H 08-04-04
             ;   ----------|--------|--------------|--------------
             ;   MP4000.58 |6208-8=0|6208-8=16I8O  |6208-8=8I4O4PT
MP 4000.58 : 
MP 4000.59 : 
MP 4000.60 : 
MP 4000.61 : 
MP 4000.62 : 
MP 4000.63 : 
             ;--------------------------------------------------------------
             ;MP4020 PLC functions
             ;Input: %xxxxxxxxxxxxx
             ;Bit 0   Reserved
             ;Bit 1   Reserved
             ;Bit 2   Reserved
             ;Bit 3   Reserved
             ;Bit 4   Reserved
             ;Bit 5   Double-spindle operation active
             ;Bit 6   Reserved
             ;Bit 7   Reserved
             ;Bit 8   Behavior in an EMERGENCY STOP
             ;        0 = ACTUAL/NOMINAL values are transferred
             ;        1 = Return to Contour is activated (as with M4580)
             ;Bit 9   Simulated keys
             ;        0 = Are transferred directly to the NC
             ;        1 = Are processed through an active PLC window
             ;            before being transferred to the NC
             ;Bit 10  Disabled keys
             ;        0 = Are transferred to the active PLC window only
             ;        1 = Are transferred to neither the NC
             ;            nor the active PLC window
             ;Bit 11  PLC counter in MP4120.x
             ;        0 = Input in PLC cycles
             ;        1 = Input in seconds
             ;Bit 12  Font size in PLC window
             ;        0 = Automatic adaptation of font size to screen
             ;        1 = Font size for BF 120
             ;Bit 13  Reserved (set to 0)
             ;Bit 14  Configuration of the PLC module interface
             ;        0 = 14 axes plus spindle
             ;            Axes: bit 0 to bit 14, spindle: bit 15
             ;        1 = 18 axes plus spindle (only in HSCI systems)
             ;            Axes: bit 0 to bit 17, spindle: bit 31
             ;Bit 15  Saving the changes made to tables to the HDR
             ;        0 = Cyclic saving of tables
             ;            (same behavior as up to and including 60642x01 SP4)
             ;        1 = Save the tables each time they are closed
             ;Bit 16  Resetting behavior of the strobe markers
             ;        0 = Do not reset the strobe marker until the action
             ;            initiated by acknowledging the strobe has been
             ;            completed by the NC.
             ;        1 = Reset the strobe marker immediately upon
             ;            acknowledgement(same behavior as up to and
             ;            including 60642x-01 SP4)
             ;Bit 17  Message "Debug functions of the PLC are active"
             ;        0 = Message is triggerd
             ;        1 = Suppress the message if M4753 = 1 has triggered
             ;            this message
             ;11111111----------
             ;765432109876543210 <- bit #
MP 4020    : %000000100000000000
MP 4030.0  : 0
MP 4030.1  : 1
MP 4030.2  : 2
MP 4030.3  : 3
MP 4031    : -1
             ;--------------------------------------------------------------
             ;MP4040 Set a PLC output after shutdown
             ;       PLC output definied in MP4044
             ;Input: 0 = Do not set a PLC output
             ;       1 = Only after shutdown via Module 9279
             ;       2 = After shutdown via Module 9279 or soft key
MP 4040    : 1
             ;--------------------------------------------------------------
             ;MP4041 Time after shutdown of the control until
             ;       setting the PLC output from MP4044
             ;Input: 0 to 1000 [s]
MP 4041    : 30
MP 4042    : 25
             ;--------------------------------------------------------------
             ;MP4043 Delay time at shutdown to do final PLC actions
             ;Input: 1 to 1000 [s]
             ;       0 = No delay time
MP 4043    : 0
             ;--------------------------------------------------------------
             ;MP4044 PLC output to be set after shut down.
             ;       On controls with HSCI
             ;Input: symbolic name of IO configurator
MP 4044    : O222
             ;--------------------------------------------------------------
             ;MP4050 Pulse for path-dependent lubrication
             ;Input: 0 to 99999.999 [m]
MP 4050.0  : 8
MP 4050.1  : 8
MP 4050.2  : 8
MP 4050.3  : 99999.999
MP 4050.4  : 99999.999
MP 4050.5  : 99999.999
MP 4050.6  : 30
MP 4050.7  : 30
MP 4050.8  : 30
MP 4050.9  : 30
MP 4050.10 : 30
MP 4050.11 : 30
MP 4050.12 : 30
MP 4050.13 : 30
MP 4060.0  : +30
MP 4060.1  : +30
MP 4060.2  : +30
MP 4060.3  : +30
MP 4061.0  : 0
MP 4061.1  : 0
MP 4061.2  : 0
MP 4061.3  : 0
             ;--------------------------------------------------------------
             ;MP4070 Compensation amount per PLC cycle for lagged-tracking
             ;axis-error compensation (thermal compensation)
             ;Input: 0.0001 to 0.005 [mm]
MP 4070    : 0.0001
             ;--------------------------------------------------------------
             ;MP4110 Run time for PLC Timers T0..T47
             ;Input [s]
MP 4110.0  : 0.25       ;Timer 250ms
MP 4110.1  : 0
MP 4110.2  : 0.5        ;Keys pulse identifier
MP 4110.3  : 0
MP 4110.4  : 0
MP 4110.5  : 0
MP 4110.6  : 0
MP 4110.7  : 0.25       ;Spindle delayed stop (automatic mode)
MP 4110.8  : 0.5        ;Switch off axes -t-> axis monitoring off/on
MP 4110.9  : 4          ;Servo drives ready
MP 4110.10 : 10         ;Spindle speed change
MP 4110.11 : 0.25       ;Servo drive release spindle
MP 4110.12 : 10         ;Timeout gear
MP 4110.13 : 0.4        ;Gear jog, general timer
MP 4110.14 : 0.2        ;Gear jog cw
MP 4110.15 : 0.2        ;Tool magazine, definite stop
MP 4110.16 : 40         ;Tool magazine timeout
MP 4110.17 : 0          ;Tool magazine, general timer
MP 4110.18 : 30         ;Tool changer timeout
MP 4110.19 : 0.2        ;Tool changer, general timer
MP 4110.20 : 15         ;1st lubrication, general
MP 4110.21 : 30         ;1st lubrication, pressure generation
MP 4110.22 : 5          ;Hydraulic system, pressure generation
MP 4110.23 : 10         ;Timeout alternating table
MP 4110.24 : 1          ;Start index table
MP 4110.25 : 0.25       ;Tool changing arm middle position -t-> stop
MP 4110.26 : 50         ;Pallet changer timeout
MP 4110.27 : 0.15       ;Pallet changer general timer
MP 4110.28 : 10         ;3D head timeout
MP 4110.29 : 0.5        ;3D head, general timer
MP 4110.30 : 0          ;TSxxx air blow duration
MP 4110.31 : 0.2        ;Tool magazine 2, definite stop
MP 4110.32 : 40         ;Tool magazine 2 timeout
MP 4110.33 : 0          ;Tool magazine 2, general timer
MP 4110.34 : 40         ;Tool selection timeout
MP 4110.35 : 15         ;2nd lubrication, general
MP 4110.36 : 30         ;2nd lubrication, pressure generation
MP 4110.37 : 15         ;3rd lubrication, general
MP 4110.38 : 30         ;3rd lubrication, pressure generation
MP 4110.39 : 1          ;Tool changer, 2nd general timer
MP 4110.40 : 0
MP 4110.41 : 105        ;Tool changer 2 timeout
MP 4110.42 : 0.5        ;Tool changer 2, general timer (arm positioning)
MP 4110.43 : 0.2        ;Tool changer 2, 2nd timer (tool clamp/unclamp)
MP 4110.44 : 0
MP 4110.45 : 0
MP 4110.46 : 1.5        ;Control voltage on -t-> off
MP 4110.47 : 1.5        ;Control voltage off -t-> on
             ;--------------------------------------------------------------
             ;MP4111 Cycle time for PLC timer T96..T999,
             ;Start of timer with Module 9006
             ;Number of additional timers in OEM.SYS
             ;  - Keyword: NUMBERMP4111 = x ; [1..999]
             ;Input [s]
MP 4111.96 : 0          ;Unclamping 1
MP 4111.97 : 0          ;Unclamping 2
MP 4111.98 : 0.5        ;Unclamping 3
MP 4111.99 : 1          ;Unclamping 4
MP 4111.100: 0.5        ;Unclamping 5
MP 4111.101: 0.5        ;Unclamping 6
MP 4111.102: 0.5        ;Unclamping 7
MP 4111.103: 0.5        ;Unclamping 8
MP 4111.104: 0.5        ;Unclamping 9
MP 4111.105: 0.5        ;Unclamping 10
MP 4111.106: 0.5        ;Unclamping 11
MP 4111.107: 0.5        ;Unclamping 12
MP 4111.108: 0.5        ;Unclamping 13
MP 4111.109: 0.5        ;Unclamping 14
MP 4111.110: 0.5        ;Unclamping 15
MP 4111.111: 0.5        ;Unclamping 16
MP 4111.112: 0          ;Clamping 1
MP 4111.113: 0          ;Clamping 2
MP 4111.114: 0.5        ;Clamping 3
MP 4111.115: 1          ;Clamping 4
MP 4111.116: 0.5        ;Clamping 5
MP 4111.117: 0.5        ;Clamping 6
MP 4111.118: 0.5        ;Clamping 7
MP 4111.119: 0.5        ;Clamping 8
MP 4111.120: 0.5        ;Clamping 9
MP 4111.121: 0.5        ;Clamping 10
MP 4111.122: 0.5        ;Clamping 11
MP 4111.123: 0.5        ;Clamping 12
MP 4111.124: 0.5        ;Clamping 13
MP 4111.125: 0.5        ;Clamping 14
MP 4111.126: 0.5        ;Clamping 15
MP 4111.127: 0.5        ;Clamping 16
MP 4111.128: 0          ;Timeout clamping 1
MP 4111.129: 0          ;Timeout clamping 2
MP 4111.130: 1          ;Timeout clamping 3
MP 4111.131: 5          ;Timeout clamping 4
MP 4111.132: 1          ;Timeout clamping 5
MP 4111.133: 1          ;Timeout clamping 6
MP 4111.134: 1          ;Timeout clamping 7
MP 4111.135: 1          ;Timeout clamping 8
MP 4111.136: 1          ;Timeout clamping 9
MP 4111.137: 1          ;Timeout clamping 10
MP 4111.138: 1          ;Timeout clamping 11
MP 4111.139: 1          ;Timeout clamping 12
MP 4111.140: 1          ;Timeout clamping 13
MP 4111.141: 1          ;Timeout clamping 14
MP 4111.142: 1          ;Timeout clamping 15
MP 4111.143: 1          ;Timeout clamping 16
MP 4111.144: 0          ;Waiting in position 1
MP 4111.145: 0          ;Waiting in position 2
MP 4111.146: 0          ;Waiting in position 3
MP 4111.147: 0.5        ;Waiting in position 4
MP 4111.148: 0.5        ;Waiting in position 5
MP 4111.149: 0          ;Waiting in position 6
MP 4111.150: 0          ;Waiting in position 7
MP 4111.151: 0          ;Waiting in position 8
MP 4111.152: 0          ;Waiting in position 9
MP 4111.153: 0          ;Waiting in position 10
MP 4111.154: 0          ;Waiting in position 11
MP 4111.155: 0          ;Waiting in position 12
MP 4111.156: 0          ;Waiting in position 13
MP 4111.157: 0          ;Waiting in position 14
MP 4111.158: 0          ;Waiting in position 15
MP 4111.159: 0          ;Waiting in position 16
MP 4111.160: 0.5        ;Axes monitoring off - axes group 01 (HSCI)
MP 4111.161: 0.5        ;Axes monitoring off - axes group 02 (HSCI)
MP 4111.162: 0.5        ;Axes monitoring off - axes group 03 (HSCI)
MP 4111.163: 0.5        ;Axes monitoring off - axes group 04 (HSCI)
MP 4111.164: 0.5        ;Axes monitoring off - axes group 05 (HSCI)
MP 4111.165: 0.5        ;Axes monitoring off - axes group 06 (HSCI)
MP 4111.166: 0.5        ;Axes monitoring off - axes group 07 (HSCI)
MP 4111.167: 0.5        ;Axes monitoring off - axes group 08 (HSCI)
MP 4111.168: 0
MP 4111.169: 0
MP 4111.170: 0
MP 4111.171: 0
MP 4111.172: 0
MP 4111.173: 0
MP 4111.174: 0
MP 4111.175: 0
MP 4111.176: 0
MP 4111.177: 0
MP 4111.178: 0
MP 4111.179: 0
MP 4111.180: 0
MP 4111.181: 0
MP 4111.182: 0
MP 4111.183: 0
MP 4111.184: 0
MP 4111.185: 0
MP 4111.186: 0
MP 4111.187: 0
MP 4111.188: 0
MP 4111.189: 0
MP 4111.190: 0
MP 4111.191: 0
MP 4111.192: 0
MP 4111.193: 0
MP 4111.194: 0
MP 4111.195: 0
MP 4111.196: 0
MP 4111.197: 0
MP 4111.198: 0
MP 4111.199: 0
MP 4111.200: 5          ;PLC input (I32) off-delay
MP 4111.201: 7          ;Pneumatic pressure detection delay
MP 4111.202: 30         ;Coolant level detection delay
MP 4111.203: 1.5        ;Guard 1 sensor detection delay
MP 4111.204: 1.5        ;Guard 2 sensor detection delay
MP 4111.205: 20         ;Hydraulic valve pressure detection delay
MP 4111.206: 2          ;Delay of PLC marker MG_power_delayed_on
MP 4111.207: 15         ;Pneumatic filter detection delay
MP 4111.208: 0
MP 4111.209: 6          ;Nut lubrication, general
MP 4111.210: 15         ;Nut lubrication, pressure generation
MP 4111.211: 30         ;Pneumatic counter-weight pressure detection delay
MP 4111.212: 600        ;Lubrication pneumatic spray
MP 4111.213: 5          ;Tool clamping pneumatic detection delay
MP 4111.214: 5          ;Tool clamping hydraulic detection delay
MP 4111.215: 5          ;Buzzer humming duration
MP 4111.216: 10         ;Scale & spindle bearing air clean detection delay
MP 4111.217: 10         ;Pneumatic chip flush delay
MP 4111.218: 900        ;Spindle cooling unit off-delay
MP 4111.219: 20         ;Spindle cooling pressure detection delay
MP 4111.220: 180        ;Scale & spindle bearing air clean valve off-delay
MP 4111.221: 10         ;Spindle gear cooling pressure detection delay
MP 4111.222: 10         ;CTS filter detection delay
MP 4111.223: 3          ;Spindle blow clean duration after CTS
MP 4111.224: 10         ;CTS motor outlet pressure detection delay
MP 4111.225: 1          ;Spindle gear range output delay
MP 4111.226: 0.5        ;Spindle dwell after gear range in-position
MP 4111.227: 10         ;CTS tank inlet pressure detection delay
MP 4111.228: 3          ;Spindle oil-air lubrication reset duration
MP 4111.229: 300        ;S oil-air lubrication air supply delay after M5
MP 4111.230: 1          ;CYTEC spindle tool clamping/unclamping delay
MP 4111.231: 0.25       ;CYTEC spindle aiming seal on delay
MP 4111.232: 1          ;Tool clamping line pressure detection delay
MP 4111.233: 1          ;Solenoid off after tool clamped delay
MP 4111.234: 10         ;Solenoid off after tool clamped timeout
MP 4111.235: 10         ;Thermal compensation time-out
MP 4111.236: 30         ;Thermal compensation unit error delay
MP 4111.237: 10         ;S oil-air lubrication air supply on-delay
MP 4111.238: 180        ;Spindle rotary joint cooling off-delay
MP 4111.239: 30         ;Spindle rotary joint cooling circulation timeout
MP 4111.240: 2          ;Coolant spray/air blast switching delay
MP 4111.241: 5          ;S oil-air lubrication pneumatic detection delay
MP 4111.242: 5          ;S oil-air MIXER pneumatic detection delay
MP 4111.243: 5          ;Off delay of spindle orientation operation
MP 4111.244: 10         ;M08 motor outlet pressure detection delay
MP 4111.245: 2          ;Motor pause time for CTS high/low pressure switching
MP 4111.246: 5          ;Hydraulic system 2, gravity axis balance pressure
MP 4111.247: 5          ;Hydraulic system 2, tool&axes clamp/unclamp pressure
MP 4111.248: 0
MP 4111.249: 0
MP 4111.250: 0.4        ;Carrousel magazine braking DC voltage injection time
MP 4111.251: 20         ;Shifting pocket to magazine position timeout
MP 4111.252: 0.5        ;Shifting pocket to magazine position general timer
MP 4111.253: 20         ;Shifting pocket to TC position timeout
MP 4111.254: 0.5        ;Shifting pocket to TC position general timer
MP 4111.255: 15         ;Tool unloading from magazine pocket timeout
MP 4111.256: 1          ;Tool unloading from magazine pocket general timer
MP 4111.257: 15         ;Tool loading to magazine pocket timeout
MP 4111.258: 1          ;Tool loading to magazine pocket general timer
MP 4111.259: 0
MP 4111.260: 15         ;Tool unloading from magazine 2 pocket timeout
MP 4111.261: 1          ;Tool unloading from magazine 2 pocket general timer
MP 4111.262: 15         ;Tool loading to magazine 2 pocket timeout
MP 4111.263: 1          ;Tool loading to magazine 2 pocket general timer
MP 4111.264: 0
MP 4111.265: 1          ;Shifting pocket to magazine pos 2nd general timer
MP 4111.266: 0
MP 4111.267: 0
MP 4111.268: 1          ;3D head unclamping extension (3/4 of MP4111.100/.101)
MP 4111.269: 0
MP 4111.270: 105        ;V/H head turning time-out
MP 4111.271: 1          ;V/H head turning, general timer
MP 4111.272: 1          ;V/H head unclamping extension after head unclamped
MP 4111.273: 0
MP 4111.274: 0
MP 4111.275: 3          ;Tool measurement unit cover opening/closing timeout
MP 4111.276: 0
MP 4111.277: 0
MP 4111.278: 0
MP 4111.279: 0
MP 4111.280: 30         ;Automatic door 1 opening time-out
MP 4111.281: 30         ;Automatic door 1 closing time-out
MP 4111.282: 30         ;Automatic door 2 opening time-out
MP 4111.283: 30         ;Automatic door 2 closing time-out
MP 4111.284: 0
MP 4111.285: 0
MP 4111.286: 0
MP 4111.287: 30         ;Magazine panel tool selection I/O link timeout
MP 4111.288: 20         ;Magazine panel data display timeout
MP 4111.289: 20         ;Magazine panel data link timeout
MP 4111.290: 0
MP 4111.291: 0
MP 4111.292: 0
MP 4111.293: 0
MP 4111.294: 0
MP 4111.295: 0
             ;--------------------------------------------------------------
             ;Preset value for PLC counters Z0..Z47
             ;Input [s]
MP 4120.0  : 0
MP 4120.1  : 0
MP 4120.2  : 0
MP 4120.3  : 0
MP 4120.4  : 0
MP 4120.5  : 0
MP 4120.6  : 0
MP 4120.7  : 0
MP 4120.8  : 0
MP 4120.9  : 0
MP 4120.10 : 0
MP 4120.11 : 0
MP 4120.12 : 0
MP 4120.13 : 0
MP 4120.14 : 0
MP 4120.15 : 0
MP 4120.16 : 0
MP 4120.17 : 0
MP 4120.18 : 0
MP 4120.19 : 0
MP 4120.20 : 0
MP 4120.21 : 0
MP 4120.22 : 0
MP 4120.23 : 0
MP 4120.24 : 0
MP 4120.25 : 0
MP 4120.26 : 0
MP 4120.27 : 0
MP 4120.28 : 0
MP 4120.29 : 0
MP 4120.30 : 0
MP 4120.31 : 0
MP 4120.32 : 0
MP 4120.33 : 0
MP 4120.34 : 0
MP 4120.35 : 0
MP 4120.36 : 0
MP 4120.37 : 0
MP 4120.38 : 0
MP 4120.39 : 0
MP 4120.40 : 0
MP 4120.41 : 30         ;CTS tank low level
MP 4120.42 : 0
MP 4120.43 : 0
MP 4120.44 : 0
MP 4120.45 : 0
MP 4120.46 : 30         ;Chip flush intermittent control on duration
MP 4120.47 : 30         ;Chip flush intermittent control off duration
             ;--------------------------------------------------------------
             ;MP4130 Fast PLC inputs
             ;Input: 0 to 20000 [no. of the PLC input]
             ;      -1 function inactive
             ;       (Input I0 not supported from PLC Basic Program)
MP 4130.0  : -1         ;Suppress the monitoring functions
MP 4130.1  : -1         ;Reserved
MP 4130.2  : -1         ;Sets marker M4590
MP 4130.3  : -1         ;Sets marker M4591
MP 4130.4  : -1         ;Sets marker M4592
MP 4130.5  : -1         ;Sets marker M4593
             ;--------------------------------------------------------------
             ;MP4131 Activation criterion for fast PLC input
             ;Input: 0 = Activation at low level
             ;       1 = Activation at high level
MP 4131.0  : 0          ;Suppress the monitoring functions
MP 4131.1  : 0          ;Reserved
MP 4131.2  : 0          ;Sets marker M4590
MP 4131.3  : 0          ;Sets marker M4591
MP 4131.4  : 0          ;Sets marker M4592
MP 4131.5  : 0          ;Sets marker M4593
             ;--------------------------------------------------------------
             ;MP4132 Axis or axis group specific
             ;       PLC input for controls drive enabling (only CC 6xxx)
             ;Input: 0 to 20000 [no. of the PLC input]
             ;       -1 function inactive
MP 4132.0  : 301        ;I_release_axis_group_1
MP 4132.1  : -1         ;I_release_axis_group_2
MP 4132.2  : -1         ;I_release_axis_group_3
MP 4132.3  : -1         ;I_release_axis_group_4
MP 4132.4  : -1         ;I_release_axis_group_5
MP 4132.5  : -1         ;I_release_axis_group_6
MP 4132.6  : -1         ;I_release_axis_group_7
MP 4132.7  : -1         ;I_release_axis_group_8
             ;--------------------------------------------------------------
             ;MP4210 General values for the PLC (D768 to D956)
             ;Input: -99 999,9999 to +99 999,9999
MP 4210.0  : +30000     ;Feed PLC pos X
MP 4210.1  : +30000     ;Feed PLC pos Y
MP 4210.2  : +24000     ;Feed PLC pos Z
MP 4210.3  : +4000      ;Feed PLC pos 4
MP 4210.4  : +5000      ;Feed PLC pos 5
MP 4210.5  : +5000      ;Feed PLC pos 6
MP 4210.6  : +5000      ;Feed PLC pos 7
MP 4210.7  : +5000      ;Feed PLC pos 8
MP 4210.8  : +5000      ;Feed PLC pos 9
MP 4210.9  : +2000      ;Feed rate for guard open/safety mode 2
MP 4210.10 : +0         ;Tool change pos X
MP 4210.11 : +0         ;Tool change pos Y out (end position)
MP 4210.12 : +0         ;Tool change pos Y in  (TC position)
MP 4210.13 : +0         ;Tool change pos Z out (TC top/Z+ position)
MP 4210.14 : +637.207   ;Tool change pos Z in  (TC bottom/Z- position)
MP 4210.15 : +22.3      ;Tool change pos spindle
MP 4210.16 : +0         ;Tool change pos X manual
MP 4210.17 : +0         ;Tool change pos Y manual
MP 4210.18 : +637.207   ;Tool change pos Z manual
MP 4210.19 : +0         ;Tool change or V/H head change pos Z safety
MP 4210.20 : +5000      ;Feed rate for safety mode 4
MP 4210.21 : +6         ;Spindle n min [rpm]
MP 4210.22 : +10        ;Spindle n actl=noml [rpm; n < 100]
MP 4210.23 : +80        ;Spindle n actl=noml [%; n > 100]
MP 4210.24 : +10        ;Spindle torque min [%]
MP 4210.25 : +150       ;Spindle torque max 0=from 1..150 [%]
MP 4210.26 : +1.5       ;Spindle factor feedlimitation at max torque
MP 4210.27 : +2000      ;Spindle rpm for guard open/safety mode 2
MP 4210.28 : +100       ;Spindle rpm reference
MP 4210.29 : +5000      ;Spindle rpm for safety mode 4
MP 4210.30 : +60        ;Spindle temperature max. (Pt 100)
MP 4210.31 : +0         ;Spindle analog drive utilization factor (%/10V)
             
MP 4210.32 : +0         ;Tool change pos X out (end position)
MP 4210.33 : +0         ;Temp. comp. factor [um/deg] at T(diff) 00..10 deg
MP 4210.34 : +0         ;Temp. comp. factor [um/deg] at T(diff) 10..20 deg
MP 4210.35 : +0         ;Temp. comp. factor [um/deg] at T(diff) 20..30 deg
MP 4210.36 : +0         ;Temp. comp. factor [um/deg] at T(diff) 30..40 deg
MP 4210.37 : +0         ;Temp. comp. factor [um/deg] at T(diff) 40..50 deg
MP 4210.38 : +0         ;Alternating table end position Y
MP 4210.39 : +0         ;Pallet changer number of pallets
MP 4210.40 : +5         ;Minimum interval for axis lubrication [min]
MP 4210.41 : +120       ;Maximum interval for axis lubrication [min]
MP 4210.42 : +1         ;PLC module axes delay time [PLC cycles]
MP 4210.43 : +0         ;Alternating table limit X / Pallet position X
MP 4210.44 : +0         ;Alternating table position X1
MP 4210.45 : +0         ;Alternating table position X2
MP 4210.46 : +0         ;Alternating table position Y / Pallet position Y
MP 4210.47 : +0         ;Alternating table position Z / Pallet position Z
             ;--------------------------------------------------------------
             ;MP4220 General values for the PLC (W960 to W974)
             ;Input: 10 to 30 000
MP 4220.0  : 2000
MP 4220.1  : 2000
MP 4220.2  : 2000
MP 4220.3  : 2000
MP 4220.4  : 2000
             ;--------------------------------------------------------------
             ;MP4230 General values for the PLC (Module 9032)
             ;Input: -99 999.9999 to +99 999.9999
MP 4230.0  : -99999     ;Monitoring working area manual mode axis Y
MP 4230.1  : -99999     ;Monitoring working area manual mode axis Z
MP 4230.2  : -99999     ;Monitoring working area auto mode axis Y
MP 4230.3  : -99999     ;Monitoring working area auto mode axis Z
MP 4230.4  : +0         ;Monitoring working area manual mode axis B
MP 4230.5  : +0         ;Monitoring working area auto mode axis Y2/Z2
MP 4230.6  : +0         ;Monitoring working area auto mode axis B
MP 4230.7  : +300       ;S head bearing lubrication interval (hour)
MP 4230.8  : +0         ;max torque 3D head axis 1 [A], (+0 = motor table)
MP 4230.9  : +0         ;max torque 3D head axis 2 [A], (+0 = motor table)
MP 4230.10 : +0         ;3D head axis 1 offset spindle position
MP 4230.11 : +0         ;3D head axis 2 offset spindle position
MP 4230.12 : +0         ;Position after reference axis X
MP 4230.13 : +0         ;Position after reference axis Y
MP 4230.14 : +0         ;Position after reference axis Z
MP 4230.15 : +0         ;Position after reference axis 4
MP 4230.16 : +0         ;Position after reference axis 5
MP 4230.17 : +0         ;Position after reference axis 6
MP 4230.18 : +0         ;Position after reference axis 7
MP 4230.19 : +0         ;Position after reference axis 8
MP 4230.20 : +0         ;Position after reference axis 9
MP 4230.21 : +0         ;Position after reference axis 10
MP 4230.22 : +0         ;Position after reference axis 11
MP 4230.23 : +0         ;Position after reference axis 12
MP 4230.24 : +0         ;Tool unclamped sensor voltage upper limit (V)
MP 4230.25 : +0         ;Tool unclamped sensor voltage lower limit (V)
MP 4230.26 : +0         ;Clamped with tool sensor voltage upper limit (V)
MP 4230.27 : +0         ;Clamped with tool sensor voltage lower limit (V)
MP 4230.28 : +0         ;Clamped without tool sensor voltage upper limit (V)
MP 4230.29 : +0         ;Clamped without tool sensor voltage lower limit (V)
MP 4230.30 : +0         ;min torque 3D head axis 1 [A]
MP 4230.31 : +0         ;min torque 3D head axis 2 [A]
MP 4230.32 : +0
MP 4230.33 : +0
MP 4230.34 : +0         ;Z-axis compensation for 4th axis clamped
MP 4230.35 : +0
MP 4230.36 : +0         ;Tool change pos 7 (V-axis)
MP 4230.37 : +0
MP 4230.38 : +0
MP 4230.39 : +0
MP 4230.40 : +99999     ;Monitoring working area manual mode axis Y2
MP 4230.41 : +99999     ;Monitoring working area manual mode axis Z2
MP 4230.42 : +15        ;Intervals for 2nd axis lubrication [min]
MP 4230.43 : +15        ;Intervals for 3rd axis lubrication [min]
MP 4230.44 : +0
MP 4230.45 : +0
MP 4230.46 : +0
MP 4230.47 : +0         ;S rear bearing temperature limitation (degree)
MP 4230.48 : +0         ;Spindle rpm limitation for air through spindle (rpm)
MP 4230.49 : +0         ;Coolant level switch active(=0)/inactive(<>0)
MP 4230.50 : +0
MP 4230.51 : +0
MP 4230.52 : +0
MP 4230.53 : +0
MP 4230.54 : +0
MP 4230.55 : +0
MP 4230.56 : +0
MP 4230.57 : +0
MP 4230.58 : +0
MP 4230.59 : +0
MP 4230.60 : +0         ;Manual lubrication interval X (hour)
MP 4230.61 : +0         ;Manual lubrication interval Y (hour)
MP 4230.62 : +0         ;Manual lubrication interval Z (hour)
MP 4230.63 : +0
MP 4230.64 : +0
MP 4230.65 : +0
MP 4230.66 : +0         ;Pocket offset of magazine tool loading position
MP 4230.67 : +0
MP 4230.68 : +0
MP 4230.69 : +0
             ;--------------------------------------------------------------
             ;MP4231 General values for the PLC (Module 9032)
             ;Input: -99 999.9999 to +99 999.9999
MP 4231.0  : +0         ;X ball screw nut lubricating travel (m)
MP 4231.1  : +0         ;Tool change pos 4 (degree)
MP 4231.2  : +0         ;Tool change pos 5 (degree)
MP 4231.3  : +0         ;Tool change pos 4 manual (degree)
MP 4231.4  : +0         ;Tool change pos 5 manual (degree)
MP 4231.5  : +0.1       ;3D head S preposition window (degree)
MP 4231.6  : +0         ;3D head axis 2 motion Y+ limitation (mm)
MP 4231.7  : +0         ;V/H head X geometry offset (mm)
MP 4231.8  : +0         ;V/H head Y geometry offset (mm)
MP 4231.9  : +0         ;V/H head Z geometry offset (mm)
MP 4231.10 : +0         ;3D head axis 1 motion Y+ limitation (mm)
MP 4231.11 : +0         ;Tool or V/H head change pos Y safety
MP 4231.12 : +0         ;V/H or 3D head change pos X or Y (mm)
MP 4231.13 : +0         ;V/H or 3D head change pos Z (mm)
MP 4231.14 : +0.5       ;V/H head pos window S (degree)
MP 4231.15 : +0         ;S position before V head unclamping (degree)
MP 4231.16 : +0         ;S position before H head unclamping (degree)
MP 4231.17 : +180       ;S position for V/H head to horizontal pos (degree)
MP 4231.18 : +180       ;S position for V/H head to vertical pos (degree)
MP 4231.19 : +6         ;V/H head turning S speed (rpm)
MP 4231.20 : +50        ;S head transmission lubrication interval (hour)
MP 4231.21 : +0         ;Spindle without tool maximum speed (rpm)
MP 4231.22 : +30        ;S front bearing temperature limitation (degree)
MP 4231.23 : +0         ;Spindle warming-up 3rd stage duration (sec)
MP 4231.24 : +0         ;Spindle warming-up 1st stage duration (sec)
MP 4231.25 : +0         ;Spindle warming-up 2nd stage duration (sec)
MP 4231.26 : +0         ;Spindle warming-up 1st stage speed (rpm)
MP 4231.27 : +0         ;Spindle warming-up 2nd stage speed (rpm)
MP 4231.28 : +0         ;Spindle warming-up maximum allowable pause (sec)
MP 4231.29 : +0         ;Spindle rewarming-up standstill time (min)
MP 4231.30 : +0         ;Spindle warming-up 3rd stage speed (rpm)
MP 4231.31 : +0         ;Spindle rpm limitation with spindle head (rpm)
             
             ;--------------------------------------------------------------
             ;MP4310 General parameters for the PLC
             ;Input: $0000 [hexadecimal] or
             ;       %0000000000000000 [binary]
             ;
             ;MP4310.0
             ;bit 0, PLC address M4300
             ;       0 = Same valve for S tool air blow & CTS air blow
             ;       1 = Different valves for S tool air blow & CTS air blow
             ;
             ;bit 1, PLC address M4301
             ;       0 = Without spindle cooling liquid pressure detection
             ;       1 = With spindle cooling liquid pressure detection
             ;
             ;bit 2, PLC address M4302
             ;       Rotary axes to TC position (DVM-5X)
             ;       0 = One at a time
             ;       1 = Simultaneously
             ;
             ;bit 3, PLC address M4303
             ;       0 = Without 1st additional axis on/off button selection
             ;       1 = With 1st additional axis on/off button selection
             ;
             ;bit 4, PLC address M4304
             ;       0 = Manual head rear axis without 0 degree sensor
             ;       1 = Manual head rear axis with 0 degree sensor
             ;
             ;bit 5, PLC address M4305
             ;       0 = Without paper filter device
             ;       1 = With paper filter device
             ;
             ;bit 6, PLC address M4306
             ;       0 = Coolant through spindle without motor pressure
             ;           sensor
             ;       1 = Coolant through spindle with motor pressure sensor
             ;
             ;bit 7, PLC address M4307
             ;       0 = Hydraulic pressure low --> without M05 & NC STOP
             ;       1=  Hydraulic pressure low --> M05 & NC STOP
             ;
             ;bit 8, PLC address M4308
             ;       0 = Without spindle tool clamped without tool sensor
             ;       1 = With spindle tool clamped without tool sensor
             ;
             ;bit 9, PLC address M4309
             ;       0 = Magazine standby pocket tool detection at pocket out
             ;           pos
             ;       1 = Magazine standby pocket tool detection at pocket in
             ;           pos
             ;
             ;bit A, PLC address M4310
             ;       0 = S bearing temp. detection with 1 sensor (absolute)
             ;       1 = S bearing temp. detection with 2 sensors (relative)
             ;
             ;bit B, PLC address M4311
             ;       Magazine guard for non-servo magazine
             ;       0 = Without lock device
             ;       1 = With lock device
             ;
             ;bit C, PLC address M4312
             ;       Magazine servo during TC magazine loading
             ;       0 = Hirth motion (slow)
             ;       1 = Continuous motion (fast)
             ;
             ;bit D, PLC address M4313
             ;       0 = Magazine servo commissioning inactive
             ;       1 = Magazine servo commissioning active (V+/V- key)
             ;
             ;bit E, PLC address M4314
             ;       0 = Anti-backlash motion inactive (UMC)
             ;       1 = Anti-backlash motion active (UMC)
             ;
             ;bit F, PLC address M4315
             ;       0 = Keys for torque motor brake releasing inactive
             ;       1 = Keys for torque motor brake releasing active
             ;FEDCBA9876543210 <- bit #
MP 4310.0  : %0000000000000000;M4300-M4315, W976
             ;
             ;MP4310.1
             ;bit 0, PLC address M4316
             ;       0 = NC STOP --> feed stop --> spindle stop
             ;       1 = NC STOP --> feed stop --> without spindle stop
             ;
             ;bit 1, PLC address M4317
             ;       0 = Additional logbook information inactive
             ;       1 = Additional logbook information active
             ;
             ;bit 2, PLC address M4318
             ;       0 = Coolant spray only when M03/M04
             ;       1 = Coolant spray not controlled by M03/M04
             ;
             ;bit 3, PLC address M4319
             ;       0 = Without air through spindle option
             ;       1 = With air through spindle option
             ;
             ;bit 4, PLC address M4320
             ;       0 = Coolant spray & air blast in different nozzles
             ;       1 = Coolant spray & air blast in same nozzles
             ;
             ;bit 5, PLC address M4321
             ;       0 = ITRI manual or V/H head with tool clamped sensor
             ;       1 = ITRI manual or V/H head without tool clamped sensor
             ;
             ;bit 6, PLC address M4322
             ;       0 = With monitoring of toolchange without TOOL CALL
             ;       1 = Without monitoring of toolchange without TOOL CALL
             ;
             ;bit 7, PLC address M4323
             ;       0 = Pneumatic fine filter without detection
             ;       1 = Pneumatic fine filter with detection
             ;
             ;bit 8, PLC address M4324
             ;       0 = Without spindle tool clamping pneumatic detection
             ;       1 = With spindle tool clamping pneumatic detection
             ;
             ;bit 9, PLC address M4325
             ;       0 = Standby tool number display inactive
             ;       1 = Standby tool number display active
             ;
             ;bit A, PLC address M4326
             ;       0 = Carrousel magazine motor with external braking unit
             ;       1 = Carrousel magazine motor with internal braking unit
             ;
             ;bit B, PLC address M4327
             ;       0 = Without chip flush intermittent control
             ;       1 = With chip flush intermittent control
             ;
             ;bit C, PLC address M4328
             ;       0 = Output = 1 --> VI axis unclamping
             ;       1 = Output = 1 --> VI axis clamping
             ;
             ;bit D, PLC address M4329
             ;       0 = Pneumatic level low --> without M05 & NC STOP
             ;       1 = Pneumatic level low --> M05 & NC STOP
             ;
             ;bit E, PLC address M4330
             ;       Operation guard open + permission key
             ;       0 = NC START key invalid
             ;       1 = NC START key valid
             ;
             ;bit F, PLC address M4331
             ;       0 = Without V/H head geometry compensation
             ;       1 = With V/H head geometry compensation
             ;FEDCBA9876543210 <- bit #
MP 4310.1  : %0010000000000001;M4316-M4331, W978
             ;
             ;MP4310.2
             ;bit 0, PLC address M4332
             ;       0 = Start-up procedure inactive
             ;       1 = Start-up procedure active
             ;
             ;bit 1, PLC address M4333
             ;       0 = PLC protections of safety guard active
             ;       1 = PLC protections of safety guard inactive
             ;
             ;bit 2, PLC address M4334
             ;       0 = Cutting feed only when M03/M04/M19/M20
             ;       1 = Cutting feed not controlled by M03/M04/M19/M20
             ;
             ;bit 3, PLC address M4335
             ;       0 = Alternating table/pallet change active
             ;       1 = Alternating table/pallet change inactive
             ;
             ;bit 4, PLC address M4336
             ;       0 = One NC START key press for traversing all reference
             ;           points
             ;       1 = Each NC START key press only for traversing one-axis
             ;           reference point
             ;
             ;bit 5, PLC address M4337
             ;       0 = Automatic tool change IV positioning inactive
             ;       1 = Automatic tool change IV positioning active
             ;
             ;bit 6, PLC address M4338
             ;       0 = With monitoring function of tool in spindle
             ;       1 = Without monitoring function of tool in spindle
             ;
             ;bit 7, PLC address M4339
             ;       0 = With monitoring function of tool in magazine standby
             ;           pocket
             ;       1 = Without monitoring function of tool in magazine
             ;           standby pocket
             ;
             ;bit 8, PLC address M4340
             ;       0 = T0 not required for T-automatic <-> T-manual
             ;       1 = T0 required for T-automatic <-> T-manual
             ;
             ;bit 9, PLC address M4341
             ;       0 = Manual tool change positioning active
             ;       1 = Manual tool change positioning inactive
             ;
             ;bit A, PLC address M4342
             ;       0 = No safety guard unlocking when M02/M30/END PGM
             ;       1 = Safety guard unlocking when M02/M30/END PGM
             ;
             ;bit B, PLC address M4343
             ;       0 = Magazine standby pocket number display active
             ;       1 = Magazine standby pocket number display inactive
             ;
             ;bit C, PLC address M4344
             ;       (Only for temperature compensation without table)
             ;       0 = Temperature compensation inactive
             ;       1 = Temperature compensation active
             ;
             ;bit D, PLC address M4345
             ;       0 = T0 not required for magazine tool loading
             ;       1 = T0 required for magazine tool loading
             ;
             ;bit E, PLC address M4346
             ;       0 = PLC preset table TNC:\DATUM\PRESET.TAB inactive
             ;       1 = PLC preset table TNC:\DATUM\PRESET.TAB active
             ;
             ;bit F, PLC address M4347
             ;       0 = With torque monitoring function of spindle tool
             ;       1 = Without torque monitoring function of spindle tool
             ;FEDCBA9876543210 <- bit #
MP 4310.2  : %0000000011000000;M4332-M4347, W980
             ;
             ;MP4310.3
             ;bit 0, PLC address M4348
             ;       0 = IV axis on reference dog, sensor signal = 0
             ;       1 = IV axis on reference dog, sensor signal = 1
             ;
             ;bit 1, PLC address M4349
             ;       0 = With gear range display
             ;       1 = Without gear range display
             ;
             ;bit 2, PLC address M4350
             ;       0 = Without scale & spindle bearing air clean control
             ;       1 = With scale & spindle bearing air clean control
             ;
             ;bit 3, PLC address M4351
             ;       0 = Without spindle tool blow clean after CTS
             ;       1 = With spindle tool blow clean after CTS
             ;
             ;bit 4, PLC address M4352
             ;       0 = Without ball screw cooling system option
             ;       1 = With ball screw cooling system option
             ;
             ;bit 5, PLC address M4353
             ;       0 = Without TC saving time
             ;       1 = With TC saving time
             ;           (Only for FBE/EBM sliding arm, MB shifting arm)
             ;
             ;bit 6, PLC address M4354
             ;       0 = Coolant through spindle without filter sensor
             ;       1 = Coolant through spindle with filter sensor
             ;
             ;bit 7, PLC address M4355
             ;       0 = Without pneumatic chip flush
             ;       1 = With pneumatic chip flush
             ;
             ;bit 8, PLC address M4356
             ;       0 = 3D head manual operation invalid
             ;       1 = 3D head manual operation valid
             ;
             ;bit 9, PLC address M4357
             ;       0 = Each guard with its own unlock output
             ;       1 = Two guards with one unlock output
             ;
             ;bit A, PLC address M4358
             ;       0 = Automatic tool change X positioning inactive
             ;       1 = Automatic tool change X positioning active
             ;
             ;bit B, PLC address M4359
             ;       0 = Automatic tool change Y positioning inactive
             ;       1 = Automatic tool change Y positioning active
             ;
             ;bit C, PLC address M4360
             ;       0 = Without lubrication pneumatic spray
             ;       1 = With lubrication pneumatic spray
             ;
             ;bit D, PLC address M4361
             ;       0 = TC out of machining travel (double column/LBM)
             ;       1 = TC within machining travel (double column/LBM)
             ;
             ;bit E, PLC address M4362
             ;       0 = Nut lubrication unit without pressure sensor
             ;       1 = Nut lubrication unit with pressure sensor
             ;
             ;bit F, PLC address M4363
             ;       0 = Without X ball screw nut lubrication
             ;       1 = With X ball screw nut lubrication
             ;FEDCBA9876543210 <- bit #
MP 4310.3  : %0000000001001100;M4348-M4363, W982
             ;
             ;MP4310.4
             ;bit 0, PLC address M4364
             ;       0 = Output = 1 --> IV axis unclamping
             ;       1 = Output = 1 --> IV axis clamping
             ;
             ;bit 1, PLC address M4365
             ;       0 = Output = 1 --> V axis unclamping
             ;       1 = Output = 1 --> V axis clamping
             ;
             ;bit 2, PLC address M4366
             ;       0 = Without automatic power off after M02/M30/END PGM
             ;           option
             ;       1 = With automatic power off after M02/M30/END PGM
             ;           option
             ;
             ;bit 3, PLC address M4367
             ;       0 = Coolant through spindle option
             ;       1 = Coolant through tool option
             ;
             ;bit 4, PLC address M4368
             ;       0 = Alternating table/pallet change X prepos inactive
             ;       1 = Alternating table/pallet change X prepos active
             ;
             ;bit 5, PLC address M4369
             ;       0 = Without secondary safety guard(s) option
             ;       1 = With secondary safety guard(s) option
             ;
             ;bit 6, PLC address M4370
             ;       0 = Without spindle cooling system option
             ;       1 = With spindle cooling system option
             ;
             ;bit 7, PLC address M4371
             ;       0 = Without spindle gear cooling system
             ;       1 = With spindle gear cooling system
             ;
             ;bit 8, PLC address M4372
             ;       0 = Pneumatic shifting of carrousel tool changer
             ;       1 = Motor shifting of carrousel tool changer
             ;
             ;bit 9, PLC address M4373
             ;       0 = Arm in-position, arm position sensor = 0
             ;       1 = Arm in-position, arm position sensor = 1
             ;
             ;bit A, PLC address M4374
             ;       0 = Magazine in-position, magazine counter sensor = 0
             ;       1 = Magazine in-position, magazine counter sensor = 1
             ;
             ;bit B, PLC address M4375
             ;       0 = Chip conveyor forward motion controlled by coolant
             ;           chip flush
             ;       1 = Chip conveyor not controlled by coolant chip flush
             ;
             ;bit C, PLC address M4376
             ;       0 = Magazine with 1 counting sensor only
             ;       1 = Magazine with 2 counting sensors
             ;
             ;bit D, PLC address M4377
             ;       0 = Coolant water gun controlled by solenoid
             ;       1 = Coolant water gun controlled by motor
             ;
             ;bit E, PLC address M4378
             ;       0 = Magazine without safety guard and operation panel
             ;       1 = Magazine with safety guard and operation panel
             ;
             ;bit F, PLC address M4379
             ;       0 = Magazine without standby pocket tool detection
             ;           sensor
             ;       1 = Magazine with standby pocket tool detection sensor
             ;FEDCBA9876543210 <- bit #
MP 4310.4  : %0000010001000100;M4364-M4379, W984
             ;
             ;MP4310.5
             ;bit 0, PLC address M4380
             ;       reserved
             ;
             ;bit 1, PLC address M4381
             ;       reserved
             ;
             ;bit 2, PLC address M4382
             ;       reserved
             ;
             ;bit 3, PLC address M4383
             ;       reserved
             ;
             ;bit 4, PLC address M4384
             ;       reserved
             ;
             ;bit 5, PLC address M4385
             ;       reserved
             ;
             ;bit 6, PLC address M4386
             ;       reserved
             ;
             ;bit 7, PLC address M4387
             ;       reserved
             ;
             ;bit 8, PLC address M4388
             ;       reserved
             ;
             ;bit 9, PLC address M4389
             ;       reserved
             ;
             ;bit A, PLC address M4390
             ;       reserved
             ;
             ;bit B, PLC address M4391
             ;       reserved
             ;
             ;bit C, PLC address M4392
             ;       reserved
             ;
             ;bit D, PLC address M4393
             ;       reserved
             ;
             ;bit E, PLC address M4394
             ;       0 = No safety guard unlocking when emergency stop
             ;       1 = Safety guard unlocking when emergency stop
             ;
             ;bit F, PLC address M4395
             ;       0 = Without oil mist function
             ;       1 = With oil mist function
             ;FEDCBA9876543210 <- bit #
MP 4310.5  : %0000000000000000;M4380-M4395, W986
             ;Intervals for 1st axis lubrication [min]
MP 4310.6  : 15         ;M4396-M4411, W988
             ;
             ;MP4310.7
             ;bit 0, PLC address M4412
             ;       0 = Without spindle tool clamping hydraulic detection
             ;       1 = With spindle tool clamping hydraulic detection
             ;
             ;bit 1, PLC address M4413
             ;       0 = Spindle tool clamped without tool -> M05
             ;       1 = Spindle tool clamped without tool -> M03/M04
             ;           possible
             ;
             ;bit 2, PLC address M4414
             ;       0 = Magazine in-position, magazine 2nd counter sensor=0
             ;       1 = Magazine in-position, magazine 2nd counter sensor=1
             ;
             ;bit 3, PLC address M4415
             ;       0 = Without pneumatic to hydraulic unit
             ;       1 = With pneumatic to hydraulic unit
             ;
             ;bit 4, PLC address M4416
             ;       0 = Without dust/mist collector device
             ;       1 = With dust/mist collector device
             ;
             ;bit 5, PLC address M4417
             ;       0 = Without electrical cabinet air conditioner
             ;       1 = With electrical cabinet air conditioner
             ;
             ;bit 6, PLC address M4418
             ;       0 = Program running -> chip conveyor on function
             ;           inactive
             ;       1 = Program running -> chip conveyor on function active
             ;
             ;bit 7, PLC address M4419
             ;       0 = DCM option not purchased
             ;       1 = DCM option purchased
             ;
             ;bit 8, PLC address M4420
             ;       0 = EMG -> IV axis stop & brake applied simultaneously
             ;       1 = EMG -> IV axis stop -> brake applied
             ;
             ;bit 9, PLC address M4421
             ;       0 = EMG -> V axis stop & brake applied simultaneously
             ;       1 = EMG -> V axis stop -> brake applied
             ;
             ;bit A, PLC address M4422
             ;       0 = EMG -> VI axis stop & brake applied simultaneously
             ;       1 = EMG -> VI axis stop -> brake applied
             ;
             ;bit B, PLC address M4423
             ;       0 = MACRO call of DCM partition CMO inactive
             ;       1 = MACRO call of DCM partition CMO active
             ;
             ;bit C, PLC address M4424
             ;       0 = Working area monitoring before axes referenced
             ;           inactive
             ;       1 = Working area monitoring before axes referenced
             ;           active
             ;
             ;bit D, PLC address M4425
             ;       0 = Machine without service door(s)
             ;       1 = Machine with service door(s)
             ;
             ;bit E, PLC address M4426
             ;       When MP4310.7 bit F = 1,
             ;       0 = Only for program run single block & full sequence
             ;           modes
             ;       1 = For all modes
             ;
             ;bit F, PLC address M4427
             ;       0 = Automatic door opening after M61/M62 inactive
             ;       1 = Automatic door opening after M61/M62 active
             ;FEDCBA9876543210 <- bit #
MP 4310.7  : %0000000000000000;M4412-M4427, W990
             ;
             ;MP4310.8
             ;bit 0, PLC address M4428
             ;       reserved
             ;
             ;bit 1, PLC address M4429
             ;       reserved
             ;
             ;bit 2, PLC address M4430
             ;       reserved
             ;
             ;bit 3, PLC address M4431
             ;       reserved
             ;
             ;bit 4, PLC address M4432
             ;       Magazine 2 servo during TC magazine 2 loading
             ;       0 = Hirth motion (slow)
             ;       1 = Continuous motion (fast)
             ;
             ;bit 5, PLC address M4433
             ;       0 = Carrousel magazine 2 motor with external braking
             ;           unit
             ;       1 = Carrousel magazine 2 motor with internal braking
             ;           unit
             ;
             ;bit 6, PLC address M4434
             ;       0 = Magazine 2 standby pocket tool detection at pocket
             ;           out pos
             ;       1 = Magazine 2 standby pocket tool detection at pocket
             ;           in pos
             ;
             ;bit 7, PLC address M4435
             ;       0 = With monitoring function of tool in magazine 2
             ;           standby pocket
             ;       1 = Without monitoring function of tool in magazine 2
             ;           standby pocket
             ;
             ;bit 8, PLC address M4436
             ;       0 = Pneumatic shifting of carrousel tool changer 2
             ;       1 = Motor shifting of carrousel tool changer 2
             ;
             ;bit 9, PLC address M4437
             ;       0 = Arm 2 in-position, arm 2 position sensor = 0
             ;       1 = Arm 2 in-position, arm 2 position sensor = 1
             ;
             ;bit A, PLC address M4438
             ;       0 = Magazine 2 in-position, magazine 2 counter sensor=0
             ;       1 = Magazine 2 in-position, magazine 2 counter sensor=1
             ;
             ;bit B, PLC address M4439
             ;       Magazine 2 guard for non-servo magazine
             ;       0 = Without lock device
             ;       1 = With lock device
             ;
             ;bit C, PLC address M4440
             ;       0 = Magazine 2 with counting sensor only
             ;       1 = Magazine 2 with exact position & counting sensors
             ;
             ;bit D, PLC address M4441
             ;       0 = T0 not required for magazine 2 tool loading
             ;       1 = T0 required for magazine 2 tool loading
             ;
             ;bit E, PLC address M4442
             ;       0 = Magazine 2 without safety guard and operation panel
             ;       1 = Magazine 2 with safety guard and operation panel
             ;
             ;bit F, PLC address M4443
             ;       0 = Magazine 2 without standby pocket tool detection
             ;           sensor
             ;       1 = Magazine 2 with standby pocket tool detection
             ;           sensor
             ;FEDCBA9876543210 <- bit #
MP 4310.8  : %0000000000000000;M4428-M4443, W992
             ;
             ;MP4310.9
             ;bit 0, PLC address M4444
             ;       0 = Z-axis compensation for 4th axis clamped inactive
             ;       1 = Z-axis compensation for 4th axis clamped active
             ;
             ;bit 1, PLC address M4445
             ;       0 = MP4310.9 bit 0 = 1 always available
             ;       1 = Only when 4th axis unlocked (M401), MP4310.9 bit 0
             ;           =1 available
             ;
             ;bit 2, PLC address M4446
             ;       reserved
             ;
             ;bit 3, PLC address M4447
             ;       0 = V axis overtravel, sensor signal = 0
             ;       1 = V axis overtravel, sensor signal = 1
             ;
             ;bit 4, PLC address M4448
             ;       0 = IV axis clamping/unclamping line high/low pressure
             ;           sensor inactive (4 sensors)
             ;       1 = IV axis clamping/unclamping line high/low pressure
             ;           sensor active (4 sensors)
             ;
             ;bit 5, PLC address M4449
             ;       0 = V axis clamping/unclamping line high/low pressure
             ;           sensor inactive (4 sensors)
             ;       1 = V axis clamping/unclamping line high/low pressure
             ;           sensor active (4 sensors)
             ;
             ;bit 6, PLC address M4450
             ;       reserved
             ;
             ;bit 7, PLC address M4451
             ;       reserved
             ;
             ;bit 8, PLC address M4452
             ;       0 = V+ & V- end position by proximity sensors inactive
             ;       1 = V+ & V- end position by proximity sensors active
             ;
             ;bit 9, PLC address M4453
             ;       0 = NC STOP after TIME1 of TOOL TABLE
             ;       1 = PGM running after TIME1 of TOOL TABLE
             ;
             ;bit A, PLC address M4454
             ;       For coolant water gun controlled by solenoid
             ;       (MP4310.4 bit D = 0)
             ;       0 = Coolant water gun using coolant motor
             ;       1 = Coolant water gun using chip flush motor
             ;
             ;bit B, PLC address M4455
             ;       0 = CTS high/low pressure switching function inactive
             ;       1 = CTS high/low pressure switching function active
             ;
             ;bit C, PLC address M4456
             ;       0 = Without pneumatic counter-weight balancing sensor
             ;       1 = With pneumatic counter-weight balancing sensor
             ;
             ;bit D, PLC address M4457
             ;       0 = Coolant motor without motor pressure sensor
             ;       1 = Coolant motor with motor pressure sensor
             ;
             ;bit E, PLC address M4458
             ;       0 = Automatic tool change V positioning inactive
             ;       1 = Automatic tool change V positioning active
             ;
             ;bit F, PLC address M4459
             ;       0 = Magazine touch panel (tool selection) inactive
             ;       1 = Magazine touch panel (tool selection) active
             ;FEDCBA9876543210 <- bit #
MP 4310.9  : %0000000000000000;M4444-M4459, W994
             ;--------------------------------------------------------------
             ;MP4400 Profibus configuration
             ;Input: %xx
             ;Bit 0 Definition of the sequence of options
             ;      0 = Sequence as in *.IOC file
             ;      1 = Sequence as MP4000.x
             ;Bit 1 reserved
             ;10 <- bit #
MP 4400    : %00
             ;==============================================================
             ;MP 5000 .. 5999
             ;Serial interfaces
             ;==============================================================
             ;MP5000 Disable the data interface
             ;Input: 0 = No interface disabled
             ;       1 = RS-232-C/V.24 interface disabled
             ;       2 = RS-422/V.11 interface disabled
MP 5000    : 0
             ;--------------------------------------------------------------
             ;MP5020 User-defined transmission protocol
             ;Bit 0   7 or 8 data bits          0 = 7 data bits
             ;                                  1 = 8 data bits
             ;Bit 1   Block check character     0 = Any BCC
             ;                                  1 = BCC not control character
             ;Bit 2   Transmission stop by RTS  0 = Inactive
             ;                                  1 = Active
             ;Bit 3   Transmission stop by DC3  0 = Inactive
             ;                                  1 = Active
             ;Bit 4   Character parity          0 = Even
             ;                                  1 = Odd
             ;Bit 5   Character parity          0 = Not desired
             ;                                  1 = Desired
             ;Bit 6,7 Stop bits
             ;                                    Bit 6  Bit 7
             ;                   1 1/2 stop bits    0      0
             ;                   2 stop bits        0      0
             ;                   1 stop bit         0      1
             ;                                      1      1
             ;Bit 8,9 reserved
             ;Bit 10  Check of missing connection 0 = Active
             ;                                    1 = Inactive
             ;1----------
             ;09876543210 <- bit #
MP 5020.0  : %00010101000;EXT1 mode
MP 5020.1  : %00010101000;EXT2 mode
MP 5020.2  : %00010101000;PLC mode (EXT3)
MP 5020.3  : %00010101000;PLC mode (EXT4)
             ;--------------------------------------------------------------
             ;MP5030 Activation of data transfer protocol
             ;        0 = "Standard data transfer protocol"
             ;        1 = "Blockwise transfer ACK/NAK"
             ;        2 = Without protocol (only for MP5030.2)
MP 5030.0  : 0          ;EXT1 mode
MP 5030.1  : 0          ;EXT2 mode
MP 5030.2  : 1          ;PLC mode (EXT3)
MP 5030.3  : 1          ;PLC mode (EXT4)
             ;--------------------------------------------------------------
             ;MP5040 Data transfer rate in the EXT3 operating mode
             ;(data transfer via PLC)
             ;Input: 0 = 110 bits/s    6 = 4800 bits/s
             ;       1 = 150 bits/s    7 = 9600 bits/s
             ;       2 = 300 bits/s    8 = 19200 bits/s
             ;       3 = 600 bits/s    9 = 38400 bits/s
             ;       4 = 1200 bits/s  10 = 57600 bits/s
             ;       5 = 2400 bits/s  11 = 115200 bits/s
MP 5040.0  : 7          ;EXT3
MP 5040.1  : 7          ;EXT4
             ;==============================================================
             ;MP 6000 .. 6499
             ;3-D touch probe
             ;==============================================================
             ;MP6010 Select the touch probe
             ;Input:   0 = Touch probe with cable transmission
             ;             (TS 120/TS 220/TS 230)
             ;         1 = Infrared probe(TS 511/TS 630/TS 632)
             ;         2 = Infrared probe (TS 440/TS 640)
             ;         3 = Infrared probe without battery (TS 444)
MP 6010    : 2
             ;--------------------------------------------------------------
             ;MP6120 Probing feed rate (triggering touch probe)
             ;Input: 1 to 10 000 [mm/min]
MP 6120    : 500
             ;--------------------------------------------------------------
             ;MP6130 Maximum measuring range
             ;Input: 0.001 to 99 999.9999 [mm]
MP 6130    : 100
             ;--------------------------------------------------------------
             ;MP6140 Setup clearance over measuring point
             ;Input: 0.001 to 99 999.9999 [mm]
MP 6140    : 2
             ;--------------------------------------------------------------
             ;MP6150 Rapid traverse in probing cycle (triggering touch probe)
             ;Input: 10 to 10 000 [mm/min]
MP 6150    : 2000
             ;--------------------------------------------------------------
             ;MP6151 Rapid traverse at pre-positioning in probing cycle
             ;Input: 0 = Inactive (use rapid traverse from MP6150)
             ;       1 = Active (use rapid traverse from MP101x)
MP 6151    : 0
             ;--------------------------------------------------------------
             ;MP6160 M function for 180° rotation of the spindle
             ;Input: -1 to 999
             ;       -1 = Orientation by NC
             ;        0 = Function inactive
MP 6160    : -1
             ;--------------------------------------------------------------
             ;MP6161 M function for orienting the infrared touch probe before
             ;       each measurement
             ;Input: -1 to 999
             ;       -1 = Orientation by NC
             ;        0 = Function inactive
MP 6161    : +20
             ;--------------------------------------------------------------
             ;MP6162 Orientation angle
             ;Input: 0 to 359.9999 [°]
MP 6162    : 0
             ;--------------------------------------------------------------
             ;MP6163 Difference between MP6162 and the current spindle angle
             ;       before executing an oriented spindle stop
             ;Input: 0 to 3.0000 [°]
MP 6163    : 0
             ;--------------------------------------------------------------
             ;MP6165 Function for infrared touch probe during manual and
             ;       automatic measurement: The touch probe is oriented such
             ;       that the stylus always deflects in the same direction
             ;       (increased probing accuracy)
             ;Input: 0 or 1
             ;       1= Function active
MP 6165    : 1
             ;--------------------------------------------------------------
             ;MP6166 Funktion for touch probe in manual measurements:
             ;       The touch probe is moved to the touch point by
             ;       considering the basic rotation
             ;Input: 0 = Inactive
             ;       1 = Active
MP 6166    : 0
             ;--------------------------------------------------------------
             ;MP6170 Number of programmed measurements (mean value
             ;       generation)
             ;Input: 1 to 3
MP 6170    : 1
             ;--------------------------------------------------------------
             ;MP6171 Max. permissible tolerance (MP6170 > 1)
             ;Input: 0.0020 to 0.9999 [mm]
MP 6171    : 0.002
             ;--------------------------------------------------------------
             ;MP6180 Coordinate center of calibration ring for cycle
             ;       TCH PROBE 2 "TS Calibration" with respect to the machine
             ;       datum (traverse range 1)
             ;Input: -99 999.9999 to +99 999.9999 [mm]
MP 6180.0  : +0         ;X REF coordinate
MP 6180.1  : +0         ;Y
MP 6180.2  : +0         ;Z
             ;--------------------------------------------------------------
             ;MP6181 Coordinate center of calibration ring for cycle
             ;       TCH PROBE 2 "TS Calibration" with respect to the machine
             ;       datum (traverse range 2)
             ;Input: -99 999.9999 to +99 999.9999 [mm]
MP 6181.0  : +0         ;X REF coordinate
MP 6181.1  : +0         ;Y
MP 6181.2  : +0         ;Z
             ;--------------------------------------------------------------
             ;MP6182 Coordinate center of calibration ring for cycle
             ;       TCH PROBE 2 "TS Calibration" with respect to the machine
             ;       datum (traverse range 3)
             ;Input: -99 999.9999 to +99 999.9999 [mm]
MP 6182.0  : +0         ;X REF coordinate
MP 6182.1  : +0         ;Y
MP 6182.2  : +0         ;Z
             ;--------------------------------------------------------------
             ;MP6185 Distance of probing point below ring top surface for
             ;       cycle TCH PROBE 2 "TS Calibration" with respect to
             ;       the machine datum
             ;Input: 0.001 to 99 999.9999 [mm]
MP 6185.0  : 5          ;(Traverse range 1)
MP 6185.1  : 5          ;(Traverse range 2)
MP 6185.2  : 5          ;(Traverse range 3)
             ;==============================================================
             ;MP 6500 .. 6599
             ;Tool touch probe for tool measurement
             ;==============================================================
             ;MP6500 Tool measurement with TT 130
             ;Input: %xxxxxxxxxxxxxxx
             ;Bit 0   0 = Tool measurement disabled
             ;        1 = Tool measurement active
             ;Bit 1   0 = Tool radius measurement active, tool length
             ;            measurement with rotating spindle
             ;        1 = Tool radius / individual tool measurement disabled
             ;Bit 2   0 = Tool length measurement with stationary spindle
             ;            (bit 1=1)
             ;        1 = Tool length measurement with rotating spindle
             ;            (TT:R-OFFS must be entered)
             ;Bit 3   0 = Tool measurement with spindle orientation
             ;        1 = Tool measurement without spindle orientation
             ;            No individual tooth measurement, radius incorrect
             ;Bit 4   0 = Measuring speed max. 1000 rpm
             ;        1 = Measuring speed not limited
             ;Bit 5   0 = No NC stop after tool breakage (check tool)
             ;        1 = NC stop after tool breakage (check tool)
             ;Bit 6   0 = No NC stop after tool breakage (measure tool)
             ;        1 = NC stop after tool breakage (measure tool)
             ;Bit 7   0 = Reserved
             ;        1 = Reserved
             ;Bit 8   0 = Calibration completed (probe contact free)
             ;        1 = Calibration in only one axis
             ;Bit 9   Automatic measurement of the direction of the probe
             ;        contact basic rotation (Bit8 = 1)
             ;        0 = Basic rotation is not measured
             ;        1 = Basic rotation of the probe element is
             ;            automatically measured (rectangular stylus)
             ;Bit 10  Probing routine
             ;        0 = Pre-positioning to starting point in all
             ;            threeprincipal axes
             ;        1 = Pre-positioning to starting point in the tool
             ;            axis and in the axis of the probing direction
             ;            (MP6505)
             ;Bit 11  Tool checking and changing in the tool table
             ;        0 = After "tool checking" the tool table is changed
             ;        1 = After "tool checking" the tool table is not changed
             ;Bit 12  0 = Tool measurement does not include the PLC
             ;            datum shift
             ;        1 = Tool measurement includes PLC datum shift
             ;Bit 13  0 = Tool is measured in the same tilted position
             ;            in which the tool touch probe was calibrated
             ;        1 = Tool measurement in different tilted system
             ;Bit 14  0 = Tool measurement with rotating spindle and
             ;            number of teeth = 0
             ;        1 = Tool measurement with stationary spindle and
             ;            number of teeth = 0
             ;11111----------
             ;432109876543210 <- bit #
MP 6500    : %000000001100001
             ;--------------------------------------------------------------
             ;MP6502 Configuration of TT touch probe
             ;Input:
             ; 0 = Cable connection
             ; 1 = Control via IR infrared
             ; 2 = Control via radio
MP 6502    : 1
             
             
             
             ;--------------------------------------------------------------
             ;MP6505 Probing direction for tool measurement
             ;Input:
             ;0 = Positive probing direction of the angle reference axis (0°)
             ;1 = Positive probing direction in the 90°-axis
             ;2 = Negative probing direction of the angle reference axis (0°)
             ;3 = Negative probing direction in the 90°-axis
MP 6505.0  : 0          ;Traverse range 1
MP 6505.1  : 0          ;Traverse range 2
MP 6505.2  : 0          ;Traverse range 3
             ;--------------------------------------------------------------
             ;MP6507 Calculation of the probing feed rate
             ;Input:
             ; 0 = Calculation of the probing feed rate with constant
             ;     tolerance
             ; 1 = Calculation of the probing feed rate with variable
             ;     tolerance
             ; 2 = Constant probing feed rate
MP 6507    : 0
             ;--------------------------------------------------------------
             ;MP6508 Transmission-receiver unit SE 642 (only HSCI)
             ;Input:
             ; 0 = Inactive (TS at X112, TT at X113)
             ; 1 = SE 642 active (TS & TT at X112)
MP 6508    : 0
             ;--------------------------------------------------------------
             ;MP6509 reserved
MP 6509    : 0
             ;--------------------------------------------------------------
             ;MP6510 Max. permissible measuring error for tool measurement
             ;       with rotating tool
             ;Input: 0.002 to 0.999 [mm]
MP 6510.0  : 0.005      ;First measuring error
MP 6510.1  : 0.01       ;Second measuring error
             ;--------------------------------------------------------------
             ;MP6520 Probing feed rate for tool measurement with
             ;       non-rotating tool
             ;Input: 1 to 10 000 [mm/min]
MP 6520    : 100
             ;--------------------------------------------------------------
             ;MP6530 Distance from the tool end to the top of the probe
             ;       contact during tool radius measurement
             ;Input: 0.001 to 99.9999 [mm]
MP 6530.0  : 5          ;Traverse range 1
MP 6530.1  : 5          ;Traverse range 2
MP 6530.2  : 5          ;Traverse range 3
             ;--------------------------------------------------------------
             ;MP6531 Diameter or edge length of the probe contact
             ;Input: 0.001 to 99 999.9999 [mm]
MP 6531.0  : 40         ;Traverse range 1
MP 6531.1  : 40         ;Traverse range 2
MP 6531.2  : 40         ;Traverse range 3
             ;--------------------------------------------------------------
             ;MP6540 Safety zone around the probe contact for pre-positioning
             ;Input: 0.001 to 99 999.9999 [mm]
MP 6540.0  : 10         ;Tool-axis direction
MP 6540.1  : 10         ;Machining plane
             ;--------------------------------------------------------------
             ;MP6550 Rapid traverse in probing cycle
             ;Input: 10 to 1 000 000 [mm/min]
MP 6550    : 2000
             ;--------------------------------------------------------------
             ;MP6560 M function for spindle orientation during
             ;individual tooth measurement
             ;Input: -1 to 999
             ;       -1 = Orientation by NC
             ;        0 = Function inactive
MP 6560    : -1
             ;--------------------------------------------------------------
             ;MP6562 M function before or after tool measurement cycle
             ;Input: -1 to 999
             ;       -1 = Function inactive
MP 6562.0  : -1         ;before tool measurement cycle
MP 6562.1  : -1         ;after tool measurement cycle
             ;--------------------------------------------------------------
             ;MP6570 Maximum permissible surface cutting speed of
             ;the tool tooth
             ;Input: 1.0000 to 120.0000 [m/min]
MP 6570    : 40
             ;--------------------------------------------------------------
             ;MP6572 Maximum permissible speed for tool measurement
             ;Input: 0 to 1000 rpm
             ;       0 = 1000 rpm
MP 6572    : 0
             ;--------------------------------------------------------------
             ;MP6580 Coordinates of the probe contact center with respect to
             ;the machine datum (traverse range 1)
             ;Input: -99 999.9999 to +99 999.9999 [mm]
MP 6580.0  : +161       ;X position of probe contact (reference)
MP 6580.1  : +524       ;Y
MP 6580.2  : +144
             ;--------------------------------------------------------------
             ;MP6581 Coordinates of the probe contact center with respect to
             ;the machine datum (traverse range 2)
             ;Input: -99 999.9999 to +99 999.9999 [mm]
MP 6581.0  : +0         ;X position of probe contact (reference)
MP 6581.1  : +0         ;Y
MP 6581.2  : +0         ;Z
             ;--------------------------------------------------------------
             ;MP6582 Coordinates of the probe contact center with respect to
             ;the machine datum (traverse range 3)
             ;Input: -99 999.9999 to +99 999.9999 [mm]
MP 6582.0  : +0         ;X position of probe contact (reference)
MP 6582.1  : +0         ;Y
MP 6582.2  : +0         ;Z
             ;--------------------------------------------------------------
             ;MP6585 Monitor the rotary and additional linear axes
             ;       during the start of tool measurement
             ;Input: %000000 (WVUCBA) bit-encoded
             ;WVUCBA <- axis name
MP 6585    : %000000
             ;--------------------------------------------------------------
             ;MP6586 Ref. coordinate for monitoring from MP6585
             ;Input: -99 999.9999 to +99 999.9999 [mm]
MP 6586.0  : +0         ;A axis
MP 6586.1  : +0         ;B axis
MP 6586.2  : +0         ;C axis
MP 6586.3  : +0         ;U axis
MP 6586.4  : +0         ;V axis
MP 6586.5  : +0         ;W axis
             ;--------------------------------------------------------------
             ;MP6600 KinematicsOpt: maximum permitted modification
             ;Input: 0.010 to 1.000 [mm]
MP 6600    : 0.05
             ;--------------------------------------------------------------
             ;MP6601 KinematicsOpt: maximum permitted deviation of the
             ;       calibration ball
             ;Input: 0.010 to 0.100 [mm]
MP 6601    : 0.02
             ;--------------------------------------------------------------
             ;MP6602 KinematicsOpt: M-function macro for postioning of the
             ;       rotary axes
             ;Input: -1 to 999
             ;       -1 = Function inactive
MP 6602    : -1
             ;==============================================================
             ;MP 7100 .. 7199
             ;Tapping
             ;==============================================================
             ;MP7110 Limits for feed rate override during tapping
             ;Input: 0 to 150 [%]
MP 7110.0  : 95         ;Minimum
MP 7110.1  : 105        ;Maximum
             ;--------------------------------------------------------------
             ;MP7120 Waiting times during the tapping cycle
             ;Input: 0 to 65.535 [s]
MP 7120.0  : 0          ;Dwell time for rotational direction reversal
MP 7120.1  : 0          ;Advanced switching time
             ;(only with coded spindle speed output)
MP 7120.2  : 0          ;Spindle slow-down time after reaching the hole depth
             ;--------------------------------------------------------------
             ;MP7130 Run-in behavior of the spindle during rigid tapping
             ;Input: 0.001 to 10 [°/min]
MP 7130    : 1
             ;--------------------------------------------------------------
             ;MP7150 Positioning window for the tool axis
             ;Please note: The value must be smaller than or equal to the
             ;window in MP1030.
             ;Input: 0.0001 to 2 [mm]
MP 7150    : 0.01
             ;--------------------------------------------------------------
             ;MP7160 Behaviour of the spindle with cycle 17, 207 and 18
             ;Input: %xxxxx
             ;Bit 0  0 = Spindle orientation before executing Cycle 17
             ;       1 = No spindle orientation before executing Cycle 17
             ;Bit 1  0 = No limitation of the spindle speed
             ;       1 = For small thread depths, the spindle speed is
             ;           limited so that the spindle runs with constant speed
             ;           approx. 1/3 of the time.
             ;Bit 2  0 = Spindle operated without position feedback control
             ;       1 = Spindle operated with position feedback control
             ;Bit 3  0 = Acceleration feedforward control active
             ;       1 = Acceleration feedforward control inactive
             ;Bit 4  0 = Tool axis of the spindle is tracked
             ;       1 = Tool axis and spindle interpolated
             ;43210 <- bit #
MP 7160    : %00000
             ;==============================================================
             ;MP 7200 .. 7399
             ;Programming and display
             ;==============================================================
             ;MP7210 Programming station
             ;Input: 0 = Controlling and programming
             ;       1 = Programming station with PLC active
             ;       2 = Programming station with PLC inactive
             ;       3 = Programming station with "PLC and
             ;           emergency stop active" (X41/34, X42/4)
MP 7210    : 0
             ;--------------------------------------------------------------
             ;MP7212 Power interruption
             ;Input: 0 = The "Power interrupted" message must be
             ;           acknowledged with the CE key
             ;       1 = The "Power interrupted" message does not appear
MP 7212    : 0
             ;--------------------------------------------------------------
             ;MP7220 Increment for N numbers in ISO programming
             ;Input: 0 to 250
             ; (0=no automatic generation)
MP 7220    : 10
             ;--------------------------------------------------------------
             ;MP7224 Disable file types
             ;Input: %xxxxxxxx
             ; 0 = Not disabled, not protected
             ; 1 = Disabled, protected
             ;Bit 0   Heidenhain programs   *.h files
             ;Bit 1   ISO programs          *.i files
             ;Bit 2   Tool tables           *.t files
             ;Bit 3   Datum tables          *.d files
             ;Bit 4   Pallet tables         *.p files
             ;Bit 5   Text files            *.a files
             ;Bit 6   Help files          *.hlp files
             ;Bit 7   Point tables        *.pnt files
             ;76543210 <- bit #
MP 7224.0  : %00000000  ;Disable file type
MP 7224.1  : %00000000  ;Set file type to read-only
MP 7224.2  : %00000000  ;Disable softkey EDIT OFF/ON
             ;--------------------------------------------------------------
             ;MP7226 Length of tables
             ;Input: 0 to 255 [lines]
MP 7226.0  : 0          ;Reserved
MP 7226.1  : 20         ;Datum table (*.d)
             ;--------------------------------------------------------------
             ;MP7229.0 Program length for program inspection
             ;Input: 100 to 9999
MP 7229.0  : 200
             ;--------------------------------------------------------------
             ;MP7229.1 Program length to which FK blocks are allowed
             ;Input: 100 to 9999
MP 7229.1  : 9999
             ;--------------------------------------------------------------
             ;MP7230 Conversational language
             ;Input:
             ;         0 = English
             ;         1 = German
             ;         2 = Czech
             ;         3 = French
             ;         4 = Italian
             ;         5 = Spanish
             ;         6 = Portuguese
             ;         7 = Swedish
             ;         8 = Danish
             ;         9 = Finnish
             ;         10= Dutch
             ;         11= Polish
             ;         12= Hungarian
             ;         13= Reserved (only MP7230.0)
             ;         14= Russian
             ;         15= Chinese (simplified)
             ;         16= Chinese (traditional)
             ;         17= Slowenian (Option #41 - Additional Language)
             ;         18= Norwegian (Option #41 - Additional Language)
             ;         19= Slovak    (Option #41 - Additional Language)
             ;         20= Latvian   (Option #41 - Additional Language)
             ;         21= Korean    (Option #41 - Additional Language)
             ;         22= Estonian  (Option #41 - Additional Language)
             ;         23= Turkish   (Option #41 - Additional Language)
             ;         24= Romanian  (Option #41 - Additional Language)
             ;         25= Lithuanian(Option #41 - Additional Language)
MP 7230.0  : 0          ;NC dialogs
             
             
MP 7230.1  : 0          ;PLC dialogs (user cycle, user parameter)
MP 7230.2  : 0          ;PLC error message
MP 7230.3  : 0          ;Help files
             ;--------------------------------------------------------------
             ;MP7237 PLC operating times counter (code number 857282)
             ;Input: %7654321 bit-encoded
             ;1111---------
             ;3210987654321 <- PLC counter #
             ;111----------
             ;2109876543210 <- bit #
MP 7237.0  : %0000000000011;Display PLC counter
MP 7237.1  : %1111111111111;Reset PLC counter
             ;Reset NC counter (code number 857282)
             ;Input: %xxxxxxx bit-encoded
             ;Bit 0   Reserved
             ;Bit 1   "Machine on" can be reset
             ;Bit 2   "Program run" can be reset
             ;111----------
             ;2109876543210 <- bit #
MP 7237.2  : %0000000000000
             ;------------------------------------------------------------
             ;MP7238 Dialogs for PLC counter
             ;Input: Line number in the dialog file
MP 7238.0  : 16         ;Counter 1
MP 7238.1  : 17         ;Counter 2
MP 7238.2  : 18         ;Counter 3
MP 7238.3  : 19         ;Counter 4
MP 7238.4  : 20         ;Counter 5
MP 7238.5  : 21         ;Counter 6
MP 7238.6  : 22         ;Counter 7
MP 7238.7  : 23         ;Counter 8
MP 7238.8  : 24         ;Counter 9
MP 7238.9  : 25         ;Counter 10
MP 7238.10 : 26         ;Counter 11
MP 7238.11 : 27         ;Counter 12
MP 7238.12 : 28         ;Counter 13
             ;--------------------------------------------------------------
             ;MP 7245 Enable auxiliary cycles (e.g. Cycle 18)
             ;Input: %xx
             ;Bit 0   0 = Auxiliary cycle 18 disabled
             ;        1 = Auxiliary cycle 18 enabled
             ;Bit 1   0 = Cycle 32 enabled
             ;        1 = Cycle 32 disabled
MP 7245    : %01
             ;--------------------------------------------------------------
             ;MP7246 Machine parameter with multiple function
             ;Input: %xxxxx
             ;Bit 0   0 = Paraxial positioning blocks enabled
             ;        1 = Paraxial positioning blocks disabled
             ;Bit 1   0 = Deletion with the DEL key does not have to be
             ;            confirmed.
             ;        1 = Deletion with the DEL key must be confirmed.
             ;Bit 2   0 = Do not create tool-usage file
             ;        1 = Create tool-usage file
             ;Bit 3   0 = Do not create file for AFC settings
             ;        1 = Create file for AFC settings
             ;Bit 4   0 = Do not create file for machining time of each
             ;            NC block
             ;        1 = Create file for machining time of each NC block
             ;43210 <- bit #
MP 7246    : %01000
             ;--------------------------------------------------------------
             ;MP7251 Number of global Q parameters given to the
             ;calling program by the manufacturer cycle
             ;Input: 0 to 40
MP 7251    : 0
             ;--------------------------------------------------------------
             ;MP7260 Size of the tool table
             ;Input: 0 to 30 000
MP 7260    : 999        ;Number of tools (T number)
             ;--------------------------------------------------------------
             ;MP7261 Size of the pocket tables
             ;Input: 0 to 9999
MP 7261.0  : 24         ;Number of pockets in magazine 1 (P number)
MP 7261.1  : 0          ;Number of pockets in magazine 2 (P number)
MP 7261.2  : 0          ;Number of pockets in magazine 3 (P number)
MP 7261.3  : 0          ;Number of pockets in magazine 4 (P number)
MP 7261.4  : 0          ;Number of pockets in magazine 5 (P number)
MP 7261.5  : 0          ;Number of pockets in magazine 6 (P number)
MP 7261.6  : 0          ;Number of pockets in magazine 7 (P number)
MP 7261.7  : 0          ;Number of pockets in magazine 8 (P number)
MP 7261.8  : 0          ;Number of pockets in magazine 9 (P number)
MP 7261.9  : 0          ;Number of pockets in magazine 10 (P number)
MP 7261.10 : 0          ;Number of pockets in magazine 11 (P number)
MP 7261.11 : 0          ;Number of pockets in magazine 12 (P number)
MP 7261.12 : 0          ;Number of pockets in magazine 13 (P number)
MP 7261.13 : 0          ;Number of pockets in magazine 14 (P number)
MP 7261.14 : 0          ;Number of pockets in magazine 15 (P number)
MP 7261.15 : 0          ;Number of pockets in magazine 16 (P number)
             ;--------------------------------------------------------------
             ;MP7262 Maximum index number for indexed tools
             ;Input: 0 to 9
MP 7262    : 3
             ;--------------------------------------------------------------
             ;MP7263 Pocket table
             ;Input: %xxxxxxx
             ;       Bit 0 - Soft key "Pocket table"
             ;               0 = Show soft key
             ;               1 = Hide soft key
             ;       Bit 1 - Output of the columns for file functions
             ;               0 = Output only the displayed columns
             ;               1 = Output all columns
             ;       Bit 2 - Soft key "Edit ON/OFF"
             ;               0 = Show soft key
             ;               1 = Hide soft key
             ;       Bit 3 - Soft keys "Reset pocket table"
             ;              and "Reset column T"
             ;               0 = Show soft keys
             ;               1 = Hide soft keys
             ;       Bit 4 - Deletion possible for a tool that is in
             ;               the pocket table.
             ;               Deletion must be confirmed.
             ;               0 = Deletion not possible (behavior as before)
             ;               1 = Deletion possible (with confirmation)
             ;       Bit 5 - Deletion of a tool possible even without
             ;               confirmation (if bit 4 = 1).
             ;               0 = Deletion not possible without confirmation
             ;               1 = Deletion possible without confirmation
             ;       Bit 6 - Deletion of index entries of a tool behaves
             ;               like deletion of a tool.
             ;               Settings in bits 4 and 5 are
             ;               taken into account.
             ;               0 = Deletion always possible (behavior as
             ;                   before)
             ;               1 = Deletion possible depending on settings
             ;                   in bits 4 and 5
             ;6543210 <- bit #
MP 7263    : %0000000
             ;--------------------------------------------------------------
             ;MP7266 Tool table: Tool data (0=inactive)
MP 7266.0  : 1          ;Tool name (Name)
MP 7266.1  : 2          ;Tool length (L)
MP 7266.2  : 3          ;Tool radius (R)
MP 7266.3  : 4          ;Tool radius 2 (R2)
MP 7266.4  : 5          ;Tool length oversize (DL)
MP 7266.5  : 6          ;Tool radius oversize (DR)
MP 7266.6  : 7          ;Tool radius oversize 2 (DR2)
MP 7266.7  : 8          ;Tool locked? (TL)
MP 7266.8  : 9          ;Replacement tool (RT)
MP 7266.9  : 10         ;Time 1
MP 7266.10 : 11         ;Time 2
MP 7266.11 : 12         ;Current Time
MP 7266.12 : 13         ;Comment on the tool (DOC)
MP 7266.13 : 14         ;Number of tool teeth (CUT)
MP 7266.14 : 15         ;Tolerance for tool length (LTOL)
MP 7266.15 : 16         ;Tolerance for tool radius (RTOL)
MP 7266.16 : 17         ;Cutting direction of the tool (DIRECT)
MP 7266.17 : 18         ;PLC status (PLC)
MP 7266.18 : 19         ;Tool length offset (TT:L-OFFS)
MP 7266.19 : 20         ;Tool radius offset (TT:R-OFFS)
MP 7266.20 : 21         ;Breakage tolerance for tool length (LBREAK)
MP 7266.21 : 22         ;Breakage tolerance for tool radius (TBREAK)
MP 7266.22 : 23         ;Tooth length (LCUTS)
MP 7266.23 : 24         ;Plunge angle (ANGLE)
MP 7266.24 : 25         ;Tool type (TYP)
MP 7266.25 : 26         ;Tool cutting material (TMAT)
MP 7266.26 : 27         ;Cutting data table (CDT)
MP 7266.27 : 28         ;PLC value (PLC-VAL)
MP 7266.28 : 29         ;Probe center offset in reference axis (CAL-OF1)
MP 7266.29 : 30         ;Probe center offset in minor axis (CAL-OF2)
MP 7266.30 : 31         ;Spindle angle during calibration (CAL-ANG)
MP 7266.31 : 32         ;Tool type for pocket table (PTYP)
MP 7266.32 : 33         ;Maximum shaft speed (NMAX)
MP 7266.33 : 34         ;Retract tool (LIFTOFF)
MP 7266.34 : 35         ;PLC value (P1)
MP 7266.35 : 36         ;PLC value (P2)
MP 7266.36 : 37         ;PLC value (P3)
MP 7266.37 : 38         ;Additional kinematic description (KINEMATIK)
MP 7266.38 : 39         ;Point angle for DRILL and CSINK (T-ANGLE)
MP 7266.39 : 40         ;Thread pitch for TAP (PITCH)
MP 7266.40 : 41         ;AFC: Strategy-name for Adaptive Feed Control
MP 7266.41 : 42         ;Tolerance value for tool radius R2 (R2TOL)
MP 7266.42 : 0          ;Link to compensation value table (DR2TABLE, Opt #92)
MP 7266.43 : 44         ;Time stamp of last use tool (LAST-USE)
MP 7266.44 : 45         ;Active Chatter Compensation (ACC, Option #145)
MP 7266.45 : 46         ;Exceeding the tool life (OVRTIME)
MP 7266.46 : 0          ;Collision length (CL)
MP 7266.47 : 0          ;Collision radius (CR)
MP 7266.48 : 47         ;PLC value (P4)
             ;--------------------------------------------------------------
             ;MP7267 Elements of the pocket table
             ;Input: 0 = No display
             ;       1 to 99 = Position in the pocket table
MP 7267.0  : 1          ;Tool number (T)
MP 7267.1  : 3          ;Special tool (ST) single-arm = 0, double-arm = 3
MP 7267.2  : 4          ;Fixed pocket (F)  single-arm = 0, double-arm = 4
MP 7267.3  : 5          ;Locked pocket (L)
MP 7267.4  : 0          ;PLC status (PLC)
MP 7267.5  : 2          ;Tool name (TNAME)
MP 7267.6  : 6          ;Comment from the tool table (DOC)
MP 7267.7  : 0          ;Tool type for pocket table (PTYP)
MP 7267.8  : 0          ;Value 1 (P1)
MP 7267.9  : 0          ;Value 2 (P2)
MP 7267.10 : 0          ;Value 3 (P3)
MP 7267.11 : 0          ;Value 4 (P4)
MP 7267.12 : 0          ;Value 5 (P5)
MP 7267.13 : 0          ;Reserve pocket (RSV)
MP 7267.14 : 0          ;Pocket above locked (LOCKED_ABOVE)
MP 7267.15 : 0          ;Pocket below locked (LOCKED_BELOW)
MP 7267.16 : 0          ;Pocket left locked (LOCKED_LEFT)
MP 7267.17 : 0          ;Pocket right locked (LOCKED_RIGHT)
MP 7267.18 : 0          ;S1 Value (P6)
MP 7267.19 : 0          ;S2 Value (P7)
             ;--------------------------------------------------------------
             ;MP7268 Sorting/hiding columns of the preset table
             ;Input: 1 to 99 = Order of columns
             ;       0 = Hide column
MP 7268.0  : 1          ;DOC
MP 7268.1  : 2          ;ROT
MP 7268.2  : 3          ;X
MP 7268.3  : 4          ;Y
MP 7268.4  : 5          ;Z
MP 7268.5  : 6          ;A
MP 7268.6  : 7          ;B
MP 7268.7  : 8          ;C
MP 7268.8  : 0          ;U
MP 7268.9  : 0          ;V
MP 7268.10 : 0          ;W
             ;--------------------------------------------------------------
             ;MP7270 Feed-rate display in manual operating modes
             ;(manual operation and electronic handwheel)
             ;0 = Display of the feed rate only by pressing an
             ;    axis direction key (axis-specific feed rate from MP1020.x)
             ;1 = Display of the feed rate also before pressing an
             ;    axis direction key (smallest value from MP1020.x for
             ;    all axes)
MP 7270    : 0
             ;--------------------------------------------------------------
             ;MP7280 Definition of the decimal character
             ;Input: 0 = Comma
             ;       1 = Point
MP 7280    : 1
             ;--------------------------------------------------------------
             ;MP7281 Depiction of the NC program
             ;Input: 0 = Always show all NC blocks completely
             ;       1 = Only show current block completely
             ;       2 = Only show complete NC block during creation or
             ;           change
MP 7281.0  : 0          ; Programming & Editing
MP 7281.1  : 0          ; Program Run
             ;--------------------------------------------------------------
             ;MP7285 Tool length offset for the
             ;tool-axis position display
             ;Input: 0 = Tool length is not offset
             ;       1 = Tool length is offset
MP 7285    : 1
             ;--------------------------------------------------------------
             ;MP7289 Position display step for the spindle
             ;Input: 0 = 0.1 [degrees]
             ;       1 = 0.05 [degrees]
             ;       2 = 0.01 [degrees]
             ;       3 = 0.005 [degrees]
             ;       4 = 0.001 [degrees]
             ;       5 = 0.0005 [degrees]
             ;       6 = 0.0001 [degrees]
MP 7289    : 4
             ;--------------------------------------------------------------
             ;MP7290 Position display step
             ;Input: 0 = 0.1 [mm] or [degrees]
             ;       1 = 0.05 [mm] or [degrees]
             ;       2 = 0.01 [mm] or [degrees]
             ;       3 = 0.005 [mm] or [degrees]
             ;       4 = 0.001 [mm] or [degrees]
             ;       5 = 0.0005 [mm] or [degrees]
             ;       6 = 0.0001 [mm] or [degrees]
MP 7290.0  : 4
MP 7290.1  : 4
MP 7290.2  : 4
MP 7290.3  : 4
MP 7290.4  : 4
MP 7290.5  : 4
MP 7290.6  : 4
MP 7290.7  : 4
MP 7290.8  : 4
MP 7290.9  : 4
MP 7290.10 : 4
MP 7290.11 : 4
MP 7290.12 : 4
MP 7290.13 : 4
             ;--------------------------------------------------------------
             ;MP7291 Displayed axes
             ;Input: SWvUcBaZyX
             ; X = NC axes (capital letters)
             ; x = PLC axes (lowercase letters)
             ; - = Non-displayed axes
             ;987654321 <- display sequence
MP 7291.0  : S----CAZYX ;Traverse range 1
MP 7291.1  : S----CAZYX ;Traverse range 2
MP 7291.2  : S----CAZYX ;Traverse range 3
             ;--------------------------------------------------------------
             ;MP7294 Disable axis-specific datum setting in the
             ;       preset table
             ;Input: %xxxxxxxxxxxxxx
             ;       Bits 0 to 13 represent axes 1 to 14
             ;       0 = Enabled
             ;       1 = Disabled
             ;.............987654ZYX <- axis
             ;221111111111----------
             ;1098765432109876543210 <- bit #
MP 7294    : %0000000000000000000000
             ;--------------------------------------------------------------
             ;MP7295 Disable "Datum setting"
             ;Input: %xxxxxxxxxxxxxx
             ;       Bits 0 to 13 represent axes 1 to 14
             ;       0 = Enabled
             ;       1 = Disabled
             ;.............987654ZYX <- axis
             ;221111111111----------
             ;1098765432109876543210 <- bit #
MP 7295    : %0000000000000000000000
             ;--------------------------------------------------------------
             ;MP7296 Datum setting with axis keys
             ;Input:
             ;Bit 0 = 0 Datum can be set with axis keys and by soft key
             ;Bit 0 = 1 Datum can be set only by soft key
             ;10 <- bit #
MP 7296    : %00
             ;--------------------------------------------------------------
             ;MP7300 Erasing the status display and Q parameters
             ;Input: 0 = Status display, Q parameters and tool data
             ;           are deleted when the program is selected
             ;       1 = Status display, Q parameters and tool data
             ;           are deleted with M02, M30, END PGM or when
             ;           a program is selected
             ;       2 = Erase the status display and tool data
             ;           when a program is selected
             ;       3 = Erase the status display and tool data
             ;           when a program is selected or with M02, M30, or
             ;           END PGM
             ;       4 = Status display and Q parameters are deleted
             ;           when a program is selected
             ;       5 = Status display and Q parameters are deleted when
             ;           a program is selected or with M02, M30, or END PGM
             ;       6 = Erase the status display when a program is selected
             ;       7 = Erase the status display when a program is selected
             ;           or with M02, M30, or END PGM
MP 7300    : 5
             ;--------------------------------------------------------------
             ;MP7310 Graphic display mode
             ;Input: %xxxxxxx
             ;Bit 0   Changing the projection in three planes
             ;        0 = German-preferred projection
             ;        1 = US-preferred projection
             ;Bit 1   Rotation of the coordinate system in the machining
             ;        plane by +90°
             ;        0 = No rotation
             ;        1 = Coordinate system rotated by +90°
             ;Bit 2   BLK form after datum shift
             ;        0 = BLK form is not shifted
             ;        1 = BLK form is shifted
             ;Bit 3   Display of cursor position in projection in 3 planes
             ;        0 = No display
             ;        1 = Display of the cursor position
             ;Bit 4   reserved
             ;Bit 5   3D graphics in program test
             ;        0 = 2,5D and 3D (only with MC 420 or MC 422B)
             ;        1 = 2,5D
             ;Bit 6   Stock removal with an inclined tool
             ;        0 = Inactive
             ;        1 = Active
             ;Bit 7   Evaluation of the column LCUTS from TOOL.T, so that
             ;        special tools can be shown.
             ;        0 = Free evaluation
             ;        1 = Exact evaluation
             ;76543210 <- bit #
MP 7310    : %00000000
             ;--------------------------------------------------------------
             ;MP7312 Limits the tooth lengs if no value is entered in the
             ;LCUTS column in the tool table
             ;Input: 0 = No limitation, infinetly long tooth length
             ;       > 0 Tooth length = 2 * tool radius * MP7312
MP 7312    : 5.00000
             ;--------------------------------------------------------------
             ;MP7315 Tool radius for graphic simulation of laser and
             ;water jet without Tool Call
             ;Input: 0.0000 to 99 999.9999 [mm]
MP 7315    : 0
             ;--------------------------------------------------------------
             ;MP 7316 Penetration depth in the blank form for graphic
             ;simulation of laser and water jet without Tool Call
             ;Input: 0.0000 to 99 999.9999 [mm]
MP 7316    : 0
             ;--------------------------------------------------------------
             ;MP7317 Activating the graphic simulation
             ;Input: Number of M command
MP 7317.0  : 0          ;M command to start simulation
MP 7317.1  : 0          ;M command to end simulation
             ;--------------------------------------------------------------
             ;MP7330 Specifying the user parameters for the MODE function
             ;Input: MP number
             ;Warning: Enter a 2-digit index (i.e. 110.01, not 110.1)
MP 7330.0  : 0
MP 7330.1  : 0
MP 7330.2  : 0
MP 7330.3  : 0
MP 7330.4  : 0
MP 7330.5  : 0
MP 7330.6  : 0
MP 7330.7  : 0
MP 7330.8  : 0
MP 7330.9  : 4210.26    ;Spindle factor feedlimitation at max torque
MP 7330.10 : 0
MP 7330.11 : 4000.44    ;4th axis clamping mode on/off
MP 7330.12 : 4000.45    ;5th axis clamping mode on/off
MP 7330.13 : 0
MP 7330.14 : 0
MP 7330.15 : 0
             ;--------------------------------------------------------------
             ;MP7340 Specifying the dialogs for the user parameters
             ;file: DIALOG.A)
MP 7340.0  : 0
MP 7340.1  : 1
MP 7340.2  : 2
MP 7340.3  : 3
MP 7340.4  : 4
MP 7340.5  : 5
MP 7340.6  : 6
MP 7340.7  : 7
MP 7340.8  : 8
MP 7340.9  : 9
MP 7340.10 : 10
MP 7340.11 : 11
MP 7340.12 : 12
MP 7340.13 : 13
MP 7340.14 : 14
MP 7340.15 : 15
             ;--------------------------------------------------------------
             ;MP735x Parameters for calculating the color definitions
             ;   Red    -   Green   -   Blue
             ;0..F-0..F - 0..F-0..F - 0..F-0..F
             ;rough-fine - rough-fine - rough-fine
             ;e.g. $03F3F3F -> white color
             ;--------------------------------------------------------------
MP 7350    : $0808080   ;Window frame
             ;--------------------------------------------------------------
             ;Error messages
MP 7351.0  : $0FF2020   ;Error
MP 7351.1  : $000FF00   ;Warning
MP 7351.2  : $00000FF   ;Information
             ;--------------------------------------------------------------
             ;"Machine" operating mode display
MP 7352.0  : $0ECECEC   ;Background
MP 7352.1  : $0000000   ;Text for operating mode
MP 7352.2  : $00000FF   ;Dialog
             ;--------------------------------------------------------------
             ;"Programming" operating mode display
MP 7353.0  : $0C0C0C0   ;Background
MP 7353.1  : $0000000   ;Text for operating mode
MP 7353.2  : $00000FF   ;Dialog
             ;--------------------------------------------------------------
             ;"Machine" program text display
MP 7354.0  : $0FFFFFF   ;Background
MP 7354.1  : $0000000   ;General program text
MP 7354.2  : $00000FF   ;Active block
MP 7354.3  : $0A0A0A0   ;Comments and not used MP's in the MP file
MP 7354.4  : $0C8C8C8   ;Background of inactive window (structure)
             ;--------------------------------------------------------------
             ;"Programming" program text display
MP 7355.0  : $0FFFFFF   ;Background
MP 7355.1  : $0000000   ;General program text
MP 7355.2  : $00000FF   ;Active block
MP 7355.3  : $0A0A0A0   ;Comments and not used MP's in the MP file
MP 7355.4  : $0C8C8C8   ;Background of inactive window (structure)
             ;--------------------------------------------------------------
             ;"Machine" soft-key display
MP 7357.0  : $0C0C0C0   ;Background
MP 7357.1  : $0000000   ;Font colour
MP 7357.2  : $0000000   ;Inactive soft-key row
MP 7357.3  : $00000FF   ;Active soft-key row
             ;--------------------------------------------------------------
             ;"Programming" soft-key display
MP 7358.0  : $0C0C0C0   ;Background
MP 7358.1  : $0000000   ;Symbols
MP 7358.2  : $0000000   ;Inactive soft-key row
MP 7358.3  : $00000FF   ;Active soft-key row
             ;--------------------------------------------------------------
             ;Graphics: 3-D view and plan view
MP 7360.0  : $0AAAAAA   ;Background
MP 7360.1  : $08888F0   ;Top surface
MP 7360.2  : $00011AA   ;3-D: Front face
MP 7360.3  : $0FFFFFF   ;Text display in the graphics window
MP 7360.4  : $00000FF   ;3-D: Lateral face
MP 7360.5  : $0550000   ;Lowest point of the blank form
MP 7360.6  : $0FFFFFF   ;Highest point of blank form (below surface)
MP 7360.7  : $0CC1919   ;3-D: Sectional plane
MP 7360.8  : $06666CC   ;3-D: Tool 1
MP 7360.9  : $066CC66   ;3-D: Tool 2
MP 7360.10 : $06666CC   ;3-D: Tool 3
MP 7360.11 : $066CC66   ;3-D: Tool 4
MP 7360.12 : $0CC6666   ;3-D: Tool 5
MP 7360.13 : $0CC66CC   ;3-D: Tool 6
MP 7360.14 : $066CCCC   ;3-D: Tool 7
MP 7360.15 : $066CC19   ;3-D: Tool 8
             ;--------------------------------------------------------------
             ;Graphics: Projection in three planes
MP 7361.0  : $0AAAAAA   ;Background
MP 7361.1  : $00000E8   ;Top view
MP 7361.2  : $00000E8   ;Front and side view
MP 7361.3  : $0FF0000   ;Axis cross and text in the graphic display
MP 7361.4  : $0FF00FF   ;Cursor
             ;--------------------------------------------------------------
             ;Additional status display in the graphics window
MP 7362.0  : $0ECECEC   ;Background of graphic window
MP 7362.1  : $0FFFFFF   ;Background of status display
MP 7362.2  : $00000FF   ;Status symbols
MP 7362.3  : $00000FF   ;Status values
MP 7362.4  : $0C0C0C0   ;Background of not selected tab
MP 7362.5  : $0ECECEC   ;Tab AFC - Background bar, diagram
MP 7362.6  : $000FF80   ;Tab AFC - Color actual override factor
MP 7362.7  : $000AAFF   ;Tab AFC - Color actual spindle load
             ;--------------------------------------------------------------
             ;Programming graphics
MP 7363.0  : $0ECECEC   ;Background
MP 7363.1  : $00000FF   ;Resolved contour
MP 7363.2  : $0FF00FF   ;Subprograms and frames for zooming
MP 7363.3  : $000EC00   ;Alternate solutions
MP 7363.4  : $0FF0000   ;Unresolved contour
MP 7363.5  : $00080FF   ;Contour in rapid traverse
MP 7363.6  : $0000000   ;Block number
             ;--------------------------------------------------------------
             ;Color of the help illustrations for cycles
MP 7364.0  : $0DBD3DB   ;Red
MP 7364.1  : $0FF0000   ;Yellow
MP 7364.2  : $0808080   ;Green
MP 7364.3  : $0000000   ;Turquoise
MP 7364.4  : $0000163   ;Blue
MP 7364.5  : $0000000   ;Violet
MP 7364.6  : $0000000   ;Black
MP 7364.7  : $0AA0000   ;Gray line color (color 8)
MP 7364.8  : $000EEEE   ;Color for highlighted graphic elements
MP 7364.9  : $0DBD3DB   ;Background
             ;--------------------------------------------------------------
             ;Oscilloscope
MP 7365.0  : $0FFFFFF   ;Background
MP 7365.1  : $0808080   ;Grid
MP 7365.2  : $00000FF   ;Cursor and text
MP 7365.3  : $0FF0000   ;Selected channel
MP 7365.4  : $0C08030   ;Channel 1
MP 7365.5  : $000FF00   ;Channel 2
MP 7365.6  : $0FF00FF   ;Channel 3
MP 7365.7  : $00000FF   ;Channel 4
MP 7365.8  : $0FFCF00   ;Channel 5
MP 7365.9  : $000CFFF   ;Channel 6
             ;--------------------------------------------------------------
             ;Pop-up window (HELP key, pop-up menus)
MP 7366.0  : $0ECECEC   ;Background
MP 7366.1  : $0000000   ;Text or foreground
MP 7366.2  : $00000FF   ;Active line
MP 7366.3  : $0FF0000   ;Title line
MP 7366.4  : $0FFFFFF   ;Scroll-bar field
MP 7366.5  : $0FF00FF   ;Scroll bar
MP 7366.6  : $0000000   ;Reserved
MP 7366.7  : $0202020   ;Reserved
MP 7366.8  : $0404040   ;Reserved
MP 7366.9  : $0606060   ;Reserved
MP 7366.10 : $0808080   ;Reserved
MP 7366.11 : $0A0A0A0   ;Reserved
MP 7366.12 : $0C0C0C0   ;Reserved
MP 7366.13 : $0E0E0E0   ;Reserved
MP 7366.14 : $0FFFFFF   ;Reserved
             ;--------------------------------------------------------------
             ;Large PLC window
MP 7367.0  : $0ECECEC   ;Color 0  gray background
MP 7367.1  : $0000000   ;Color 1  black
MP 7367.2  : $0D0D0D0   ;Color 2  gray 10%
MP 7367.3  : $0A0A0A0   ;Color 3  gray 20%
MP 7367.4  : $0808080   ;Color 4  gray 40%
MP 7367.5  : $0404040   ;Color 5  gray 60%
MP 7367.6  : $0202020   ;Color 6  gray 80%
MP 7367.7  : $000FF9C   ;Color 7  bilious green
MP 7367.8  : $0FFFF7C   ;Color 8  yellow
MP 7367.9  : $0FF0000   ;Color 9  red
MP 7367.10 : $000FF00   ;Color 10 green
MP 7367.11 : $0001C5C   ;Color 11 dark blue
MP 7367.12 : $000FFFF   ;Color 12 turquoise
MP 7367.13 : $00000FF   ;Color 13 blue
MP 7367.14 : $0FF00FF   ;Color 14 violet
             ;--------------------------------------------------------------
             ;Pocket calculator
MP 7368.0  : $0ACACAC   ;Background
MP 7368.1  : $0FFFFFF   ;Background for keys and displays
MP 7368.2  : $00000FF   ;Key texts ("os" in "cos")
MP 7368.3  : $0FF0040   ;Key symbols
             ;--------------------------------------------------------------
             ;Directory tree in PGM MGT
MP 7369.0  : $0ECECEC   ;Text background
MP 7369.1  : $0000000   ;Text
MP 7369.2  : $00000FF   ;Text background of the active folder
MP 7369.3  : $0000000   ;Line color of the tree structure
MP 7369.4  : $0FF6000   ;Folders
MP 7369.5  : $0FF0040   ;Drives
MP 7369.6  : $0FF0000   ;Text background in title of the browser window
             ;--------------------------------------------------------------
             ;Small PLC window
MP 7370.0  : $0C0C0C0   ;Color 0  gray background
MP 7370.1  : $0000000   ;Color 1  black
MP 7370.2  : $0D0D0D0   ;Color 2  gray 10%
MP 7370.3  : $0A0A0A0   ;Color 3  gray 20%
MP 7370.4  : $0808080   ;Color 4  gray 40%
MP 7370.5  : $0404040   ;Color 5  gray 60%
MP 7370.6  : $0202020   ;Color 6  gray 80%
MP 7370.7  : $000FF9C   ;Color 7  bilious green
MP 7370.8  : $0FFFF7C   ;Color 8  yellow
MP 7370.9  : $0FF0000   ;Color 9  red
MP 7370.10 : $000FF00   ;Color 10 green
MP 7370.11 : $0001C5C   ;Color 11 dark blue
MP 7370.12 : $000FFFF   ;Color 12 turquoise
MP 7370.13 : $00000FF   ;Color 13 blue
MP 7370.14 : $0FF00FF   ;Color 14 violet
MP 7370.15 : $0FFFFFF   ;Color 15 white
             ;--------------------------------------------------------------
             ;Status window and PLC window
MP 7371.0  : $0ECECEC   ;Background window
MP 7371.1  : $0FFFFFF   ;Background elements
MP 7371.2  : $00000FF   ;Values other than positions
MP 7371.3  : $00000FF   ;Positions
             ;--------------------------------------------------------------
             ;smarT.NC
MP 7375.0  : $0E8E8E8   ;Background: forms
MP 7375.1  : $0C0C0C0   ;Background: not selected tab
MP 7375.2  : $0FFFFFF   ;Background: tree view and input boxes
MP 7375.3  : $0D0D0D0   ;Background: inactive input box
MP 7375.4  : $0D2D2D2   ;Background: help illustrations
MP 7375.5  : $00022FF   ;Cursor: tree view and background current block
MP 7375.6  : $0A0E0FF   ;Cursor: Treeview, if focus in form
MP 7375.7  : $0000000   ;Text colour: inactive input box
MP 7375.8  : $0FFFFFF   ;Text colour: active input box
MP 7375.9  : $00000FF   ;Text colour: radio buttons
MP 7375.10 : $0A0A0A0   ;Text colour: inactive label
MP 7375.11 : $0FF0000   ;Background: Radio/check buttons with "mouseover"
MP 7375.12 : $000FF00   ;Global data PREDEF: background
MP 7375.13 : $0000000   ;Global data PREDEF: text
MP 7375.14 : $0FF0000   ;Global data changed: background
MP 7375.15 : $0000000   ;Global data changed: text
MP 7375.16 : $0FFFFC0   ;Tooltip: background
MP 7375.17 : $0000000   ;Tooltip: text
MP 7375.18 : $0FF0000   ;Dialog title: background
MP 7375.19 : $0000000   ;Dialog title: text
MP 7375.20 : $000FA00   ;Pattern generator: positions on same level
MP 7375.21 : $00000C8   ;Pattern generator: current active positions
MP 7375.22 : $0800000   ;Pattern generator: deleted positions
MP 7375.23 : $0FF6432   ;Pattern generator: fade out positions
MP 7375.24 : $07B342D   ;Pattern generator: rectangle for zoom
             ;==============================================================
             ;MP 7400 .. 7599
             ;Machining and Program Run
             ;==============================================================
             ;MP7400 Look-Ahead memory of interpolation unit (only with
             ;software option 2)
             ;Input: 0 = 256 blocks
             ;       1 = 512 blocks
             ;       2 = 1024 blocks
MP 7400    : 0
             ;--------------------------------------------------------------
             ;MP7410 Scaling cycle in two or three axes
             ;Input:
             ; 0 = Scaling cycle is effective in all three principal axes
             ; 1 = Scaling cycle is effective only in the working plane
MP 7410    : 0
             ;--------------------------------------------------------------
             ;MP7411 Tool data in the touch probe block
             ;Input: 0 = The calibrated data of the touch probe is used
             ;       1 = The current tool data (display of tool name and
             ;           tool number) from the last TOOL CALL is used.
MP 7411    : 1
             ;--------------------------------------------------------------
             ;MP7412 Settings for cycles
             ;Input: %x
             ;Bit 0  Behavior of marker M4031 in Cycle 209
             ;       0 = Set M4031 after the spindle orientation
             ;           (previous behavior)
             ;       1 = Set M4031 before the spindle orientation
MP 7412    : 0
             ;--------------------------------------------------------------
             ;MP7420 Cycles for milling pockets with combined contours
             ;Input: %xxxxx
             ;Bit 0   Milling direction for channel milling
             ;        0 = Channel milling of the contours for pockets
             ;            counterclockwise, clockwise for islands
             ;        1 = Channel milling of the contours for pockets
             ;            clockwise, counterclockwise for islands
             ;Bit 1   Sequence for rough-out and channel milling (only SL1)
             ;        0 = First channel milling, then pocket rough-out
             ;        1 = First pocket rough-out, then channel milling
             ;Bit 2   Merging of listed contours
             ;        0 = Contours are merged only if the
             ;            tool center paths intersect
             ;        1 = Contours are merged if the
             ;            programmed contours intersect
             ;Bit 3   Rough-out and channel milling to pocket depth or
             ;        for every infeed
             ;        0 = Rough-out and channel milling uninterrupted
             ;            to pocket depth
             ;        1 = For each process: first channel milling and then
             ;            rough-out (depending on bit 1), before next
             ;            infeed occurs
             ;Bit 4   Traverse after machining the contour pocket
             ;        0 = Position that was traversed to before machining
             ;        1 = Tool axis to clearance height
             ;76543210 <- bit #
MP 7420    : %00000000
             ;--------------------------------------------------------------
             ;MP7430 Overlap factor for pocket milling
             ;Input: 0.1 to 1.414
MP 7430    : 1
             ;--------------------------------------------------------------
             ;MP7431 Arc end-point tolerance
             ;Input: 0.0001 to 0.016 [mm]
MP 7431    : 0.01
             ;--------------------------------------------------------------
             ;MP7432 Limit switch tolerance with M140/M150
             ;Input: 0.0001 to 1.0000 [mm]
             ;0 = Limit switch tolerance deactivated
MP 7432    : 0
             ;--------------------------------------------------------------
             ;MP7440 Output of M functions
             ;Input: %xxxxx
             ;Bit 0   Program stop with M06
             ;        0 = Program stop with M06
             ;        1 = No program stop with M06
             ;Bit 1   Modal cycle call M89
             ;        0 = Normal code transfer of M89 at beginning of block
             ;        1 = Modal cycle call M89 at end of block
             ;Bit 2   Program stop with M functions
             ;        0 = Program stop until acknowledgement of the
             ;            M function
             ;        1 = No program stop (no waiting for confirmation)
             ;Bit 3   Switching of kv factors with M105/M106
             ;        0 = Function is not in effect
             ;        1 = Function is in effect
             ;Bit 4   Reduced feed rate in the tool axis with M103
             ;        0 = Function is not in effect
             ;        1 = Function is in effect
             ;Bit 5   Reserved
             ;Bit 6   M134 is automatically activated when an
             ;        NC program is selected
             ;        0 = Function is not in effect
             ;        1 = Function is in effect
             ;Bit 7   Internal spindle status after M00
             ;        0 = Switching to M05
             ;        1 = Not switching to M05
             ;76543210 <- bit #
MP 7440    : %01011011
             ;--------------------------------------------------------------
             ;MP7441 Error message during cycle call
             ;Input: %xxx bit-encoded
             ;Bit 0   Error message "Spindle ?" when calling a
             ;        machining cycle
             ;        0 = Error message is not suppressed
             ;        1 = Error message is suppressed
             ;Bit 1   Reserved
             ;Bit 2   Error message "Enter depth as negative" for the
             ;        machining cycles when depth is programmed as positive
             ;        0 = Error message is suppressed
             ;        1 = Error message is not suppressed
             ;210 <- bit #
MP 7441    : %100
             ;--------------------------------------------------------------
             ;MP7442 Number of the M function for spindle orientation
             ;       in the cycles
             ;       (e.g.: Cycles 202/204/209)
             ;Input: -1 to 999
             ;       -1: Oriented spindle stop by the NC
             ;        0: No oriented spindle stop
             ; 1 to 999: Number of the M function
MP 7442    : +20
             ;--------------------------------------------------------------
             ;MP7444 Delay time for the change signals M/S/T
             ;Input: 0 = Change signal after complete filter run time
             ;       1 = Change signal if the nominal feed rate reaches 0
             ;           before the actual value (formed by the filter) did.
             ;       2 = Change signal if the nominal feed rate reaches 0
             ;           before the actual value (formed by the filter) did,
             ;           and an additional delay until the actual value
             ;           reaches the time window the first time.
MP 7444    : 0
             ;--------------------------------------------------------------
             ;MP7450 Activate simulated tool change position from MP951
             ;Input: %987654321 bit-encoded
             ;       0 = Do not offset
             ;       1 = Offset
             ;.............987654ZYX <- axis
             ;221111111111----------
             ;1098765432109876543210 <- bit #
MP 7450    : %0000000000000000000100
             ;--------------------------------------------------------------
             ;MP7451 Feed rate for returning to the contour
             ;Input: 10 to 1 000 000 [mm/min]
MP 7451.0  : 2000
MP 7451.1  : 2000
MP 7451.2  : 2000
MP 7451.3  : 2000
MP 7451.4  : 2000
MP 7451.5  : 2000
MP 7451.6  : 2000
MP 7451.7  : 2000
MP 7451.8  : 2000
MP 7451.9  : 2000
MP 7451.10 : 2000
MP 7451.11 : 2000
MP 7451.12 : 2000
MP 7451.13 : 2000
             ;--------------------------------------------------------------
             ;MP7460 reserved
MP 7460.0  : 0
MP 7460.1  : 0
             ;--------------------------------------------------------------
             ;MP7461 reserved
MP 7461.0  : 0
MP 7461.1  : 0
             ;--------------------------------------------------------------
             ;MP7470 Maximum feed rate at 100% override
             ;Input: 0 = No limitation
             ;       1 = To 300 000 [mm/min]
MP 7470    : 0
             ;--------------------------------------------------------------
             ;MP7471 Maximum feed rate of linear axes with M128 (compensating
             ;movement by positioning of an angular axis)
             ;Input: 0 = No limitation
             ;       1 = To 300 000 [mm/min]
MP 7471    : 0
             ;--------------------------------------------------------------
             ;MP7475 Reference for datum points
             ;Input: 0 = Reference is workpiece datum
             ;       1 = Reference is machine datum
MP 7475    : 0
             ;--------------------------------------------------------------
             ;MP7480 Tool change, output of T/P code
             ;Input:   0 = No output
             ;         1 = Fixed pocket T number if change
             ;         2 = Fixed pocket T number always
             ;         3 = Variable T/P number if change
             ;         4 = Variable T/P number always
             ;         5 = Fixed pocket T/P number if change
             ;         6 = Fixed pocket T/P number always
MP 7480.0  : 3          ;Tool Call double-arm = 3, no ATC = 1
MP 7480.1  : 3          ;Tool Def  double-arm = 3, no ATC = 0
             ;--------------------------------------------------------------
             ;MP7481 Removing/returning tools between magazines
             ;Input: %xxxxxxxx
             ;       0 = Control removing/returning with M4540
             ;       1 = First remove new, then return old
             ;76543210 <- bit #
MP 7481.0  : %00000000  ;Magazine 1 for returning the old tool
MP 7481.1  : %00000000  ;Magazine 2 for returning the old tool
MP 7481.2  : %00000000  ;Magazine 3 for returning the old tool
MP 7481.3  : %00000000  ;Magazine 4 for returning the old tool
MP 7481.4  : %00000000  ;Magazine 5 for returning the old tool
MP 7481.5  : %00000000  ;Magazine 6 for returning the old tool
MP 7481.6  : %00000000  ;Magazine 7 for returning the old tool
MP 7481.7  : %00000000  ;Magazine 8 for returning the old tool
MP 7481.8  : %00000000  ;Magazine 9 for returning the old tool
MP 7481.9  : %00000000  ;Magazine 10 for returning the old tool
MP 7481.10 : %00000000  ;Magazine 11 for returning the old tool
MP 7481.11 : %00000000  ;Magazine 12 for returning the old tool
MP 7481.12 : %00000000  ;Magazine 13 for returning the old tool
MP 7481.13 : %00000000  ;Magazine 14 for returning the old tool
MP 7481.14 : %00000000  ;Magazine 15 for returning the old tool
MP 7481.15 : %00000000  ;Magazine 16 for returning the old tool
             ;--------------------------------------------------------------
             ;MP7482 Fixed pocket coding for magazines 1 to 16
             ;Input: %xxx
             ;       0 = Magazine does not use fixed pocket coding
             ;       1 = Magazine uses fixed pocket coding
             ;Bit 0..15 - Magazine 1..16
             ;1111111---------
             ;6543210987654321 <- magazine #
MP 7482    : %0000000000000000
             ;--------------------------------------------------------------
             ;MP7483 Using Tool-Number or Tool-Name in TOOL CALL/TOOL DEF
             ;Input: 0 = Tool-Name and Tool-Number allowed
             ;       1 = only Tool-Name allowed
             ;       2 = only Tool-Number allowed
MP 7483    : 0
             ;--------------------------------------------------------------
             ;MP7484 Order of the Tool-Magazines which should be scanned
             ;       for the called tool
             ;Input: 0 to 7 [Index from MP7261]
             ;       -1 = Cancel
MP 7484.0  : -1
MP 7484.1  : -1
MP 7484.2  : -1
MP 7484.3  : -1
MP 7484.4  : -1
MP 7484.5  : -1
MP 7484.6  : -1
MP 7484.7  : -1
MP 7484.8  : -1
MP 7484.9  : -1
MP 7484.10 : -1
MP 7484.11 : -1
MP 7484.12 : -1
MP 7484.13 : -1
MP 7484.14 : -1
MP 7484.15 : -1
             ;--------------------------------------------------------------
             ;MP7485 Act upon usage time for tool selection
             ;Input: 0 to 100 [%]
MP 7485    : 10
             ;--------------------------------------------------------------
             ;MP7490 Number of traverse ranges
             ;Input: %xxxx
             ;Bit 0   Number of traverse ranges
             ;        0 = One traverse range
             ;        1 = Three traverse ranges
             ;Bit 1   Number of datums
             ;        0 = One datum per traverse range
             ;        1 = One datum for all traverse ranges
             ;Bit 2   Calibration data for workpiece measurement
             ;        0 = One set of calibration data for all traverse ranges
             ;        1 = Separate set of calibration data for each
             ;            traverse range
             ;Bit 3   Calibration data for tool measurement
             ;        0 = One set of calibration data for all traverse ranges
             ;        1 = Separate set of calibration data for each
             ;            traverse range
             ;3210 <- bit #
MP 7490    : %0001
             ;--------------------------------------------------------------
             ;MP 7492 Number of the axis in which the same datum is set
             ;        during datum setting
             ;        (e.g. parallel axes)
             ;        (with active preset table)
             ;Input:   0 to 17 [number of the axis]
             ;         -1 = Do not set a datum
MP 7492.0  : -1         ;Datum set in the first axis
MP 7492.1  : -1         ;Datum set in the second axis
MP 7492.2  : -1         ;Datum set in the third axis
MP 7492.3  : -1         ;Datum set in the fourth axis
MP 7492.4  : -1         ;Datum set in the fifth axis
MP 7492.5  : -1         ;Datum set in the sixth axis
MP 7492.6  : -1         ;Datum set in the seventh axis
MP 7492.7  : -1         ;Datum set in the eighth axis
MP 7492.8  : -1         ;Datum set in the ninth axis
MP 7492.9  : -1         ;Datum set in the tenth axis
MP 7492.10 : -1         ;Datum set in the eleventh axis
MP 7492.11 : -1         ;Datum set in the twelfth axis
MP 7492.12 : -1         ;Datum set in the thirteenth axis
MP 7492.13 : -1         ;Datum set in the fourteenth axis
             ;--------------------------------------------------------------
             ;MP7493 Tolerance at datum setting with M114;
             ;       Max. deviation of actual tool orientation to the tool
             ;       axis (of swivelt coordination system) at datum setting
             ;Input: 0 to 30,0000 [degrees]
MP 7493    : 0.005
             ;--------------------------------------------------------------
             ;MP7494 Axes that always require a precision stop when
             ;       positioning (with rotary axes even irrespective of
             ;       M134)
             ;Input: %987654321 bit-encoded
             ;         0 = No precision stop
             ;         1 = Precision stop
             ;.............987654ZYX <- axis
             ;221111111111----------
             ;1098765432109876543210 <- bit #
MP 7494    : %0000000000000000000000
             ;--------------------------------------------------------------
             ;MP7502 M144 (M145) Include machine kinematics
             ;Input: %xxx
             ;Bit 0   0 = M144/M145 inactive
             ;        1 = M144/M145 active
             ;Bit 1   M144/M145 in the automatic modes
             ;        0 = M144/M145 active
             ;        1 = M144 is activated automatically at the start of
             ;            an NC program
             ;Bit 2   M144/M145 in the manual modes
             ;        0 = M144/M145 inactive
             ;        1 = M144/M145 active
             ;210 <- bit #
MP 7502    : %000
             ;--------------------------------------------------------------
             ;MP7503 Activating of FCL function "Virtual Tool-axis"
             ;       supports Moving/Repositioning in tool-axis direction
             ;Input: 0 = Inactive
             ;       1 = Active
MP 7503    : 0
             ;--------------------------------------------------------------
             ;MP7506 Selection of a kinematic while booting up the control
             ;Input: -1 = Inactive
             ;       0 to 999
MP 7506    : +0
             ;--------------------------------------------------------------
             ;MP7507 Selection of a kinematic via softkey
             ;Input: %xxx
             ;Bit 0   0 = Selection of a kinematic in editing operting modes
             ;            not possible
             ;        1 = Selection of a kinematic in editing operting modes
             ;            for simulation in program test mode possible
             ;Bit 1   0 = Selection of a kinematic in machining operting
             ;            modes not possible
             ;        1 = Selection of a kinematic in machining operting
             ;            modes for the real machine possible
             ;Bit 2   0 = Softkey 3D ROT not available in Test Run mode
             ;        1 = Softkey 3D ROT available in Test Run mode
             ;Bit 3   0 = Keyword KINEMATIC possible
             ;        1 = Keyword KINEMATIC disabled
             ;3210 <- bit #
MP 7507    : %0101
MP 7600.0  : 5
MP 7600.1  : 7
             ;--------------------------------------------------------------
             ;MP7602 PLC cycle time
             ;Input: 0 to 60 [ms]
             ;       0 - 12 = 12 ms
MP 7602    : 20
             ;--------------------------------------------------------------
             ;MP7620 Feed-rate override and spindle speed override
             ;Input: %xxxxxxxxxx
             ;Bit 0   Feed rate override if rapid traverse key is pressed
             ;        in Program Run mode
             ;        0 = Override not effective
             ;        1 = Override effective
             ;Bit 1   Non-functional
             ;Bit 2   Feed rate override if rapid traverse key and machine
             ;        direction buttons pressed in Manual operating mode
             ;        0 = Override not effective
             ;        1 = Override effective
             ;Bit 3   TEs overrides
             ;        TEs without rapid traverse override
             ;        0 = Feed rate override and spindle speed override in
             ;            resolution of bit 4 setting
             ;        1 = Feed rate override and spindle speed override in
             ;            non-linear curve
             ;
             ;        TEs with rapid traverse override (rapid traverse
             ;        override always in non-linear curve)
             ;        0 = Feed rate override and spindle speed override in
             ;            1 % steps
             ;        1 = Feed rate override and spindle speed override in
             ;            non-linear curve
             ;Bit 4   TEs overrides
             ;        TEs without rapid traverse override
             ;        0 = Feed rate override and spindle speed override in
             ;            1 % steps
             ;        1 = Feed rate override and spindle speed override in
             ;            non-linear curve
             ;
             ;        TEs with rapid traverse override
             ;        Non-functional
             ;Bit 5   TE with potentiometer for rapid traverse override
             ;        instead of spindle speed override
             ;        Please refer to the notes in the Technical Manual
             ;        0 = TE with spindle speed override
             ;        1 = TE with rapid traverse override
             ;Bit 6   Feedrate smoothing
             ;        0 = Inactive
             ;        1 = Active
             ;Bit 7   Only with Functional Safety:
             ;        Adaption of 100% position of potentiometers to F-LIMIT
             ;        0 = Inactive
             ;        1 = Adapt 100% position to F-LIMIT,
             ;            100% is equivalent to active SLS limit
             ;Bit 8   Error message if feed rate or rapid traverse override
             ;        at 0 %
             ;        0 = Inactive
             ;        1 = Active
             ;Bit 9   HSCI keyboard with three override potentiometers (feed
             ;        rate-, spindle speed- and rapid traverse override)
             ;        0 = Inactive
             ;        1 = Active
             ;9876543210 <- bit #
MP 7620    : %0000001111
             ;--------------------------------------------------------------
             ;MP7621 Increments of the spindle override by the "+"- and "-"-
             ;       keys (just for corresponding operating panels)
             ;Input: 1 to 10 [%]
             ;         0 = 5 %
MP 7621    : 0
             ;--------------------------------------------------------------
             ;MP7630 Time after EMERGENCY STOP-Test to be in standby again
             ;Input: 1 to 999 [ms]
             ;       0 = previous behaviour
MP 7630    : 0
             ;--------------------------------------------------------------
             ;MP7640 Handwheel
             ;       MP7640.0 for X23 of the last machine operating panel
             ;       in the HSCI chain
             ;       MP7640.1 to .3 reserved
             ;       MP7640.4 for X29 of the MC
             ;Input: 0 = No handwheel
             ;       1 = HR330 (evaluate all keys by NC)
             ;       2 = HR130 or HR330
             ;       3 = HR330 (evaluation of keys +, - and
             ;           "rapid traverse" by PLC)
             ;       4 = HR332 (evaluate all keys by PLC)
             ;       5 = Up to three HR150 via HRA110
             ;       6 = HR410
             ;       7 = reserved
             ;       8 = reserved
             ;       9 = reserved
             ;      10 = reserved
             ;      11 = HR420/HR520 without supporting the LEDs
             ;      12 = HR550 (wireless handwheel)
             ;      13 = HR520 with supporting the LEDs
MP 7640.0  : 6
MP 7640.1  : 0
MP 7640.2  : 0
MP 7640.3  : 0
MP 7640.4  : 0
             ;--------------------------------------------------------------
             ;MP7641 Functions of the handwheel
             ;Input: %xxxxxxxxxx
             ;Bit 0   Entry of the interpolation factor
             ;        0 = Through TNC keyboard
             ;        1 = Through PLC Module 9036
             ;Bit 1   Handwheel with detent positions
             ;        0 = Without detent positions
             ;        1 = With detent positions
             ;Bit 2   Axis direction keys and rapid traverse (HR 420/HR 5xx)
             ;        0 = By the NC
             ;        1 = By the PLC
             ;Bit 3   NC start/NC stop on the HR 420
             ;        0 = By the NC
             ;        1 = By the PLC
             ;Bit 4   Handwheel superimpositioning in tool-axis direction
             ;        can be activated
             ;        0 = Handwheel superimpositioning in tool-axis direction
             ;            not possible
             ;        1 = Handwheel superimpositioning in tool-axis direction
             ;            can be activated in global settings
             ;Bit 5   Inactive behaviour of the HR 420/HR 5xx
             ;        0 = Keys on HR 420 only sent to PLC when HR is active
             ;        1 = Keys on HR 420 sent to PLC also, when the HR is
             ;            inactive
             ;Bit 6   Selecting and traversing auxiliary axes with HR 420
             ;        /HR 5xx
             ;        0 = not possible
             ;        1 = possible
             ;Bit 7   TeachIn button on HR 550
             ;        0 = by NC
             ;        1 = by PLC
             ;Bit 8   CTRL button on HR 550
             ;        0 = by NC
             ;        1 = by PLC
             ;Bit 9   PLC softkeys and HR 420/HR 5xx active
             ;        Please refer to the notes in the Technical Manual
             ;        0 = PLC softkeys inactive if HR active
             ;        1 = PLC-softkeys active if HR active
             ;Bit 10  Reserved
             ;1----------
             ;09876543210 <- bit #
MP 7641    : %00000110000
             ;--------------------------------------------------------------
             ;MP7642 Handwheel name
             ;       MP7642.0 for X23 of the last machine operating panel
             ;       in the HSCI chain
             ;       MP7642.1 to .3 reserved
             ;       MP7642.4 for X29 of the MC
             ;Input: Handwheel name, max 31 characters
MP 7642.0  : 
MP 7642.1  : 
MP 7642.2  : 
MP 7642.3  : 
MP 7642.4  : 
             ;--------------------------------------------------------------
             ;MP7645.0 Handwheel initialization (HR332/HR410/HRA110)
             ;Input:
             ;HR332: Evaluation of handwheel keys and LED
             ;       0 = Axis keys by NC (other keys by PLC)
             ;       1 = Keys by PLC
             ;HR410: Evaluation of handwheel keys and LED
             ;       0 = Keys by NC (function keys FCT A-C by PLC)
             ;       1 = Keys by PLC
             ;HRA110: Assignment of the 3rd handwheel axis via
             ;        axis selection switch
             ;       0 = Left stop = Z axis
             ;       1 = Left stop = X axis
             ;       2 = Switch setting 3 = Z axis
             ;HRA5xx: Interpretation of permissive buttons
             ;       0 = both relays as normally open contacts
             ;       1 = one relays as normally open contact
             ;           one relays as normally closed contact
MP 7645.0  : 0
             ;--------------------------------------------------------------
             ;MP7645.1 HRA110 initialization: Assignment of the 3rd
             ;         handwheel axis via MP
             ;Input: 0 = Simulation of switch setting 1 (left stop)
             ;           of the axis selection switch. Assignment of the 3rd
             ;           handwheel via MP 7645.0 (input value 0 or 1)
             ;       1 = X axis
             ;       2 = Y axis
             ;       4 = Z axis
             ;       8 = Axis 4
             ;      16 = Axis 5
MP 7645.1  : 4
             ;--------------------------------------------------------------
             ;MP7645.2 HRA110 initialization: Axis selection via axis
             ;         selection switch or MP 7645.1
             ;Input: 0 = Axis selection: Axis selection switch
             ;       1 = Axis selection through MP 7645.1
MP 7645.2  : 0
             ;--------------------------------------------------------------
             ;MP7645.3 HR550 Behaviour relay contact permissive keys
             ;Input: 0 = 2 closer
             ;       1 = 1 closer / 1 opener (for FS necessary)
MP 7645.3  : 0
             ;--------------------------------------------------------------
MP 7645.4  : 0          ;Reserved
MP 7645.5  : 0          ;Reserved
MP 7645.6  : 0          ;Reserved
MP 7645.7  : 0          ;Reserved
             ;--------------------------------------------------------------
             ;MP7646.x reserved
MP 7646.0  : 0
MP 7646.1  : 0
MP 7646.2  : 0
MP 7646.3  : 0
MP 7646.4  : 0
MP 7646.5  : 0
MP 7646.6  : 0
MP 7646.7  : 0
             ;--------------------------------------------------------------
             ;MP7647.x reserved
MP 7647.0  : 0
MP 7647.1  : 0
MP 7647.2  : 0
MP 7647.3  : 0
MP 7647.4  : 0
MP 7647.5  : 0
MP 7647.6  : 0
MP 7647.7  : 0
             ;--------------------------------------------------------------
             ;MP7648.x reserved
MP 7648.0  : 0
MP 7648.1  : 0
MP 7648.2  : 0
MP 7648.3  : 0
MP 7648.4  : 0
MP 7648.5  : 0
MP 7648.6  : 0
MP 7648.7  : 0
             ;--------------------------------------------------------------
             ;MP7649.0 Handwheel initialization (HR332/HR410/HRA110/HR550FS)
             ;         MP7649.x for X29 of the MC
             ;Input:
             ;HR332: Evaluation of handwheel keys and LED
             ;       0 = Axis keys by NC (other keys by PLC)
             ;       1 = Keys by PLC
             ;HR410: Evaluation of handwheel keys and LED
             ;       0 = Keys by NC (function keys FCT A-C by PLC)
             ;       1 = Keys by PLC
             ;HRA110: Assignment of the 3rd handwheel axis via
             ;        axis selection switch
             ;       0 = Left stop = Z axis
             ;       1 = Left stop = X axis
             ;       2 = Switch setting 3 = Z axis
MP 7649.0  : 0
             ;--------------------------------------------------------------
             ;MP7649.1 HRA110 initialization: Assignment of the 3rd
             ;         handwheel axis via MP
             ;Input: 0 = Simulation of switch setting 1 (left stop)
             ;           of the axis selection switch. Assignment of the 3rd
             ;           handwheel via MP 7649.0 (input value 0 or 1)
             ;       1 = X axis
             ;       2 = Y axis
             ;       4 = Z axis
             ;       8 = Axis 4
             ;      16 = Axis 5
MP 7649.1  : 0
             ;--------------------------------------------------------------
             ;MP7649.2 HRA110 initialization: Axis selection via axis selection
             ;         switch or MP 7649.1
             ;Input: 0 = Axis selection: Axis selection switch
             ;       1 = Axis selection through MP 7645.1
MP 7649.2  : 0
             ;--------------------------------------------------------------
             ;MP7649.3 HRA5xx: Interpretation of permissive buttons
             ;Input: 0 = both relays as normally open contacts
             ;       1 = one relays as normally open contact
             ;           one relays as normally closed contact
MP 7649.3  : 0
MP 7649.4  : 0          ;Reserved
MP 7649.5  : 0          ;Reserved
MP 7649.6  : 0          ;Reserved
MP 7649.7  : 0          ;Reserved
             ;--------------------------------------------------------------
             ;MP7650 Counting direction for handwheel
             ;       MP7650.0 for X23 of the last machine operating panel
             ;       in the HSCI chain
             ;       MP7650.1 to .3 reserved
             ;       MP7650.4 for X29 of the MC
             ;Input: 0 = Negative counting direction
             ;       1 = Positive counting direction
             ;.............987654ZYX <- axis
             ;221111111111----------
             ;1098765432109876543210 <- bit #
MP 7650.0  : %0000000000000000000000
MP 7650.1  : %0000000000000000000000
MP 7650.2  : %0000000000000000000000
MP 7650.3  : %0000000000000000000000
MP 7650.4  : %0000000000000000000000
             ;--------------------------------------------------------------
             ;MP7660 Threshold sensitivity for electronic handwheel
             ;Input: 0 to 65 535 [increments]
MP 7660    : 100
             ;--------------------------------------------------------------
             ;MP7670 Minimum interpolation factor for handwheel
             ;Input: 0 to 10
MP 7670.0  : 10         ;For low speed
MP 7670.1  : 3          ;For medium speed (only HR 410)
MP 7670.2  : 0          ;For high speed (only HR 410)
             ;--------------------------------------------------------------
             ;MP7671 Manual feed rate in Handwheel operating mode with HR 410
             ;Input: 0 to 1000 [% of MP1020]
MP 7671.0  : 10         ;Low speed
MP 7671.1  : 50         ;Medium speed
MP 7671.2  : 100        ;High speed
             ;--------------------------------------------------------------
             ;MP7672 For HR 410 a path for every detent position can be set
             ;       if MP7641 Bit1 = 1
             ;Input: 0,0000 to 1,0000 [mm]
MP 7672.0  : 0.001      ;Low speed
MP 7672.1  : 0.01       ;Medium speed
MP 7672.2  : 0.1        ;High speed
             ;--------------------------------------------------------------
             ;MP7674 HR 130, HR 330, HR 332 always and HR 4x0,
             ;       if MP7641 Bit1 = 0 (without detent positions),
             ;       a minimum axis-specific interpolation factor
             ;       can be set
             ;Input: 0 = no limitation
             ;         1 to 10
MP 7674.0  : 0
MP 7674.1  : 0
MP 7674.2  : 0
MP 7674.3  : 0
MP 7674.4  : 0
MP 7674.5  : 0
MP 7674.6  : 0
MP 7674.7  : 0
MP 7674.8  : 0
MP 7674.9  : 0
MP 7674.10 : 0
MP 7674.11 : 0
MP 7674.12 : 0
MP 7674.13 : 0
             ;--------------------------------------------------------------
             ;MP7675 HR 130, HR 330, HR 332 of no significance
             ;       HR 410, HR 420 if MP7641 Bit1 = 1
             ;       (with detent positions) a maximum axis-specific
             ;       path for every detent position in [mm] can be set
             ;       HR 420 if MP7641 Bit1 = 0
             ;       (without detent positions) a maximum axis-specific
             ;       path every handwheel-revolution in [mm] can be set
             ;Input: 0 = no limitation
             ;         0,0001 to 10,0000 [mm]
MP 7675.0  : 0
MP 7675.1  : 0
MP 7675.2  : 0
MP 7675.3  : 0
MP 7675.4  : 0
MP 7675.5  : 0
MP 7675.6  : 0
MP 7675.7  : 0
MP 7675.8  : 0
MP 7675.9  : 0
MP 7675.10 : 0
MP 7675.11 : 0
MP 7675.12 : 0
MP 7675.13 : 0
             ;--------------------------------------------------------------
             ;MP7680 Machine parameter with multiple function
             ;Input: %xxxxxxxxxxxxxxxxxx
             ;Bit 0   Memory function for axis direction keys
             ;        0 = Not saved
             ;        1 = Saved
             ;Bit 1   Returning to the contour
             ;        0 = Inactive
             ;        1 = Active
             ;Bit 2   Block scan
             ;        0 = Inactive
             ;        1 = Active
             ;Bit 3   Interruption of block scan at Stop or M06
             ;        0 = Interruption
             ;        1 = No interruption
             ;Bit 4   Inclusion of programmed dwell time
             ;        during the block scan
             ;        0 = Include the dwell time
             ;        1 = Do not include the dwell time
             ;Bit 5   Start of calculation for block scan
             ;        0 = Start from block with cursor
             ;        1 = Start from beginning of program
             ;Bit 6   Tool length in blocks with normal vectors
             ;        0 = Without DR2 from tool table (lower edge PSP)
             ;        1 = With DR2 from tool table (center of sphere PT)
             ;Bit 7   M112: Inserting a defined rounding arc or spline
             ;        0 = Defined rounding arcs are always inserted
             ;        1 = Defined rounding arcs are always inserted if
             ;            the acceleration from MP1060.x or MP1070
             ;            was exceeded
             ;Bit 8   M112: Insertion of rounding arc or cubic spline
             ;        0 = Rounding arc is inserted
             ;        1 = A cubic spline is inserted instead of a
             ;            rounding arc
             ;Bit 9   M112: Constant jerk on spline (Bit8 = 1)
             ;        0 = No constant jerk
             ;        1 = Constant jerk
             ;Bit 10  Compensated outside corner with spline instead of
             ;        circle (corners remain sharp with nominal-value
             ;        filtering)
             ;        0 = Inactive
             ;        1 = Active
             ;Bit 11  Behavior of M116
             ;        0 = Rotary axis is parallel to linear axis
             ;        1 = Any position of rotary axis to linear axis
             ;Bit 12  Behavior of Cycle 28
             ;        0 = Standard behavior
             ;        1 = Tangential approach and departure to slot wall,
             ;            insert rounding at beginning and end of slot
             ;Bit 13  Behavior during program interruption with axis movement
             ;        0 = "Restore position" is automatically activated
             ;        1 = Do not activate "Restore position"
             ;Bit 14  NC start after NC stop and internal stop
             ;        0 = NC start permitted
             ;        1 = NC start only permitted after GOTO
             ;Bit 15  NC start after program termination
             ;        0 = NC start permitted
             ;        1 = NC start not permitted (information window)
             ;Bit 16  Approach/departure movements of cycle 39
             ;        0 = Arc approach/departure movements active
             ;        1 = Arc approach/departure movements inactive
             ;Bit 17  Behavior of markers M4175, M4176
             ;        0 = M4175, M4176 cleared at an internal stop
             ;        1 = M4176 remains set during RUNCANCEL macro
             ;Bit 18  Manual measurement of basic rotation with inclined
             ;        rotary axes
             ;        0 = Error message "Axis angle not equal to tilt angle"
             ;            is active
             ;        1 = Error message is not active
             ;111111111----------
             ;8765432109876543210 <- bit #
MP 7680    : %0000000010000111111
             ;--------------------------------------------------------------
             ;MP7681 M/S/T/Q transfer to the PLC during block scan
             ;Input: %xxxxx
             ;Bit 0   0 = Transfer M functions to PLC
             ;        1 = Collect M functions, transfer after block scan
             ;Bit 1   0 = Transfer T code to PLC
             ;        1 = After block scan, transfer last T code
             ;Bit 2   0 = Transfer S or G code
             ;        1 = After block scan, transfer last S or G code
             ;Bit 3   0 = Transfer FN19 to PLC
             ;        1 = After block scan, transfer last FN19
             ;Bit 4   0 = MP subfiles cannot be activated during block scan
             ;        1 = MP subfiles can be activated during block scan
             ;            (FN17 ID1020). If block scan is interrupted,
             ;            the changes remain active!
             ;543210 <- bit #
MP 7681    : %010111
             ;--------------------------------------------------------------
             ;MP7682 Machine parameter with multiple function
             ;Input:% xxxxxxxxxxxxxxx
             ;Bit 0   Incremental block after Tool Call
             ;        0 = With length compensation
             ;        1 = Without length compensation
             ;Bit 1   Set reference value for datum
             ;        (Use MP7500 bit 5, since nominal-to-actual value
             ;         transfer may change the datum)
             ;        0 = Actual value
             ;        1 = Nominal value
             ;Bit 2   Traverse path: Rotary axes (modulo)
             ;        0 = Positioning without passing over zero
             ;        1 = Positioning on the shortest path
             ;Bit 3   Reserved
             ;Bit 4   Tolerance for rotary axes with M114
             ;        0 = With consideration of head dimensions
             ;        1 = Without consideration of head dimensions
             ;Bit 5   Feed rate for M128
             ;        0 = Feed rate refers to tool tip
             ;        1 = Feed rate from interpolation of all axes involved
             ;Bit 6   Behaviour for TOOL DEF strobe
             ;        0 = TOOL DEF strobe must be acknowledged by PLC,
             ;            depending on the NC program (TOOL DEF within
             ;            a continous contour)
             ;        1 = TOOL DEF strobe must always be acknowledged by
             ;            PLC
             ;Bit 7   Block elements in TOOL CALL and S in ISO blocks
             ;        0 = Process them as programmed
             ;        1 = Process them at block start
             ;            (Block display will not be changed)
             ;Bit 8   Behaviour of M8 at the end of Cycle 202 and 204
             ;        0 = At the end of Cycle 202 and 204 the state of
             ;            M8 will be reconstituted
             ;        1 = M8 will not be reconstituted at the end of
             ;            Cycle 202 and 204
             ;Bit 9   Transfer "tilt working plane"-state from NC-Program to
             ;        "Manual Operation" during program interruption
             ;        0 = "Tilt working plane"-state from NC-Program will
             ;             not be transfered to "Manual Operation" during
             ;             program interruption
             ;        1 = "Tilt working plane"-state will be transfered
             ;             to "Manual Operation" during program interruption
             ;Bit 10  Peripheral Milling
             ;        0 = Peripheral Milling permitted
             ;        1 = Peripheral Milling inactive
             ;Bit 11  Reserved
             ;Bit 12  Error message for R2 > R in tool table
             ;        0 = Error message not suppressed
             ;        1 = Error message suppressed
             ;Bit 13  No program interruption upon invalid TOOL DEF
             ;        0 = Error message and NC stop upon invalid TOOL DEF
             ;        1 = Only warning upon invalid TOOL DEF
             ;Bit 14  No insertion of the tool if TIME2 has expired
             ;        0 = Tool is inserted even if TIME2 has expired
             ;        1 = TOOL CALL for a tool where TIME2 has expired
             ;            leads to an error message and NC stop
             ;Bit 15  Reserved
             ;Bit 16  Reserved
             ;Bit 17  Behavior of control-in-operation signal at program end
             ;        if a cancel macro exists
             ;        0 = Control-in-operation signal is deleted at program
             ;            end and is set again by the NC software by
             ;            executing the cancel macro (behavior as before)
             ;        1 = Control-in-operation signal is not deleted by the
             ;            NC software until the cancel macro has been
             ;            completed
             ;11111111----------
             ;765432109876543210 <- bit #
MP 7682    : %000000000000000111
             ;--------------------------------------------------------------
             ;MP7683 Executing pallets and NC programs
             ;Input: %xxxxxxxxxx
             ;Bit 0   reserved
             ;Bit 1   Operating mode Program Run, Full Sequence
             ;        (workpiece oriented)
             ;        0 = NC start runs a complete NC program for one
             ;            workpiece
             ;        1 = NC start runs all NC programs to the next
             ;            pallet
             ;Bit 2   Operating mode Program Run, Full Sequence
             ;        0 = As defined in bit 1
             ;        1 = NC start executes all NC programs and pallets
             ;            to the end of the table
             ;Bit 3   Operating mode Program Run, Full Sequence
             ;        0 = As defined in bit 1
             ;        1 = When the end of the table is reached, the process
             ;            begins again with the first line
             ;Bit 4   Operating mode Program Run, Full Sequence
             ;        0 = Active pallet table cannot be edited
             ;        1 = Active pallet table can be edited
             ;Bit 5   AUTOSTART soft key
             ;        0 = Do not display
             ;        1 = Display
             ;Bit 6   0 = Display pallet table and NC program
             ;        1 = Display pallet table or NC program
             ;Bit 7   AUTOSTART function
             ;        0 = AUTOSTART performed by the NC
             ;        1 = AUTOSTART performed by the PLC
             ;Bit 8   Operating mode Program Run, Full Sequence
             ;        (tool oriented)
             ;        0 = NC start runs over all connected workpieces
             ;            until next tool change
             ;        1 = NC start runs all NC programs to the next
             ;            pallet
             ;Bit 9   EDIT PALETTE soft key
             ;        0 = Do not display
             ;        1 = Display
             ;9876543210 <- bit #
MP 7683    : %0110101110
             ;--------------------------------------------------------------
             ;MP7684 Nominal position value filter and path control with M128
             ;Input: %xxxxxxxxxxx
             ;Bit 0   Nominal position value filter
             ;        0 = Include acceleration
             ;        1 = Do not include the acceleration
             ;Bit 1   Nominal position value filter
             ;        0 = Include the jerk
             ;        1 = Do not include the jerk
             ;        Nominal position value filter
             ;Bit 2   0 = Include the tolerance
             ;        1 = Do not include the tolerance
             ;Bit 3   Nominal position value filter
             ;        0 = Include the radial acceleration
             ;        1 = Do not include the radial acceleration
             ;Bit 4   Nominal position value filter (prior to MP1222 and
             ;        MP1223)
             ;        0 = Include jerk and tolerance limits at curvature
             ;            changes
             ;        1 = Do not include jerk and tolerance limits at
             ;            curvature changes
             ;Bit 5   reserved
             ;Bit 6   reserved
             ;Bit 7   reserved
             ;Bit 8   reserved
             ;Bit 9   Traversing with high rate of movement by rotation axes
             ;        in conjunction with M128 (movement smaller 2 degree
             ;        and M118 inactive)
             ;        0 = Previous behaviour
             ;        1 = Active
             ;Bit 10  Modification of the calculation of contouring feed
             ;        rate at the beginning of a contour-element
             ;        0 = Active
             ;        1 = Previous behaviour (up to and including 340490-02)
             ;1----------
             ;09876543210 <- bit #
MP 7684    : %00000000000
             ;--------------------------------------------------------------
             ;MP7690 Evaluation of the electronic ID labels
             ;Bit 0   HEIDENHAIN power modules
             ;        0 = Active
             ;        1 = Inactive
             ;Bit 1   HEIDENHAIN synchronous motors
             ;        0 = Active
             ;        1 = Inactive
             ;Bit 2   HEIDENHAIN power supply units
             ;        0 = Active
             ;        1 = Inactive
             ;210 <- bit #
MP 7690    : %000
             ;==============================================================
             ;MP 13000 .. 13999
             ;Spindle control (2nd spindle)
             ;==============================================================
             ;MP13010 Output of spindle speed, oriented spindle stop
             ;Input:
             ; 0 = No output of spindle speed
             ; 1 = Encoded output of spindle speed only
             ;     if the speed changes
             ; 2 = Encoded output of spindle speed at each Tool Call
             ; 3 = Analog output of spindle speed, but
             ;     gear changing signal only if the gear range changes
             ; 4 = Analog output of spindle speed, but
             ;     gear changing signal at every Tool Call
             ; 5 = Analog output of spindle speed and no
             ;     gear changing signal
             ; 6 = Same as 3, but with closed-loop spindle for
             ;     orientation
             ; 7 = Same as 4, but with closed-loop spindle for
             ;     orientation
             ; 8 = Same as 5, but with closed-loop spindle for
             ;     orientation
MP 13010   : 0
             ;--------------------------------------------------------------
             ;MP13011 Function of analog output s if MP13010 < 3
             ;Input: 0 = No special function (0 volt)
             ;       1 = Voltage proportional to the current machining feed
             ;           rate (depending on MP13012)
             ;       2 = Voltage is defined via the PLC module 9130
             ;       3 = Voltage is defined through M functions
             ;           (M200 to M204)
MP 13011   : 0
             ;--------------------------------------------------------------
             ;MP13012 Feed rate from output of an analog voltage of 10 V
             ;(MP13011 = 1)
             ;Input: 0 to 300 000 [mm/min]
             ;Laser power control M200-M204
MP 13012   : 0          ;0 to 30 000 m/min = 10 volts
             ;--------------------------------------------------------------
             ;MP13013.0 to MP13013.11 Characteristic curve kink points for
             ;output of the analog voltage with M202
             ;Input: 10 to 300 000 [mm/min]
MP 13013.0 : 0
MP 13013.1 : 0
MP 13013.2 : 0
MP 13013.3 : 0
MP 13013.4 : 0
MP 13013.5 : 0
MP 13013.6 : 0
MP 13013.7 : 0
MP 13013.8 : 0
MP 13013.9 : 0
MP 13013.10: 0
MP 13013.11: 0
             ;--------------------------------------------------------------
             ;MP13014.0 to MP13014.11 Characteristic curve kink points for
             ;output of the analog voltage with M202
             ;Input: 0.000 to 9.999 [V]
MP 13014.0 : 0
MP 13014.1 : 0
MP 13014.2 : 0
MP 13014.3 : 0
MP 13014.4 : 0
MP 13014.5 : 0
MP 13014.6 : 0
MP 13014.7 : 0
MP 13014.8 : 0
MP 13014.9 : 0
MP 13014.10: 0
MP 13014.11: 0
             ;--------------------------------------------------------------
             ;MP13020 Definition of speed range of BCD spindle
             ;Input: 0 to 99 999
             ;Limitation of speed codes (no limitation with 01991)
             ;    1* increment (1..9)
             ;+  10* max. code (1..99)
             ;+1000* min. code (1..99)
MP 13020   : 1991
             ;--------------------------------------------------------------
             ;MP13030 Axis stop with a Tool Call in which only a
             ;spindle speed is transferred
             ;Input: %xx
             ;Bit 0   Axis stops with Tool Call
             ;        0 = Active
             ;        1 = Inactive
             ;Bit 1   Regulate to speed 0 when switching to another
             ;        gear range
             ;        0 = Active
             ;        1 = Inactive
             ;10 <- bit #
MP 13030   : %00
             ;--------------------------------------------------------------
             ;MP13120 Speed 0 permitted
             ;Input:  0: S = 0 permitted
             ;        1: S = 0 not permitted
MP 13120   : 1
             ;--------------------------------------------------------------
             ;MP13130 Polarity of the S analog voltage
             ;Input: 0 = M03 positive voltage; M04 negative voltage
             ;       1 = M03 negative voltage; M04 positive voltage
             ;       2 = M03 and M04 positive voltage
             ;       3 = M03 and M04 negative voltage
MP 13130   : 1
             ;--------------------------------------------------------------
             ;MP13140 Counting direction of the encoder signals for the
             ;        spindle
             ;Input: 0 = Positive counting direction with M03
             ;       1 = Negative counting direction with M03
MP 13140   : 1
             ;--------------------------------------------------------------
             ;MP13142 Line count of the rotary encoder on the spindle
             ;Input: 100 to 100 000 [lines]
MP 13142   : 2048
             ;--------------------------------------------------------------
             ;MP13143 Mounting configuration of the spindle position encoder
             ;Input: 0 = Direct
             ;       1 = Transmission (reference pulse from input X30)
             ;       2 = Transmission (reference end position of input X30
             ;           and reference pulse from encoder, not possible if
             ;           transmission is located between ROD and spindle)
             ;       3 = Transmission (2nd reference pulse from input X30)
             ;       4 = Referencing via EnDat (not possible if
             ;           transmission is located between ROD and spindle)
             ;       5 = like input = 0, but evaluation of transmission
MP 13143   : 0
             ;--------------------------------------------------------------
             ;MP13210 S digital: Motor revolutions at rated speed
             ;Input: 0 to 100 [1000/min]
             ;
             ;       S analog : S analog voltage at rated speed
             ;Input: 0 to 9.999 [V]
MP 13210.0 : 1.5        ;1st gear range
MP 13210.1 : 1.5        ;2nd gear range
MP 13210.2 : 0          ;3rd gear range
MP 13210.3 : 0          ;4th gear range
MP 13210.4 : 0          ;5th gear range
MP 13210.5 : 0          ;6th gear range
MP 13210.6 : 0          ;7th gear range
MP 13210.7 : 0          ;8th gear range
             ;--------------------------------------------------------------
             ;MP13240.1 S digital: Minimum possible output speed of motor
             ;Input:    0 to 9.999 [1000/min]
             ;
             ;          S analog : Minimum possible S analog output voltage
             ;          Input: 0 to 9.999 [V]
MP 13240.1 : 0.05
             ;--------------------------------------------------------------
             ;MP13240.2 S digital: Motor speed for gear changing
             ;Input:    0 to 9.999 [1000/min]
             ;
             ;          S analog : Jog voltage for gear changing
             ;          Input: 0 to 9.999 [V]
MP 13240.2 : 0.1
             ;--------------------------------------------------------------
             ;MP13310 S override limitation
             ;Input: 0 to 100 [%]
MP 13310.0 : 150        ;Maximum
MP 13310.1 : 0          ;Minimum
             ;--------------------------------------------------------------
             ;MP13350 Maximum permissible overshoot of the spindle rpm for
             ;        monitoring the spindle rpm, as a percentage of the
             ;        nominal speed value
             ;Input: 0 to 100 [%]
MP 13350   : 0
             ;--------------------------------------------------------------
             ;MP13351 Entry of an absolute value for the permissible
             ;        overshoot of the rpm. This value is only used if the
             ;        absolute value resulting from MP13350 is less than the
             ;        the value entered for MP13351.
             ;        (for low nominal speeds)
             ;Input: 0,001 to 100000,000 [rpm]
             ;       0 = Monitoring deactivated
MP 13351   : 0
             ;--------------------------------------------------------------
             ;MP13411 Ramp gradient of spindle
             ;Input: S digital: 0 to 1.999 [(1000/min)/ms]
             ;       S analog : 0 to 1.999 [V/ms]
MP 13411.0 : 0.01
MP 13411.1 : 0.01
MP 13411.2 : 0.01
MP 13411.3 : 0.01
MP 13411.4 : 0.01
MP 13411.5 : 0.01
MP 13411.6 : 0.01
MP 13411.7 : 0.01
             ;--------------------------------------------------------------
             ;MP13412 Multiplier for MP13411
             ;Input: 0 to 1.999
MP 13412.0 : 1          ;With M03, M04, M05
MP 13412.1 : 1          ;For oriented spindle stop
MP 13412.2 : 1          ;For tapping
MP 13412.3 : 1          ;For rigid tapping
             ;--------------------------------------------------------------
             ;MP13415 Spindle overshoot behavior
             ;Input: 0 to 1000 [ms]
MP 13415.0 : 100        ;With M03, M04, M05
MP 13415.1 : 100        ;For oriented spindle stop
MP 13415.2 : 100        ;For tapping
MP 13415.3 : 100        ;For rigid tapping
             ;--------------------------------------------------------------
             ;MP13420 Positioning window for the spindle
             ;Input: 0 to 360 [degrees]
MP 13420   : 0.5
             ;--------------------------------------------------------------
             ;MP13430 Deviation of the reference mark from the desired
             ;position (spindle preset)
             ;Input: 0 to 360 [degrees]
MP 13430   : 0
             ;--------------------------------------------------------------
             ;MP13440 kv factor for spindle orientation
             ;Input: 0.1 to 10 [1000/min]
MP 13440.0 : 1          ;1st gear range
MP 13440.1 : 1          ;2nd gear range
MP 13440.2 : 1          ;3rd gear range
MP 13440.3 : 1          ;4th gear range
MP 13440.4 : 1          ;5th gear range
MP 13440.5 : 1          ;6th gear range
MP 13440.6 : 1          ;7th gear range
MP 13440.7 : 1          ;8th gear range
             ;--------------------------------------------------------------
             ;MP13450 Spindle motor revolutions
             ;Input: 0 to 65535 [rpm]
             ;       0 no transmission
MP 13450.0 : 0          ;1st gear range
MP 13450.1 : 0          ;2nd gear range
MP 13450.2 : 0          ;3rd gear range
MP 13450.3 : 0          ;4th gear range
MP 13450.4 : 0          ;5th gear range
MP 13450.5 : 0          ;6th gear range
MP 13450.6 : 0          ;7th gear range
MP 13450.7 : 0          ;8th gear range
             ;--------------------------------------------------------------
             ;MP13451 Spindle revolutions
             ;Input: 0 to 65535 [rpm]
             ;       0 no transmission
MP 13451.0 : 0          ;1st gear range
MP 13451.1 : 0          ;2nd gear range
MP 13451.2 : 0          ;3rd gear range
MP 13451.3 : 0          ;4th gear range
MP 13451.4 : 0          ;5th gear range
MP 13451.5 : 0          ;6th gear range
MP 13451.6 : 0          ;7th gear range
MP 13451.7 : 0          ;8th gear range
             ;--------------------------------------------------------------
             ;MP13510 Rated speed for gear ranges
             ;Input: 0 to 99 999.999 [rpm]
MP 13510.0 : 1500       ;1st gear range
MP 13510.1 : 15000      ;2nd gear range
MP 13510.2 : 0          ;3rd gear range
MP 13510.3 : 0          ;4th gear range
MP 13510.4 : 0          ;5th gear range
MP 13510.5 : 0          ;6th gear range
MP 13510.6 : 0          ;7th gear range
MP 13510.7 : 0          ;8th gear range
             ;--------------------------------------------------------------
             ;MP13515 Maximum spindle speed
             ;Input: 0 to 99 999.999 [rpm]
MP 13515.0 : 1500       ;1st gear range
MP 13515.1 : 15000      ;2nd gear range
MP 13515.2 : 0          ;3rd gear range
MP 13515.3 : 0          ;4th gear range
MP 13515.4 : 0          ;5th gear range
MP 13515.5 : 0          ;6th gear range
MP 13515.6 : 0          ;7th gear range
MP 13515.7 : 0          ;8th gear range
             ;--------------------------------------------------------------
             ;MP13520.0 Speed activation through marker M4011
             ;Input: 0 to 99 999.999 [rpm]
MP 13520.0 : 10
             ;--------------------------------------------------------------
             ;MP13520.1 Spindle speed for oriented stop
             ;Input: 0 to 99 999.999 [rpm]
MP 13520.1 : 500
             ;--------------------------------------------------------------
             ;MP13530 Increased spindle power for roughing
             ;Input: 0 = Inactive
             ;       1 = Increased spindle power for roughing
MP 13530   : 0
             ;--------------------------------------------------------------
             ;MP13540 Allowed undershooting of the actual value of the
             ;       spindle speed. Defined via an percentage-factor of the
             ;       nominal value of the spindel speed.
             ;Input: 0 to 100 [%]
             ;       0 = Monitoring inactive
MP 13540   : 0
             ;--------------------------------------------------------------
             ;MP13542 Minimum value of the spindle speed at which the
             ;       monitoring from MP3540 becomes active.
             ;Input: 0,001 to 99 999.999 [rpm]
             ;       0 = Monitoring inactive
MP 13542   : 0
             ;--------------------------------------------------------------
             ;MP13550 Delay of Emergency-Stop reaction of the spindle
             ;Input: 0.001 to 0.100 [s]
             ;       0 = Delay inactive
MP 13550   : 0
