             ;Machinenparameter iTNC530 MC 422 D/MC 6xxx
             ;CC 422/CC 424(B)/CC 61xx
             ;Id Nr: 606 420-04, 606 422-04, 606 424-04, 606 425-04
             ;
             ;Erweiterte Schnittstelle 22 axes / 24 controled loops
             ;
             ;Date of information: 2015-08-06
             ;
             ;==================================================================
             ;Code numbers:
             ;==================================================================
             ; Edit machine parameters  ...................... 95148
             ; Short form of machine parameters .............. 123
             ; Offset compensation ........................... 75368
             ; Unprotect PGM ................................. 86357
             ; Digital scope 6 channels ...................... 688379
             ; Reset PLC counter ............................. 857282
             ; Monitor network ............................... NETMONI
             ; Set up network ................................ NET123
             ; DSP monitor ................................... DSP123
             ; Check log book (LSV-2 off/on) .. .............. LOGBOOK
             ; Select machine kinematic ...................... KINEMATIC
             ; Execute NC Software Update .................... SETUP
             ; Systeminformations TNC:\version.a ............ VERSION
             ; Display active options ........................ SIK
             ;
             ;==================================================================
             ; Special keys and functions
             ;==================================================================
             ; Entering special characters via the TNC keyboard:
             ; <Form Feed>   = <Shift> + <Ret>
             ; <|>           = <Shift> + <!>
             ; <~>           = <Shift> + <^>
             ; <Esc>         = <Shift> + <#>
             ;
             ; Switch between CAPITAL and lowercase:
             ; <Shift> + <Space>
             ;
             ;==================================================================
             ;Contents
             ;==================================================================
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
             ;==================================================================
             ;Input format
             ;==================================================================
             ;Leading character % = binary input
             ;    e.g.:  MP 10 : % 000111111
             ;                 :   987654321 axis
             ;Leading character $ = hexadecimal input
             ;    e.g.:  MP 4310.0  : $800F
             ;                         1000 0000 0000 1111
             ;                        -32753
             ;==================================================================
             ;MP 10 .. 999
             ;Measuring systems and machine axes
             ;==================================================================
             ;MP10 Active axes
             ;Input: %987654321 bit-encoded
             ;2109876543210987654ZYX
MP 10      : %0010000000000011111111
             ;------------------------------------------------------------------
             ;MP12 Axes in demo mode
             ;Input: %987654321 bit-encoded
             ;       0 = demo mode inactive
             ;       1 = demo mode active
             ;2109876543210987654ZYX
MP 12      : %0000000000000000000000
             ;------------------------------------------------------------------
             ;MP20 Check the measuring system signals for the axes
             ;Input: %987654321 bit-encoded
             ;2109876543210987654ZYX
MP 20.0    : %1111111111111111111111;Absolute pos of distance-coded ref mark
MP 20.1    : %1111111111111111111111;Amplitude
MP 20.2    : %1111111111111111111111;Edge separation
             ;------------------------------------------------------------------
             ;MP21 Check the measuring system signals for the spindle
             ;Input: %yx bit-encoded
             ;       x = 1st spindle
             ;       y = 2nd spindle
MP 21.0    : %11        ;Absolute position of distance-coded ref mark
MP 21.1    : %11        ;Amplitude
MP 21.2    : %11        ;Edge separation
             ;------------------------------------------------------------------
             ;MP100 Assignment of axis designation
             ;Input: WvUcBaZyX
             ; X = NC axes (capital letters)
             ; x = PLC axes (lowercase letters)
             ; - = Non-displayed axes
             ;109876543210987654ZYX
MP 100.0   : --C-----------a--BAZYX;Traverse range 1
MP 100.1   : --C-----------a--BAZYX;Traverse range 2
MP 100.2   : --C-----------a--BAZYX;Traverse range 3
             ;------------------------------------------------------------------
             ;MP108 Assignment of axis to controller basic-board
             ;      (only CC 6xxx)
             ;Input: 0..4 = Number of controller basic-board  in HSCI system
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
MP 108.14  : 0
MP 108.15  : 0
MP 108.16  : 0
MP 108.17  : 0
MP 108.18  : 0
MP 108.19  : 0
MP 108.20  : 0
MP 108.21  : 0
             ;------------------------------------------------------------------
             ;MP109 Assignment of spindles to controller basic-board
             ;        (only CC 6xxx)
             ;Input: 0..4 = Number of controller basic-board in HSCI system
             ;              Only order of controller basic-boards in the
             ;              HSCI system affect on address 0..4
             ;              1. controller basic-board gets adr. 0
MP 109.0   : 0
MP 109.1   : 0
             ;------------------------------------------------------------------
             ;MP110 Assign position encoder inputs to axes
             ;Input:   0        = No position encoder input
             ;         1..6     = Position encoder input X1..X6
             ;         35..38   = Position encoder input X35..X38
             ;         201..214 = Position encoder input X201..X214
MP 110.0   : 1
MP 110.1   : 2
MP 110.2   : 3
MP 110.3   : 4
MP 110.4   : 5
MP 110.5   : 35
MP 110.6   : 36
MP 110.7   : 37
MP 110.8   : 0
MP 110.9   : 0
MP 110.10  : 0
MP 110.11  : 0
MP 110.12  : 0
MP 110.13  : 0
MP 110.14  : 0
MP 110.15  : 0
MP 110.16  : 0
MP 110.17  : 0
MP 110.18  : 0
MP 110.19  : 0
MP 110.20  : 0
MP 110.21  : 0
             ;------------------------------------------------------------------
             ;MP111 Assign position encoder inputs for spindles
             ;Input:   0        = No position encoder input
             ;         1..6     = Position encoder input X1..X6
             ;         35..38   = Position encoder input X35..X38
             ;         201..214 = Position encoder input X201..X214
MP 111.0   : 6          ;1st spindle
MP 111.1   : 38         ;2nd spindle
             ;------------------------------------------------------------------
             ;MP112 Assign speed encoder inputs to axes
             ;      (only CC 422)
             ;Input:   0      = No speed encoder input
             ;         15..20 = Speed encoder input X15..X20
             ;         80..85 = Speed encoder input X80..X85
MP 112.0   : 0
MP 112.1   : 0
MP 112.2   : 0
MP 112.3   : 0
MP 112.4   : 0
MP 112.5   : 0
MP 112.6   : 0
MP 112.7   : 0
MP 112.8   : 0
MP 112.9   : 0
MP 112.10  : 0
MP 112.11  : 0
MP 112.12  : 0
MP 112.13  : 0
MP 112.14  : 0
MP 112.15  : 0
MP 112.16  : 0
MP 112.17  : 0
MP 112.18  : 0
MP 112.19  : 0
MP 112.20  : 0
MP 112.21  : 0
             ;------------------------------------------------------------------
             ;MP113 Assign speed encoder inputs for spindles
             ;Input:   0      = No speed encoder
             ;         15..20 = Speed encoder input X15..X20
             ;         80..85 = Speed encoder input X80..X85
MP 113.0   : 0          ;1st spindle
MP 113.1   : 0          ;2nd spindle
             ;------------------------------------------------------------------
             ;MP115.0 Definition of position encoder inputs 1 Vpp or 11 uA
             ;Input: %X38..X35 X6..X1 bit-encoded
             ;       0 = 1 Vpp
             ;       1 = 11 uA
             ;-8765654321
MP 115.0   : %00000000000
             ;MP115.1 Reserved
             ;-8765654321
MP 115.1   : %00000000000
             ;MP115.2 Low or high input frequency of position encoder inputs
             ;Input: For 1 Vpp: 0 = 27 kHz
             ;                  1 = 400 kHz
             ;       For 11 uA: 0 = 27 kHz
             ;                  1 = 140 kHz
             ;-8765654321
MP 115.2   : %00000000000
             ;------------------------------------------------------------------
             ;MP116.0 Definition of position encoder inputs 1 Vpp or 11 uA
             ;        (only CC 424, CC 61xx)
             ;Input: %X214..X201 bit-encoded
             ;       0 = 1 Vpp
             ;       1 = 11 uA
             ;6543210987654321
MP 116.0   : %0000000000000000
             ;MP116.1 Reserved
             ;6543210987654321
MP 116.1   : %0000000000000000
             ;MP116.2 Low or high input frequency of position encoder inputs
             ;Input: For 1 Vpp: 0 = 27 kHz
             ;                  1 = 400 kHz
             ;       For 11 uA: 0 = 27 kHz
             ;                  1 = 140 kHz
             ;6543210987654321
MP 116.2   : %0000000000000000
             ;------------------------------------------------------------------
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
             ;Bit 6    Check the resolution of the EnDat LS position encoder
             ;         0 = Active
             ;         1 = Do not check the resolution
             ;Bit 7    Dual-head evaluation of two EnDat 2.2 encoders
             ;         for compensating eccentricity errors in rotary axes
             ;         with torque motor
             ;         0 = Not active
             ;         1 = Active
             ;Bit 8    Evaluation of the phase position of the reference pulse
             ;         0 = Active
             ;         1 = Do not evaluate the phase position
             ;876543210
MP 118.0   : %000000000
MP 118.1   : %000000000
MP 118.2   : %000000000
MP 118.3   : %000000000
MP 118.4   : %000000000
MP 118.5   : %000000000
MP 118.6   : %000000000
MP 118.7   : %000000000
MP 118.8   : %000000000
MP 118.9   : %000000000
MP 118.10  : %000000000
MP 118.11  : %000000000
MP 118.12  : %000000000
MP 118.13  : %000000000
MP 118.14  : %000000000
MP 118.15  : %000000000
MP 118.16  : %000000000
MP 118.17  : %000000000
MP 118.18  : %000000000
MP 118.19  : %000000000
MP 118.20  : %000000000
MP 118.21  : %000000000
             ;------------------------------------------------------------------
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
             ;Bit 6    Check the resolution of the EnDat LS position encoder
             ;         0 = Active
             ;         1 = Do not check the resolution
             ;Bit 7    Dual-head evaluation of two EnDat 2.2 encoders
             ;         for compensating eccentricity errors in rotary axes
             ;         with torque motor
             ;         0 = Not active
             ;         1 = Active
             ;Bit 8    Evaluation of the phase position of the reference pulse
             ;         0 = Active
             ;         1 = Do not evaluate the phase position
             ;876543210
MP 119.0   : %000000000
MP 119.1   : %000000000
             ;------------------------------------------------------------------
             ;MP120 Assign the nominal speed value outputs to the axes
             ;Note: Assignment is permanently defined(.0 -> x1)
             ;Input: 0      = Not a servo-controlled axis
             ;       1..6   = Analog outputs X8 1..6
             ;       7..13  = Analog outputs X9 7..13
             ;       1..8   = Analog outputs X66, X67 (CMA)
             ;       51..64 = Digital outputs X51..X64
             ;                for CC with HEIDENHAIN inverters
             ;       80..85 = Digital outputs X80..X85 (for UEC)
MP 120.0   : 1
MP 120.1   : 2
MP 120.2   : 3
MP 120.3   : 4
MP 120.4   : 5
MP 120.5   : 7
MP 120.6   : 8
MP 120.7   : 9
MP 120.8   : 0
MP 120.9   : 0
MP 120.10  : 0
MP 120.11  : 0
MP 120.12  : 0
MP 120.13  : 0
MP 120.14  : 0
MP 120.15  : 0
MP 120.16  : 0
MP 120.17  : 0
MP 120.18  : 0
MP 120.19  : 0
MP 120.20  : 0
MP 120.21  : 0
             ;------------------------------------------------------------------
             ;MP121 Assign the nominal speed value outputs for spindles
             ;Input: 0      = Not a servo-controlled spindle
             ;       1..6   = Analog outputs X8 1..6
             ;       7..13  = Analog outputs X9 7..13
             ;       1..8   = Analog outputs X66, X67 (CMA)
             ;       51..64 = Digital outputs X51..X64
             ;                for CC with HEIDENHAIN inverters
             ;       80..85 = Digital outputs X80..X85 (for UEC)
MP 121.0   : 6          ;1st spindle
MP 121.1   : 0          ;2nd spindle
             ;------------------------------------------------------------------
             ;MP130 y index of the machine parameters MP2xxx.y for the axes
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
MP 130.12  : 12
MP 130.13  : 13
MP 130.14  : 14
MP 130.15  : 15
MP 130.16  : 16
MP 130.17  : 17
MP 130.18  : 18
MP 130.19  : 19
MP 130.20  : 19
MP 130.21  : 19
             ;------------------------------------------------------------------
             ;MP131 Y index of the machine parameters MP2xxx.y for the spindle
             ;      in operating mode 0
             ;Input: 0 to 17
MP 131.0   : 20         ;Spindle 1
MP 131.1   : 22         ;Spindle 2
             ;------------------------------------------------------------------
             ;MP132 Y index of the machine parameters MP2xxx.y for the spindle
             ;      in operating mode 1
             ;Input: 0 to 17
MP 132.0   : 21         ;Spindle 1
MP 132.1   : 23         ;Spindle 2
             ;------------------------------------------------------------------
             ;MP210 Counting direction of position encoder signals
             ;Input: %987654321 bit-encoded
             ;       0 = positive
             ;       1 = negative
             ;2109876543210987654ZYX
MP 210     : %0000000000000000000000
             ;------------------------------------------------------------------
             ;The signal period is automatically calculated by the TNC:
             ;Signal period = MP331 / MP332
             ;MP331 = Distance for the counting pulses from MP332
             ;Input: 0 to 99 999.9999 [mm/degrees]
MP 331.0   : 10.00000
MP 331.1   : 10.00000
MP 331.2   : 10.00000
MP 331.3   : 10.00000
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
MP 331.14  : 10.00000
MP 331.15  : 10.00000
MP 331.16  : 10.00000
MP 331.17  : 10.00000
MP 331.18  : 10.00000
MP 331.19  : 10.00000
MP 331.20  : 10.00000
MP 331.21  : 10.00000
             ;------------------------------------------------------------------
             ;MP332 Number of counting pulses in the distance from MP331
             ;Input: 1 to 16 777 215
MP 332.0   : 2048.00000
MP 332.1   : 2048.00000
MP 332.2   : 2048.00000
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
MP 332.14  : 2048.00000
MP 332.15  : 2048.00000
MP 332.16  : 2048.00000
MP 332.17  : 2048.00000
MP 332.18  : 2048.00000
MP 332.19  : 2048.00000
MP 332.20  : 2048.00000
MP 332.21  : 2048.00000
             ;------------------------------------------------------------------
             ;MP334 Number of grating periods between the zero pulses
             ;of distance-coded encoders
             ;Input: 0 to 65535
             ;       0 = 1000
MP 334.0   : 0
MP 334.1   : 0
MP 334.2   : 0
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
MP 334.14  : 0
MP 334.15  : 0
MP 334.16  : 0
MP 334.17  : 0
MP 334.18  : 0
MP 334.19  : 0
MP 334.20  : 0
MP 334.21  : 0
             ;------------------------------------------------------------------
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
MP 340.14  : 0
MP 340.15  : 0
MP 340.16  : 0
MP 340.17  : 0
MP 340.18  : 0
MP 340.19  : 0
MP 340.20  : 0
MP 340.21  : 0
             ;------------------------------------------------------------------
             ;MP410 Axis identification for axis keys IV, V and VI
             ;      (VI only with HR 5xx)
             ;Input: A/B/C/U/V/W/T mit SW-Option #44
MP 410.3   : B          ;IV
MP 410.4   : C          ;V
MP 410.5   : T          ;VI
             ;------------------------------------------------------------------
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
MP 420.7   : 1
MP 420.8   : 0
MP 420.9   : 0
MP 420.10  : 0
MP 420.11  : 0
MP 420.12  : 0
MP 420.13  : 0
MP 420.14  : 0
MP 420.15  : 0
MP 420.16  : 0
MP 420.17  : 0
MP 420.18  : 0
MP 420.19  : 0
MP 420.20  : 0
MP 420.21  : 0
             ;------------------------------------------------------------------
             ;MP430 Default increment for Hirth coupling
             ;Input: 0.0000 to 180.0000 [degrees]
MP 430.0   : 0
MP 430.1   : 0
MP 430.2   : 0
MP 430.3   : 0
MP 430.4   : 0
MP 430.5   : 0
MP 430.6   : 0
MP 430.7   : 10
MP 430.8   : 0
MP 430.9   : 0
MP 430.10  : 0
MP 430.11  : 0
MP 430.12  : 0
MP 430.13  : 0
MP 430.14  : 0
MP 430.15  : 0
MP 430.16  : 0
MP 430.17  : 0
MP 430.18  : 0
MP 430.19  : 0
MP 430.20  : 0
MP 430.21  : 0
             ;==================================================================
             ;Functional safety FS on HSCI system
             ;------------------------------------------------------------------
             ;SMP511 Time until the safety self-test.
             ;       This must occur within 72 h.
             ;Input: 1 to 4320 [min]
MP 511     : 480
             ;------------------------------------------------------------------
             ;SMP513 Unique designation of the data record
             ;Input: 0 to 99, automatically set bei SKERN
MP 513.0   : 0
MP 513.1   : 0
MP 513.2   : 0
MP 513.3   : 0
MP 513.4   : 0
MP 513.5   : 0
MP 513.6   : 0
MP 513.7   : 0
MP 513.8   : 0
MP 513.9   : 0
MP 513.10  : 0
MP 513.11  : 0
MP 513.12  : 0
MP 513.13  : 0
MP 513.14  : 0
MP 513.15  : 0
MP 513.16  : 0
MP 513.17  : 0
MP 513.18  : 0
MP 513.19  : 0
MP 513.20  : 0
MP 513.21  : 0
MP 513.22  : 0
MP 513.23  : 0
MP 513.24  : 0
MP 513.25  : 0
MP 513.26  : 0
MP 513.27  : 0
MP 513.28  : 0
MP 513.29  : 0
MP 513.30  : 0
MP 513.31  : 0
MP 513.32  : 0
MP 513.33  : 0
MP 513.34  : 0
MP 513.35  : 0
MP 513.36  : 0
MP 513.37  : 0
MP 513.38  : 0
MP 513.39  : 0
MP 513.40  : 0
MP 513.41  : 0
MP 513.42  : 0
MP 513.43  : 0
MP 513.44  : 0
MP 513.45  : 0
MP 513.46  : 0
MP 513.47  : 0
MP 513.48  : 0
MP 513.49  : 0
             ;------------------------------------------------------------------
             ;SMP514 CRC checksum of the data record
             ;Input: automatically set bei SKERN
MP 514.0   : $00000000
MP 514.1   : $00000000
MP 514.2   : $00000000
MP 514.3   : $00000000
MP 514.4   : $00000000
MP 514.5   : $00000000
MP 514.6   : $00000000
MP 514.7   : $00000000
MP 514.8   : $00000000
MP 514.9   : $00000000
MP 514.10  : $00000000
MP 514.11  : $00000000
MP 514.12  : $00000000
MP 514.13  : $00000000
MP 514.14  : $00000000
MP 514.15  : $00000000
MP 514.16  : $00000000
MP 514.17  : $00000000
MP 514.18  : $00000000
MP 514.19  : $00000000
MP 514.20  : $00000000
MP 514.21  : $00000000
MP 514.22  : $00000000
MP 514.23  : $00000000
MP 514.24  : $00000000
MP 514.25  : $00000000
MP 514.26  : $00000000
MP 514.27  : $00000000
MP 514.28  : $00000000
MP 514.29  : $00000000
MP 514.30  : $00000000
MP 514.31  : $00000000
MP 514.32  : $00000000
MP 514.33  : $00000000
MP 514.34  : $00000000
MP 514.35  : $00000000
MP 514.36  : $00000000
MP 514.37  : $00000000
MP 514.38  : $00000000
MP 514.39  : $00000000
MP 514.40  : $00000000
MP 514.41  : $00000000
MP 514.42  : $00000000
MP 514.43  : $00000000
MP 514.44  : $00000000
MP 514.45  : $00000000
MP 514.46  : $00000000
MP 514.47  : $00000000
MP 514.48  : $00000000
MP 514.49  : $00000000
             ;------------------------------------------------------------------
             ;SMP515 Information if the data record is checked and accepted
             ;Input: 0 or 1, automatically set bei SKERN
MP 515.0   : 0
MP 515.1   : 0
MP 515.2   : 0
MP 515.3   : 0
MP 515.4   : 0
MP 515.5   : 0
MP 515.6   : 0
MP 515.7   : 0
MP 515.8   : 0
MP 515.9   : 0
MP 515.10  : 0
MP 515.11  : 0
MP 515.12  : 0
MP 515.13  : 0
MP 515.14  : 0
MP 515.15  : 0
MP 515.16  : 0
MP 515.17  : 0
MP 515.18  : 0
MP 515.19  : 0
MP 515.20  : 0
MP 515.21  : 0
MP 515.22  : 0
MP 515.23  : 0
MP 515.24  : 0
MP 515.25  : 0
MP 515.26  : 0
MP 515.27  : 0
MP 515.28  : 0
MP 515.29  : 0
MP 515.30  : 0
MP 515.31  : 0
MP 515.32  : 0
MP 515.33  : 0
MP 515.34  : 0
MP 515.35  : 0
MP 515.36  : 0
MP 515.37  : 0
MP 515.38  : 0
MP 515.39  : 0
MP 515.40  : 0
MP 515.41  : 0
MP 515.42  : 0
MP 515.43  : 0
MP 515.44  : 0
MP 515.45  : 0
MP 515.46  : 0
MP 515.47  : 0
MP 515.48  : 0
MP 515.49  : 0
             ;------------------------------------------------------------------
             ;SMP516 Name of the machine. Designation that unambiguously
             ;       identifies the machine series or the machine itself.
             ;Input: automatically set bei SKERN
MP 516.0   : 0
MP 516.1   : 0
MP 516.2   : 0
MP 516.3   : 0
MP 516.4   : 0
MP 516.5   : 0
MP 516.6   : 0
MP 516.7   : 0
MP 516.8   : 0
MP 516.9   : 0
MP 516.10  : 0
MP 516.11  : 0
MP 516.12  : 0
MP 516.13  : 0
MP 516.14  : 0
MP 516.15  : 0
MP 516.16  : 0
MP 516.17  : 0
MP 516.18  : 0
MP 516.19  : 0
MP 516.20  : 0
MP 516.21  : 0
MP 516.22  : 0
MP 516.23  : 0
MP 516.24  : 0
MP 516.25  : 0
MP 516.26  : 0
MP 516.27  : 0
MP 516.28  : 0
MP 516.29  : 0
MP 516.30  : 0
MP 516.31  : 0
MP 516.32  : 0
MP 516.33  : 0
MP 516.34  : 0
MP 516.35  : 0
MP 516.36  : 0
MP 516.37  : 0
MP 516.38  : 0
MP 516.39  : 0
MP 516.40  : 0
MP 516.41  : 0
MP 516.42  : 0
MP 516.43  : 0
MP 516.44  : 0
MP 516.45  : 0
MP 516.46  : 0
MP 516.47  : 0
MP 516.48  : 0
MP 516.49  : 0
             ;------------------------------------------------------------------
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
MP 525.14  : 1
MP 525.15  : 1
MP 525.16  : 1
MP 525.17  : 1
MP 525.18  : 1
MP 525.19  : 1
MP 525.20  : 1
MP 525.21  : 1
             ;------------------------------------------------------------------
             ;SMP526 Spindles:
             ;       Default time for bringing spindles to a standstill
             ;       on an axis-specific basis on the emergency braking
             ;       ramp for SS1 reaction
             ;Input: 0.000 to 10.000 [s]
MP 526.0   : 1
MP 526.1   : 1
             ;------------------------------------------------------------------
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
MP 527.14  : 1
MP 527.15  : 1
MP 527.16  : 1
MP 527.17  : 1
MP 527.18  : 1
MP 527.19  : 1
MP 527.20  : 1
MP 527.21  : 1
             ;------------------------------------------------------------------
             ;SMP528 Spindles:
             ;       Default time for bringing spindles to a controlled
             ;       standstill on an axis-specific basis for SS2 reaction
             ;Input: 0.000 to 10.000 [s]
MP 528.0   : 1
MP 528.1   : 1
             ;------------------------------------------------------------------
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
MP 530.14  : 0.03
MP 530.15  : 0.03
MP 530.16  : 0.03
MP 530.17  : 0.03
MP 530.18  : 0.03
MP 530.19  : 0.03
MP 530.20  : 0.03
MP 530.21  : 0.03
             ;------------------------------------------------------------------
             ;SMP535 Run-times of the timers (max. 16) for SPLC program
             ;Input: 0.0 to 1000000.0 [s]
MP 535.0   : 0          ;SPLC timer T00 / T48
MP 535.1   : 0          ;SPLC timer T01 / T49
MP 535.2   : 0          ;SPLC timer T02 / T50
MP 535.3   : 0          ;SPLC timer T03 / T51
MP 535.4   : 0          ;SPLC timer T04 / T52
MP 535.5   : 0          ;SPLC timer T05 / T53
MP 535.6   : 0          ;SPLC timer T06 / T54
MP 535.7   : 0          ;SPLC timer T07 / T55
MP 535.8   : 0          ;SPLC timer T08 / T56
MP 535.9   : 0          ;SPLC timer T09 / T57
MP 535.10  : 0          ;SPLC timer T10 / T58
MP 535.11  : 0          ;SPLC timer T11 / T59
MP 535.12  : 0          ;SPLC timer T12 / T60
MP 535.13  : 0          ;SPLC timer T13 / T61
MP 535.14  : 0          ;SPLC timer T14 / T62
MP 535.15  : 0          ;SPLC timer T15 / T63
             ;------------------------------------------------------------------
             ;SMP540 Axes:
             ;       Limit value for "Safely limited speed - SLS" in
             ;       operating mode 3 (SOM 3)
             ;Input: 0 to 15000 [mm/min] or [°/min]
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
MP 540.14  : 2000
MP 540.15  : 2000
MP 540.16  : 2000
MP 540.17  : 2000
MP 540.18  : 2000
MP 540.19  : 2000
MP 540.20  : 2000
MP 540.21  : 2000
             ;------------------------------------------------------------------
             ;SMP541 Spindle:
             ;       Limit value for "Safely limited speed - SLS" in
             ;       operating mode 3 (SOM 3)
             ;Input: 0 to 16000 [rpm]
MP 541.0   : 500
MP 541.1   : 500
             ;------------------------------------------------------------------
             ;SMP545 Axes:
             ;       Limit value for standstill monitoring in SOS state
             ;Input:  0.001 to 30.000 [mm] or [°]
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
MP 545.14  : 0.5
MP 545.15  : 0.5
MP 545.16  : 0.5
MP 545.17  : 0.5
MP 545.18  : 0.5
MP 545.19  : 0.5
MP 545.20  : 0.5
MP 545.21  : 0.5
             ;------------------------------------------------------------------
             ;SMP547 Axis-specific limit value for safely limited increment
             ;Input:  0.001 to 10.000 [mm] or [°]
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
MP 547.14  : 2
MP 547.15  : 2
MP 547.16  : 2
MP 547.17  : 2
MP 547.18  : 2
MP 547.19  : 2
MP 547.20  : 2
MP 547.21  : 2
             ;------------------------------------------------------------------
             ;SMP549 Axis-specific configuration, if axis group is in
             ;       SOS or STO after SS2 reaction
             ;Input: 0 = Spindle axis groups in STO upon SS2, SS1D
             ;           axis axis groups in SOS upon SS2, SS1D
             ;       1 = Axis group in STO upon SS2, SS1D
             ;       2 = Axis group in SOS upon SS2, SS1D
MP 549.0   : 0
MP 549.1   : 0
MP 549.2   : 0
MP 549.3   : 0
MP 549.4   : 0
MP 549.5   : 0
MP 549.6   : 0
MP 549.7   : 0
             ;------------------------------------------------------------------
             ;SMP550 Axes (only with HSCI system):
             ;       Axis-specific limit value for maximum permissible
             ;       path upon SS2 reaction
             ;Input: 0.0010 to 30.0000 [mm] or [°]
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
MP 550.14  : 0.5
MP 550.15  : 0.5
MP 550.16  : 0.5
MP 550.17  : 0.5
MP 550.18  : 0.5
MP 550.19  : 0.5
MP 550.20  : 0.5
MP 550.21  : 0.5
             ;------------------------------------------------------------------
             ;SMP551 Spindle:
             ;       Limit value for "Safely limited speed - SLS" in
             ;       operating mode 4 (SOM 4)
             ;       MP560 bit 0 = 1 and code number
             ;Input: 0 to 16000 [rpm]
MP 551.0   : 500
MP 551.1   : 500
             ;------------------------------------------------------------------
             ;SMP552 Axes:
             ;       Limit value for "Safely limited speed - SLS" in
             ;       operating mode 4 (SOM 4)
             ;       MP560 bit 0 = 1 and code number
             ;Input: 0 to 5000 [mm/min] or [°/min]
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
MP 552.14  : 0
MP 552.15  : 0
MP 552.16  : 0
MP 552.17  : 0
MP 552.18  : 0
MP 552.19  : 0
MP 552.20  : 0
MP 552.21  : 0
             ;------------------------------------------------------------------
             ;SMP555 Display mode for rotary axes and PLC auxiliary axes
             ;Input: 0.0000 to 99.999 9999 [°]
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
MP 555.14  : 0
MP 555.15  : 0
MP 555.16  : 0
MP 555.17  : 0
MP 555.18  : 0
MP 555.19  : 0
MP 555.20  : 0
MP 555.21  : 0
             ;------------------------------------------------------------------
             ;SMP556 Spindle:
             ;       Limit value for monitoring the spindle totation
             ;       during SS2 in operating mode SOM_2
             ;Input: 1 to 100 [revolutions]
MP 556.0   : 2
MP 556.1   : 2
             ;------------------------------------------------------------------
             ;SMP557 Spindle:
             ;       Limit value for monitoring the spindle totation
             ;       during SS2 in operating mode SOM_3
             ;Input: 1 to 100 [revolutions]
MP 557.0   : 5
MP 557.1   : 5
             ;------------------------------------------------------------------
             ;SMP558 Spindle:
             ;       Limit value for monitoring the spindle totation
             ;       during SS2 in operating mode SOM_4
             ;Input: 1 to 100 [revolutions]
MP 558.0   : 5
MP 558.1   : 5
             ;------------------------------------------------------------------
             ;SMP560 Additional functions
             ;Input: %xxxxxxxxx
             ;Bit0     Enabling of operating mode 4 (SOM 4)
             ;         (Permissive button only for spindle or NC start
             ;         and separately selectable spindle speed and
             ;         axis feed rates)
             ;         0 = SOM 4 not allowed
             ;         1 = SOM 4 allowed
             ;Bit1     Protective doors during safety self-test
             ;         0 = Protective doors need to be closed
             ;         1 = Protective doors do not need to be closed
             ;Bit2     reserved (only MC 422)
             ;Bit3     Test of brake activation
             ;         0 = Active
             ;         1 = Not active
             ;Bit4     reserved (only MC 422)
             ;Bit5     reserved (only MC 422)
             ;Bit6     Test of switching off specific axes via PWM
             ;         output STO.B.x.
             ;         0 = Perform test via STO.B.x for specific axes
             ;         1 = Switch off and test all axes together
             ;Bit7     Permissive button of handwheel in SOM_4
             ;         0 = Permissive button has to be pressed to
             ;             maintain movements
             ;         1 = Permissive button can be released after
             ;             starting a movement
             ;Bit8     Wiring of -STO.A.G and -STOS.A.G to safety
             ;         relays in power modules
             ;         0 = Wiring exists
             ;         1 = Wiring does not exist
             ;Bit9     Compensation movements for handwheel in SOM_3, SOM_4
             ;         0 = not allowed
             ;         1 = In the El. Handwheel mode of operation the
             ;             simultaneous movement of several axes is
             ;             permitted, e.g. for compensation movements
             ;Bit10    reserved
             ;Bit11    dv/dt monitoring of the spindle at SS1 reaction
             ;         0 = active
             ;         1 = not active
             ;Bit12    Test of whether the safe outputs can be cut off
             ;         0 = active
             ;         1 = not active
             ;Bit13    SS2 on switchover of the control operation mode
             ;         0 = automatic SS2 active
             ;         1 = no automatic SS2
             ;Bit14    Switch-on of drives even after the time of the
             ;         self-test has elapsed
             ;         0 = not possible
             ;         1 = drives can be switched on even if the time
             ;             of the self-test (SMP511) has elapsed
             ;Bit15    Protection against unexpected start in SOM_4
             ;         0 = active
             ;         1 = not active
             ;5432109876543210
MP 560     : %0000000000000000
             ;------------------------------------------------------------------
             ;SMP584 Time monitoring for cross comparison of
             ;       safety inputs
             ;Input: 0.5 to 30.0 [seconds]
MP 584     : 5
             ;------------------------------------------------------------------
             ;SMP585 Inverted PLC inputs of channel A. Maximum 8.
             ;Input: symbolic name of PLC input
             ;       -1 = no inverted PLC inputs
MP 585.0   : 
MP 585.1   : 
MP 585.2   : 
MP 585.3   : 
MP 585.4   : 
MP 585.5   : 
MP 585.6   : 
MP 585.7   : 
             ;------------------------------------------------------------------
             ;SMP586 Inverted PLC inputs of channel B. Maximum 8.
             ;Input: symbolic name of PLC input
             ;       -1 = no inverted PLC inputs
MP 586.0   : 
MP 586.1   : 
MP 586.2   : 
MP 586.3   : 
MP 586.4   : 
MP 586.5   : 
MP 586.6   : 
MP 586.7   : 
             ;------------------------------------------------------------------
             ;SMP587 PLC inputs tested in the cyclic minute test.
             ;       Power must be supplied via -TEST.A and -TEST.B.
             ;       Maximum 16.
             ;Input: symbolic name of PLC input or PLC operand address
MP 587.0   : 
MP 587.1   : 
MP 587.2   : 
MP 587.3   : 
MP 587.4   : 
MP 587.5   : 
MP 587.6   : 
MP 587.7   : 
MP 587.8   : 
MP 587.9   : 
MP 587.10  : 
MP 587.11  : 
MP 587.12  : 
MP 587.13  : 
MP 587.14  : 
MP 587.15  : 
             ;------------------------------------------------------------------
             ;SMP590 Axes:
             ;       Limit value for "Safely limited speed - SLS" in
             ;       operating mode 2 (SOM 2)
             ;Input: 0 to 15000 [mm/min] or [°/min]
MP 590.0   : 200
MP 590.1   : 200
MP 590.2   : 200
MP 590.3   : 200
MP 590.4   : 200
MP 590.5   : 200
MP 590.6   : 200
MP 590.7   : 200
MP 590.8   : 200
MP 590.9   : 200
MP 590.10  : 200
MP 590.11  : 200
MP 590.12  : 200
MP 590.13  : 200
MP 590.14  : 200
MP 590.15  : 200
MP 590.16  : 200
MP 590.17  : 200
MP 590.18  : 200
MP 590.19  : 200
MP 590.20  : 200
MP 590.21  : 200
             ;------------------------------------------------------------------
             ;SMP591 Spindle:
             ;       Limit value for "Safely limited speed - SLS" in
             ;       operating mode 2 (SOM 2)
             ;Input: 0 to 16000 [rpm]
MP 591.0   : 200
MP 591.1   : 200
             ;------------------------------------------------------------------
             ;SMP600 Axes:
             ;       Assignment of axes to axis groups
             ;Input: 0 to 7
             ;       -1 = Not a safe axis
MP 600.0   : -1
MP 600.1   : -1
MP 600.2   : -1
MP 600.3   : -1
MP 600.4   : -1
MP 600.5   : -1
MP 600.6   : -1
MP 600.7   : -1
MP 600.8   : -1
MP 600.9   : -1
MP 600.10  : -1
MP 600.11  : -1
MP 600.12  : -1
MP 600.13  : -1
MP 600.14  : -1
MP 600.15  : -1
MP 600.16  : -1
MP 600.17  : -1
MP 600.18  : -1
MP 600.19  : -1
MP 600.20  : -1
MP 600.21  : -1
             ;------------------------------------------------------------------
             ;SMP601 Spindles:
             ;       Assignment of spindles to axis groups
             ;Input: 0 to 7
             ;       -1 = Not a safe spindle
MP 601.0   : -1
MP 601.1   : -1
             ;------------------------------------------------------------------
             ;SMP610 Definition of braking sequence
             ;       Definition of axis groups that must be at standstill
             ;       before the axis group itself is braked
             ;Input: %xxxxxxxx
             ;       1 = Prior braking of axis group required
             ;76543210
MP 610.0   : %00000000
MP 610.1   : %00000000
MP 610.2   : %00000000
MP 610.3   : %00000000
MP 610.4   : %00000000
MP 610.5   : %00000000
MP 610.6   : %00000000
MP 610.7   : %00000000
             ;------------------------------------------------------------------
             ;SMP620 Time monitoring for brake test
             ;Input: 0 to 60000 [min]
             ;       0 = no time monitoring
MP 620.0   : 480
MP 620.1   : 480
MP 620.2   : 480
MP 620.3   : 480
MP 620.4   : 480
MP 620.5   : 480
MP 620.6   : 480
MP 620.7   : 480
MP 620.8   : 480
MP 620.9   : 480
MP 620.10  : 480
MP 620.11  : 480
MP 620.12  : 480
MP 620.13  : 480
MP 620.14  : 480
MP 620.15  : 480
MP 620.16  : 480
MP 620.17  : 480
MP 620.18  : 480
MP 620.19  : 480
MP 620.20  : 480
MP 620.21  : 480
             ;------------------------------------------------------------------
             ;SMP630 Maximum permissible speed deviation in % for
             ;       actual/nominal monitoring with speed values for axis
             ;Input: 0 to 99 [%] of the current nominal speed
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
MP 630.14  : 1
MP 630.15  : 1
MP 630.16  : 1
MP 630.17  : 1
MP 630.18  : 1
MP 630.19  : 1
MP 630.20  : 1
MP 630.21  : 1
             ;------------------------------------------------------------------
             ;SMP631 Maximum permissible speed deviation in % for
             ;       actual/nominal monitoring with speed values for spindle
             ;Input: 0 to 20 [%] of the current nominal speed
MP 631.0   : 1
MP 631.1   : 1
             ;------------------------------------------------------------------
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
MP 632.14  : 10
MP 632.15  : 10
MP 632.16  : 10
MP 632.17  : 10
MP 632.18  : 10
MP 632.19  : 10
MP 632.20  : 10
MP 632.21  : 10
             ;------------------------------------------------------------------
             ;SMP633 Delay time for
             ;       actual/nominal monitoring with speed values for spindle
             ;Input: 0 to 60 [s] of the current nominal speed
MP 633.0   : 10
MP 633.1   : 10
             ;------------------------------------------------------------------
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
MP 641.14  : 1
MP 641.15  : 1
MP 641.16  : 1
MP 641.17  : 1
MP 641.18  : 1
MP 641.19  : 1
MP 641.20  : 1
MP 641.21  : 1
             ;-------------------------------------------------------------
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
MP 642.14  : 1
MP 642.15  : 1
MP 642.16  : 1
MP 642.17  : 1
MP 642.18  : 1
MP 642.19  : 1
MP 642.20  : 1
MP 642.21  : 1
             ;-------------------------------------------------------------
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
MP 646.14  : +0
MP 646.15  : +0
MP 646.16  : +0
MP 646.17  : +0
MP 646.18  : +0
MP 646.19  : +0
MP 646.20  : +0
MP 646.21  : +0
             ;-------------------------------------------------------------
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
MP 650.14  : +99999
MP 650.15  : +99999
MP 650.16  : +99999
MP 650.17  : +99999
MP 650.18  : +99999
MP 650.19  : +99999
MP 650.20  : +99999
MP 650.21  : +99999
             ;-------------------------------------------------------------
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
MP 670.14  : -99999
MP 670.15  : -99999
MP 670.16  : -99999
MP 670.17  : -99999
MP 670.18  : -99999
MP 670.19  : -99999
MP 670.20  : -99999
MP 670.21  : -99999
             ;-------------------------------------------------------------
             ;SMP691.0 Check sum PLC code SPLC program
MP 691.0   : 0
             ;-------------------------------------------------------------
             ;SMP691.1 MC check sum binary code SPLC program
MP 691.1   : 0
             ;-------------------------------------------------------------
             ;SMP691.2 CC check sum binary code SPLC program
MP 691.2   : 0
             ;-------------------------------------------------------------
             ;SMP692.x Freely definable data for SPLC program
             ;Input: -2147483647 to 2147483647
MP 692.0   : 0
MP 692.1   : 0
MP 692.2   : 0
MP 692.3   : 0
MP 692.4   : 0
MP 692.5   : 0
MP 692.6   : 0
MP 692.7   : 0
MP 692.8   : 0
MP 692.9   : 0
MP 692.10  : 0
MP 692.11  : 0
MP 692.12  : 0
MP 692.13  : 0
MP 692.14  : 0
MP 692.15  : 0
             ;-------------------------------------------------------------
             ;SMP693 Version of the SPLC-API used to create the SPLC program
             ;Input: Numerical value of the API version
             ;         e.g. 100 for version 1.00
MP 693     : 60
             ;-------------------------------------------------------------
             ;SMP698 Commissioning functions for FS
             ;       These Bits must be reset (= 0) before shipping the
             ;       machine
             ;Input: %xx
             ;Bit 0   Maximum speed limits for SLS are ignored
             ;        0 = Speed limits are active
             ;        1 = Speed limits are active are ignored
             ;Bit 1   Activation of the SOS safety function is ignored
             ;        0 = SOS active
             ;        1 = SOS ignored
MP 698     : %00
             ;==================================================================
             ;MP708 Dynamic backlash compensation (not CC422)
             ;Input:  0 = not active
             ;        0,0001 to 1,0000 [mm]
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
MP 708.14  : 0
MP 708.15  : 0
MP 708.16  : 0
MP 708.17  : 0
MP 708.18  : 0
MP 708.19  : 0
MP 708.20  : 0
MP 708.21  : 0
             ;------------------------------------------------------------------
             ;MP709 Filter for backlash compensation
             ;Input:    0 = no filter function for MP710
             ;          +1 bis +1000 [ms] = time constant for MP710
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
MP 709.14  : 0
MP 709.15  : 0
MP 709.16  : 0
MP 709.17  : 0
MP 709.18  : 0
MP 709.19  : 0
MP 709.20  : 0
MP 709.21  : 0
             ;------------------------------------------------------------------
             ;MP710 Backlash compensation (table <-> position encoder)
             ;Input: -9.9999 to +9.9999 [mm] or [°]
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
MP 710.14  : +0
MP 710.15  : +0
MP 710.16  : +0
MP 710.17  : +0
MP 710.18  : +0
MP 710.19  : +0
MP 710.20  : +0
MP 710.21  : +0
             ;------------------------------------------------------------------
             ;MP711 Height of reversal spikes during rotation
             ;for analog axes (MP2610.x - MP2620.x for digital axes)
             ;only with CC 42x, not for CMA with CC 61xx
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
MP 711.14  : +0
MP 711.15  : +0
MP 711.16  : +0
MP 711.17  : +0
MP 711.18  : +0
MP 711.19  : +0
MP 711.20  : +0
MP 711.21  : +0
             ;------------------------------------------------------------------
             ;MP712 Compensation value per control loop cycle time
             ;for analog axes (MP2610.x - MP2620.x for digital axes)
             ;only with CC 42x, not for CMA with CC 61xx
             ;Input: 0.000 000 to 99.999 999 [mm]
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
MP 712.14  : 0
MP 712.15  : 0
MP 712.16  : 0
MP 712.17  : 0
MP 712.18  : 0
MP 712.19  : 0
MP 712.20  : 0
MP 712.21  : 0
             ;------------------------------------------------------------------
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
MP 715.14  : +0
MP 715.15  : +0
MP 715.16  : +0
MP 715.17  : +0
MP 715.18  : +0
MP 715.19  : +0
MP 715.20  : +0
MP 715.21  : +0
             ;------------------------------------------------------------------
             ;MP716 Compensation value per control loop cycle time with M105
             ;Input: 0.000 000 to 99.999 999 [mm]
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
MP 716.14  : 0
MP 716.15  : 0
MP 716.16  : 0
MP 716.17  : 0
MP 716.18  : 0
MP 716.19  : 0
MP 716.20  : 0
MP 716.21  : 0
             ;------------------------------------------------------------------
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
MP 720.14  : +0
MP 720.15  : +0
MP 720.16  : +0
MP 720.17  : +0
MP 720.18  : +0
MP 720.19  : +0
MP 720.20  : +0
MP 720.21  : +0
             ;------------------------------------------------------------------
             ;MP730 Select the axis-error compensation
             ;Input: %..987654321 bit-encoded
             ;       0 =     Linear axis-error compensation
             ;       1 =     Non-linear axis-error compensation
             ;2109876543210987654ZYX
MP 730     : %1111111111111111111111
             ;------------------------------------------------------------------
             ;MP732 Non-linear axis-error compensation for angular axes
             ;      is mapped to range of traverse (if MP730 = 1).
             ;      Instead of 0..360 degree e.g. -90..+90 degree.
             ;Input: %987654321 bit-encoded
             ;       0 =     not active (usual compensation)
             ;       1 =     active (mapping to range of traverse)
             ;2109876543210987654ZYX
MP 732     : %0000000000000000000000
             ;------------------------------------------------------------------
             ;MP736 Maximum limits for compensation values (as of CC 61xx)
             ;Input:  0.0000 to 30.0000 [mm]
             ;      Value for KinematicsComp
MP 736.0   : 20
             ;      Value for TEMPCOMP
MP 736.1   : 20
             ;------------------------------------------------------------------
             ;MP737 Velocity at which compensation values are compensated.
             ;      For TEMPCOMP and KinematicsComp (as of CC 61xx)
             ;Input:  0 to 1000 [mm/min]
MP 737     : 2
             ;------------------------------------------------------------------
             ;MP748 Dynamic reversal error compensation (not CC422)
             ;Input:  0 = not active
             ;        0,0001 to 1,0000 [mm]
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
MP 748.14  : 0
MP 748.15  : 0
MP 748.16  : 0
MP 748.17  : 0
MP 748.18  : 0
MP 748.19  : 0
MP 748.20  : 0
MP 748.21  : 0
             ;------------------------------------------------------------------
             ;MP750 Backlash compensation (distance) (position <-> speedencoder)
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
MP 750.14  : +0
MP 750.15  : +0
MP 750.16  : +0
MP 750.17  : +0
MP 750.18  : +0
MP 750.19  : +0
MP 750.20  : +0
MP 750.21  : +0
             ;------------------------------------------------------------------
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
MP 752.14  : 0
MP 752.15  : 0
MP 752.16  : 0
MP 752.17  : 0
MP 752.18  : 0
MP 752.19  : 0
MP 752.20  : 0
MP 752.21  : 0
             ;------------------------------------------------------------------
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
MP 810.5   : 360
MP 810.6   : 360
MP 810.7   : 400
MP 810.8   : 0
MP 810.9   : 0
MP 810.10  : 0
MP 810.11  : 0
MP 810.12  : 0
MP 810.13  : 0
MP 810.14  : 0
MP 810.15  : 0
MP 810.16  : 0
MP 810.17  : 0
MP 810.18  : 0
MP 810.19  : 360
MP 810.20  : 0
MP 810.21  : 0
             ;------------------------------------------------------------------
             ;MP812 Active software limit switches with modulo display, M94 and
             ;      encoders with EnDat interface
             ;Input: %987654321 bit-encoded
             ;       0 = Software limit switch not active
             ;       1 = Software limit switch active
             ;2109876543210987654ZYX
MP 812     : %0000000000000000000000
             ;------------------------------------------------------------------
             ;MP830 Wiring of the first holding brake of an axis
             ;Input: -1 or -2
             ;        -1 = The brake is controlled through a PLC output.
             ;             PLC-Ausgang. In systems with functional safety
             ;             this means that the brake is controlled exclusively
             ;             through a dual channle FS output without
             ;             participation of the inverter.
             ;        -2 = The brake is controlled through X344 of the
             ;             inverter. In systems with functional safety
             ;             this means that the +24V side of the brake
             ;             is controlled through the inverter, the 0V side
             ;             is controlled through the A channel of an FS
             ;             output.
MP 830.0   : -2
MP 830.1   : -2
MP 830.2   : -2
MP 830.3   : -2
MP 830.4   : -2
MP 830.5   : -2
MP 830.6   : -2
MP 830.7   : -2
MP 830.8   : -2
MP 830.9   : -2
MP 830.10  : -2
MP 830.11  : -2
MP 830.12  : -2
MP 830.13  : -2
MP 830.14  : -2
MP 830.15  : -2
MP 830.16  : -2
MP 830.17  : -2
MP 830.18  : -2
MP 830.19  : -2
MP 830.20  : -2
MP 830.21  : -2
             ;------------------------------------------------------------------
             ;MP831 Wiring of the second holding brake of an axis
             ;Input: -1 or -2
             ;        -1 = The brake is controlled through a PLC output.
             ;             PLC-Ausgang. In systems with functional safety
             ;             this means that the brake is controlled exclusively
             ;             through a dual channle FS output without
             ;             participation of the inverter.
             ;        -2 = The brake is controlled through X344 of the
             ;             inverter. In systems with functional safety
             ;             this means that the +24V side of the brake
             ;             is controlled through the inverter, the 0V side
             ;             is controlled through the A channel of an FS
             ;             output.
MP 831.0   : -2
MP 831.1   : -2
MP 831.2   : -2
MP 831.3   : -2
MP 831.4   : -2
MP 831.5   : -2
MP 831.6   : -2
MP 831.7   : -2
MP 831.8   : -2
MP 831.9   : -2
MP 831.10  : -2
MP 831.11  : -2
MP 831.12  : -2
MP 831.13  : -2
MP 831.14  : -2
MP 831.15  : -2
MP 831.16  : -2
MP 831.17  : -2
MP 831.18  : -2
MP 831.19  : -2
MP 831.20  : -2
MP 831.21  : -2
             ;------------------------------------------------------------------
             ;MP835 Motor brake test of first holding brake of an axis:
             ;      Factor for rated current
             ;Input:   0,1 bis 30,0 [* rated current]
             ;         0 = do not carry out test; or motor without motor brake
MP 835.0   : 0
MP 835.1   : 0
MP 835.2   : 0
MP 835.3   : 0
MP 835.4   : 0
MP 835.5   : 0
MP 835.6   : 0
MP 835.7   : 0
MP 835.8   : 0
MP 835.9   : 0
MP 835.10  : 0
MP 835.11  : 0
MP 835.12  : 0
MP 835.13  : 0
MP 835.14  : 0
MP 835.15  : 0
MP 835.16  : 0
MP 835.17  : 0
MP 835.18  : 0
MP 835.19  : 0
MP 835.20  : 0
MP 835.21  : 0
             ;------------------------------------------------------------------
             ;MP836 Motor brake test of second holding brake of an axis:
             ;      Factor for rated current
             ;Input:   0,1 bis 30,0 [* rated current]
             ;         0 = do not carry out test; or motor without motor brake
MP 836.0   : 0
MP 836.1   : 0
MP 836.2   : 0
MP 836.3   : 0
MP 836.4   : 0
MP 836.5   : 0
MP 836.6   : 0
MP 836.7   : 0
MP 836.8   : 0
MP 836.9   : 0
MP 836.10  : 0
MP 836.11  : 0
MP 836.12  : 0
MP 836.13  : 0
MP 836.14  : 0
MP 836.15  : 0
MP 836.16  : 0
MP 836.17  : 0
MP 836.18  : 0
MP 836.19  : 0
MP 836.20  : 0
MP 836.21  : 0
             ;------------------------------------------------------------------
             ;MP840 Motor brake test of first holding brake of an axis:
             ;      Maximum permissible path
             ;Input: 0 to 10 [mm] or [degrees]
MP 840.0   : 0
MP 840.1   : 0
MP 840.2   : 0
MP 840.3   : 0
MP 840.4   : 0
MP 840.5   : 0
MP 840.6   : 0
MP 840.7   : 0
MP 840.8   : 0
MP 840.9   : 0
MP 840.10  : 0
MP 840.11  : 0
MP 840.12  : 0
MP 840.13  : 0
MP 840.14  : 0
MP 840.15  : 0
MP 840.16  : 0
MP 840.17  : 0
MP 840.18  : 0
MP 840.19  : 0
MP 840.20  : 0
MP 840.21  : 0
             ;------------------------------------------------------------------
             ;MP841 Motor brake test of second holding brake of an axis:
             ;      Maximum permissible path
             ;Input: 0 to 10 [mm] or [degrees]
MP 841.0   : 0
             ;------------------------------------------------------------------
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
MP 850.14  : 0
MP 850.15  : 0
MP 850.16  : 0
MP 850.17  : 0
MP 850.18  : 0
MP 850.19  : 0
MP 850.20  : 0
MP 850.21  : 0
             ;------------------------------------------------------------------
             ;MP855 Synchronization monitoring
             ;Input: 0 to 100.0000 [mm]
             ;       0 = Monitoring not active
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
MP 855.14  : 0
MP 855.15  : 0
MP 855.16  : 0
MP 855.17  : 0
MP 855.18  : 0
MP 855.19  : 0
MP 855.20  : 0
MP 855.21  : 0
             ;------------------------------------------------------------------
             ;MP860 Synchronous control
             ;Input: %xxx
             ;Bit 0 Datum for synchronous control
             ;       0 = Datum at position after switch-on
             ;       1 = Datum at reference marks (machine datum)
             ;Bit 1 Master-slave torque control
             ;       0 = No master-slave torque controlled axis
             ;       1 = Axis is torque slave axis
             ;Bit 2 Brake test for synchronized axes
             ;       0 = Test brakes of synchronized axes simultaneously
             ;       1 = Test brakes of synchronized axes sequentially
             ;           (new behavior, from 340 49x-06 SP02 and
             ;            606 42x-01 SP02)
             ;210
MP 860.0   : 0
MP 860.1   : 0
MP 860.2   : 0
MP 860.3   : 0
MP 860.4   : 0
MP 860.5   : 0
MP 860.6   : 0
MP 860.7   : 0
MP 860.8   : 0
MP 860.9   : 0
MP 860.10  : 0
MP 860.11  : 0
MP 860.12  : 0
MP 860.13  : 0
MP 860.14  : 0
MP 860.15  : 0
MP 860.16  : 0
MP 860.17  : 0
MP 860.18  : 0
MP 860.19  : 0
MP 860.20  : 0
MP 860.21  : 0
             ;------------------------------------------------------------------
             ;MP91x + Ranges of traverse (activation via PLC)
             ;Input: -99 999.9999 to +99 999.9999 [mm] or [degrees]
             ;+Limit switch for 1st range of traverse
MP 910.0   : +1000
MP 910.1   : +1000
MP 910.2   : +1000
MP 910.3   : +99999
MP 910.4   : +99999
MP 910.5   : +99999
MP 910.6   : +99999
MP 910.7   : +99999
MP 910.8   : +0
MP 910.9   : +0
MP 910.10  : +0
MP 910.11  : +0
MP 910.12  : +0
MP 910.13  : +0
MP 910.14  : +0
MP 910.15  : +0
MP 910.16  : +0
MP 910.17  : +0
MP 910.18  : +0
MP 910.19  : +99999
MP 910.20  : +0
MP 910.21  : +0
             ;------------------------------------------------------------------
             ;+Limit switch for 2nd range of traverse
MP 911.0   : +1000
MP 911.1   : +1000
MP 911.2   : +1000
MP 911.3   : +99999
MP 911.4   : +99999
MP 911.5   : +99999
MP 911.6   : +99999
MP 911.7   : +99999
MP 911.8   : +0
MP 911.9   : +0
MP 911.10  : +0
MP 911.11  : +0
MP 911.12  : +0
MP 911.13  : +0
MP 911.14  : +0
MP 911.15  : +0
MP 911.16  : +0
MP 911.17  : +0
MP 911.18  : +0
MP 911.19  : +99999
MP 911.20  : +0
MP 911.21  : +0
             ;------------------------------------------------------------------
             ;+Limit switch for 3rd range of traverse
MP 912.0   : +1000
MP 912.1   : +1000
MP 912.2   : +1000
MP 912.3   : +99999
MP 912.4   : +99999
MP 912.5   : +99999
MP 912.6   : +99999
MP 912.7   : +99999
MP 912.8   : +0
MP 912.9   : +0
MP 912.10  : +0
MP 912.11  : +0
MP 912.12  : +0
MP 912.13  : +0
MP 912.14  : +0
MP 912.15  : +0
MP 912.16  : +0
MP 912.17  : +0
MP 912.18  : +0
MP 912.19  : +99999
MP 912.20  : +0
MP 912.21  : +0
             ;------------------------------------------------------------------
             ;MP92x - Ranges of traverse (activation via PLC)
             ;Input: -99 999.9999 to +99 999.9999 [mm] or [degrees]
             ;-Limit switch for 1st range of traverse
MP 920.0   : -1000
MP 920.1   : -1000
MP 920.2   : -1000
MP 920.3   : -99999
MP 920.4   : -99999
MP 920.5   : -99999
MP 920.6   : -99999
MP 920.7   : -99999
MP 920.8   : +0
MP 920.9   : +0
MP 920.10  : +0
MP 920.11  : +0
MP 920.12  : +0
MP 920.13  : +0
MP 920.14  : +0
MP 920.15  : +0
MP 920.16  : +0
MP 920.17  : +0
MP 920.18  : +0
MP 920.19  : -99999
MP 920.20  : +0
MP 920.21  : +0
             ;------------------------------------------------------------------
             ;-Limit switch for 2nd range of traverse
MP 921.0   : -1000
MP 921.1   : -1000
MP 921.2   : -1000
MP 921.3   : -99999
MP 921.4   : -99999
MP 921.5   : -99999
MP 921.6   : -99999
MP 921.7   : -99999
MP 921.8   : +0
MP 921.9   : +0
MP 921.10  : +0
MP 921.11  : +0
MP 921.12  : +0
MP 921.13  : +0
MP 921.14  : +0
MP 921.15  : +0
MP 921.16  : +0
MP 921.17  : +0
MP 921.18  : +0
MP 921.19  : -99999
MP 921.20  : +0
MP 921.21  : +0
             ;------------------------------------------------------------------
             ;-Limit switch for 3rd range of traverse
MP 922.0   : -1000
MP 922.1   : -1000
MP 922.2   : -1000
MP 922.3   : -99999
MP 922.4   : -99999
MP 922.5   : -99999
MP 922.6   : -99999
MP 922.7   : -99999
MP 922.8   : +0
MP 922.9   : +0
MP 922.10  : +0
MP 922.11  : +0
MP 922.12  : +0
MP 922.13  : +0
MP 922.14  : +0
MP 922.15  : +0
MP 922.16  : +0
MP 922.17  : +0
MP 922.18  : +0
MP 922.19  : -99999
MP 922.20  : +0
MP 922.21  : +0
             ;------------------------------------------------------------------
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
MP 950.14  : +0
MP 950.15  : +0
MP 950.16  : +0
MP 950.17  : +0
MP 950.18  : +0
MP 950.19  : +0
MP 950.20  : +0
MP 950.21  : +0
             ;------------------------------------------------------------------
             ;MP951 Simulated tool-change position for
             ;Tool Call during a block scan
             ;Input: -99 999.9999 to +99 999.9999 [mm] or [degrees]
MP 951.0   : +0
MP 951.1   : +0
MP 951.2   : +0
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
MP 951.14  : +0
MP 951.15  : +0
MP 951.16  : +0
MP 951.17  : +0
MP 951.18  : +0
MP 951.19  : +0
MP 951.20  : +0
MP 951.21  : +0
             ;------------------------------------------------------------------
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
MP 960.14  : +0.00000
MP 960.15  : +0.00000
MP 960.16  : +0.00000
MP 960.17  : +0.00000
MP 960.18  : +0.00000
MP 960.19  : +0.00000
MP 960.20  : +0.00000
MP 960.21  : +0.00000
             ;==================================================================
             ;MP 1000 .. 1399
             ;Positioning / Controlling
             ;==================================================================
             ;MP1010 Axis-specific rapid traverse. At program-end and
             ;       program-choice is always MP1010 active.
             ;Input: 10 to 1 000 000 [mm/min] or [degrees/min]
MP 1010.0  : 10000
MP 1010.1  : 10000
MP 1010.2  : 10000
MP 1010.3  : 2000
MP 1010.4  : 2000
MP 1010.5  : 2000
MP 1010.6  : 2000
MP 1010.7  : 10000
MP 1010.8  : 10000
MP 1010.9  : 10000
MP 1010.10 : 10000
MP 1010.11 : 10000
MP 1010.12 : 10000
MP 1010.13 : 10000
MP 1010.14 : 10000
MP 1010.15 : 10000
MP 1010.16 : 10000
MP 1010.17 : 10000
MP 1010.18 : 10000
MP 1010.19 : 10000
MP 1010.20 : 10000
MP 1010.21 : 10000
             ;------------------------------------------------------------------
             ;MP1011 Max. rapid traverse for path interpolation
             ;Input: 10 to 1 000 000 [mm/min] or [degrees/min]
MP 1011    : 100000
             ;------------------------------------------------------------------
             ;MP1012 Second axis-specific rapid traverse. Switching between
             ;       rapid traverse in MP1010 or MP1012 via function FN17.
             ;       With use of rapid traverse in MP1012 is
             ;       MP1011 inactive.
             ;Input: 10 to 1 000 000 [mm/min] or [degrees/min]
MP 1012.0  : 10000
MP 1012.1  : 10000
MP 1012.2  : 10000
MP 1012.3  : 2000
MP 1012.4  : 2000
MP 1012.5  : 2000
MP 1012.6  : 2000
MP 1012.7  : 10000
MP 1012.8  : 10000
MP 1012.9  : 10000
MP 1012.10 : 10000
MP 1012.11 : 10000
MP 1012.12 : 10000
MP 1012.13 : 10000
MP 1012.14 : 10000
MP 1012.15 : 10000
MP 1012.16 : 10000
MP 1012.17 : 10000
MP 1012.18 : 10000
MP 1012.19 : 10000
MP 1012.20 : 10000
MP 1012.21 : 10000
             ;------------------------------------------------------------------
             ;MP1020 Axis-specific manual feed rate
             ;Input: 10 to 1 000 000 [mm/min] or [degrees/min]
MP 1020.0  : 1000
MP 1020.1  : 1000
MP 1020.2  : 1000
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
MP 1020.14 : 1000
MP 1020.15 : 1000
MP 1020.16 : 1000
MP 1020.17 : 1000
MP 1020.18 : 1000
MP 1020.19 : 1000
MP 1020.20 : 1000
MP 1020.21 : 1000
             ;------------------------------------------------------------------
             ;MP1030 Axis-specific positioning window
             ;Input: 0.0001 to 2.0000 [mm] or [degrees]
MP 1030.0  : 0.01
MP 1030.1  : 0.01
MP 1030.2  : 0.01
MP 1030.3  : 0.01
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
MP 1030.14 : 0.01
MP 1030.15 : 0.01
MP 1030.16 : 0.01
MP 1030.17 : 0.01
MP 1030.18 : 0.01
MP 1030.19 : 0.01
MP 1030.20 : 0.01
MP 1030.21 : 0.01
             ;------------------------------------------------------------------
             ;MP1040 Polarity of the nominal speed value for positive
             ;       direction of traverse
             ;Input: %987654321 bit-encoded
             ; 0 = Positive
             ; 1 = Negative
             ;2109876543210987654ZYX
MP 1040    : %0000000000000000000000
             ;------------------------------------------------------------------
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
MP 1050.14 : 1
MP 1050.15 : 1
MP 1050.16 : 1
MP 1050.17 : 1
MP 1050.18 : 1
MP 1050.19 : 1
MP 1050.20 : 1
MP 1050.21 : 1
             ;------------------------------------------------------------------
             ;MP1054 Linear distance of one motor revolution
             ;Input: 0 to a maximum of 15 figures [mm]
             ;      Example: 123456798,012345 [mm]
             ;      Example: 123,456798012345 [mm]
MP 1054.0  : 10
MP 1054.1  : 10
MP 1054.2  : 10
MP 1054.3  : 10
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
MP 1054.14 : 10
MP 1054.15 : 10
MP 1054.16 : 10
MP 1054.17 : 10
MP 1054.18 : 10
MP 1054.19 : 10
MP 1054.20 : 10
MP 1054.21 : 10
             ;------------------------------------------------------------------
             ;MP1060 Axis-specific acceleration
             ;Input: 0.001 to 500 [m/s^2] or [1000°/s^2]
MP 1060.0  : 0.5
MP 1060.1  : 0.5
MP 1060.2  : 0.5
MP 1060.3  : 0.5
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
MP 1060.14 : 0.5
MP 1060.15 : 0.5
MP 1060.16 : 0.5
MP 1060.17 : 0.5
MP 1060.18 : 0.5
MP 1060.19 : 0.5
MP 1060.20 : 0.5
MP 1060.21 : 0.5
             ;------------------------------------------------------------------
             ;MP1061 Acceleration limitation for path interpolation
             ;       MP1061.0 Path acceleration for machining feed rate
             ;       MP1061.1 Path acceleration for rapid traverse
             ;Input: 0.001 to 500 [m/s^2]
MP 1061.0  : 0.5
MP 1061.1  : 0.5
             ;------------------------------------------------------------------
             ;MP1070 Radial acceleration
             ;Input: 0.001 to 500 [m/s^2]
MP 1070    : 0.5
             ;------------------------------------------------------------------
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
MP 1080.14 : 0
MP 1080.15 : 0
MP 1080.16 : 0
MP 1080.17 : 0
MP 1080.18 : 0
MP 1080.19 : 0
MP 1080.20 : 0
MP 1080.21 : 0
             ;------------------------------------------------------------------
             ;MP1085 Max. permissible axis-specific jerk during machining
             ;       movements for the operating modes
             ;       "Program Run Full Sequence," "Program Run Single Block,"
             ;       and "Positioning with Manual Data Input",
             ;       as long as feed rate not FMAX or
             ;       feed rate < value in MP1092.
             ;       (up to now for all axis in MP1090.0)
             ;            MP1060.x^2 * 60000
             ; MP1085.x >= ------------------
             ;               MP1010.x
             ;Input: 0.1 to 9999.9 [m/s^3] or [1000°/s^3]
MP 1085.0  : 10
MP 1085.1  : 10
MP 1085.2  : 10
MP 1085.3  : 10
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
MP 1085.14 : 10
MP 1085.15 : 10
MP 1085.16 : 10
MP 1085.17 : 10
MP 1085.18 : 10
MP 1085.19 : 10
MP 1085.20 : 10
MP 1085.21 : 10
             ;------------------------------------------------------------------
             ;MP1086 Max. permissible axis-specific jerk during machining
             ;       movements for the operating modes
             ;       "Program Run Full Sequence," "Program Run Single Block,"
             ;       and "Positioning with Manual Data Input",
             ;       when feed rate at rapid traverse or
             ;       feed rate > value in MP1092.
             ;       (up to now for all axis in MP1090.1)
             ;            MP1060.x^2 * 60000
             ; MP1086.x >= ------------------
             ;               MP1010.x
             ;Input: 0.1 to 9999.9 [m/s^3] or [1000°/s^3]
             ;       0 = not active (even with FMAX value from MP1085.x)
MP 1086.0  : 20
MP 1086.1  : 20
MP 1086.2  : 20
MP 1086.3  : 20
MP 1086.4  : 20
MP 1086.5  : 20
MP 1086.6  : 20
MP 1086.7  : 20
MP 1086.8  : 20
MP 1086.9  : 20
MP 1086.10 : 20
MP 1086.11 : 20
MP 1086.12 : 20
MP 1086.13 : 20
MP 1086.14 : 20
MP 1086.15 : 20
MP 1086.16 : 20
MP 1086.17 : 20
MP 1086.18 : 20
MP 1086.19 : 20
MP 1086.20 : 20
MP 1086.21 : 20
             ;------------------------------------------------------------------
             ;MP1087 Axis-specific jerk limitation in the manual
             ;       mode of operation
             ;Input: 0.1 to 1000 [m/s^3] or [1000°/s^3]
MP 1087.0  : 10
MP 1087.1  : 10
MP 1087.2  : 10
MP 1087.3  : 10
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
MP 1087.14 : 10
MP 1087.15 : 10
MP 1087.16 : 10
MP 1087.17 : 10
MP 1087.18 : 10
MP 1087.19 : 10
MP 1087.20 : 10
MP 1087.21 : 10
             ;------------------------------------------------------------------
             ;MP1088 Axis-specific jerk limitation for non-filtert
             ;       movements
             ;       (PLC positioning, measuring cycles)
             ;Input: 0,1 to 9999,9 [m/s^3] or [1000°/s^3]
MP 1088.0  : 5
MP 1088.1  : 5
MP 1088.2  : 5
MP 1088.3  : 5
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
MP 1088.14 : 5
MP 1088.15 : 5
MP 1088.16 : 5
MP 1088.17 : 5
MP 1088.18 : 5
MP 1088.19 : 5
MP 1088.20 : 5
MP 1088.21 : 5
             ;------------------------------------------------------------------
             ;MP1089 Axis-specific jerk limitation during reference traverse
             ;Input: 0.1 to 1000 [m/s^3] or [1000°/s^3]
MP 1089.0  : 10
MP 1089.1  : 10
MP 1089.2  : 10
MP 1089.3  : 10
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
MP 1089.14 : 10
MP 1089.15 : 10
MP 1089.16 : 10
MP 1089.17 : 10
MP 1089.18 : 10
MP 1089.19 : 10
MP 1089.20 : 10
MP 1089.21 : 10
             ;------------------------------------------------------------------
             ;MP1090 Limitation of contouring jerk as a result from
             ;       MP1085.x or MP1086.x
             ;            MP1060^2 * 60000
             ; MP1090 >= ------------------
             ;               MP1010
             ;Input: 0.1 to 9999.9 [m/s^3] or [1000°/s^3]
             ;       0 = not active
MP 1090.0  : 10         ;Jerk limitation if a feed rate-value is programmed
MP 1090.1  : 20         ;Jerk limitation if feed rate FMAX is programmed
             ;------------------------------------------------------------------
             ;MP1092 Feed rate value to switch between jerk in MP1085.x or
             ;       MP1086.x
             ;       programmed feed rate < MP1092 --> MP1085.x active
             ;       programmed feed rate > MP1092 --> MP1086.x active
             ;       MP1092 = 0 --> MP1085.x during machining movements
             ;       without FMAX active and MP1086.x during machining
             ;       movements with FMAX active.
             ;Input: 0 to 300 000 [mm/min]
MP 1092    : 0
             ;------------------------------------------------------------------
             ;MP1094 An HSC filter can be activated by entering the limit
             ;       frequency (20-30% below resonance frequency of machine).
             ;       The jerk from MP1098 is effective.
             ;       MP1095 and MP1099 have no effect.
             ;Input: 0 = Same filter as before
             ;       1 to 166.0 [Hz]
MP 1094    : 0
             ;------------------------------------------------------------------
             ;MP1095 Cycle 32 3-D Milling - Filter
             ;Input:   0 to 1
             ; 0 = Single filter
             ; 1 = Double filter
MP 1095.0  : 0          ;Program Run, Positioning with Manual Data Input
MP 1095.1  : 0          ;Manual, Handwheel, Jog Increment, Reference Points
             ;------------------------------------------------------------------
             ;MP1095 Cycle 32 3-D Milling - Predefined tolerance
             ;Input:   0 to 3
MP 1096.0  : 0.02       ;Corners with movement at machining feed-rate
MP 1096.1  : 0.08       ;Corners with movement at rapid traverse
             ;------------------------------------------------------------------
             ;MP1097 Cycle 32 3-D Milling - Jerk limitation with single f
             ;Input: 0.1 to 1000 [m/s^3]
MP 1097.0  : 10
MP 1097.1  : 10
MP 1097.2  : 10
MP 1097.3  : 10
MP 1097.4  : 10
MP 1097.5  : 10
MP 1097.6  : 10
MP 1097.7  : 10
MP 1097.8  : 10
MP 1097.9  : 10
MP 1097.10 : 10
MP 1097.11 : 10
MP 1097.12 : 10
MP 1097.13 : 10
MP 1097.14 : 10
MP 1097.15 : 10
MP 1097.16 : 10
MP 1097.17 : 10
MP 1097.18 : 10
MP 1097.19 : 10
MP 1097.20 : 10
MP 1097.21 : 10
             ;------------------------------------------------------------------
             ;MP1098 Cycle 32 3-D Milling - Jerk limitation with double f
             ;Input: 0.1 to 1000 [m/s^3]
MP 1098.0  : 10
MP 1098.1  : 10
MP 1098.2  : 10
MP 1098.3  : 10
MP 1098.4  : 10
MP 1098.5  : 10
MP 1098.6  : 10
MP 1098.7  : 10
MP 1098.8  : 10
MP 1098.9  : 10
MP 1098.10 : 10
MP 1098.11 : 10
MP 1098.12 : 10
MP 1098.13 : 10
MP 1098.14 : 10
MP 1098.15 : 10
MP 1098.16 : 10
MP 1098.17 : 10
MP 1098.18 : 10
MP 1098.19 : 10
MP 1098.20 : 10
MP 1098.21 : 10
             ;------------------------------------------------------------------
             ;MP1099 Cycle 32 3-D Milling - Minimum filter order
             ;Input: 0 to 20
MP 1099.0  : 5          ;Single filter
MP 1099.1  : 3          ;Double filter
             ;------------------------------------------------------------------
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
MP 1110.14 : 0.2
MP 1110.15 : 0.2
MP 1110.16 : 0.2
MP 1110.17 : 0.2
MP 1110.18 : 0.2
MP 1110.19 : 0.2
MP 1110.20 : 0.2
MP 1110.21 : 0.2
             ;------------------------------------------------------------------
             ;MP1120 Standstill monitoring for field angle determination
             ;Input: 0.0000 to 300.0000 [mm] or [°]
MP 1120.0  : 0.5
MP 1120.1  : 0.5
MP 1120.2  : 0.5
MP 1120.3  : 0.5
MP 1120.4  : 0.5
MP 1120.5  : 0.5
MP 1120.6  : 0.5
MP 1120.7  : 0.5
MP 1120.8  : 0.5
MP 1120.9  : 0.5
MP 1120.10 : 0.5
MP 1120.11 : 0.5
MP 1120.12 : 0.5
MP 1120.13 : 0.5
MP 1120.14 : 0.5
MP 1120.15 : 0.5
MP 1120.16 : 0.5
MP 1120.17 : 0.5
MP 1120.18 : 0.5
MP 1120.19 : 0.5
MP 1120.20 : 0.5
MP 1120.21 : 0.5
             ;------------------------------------------------------------------
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
MP 1140.14 : 0.03
MP 1140.15 : 0.03
MP 1140.16 : 0.03
MP 1140.17 : 0.03
MP 1140.18 : 0.03
MP 1140.19 : 0.03
MP 1140.20 : 0.03
MP 1140.21 : 0.03
             ;------------------------------------------------------------------
             ;MP1144 Movement monitoring for position and speed
             ;(Gross positioning error x C)
             ;Input: 0 to 99 999.9999 [mm]
             ;       0 = No monitoring
MP 1144.0  : 0.5
MP 1144.1  : 0.5
MP 1144.2  : 0.5
MP 1144.3  : 0.5
MP 1144.4  : 0.5
MP 1144.5  : 0.5
MP 1144.6  : 0.5
MP 1144.7  : 0.5
MP 1144.8  : 0.5
MP 1144.9  : 0.5
MP 1144.10 : 0.5
MP 1144.11 : 0.5
MP 1144.12 : 0.5
MP 1144.13 : 0.5
MP 1144.14 : 0.5
MP 1144.15 : 0.5
MP 1144.16 : 0.5
MP 1144.17 : 0.5
MP 1144.18 : 0.5
MP 1144.19 : 0.5
MP 1144.20 : 0.5
MP 1144.21 : 0.5
             ;------------------------------------------------------------------
             ;MP1146 Position monitoring for EnDat, max. permissible
             ;       difference between stored switch-off position and
             ;       the position transferred via the EnDat interface
             ;Input: 0.0000 to 300.0000 [mm]
             ;       0: No difference permitted
MP 1146.0  : 1
MP 1146.1  : 1
MP 1146.2  : 1
MP 1146.3  : 1
MP 1146.4  : 1
MP 1146.5  : 1
MP 1146.6  : 1
MP 1146.7  : 1
MP 1146.8  : 1
MP 1146.9  : 1
MP 1146.10 : 1
MP 1146.11 : 1
MP 1146.12 : 1
MP 1146.13 : 1
MP 1146.14 : 1
MP 1146.15 : 1
MP 1146.16 : 1
MP 1146.17 : 1
MP 1146.18 : 1
MP 1146.19 : 1
MP 1146.20 : 1
MP 1146.21 : 1
             ;------------------------------------------------------------------
             ;MP1147 Sperren der Positionsübernahme für Achsen mit
             ;       EnDat-Messgerät
             ;Eingabe: %..987654321 bitcodiert
             ;         0 = Übernahme der Position möglich
             ;         1 = Positionsübernahme gesperrt. Fehlermeldung
             ;             und Not-Halt.
             ;2109876543210987654ZYX
MP 1147    : %0000000000000000000000
             ;------------------------------------------------------------------
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
             ;------------------------------------------------------------------
             ;MP1160 (only CC 424, CC 61xx) Distance for LIFTOFF
             ;Distance from tool to contour, which is taken by the LIFTOFF
             ;function (at emergency stop at DSP-faults)
             ;MP appears at CC 422 too, but has no function.
             ;Input 0 bis 30.0000 [mm]
             ;Default: 0.1 [mm]
MP 1160    : 0.1
             ;------------------------------------------------------------------
             ;MP1200 Nominal position value filter
             ;Input: 0 = Single filter
             ;       1 = Double filter
             ;       2 = HSC filter
             ;       3 = "Advanced HSC filter"
MP 1200    : 0
             ;------------------------------------------------------------------
             ;MP1201 Nominal position value filter for modes: Manual,
             ;       Hand wheel, jog increment, reference marks
             ;Input: 0 = Single filter
             ;       1 = Double filter
MP 1201    : 0
             ;------------------------------------------------------------------
             ;MP1202 Cycle 32 3D milling - predefined tolerance
             ;Input: 0,0000 to 3,0000 [mm]
MP 1202.0  : 0.02       ;Corners with movement with machining feed rate
MP 1202.1  : 0.25       ;Corners with movement with rapid traverse
             ;------------------------------------------------------------------
             ;MP1205 Reducing contouring feed rate at the beginning of a
             ;       contour-element
             ;Input: 0 = not active (fast but less precision)
             ;       1 = active (slow but more precision)
MP 1205    : 0
             ;------------------------------------------------------------------
             ;MP1210 Cut-off frequency for single filter
             ;Input: 0,0 to 166,0 [Hz]
MP 1210    : 20
             ;------------------------------------------------------------------
             ;MP1211 Cut-off frequency for double filter
             ;Input: 0,0 to 166,0 [Hz]
MP 1211    : 20
             ;------------------------------------------------------------------
             ;MP1212 Cut-off frequency for for HSC filter
             ;Input: 0,0 to 166,0 [Hz]
MP 1212    : 12
             ;------------------------------------------------------------------
             ;MP1213 Cut-off frequency for "advanced HSC filter"
             ;Input: 0,0 to 166,0 [Hz]
MP 1213    : 12
             ;------------------------------------------------------------------
             ;MP1218 Cut-off frequency for single filter for
             ;       rotary axis (A, B, C)
             ;Input: 0,0 to 166,0 [Hz]
             ;       0 = Triangle filter (default)
MP 1218.0  : 0
MP 1218.1  : 0
MP 1218.2  : 0
             ;------------------------------------------------------------------
             ;MP1222 Tolerance at changes of curvature for HSC filter
             ;Input: 0 = Ignore tolerance
             ;       1 = Do not ignore tolerance
MP 1222    : 0
             ;------------------------------------------------------------------
             ;MP1223 Tolerance at changes of curvature for
             ;       "advanced HSC filter"
             ;Input: 0 = Ignore tolerance
             ;       1 = Do not ignore tolerance
MP 1223    : 0
             ;------------------------------------------------------------------
             ;MP1230 Jerk at corners for single filter
             ;Input: 0,1 to 1000,0 [m/s^3]
MP 1230.0  : 10
MP 1230.1  : 10
MP 1230.2  : 10
MP 1230.3  : 10
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
MP 1230.14 : 10
MP 1230.15 : 10
MP 1230.16 : 10
MP 1230.17 : 10
MP 1230.18 : 10
MP 1230.19 : 10
MP 1230.20 : 10
MP 1230.21 : 10
             ;------------------------------------------------------------------
             ;MP1231 Jerk at corners for  double filter
             ;Input: 0,1 to 1000,0 [m/s^3]
MP 1231.0  : 10
MP 1231.1  : 10
MP 1231.2  : 10
MP 1231.3  : 10
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
MP 1231.14 : 10
MP 1231.15 : 10
MP 1231.16 : 10
MP 1231.17 : 10
MP 1231.18 : 10
MP 1231.19 : 10
MP 1231.20 : 10
MP 1231.21 : 10
             ;------------------------------------------------------------------
             ;MP1232 Jerk at corners for HSC filter
             ;Input: 0,1 to 1000,0 [m/s^3]
MP 1232.0  : 10
MP 1232.1  : 10
MP 1232.2  : 10
MP 1232.3  : 10
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
MP 1232.14 : 10
MP 1232.15 : 10
MP 1232.16 : 10
MP 1232.17 : 10
MP 1232.18 : 10
MP 1232.19 : 10
MP 1232.20 : 10
MP 1232.21 : 10
             ;------------------------------------------------------------------
             ;MP1233 Jerk at corners for "advanced HSC filter"
             ;Input: 0,1 to 1000,0 [m/s^3]
MP 1233.0  : 10
MP 1233.1  : 10
MP 1233.2  : 10
MP 1233.3  : 10
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
MP 1233.14 : 10
MP 1233.15 : 10
MP 1233.16 : 10
MP 1233.17 : 10
MP 1233.18 : 10
MP 1233.19 : 10
MP 1233.20 : 10
MP 1233.21 : 10
             ;------------------------------------------------------------------
             ;MP1240 Jerk at changes of curvature for single filter
             ;Input: 0,1 to 1000,0 [m/s^3]
MP 1240.0  : 10
MP 1240.1  : 10
MP 1240.2  : 10
MP 1240.3  : 10
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
MP 1240.14 : 10
MP 1240.15 : 10
MP 1240.16 : 10
MP 1240.17 : 10
MP 1240.18 : 10
MP 1240.19 : 10
MP 1240.20 : 10
MP 1240.21 : 10
             ;------------------------------------------------------------------
             ;MP1241 Jerk at changes of curvature for double filter
             ;Input: 0,1 to 1000,0 [m/s^3]
MP 1241.0  : 10
MP 1241.1  : 10
MP 1241.2  : 10
MP 1241.3  : 10
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
MP 1241.14 : 10
MP 1241.15 : 10
MP 1241.16 : 10
MP 1241.17 : 10
MP 1241.18 : 10
MP 1241.19 : 10
MP 1241.20 : 10
MP 1241.21 : 10
             ;------------------------------------------------------------------
             ;MP1242 Jerk at changes of curvature for HSC filter
             ;Input: 0,1 to 1000,0 [m/s^3]
MP 1242.0  : 10
MP 1242.1  : 10
MP 1242.2  : 10
MP 1242.3  : 10
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
MP 1242.14 : 10
MP 1242.15 : 10
MP 1242.16 : 10
MP 1242.17 : 10
MP 1242.18 : 10
MP 1242.19 : 10
MP 1242.20 : 10
MP 1242.21 : 10
             ;------------------------------------------------------------------
             ;MP1243 Jerk at changes of curvature for "advanced HSC filter"
             ;Input: 0,1 to 1000,0 [m/s^3]
MP 1243.0  : 10
MP 1243.1  : 10
MP 1243.2  : 10
MP 1243.3  : 10
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
MP 1243.14 : 10
MP 1243.15 : 10
MP 1243.16 : 10
MP 1243.17 : 10
MP 1243.18 : 10
MP 1243.19 : 10
MP 1243.20 : 10
MP 1243.21 : 10
             ;------------------------------------------------------------------
             ;MP1250 Factor for Jerk at corners/change of curvature at
             ;changes between rapid traverses
             ;Input: 0,0000 to +30,0000
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
MP 1250.14 : 1
MP 1250.15 : 1
MP 1250.16 : 1
MP 1250.17 : 1
MP 1250.18 : 1
MP 1250.19 : 1
MP 1250.20 : 1
MP 1250.21 : 1
             ;------------------------------------------------------------------
             ;MP1262 Minimum filter order for "HSC filter"
             ;Input: 0 to 31
MP 1262    : 31
             ;------------------------------------------------------------------
             ;MP1263 Minimum filter order for "Advanced HSC filter"
             ;Input: 0 to 31
MP 1263    : 31
             ;------------------------------------------------------------------
             ;MP1290 Maximum angle tolerance for
             ;"DCM - Dynamic Collison Monitoring"
             ;Input: 0.0000 bis 3.0000 [°]
             ;Default: 3 [°]
MP 1290    : 3
             ;------------------------------------------------------------------
             ;MP1292 Allowance for collision monitored objects at active
             ;"DCM - Dynamic Collision Monitoring" (with M128 or TCPM)
             ;Input: 0 bis 1000 [mm]
MP 1292    : 0
             ;------------------------------------------------------------------
             ;MP1294 Allow in Manual Operation and El. Handwheel
             ;only the movement of a single axis to achieve higher
             ;feed rates with active DCM
             ;Input: 0 = not active
             ;       1 = active
MP 1294    : 0
             ;------------------------------------------------------------------
             ;MP1295 Error message "Two collision-monitored objects have come
             ;       below a certain minimum clearance." while M140 is active
             ;       if two collision objects repeatedly came below the minimum
             ;       clearance. This is regardless of whether DCM is active or
             ;       inactive.
             ;Input: 0 = No error message
             ;       1 = Error message is displayed
MP 1295    : 0
             ;------------------------------------------------------------------
             ;MP1310 Order for checking the axes
             ;Input: 1 to 18 [number at which point the axis should be
             ;       checked]
             ;       0 = not active
MP 1310.0  : +0
MP 1310.1  : +0
MP 1310.2  : +0
MP 1310.3  : +0
MP 1310.4  : +0
MP 1310.5  : +0
MP 1310.6  : +0
MP 1310.7  : +0
MP 1310.8  : +0
MP 1310.9  : +0
MP 1310.10 : +0
MP 1310.11 : +0
MP 1310.12 : +0
MP 1310.13 : +0
MP 1310.14 : +0
MP 1310.15 : +0
MP 1310.16 : +0
MP 1310.17 : +0
MP 1310.18 : +0
MP 1310.19 : +0
MP 1310.20 : +0
MP 1310.21 : +0
             ;------------------------------------------------------------------
             ;MP1320 Direction for traversing the reference marks
             ;Input: %987654321 bit-encoded
             ; 0 = Positive
             ; 1 = Negative
             ;2109876543210987654ZYX
MP 1320    : %0000000000000000000000
             ;------------------------------------------------------------------
             ;MP1330 Velocity for traversing the reference marks
             ;Input: 80 to 1 000 000 [mm/min]
MP 1330.0  : 10000
MP 1330.1  : 10000
MP 1330.2  : 10000
MP 1330.3  : 10000
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
MP 1330.14 : 10000
MP 1330.15 : 10000
MP 1330.16 : 10000
MP 1330.17 : 10000
MP 1330.18 : 10000
MP 1330.19 : 10000
MP 1330.20 : 10000
MP 1330.21 : 10000
             ;------------------------------------------------------------------
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
MP 1331.14 : 500
MP 1331.15 : 500
MP 1331.16 : 500
MP 1331.17 : 500
MP 1331.18 : 500
MP 1331.19 : 500
MP 1331.20 : 500
MP 1331.21 : 500
             ;------------------------------------------------------------------
             ;MP1340 Sequence for traversing the reference points
             ;Input: 0     = No evaluation of the reference mark
             ;       1..22 = Axes 1..22
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
MP 1340.14 : 15
MP 1340.15 : 16
MP 1340.16 : 17
MP 1340.17 : 18
MP 1340.18 : 19
MP 1340.19 : 20
MP 1340.20 : 21
MP 1340.21 : 22
             ;------------------------------------------------------------------
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
MP 1350.0  : 1
MP 1350.1  : 1
MP 1350.2  : 1
MP 1350.3  : 1
MP 1350.4  : 1
MP 1350.5  : 1
MP 1350.6  : 1
MP 1350.7  : 1
MP 1350.8  : 1
MP 1350.9  : 1
MP 1350.10 : 1
MP 1350.11 : 1
MP 1350.12 : 1
MP 1350.13 : 1
MP 1350.14 : 1
MP 1350.15 : 1
MP 1350.16 : 1
MP 1350.17 : 1
MP 1350.18 : 1
MP 1350.19 : 1
MP 1350.20 : 1
MP 1350.21 : 1
             ;------------------------------------------------------------------
             ;MP1352 Axis for which the monitoring of the software limit
             ;       switches can be activated before the reference marks
             ;       are passed. (Last saved actual position,
             ;       activation via softkey)
             ;Input: 0 = Monitoring can not be activated
             ;       1 = Monitoring can be activated
             ;2109876543210987654ZYX
MP 1352    : %0000000000000000000000
             ;------------------------------------------------------------------
             ;MP1355 Reference run
             ;Input: 0 = Reference run as defined in MP1350.x
             ;       1 = Reference run via EnDat interface
             ;           of speed encoder (distance from position
             ;           encoder in MP1356.x), repeated reference
             ;           run over position encoder at a later date
             ;2109876543210987654ZYX
MP 1355    : %0000000000000000000000
             ;------------------------------------------------------------------
             ;MP1356 Distance between speed encoder and position encoder if
             ;       MP1355 = 1
             ;Input: -1.79769313486E+308 to +1.79769313486E+308
             ;Input: -99 999.9999 to +99 999.9999 [mm] or [°]
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
MP 1356.14 : +0.00000
MP 1356.15 : +0.00000
MP 1356.16 : +0.00000
MP 1356.17 : +0.00000
MP 1356.18 : +0.00000
MP 1356.19 : +0.00000
MP 1356.20 : +0.00000
MP 1356.21 : +0.00000
             ;------------------------------------------------------------------
             ;MP1357 W1032 at dual reference run
             ;Input: 0 = Reset, if the reference run via the EnDat
             ;           interface of the speed encoder is done
             ;       1 = Reset, if the reference mark of the position
             ;           encoder is crossed
             ;2109876543210987654ZYX
MP 1357    : %0000000000000000000000
             ;------------------------------------------------------------------
             ;MP1358 Readout multiturn information
             ;Input: 0 = Do not readout multiturn information
             ;       1 = Readout multiturn information
MP 1358    : %0000000000000000000000
             ;------------------------------------------------------------------
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
MP 1360.14 : 0
MP 1360.15 : 0
MP 1360.16 : 0
MP 1360.17 : 0
MP 1360.18 : 0
MP 1360.19 : 0
MP 1360.20 : 0
MP 1360.21 : 0
             ;------------------------------------------------------------------
             ;MP1391.0 Velocity feedforward control in the operating modes
             ;       Manual and Handwheel
             ;Input: %xxxxxxxxxxxxxx
             ;       0 = Operation with following error (lag)
             ;       1 = Operation with velocity feedforward control
             ;2109876543210987654ZYX
MP 1391.0  : %1111111111111111111111
             ;------------------------------------------------------------------
             ;MP1391.1 Acceleration feedforward in the operating modes
             ;       Manual and Handwheel
             ;Input: %xxxxxxxxxxxxxx
             ;         0 = Acceleration feedforward control inactive
             ;         1 = Acceleration feedforward control active
             ;2109876543210987654ZYX
MP 1391.1  : %0000000000000000000000
             ;------------------------------------------------------------------
             ;MP1392 Velocity feedforward control in the operating modes
             ;       Positioning with Manual Data Input,
             ;       Program Run Single Block and Program Run Full Sequence
             ;Input: %xxxxxxxxxxxxxx
             ;       0 = Operation with following error (lag)
             ;       1 = Operation with velocity feedforward control
             ;2109876543210987654ZYX
MP 1392    : %1111111111111111111111
             ;------------------------------------------------------------------
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
MP 1396.14 : 1
MP 1396.15 : 1
MP 1396.16 : 1
MP 1396.17 : 1
MP 1396.18 : 1
MP 1396.19 : 1
MP 1396.20 : 1
MP 1396.21 : 1
             ;------------------------------------------------------------------
             ;MP1398 Motion behavior for certain positioning movements
             ;       Note: The softer movement results in a  
             ;       longer braking distance, e.g. when NC Stop is pressed or 
             ;       the probe deflected. It corresponds to a 
             ;       delay time of 20 ms, i.e. if 
             ;       F3000 mm/min = 50 mm/s , the braking distance 
             ;       is 1 mm longer.
             ;Inpout: %xx
             ;Bit 0  Softer movement for pre-positioning before
             ;       measuring movement (e.g. Cycle 417) 
             ;       0 = Not active, behavior as before
             ;           (up to and including 60642x-04 SP11)
             ;       1 = Softer movement
             ;Bit 1  Softer movement for repositioning after  
             ;       block scan or in Manual Operation  
             ;       0 = Not active, behavior as before
             ;           (up to and including 60642x-04 SP11)
             ;       1 = Softer movement
             ;10
MP 1398    : %00
             ;==================================================================
             ;MP 1400 .. 1699
             ;Operation with velocity feedforward control
             ;==================================================================
             ;MP1410 Position monitoring during operation with
             ;feedforward control
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
MP 1410.14 : 0.5
MP 1410.15 : 0.5
MP 1410.16 : 0.5
MP 1410.17 : 0.5
MP 1410.18 : 0.5
MP 1410.19 : 0.5
MP 1410.20 : 0.5
MP 1410.21 : 0.5
             ;------------------------------------------------------------------
             ;MP1420 Position monitoring during operation with feedforward contr
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
MP 1420.14 : 2
MP 1420.15 : 2
MP 1420.16 : 2
MP 1420.17 : 2
MP 1420.18 : 2
MP 1420.19 : 2
MP 1420.20 : 2
MP 1420.21 : 2
             ;------------------------------------------------------------------
             ;MP1510 kv factor for velocity feedforward control
             ;Input: 0.100 to 1000.000 [m/(min*mm)]
MP 1510.0  : 1
MP 1510.1  : 1
MP 1510.2  : 1
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
MP 1510.14 : 1
MP 1510.15 : 1
MP 1510.16 : 1
MP 1510.17 : 1
MP 1510.18 : 1
MP 1510.19 : 1
MP 1510.20 : 1
MP 1510.21 : 1
             ;------------------------------------------------------------------
             ;MP1511 Factor for static friction compensation
             ;Input:   0 to 16 777 215
             ;Static friction compensation during operation with
             ;feedforward control
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
MP 1511.14 : 0
MP 1511.15 : 0
MP 1511.16 : 0
MP 1511.17 : 0
MP 1511.18 : 0
MP 1511.19 : 0
MP 1511.20 : 0
MP 1511.21 : 0
             ;------------------------------------------------------------------
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
MP 1512.14 : 0
MP 1512.15 : 0
MP 1512.16 : 0
MP 1512.17 : 0
MP 1512.18 : 0
MP 1512.19 : 0
MP 1512.20 : 0
MP 1512.21 : 0
             ;------------------------------------------------------------------
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
MP 1513.14 : 0
MP 1513.15 : 0
MP 1513.16 : 0
MP 1513.17 : 0
MP 1513.18 : 0
MP 1513.19 : 0
MP 1513.20 : 0
MP 1513.21 : 0
             ;------------------------------------------------------------------
             ;MP1515 kv factor for velocity feedforward control (M105)
             ;Input: 0.100 to 1000.000 [m/(min*mm)]
             ;Can be switched on/off via M105, M106
MP 1515.0  : 3
MP 1515.1  : 3
MP 1515.2  : 3
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
MP 1515.14 : 3
MP 1515.15 : 3
MP 1515.16 : 3
MP 1515.17 : 3
MP 1515.18 : 3
MP 1515.19 : 3
MP 1515.20 : 3
MP 1515.21 : 3
             ;------------------------------------------------------------------
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
MP 1516.14 : 1
MP 1516.15 : 1
MP 1516.16 : 1
MP 1516.17 : 1
MP 1516.18 : 1
MP 1516.19 : 1
MP 1516.20 : 1
MP 1516.21 : 1
             ;------------------------------------------------------------------
             ;MP1521 Transient response during acceleration and deceleration
             ;Input: 0 to 255 [ms]
MP 1521    : 0
             ;------------------------------------------------------------------
             ;MP1522 Speed smoothing
             ;recommended only for machines natural frequency (Fma) <=25Hz
             ;initial value Fma/2, increase in steps of 5ms while lag error
             ;is decreasing
             ;Input: 0 to 60 [ms]
MP 1522    : 0
             ;==================================================================
             ;MP 1700 .. 1899
             ;Operation with following error (servo lag)
             ;==================================================================
             ;MP1710 Position monitoring for operation with following error
             ;Erasable (positioning error x)
             ;Input: 0.0000 to 300.0000 [mm]
MP 1710.0  : 12
MP 1710.1  : 12
MP 1710.2  : 12
MP 1710.3  : 12
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
MP 1710.14 : 12
MP 1710.15 : 12
MP 1710.16 : 12
MP 1710.17 : 12
MP 1710.18 : 12
MP 1710.19 : 12
MP 1710.20 : 12
MP 1710.21 : 12
             ;------------------------------------------------------------------
             ;MP1720 Position monitoring for operation with following error
             ;EMERGENCY STOP (gross positioning error x A)
             ;Input: 0.0000 to 300.0000 [mm]
MP 1720.0  : 14
MP 1720.1  : 14
MP 1720.2  : 14
MP 1720.3  : 14
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
MP 1720.14 : 14
MP 1720.15 : 14
MP 1720.16 : 14
MP 1720.17 : 14
MP 1720.18 : 14
MP 1720.19 : 14
MP 1720.20 : 14
MP 1720.21 : 14
             ;------------------------------------------------------------------
             ;MP1810 kv factor for operation with following error
             ;Input: 0.100 to 20.000 [m/(min*mm)]
MP 1810.0  : 1
MP 1810.1  : 1
MP 1810.2  : 1
MP 1810.3  : 1
MP 1810.4  : 1
MP 1810.5  : 1
MP 1810.6  : 1
MP 1810.7  : 1
MP 1810.8  : 1
MP 1810.9  : 1
MP 1810.10 : 1
MP 1810.11 : 1
MP 1810.12 : 1
MP 1810.13 : 1
MP 1810.14 : 1
MP 1810.15 : 1
MP 1810.16 : 1
MP 1810.17 : 1
MP 1810.18 : 1
MP 1810.19 : 1
MP 1810.20 : 1
MP 1810.21 : 1
             ;------------------------------------------------------------------
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
MP 1815.14 : 3
MP 1815.15 : 3
MP 1815.16 : 3
MP 1815.17 : 3
MP 1815.18 : 3
MP 1815.19 : 3
MP 1815.20 : 3
MP 1815.21 : 3
             ;------------------------------------------------------------------
             ;MP1820 Multiplication factor for the kv factor
             ;Following error mode: Multiplication factor for kv after
             ;characteristic curve kink point
             ;Input:   0.001 to 1.000
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
MP 1820.14 : 1
MP 1820.15 : 1
MP 1820.16 : 1
MP 1820.17 : 1
MP 1820.18 : 1
MP 1820.19 : 1
MP 1820.20 : 1
MP 1820.21 : 1
             ;------------------------------------------------------------------
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
MP 1830.14 : 100
MP 1830.15 : 100
MP 1830.16 : 100
MP 1830.17 : 100
MP 1830.18 : 100
MP 1830.19 : 100
MP 1830.20 : 100
MP 1830.21 : 100
             ;==================================================================
             ;MP 2000 .. 2999
             ;Current control with PWM interface
             ;Integrated speed and current control
             ;
             ;==================================================================
             ;MP2000 Axis specific switching from single speed axis to
             ;       double speed axis (only CC 6xxx, SW-Option)
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
MP 2000.16 : 0
MP 2000.17 : 0
MP 2000.18 : 0
MP 2000.19 : 0
MP 2000.20 : 0
MP 2000.21 : 0
MP 2000.22 : 0
MP 2000.23 : 0
             ;------------------------------------------------------------------
             ;MP2040 Axis groups for which drive release via
             ;X150 (PWM output X51..X56) / X151 (PWM output X57..X64)
             ;can be switched off
             ;W524 Bit0=1 - switch off via X150/X151 active
             ;Input: %32...1 bit coded (spindle on bit #31)
             ;(Bit #15 for spindle no longer required on extended interface)
             ;              21098765432109876543210987654321
             ;MP 2040.0  : %10000000000000000000000001101111;Axis group 1
             ;MP 2040.1  : %00000000000000000000000000010000;Axis group 2
             ;21098765432109876543210987654321
MP 2040.0  : %00000000000000000000000000000000;Axis group 1
MP 2040.1  : %00000000000000000000000000000000;Axis group 2
MP 2040.2  : %00000000000000000000000000000000;Axis group 3
MP 2040.3  : %00000000000000000000000000000000;Axis group 4
MP 2040.4  : %00000000000000000000000000000000;Axis group 5
MP 2040.5  : %00000000000000000000000000000000;Axis group 6
MP 2040.6  : %00000000000000000000000000000000;Axis group 7
MP 2040.7  : %00000000000000000000000000000000;Axis group 8
             ;------------------------------------------------------------------
             ;MP2050 Functionality of drive release I32 (X42/33)(not CC 61xx)
             ;Input: 0 = EMERGENCY STOP for all axes, Module 9169 not
             ;           effective
             ;       1 = EMERGENCY STOP for all axes for which Module
             ;           9169 is not effective
             ;       2 = I32 and Module 9169 have no function
MP 2050    : 0
             ;------------------------------------------------------------------
             ;MP2100 Type of axis power modules
             ;Input:   Name from file <Inverter.inv>
MP 2100.0  : HEIDENHAIN-UM121; X
MP 2100.1  : HEIDENHAIN-UM121; Y
MP 2100.2  : HEIDENHAIN-UM121; Z
MP 2100.3  : HEIDENHAIN-UM121; a
MP 2100.4  : HEIDENHAIN-UM121; A
MP 2100.5  : HEIDENHAIN-UM113; B
MP 2100.6  : HEIDENHAIN-UM113; C
MP 2100.7  : HEIDENHAIN-UM121
MP 2100.8  : 
MP 2100.9  : 
MP 2100.10 : 
MP 2100.11 : 
MP 2100.12 : 
MP 2100.13 : 
MP 2100.14 : 
MP 2100.15 : 
MP 2100.16 : 
MP 2100.17 : 
MP 2100.18 : 
MP 2100.19 : HEIDENHAIN-UM113;Spindle 1 - C-axis mode
MP 2100.20 : HEIDENHAIN-UM113;Spindle 1 - mode 0 (Y mode)
MP 2100.21 : HEIDENHAIN-UM113;Spindle 1 - mode 1 (D mode)
MP 2100.22 : HEIDENHAIN-UM113;Spindle 2 - mode 0 (Y mode)
MP 2100.23 : HEIDENHAIN-UM113;Spindle 2 - mode 1 (D mode)
             ;------------------------------------------------------------------
             ;MP2150 Powerfail in the control via AC-fail or
             ;       powerfail signal (not CC 61xx)
             ;Input: 0 = Only AC-fail signal
             ;       1 = Powerfail and AC-fail signal
             ;       2 = reserved
             ;       3 = Only powerfail signal
MP 2150    : 3
             ;------------------------------------------------------------------
             ;MP2160 Field weakening for synchronous motors
             ;Input: 0 = Voltage-protection module not present
             ;       1 = Voltage-protection module present
             ;       2 = Limited field weakening without
             ;           voltage-protection module for EcoDyn motors
             ;       9 = Voltage-protection module present
             ;           Alternative to 1
             ;       10= Limited field weakening without
             ;           voltage-protection module for EcoDyn motors
             ;           Alternative to 2
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
MP 2160.16 : 0
MP 2160.17 : 0
MP 2160.18 : 0
MP 2160.19 : 0
MP 2160.20 : 0
MP 2160.21 : 0
MP 2160.22 : 0
MP 2160.23 : 0
             ;------------------------------------------------------------------
             ;MP2170 Waiting period between switch-on of the drive and
             ;       the drive's readiness signal
             ;Input: 0 to 4.999 [s]
             ;       0 = 2 s
MP 2170    : 0
             ;------------------------------------------------------------------
             ;MP2172 Delay for removal of inverter ready signal after serious
             ;error messages (internal emergency stop) which are triggered
             ;by the control
             ;Input: 0 to 60 [s]
             ;       0 = 3 s (default value)
MP 2172    : 0
             ;------------------------------------------------------------------
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
MP 2173.16 : 0
MP 2173.17 : 0
MP 2173.18 : 0
MP 2173.19 : 0
MP 2173.20 : 0
MP 2173.21 : 0
MP 2173.22 : 0
MP 2173.23 : 0
             ;------------------------------------------------------------------
             ;MP2180 PWM frequency
             ;       CC 422:
             ;         Same PWM frequency for axis groups
             ;       CC 424, CC61xx:
             ;         On controller board only simple or double
             ;         PWM fundamental frequency
             ;         (e. g. 5000 Hz or 10000 Hz)
             ;Input: 0 = 5000 Hz
             ;       3200 to 3999 = 3333 Hz
             ;       4000 to 4999 = 4166 Hz (CC 424: 4000 Hz)
             ;       5000 to 6000 = 5000 Hz
             ;       6001 to 7999 = 6666 Hz
             ;       8000 to 9999 = 8333 Hz (CC 424: 8000 Hz)
             ;       10000 = 10000 Hz
MP 2180.0  : 0
MP 2180.1  : 0
MP 2180.2  : 0
MP 2180.3  : 0
MP 2180.4  : 0
MP 2180.5  : 0
MP 2180.6  : 0
MP 2180.7  : 0
MP 2180.8  : 0
MP 2180.9  : 0
MP 2180.10 : 0
MP 2180.11 : 0
MP 2180.12 : 0
MP 2180.13 : 0
MP 2180.14 : 0
MP 2180.15 : 0
MP 2180.16 : 0
MP 2180.17 : 0
MP 2180.18 : 0
MP 2180.19 : 0
MP 2180.20 : 0
MP 2180.21 : 0
MP 2180.22 : 0
MP 2180.23 : 0
             ;------------------------------------------------------------------
             ;MP2182 PWM frequency mode
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
MP 2182.16 : 0
MP 2182.17 : 0
MP 2182.18 : 0
MP 2182.19 : 0
MP 2182.20 : 0
MP 2182.21 : 0
MP 2182.22 : 0
MP 2182.23 : 0
             ;------------------------------------------------------------------
             ;MP2184 reserved (not CC 61xx)
             ;Input: 0
MP 2184.0  : 0
MP 2184.1  : 0
MP 2184.2  : 0
MP 2184.3  : 0
MP 2184.4  : 0
MP 2184.5  : 0
MP 2184.6  : 0
MP 2184.7  : 0
MP 2184.8  : 0
MP 2184.9  : 0
MP 2184.10 : 0
MP 2184.11 : 0
MP 2184.12 : 0
MP 2184.13 : 0
MP 2184.14 : 0
MP 2184.15 : 0
MP 2184.16 : 0
MP 2184.17 : 0
MP 2184.18 : 0
MP 2184.19 : 0
MP 2184.20 : 0
MP 2184.21 : 0
MP 2184.22 : 0
MP 2184.23 : 0
             ;------------------------------------------------------------------
             ;MP2186 Speed to switch from simple to double PWM fundamental
             ;       frequency. Use only with HEIDENHAIN power moduls.
             ;       (only CC 424, CC 61xx)
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
MP 2186.16 : 0
MP 2186.17 : 0
MP 2186.18 : 0
MP 2186.19 : 0
MP 2186.20 : 0
MP 2186.21 : 0
MP 2186.22 : 0
MP 2186.23 : 0
             ;------------------------------------------------------------------
             ;MP2188 Speed to switch back from double to simple PWM
             ;       fundamental frequency. Necessary: MP2188.x < MP2186.x
             ;       Use only with HEIDENHAIN power moduls.
             ;       (only CC 424, CC 61xx)
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
MP 2188.16 : 0
MP 2188.17 : 0
MP 2188.18 : 0
MP 2188.19 : 0
MP 2188.20 : 0
MP 2188.21 : 0
MP 2188.22 : 0
MP 2188.23 : 0
             ;------------------------------------------------------------------
             ;MP2190 DC-link voltage of inverter
             ;       HEIDENHAIN inverter regenerative: 650 V
             ;       HEIDENHAIN inverter non-regenerative: 565 V
             ;Input: 0 to 3000 [V]
             ;       * = Value of table SUPPLY.SPY
MP 2190.0  : *
MP 2190.1  : 
MP 2190.2  : 
MP 2190.3  : 
MP 2190.4  : 
             ;------------------------------------------------------------------
             ;MP2192 Trigger threshold for LIFTOFF. Voltage in [%]
             ;       of the dc link voltage. If the dc link voltage is
             ;       below the trigger threshold an NC-Stop and LIFTOFF
             ;       will be executed.
             ;       Use only with regenerative HEIDENHAIN inverters.
             ;       (only CC 424)
             ;Input: 0 to 100 [%]
MP 2192    : 0
             ;------------------------------------------------------------------
             ;MP2194 Value of the dc link voltage as of which the spindle
             ;       will brake in case of a power failure. Use only
             ;       with regenerative HEIDENHAIN inverters.
             ;       (only CC 424)
             ;Input: 0 to 3000 [V]
MP 2194    : 0
             ;------------------------------------------------------------------
             ;MP2195 Suppress error messages from the
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
             ;        1 = Error message is suppressed, LIFTOFF not active
             ;Bit 8   with CC 61xx: "-PF.PS.DC" signal
             ;        0 = Error message is not suppressed
             ;        1 = Error message is suppressed
             ;Bit 9   with CC 61xx: Monitoring of dc-link voltage
             ;        0 = Monitoring of dc-link voltage, LIFTOFF active
             ;        1 = Monitoring of dc-link voltage, LIFTOFF not active
             ;Bit 10  with CC 61xx: Power monitoring of UV(R)
             ;        0 = Power monitoring active
             ;        1 = Power monitoring not active
             ;09876543210
MP 2195    : %00000000000
             ;------------------------------------------------------------------
             ;MP2196 Identifier of power supply unit in MP2198.x
             ;Input:   P, Q, R or T
MP 2196    : 
             ;------------------------------------------------------------------
             ;MP2198 Type of power supply unit
             ;Input:   Name from file <Supply.spy>
MP 2198.0  : 
MP 2198.1  : 
MP 2198.2  : 
MP 2198.3  : 
MP 2198.4  : 
             ;------------------------------------------------------------------
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
MP 2199.16 : 0
MP 2199.17 : 0
MP 2199.18 : 0
MP 2199.19 : 0
MP 2199.20 : 0
MP 2199.21 : 0
MP 2199.22 : 0
MP 2199.23 : 0
             ;------------------------------------------------------------------
             ;MP2200 Type of axis motors
             ;Input:   Name from file <Motor.mot>
MP 2200.0  : QSY096A
MP 2200.1  : QSY096A
MP 2200.2  : QSY096A
MP 2200.3  : QSY096A
MP 2200.4  : QSY096A
MP 2200.5  : QAN-3M-(Y/D-SPG)
MP 2200.6  : QAN-3M-(Y/D-SPG)
MP 2200.7  : QSY096A
MP 2200.8  : 
MP 2200.9  : 
MP 2200.10 : 
MP 2200.11 : 
MP 2200.12 : 
MP 2200.13 : 
MP 2200.14 : 
MP 2200.15 : 
MP 2200.16 : 
MP 2200.17 : 
MP 2200.18 : 
MP 2200.19 : QAN-3M-(Y/D-SPG);Spindel 1 - C axis mode
MP 2200.20 : QAN-3M-(Y/D-SPG);Spindel 1 - mode 0 (Y mode)
MP 2200.21 : QAN-3M-(Y/D-SPG);Spindel 1 - mode 1 (D mode)
MP 2200.22 : QAN-3M-(Y/D-SPG);Spindel 2 - mode 0 (Y mode)
MP 2200.23 : QAN-3M-(Y/D-SPG);Spindel 2 - mode 1 (D mode)
             ;------------------------------------------------------------------
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
MP 2202.16 : *
MP 2202.17 : *
MP 2202.18 : *
MP 2202.19 : *
MP 2202.20 : *
MP 2202.21 : *
MP 2202.22 : *
MP 2202.23 : *
             ;------------------------------------------------------------------
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
MP 2204.16 : *
MP 2204.17 : *
MP 2204.18 : *
MP 2204.19 : *
MP 2204.20 : *
MP 2204.21 : *
MP 2204.22 : *
MP 2204.23 : *
             ;------------------------------------------------------------------
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
             ;       13 to 18: reserved
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
MP 2206.16 : *
MP 2206.17 : *
MP 2206.18 : *
MP 2206.19 : *
MP 2206.20 : *
MP 2206.21 : *
MP 2206.22 : *
MP 2206.23 : *
             ;------------------------------------------------------------------
             ;MP2207 Overwrite "Type of temperature sensor" from the motor table
             ;Eingabe: * = Input from the motor table active
             ;         0 = KTY84-130
             ;         1 = PT1000
MP 2207.0  : *
MP 2207.1  : *
MP 2207.2  : *
MP 2207.3  : *
MP 2207.4  : *
MP 2207.5  : *
MP 2207.6  : *
MP 2207.7  : *
MP 2207.8  : *
MP 2207.9  : *
MP 2207.10 : *
MP 2207.11 : *
MP 2207.12 : *
MP 2207.13 : *
MP 2207.14 : *
MP 2207.15 : *
MP 2207.16 : *
MP 2207.17 : *
MP 2207.18 : *
MP 2207.19 : *
MP 2207.20 : *
MP 2207.21 : *
MP 2207.22 : *
MP 2207.23 : *
             ;------------------------------------------------------------------
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
MP 2208.16 : *
MP 2208.17 : *
MP 2208.18 : *
MP 2208.19 : *
MP 2208.20 : *
MP 2208.21 : *
MP 2208.22 : *
MP 2208.23 : *
             ;------------------------------------------------------------------
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
MP 2209.16 : *
MP 2209.17 : *
MP 2209.18 : *
MP 2209.19 : *
MP 2209.20 : *
MP 2209.21 : *
MP 2209.22 : *
MP 2209.23 : *
             ;------------------------------------------------------------------
             ;MP2210 Factor for reduction of the magnetisation current
             ;       (only CC 424, CC 61xx)
             ;Input: 0 to 99 [%]
             ;       0 = Function not active
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
MP 2210.16 : 0
MP 2210.17 : 0
MP 2210.18 : 0
MP 2210.19 : 0
MP 2210.20 : 0
MP 2210.21 : 0
MP 2210.22 : 0
MP 2210.23 : 0
             ;------------------------------------------------------------------
             ;MP2211 Reserved (only CC 424, CC 61xx)
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
MP 2211.16 : 0
MP 2211.17 : 0
MP 2211.18 : 0
MP 2211.19 : 0
MP 2211.20 : 0
MP 2211.21 : 0
MP 2211.22 : 0
MP 2211.23 : 0
             ;------------------------------------------------------------------
             ;MP2220 Monitoring functions
             ;Input: %xxxxxxxxxxxxxxxx
             ;Bit 0   Reference-mark monitoring
             ;        0 = Active
             ;        1 = Not active
             ;Bit 1   Monitoring of rotational direction
             ;        0 = Active
             ;        1 = Not active
             ;Bit 2   Power limiting of the spindle for -ERR.IZ.GR
             ;        (only for HEIDENHAIN inverters except UE 2xx)
             ;        0 = Active
             ;        1 = Not active
             ;Bit 3   Vibrations are suppressed during switch-off of
             ;        drive
             ;        0 = Active
             ;        1 = Not active
             ;Bit 4   only CC 424, CC 61xx:
             ;        Monitoring temperature too high
             ;        0 = Active
             ;        1 = Not active
             ;Bit 5   only CC 424, CC 61xx:
             ;        Monitoring temperature too low
             ;        0 = Active
             ;        1 = Not active
             ;Bit 6   reserved
             ;Bit 7   CC 422: reserved
             ;Bit 7   only CC 424, CC 61xx:
             ;        Monitoring input frequency encoder
             ;        0 = Active
             ;        1 = Not active
             ;Bit 8   only CC 424, CC 61xx:
             ;        Compensating the mechanical offset
             ;        by a step by step increasing of the kV
             ;        0 = Active
             ;        1 = Not active
             ;Bit 9   Reserved
             ;Bit 10  Reserved
             ;Bit 11  Reserved
             ;Bit 12  Reserved
             ;Bit 13  Reserved
             ;Bit 14  Reserved
             ;Bit 15  Reserved
             ;5432109876543210
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
MP 2220.16 : %0000000000000000
MP 2220.17 : %0000000000000000
MP 2220.18 : %0000000000000000
MP 2220.19 : %0000000000000000
MP 2220.20 : %0000000000000000
MP 2220.21 : %0000000000000000
MP 2220.22 : %0000000000000000
MP 2220.23 : %0000000000000000
             ;------------------------------------------------------------------
             ;MP2221 Special control functions
             ;Input: %xxxxxxxxxxxxxxxx
             ;Bit 0   Reserved
             ;Bit 1   Reserved
             ;Bit 2   Reserved
             ;Bit 3   Reserved
             ;Bit 4   Reserved
             ;Bit 5   Reserved
             ;Bit 6   Reserved
             ;Bit 7   Reduction of the switch-on time of the drive
             ;        0 = active
             ;        1 = not active (reduction of about 50 ms)
             ;Bit 8   reserviert
             ;Bit 9   reserviert
             ;Bit 10  only CC 61xx:
             ;        Handling of linear and synchronous motors to attain
             ;        higher milling power with AFC
             ;        0 = active
             ;        1 = not active
             ;Bit 11  Reserved
             ;Bit 12  Reserved
             ;Bit 13  Reserved
             ;Bit 14  Reserved
             ;Bit 15  Reserved
             ;5432109876543210
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
MP 2221.16 : %0000000000000000
MP 2221.17 : %0000000000000000
MP 2221.18 : %0000000000000000
MP 2221.19 : %0000000000000000
MP 2221.20 : %0000000000000000
MP 2221.21 : %0000000000000000
MP 2221.22 : %0000000000000000
MP 2221.23 : %0000000000000000
             ;------------------------------------------------------------------
             ;MP2222 Special control functions
             ;Input: %xxxxxxxxxxxxxxxx
             ;Bit 0   Reserved
             ;Bit 1   Reserved
             ;Bit 2   CC 61xx, EnDat 2.2:
             ;        Temperature signal not from EnDat 2.2 prorocol but
             ;        from KTY/PT1000 input
             ;        0 = inactive
             ;        1 = active
             ;Bit 3   Reserved
             ;Bit 4   Reserved
             ;Bit 5   Reserved
             ;Bit 6   Reserved
             ;Bit 7   Reserved
             ;Bit 8   Reserved
             ;Bit 9   Reserved
             ;Bit 10  Reserved
             ;Bit 11  Reserved
             ;Bit 12  Reserved
             ;Bit 13  Reserved
             ;Bit 14  Reserved
             ;Bit 15  Reserved
             ;5432109876543210
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
MP 2222.16 : %0000000000000000
MP 2222.17 : %0000000000000000
MP 2222.18 : %0000000000000000
MP 2222.19 : %0000000000000000
MP 2222.20 : %0000000000000000
MP 2222.21 : %0000000000000000
MP 2222.22 : %0000000000000000
MP 2222.23 : %0000000000000000
             ;------------------------------------------------------------------
             ;MP2223 Special control functions
             ;Input: %xxxxxxxxxxxxxxxx
             ;Bit 0   Reserved
             ;Bit 1   only CC 424, CC 61xx:
             ;        Automatic including of holding current in MP2630
             ;        by using PLC module 9128 and 9158
             ;        0 = Not active
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
             ;Bit 11  only CC 424, CC 61xx:
             ;        Switch-on of vertical and rotary axes with current that
             ;        was active at the time of switch-off
             ;        0 = Switch-on current like switch-off current
             ;        1 = Function not active
             ;Bit 12  Reserved
             ;Bit 13  Reserved
             ;Bit 14  Reserved
             ;Bit 15  Reserved
             ;5432109876543210
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
MP 2223.16 : %0000000000000000
MP 2223.17 : %0000000000000000
MP 2223.18 : %0000000000000000
MP 2223.19 : %0000000000000000
MP 2223.20 : %0000000000000000
MP 2223.21 : %0000000000000000
MP 2223.22 : %0000000000000000
MP 2223.23 : %0000000000000000
             ;------------------------------------------------------------------
             ;MP2230 Motor brake test: Factor for rated current
             ;Input:   0 to 30
             ;         0 = do not carry out test; or motor without motor brake
MP 2230.0  : 0
MP 2230.1  : 0
MP 2230.2  : 0
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
MP 2230.16 : 0
MP 2230.17 : 0
MP 2230.18 : 0
MP 2230.19 : 0
MP 2230.20 : 0
MP 2230.21 : 0
MP 2230.22 : 0
MP 2230.23 : 0
             ;------------------------------------------------------------------
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
MP 2232.16 : 0.1
MP 2232.17 : 0.1
MP 2232.18 : 0.1
MP 2232.19 : 0.1
MP 2232.20 : 0.1
MP 2232.21 : 0.1
MP 2232.22 : 0.1
MP 2232.23 : 0.1
             ;------------------------------------------------------------------
             ;MP2234 Internal activation of the motor brakes via the
             ;       PWM interface (not CC 61xx)
             ;Input: %xx
             ;Bit 0  0 = Signal is output
             ;       1 = Signal is not output
             ;Bit 1  reserved
             ;10
MP 2234.0  : %00
MP 2234.1  : %00
MP 2234.2  : %00
MP 2234.3  : %00
MP 2234.4  : %00
MP 2234.5  : %00
MP 2234.6  : %00
MP 2234.7  : %00
MP 2234.8  : %00
MP 2234.9  : %00
MP 2234.10 : %00
MP 2234.11 : %00
MP 2234.12 : %00
MP 2234.13 : %00
MP 2234.14 : %00
MP 2234.15 : %00
MP 2234.16 : %00
MP 2234.17 : %00
MP 2234.18 : %00
MP 2234.19 : %00
MP 2234.20 : %00
MP 2234.21 : %00
MP 2234.22 : %00
MP 2234.23 : %00
             ;------------------------------------------------------------------
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
MP 2236.16 : 10
MP 2236.17 : 10
MP 2236.18 : 10
MP 2236.19 : 10
MP 2236.20 : 10
MP 2236.21 : 10
MP 2236.22 : 10
MP 2236.23 : 10
             ;------------------------------------------------------------------
             ;MP2250 Determination of the field angle without movement of
             ;       the motor (only CC 424, CC 61xx)
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
MP 2250.16 : 0
MP 2250.17 : 0
MP 2250.18 : 0
MP 2250.19 : 0
MP 2250.20 : 0
MP 2250.21 : 0
MP 2250.22 : 0
MP 2250.23 : 0
             ;------------------------------------------------------------------
             ;MP2252 reserved (only CC 424, CC 61xx)
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
MP 2252.16 : 0
MP 2252.17 : 0
MP 2252.18 : 0
MP 2252.19 : 0
MP 2252.20 : 0
MP 2252.21 : 0
MP 2252.22 : 0
MP 2252.23 : 0
             ;------------------------------------------------------------------
             ;MP2254 Determination of the field angle
             ;Input:   0 = Field angle is determined during operation,
             ;             softkey has no function
             ;             (without plausibility check)
             ;         1 = Only CC 422: Determination of the field angle via
             ;             softkey, movement of the motor is allowed
             ;         2 = Only CC 424, CC 61xx:
             ;             Determination of the field angle via
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
MP 2254.16 : 0
MP 2254.17 : 0
MP 2254.18 : 0
MP 2254.19 : 0
MP 2254.20 : 0
MP 2254.21 : 0
MP 2254.22 : 0
MP 2254.23 : 0
             ;------------------------------------------------------------------
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
MP 2256.16 : +0
MP 2256.17 : +0
MP 2256.18 : +0
MP 2256.19 : +0
MP 2256.20 : +0
MP 2256.21 : +0
MP 2256.22 : +0
MP 2256.23 : +0
             ;------------------------------------------------------------------
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
MP 2257.16 : $00000000
MP 2257.17 : $00000000
MP 2257.18 : $00000000
MP 2257.19 : $00000000
MP 2257.20 : $00000000
MP 2257.21 : $00000000
MP 2257.22 : $00000000
MP 2257.23 : $00000000
             ;------------------------------------------------------------------
             ;MP2260 name for the torque-ripple-compensation file
             ;Input: [xx_<MotorNamefromMotorTable>.TRC]
             ;       no entry: not active
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
MP 2260.16 : 
MP 2260.17 : 
MP 2260.18 : 
MP 2260.19 : 
MP 2260.20 : 
MP 2260.21 : 
MP 2260.22 : 
MP 2260.23 : 
             ;------------------------------------------------------------------
             ;MP2261 Deactivate compensations
             ;       Bit 0: torque ripple compensation
             ;       Bit 1: gear error compensation
             ;Input: %0000000000000000
             ;       1 = compensation not active
             ;5432109876543210
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
MP 2261.16 : %0000000000000000
MP 2261.17 : %0000000000000000
MP 2261.18 : %0000000000000000
MP 2261.19 : %0000000000000000
MP 2261.20 : %0000000000000000
MP 2261.21 : %0000000000000000
MP 2261.22 : %0000000000000000
MP 2261.23 : %0000000000000000
             ;------------------------------------------------------------------
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
MP 2302.10 : 1          ;Spindel 1 - C-Achs Betrieb
MP 2302.11 : 1          ;Spindel 1 - Betriebsart 0 (Y-Mode)
MP 2302.12 : 1          ;Spindel 1 - Betriebsart 1 (D-Mode)
MP 2302.13 : 1          ;Spindel 2 - C-Achs Betrieb
MP 2302.14 : 1          ;Spindel 2 - Betriebsart 0 (Y-Mode)
MP 2302.15 : 1          ;Spindel 2 - Betriebsart 1 (D-Mode)
MP 2302.16 : 1
MP 2302.17 : 1
MP 2302.18 : 1
MP 2302.19 : 1
MP 2302.20 : 1
MP 2302.21 : 1
MP 2302.22 : 1
MP 2302.23 : 1
             ;------------------------------------------------------------------
             ;MP2304 Reference value for I2t monitoring of the power module
             ;Input: 0 to 1000.000 [factor]
             ;       0 = No monitoring
             ;     > 0 = Factor for rated current of the power module as referen
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
MP 2304.16 : 1
MP 2304.17 : 1
MP 2304.18 : 1
MP 2304.19 : 1
MP 2304.20 : 1
MP 2304.21 : 1
MP 2304.22 : 1
MP 2304.23 : 1
             ;------------------------------------------------------------------
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
MP 2308.16 : 0
MP 2308.17 : 0
MP 2308.18 : 0
MP 2308.19 : 0
MP 2308.20 : 0
MP 2308.21 : 0
MP 2308.22 : 0
MP 2308.23 : 0
             ;------------------------------------------------------------------
             ;MP2309 Time after switch-on of the controller in which the
             ;       parameters are adapted to active brakes. This is the
             ;       time from opening the break till finding the holding
             ;       current.
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
MP 2309.16 : 0
MP 2309.17 : 0
MP 2309.18 : 0
MP 2309.19 : 0
MP 2309.20 : 0
MP 2309.21 : 0
MP 2309.22 : 0
MP 2309.23 : 0
             ;------------------------------------------------------------------
             ;MP2312 Factor for utilization of the motors
             ;Input: 0 to 1000.000
             ;       0: Factor 1
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
MP 2312.16 : 1
MP 2312.17 : 1
MP 2312.18 : 1
MP 2312.19 : 1
MP 2312.20 : 1
MP 2312.21 : 1
MP 2312.22 : 1
MP 2312.23 : 1
             ;------------------------------------------------------------------
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
MP 2390.16 : 0
MP 2390.17 : 0
MP 2390.18 : 0
MP 2390.19 : 0
MP 2390.20 : 0
MP 2390.21 : 0
MP 2390.22 : 0
MP 2390.23 : 0
             ;------------------------------------------------------------------
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
MP 2392.16 : 0
MP 2392.17 : 0
MP 2392.18 : 0
MP 2392.19 : 0
MP 2392.20 : 0
MP 2392.21 : 0
MP 2392.22 : 0
MP 2392.23 : 0
             ;------------------------------------------------------------------
             ;MP2392 Maximum drive performance after PLC request
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
MP 2393.16 : 0
MP 2393.17 : 0
MP 2393.18 : 0
MP 2393.19 : 0
MP 2393.20 : 0
MP 2393.21 : 0
MP 2393.22 : 0
MP 2393.23 : 0
             ;------------------------------------------------------------------
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
MP 2394.16 : 0
MP 2394.17 : 0
MP 2394.18 : 0
MP 2394.19 : 0
MP 2394.20 : 0
MP 2394.21 : 0
MP 2394.22 : 0
MP 2394.23 : 0
             ;------------------------------------------------------------------
             ;MP2396 Maximum drive torque
             ;Input: 0.1 to 999999.9 [Nm]
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
MP 2396.16 : 0
MP 2396.17 : 0
MP 2396.18 : 0
MP 2396.19 : 0
MP 2396.20 : 0
MP 2396.21 : 0
MP 2396.22 : 0
MP 2396.23 : 0
             ;------------------------------------------------------------------
             ;MP2420 Proportional factor for the current controller
             ;Input: 0 to 9999.99 [V/A]
             ;       * = automatic calculation of P-Factor
MP 2420.0  : *
MP 2420.1  : *
MP 2420.2  : *
MP 2420.3  : *
MP 2420.4  : *
MP 2420.5  : *
MP 2420.6  : *
MP 2420.7  : *
MP 2420.8  : *
MP 2420.9  : *
MP 2420.10 : *          ;Spindel 1 - C-Achs Betrieb
MP 2420.11 : *          ;Spindel 1 - Betriebsart 0 (Y-Mode)
MP 2420.12 : *          ;Spindel 1 - Betriebsart 1 (D-Mode)
MP 2420.13 : *          ;Spindel 2 - C-Achs Betrieb
MP 2420.14 : *          ;Spindel 2 - Betriebsart 0 (Y-Mode)
MP 2420.15 : *          ;Spindel 2 - Betriebsart 1 (D-Mode)
MP 2420.16 : *
MP 2420.17 : *
MP 2420.18 : *
MP 2420.19 : *
MP 2420.20 : *
MP 2420.21 : *
MP 2420.22 : *
MP 2420.23 : *
             ;------------------------------------------------------------------
             ;MP2430 Integral-action factor for current controller
             ;Input: 0 to 9 999 999 [Vs/A]
             ;       * = automatic calculation of I-Factor
MP 2430.0  : *
MP 2430.1  : *
MP 2430.2  : *
MP 2430.3  : *
MP 2430.4  : *
MP 2430.5  : *
MP 2430.6  : *
MP 2430.7  : *
MP 2430.8  : *
MP 2430.9  : *
MP 2430.10 : *          ;Spindel 1 - C-Achs Betrieb
MP 2430.11 : *          ;Spindel 1 - Betriebsart 0 (Y-Mode)
MP 2430.12 : *          ;Spindel 1 - Betriebsart 1 (D-Mode)
MP 2430.13 : *          ;Spindel 2 - C-Achs Betrieb
MP 2430.14 : *          ;Spindel 2 - Betriebsart 0 (Y-Mode)
MP 2430.15 : *          ;Spindel 2 - Betriebsart 1 (D-Mode)
MP 2430.16 : *
MP 2430.17 : *
MP 2430.18 : *
MP 2430.19 : *
MP 2430.20 : *
MP 2430.21 : *
MP 2430.22 : *
MP 2430.23 : *
             ;------------------------------------------------------------------
             ;MP2440 with CC 61xx: Limit frequency of current controller
             ;       feed forward
             ;Input: 0,1 to 5000,0 [Hz]
             ;       0 = current controller feed forward not active
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
MP 2440.16 : 0
MP 2440.17 : 0
MP 2440.18 : 0
MP 2440.19 : 0
MP 2440.20 : 0
MP 2440.21 : 0
MP 2440.22 : 0
MP 2440.23 : 0
             ;------------------------------------------------------------------
             ;MP2450 with CC 61xx: Compensation of dead time
             ;Input: 0 or 1
             ;       0 = compensation not active
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
MP 2450.16 : 0
MP 2450.17 : 0
MP 2450.18 : 0
MP 2450.19 : 0
MP 2450.20 : 0
MP 2450.21 : 0
MP 2450.22 : 0
MP 2450.23 : 0
             ;------------------------------------------------------------------
             ;MP2500 Proportional factor of the speed controller
             ;Input: 0 to 1 000 000.000 [As]
MP 2500.0  : 1
MP 2500.1  : 1
MP 2500.2  : 1
MP 2500.3  : 1
MP 2500.4  : 1
MP 2500.5  : 1
MP 2500.6  : 1
MP 2500.7  : 1
MP 2500.8  : 1
MP 2500.9  : 1
MP 2500.10 : 1
MP 2500.11 : 1
MP 2500.12 : 1
MP 2500.13 : 1
MP 2500.14 : 1
MP 2500.15 : 1
MP 2500.16 : 1
MP 2500.17 : 1
MP 2500.18 : 1
MP 2500.19 : 7          ;Spindel 1 - C-Achs Betrieb
MP 2500.20 : 7          ;Spindel 1 - Betriebsart 0 (Y-Mode)
MP 2500.21 : 7          ;Spindel 1 - Betriebsart 1 (D-Mode)
MP 2500.22 : 7          ;Spindel 2 - Betriebsart 0 (Y-Mode)
MP 2500.23 : 7          ;Spindel 2 - Betriebsart 1 (D-Mode)
             ;------------------------------------------------------------------
             ;MP2510 Integral factor of the speed controller
             ;Input: 0 to 100 000 000 [A]
MP 2510.0  : 250
MP 2510.1  : 250
MP 2510.2  : 250
MP 2510.3  : 250
MP 2510.4  : 250
MP 2510.5  : 250
MP 2510.6  : 250
MP 2510.7  : 250
MP 2510.8  : 250
MP 2510.9  : 250
MP 2510.10 : 250
MP 2510.11 : 250
MP 2510.12 : 250
MP 2510.13 : 250
MP 2510.14 : 250
MP 2510.15 : 250
MP 2510.16 : 250
MP 2510.17 : 250
MP 2510.18 : 250
MP 2510.19 : 500        ;Spindel 1 - C-Achs Betrieb
MP 2510.20 : 500        ;Spindel 1 - Betriebsart 0 (Y-Mode)
MP 2510.21 : 500        ;Spindel 1 - Betriebsart 1 (D-Mode)
MP 2510.22 : 500        ;Spindel 2 - Betriebsart 0 (Y-Mode)
MP 2510.23 : 500        ;Spindel 2 - Betriebsart 1 (D-Mode)
             ;------------------------------------------------------------------
             ;MP2512 Integral-action factor limitation of the speed controller
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
MP 2512.16 : 0
MP 2512.17 : 0
MP 2512.18 : 0
MP 2512.19 : 0
MP 2512.20 : 0
MP 2512.21 : 0
MP 2512.22 : 0
MP 2512.23 : 0
             ;------------------------------------------------------------------
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
MP 2520.16 : 0
MP 2520.17 : 0
MP 2520.18 : 0
MP 2520.19 : 0
MP 2520.20 : 0
MP 2520.21 : 0
MP 2520.22 : 0
MP 2520.23 : 0
             ;------------------------------------------------------------------
             ;MP2530 PT2 element of speed controller
             ;Try compensation with low-pass filter (MP2560) first.
             ;Input: 0 to 1 [s]
MP 2530.0  : 0
MP 2530.1  : 0
MP 2530.2  : 0
MP 2530.3  : 0
MP 2530.4  : 0
MP 2530.5  : 0
MP 2530.6  : 0
MP 2530.7  : 0
MP 2530.8  : 0
MP 2530.9  : 0
MP 2530.10 : 0
MP 2530.11 : 0
MP 2530.12 : 0
MP 2530.13 : 0
MP 2530.14 : 0
MP 2530.15 : 0
MP 2530.16 : 0
MP 2530.17 : 0
MP 2530.18 : 0
MP 2530.19 : 0
MP 2530.20 : 0
MP 2530.21 : 0
MP 2530.22 : 0
MP 2530.23 : 0
             ;------------------------------------------------------------------
             ;MP2540 Band-rejection filter damping
             ;Input: 0 to 18,9 [dB]
MP 2540.0  : 0
MP 2540.1  : 0
MP 2540.2  : 0
MP 2540.3  : 0
MP 2540.4  : 0
MP 2540.5  : 0
MP 2540.6  : 0
MP 2540.7  : 0
MP 2540.8  : 0
MP 2540.9  : 0
MP 2540.10 : 0
MP 2540.11 : 0
MP 2540.12 : 0
MP 2540.13 : 0
MP 2540.14 : 0
MP 2540.15 : 0
MP 2540.16 : 0
MP 2540.17 : 0
MP 2540.18 : 0
MP 2540.19 : 0
MP 2540.20 : 0
MP 2540.21 : 0
MP 2540.22 : 0
MP 2540.23 : 0
             ;------------------------------------------------------------------
             ;MP2542 Band-rejection filter damping, filter 1
             ;       (only with CC 424, CC 61xx)
             ;Input: 0 to +100 [dB]
MP 2542.0  : +0
MP 2542.1  : +0
MP 2542.2  : +0
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
MP 2542.16 : +0
MP 2542.17 : +0
MP 2542.18 : +0
MP 2542.19 : +0
MP 2542.20 : +0
MP 2542.21 : +0
MP 2542.22 : +0
MP 2542.23 : +0
             ;------------------------------------------------------------------
             ;MP2543 Band-rejection filter damping, filter 2
             ;       (only with CC 424, CC 61xx)
             ;Input: 0 to +100 [dB]
MP 2543.0  : +0
MP 2543.1  : +0
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
MP 2543.16 : +0
MP 2543.17 : +0
MP 2543.18 : +0
MP 2543.19 : +0
MP 2543.20 : +0
MP 2543.21 : +0
MP 2543.22 : +0
MP 2543.23 : +0
             ;------------------------------------------------------------------
             ;MP2544 Band-rejection filter damping, filter 3
             ;       (only with CC 424, CC 61xx)
             ;Input: 0 to +100 [dB]
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
MP 2544.16 : +0
MP 2544.17 : +0
MP 2544.18 : +0
MP 2544.19 : +0
MP 2544.20 : +0
MP 2544.21 : +0
MP 2544.22 : +0
MP 2544.23 : +0
             ;------------------------------------------------------------------
             ;MP2545 Band-rejection filter damping, filter 4
             ;       (only with CC 424, CC 61xx)
             ;Input: 0 to +100 [dB]
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
MP 2545.16 : +0
MP 2545.17 : +0
MP 2545.18 : +0
MP 2545.19 : +0
MP 2545.20 : +0
MP 2545.21 : +0
MP 2545.22 : +0
MP 2545.23 : +0
             ;------------------------------------------------------------------
             ;MP2546 Band-rejection filter damping, filter 5
             ;       (only with CC 424, CC 61xx)
             ;Input: 0 to +100 [dB]
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
MP 2546.16 : +0
MP 2546.17 : +0
MP 2546.18 : +0
MP 2546.19 : +0
MP 2546.20 : +0
MP 2546.21 : +0
MP 2546.22 : +0
MP 2546.23 : +0
             ;------------------------------------------------------------------
             ;MP2547 Band-rejection filter damping, filter 6
             ;       (only with CC 424, CC 61xx)
             ;Input: 0 to +100 [dB]
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
MP 2547.16 : +0
MP 2547.17 : +0
MP 2547.18 : +0
MP 2547.19 : +0
MP 2547.20 : +0
MP 2547.21 : +0
MP 2547.22 : +0
MP 2547.23 : +0
             ;------------------------------------------------------------------
             ;MP2550 Band-rejection filter center frequency
             ;Input: 0 to 999,9 [Hz]
MP 2550.0  : 0
MP 2550.1  : 0
MP 2550.2  : 0
MP 2550.3  : 0
MP 2550.4  : 0
MP 2550.5  : 0
MP 2550.6  : 0
MP 2550.7  : 0
MP 2550.8  : 0
MP 2550.9  : 0
MP 2550.10 : 0
MP 2550.11 : 0
MP 2550.12 : 0
MP 2550.13 : 0
MP 2550.14 : 0
MP 2550.15 : 0
MP 2550.16 : 0
MP 2550.17 : 0
MP 2550.18 : 0
MP 2550.19 : 0
MP 2550.20 : 0
MP 2550.21 : 0
MP 2550.22 : 0
MP 2550.23 : 0
             ;------------------------------------------------------------------
             ;MP2552 Band-rejection filter center frequency, filter 1
             ;       (only with CC 424, CC 61xx)
             ;Input: 0 to 30 000,0 [Hz]
MP 2552.0  : 0
MP 2552.1  : 0
MP 2552.2  : 0
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
MP 2552.16 : 0
MP 2552.17 : 0
MP 2552.18 : 0
MP 2552.19 : 0
MP 2552.20 : 0
MP 2552.21 : 0
MP 2552.22 : 0
MP 2552.23 : 0
             ;------------------------------------------------------------------
             ;MP2553 Band-rejection filter center frequency, filter 2
             ;       (only with CC 424, CC 61xx)
             ;Input: 0 to 30 000,0 [Hz]
MP 2553.0  : 0
MP 2553.1  : 0
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
MP 2553.16 : 0
MP 2553.17 : 0
MP 2553.18 : 0
MP 2553.19 : 0
MP 2553.20 : 0
MP 2553.21 : 0
MP 2553.22 : 0
MP 2553.23 : 0
             ;------------------------------------------------------------------
             ;MP2554 Band-rejection filter center frequency, filter 3
             ;       (only with CC 424, CC 61xx)
             ;Input: 0 to 30 000,0 [Hz]
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
MP 2554.16 : 0
MP 2554.17 : 0
MP 2554.18 : 0
MP 2554.19 : 0
MP 2554.20 : 0
MP 2554.21 : 0
MP 2554.22 : 0
MP 2554.23 : 0
             ;------------------------------------------------------------------
             ;MP2555 Band-rejection filter center frequency, filter 4
             ;       (only with CC 424, CC 61xx)
             ;Input: 0 to 30 000,0 [Hz]
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
MP 2555.16 : 0
MP 2555.17 : 0
MP 2555.18 : 0
MP 2555.19 : 0
MP 2555.20 : 0
MP 2555.21 : 0
MP 2555.22 : 0
MP 2555.23 : 0
             ;------------------------------------------------------------------
             ;MP2556 Band-rejection filter center frequency, filter 5
             ;       (only with CC 424, CC 61xx)
             ;Input: 0 to 30 000,0 [Hz]
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
MP 2556.16 : 0
MP 2556.17 : 0
MP 2556.18 : 0
MP 2556.19 : 0
MP 2556.20 : 0
MP 2556.21 : 0
MP 2556.22 : 0
MP 2556.23 : 0
             ;------------------------------------------------------------------
             ;MP2557 Band-rejection filter center frequency, filter 6
             ;       (only with CC 424, CC 61xx)
             ;Input: 0 to 30 000,0 [Hz]
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
MP 2557.16 : 0
MP 2557.17 : 0
MP 2557.18 : 0
MP 2557.19 : 0
MP 2557.20 : 0
MP 2557.21 : 0
MP 2557.22 : 0
MP 2557.23 : 0
             ;------------------------------------------------------------------
             ;MP2560 Low-pass filter
             ;CC 422: Compensation of high-frequency interference oscillations
             ;        (>600 Hz), if no satisfactory results: use PT2 element
             ;        (MP2530)
             ;        Input: 0 = No low-pass filter
             ;               1 = 1st order low-pass filter (600..700 Hz)
             ;               2 = 2nd order low-pass filter (>700 Hz)
             ;CC 424, CC 61xx: Filter order of the low-pass filter
             ;        Input:   0 to 20
             ;        Recommended input value: 0
             ;        Recommended input for high current noise of
             ;        high frequency spindles: 10 to 20
MP 2560.0  : 0
MP 2560.1  : 0
MP 2560.2  : 0
MP 2560.3  : 0
MP 2560.4  : 0
MP 2560.5  : 0
MP 2560.6  : 0
MP 2560.7  : 0
MP 2560.8  : 0
MP 2560.9  : 0
MP 2560.10 : 0
MP 2560.11 : 0
MP 2560.12 : 0
MP 2560.13 : 0
MP 2560.14 : 0
MP 2560.15 : 0
MP 2560.16 : 0
MP 2560.17 : 0
MP 2560.18 : 0
MP 2560.19 : 0
MP 2560.20 : 0
MP 2560.21 : 0
MP 2560.22 : 0
MP 2560.23 : 0
             ;------------------------------------------------------------------
             ;MP2561 Maximum motor encoder input frequency
             ;       (only with CC 61xx)
             ;Input: %xxxxxxxxxxxxxxxx
             ;Bit 0   0 = 400 kHz
             ;        1 = 800 kHz
             ;5432109876543210
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
MP 2561.16 : %0000000000000000
MP 2561.17 : %0000000000000000
MP 2561.18 : %0000000000000000
MP 2561.19 : %0000000000000000
MP 2561.20 : %0000000000000000
MP 2561.21 : %0000000000000000
MP 2561.22 : %0000000000000000
MP 2561.23 : %0000000000000000
             ;------------------------------------------------------------------
             ;MP2562 Filter type for filter 1
             ;       (only with CC 424, CC 61xx)
             ;Input: 0 = No filter
             ;       1 = PT2 low-pass filter (speed controller)
             ;       2 = Band-rejection filter (speed controller)
             ;       3 = Phase lifting filter (speed controller)
             ;       11 = PT2 low-pass filter (position controller)
             ;       12 = Band-rejection filter (position controller)
             ;       13 = Phase lifting filter (position controller)
MP 2562.0  : 0
MP 2562.1  : 0
MP 2562.2  : 0
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
MP 2562.16 : 0
MP 2562.17 : 0
MP 2562.18 : 0
MP 2562.19 : 0
MP 2562.20 : 0
MP 2562.21 : 0
MP 2562.22 : 0
MP 2562.23 : 0
             ;------------------------------------------------------------------
             ;MP2563 Filter type for filter 2
             ;       (only with CC 424, CC 61xx)
             ;Input: 0 = No filter
             ;       1 = PT2 low-pass filter (speed controller)
             ;       2 = Band-rejection filter (speed controller)
             ;       3 = Phase lifting filter (speed controller)
             ;       11 = PT2 low-pass filter (position controller)
             ;       12 = Band-rejection filter (position controller)
             ;       13 = Phase lifting filter (position controller)
MP 2563.0  : 0
MP 2563.1  : 0
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
MP 2563.16 : 0
MP 2563.17 : 0
MP 2563.18 : 0
MP 2563.19 : 0
MP 2563.20 : 0
MP 2563.21 : 0
MP 2563.22 : 0
MP 2563.23 : 0
             ;------------------------------------------------------------------
             ;MP2564 Filter type for filter 3
             ;       (only with CC 424, CC 61xx)
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
MP 2564.16 : 0
MP 2564.17 : 0
MP 2564.18 : 0
MP 2564.19 : 0
MP 2564.20 : 0
MP 2564.21 : 0
MP 2564.22 : 0
MP 2564.23 : 0
             ;------------------------------------------------------------------
             ;MP2565 Filter type for filter 4
             ;       (only with CC 424, CC 61xx)
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
MP 2565.16 : 0
MP 2565.17 : 0
MP 2565.18 : 0
MP 2565.19 : 0
MP 2565.20 : 0
MP 2565.21 : 0
MP 2565.22 : 0
MP 2565.23 : 0
             ;------------------------------------------------------------------
             ;MP2566 Filter type for filter 5
             ;       (only with CC 424, CC 61xx)
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
MP 2566.16 : 0
MP 2566.17 : 0
MP 2566.18 : 0
MP 2566.19 : 0
MP 2566.20 : 0
MP 2566.21 : 0
MP 2566.22 : 0
MP 2566.23 : 0
             ;------------------------------------------------------------------
             ;MP2567 Filter type for filter 6
             ;       (only with CC 424, CC 61xx)
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
MP 2567.16 : 0
MP 2567.17 : 0
MP 2567.18 : 0
MP 2567.19 : 0
MP 2567.20 : 0
MP 2567.21 : 0
MP 2567.22 : 0
MP 2567.23 : 0
             ;------------------------------------------------------------------
             ;MP2572 Band-rejection filter bandwidth for filter 1
             ;       (only with CC 424, CC 61xx)
             ;Input: 0 to 30 000,0 [Hz]
MP 2572.0  : 0
MP 2572.1  : 0
MP 2572.2  : 0
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
MP 2572.16 : 0
MP 2572.17 : 0
MP 2572.18 : 0
MP 2572.19 : 0
MP 2572.20 : 0
MP 2572.21 : 0
MP 2572.22 : 0
MP 2572.23 : 0
             ;------------------------------------------------------------------
             ;MP2573 Band-rejection filter bandwidth for filter 2
             ;       (only with CC 424, CC 61xx)
             ;Input: 0 to 30 000,0 [Hz]
MP 2573.0  : 0
MP 2573.1  : 0
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
MP 2573.16 : 0
MP 2573.17 : 0
MP 2573.18 : 0
MP 2573.19 : 0
MP 2573.20 : 0
MP 2573.21 : 0
MP 2573.22 : 0
MP 2573.23 : 0
             ;------------------------------------------------------------------
             ;MP2574 Band-rejection filter bandwidth for filter 3
             ;       (only with CC 424, CC 61xx)
             ;Input: 0 to 30 000,0 [Hz]
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
MP 2574.16 : 0
MP 2574.17 : 0
MP 2574.18 : 0
MP 2574.19 : 0
MP 2574.20 : 0
MP 2574.21 : 0
MP 2574.22 : 0
MP 2574.23 : 0
             ;------------------------------------------------------------------
             ;MP2575 Band-rejection filter bandwidth for filter 4
             ;       (only with CC 424, CC 61xx)
             ;Input: 0 to 30 000,0 [Hz]
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
MP 2575.16 : 0
MP 2575.17 : 0
MP 2575.18 : 0
MP 2575.19 : 0
MP 2575.20 : 0
MP 2575.21 : 0
MP 2575.22 : 0
MP 2575.23 : 0
             ;------------------------------------------------------------------
             ;MP2576 Band-rejection filter bandwidth for filter 5
             ;       (only with CC 424, CC 61xx)
             ;Input: 0 to 30 000,0 [Hz]
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
MP 2576.16 : 0
MP 2576.17 : 0
MP 2576.18 : 0
MP 2576.19 : 0
MP 2576.20 : 0
MP 2576.21 : 0
MP 2576.22 : 0
MP 2576.23 : 0
             ;------------------------------------------------------------------
             ;MP2577 Band-rejection filter bandwidth for filter 6
             ;       (only with CC 424, CC 61xx)
             ;Input: 0 to 30 000,0 [Hz]
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
MP 2577.16 : 0
MP 2577.17 : 0
MP 2577.18 : 0
MP 2577.19 : 0
MP 2577.20 : 0
MP 2577.21 : 0
MP 2577.22 : 0
MP 2577.23 : 0
             ;------------------------------------------------------------------
             ;MP2590 Ramp for braking in an EMERGENCY STOP
             ;Input: 0,001 to 999,999 [rev/min/ms]
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
MP 2590.16 : 0
MP 2590.17 : 0
MP 2590.18 : 0
MP 2590.19 : 0
MP 2590.20 : 0
MP 2590.21 : 0
MP 2590.22 : 0
MP 2590.23 : 0
             ;------------------------------------------------------------------
             ;MP2600 Acceleration feedforward control
             ;Input: 0 to 1000 [A/(rev/s^2)]
MP 2600.0  : 0
MP 2600.1  : 0
MP 2600.2  : 0
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
MP 2600.16 : 0
MP 2600.17 : 0
MP 2600.18 : 0
MP 2600.19 : 0
MP 2600.20 : 0
MP 2600.21 : 0
MP 2600.22 : 0
MP 2600.23 : 0
             ;------------------------------------------------------------------
             ;MP2602 IPC time constant T 1
             ;Input: 0 to 1,0000 [s]
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
MP 2602.16 : 0
MP 2602.17 : 0
MP 2602.18 : 0
MP 2602.19 : 0
MP 2602.20 : 0
MP 2602.21 : 0
MP 2602.22 : 0
MP 2602.23 : 0
             ;------------------------------------------------------------------
             ;MP2604 IPC time constant T 2
             ;Input: 0 to 1,0000 [s]
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
MP 2604.16 : 0
MP 2604.17 : 0
MP 2604.18 : 0
MP 2604.19 : 0
MP 2604.20 : 0
MP 2604.21 : 0
MP 2604.22 : 0
MP 2604.23 : 0
             ;------------------------------------------------------------------
             ;MP2606 Following error during jerk
             ;Input CC 422: 0,000 to 10,000
             ;      CC 424, CC61xx: 0,000 to 0,800
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
MP 2606.16 : 0
MP 2606.17 : 0
MP 2606.18 : 0
MP 2606.19 : 0
MP 2606.20 : 0
MP 2606.21 : 0
MP 2606.22 : 0
MP 2606.23 : 0
             ;------------------------------------------------------------------
             ;MP2607 Damping factor for active damping of low-frequency
             ;       oscillations (not CC 61xx)
             ;Input: 0 to 30,000
             ;       0: No damping
             ;       1,5: Typical damping factor
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
MP 2607.16 : 0
MP 2607.17 : 0
MP 2607.18 : 0
MP 2607.19 : 0
MP 2607.20 : 0
MP 2607.21 : 0
MP 2607.22 : 0
MP 2607.23 : 0
             ;------------------------------------------------------------------
             ;MP2608 Damping time constant for active damping
             ;       of low-frequency oscillations (not CC 61xx)
             ;Input: 0 to 0,9999 [s]
             ;       0: No damping
             ;       0,005 to 0,02: Typical damping time constant
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
MP 2608.16 : 0
MP 2608.17 : 0
MP 2608.18 : 0
MP 2608.19 : 0
MP 2608.20 : 0
MP 2608.21 : 0
MP 2608.22 : 0
MP 2608.23 : 0
             ;------------------------------------------------------------------
             ;MP2609 Separation frequency for Crossover Position Filter
             ;       (CPF) for the passive damping of machine resonances,
             ;       can be used for drive motors with speed and position
             ;       encoder (2-encoder systems), (only with CC 61xx).
             ;Input: 0 to 300,0000 [Hz]
             ;       0: CPF inactive
             ;       10,0000 to 100,0000: Typical frequency range
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
MP 2609.16 : 0
MP 2609.17 : 0
MP 2609.18 : 0
MP 2609.19 : 0
MP 2609.20 : 0
MP 2609.21 : 0
MP 2609.22 : 0
MP 2609.23 : 0
             ;------------------------------------------------------------------
             ;MP2610 Friction compensation at approx. 10 rpm
             ;Input: 0 to 100 [A]
MP 2610.0  : 0
MP 2610.1  : 0
MP 2610.2  : 0
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
MP 2610.16 : 0
MP 2610.17 : 0
MP 2610.18 : 0
MP 2610.19 : 0
MP 2610.20 : 0
MP 2610.21 : 0
MP 2610.22 : 0
MP 2610.23 : 0
             ;------------------------------------------------------------------
             ;MP2612 Delay of friction compensation
             ;Input: 0 to 1 [s], if MP2614 = 0
             ;       0 to 1 [mm], if MP2614 != 0
MP 2612.0  : 0
MP 2612.1  : 0
MP 2612.2  : 0
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
MP 2612.16 : 0
MP 2612.17 : 0
MP 2612.18 : 0
MP 2612.19 : 0
MP 2612.20 : 0
MP 2612.21 : 0
MP 2612.22 : 0
MP 2612.23 : 0
             ;------------------------------------------------------------------
             ;MP2614 Dynamic friction compensation
             ;       (only with CC 424, CC 61xx)
             ;Input: 0 to 100.000 [mm]
MP 2614.0  : 0
MP 2614.1  : 0
MP 2614.2  : 0
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
MP 2614.16 : 0
MP 2614.17 : 0
MP 2614.18 : 0
MP 2614.19 : 0
MP 2614.20 : 0
MP 2614.21 : 0
MP 2614.22 : 0
MP 2614.23 : 0
             ;------------------------------------------------------------------
             ;MP2620 Friction compensation at rated speed
             ;Input: 0 to 100.000 [A]
MP 2620.0  : 0
MP 2620.1  : 0
MP 2620.2  : 0
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
MP 2620.16 : 0
MP 2620.17 : 0
MP 2620.18 : 0
MP 2620.19 : 0
MP 2620.20 : 0
MP 2620.21 : 0
MP 2620.22 : 0
MP 2620.23 : 0
             ;------------------------------------------------------------------
             ;MP2630 Holding current for vertical axis (weight compensation)
             ;Input: -100,000 to +100,000 [A]
MP 2630.0  : +0
MP 2630.1  : +0
MP 2630.2  : +0
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
MP 2630.16 : +0
MP 2630.17 : +0
MP 2630.18 : +0
MP 2630.19 : +0
MP 2630.20 : +0
MP 2630.21 : +0
MP 2630.22 : +0
MP 2630.23 : +0
             ;------------------------------------------------------------------
             ;MP2640 Torsion-Compensation between position encoder and
             ;       speed encoder
             ;Input: 0,001 to 100,000 [um/A]
             ;       0 = inactive
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
MP 2640.16 : 0
MP 2640.17 : 0
MP 2640.18 : 0
MP 2640.19 : 0
MP 2640.20 : 0
MP 2640.21 : 0
MP 2640.22 : 0
MP 2640.23 : 0
             ;------------------------------------------------------------------
             ;MP2690 Limitation of compensation values for backlash,
             ;       static friction and reversal error
             ;Input: 0 to 1,0000 [seconds]
             ;         0 = not active
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
MP 2690.16 : 0
MP 2690.17 : 0
MP 2690.18 : 0
MP 2690.19 : 0
MP 2690.20 : 0
MP 2690.21 : 0
MP 2690.22 : 0
MP 2690.23 : 0
             ;------------------------------------------------------------------
             ;MP2700 Axis specific activation of additional
             ;       compensations (software options)
             ;Input: %xxxxxxxxxxxxxxxxxx
             ;Bit 0   CTC compensation (option 141)
             ;        0 = inactive
             ;        1 = active
             ;Bit 1   PAC compensation (option 142)
             ;        0 = inactive
             ;        1 = active
             ;Bit 2   LAC compensation (option 143)
             ;        0 = inactive
             ;        1 = active
             ;Bit 3   MAC compensation (Option 144)
             ;        0 = inactive
             ;        1 = active
             ;Bit 4   ACC compensation (Option 145)
             ;        0 = inactive
             ;        1 = active
             ;Bit 5   AVD compensation (Option 146)
             ;        0 = inactive
             ;        1 = active
             ;Bit 6..15 reserved
             ;5432109876543210
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
MP 2700.16 : %0000000000000000
MP 2700.17 : %0000000000000000
MP 2700.18 : %0000000000000000
MP 2700.19 : %0000000000000000
MP 2700.20 : %0000000000000000
MP 2700.21 : %0000000000000000
MP 2700.22 : %0000000000000000
MP 2700.23 : %0000000000000000
             ;------------------------------------------------------------------
             ;MP2710 ACC: Selection of principal of function of the
             ;       automatic mode (Only valid if bit 4 is set in MP2700)
             ;Eingabe: 0 = Robust standard mode with an average application
             ;             of a signal for damping the chatter
             ;             (recommended)
             ;         1 = Chatter damping is increased for the selected
             ;             axis; this can sometimes be advisable for one
             ;             or two axes (e.g. vertical axes) and results in
             ;             slightly reduced control accuracy
             ;             (following error increases slightly)
             ;         2 = Chatter damping is significantly increased for
             ;             the selected axis; this is usually advisable
             ;             for no more than one axis and results in
             ;             reduced control accuracy
             ;             (following error increases)
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
MP 2710.16 : 0
MP 2710.17 : 0
MP 2710.18 : 0
MP 2710.19 : 0
MP 2710.20 : 0
MP 2710.21 : 0
MP 2710.22 : 0
MP 2710.23 : 0
             ;------------------------------------------------------------------
             ;MP2720 AVD: Indexing different compensation files
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
MP 2720.16 : 0
MP 2720.17 : 0
MP 2720.18 : 0
MP 2720.19 : 0
MP 2720.20 : 0
MP 2720.21 : 0
MP 2720.22 : 0
MP 2720.23 : 0
             ;------------------------------------------------------------------
             ;MP2800 reserved
             ;Input: %0000000000000000
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
MP 2800.16 : %0000000000000000
MP 2800.17 : %0000000000000000
MP 2800.18 : %0000000000000000
MP 2800.19 : %0000000000000000
MP 2800.20 : %0000000000000000
MP 2800.21 : %0000000000000000
MP 2800.22 : %0000000000000000
MP 2800.23 : %0000000000000000
             ;------------------------------------------------------------------
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
MP 2900.16 : +0
MP 2900.17 : +0
MP 2900.18 : +0
MP 2900.19 : +0
MP 2900.20 : +0
MP 2900.21 : +0
MP 2900.22 : +0
MP 2900.23 : +0
             ;------------------------------------------------------------------
             ;MP2910 Gain for the torque controller of the master-slave
             ;       torque control
             ;Input: 0.001 to 999.999 [1/(Nm*min)]
             ;       (entry under slave axis)
MP 2910.0  : 0.001
MP 2910.1  : 0.001
MP 2910.2  : 0.001
MP 2910.3  : 0.001
MP 2910.4  : 0.001
MP 2910.5  : 0.001
MP 2910.6  : 0.001
MP 2910.7  : 0.001
MP 2910.8  : 0.001
MP 2910.9  : 0.001
MP 2910.10 : 0.001
MP 2910.11 : 0.001
MP 2910.12 : 0.001
MP 2910.13 : 0.001
MP 2910.14 : 0.001
MP 2910.15 : 0.001
MP 2910.16 : 0.001
MP 2910.17 : 0.001
MP 2910.18 : 0.001
MP 2910.19 : 0.001
MP 2910.20 : 0.001
MP 2910.21 : 0.001
MP 2910.22 : 0.001
MP 2910.23 : 0.001
             ;------------------------------------------------------------------
             ;MP2912 with CC 61xx: Configuration of the master-slave
             ;       torque control
             ;Input: %xx
             ;Bit 0  Application of speed compensation value
             ;       0 = speed compensation value applied to master
             ;           and slave
             ;       1 = speed compensation value applied only to slave
             ;Bit 1  Feedforward control of master-slave torque control
             ;       0 = not active
             ;       1 = active
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
MP 2912.16 : %00
MP 2912.17 : %00
MP 2912.18 : %00
MP 2912.19 : %00
MP 2912.20 : %00
MP 2912.21 : %00
MP 2912.22 : %00
MP 2912.23 : %00
             ;-------------------------------------------------------------
             ;MP2920 Factor for variable torque distribution of the
             ;       master-slave torque control
             ;Input: 0.000 to 100.000
             ;       (entry under slave axis)
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
MP 2920.16 : 0
MP 2920.17 : 0
MP 2920.18 : 0
MP 2920.19 : 0
MP 2920.20 : 0
MP 2920.21 : 0
MP 2920.22 : 0
MP 2920.23 : 0
             ;-------------------------------------------------------------
             ;MP2930 Factor for variable speed compensation of the
             ;       master-slave torque control (not CC 61xx)
             ;Input: -100.00 to 100.00 [%]
             ;       (entry under slave axis)
MP 2930.0  : +0
MP 2930.1  : +0
MP 2930.2  : +0
MP 2930.3  : +0
MP 2930.4  : +0
MP 2930.5  : +0
MP 2930.6  : +0
MP 2930.7  : +0
MP 2930.8  : +0
MP 2930.9  : +0
MP 2930.10 : +0
MP 2930.11 : +0
MP 2930.12 : +0
MP 2930.13 : +0
MP 2930.14 : +0
MP 2930.15 : +0
MP 2930.16 : +0
MP 2930.17 : +0
MP 2930.18 : +0
MP 2930.19 : +0
MP 2930.20 : +0
MP 2930.21 : +0
MP 2930.22 : +0
MP 2930.23 : +0
             ;-------------------------------------------------------------
             ;MP2990 Reserved
             ;Input: 0
MP 2990.0  : 0
MP 2990.1  : 0
MP 2990.2  : 0
MP 2990.3  : 0
MP 2990.4  : 0
MP 2990.5  : 0
             ;==================================================================
             ;MP 3000 .. 3999
             ;Spindle control (1st spindle)
             ;==================================================================
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
MP 3010    : 6
             ;------------------------------------------------------------------
             ;MP3011 Function of analog output s if MP3010 < 3
             ;Input: 0 = No special function (0 volt)
             ;       1 = Voltage proportional to the current machining feed
             ;           rate (depending on MP3012)
             ;       2 = Voltage is defined via the PLC module 9130
             ;       3 = Voltage is defined through M functions
             ;           (M200 to M204)
MP 3011    : 0
             ;------------------------------------------------------------------
             ;MP3012 Feed rate from output of an analog voltage of 10 V
             ;(MP3011 = 1)
             ;Input: 0 to 300 000 [mm/min]
             ;Laser power control M200-M204
MP 3012    : 0          ;0 to 30 000 m/min = 10 volts
             ;------------------------------------------------------------------
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
             ;------------------------------------------------------------------
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
             ;------------------------------------------------------------------
             ;MP3020 Definition of speed range of BCD spindle
             ;Input: 0 to 99 999
             ;Limitation of speed codes (no limitation with 01991)
             ;    1* increment (1..9)
             ;+  10* max. code (1..99)
             ;+1000* min. code (1..99)
MP 3020    : 1991
             ;------------------------------------------------------------------
             ;MP3030 Axis stop with a Tool Call in which only a
             ;spindle speed is transferred
             ;Input: %xx
             ;Bit 0   Axis stops with Tool Call
             ;        0 = Active
             ;        1 = Not active
             ;Bit 1   Regulate to speed 0 when switching to another
             ;        gear range
             ;        0 = Active
             ;        1 = Not active
             ;10
MP 3030    : %01
             ;------------------------------------------------------------------
             ;MP3120 Speed 0 permitted
             ;Input: 0: S = 0 permitted
             ;       1: S = 0 not permitted
MP 3120    : 1
             ;------------------------------------------------------------------
             ;MP3130 Polarity of speed setpoint for the spindle
             ;Input: 0 = M03 positive voltage; M04 negative voltage
             ;       1 = M03 negative voltage; M04 positive voltage
             ;       2 = M03 and M04 positive voltage
             ;       3 = M03 and M04 negative voltage
MP 3130    : 0
             ;------------------------------------------------------------------
             ;MP3140 Counting direction of the encoder signals for the spindle
             ;Input: 0 = Positive counting direction with M03
             ;       1 = Negative counting direction with M03
MP 3140    : 0
             ;------------------------------------------------------------------
             ;MP3142 Line count of the rotary encoder on the spindle
             ;Input: 100 to 100000 [lines]
MP 3142    : 2048
             ;------------------------------------------------------------------
             ;MP3143 Mounting configuration of the spindle position encoder
             ;Input: 0 = Direct
             ;       1 = Transmission (reference pulse from input X30)
             ;       2 = Transmission (reference end position of input X30 and
             ;           reference pulse from encoder, not possible if
             ;           transmission is located between ROD and spindle)
             ;       3 = Transmission (2nd reference pulse from input X30)
             ;       4 = Referencing via EnDat (not possible if
             ;           transmission is located between ROD and spindle)
             ;       5 = like input = 0, but evaluation of transmission
MP 3143    : 0
             ;------------------------------------------------------------------
             ;MP3210 S digital: Motor revolutions at rated speed
             ;Input: 0 to 100 [1000/min]
             ;
             ;       S analog : S analog voltage at rated speed
             ;Input: 0 to 9.999 [V]
MP 3210.0  : 5          ;1st gear range
MP 3210.1  : 10         ;2nd gear range
MP 3210.2  : 0          ;3rd gear range
MP 3210.3  : 0          ;4th gear range
MP 3210.4  : 0          ;5th gear range
MP 3210.5  : 0          ;6th gear range
MP 3210.6  : 0          ;7th gear range
MP 3210.7  : 0          ;8th gear range
             ;------------------------------------------------------------------
             ;MP3240.1 S digital: Minimum possible output speed of motor
             ;Input: 0 to 9.999 [1000/min]
             ;
             ;       S analog : Minimum possible S analog output voltage
             ;Input: 0 to 9.999 [V]
MP 3240.1  : 0.05
             ;------------------------------------------------------------------
             ;MP3240.2 S digital: Motor speed for gear changing
             ;Input: 0 to 9.999 [1000/min]
             ;
             ;       S analog : Jog voltage for gear changing
             ;Input: 0 to 9.999 [V]
MP 3240.2  : 0.01
             ;------------------------------------------------------------------
             ;MP3310 S override limitation
             ;Input: 0 to 100 [%]
MP 3310.0  : 130        ;Maximum
MP 3310.1  : 50         ;Minimum
             ;------------------------------------------------------------------
             ;MP3350 Maximum permissible overshoot of the spindle rpm for
             ;       monitoring the spindle rpm, as a percentage of the
             ;       nominal speed value
             ;Input: 0 to 100 [%]
MP 3350    : 0
             ;------------------------------------------------------------------
             ;MP3351 Entry of an absolute value for the permissible
             ;       overshoot of the rpm. This value is only used if the
             ;       absolute value resulting from MP3350 is less than the
             ;       the value entered for MP3351. (for low nominal speeds)
             ;Input: 0,001 to 100000,000 [rpm]
             ;       0 = Monitoring deactivated
MP 3351    : 0
             ;------------------------------------------------------------------
             ;MP3411 Ramp gradient of spindle
             ;Input: S digital: 0 to 1.999 [(1000/min)/ms]
             ;       S analog : 0 to 1.999 [V/ms]
MP 3411.0  : 0.001
MP 3411.1  : 0.001
MP 3411.2  : 0.001
MP 3411.3  : 0.001
MP 3411.4  : 0.001
MP 3411.5  : 0.001
MP 3411.6  : 0.001
MP 3411.7  : 0.001
             ;------------------------------------------------------------------
             ;MP3412 Multiplier for MP3411
             ;Input: 0.000 to 1.999
MP 3412.0  : 1          ;With M05
MP 3412.1  : 1          ;For oriented spindle stop
MP 3412.2  : 1          ;For tapping
MP 3412.3  : 1          ;For rigid tapping
             ;------------------------------------------------------------------
             ;MP3415 Spindle overshoot behavior
             ;Input: 0 to 1.000 [ms]
MP 3415.0  : 100        ;With M03, M04, M05
MP 3415.1  : 100        ;For oriented spindle stop
MP 3415.2  : 100        ;For tapping
MP 3415.3  : 100        ;For rigid tapping
             ;------------------------------------------------------------------
             ;MP3420 Positioning window for the spindle
             ;Input: 0 to 360 [degrees]
MP 3420    : 0.05
             ;------------------------------------------------------------------
             ;MP3430 Deviation of the reference mark from the desired
             ;position (spindle preset)
             ;Input: 0 to 360 [degrees]
MP 3430    : 0
             ;------------------------------------------------------------------
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
             ;------------------------------------------------------------------
             ;MP3450 Spindle motor revolutions
             ;Input: 0 to 65535 [rpm]
             ;       0 no transmission
MP 3450.0  : 0          ;1st gear range
MP 3450.1  : 0          ;2nd gear range
MP 3450.2  : 0          ;3rd gear range
MP 3450.3  : 0          ;4th gear range
MP 3450.4  : 0          ;5th gear range
MP 3450.5  : 0          ;6th gear range
MP 3450.6  : 0          ;7th gear range
MP 3450.7  : 0          ;8th gear range
             ;------------------------------------------------------------------
             ;MP3451 Spindle revolutions
             ;Input: 0 to 65535 [rpm]
             ;       0 no transmission
MP 3451.0  : 0          ;1st gear range
MP 3451.1  : 0          ;2nd gear range
MP 3451.2  : 0          ;3rd gear range
MP 3451.3  : 0          ;4th gear range
MP 3451.4  : 0          ;5th gear range
MP 3451.5  : 0          ;6th gear range
MP 3451.6  : 0          ;7th gear range
MP 3451.7  : 0          ;8th gear range
             ;------------------------------------------------------------------
             ;MP3510 Rated speed for gear ranges
             ;Input: 0 to 999 999.999 [rpm]
MP 3510.0  : 5000       ;1st gear range
MP 3510.1  : 100000     ;2nd gear range
MP 3510.2  : 0          ;3rd gear range
MP 3510.3  : 0          ;4th gear range
MP 3510.4  : 0          ;5th gear range
MP 3510.5  : 0          ;6th gear range
MP 3510.6  : 0          ;7th gear range
MP 3510.7  : 0          ;8th gear range
             ;------------------------------------------------------------------
             ;MP3515 Maximum spindle speed
             ;Input: 0 to 999 999.999 [rpm]
MP 3515.0  : 5000       ;1st gear range
MP 3515.1  : 100000     ;2nd gear range
MP 3515.2  : 0          ;3rd gear range
MP 3515.3  : 0          ;4th gear range
MP 3515.4  : 0          ;5th gear range
MP 3515.5  : 0          ;6th gear range
MP 3515.6  : 0          ;7th gear range
MP 3515.7  : 0          ;8th gear range
             ;------------------------------------------------------------------
             ;MP3520.0 Speed activation through marker M4011
             ;Input: 0 to 99 999.999 [rpm]
MP 3520.0  : 10
             ;------------------------------------------------------------------
             ;MP3520.1 Spindle speed for oriented spindle stop
             ;Input: 0 to 99 999.999 [rpm]
MP 3520.1  : 500
             ;------------------------------------------------------------------
             ;MP3530 Increased spindle power for roughing
             ;Input: 0 = not active
             ;       1 = increased spindle power for roughing
MP 3530    : 0
             ;------------------------------------------------------------------
             ;MP3540 Allowed undershooting of the actual value of the
             ;       spindle speed. Defined via an percentage-factor of the
             ;       nominal value of the spindel speed.
             ;Input: 0 to 100 [%]
             ;       0 = Monitoring not active
MP 3540    : 0
             ;------------------------------------------------------------------
             ;MP3542 Minimum value of the spindle speed at which the
             ;       monitoring from MP3540 becomes active.
             ;Input: 0,001 to 99 999,999 [rpm]
             ;         0 = Monitoring not active
MP 3542    : 0
             ;------------------------------------------------------------------
             ;MP3550 Delay of Emergency-Stop reaction of the spindle
             ;Input: 0,001 to 0,100 [s]
             ;         0 = Delay not active
MP 3550    : 0
             ;==================================================================
             ;MP 4000 .. 4999
             ;Integral PLC
             ;==================================================================
             ;MP4000 Options for the conditional compilation of the PLC program
             ;Input: String
MP 4000.0  : 
MP 4000.1  : 
MP 4000.2  : 
MP 4000.3  : 
MP 4000.4  : 
MP 4000.5  : 
MP 4000.6  : 
MP 4000.7  : 
MP 4000.8  : 
MP 4000.9  : 
MP 4000.10 : 
MP 4000.11 : 
MP 4000.12 : 
MP 4000.13 : 
MP 4000.14 : 
MP 4000.15 : 
MP 4000.16 : 
MP 4000.17 : 
MP 4000.18 : 
MP 4000.19 : 
MP 4000.20 : 
MP 4000.21 : 
MP 4000.22 : 
MP 4000.23 : 
MP 4000.24 : 
MP 4000.25 : 
MP 4000.26 : 
MP 4000.27 : 
MP 4000.28 : 
MP 4000.29 : 
MP 4000.30 : 
MP 4000.31 : 
MP 4000.32 : 
MP 4000.33 : 
MP 4000.34 : 
MP 4000.35 : 
MP 4000.36 : 
MP 4000.37 : 
MP 4000.38 : 
MP 4000.39 : 
MP 4000.40 : 
MP 4000.41 : 
MP 4000.42 : 
MP 4000.43 : 
MP 4000.44 : 
MP 4000.45 : 
MP 4000.46 : 
MP 4000.47 : 
MP 4000.48 : 
MP 4000.49 : 
MP 4000.50 : 
MP 4000.51 : 
MP 4000.52 : 
MP 4000.53 : 
MP 4000.54 : 
MP 4000.55 : 
MP 4000.56 : 
MP 4000.57 : 
MP 4000.58 : 
MP 4000.59 : 
MP 4000.60 : 
MP 4000.61 : 
MP 4000.62 : 
MP 4000.63 : 
             ;------------------------------------------------------------------
             ;MP4020 PLC functions
             ;Input: %xxxxxxxxxxxxx
             ;Bit 0   Reserved
             ;Bit 1   Reserved
             ;Bit 2   Reserved
             ;Bit 3   Reserved
             ;Bit 4   Reserved
             ;Bit 5   Double-spindle operation active
             ;Bit 6   Reserved
             ;Bit 7   Mean-value generation of thermistor inputs
             ;        (not on HSCI systems)
             ;        0 = Values of Pt100 inputs with a change
             ;            rate of 1 K/s are transferred
             ;        1 = Values of Pt100 inputs are transferred immediately
             ;Bit 8   Behavior in an EMERGENCY STOP
             ;        0 = ACTUAL/NOMINAL values are transferred
             ;        1 = Return to Contour is activated (as with M4580)
             ;Bit 9   Simulated keys
             ;        0 = Are transferred directly to the NC
             ;        1 = Are processed by an active PLC window
             ;            before being transferred to the NC
             ;Bit10   Disabled keys
             ;        0 = Are transferred to the active PLC window only
             ;        1 = Are transferred to neither the NC nor the active PLC w
             ;
             ;Bit 11  PLC counter in MP4120.x
             ;        0 = Input in PLC cycles
             ;        1 = Input in seconds
             ;Bit 12  Font size in PLC window
             ;        0 = Automatic adaptation of font size to screen
             ;        1 = Font size for BF 120
             ;Bit 13  Reserved
             ;Bit 14  Configuration of the PLC module interface
             ;        0 = 14 axes plus spindle
             ;            Axes: bit 0 to bit 14, spindle: bit 15
             ;        1 = 18 axes plus spindle (only in HSCI systems)
             ;            Axes: bit 0 to bit 17, spindle: bit 31
             ;Bit 15  Saving the changes made to tables to the HDR
             ;        0 = cyclic saving of tables
             ;            (same behavior as up to and including 34049x-06
             ;            SP4 or 60642x-01 SP 04)
             ;        1 = save the tables each time they are closed
             ;Bit 16  Resetting behavior of the strobe markers
             ;        0 = do not reset the strobe marker until the action
             ;            initiated by acknowledging the strobe has been
             ;            completed by the NC.
             ;        1 = reset the strobe marker immediately upon
             ;            acknowledgement(same behavior as up to and
             ;            including 34049x-06 SP4 or 60642x-01 SP 04)
             ;Bit17 - Message "Debug functions of the PLC are active"
             ;        0 = Message is triggerd
             ;        1 = Suppress the message if MP4753 = 1 has triggered
             ;            this message
             ;765432109876543210
MP 4020    : %000100100000000000
             ;------------------------------------------------------------------
             ;MP4030 Assignment of logical/physical PL
             ;       (not on HSCI systems)
             ;Input:   0 = 1st logical PL
             ;         1 = 2nd logical PL
             ;         2 = 3rd logical PL
             ;         3 = 4th logical PL
MP 4030.0  : 0          ; 1st physical PL
MP 4030.1  : 1          ; 2nd physical PL
MP 4030.2  : 2          ; 3rd physical PL
MP 4030.3  : 3          ; 4th physical PL
             ;------------------------------------------------------------------
             ;MP4031 Monitoring the really connected PLs
             ;       (Not on HSCI systems)
             ;Input: 0 to 4 [number of connected PLs]
             ;       -1 = Monitoring not active
MP 4031    : -1
             ;------------------------------------------------------------------
             ;MP4040 Set a PLC output after shutdown
             ;       PLC output definied in MP 4042/MP 4044
             ;Input: 0 = Do not set a PLC output
             ;       1 = Only after shutdown via Module 9279
             ;       2 = After shutdown via Module 9279 or soft key
MP 4040    : 0
             ;------------------------------------------------------------------
             ;MP4041 Time after shutdown of the control until
             ;       setting the PLC output from MP 4042/MP 4044
             ;Input: 0 to 1000 [s]
MP 4041    : 5
             ;------------------------------------------------------------------
             ;MP4042 PLC output to be set after shut down.
             ;       (Not on HSCI systems)
             ;Input: 0 to 30
MP 4042    : 0          ;address of O_auto_power_off_after_END_PGM
             ;------------------------------------------------------------------
             ;MP4043 Delay time at shutdown to do final PLC actions
             ;Input: 1 to 1000 [s]
             ;       0 = No delay time
MP 4043    : 0
             ;------------------------------------------------------------------
             ;MP4044 PLC output to be set after shut down.
             ;       (On controls with HSCI)
             ;Input: symbolic name of PLC output
MP 4044    : 
             ;------------------------------------------------------------------
             ;MP4050 Pulse for path-dependent lubrication
             ;Input: 0 to 99999,999 [m]
MP 4050.0  : 30
MP 4050.1  : 30
MP 4050.2  : 30
MP 4050.3  : 30
MP 4050.4  : 30
MP 4050.5  : 30
MP 4050.6  : 30
MP 4050.7  : 30
MP 4050.8  : 30
MP 4050.9  : 30
MP 4050.10 : 30
MP 4050.11 : 30
MP 4050.12 : 30
MP 4050.13 : 30
MP 4050.14 : 30
MP 4050.15 : 30
MP 4050.16 : 30
MP 4050.17 : 30
MP 4050.18 : 30
MP 4050.19 : 30
MP 4050.20 : 30
MP 4050.21 : 30
             ;------------------------------------------------------------------
             ;MP4060 Output, which should be switched off delayed by the
             ;       time of MP4061.x, when switching off all outputs.
             ;       On controls with HSCI it ist configured via IOconfig
             ;Input: 0 to 30
             ;       -1 = Do not switch off output delayed
MP 4060.0  : -1
MP 4060.1  : -1
MP 4060.2  : -1
MP 4060.3  : -1
             ;------------------------------------------------------------------
             ;MP4061 Delay time for switching off the outputs of MP4060.x.
             ;       On controls with HSCI it ist configured via IOconfig
             ;Input: 0 to 5,000 [s]
MP 4061.0  : 0
MP 4061.1  : 0
MP 4061.2  : 0
MP 4061.3  : 0
             ;------------------------------------------------------------------
             ;MP4070 Compensation amount per PLC cycle for lagged-tracking
             ;axis-error compensation (thermal compensation)
             ;Input: 0.0001 to 0.005 [mm]
MP 4070    : 0.0001
             ;------------------------------------------------------------------
             ;MP4110 Runtime for PLC Timers T0..T47
             ;Input [s]
MP 4110.0  : 0.125      ;Timer 250ms
MP 4110.1  : 0
MP 4110.2  : 0.5        ;Key impulse recognition
MP 4110.3  : 20         ;Referenz Timeout
MP 4110.4  : 1          ;Spindle nominal value on ramp
MP 4110.5  : 0.25       ;Spindle delayed stop, after axis stop
MP 4110.6  : 2.5        ;Drive enabling off-t --> Axis monitoring off/on
MP 4110.7  : 4          ;Servo drives ready
MP 4110.8  : 10         ;Spindle speed change
MP 4110.9  : 0.25       ;Servo drive release for spindle orientation
MP 4110.10 : 10         ;Gear timeout
MP 4110.11 : 0.1        ;Gear jog, pause
MP 4110.12 : 0.3        ;Gear jog, right (cw)
MP 4110.13 : 0.7        ;Gear jog, left (ccw)
MP 4110.14 : 1          ;Gear, delayed stop
MP 4110.15 : 2          ;S override turned
MP 4110.16 : 2          ;Emergency stop test 2 channels
MP 4110.17 : 0.3        ;Emergency stop loop off
MP 4110.18 : 20         ;Tool changer timeout
MP 4110.19 : 0.25       ;Tool changer, generic timer
MP 4110.20 : 10         ;Lubrication (delayed off, test pressure 0)
MP 4110.21 : 5          ;Lubrication, pressure generation (sensor)
MP 4110.22 : 10         ;Hydraulic system, pressure generation
MP 4110.23 : 1          ;Start indexing fixture
MP 4110.24 : 0.25       ;Tool changing arm cam, middle position -t --> stop
MP 4110.25 : 50         ;Pallet changer timeout
MP 4110.26 : 0.15       ;Pellet changer, generic timer
MP 4110.27 : 10         ;3-D head timeout
MP 4110.28 : 0.5        ;3-D head, generic timer
MP 4110.29 : 3          ;TSxxx Air blowing
MP 4110.30 : 20         ;Tool magazine timeout
MP 4110.31 : 0.3        ;Tool magazine 1, oriented stop
MP 4110.32 : 0.3        ;Tool magazine 2, oriented stop
MP 4110.33 : 0          ;Tool magazine 1, delayed stop
MP 4110.34 : 0          ;Tool magazine 2, delayed stop
MP 4110.35 : 5          ;TCP-IP Client Timeout
MP 4110.36 : 0.5        ;Axes monitoring off - I32/X150 (not HSCI)
MP 4110.37 : 0
MP 4110.38 : 0
MP 4110.39 : 0
MP 4110.40 : 0
MP 4110.41 : 0
MP 4110.42 : 0
MP 4110.43 : 0
MP 4110.44 : 0.2        ;FS: motion request for an axis despite SOS
MP 4110.45 : 0.1        ;FS: drive-on request for spindle without FS-release
MP 4110.46 : 1.5        ;Control voltage on -t-> off
MP 4110.47 : 1.5        ;Control voltage off -t-> on
             ;------------------------------------------------------------------
             ;MP4111 Cycle time for PLC timer T96..T999,
             ;Start of timer with Module 9006
             ;Number of additional timers in OEM.SYS
             ;  - Keyword: NUMBERMP4111 = x ; [1..999]
             ;Input [s]
MP 4111.96 : 0          ;Unclamping 1
MP 4111.97 : 0          ;Unclamping 2
MP 4111.98 : 0.5        ;Unclamping 3
MP 4111.99 : 0.5        ;Unclamping 4
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
MP 4111.112: 0.5        ;Unclamping 17
MP 4111.113: 0.5        ;Unclamping 18
MP 4111.114: 0.5        ;Unclamping 19
MP 4111.115: 0.5        ;Unclamping 20
MP 4111.116: 0.5        ;Unclamping 21
MP 4111.117: 0.5        ;Unclamping 22
MP 4111.118: 0          ;Clamping 1
MP 4111.119: 0          ;Clamping 2
MP 4111.120: 0.5        ;Clamping 3
MP 4111.121: 0.5        ;Clamping 4
MP 4111.122: 0.5        ;Clamping 5
MP 4111.123: 0.5        ;Clamping 6
MP 4111.124: 0.5        ;Clamping 7
MP 4111.125: 0.5        ;Clamping 8
MP 4111.126: 0.5        ;Clamping 9
MP 4111.127: 0.5        ;Clamping 10
MP 4111.128: 0.5        ;Clamping 11
MP 4111.129: 0.5        ;Clamping 12
MP 4111.130: 0.5        ;Clamping 13
MP 4111.131: 0.5        ;Clamping 14
MP 4111.132: 0.5        ;Clamping 15
MP 4111.133: 0.5        ;Clamping 16
MP 4111.134: 0.5        ;Clamping 17
MP 4111.135: 0.5        ;Clamping 18
MP 4111.136: 0.5        ;Clamping 19
MP 4111.137: 0.5        ;Clamping 20
MP 4111.138: 0.5        ;Clamping 21
MP 4111.139: 0.5        ;Clamping 22
MP 4111.140: 3          ;Timeout axis 1
MP 4111.141: 3          ;Timeout axis 2
MP 4111.142: 3          ;Timeout axis 3
MP 4111.143: 3          ;Timeout axis 4
MP 4111.144: 3          ;Timeout axis 5
MP 4111.145: 3          ;Timeout axis 6
MP 4111.146: 3          ;Timeout axis 7
MP 4111.147: 3          ;Timeout axis 8
MP 4111.148: 3          ;Timeout axis 9
MP 4111.149: 3          ;Timeout axis 10
MP 4111.150: 3          ;Timeout axis 11
MP 4111.151: 3          ;Timeout axis 12
MP 4111.152: 3          ;Timeout axis 13
MP 4111.153: 3          ;Timeout axis 14
MP 4111.154: 3          ;Timeout axis 15
MP 4111.155: 3          ;Timeout axis 16
MP 4111.156: 3          ;Timeout axis 17
MP 4111.157: 3          ;Timeout axis 18
MP 4111.158: 3          ;Timeout axis 19
MP 4111.159: 3          ;Timeout axis 20
MP 4111.160: 3          ;Timeout axis 21
MP 4111.161: 3          ;Timeout axis 22
MP 4111.162: 0.1        ;Waiting in position 1
MP 4111.163: 0.1        ;Waiting in position 2
MP 4111.164: 0.1        ;Waiting in position 3
MP 4111.165: 0.1        ;Waiting in position 4
MP 4111.166: 0.1        ;Waiting in position 5
MP 4111.167: 0.1        ;Waiting in position 6
MP 4111.168: 0.1        ;Waiting in position 7
MP 4111.169: 0.1        ;Waiting in position 8
MP 4111.170: 0.1        ;Waiting in position 9
MP 4111.171: 0.1        ;Waiting in position 10
MP 4111.172: 0.1        ;Waiting in position 11
MP 4111.173: 0.1        ;Waiting in position 12
MP 4111.174: 0.1        ;Waiting in position 13
MP 4111.175: 0.1        ;Waiting in position 14
MP 4111.176: 0.1        ;Waiting in position 15
MP 4111.177: 0.1        ;Waiting in position 16
MP 4111.178: 0.1        ;Waiting in position 17
MP 4111.179: 0.1        ;Waiting in position 18
MP 4111.180: 0.1        ;Waiting in position 19
MP 4111.181: 0.1        ;Waiting in position 20
MP 4111.182: 0.1        ;Waiting in position 21
MP 4111.183: 0.1        ;Waiting in position 22
MP 4111.184: 0.5        ;Axes monitoring off - axes group 01 (HSCI)
MP 4111.185: 0.5        ;Axes monitoring off - axes group 02 (HSCI)
MP 4111.186: 0.5        ;Axes monitoring off - axes group 03 (HSCI)
MP 4111.187: 0.5        ;Axes monitoring off - axes group 04 (HSCI)
MP 4111.188: 0.5        ;Axes monitoring off - axes group 05 (HSCI)
MP 4111.189: 0.5        ;Axes monitoring off - axes group 06 (HSCI)
MP 4111.190: 0.5        ;Axes monitoring off - axes group 07 (HSCI)
MP 4111.191: 0.5        ;Axes monitoring off - axes group 08 (HSCI)
MP 4111.192: 0          ;reserved for Python
MP 4111.193: 0          ;reserved for Python
MP 4111.194: 0          ;reserved for Python
MP 4111.195: 0          ;reserved for Python
MP 4111.196: 0          ;reserved for Python
MP 4111.197: 0          ;reserved for Python
MP 4111.198: 0          ;reserved for Python
MP 4111.199: 0          ;reserved for Python
MP 4111.200: 0          ;reserved for Python
MP 4111.201: 0          ;reserved for Python
MP 4111.202: 0          ;reserved for Python
MP 4111.203: 0          ;reserved for Python
MP 4111.204: 0          ;reserved for Python
MP 4111.205: 0          ;reserved for Python
MP 4111.206: 0          ;free available (CFG_Timer_MP4111_additional)
MP 4111.207: 0
MP 4111.208: 0
MP 4111.209: 0
             ;------------------------------------------------------------------
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
MP 4120.41 : 0
MP 4120.42 : 0
MP 4120.43 : 0
MP 4120.44 : 0
MP 4120.45 : 0
MP 4120.46 : 0
MP 4120.47 : 0
             ;------------------------------------------------------------------
             ;MP4130 Fast PLC inputs
             ;Input: 0 to 20000 [no. of the PLC input]
             ;       -1 funktion inactive
             ;or
             ;Input: symbolic name of PLC input
             ;       (Input I0 not supported from PLC Basic Program)
MP 4130.0  : -1         ;32 = Suppress the monitoring functions
MP 4130.1  : -1         ;Reserved
MP 4130.2  : -1         ;Sets marker M4590
MP 4130.3  : -1         ;Sets marker M4591
MP 4130.4  : -1         ;Sets marker M4592
MP 4130.5  : -1         ;Sets marker M4593
             ;------------------------------------------------------------------
             ;MP4131 Activation criterion for fast PLC input
             ;Input: 0 = Activation at low level
             ;       1 = Activation at high level
MP 4131.0  : 0          ;Suppress the monitoring functions
MP 4131.1  : 0          ;Reserved
MP 4131.2  : 0          ;Sets marker M4590
MP 4131.3  : 0          ;Sets marker M4591
MP 4131.4  : 0          ;Sets marker M4592
MP 4131.5  : 0          ;Sets marker M4593
             ;------------------------------------------------------------------
             ;MP4132 Axis or axis group specific
             ;       PLC input for controls drive enabling (only CC 6xxx)
             ;Input: 0 to 20000 [no. of the PLC input]
             ;       -1 function inactive
             ;or
             ;Input: symbolic name of PLC input
MP 4132.0  : -1
MP 4132.1  : -1
MP 4132.2  : -1
MP 4132.3  : -1
MP 4132.4  : -1
MP 4132.5  : -1
MP 4132.6  : -1
MP 4132.7  : -1
             ;------------------------------------------------------------------
             ;MP4210 General values for the PLC (D768 to D956)
             ;Input: -99 999,9999 to +99 999,9999
             ;PLC datas stored in PLC:\MP\MP_Data.tab (from Basis_53 on)
MP 4210.0  : +0
MP 4210.1  : +0
MP 4210.2  : +0
MP 4210.3  : +0
MP 4210.4  : +0
MP 4210.5  : +0
MP 4210.6  : +0
MP 4210.7  : +0
MP 4210.8  : +0
MP 4210.9  : +0
MP 4210.10 : +0
MP 4210.11 : +0
MP 4210.12 : +0
MP 4210.13 : +0
MP 4210.14 : +0
MP 4210.15 : +0
MP 4210.16 : +0
MP 4210.17 : +0
MP 4210.18 : +0
MP 4210.19 : +0
MP 4210.20 : +0
MP 4210.21 : +0
MP 4210.22 : +0
MP 4210.23 : +0
MP 4210.24 : +0
MP 4210.25 : +0
MP 4210.26 : +0
MP 4210.27 : +0
MP 4210.28 : +0
MP 4210.29 : +0
MP 4210.30 : +0
MP 4210.31 : +0
MP 4210.32 : +0
MP 4210.33 : +0
MP 4210.34 : +0
MP 4210.35 : +0
MP 4210.36 : +0
MP 4210.37 : +0
MP 4210.38 : +0
MP 4210.39 : +0
MP 4210.40 : +0
MP 4210.41 : +0
MP 4210.42 : +0
MP 4210.43 : +0
MP 4210.44 : +0
MP 4210.45 : +0
MP 4210.46 : +0
MP 4210.47 : +0
             ;------------------------------------------------------------------
             ;MP4220 General values for the PLC (W960 to W974)
             ;Input: 10 to 30 000
MP 4220.0  : 2000
MP 4220.1  : 2000
MP 4220.2  : 2000
MP 4220.3  : 2000
MP 4220.4  : 2000
             ;------------------------------------------------------------------
             ;MP4230 General values for the PLC (Module 9032)
             ;Input: -99 999.9999 to +99 999.9999
             ;PLC datas stored in PLC:\MP\MP_Data.tab (from Basis_53 on)
MP 4230.0  : +0
MP 4230.1  : +0
MP 4230.2  : +0
MP 4230.3  : +0
MP 4230.4  : +0
MP 4230.5  : +0
MP 4230.6  : +0
MP 4230.7  : +0
MP 4230.8  : +0
MP 4230.9  : +0
MP 4230.10 : +0
MP 4230.11 : +0
MP 4230.12 : +0
MP 4230.13 : +0
MP 4230.14 : +0
MP 4230.15 : +0
MP 4230.16 : +0
MP 4230.17 : +0
MP 4230.18 : +0
MP 4230.19 : +0
MP 4230.20 : +0
MP 4230.21 : +0
MP 4230.22 : +0
MP 4230.23 : +0
MP 4230.24 : +0
MP 4230.25 : +0
MP 4230.26 : +0
MP 4230.27 : +0
MP 4230.28 : +0
MP 4230.29 : +0
MP 4230.30 : +0
MP 4230.31 : +0
             ;------------------------------------------------------------------
             ;MP4231 General values for the PLC (Module 9032)
             ;Input: -99 999.9999 to +99 999.9999
             ;PLC datas stored in PLC:\MP\MP_Data.tab (from Basis_53 on)
MP 4231.0  : +0
MP 4231.1  : +0
MP 4231.2  : +0
MP 4231.3  : +0
MP 4231.4  : +0
MP 4231.5  : +0
MP 4231.6  : +0
MP 4231.7  : +0
MP 4231.8  : +0
MP 4231.9  : +0
MP 4231.10 : +0
MP 4231.11 : +0
MP 4231.12 : +0
MP 4231.13 : +0
MP 4231.14 : +0
MP 4231.15 : +0
MP 4231.16 : +0
MP 4231.17 : +0
MP 4231.18 : +0
MP 4231.19 : +0
MP 4231.20 : +0
MP 4231.21 : +0
MP 4231.22 : +0
MP 4231.23 : +0
MP 4231.24 : +0
MP 4231.25 : +0
MP 4231.26 : +0
MP 4231.27 : +0
MP 4231.28 : +0
MP 4231.29 : +0
MP 4231.30 : +0
MP 4231.31 : +0
             ;------------------------------------------------------------------
             ;MP4310 General parameters for the PLC
             ;Input: $0000 [hexadecimal] or
             ;       %0000000000000000 [binary]
             ;PLC datas stored in PLC:\MP\MP_Config.tab (from Basis_53 on)
MP 4310.0  : $0000      ;M4300-M4315, W976
MP 4310.1  : $0000      ;M4316-M4331, W978
MP 4310.2  : $0000      ;M4332-M4347, W980
MP 4310.3  : $0000      ;M4348-M4363, W982
MP 4310.4  : $0000      ;M4364-M4379, W984
             ;Screen color pallet [0..3]
MP 4310.5  : 0          ;M4380-M4395, W986
             ;Intervals for axis lubrication [min]
MP 4310.6  : 60         ;M4396-M4411, W988
MP 4310.7  : 0          ;M4412-M4427, W990
MP 4310.8  : 0          ;M4428-M4443, W992
MP 4310.9  : 0          ;M4444-M4459, W994
             ;------------------------------------------------------------------
             ;MP4400 Profibus configuration
             ;Input: %xx
             ;Bit 0 Definition of the sequence of options
             ;      0 = Sequence as in *.IOC file
             ;      1 = Sequence as MP4000.x
             ;Bit 1 reserved
             ;10
MP 4400    : %01
             ;==================================================================
             ;MP 5000 .. 5999
             ;Serial interfaces
             ;==================================================================
             ;MP5000 Disable the data interface
             ;Input: 0 = No interface disabled
             ;       1 = RS-232-C/V.24 interface disabled
             ;       2 = RS-422/V.11 interface disabled (not with HSCI)
             ;       3 = both interfaces disabled (not with HSCI)
MP 5000    : 0
             ;------------------------------------------------------------------
             ;MP5020 User-defined transmission protocol
             ;Bit 0   7 or 8 data bits          0 = 7 data bits
             ;                                  1 = 8 data bits
             ;Bit 1   Block check character     0 = Any BCC
             ;                                  1 = BCC not control character
             ;Bit 2   Transmission stop by RTS  0 = Not active
             ;                                  1 = active
             ;Bit 3   Transmission stop by DC3  0 = Not active
             ;                                  1 = active
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
             ;Bit 10  Check of missing connection 0 = active
             ;                                    1 = not active
             ;09876543210
MP 5020.0  : %00010101010;EXT1 mode
MP 5020.1  : %00010101010;EXT2 mode
MP 5020.2  : %00010101010;PLC mode (EXT3)
MP 5020.3  : %00010101010;PLC mode (EXT4)
             ;------------------------------------------------------------------
             ;MP5030 Activation of data transfer protocol
             ;        0 = "Standard data transfer protocol"
             ;        1 = "Blockwise transfer ACK/NAK"
             ;        2 = Without protocol (only for MP5030.2)
MP 5030.0  : 0          ;EXT1 mode
MP 5030.1  : 0          ;EXT2 mode
MP 5030.2  : 1          ;PLC mode (EXT3)
MP 5030.3  : 1          ;PLC mode (EXT4)
             ;------------------------------------------------------------------
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
             ;==================================================================
             ;MP 6000 .. 6499
             ;3-D touch probe
             ;==================================================================
             ;MP6010 Select the touch probe
             ;Input:   0 = Touch probe with cable transmission
             ;             (TS 120/TS 220/TS 230)
             ;         1 = Infrared probe(TS 511/TS 630/TS 632)
             ;         2 = Infrared probe (TS 440/TS 640)
             ;         3 = Infrared probe without battery (TS 444)
MP 6010    : 0          ;Select the touch probe
             ;------------------------------------------------------------------
             ;MP6120 Probing feed rate (triggering touch probe)
             ;Input: 1 to 10 000 [mm/min]
MP 6120    : 500        ;Probing feed rate [mm/min]
             ;------------------------------------------------------------------
             ;MP6130 Maximum measuring range
             ;Input: 0.001 to 99 999.9999 [mm]
MP 6130    : 10         ;Maximum measuring range [mm]
             ;------------------------------------------------------------------
             ;MP6140 Setup clearance over measuring point
             ;Input: 0.001 to 99 999.9999 [mm]
MP 6140    : 2          ;Setup clearance over measuring point
             ;------------------------------------------------------------------
             ;MP6150 Rapid traverse in probing cycle (triggering touch probe)
             ;Input: 10 to 10 000 [mm/min]
MP 6150    : 2000       ;Rapid traverse in probing cycle [mm/min]
             ;------------------------------------------------------------------
             ;MP6151 Rapid traverse at pre-positioning in probing cycle
             ;Input: 0 = inactive (use rapid traverse from MP6150)
             ;       1 = active (use rapid traverse from MP101x)
MP 6151    : 0          ;Rapid traverse at pre-positioning
             ;------------------------------------------------------------------
             ;MP6160 M function for 180° rotation of the spindle
             ;Input: -1 to 999
             ;       -1 = Orientation by NC
             ;        0 = Function inactive
MP 6160    : +0         ;M function for 180° rotation of the spindle
             ;------------------------------------------------------------------
             ;MP6161 M function for orienting the infrared touch probe before
             ;       each measurement
             ;Input: -1 to 999
             ;       -1 = Orientation by NC
             ;        0 = Function inactive
MP 6161    : +0         ;M function orienting infrared TS before measurement
             ;------------------------------------------------------------------
             ;MP6162 Orientation angle
             ;Input: 0 to 359.9999 [°]
MP 6162    : 0          ;Orientation angle [°]
             ;------------------------------------------------------------------
             ;MP6163 Difference between MP6162 and the current spindle angle
             ;       before executing an oriented spindle stop
             ;Input: 0 to 3.0000 [°]
MP 6163    : 0          ;Difference between MP6162 and current spindle angle [°]
             ;------------------------------------------------------------------
             ;MP6165 Function for infrared touch probe during manual and
             ;       automatic measurement: The touch probe is oriented such
             ;       that the stylus always deflects in the same direction
             ;       (increased probing accuracy)
             ;Input: 0 or 1
             ;       1= Function active
MP 6165    : 1          ;Stylus always deflects in the same direction
             ;------------------------------------------------------------------
             ;MP6166 Funktion for touch probe in manual measurements:
             ;       The touch probe is moved to the touch point by
             ;       considering the basic rotation
             ;Input: 0 = inactive
             ;       1 = active
MP 6166    : 0          ;Measurement by considering the basic rotation
             ;------------------------------------------------------------------
             ;MP6170 Number of programmed measurements (mean value generation)
             ;Input: 1 to 3
MP 6170    : 1          ;Number of programmed measurements
             ;------------------------------------------------------------------
             ;MP6171 Max. permissible tolerance (MP6170 > 1)
             ;Input: 0.0020 to 0.9999 [mm]
MP 6171    : 0.002      ;Max. permissible tolerance [mm]
             ;------------------------------------------------------------------
             ;MP6180 Coordinate center of calibration ring for cycle
             ;       TCH PROBE 2 "TS Calibration" with respect to the machine
             ;       datum (traverse range 1)
             ;Input: -99 999.9999 to +99 999.9999 [mm]
MP 6180.0  : +0         ;Coordinate center of calibration ring [mm] limit 1
MP 6180.1  : +0         ;Coordinate center of calibration ring [mm] limit 2
MP 6180.2  : +0         ;Coordinate center of calibration ring [mm] limit 3
             ;------------------------------------------------------------------
             ;MP6181 Coordinate center of calibration ring for cycle
             ;       TCH PROBE 2 "TS Calibration" with respect to the machine
             ;       datum (traverse range 2)
             ;Input: -99 999.9999 to +99 999.9999 [mm]
MP 6181.0  : +0         ;Coordinate center of calibration ring [mm] limit 1
MP 6181.1  : +0         ;Coordinate center of calibration ring [mm] limit 2
MP 6181.2  : +0         ;Coordinate center of calibration ring [mm] limit 3
             ;------------------------------------------------------------------
             ;MP6182 Coordinate center of calibration ring for cycle
             ;       TCH PROBE 2 "TS Calibration" with respect to the machine
             ;       datum (traverse range 3)
             ;Input: -99 999.9999 to +99 999.9999 [mm]
MP 6182.0  : +0         ;Coordinate center of calibration ring [mm] limit 1
MP 6182.1  : +0         ;Coordinate center of calibration ring [mm] limit 2
MP 6182.2  : +0         ;Coordinate center of calibration ring [mm] limit 3
             ;------------------------------------------------------------------
             ;MP6185 Distance of probing point below ring top surface for
             ;       cycle TCH PROBE 2 "TS Calibration" with respect to
             ;       the machine datum
             ;Input: 0.001 to 99 999.9999 [mm]
MP 6185.0  : 5          ;Distance of probing ring top surface [mm] limit 1
MP 6185.1  : 5          ;Distance of probing ring top surface [mm] limit 2
MP 6185.2  : 5          ;Distance of probing ring top surface [mm] limit 3
             ;==================================================================
             ;MP 6500 .. 6599
             ;Tool touch probe for tool measurement
             ;==================================================================
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
             ;432109876543210
MP 6500    : %000000001100001
             ;------------------------------------------------------------------
             ;MP6502 Connection of the TT tool touch probe
             ;Input:
             ;0 = TT connected via cable or infrared
             ;1 = TT with radio transmission
MP 6502    : 0
             ;------------------------------------------------------------------
             ;MP6505 Probing direction for tool measurement
             ;Input:
             ;0 = Positive probing direction of the angle reference axis (0°)
             ;1 = Positive probing direction in the 90°-axis
             ;2 = Negative probing direction of the angle reference axis (0°)
             ;3 = Negative probing direction in the 90°-axis
MP 6505.0  : 0          ;Probing direction for tool measurement limit 1
MP 6505.1  : 0          ;Probing direction for tool measurement limit 2
MP 6505.2  : 0          ;Probing direction for tool measurement limit 3
             ;------------------------------------------------------------------
             ;MP6507 Calculation of the probing feed rate
             ;Input:
             ; 0 = Calculation of the probing feed rate with constant tolerance
             ; 1 = Calculation of the probing feed rate with variable tolerance
             ; 2 = Constant probing feed rate
MP 6507    : 2          ;Calculation of the probing feed rate
             ;------------------------------------------------------------------
             ;MP6508 Transmission-receiver unit SE 642 (only HSCI)
             ;Input:
             ; 1 = SE 642 active
             ; 0 = not active
MP 6508    : 0          ;SE 642 active
             ;------------------------------------------------------------------
             ;MP6509 reserved
             ;Input: 0
MP 6509    : 0
             ;------------------------------------------------------------------
             ;MP6510 Max. permissible measuring error for tool measurement
             ;with rotating tool
             ;Input: 0.002 to 0.999 [mm]
MP 6510.0  : 0.005      ;First measuring error for tool measurement [mm]
MP 6510.1  : 0.01       ;Second measuring error for tool measurement [mm]
             ;------------------------------------------------------------------
             ;MP6520 Probing feed rate for tool measurement with
             ;non-rotating tool
             ;Input: 1 to 10 000 [mm/min]
MP 6520    : 500        ;Probing feed rate for tool measurement [mm/min]
             ;------------------------------------------------------------------
             ;MP6530 Distance from the tool end to the top of the probe contact
             ;during tool radius measurement
             ;Input: 0.001 to 99.9999 [mm]
MP 6530.0  : 5          ;Distance tool end to top probe contact [mm] limit 1
MP 6530.1  : 5          ;Distance tool end to top probe contact [mm] limit 2
MP 6530.2  : 5          ;Distance tool end to top probe contact [mm] limit 3
             ;------------------------------------------------------------------
             ;MP6531 Diameter or edge length of the probe contact
             ;Input: 0.001 to 99 999.9999 [mm]
MP 6531.0  : 40         ;Diameter or edge length probe contact [mm] limit 1
MP 6531.1  : 40         ;Diameter or edge length probe contact [mm] limit 2
MP 6531.2  : 40         ;Diameter or edge length probe contact [mm] limit 3
             ;------------------------------------------------------------------
             ;MP6540 Safety zone around the probe contact for pre-positioning
             ;Input: 0.001 to 99 999.9999 [mm]
MP 6540.0  : 10         ;Safety zone probe contact tool-axis direction [mm]
MP 6540.1  : 10         ;Safety zone probe contact machining plane [mm]
             ;------------------------------------------------------------------
             ;MP6550 Rapid traverse in probing cycle
             ;Input: 10 to 1 000 000 [mm/min]
MP 6550    : 2000       ;Rapid traverse in probing cycle [mm/min]
             ;------------------------------------------------------------------
             ;MP6560 M function for spindle orientation during
             ;individual tooth measurement
             ;Input: -1 to 999
             ;       -1 = Orientation by NC
             ;        0 = Function inactive
MP 6560    : +0         ;M function spindle orientation tooth measurement
             ;------------------------------------------------------------------
             ;MP6562 M function bevor or after tool measurement cycle
             ;Input: -1 bis 999 [M function number]
             ;         -1 = Function inactive
MP 6562.0  : -1         ;M function bevor cycle start (tool measurement)
MP 6562.1  : -1         ;M function after cycle end (tool measurement)
             ;------------------------------------------------------------------
             ;MP6570 Maximum permissible surface cutting speed of
             ;the tool tooth
             ;Input: 1.0000 to 120.0000 [m/min]
MP 6570    : 100        ;Maximum surface cutting speed of tool tooth [m/min]
             ;------------------------------------------------------------------
             ;MP6572 Maximum permissible speed for tool measurement
             ;Input: 0 to 1000 [rpm]
             ;       0 = 1000 [rpm]
MP 6572    : 0          ;Maximum permissible speed for tool measurement [rpm]
             ;------------------------------------------------------------------
             ;MP6580 Coordinates of the probe contact center with respect to
             ;the machine datum (traverse range 1)
             ;Input: -99 999.9999 to +99 999.9999 [mm]
MP 6580.0  : +0         ;X position probe contact center (ref) [mm] limit 1
MP 6580.1  : +0         ;Y position probe contact center (ref) [mm] limit 1
MP 6580.2  : +0         ;Z position probe contact center (ref) [mm] limit 1
             ;------------------------------------------------------------------
             ;MP6581 Coordinates of the probe contact center with respect to
             ;the machine datum (traverse range 2)
             ;Input: -99 999.9999 to +99 999.9999 [mm]
MP 6581.0  : +0         ;X position probe contact center (ref) [mm] limit 2
MP 6581.1  : +0         ;Y position probe contact center (ref) [mm] limit 2
MP 6581.2  : +0         ;Z position probe contact center (ref) [mm] limit 2
             ;------------------------------------------------------------------
             ;MP6582 Coordinates of the probe contact center with respect to
             ;the machine datum (traverse range 3)
             ;Input: -99 999.9999 to +99 999.9999 [mm]
MP 6582.0  : +0         ;X position probe contact center (ref) [mm] limit 3
MP 6582.1  : +0         ;Y position probe contact center (ref) [mm] limit 3
MP 6582.2  : +0         ;Z position probe contact center (ref) [mm] limit 3
             ;------------------------------------------------------------------
             ;MP6585 Monitor the rotary and additional linear axes
             ;       during the start of tool measurement
             ;Input: %000000 (WVUCBA) bit-encoded
MP 6585    : %000000    ;Monitor axes during the start of tool measurement
             ;------------------------------------------------------------------
             ;MP6586 Ref. coordinate for monitoring from MP6585
             ;Input: -99 999.9999 to +99 999.9999 [mm]
MP 6586.0  : +0         ;Ref coordinate for monitoring A axis [mm]
MP 6586.1  : +0         ;Ref coordinate for monitoring B axis [mm]
MP 6586.2  : +0         ;Ref coordinate for monitoring C axis [mm]
MP 6586.3  : +0         ;Ref coordinate for monitoring U axis [mm]
MP 6586.4  : +0         ;Ref coordinate for monitoring V axis [mm]
MP 6586.5  : +0         ;Ref coordinate for monitoring W axis [mm]
             ;------------------------------------------------------------------
             ;MP6600 KinematicsOpt: maximum permitted modification
             ;Input: 0,010 bis 1,000 [mm]
MP 6600    : 0.05       ;KinematicsOpt: maximum permitted modification [mm]
             ;------------------------------------------------------------------
             ;MP6601 KinematicsOpt: maximum permitted deviation of the
             ;                     calibration ball
             ;Input: 0,010 bis 0,100 [mm]
MP 6601    : 0.02       ;KinematicsOpt: max permitted deviation ball [mm]
             ;------------------------------------------------------------------
             ;MP6602 KinematicsOpt: M-function macro for postioning of the
             ;       rotary axes
             ;Input: 0 to 999
             ;       -1 = not active
MP 6602    : -1         ;KinematicsOpt: M-function macro postioning rotary axes
             ;==================================================================
             ;MP 7100 .. 7199
             ;Tapping
             ;==================================================================
             ;MP7110 Limits for feed rate override during tapping
             ;Input: 0 to 150 [%]
MP 7110.0  : 95         ;Minimum
MP 7110.1  : 105        ;Maximum
             ;------------------------------------------------------------------
             ;MP7120 Waiting times during the tapping cycle
             ;Input: 0 to 65.535 [s]
MP 7120.0  : 0          ;Dwell time for rotational direction reversal
MP 7120.1  : 0          ;Advanced switching time
             ;(only with coded spindle speed output)
MP 7120.2  : 0          ;Spindle slow-down time after reaching the hole depth
             ;------------------------------------------------------------------
             ;MP7130 Run-in behavior of the spindle during rigid tapping
             ;Input: 0.1 to 10 [°/min]
MP 7130    : 1
             ;------------------------------------------------------------------
             ;MP7150 Positioning window for the tool axis
             ;Please note: The value must be smaller than or equal to the
             ;window in MP1030.
             ;Input: 0.0001 to 2 [mm]
MP 7150    : 0.01
             ;------------------------------------------------------------------
             ;MP7160 Behaviour of the spindle with cycle 17, 207 and 18
             ;Input: %xxxxx
             ;Bit 0  0 = Spindle orientation before executing Cycle 17
             ;       1 = No spindle orientation before executing orientation
             ;Bit 1  0 = No limitation of the spindle speed
             ;       1 = For small thread depths, the spindle speed is
             ;           limited so that the spindle runs with constant speed
             ;           approx. 1/3 of the time.
             ;Bit 2  0 = Spindle operated without position feedback control
             ;       1 = Spindle operated with position feedback control
             ;Bit 3  0 = Acceleration feedforward control active
             ;       1 = Acceleration feedforward control not active
             ;Bit 4  0 = Tool axis of the spindle is tracked
             ;       1 = Tool axis and spindle interpolated
             ;43210
MP 7160    : %00000
             ;==================================================================
             ;MP 7200 .. 7399
             ;Programming and display
             ;==================================================================
             ;MP7210 Programming station
             ;Input: 0 = Controlling and programming
             ;       1 = Programming station with PLC active
             ;       2 = Programming station with PLC inactive
             ;       3 = Programming station with "PLC and
             ;           emergency stop active" (X41/34, X42/4)
MP 7210    : 1
             ;------------------------------------------------------------------
             ;MP7212 Power interruption
             ;Input: 0 = The "Power interrupted" message must be
             ;           acknowledged with the CE key
             ;       1 = The "Power interrupted" message does not appear
MP 7212    : 1
             ;------------------------------------------------------------------
             ;MP7220 Increment for N numbers in ISO programming
             ;Input: 0 to 250
             ; (0=no automatic generation)
MP 7220    : 10
             ;------------------------------------------------------------------
             ;MP7224 Disable file types
             ;Input: %xxx xxxxx
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
             ;76543210
MP 7224.0  : %00000000  ;Disable file type
MP 7224.1  : %00000000  ;Set file type to read-only
MP 7224.2  : %00000000  ;Disable softkey EDIT OFF/ON
             ;------------------------------------------------------------------
             ;MP7225 Disable Windows drives in the TNC file management
             ;Input: A to Z
             ;       Several drives are entered without a space,
             ;       e. g. MP7225 = CDE
MP 7225    : A
             ;------------------------------------------------------------------
             ;MP7226 Length of tables
             ;Input:   0 to 255 [lines]
MP 7226.0  : 0          ;Reserved
MP 7226.1  : 20         ;Datum table (*.d)
             ;------------------------------------------------------------------
             ;MP7229.0 Program length for program inspection
             ;Input:   100 to 9999
MP 7229.0  : 200
             ;------------------------------------------------------------------
             ;MP7229.1 Program length to which FK blocks are allowed
             ;Input:   100 to 9999
MP 7229.1  : 9999
             ;------------------------------------------------------------------
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
             ;         17= Slowenian
             ;         18= Norwegian
             ;         19= Slovak
             ;         20= reserved
             ;         21= Korean
             ;         22= reserved
             ;         23= Turkish
             ;         24= Romanian
             ;         25= reserved
MP 7230.0  : 0          ;NC dialogs
MP 7230.1  : 0          ;PLC dialogs (user cycle, user parameter)
MP 7230.2  : 0          ;PLC error message
MP 7230.3  : 0          ;Help files
             ;------------------------------------------------------------------
             ;MP7237 PLC operating times counter (code number 857282)
             ;Input: %7654321 bit-encoded
             ;2109876543210
MP 7237.0  : %0000000001111;Display PLC counter
MP 7237.1  : %1111111111111;Reset PLC counter
             ;Reset NC counter (code number 857282)
             ;Input: %xxxxxxx bit-encoded
             ;Bit 0   Reserved
             ;Bit 1   "Machine on" can be reset
             ;Bit 2   "Program run" can be reset
             ;2109876543210
MP 7237.2  : %0000000000110
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
             ;------------------------------------------------------------------
             ;MP 7245 Enable auxiliary cycle 18 and cycle 32
             ;Input:
             ;Bit0   0 = Auxiliary cycle 18 disabled
             ;       1 = Auxiliary cycle 18 enabled
             ;Bit1   0 = Cycle 32 enabled
             ;       1 = Cycle 32 disabled
             ;10
MP 7245    : %01
             ;------------------------------------------------------------------
             ;MP7246 Machine parameter with multiple function
             ;Input: %xxxxx
             ;Bit 0   0 = Paraxial positioning blocks permitted
             ;        1 = Paraxial positioning blocks not permitted
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
             ;43210
MP 7246    : %01100
             ;------------------------------------------------------------------
             ;MP7251 Number of global Q parameters given to the
             ;calling program by the manufacturer cycle
             ;Input:   0 to 40
MP 7251    : 10
             ;------------------------------------------------------------------
             ;MP7260 Size of the tool table
             ;Input:   0 to 30 000
MP 7260    : 999        ;Number of tools (T number)
             ;------------------------------------------------------------------
             ;MP7261 Size of the pocket tables
             ;Input:   0 to 9999
MP 7261.0  : 40         ;Number of pockets in magazine 1 (P number)
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
             ;------------------------------------------------------------------
             ;MP7262 Maximum index number for indexed tools
             ;Input:   0 to 9
MP 7262    : 2
             ;------------------------------------------------------------------
             ;MP7263 Pocket table
             ;Input: %xxxxxxx
             ;       Bit0 - Soft key "Pocket table"
             ;       0 = Show soft key
             ;       1 = Hide soft key
             ;       Bit1 - Output of the columns for file functions
             ;       0 = Output only the displayed columns
             ;       1 = Output all columns
             ;       Bit2 - Soft key "Edit ON/OFF"
             ;       0 = Show soft key
             ;       1 = Hide soft key
             ;       Bit3 - Soft keys "Reset pocket table"
             ;              and "Reset column T"
             ;       0 = Show soft keys
             ;       1 = Hide soft keys
             ;       Bit 4 - Deletion possible for a tool that is in
             ;               the pocket table.
             ;               Deletion must be confirmed.
             ;       0 = Deletion not possible (behavior as before)
             ;       1 = Deletion possible (with confirmation)
             ;       Bit 5 - Deletion of a tool possible even without
             ;               confirmation (if bit 4 = 1).
             ;       0 = Deletion not possible without confirmation
             ;       1 = Deletion possible without confirmation
             ;       Bit 6 - Deletion of index entries of a tool behaves
             ;               like deletion of a tool.
             ;               Settings in bits 4 and 5 are
             ;               taken into account.
             ;       0 = Deletion always possible (behavior as before)
             ;       1 = Deletion possible depending on settings
             ;           in bits 4 and 5
             ;6543210
MP 7263    : %0000010
             ;------------------------------------------------------------------
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
MP 7266.42 : 0          ;Link to compensation value table
             ;           (DR2TABLE, Option #92)
MP 7266.43 : 44         ;Time stamp of last use (LAST-USE)
MP 7266.44 : 45         ;Active Chatter Compensation (ACC, Option #145)
MP 7266.45 : 46         ;Exceeding the tool life (OVRTIME)
MP 7266.46 : 47         ;Collision length, processing via PLC program (CL)
MP 7266.47 : 48         ;Collision radius, processing via PLC program (CR)
MP 7266.48 : 49         ;PLC value (P4)
             ;------------------------------------------------------------------
             ;MP7267 Elements of the pocket table
             ;Input: 0 = No display
             ;       1 to 99 = Position in the pocket table
MP 7267.0  : 1          ;Tool number (T)
MP 7267.1  : 3          ;Special tool (ST)
MP 7267.2  : 4          ;Fixed pocket (F)
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
             ;------------------------------------------------------------------
             ;MP7268.x Sorting/hiding columns of the preset table
             ;Input: 1 to 99 = Order of columns
             ;         0 = Hide column
MP 7268.0  : 1
MP 7268.1  : 2
MP 7268.2  : 3
MP 7268.3  : 4
MP 7268.4  : 5
MP 7268.5  : 6
MP 7268.6  : 7
MP 7268.7  : 8
MP 7268.8  : 9
MP 7268.9  : 10
MP 7268.10 : 11
             ;------------------------------------------------------------------
             ;MP7270 Feed-rate display in manual operating modes
             ;(manual operation and electronic handwheel)
             ;0 = Display of the feed rate only by pressing an
             ;    axis direction key (axis-specific feed rate from MP1020.x)
             ;1 = Display of the feed rate also before pressing an
             ;    axis direction key (smallest value from MP1020.x for
             ;    all axes)
MP 7270    : 0
             ;------------------------------------------------------------------
             ;MP7280 Definition of the decimal character
             ;Input: 0 = Comma
             ;       1 = Point
MP 7280    : 1
             ;------------------------------------------------------------------
             ;MP7281 Depiction of the NC program
             ;Input: 0 = Always show all NC blocks completely
             ;       1 = Only show current block completely
             ;       2 = Only show complete NC block during creation or
             ;           change
MP 7281.0  : 0          ; Programming & Editing
MP 7281.1  : 0          ; Program Run
             ;------------------------------------------------------------------
             ;MP7285 Tool length offset for the
             ;tool-axis position display
             ;Input: 0 = Tool length is not offset
             ;       1 = Tool length is offset
MP 7285    : 1
             ;------------------------------------------------------------------
             ;MP7289 Position display step for the spindle
             ;Input: 0 = 0.1 [degrees]
             ;       1 = 0.05 [degrees]
             ;       2 = 0.01 [degrees]
             ;       3 = 0.005 [degrees]
             ;       4 = 0.001 [degrees]
             ;       5 = 0.0005 [degrees]
             ;       6 = 0.0001 [degrees]
MP 7289    : 4
             ;------------------------------------------------------------------
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
MP 7290.14 : 4
MP 7290.15 : 4
MP 7290.16 : 4
MP 7290.17 : 4
MP 7290.18 : 4
MP 7290.19 : 4
MP 7290.20 : 4
MP 7290.21 : 4
             ;------------------------------------------------------------------
             ;MP7291 Displayed axes
             ;Input: SWvUcBaZyX
             ; X = NC axes (capital letters)
             ; x = PLC axes (lowercase letters)
             ; - = Non-displayed axes
             ;987654ZYX
MP 7291.0  : SC-a-BAZYX ;Traverse range 1
MP 7291.1  : SC-a-BAZYX ;Traverse range 2
MP 7291.2  : SC-a-BAZYX ;Traverse range 3
             ;------------------------------------------------------------------
             ;MP7294 Disable axis-specific datum setting in the
             ;       preset table
             ;       (as of 340 422-01, 340 480-01)
             ;Input: %xxxxxxxxxxxxxx
             ;       Bits 0 to 13 represent axes 1 to 14
             ;       0 = Not disabled
             ;       1 = Disabled
             ;2109876543210987654ZYX
MP 7294    : %0000000000000000000000
             ;------------------------------------------------------------------
             ;MP7295 Disable "Datum setting"
             ;Input: %xxxxxxxxxxxxxx
             ;       Bits 0 to 13 represent axes 1 to 14
             ;       0 = Not disabled
             ;       1 = Disabled
             ;2109876543210987654ZYX
MP 7295    : %0000000000000000000000
             ;------------------------------------------------------------------
             ;MP7296 Datum setting with axis keys
             ;Input:
             ;Bit 0 = 0 Datum can be set with axis keys and by soft key
             ;Bit 0 = 1 Datum can be set only by soft key
             ;10
MP 7296    : %01
             ;------------------------------------------------------------------
             ;MP7300 Erasing the status display and Q parameters
             ;Input values 0 to 3 should not be used because of saftey
             ;reasons. Tool data are deleted.
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
             ;------------------------------------------------------------------
             ;MP7310 Graphic display mode
             ;Input: %xxxxxxx
             ;Bit 0   Changing the projection in three planes
             ;        0 = German-preferred projection
             ;        1 = US-preferred projection
             ;Bit 1   reserved
             ;Bit 2   BLK form after datum shift
             ;        0 = BLK form is not shifted
             ;        1 = BLK form is shifted
             ;Bit 3   reserved
             ;Bit 4   reserved
             ;Bit 5   3D graphics in program test
             ;        0 = 2,5D and 3D (only with MC 420 or MC 422B and higher)
             ;        1 = 2,5D
             ;Bit 6   Stock removal with an inclined tool
             ;        0 = Not active
             ;        1 = Active
             ;Bit 7   Evaluation of the column LCUTS from TOOL.T, so that
             ;        special tools can be shown
             ;        0 = Free evaluation
             ;        1 = Exact evaluation
             ;76543210
MP 7310    : %00000000
             ;------------------------------------------------------------------
             ;MP7312 Limits the tooth lengs if no value is entered in the
             ;LCUTS column in the tool table
             ;Input:     0 = No limitation, infinetly long tooth length
             ;         > 0 Tooth length = 2 * tool radius * MP7312
MP 7312    : 5.00000
             ;------------------------------------------------------------------
             ;MP7315 Tool radius for graphic simulation of laser and
             ;water jet without Tool Call
             ;Input: 0.0000 to 99 999.9999 [mm]
MP 7315    : 5
             ;------------------------------------------------------------------
             ;MP 7316 Penetration depth in the blank form for graphic
             ;simulation of laser and water jet without Tool Call
             ;Input: 0.0000 to 99 999.9999 [mm]
MP 7316    : 0
             ;------------------------------------------------------------------
             ;MP7317 Activating the graphic simulation
             ;Input: Number of M command
MP 7317.0  : 0          ;M command to start simulation
MP 7317.1  : 0          ;M command to end simulation
             ;------------------------------------------------------------------
             ;MP7330 Specifying the user parameters for the MODE function
             ;Input: MP number
             ;Warning: Enter a 2-digit index (i.e. 110.01, not 110.1)
MP 7330.0  : 7210       ;0,1,2 Controlling,Programming,+PLC active
MP 7330.1  : 7410       ;Dimension xyz/z
MP 7330.2  : 7420       ;Behavior with cycles
MP 7330.3  : 7430       ;Pocket overlapping
MP 7330.4  : 7431       ;Tolerance for circular path
MP 7330.5  : 0
MP 7330.6  : 4310.06    ;Axis lubrication [min]
MP 7330.7  : 7640       ;Activating the handwheel
MP 7330.8  : 4310.05    ;Selecting monitor color palette
MP 7330.9  : 6580       ;TT1x0 position X [REF]
MP 7330.10 : 6580.01    ;TT1x0 position Y [REF]
MP 7330.11 : 6580.02    ;TT1x0 position Z [REF]
MP 7330.12 : 0
MP 7330.13 : 0
MP 7330.14 : 0
MP 7330.15 : 0
             ;------------------------------------------------------------------
             ;MP7340 Specifying the dialogs for user parameters
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
             ;------------------------------------------------------------------
             ;MP735x Parameters for calculating the color definitions
             ;   Red    -   Green   -   Blue
             ;0..F-0..F - 0..F-0..F - 0..F-0..F
             ;rough-fine - rough-fine - rough-fine
             ;e.g. $03F3F3F -> white color
             ;------------------------------------------------------------------
MP 7350    : $0808080   ;Window frame
             ;------------------------------------------------------------------
             ;Error messages
MP 7351.0  : $0FF2020   ;Error
MP 7351.1  : $000FF00   ;Warning
MP 7351.2  : $00000FF   ;Information
             ;------------------------------------------------------------------
             ;"Machine" operating mode display
MP 7352.0  : $0ECECEC   ;Background
MP 7352.1  : $0000000   ;Text for operating mode
MP 7352.2  : $00000FF   ;Dialog
             ;------------------------------------------------------------------
             ;"Programming" operating mode display
MP 7353.0  : $0C0C0C0   ;Background
MP 7353.1  : $0000000   ;Text for operating mode
MP 7353.2  : $00000FF   ;Dialog
             ;------------------------------------------------------------------
             ;"Machine" program text display
MP 7354.0  : $0FFFFFF   ;Background
MP 7354.1  : $0000000   ;General program text
MP 7354.2  : $00000FF   ;Active block
MP 7354.3  : $0A0A0A0   ;Comments and not used MP's in the MP file
MP 7354.4  : $0C8C8C8   ;Background of inactive window (structure)
             ;------------------------------------------------------------------
             ;"Programming" program text display
MP 7355.0  : $0FFFFFF   ;Background
MP 7355.1  : $0000000   ;General program text
MP 7355.2  : $00000FF   ;Active block
MP 7355.3  : $0A0A0A0   ;Comments and not used MP's in the MP file
MP 7355.4  : $0C8C8C8   ;Background of inactive window (structure)
             ;------------------------------------------------------------------
             ;"Machine" soft-key display
MP 7357.0  : $0C0C0C0   ;Background
MP 7357.1  : $0000000   ;Font colour
MP 7357.2  : $0000000   ;Inactive soft-key row
MP 7357.3  : $00000FF   ;Active soft-key row
             ;------------------------------------------------------------------
             ;"Programming" soft-key display
MP 7358.0  : $0C0C0C0   ;Background
MP 7358.1  : $0000000   ;Symbols
MP 7358.2  : $0000000   ;Inactive soft-key row
MP 7358.3  : $00000FF   ;Active soft-key row
             ;------------------------------------------------------------------
             ;Graphics: 3-D view and plan view
MP 7360.0  : $0AABBAA   ;Background
MP 7360.1  : $08888F0   ;Top surface
MP 7360.2  : $0FFFFFF   ;3-D: Front face
MP 7360.3  : $0000000   ;Text display in the graphics window
MP 7360.4  : $0FFFFA0   ;3-D: Lateral face
MP 7360.5  : $0000000   ;Lowest point of the blank form
MP 7360.6  : $0FFFFFF   ;Highest point of blank form (below surface)
MP 7360.7  : $0CC1919   ;3-D: Sectional plane
MP 7360.8  : $08589C6   ;3-D: Tool 1
MP 7360.9  : $0C6901C   ;3-D: Tool 2
MP 7360.10 : $06C7889   ;3-D: Tool 3
MP 7360.11 : $0866D2E   ;3-D: Tool 4
MP 7360.12 : $0B34415   ;3-D: Tool 5
MP 7360.13 : $0BF9A39   ;3-D: Tool 6
MP 7360.14 : $0818181   ;3-D: Tool 7
MP 7360.15 : $086C790   ;3-D: Tool 8
             ;------------------------------------------------------------------
             ;Graphics: Projection in three planes
MP 7361.0  : $0AAAAAA   ;Background
MP 7361.1  : $00000E8   ;Top view
MP 7361.2  : $00000E8   ;Front and side view
MP 7361.3  : $0FF0000   ;Axis cross and text in the graphic display
MP 7361.4  : $0FF00FF   ;Cursor
             ;------------------------------------------------------------------
             ;Additional status display in the graphics window
MP 7362.0  : $0ECECEC   ;Background of graphic window
MP 7362.1  : $0FFFFFF   ;Background of status display
MP 7362.2  : $00000FF   ;Status symbols
MP 7362.3  : $0FF0000   ;Status values
MP 7362.4  : $0C0C0C0   ;Background of not selected tab
MP 7362.5  : $0ECECEC   ;Tab AFC - Background bar, diagram
MP 7362.6  : $000FF80   ;Tab AFC - Color actual override factor
MP 7362.7  : $000AAFF   ;Tab AFC - Color actual spindle load
             ;------------------------------------------------------------------
             ;Programming graphics
MP 7363.0  : $0ECECEC   ;Background
MP 7363.1  : $00000FF   ;Resolved contour
MP 7363.2  : $0FF00FF   ;Subprograms and frames for zooming
MP 7363.3  : $000EC00   ;Alternate solutions
MP 7363.4  : $0FF0000   ;Unresolved contour
MP 7363.5  : $00080FF   ;Contour in rapid traverse
MP 7363.6  : $0000000   ;Block number
             ;------------------------------------------------------------------
             ;Color of the help illustrations for cycles
MP 7364.0  : $0D2D2D2   ;Color of border of help illustrations
MP 7364.1  : $0FF0000   ;no function
MP 7364.2  : $0808080   ;no function
MP 7364.3  : $0000000   ;no function
MP 7364.4  : $0000163   ;no function
MP 7364.5  : $0000000   ;no function
MP 7364.6  : $0000000   ;no function
MP 7364.7  : $0AA0000   ;no function
MP 7364.8  : $000EEEE   ;no function
MP 7364.9  : $0DBD3DB   ;no function
             ;------------------------------------------------------------------
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
             ;------------------------------------------------------------------
             ;Pop-up window (HELP key, pop-up menus)
MP 7366.0  : $0ECECEC   ;Background
MP 7366.1  : $0000000   ;Text or foreground
MP 7366.2  : $00000FF   ;Active line
MP 7366.3  : $0FF0000   ;Title line
MP 7366.4  : $0FFFFFF   ;Scroll-bar field
MP 7366.5  : $0FF0000   ;Scroll bar
MP 7366.6  : $0000000   ;Reserved
MP 7366.7  : $0202020   ;Reserved
MP 7366.8  : $0404040   ;Reserved
MP 7366.9  : $0606060   ;Reserved
MP 7366.10 : $0808080   ;Reserved
MP 7366.11 : $0A0A0A0   ;Reserved
MP 7366.12 : $0C0C0C0   ;Reserved
MP 7366.13 : $0E0E0E0   ;Reserved
MP 7366.14 : $0FFFFFF   ;Reserved
             ;------------------------------------------------------------
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
             ;MP7350    ;Color 15
             ;------------------------------------------------------------
             ;Pocket calculator
MP 7368.0  : $0ACACAC   ;Background
MP 7368.1  : $0FFFFFF   ;Background for keys and displays
MP 7368.2  : $00000FF   ;Key texts ("os" in "cos")
MP 7368.3  : $0FF0040   ;Key symbols
             ;------------------------------------------------------------
             ;Directory tree in PGM MGT
MP 7369.0  : $0ECECEC   ;Text background
MP 7369.1  : $0000000   ;Text
MP 7369.2  : $00000FF   ;Text background of the active folder
MP 7369.3  : $0000000   ;Line color of the tree structure
MP 7369.4  : $0FF6000   ;Folders
MP 7369.5  : $0FF0040   ;Drives
MP 7369.6  : $0FF0000   ;Text background in title of the browser window
             ;------------------------------------------------------------
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
             ;------------------------------------------------------------------
             ;Status window and PLC window
MP 7371.0  : $0ECECEC   ;Background window
MP 7371.1  : $0FFFFFF   ;Background elements
MP 7371.2  : $00000FF   ;Values other than positions
MP 7371.3  : $00000FF   ;Positions
             ;------------------------------------------------------------------
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
MP 7375.18 : $0FF0000   ;Dialogue title: background
MP 7375.19 : $0000000   ;Dialogue title: text
MP 7375.20 : $000FA00   ;Pattern generator: positions on same level
MP 7375.21 : $00000C8   ;Pattern generator: current active positions
MP 7375.22 : $0800000   ;Pattern generator: deleted positions
MP 7375.23 : $0FF6432   ;Pattern generator: fade out positions
MP 7375.24 : $07B342D   ;Pattern generator: rectangle for zoom
             ;------------------------------------------------------------
             ;MP7392 Screen saver options. On controls with HSCI
             ;       it ist configured via window manager XFCE
             ;MP7392.0 Time after which the screen saver will be started
             ;Input: 0 = off
             ;       1 to 99 [min]
             ;MP7392.1 Screen saver type
             ;Input: 0 = off
             ;       1 = Default screen saver of X-Server
             ;       2 = 3D lines sample
MP 7392.0  : 0
MP 7392.1  : 0
             ;============================================================
             ;MP 7400 .. 7599
             ;Machining and Program Run
             ;============================================================
             ;MP7400 Look-Ahead memory of interpolation unit
             ;(with NC-SW 430 49x only with software option 2,
             ;with NC-SW 606 42x without software option)
             ;Input: 0 = 256 blocks
             ;       1 = 512 blocks
             ;       2 = 1024 blocks
MP 7400    : 0
             ;------------------------------------------------------------
             ;MP7410 Scaling cycle in two or three axes
             ;Input:
             ; 0 = Scaling cycle is effective in all three principal axes
             ; 1 = Scaling cycle is effective only in the working plane
MP 7410    : 0
             ;------------------------------------------------------------
             ;MP7411 Tool data in the touch probe block
             ;Input: 0 = The calibrated data of the touch probe is used
             ;       1 = The current tool data from the last
             ;           TOOL CALL is used.
             ;           Display of tool name and number
MP 7411    : 1
             ;------------------------------------------------------------------
             ;MP7412 Settings for cycles
             ;Input: 0 or 1
             ;Bit 0   Behavior of marker M4031 in Cycle 209
             ;        0 = Set M4031 after the spindle orientation
             ;        1 = Set M4031 before the spindle orientation
MP 7412    : 0
             ;------------------------------------------------------------------
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
             ;76543210
MP 7420    : %00000000
             ;------------------------------------------------------------
             ;MP7430 Overlap factor for pocket milling
             ;Input:   0.1 to 1.414
MP 7430    : 1.2
             ;------------------------------------------------------------
             ;MP7431 Arc end-point tolerance
             ;Input: 0.0001 to 0.016 [mm]
MP 7431    : 0.001
             ;------------------------------------------------------------
             ;MP7432 Limit switch tolerance with M140/M150
             ;Input: 0.0001 to 1.0000 [mm]
             ;0 = Limit switch tolerance deactivated
MP 7432    : 0
             ;------------------------------------------------------------
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
             ;Bit 7   Internal spindle state in geometry
             ;        0 = with M00 int. spindle state changes to M05
             ;        1 = with M00 int. spindle state changes not to M05
             ;76543210
MP 7440    : %01011011
             ;------------------------------------------------------------------
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
             ;210
MP 7441    : %100
             ;------------------------------------------------------------------
             ;MP7442 Number of the M function for spindle orientation
             ;       in the cycles
             ;       (e.g.: Cycles 202/204/209)
             ;Input: -1 to 999
             ;       -1: Oriented spindle stop by the NC
             ;        0: No oriented spindle stop
             ; 1 to 999: Number of the M function
MP 7442    : -1
             ;------------------------------------------------------------------
             ;MP7444 Delay time for the change signals (M/S/T)
             ;Input:  0: Change signal after complete filter run time
             ;           (default behavior)
             ;        1: Change signal if the nominal feed rate
             ;           reached the value 0 before the actual value
             ;           (formed by the filters) did.
             ;        2: Change signal if the nominal feed rate reached
             ;           the value 0 before the actual value (formed by
             ;           the filters) did, and an additional delay until
             ;           the actual value reaches the time window the
             ;           first time.
MP 7444    : 0
             ;------------------------------------------------------------------
             ;MP7450 Activate simulated tool change position from MP951
             ;Input: %987654321 bit-encoded
             ;       0 = Do not offset
             ;       1 = Offset
             ;2109876543210987654ZYX
MP 7450    : %0000000000000000000000
             ;------------------------------------------------------------------
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
MP 7451.14 : 2000
MP 7451.15 : 2000
MP 7451.16 : 2000
MP 7451.17 : 2000
MP 7451.18 : 2000
MP 7451.19 : 2000
MP 7451.20 : 2000
MP 7451.21 : 2000
             ;------------------------------------------------------------------
             ;MP7460 reserved
MP 7460.0  : 0
MP 7460.1  : 0
             ;------------------------------------------------------------------
             ;MP7461 reserved
MP 7461.0  : 0
MP 7461.1  : 0
             ;------------------------------------------------------------------
             ;MP7470 Maximum feed rate at 100% override
             ;Input: 0 = No limitation
             ;       1 = To 300 000 [mm/min]
MP 7470    : 0
             ;------------------------------------------------------------------
             ;MP7471 Maximum feed rate of linear axes with M128 (compensating
             ;movement by positioning of an angular axis)
             ;Input: 0 = No limitation
             ;       1 = To 300 000 [mm/min]
MP 7471    : 0
             ;------------------------------------------------------------------
             ;MP7475 Reference for datum points
             ;Input: 0 = Reference is workpiece datum
             ;       1 = Reference is machine datum
MP 7475    : 0
             ;------------------------------------------------------------------
             ;MP7480 Tool change, output of T/P code
             ;Input:   0 = No output
             ;         1 = Fixed pocket T number if change
             ;         2 = Fixed pocket T number always
             ;         3 = Variable T/P number if change
             ;         4 = Variable T/P number always
             ;         5 = Fixed pocket T/P number if change
             ;         6 = Fixed pocket T/P number always
MP 7480.0  : 0          ;Tool Call
MP 7480.1  : 0          ;Tool Def
             ;------------------------------------------------------------------
             ;MP7481 Removing/returning tools between magazines
             ;Input: %xxxxxxxx
             ;       0 = Control removing/returning with M4540
             ;       1 = First remove new, then return old
             ;87654321
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
             ;------------------------------------------------------------------
             ;MP7482 Fixed pocket coding for magazines 1 to 4
             ;Input: %xxx
             ;       0 = Magazine does not use fixed pocket coding
             ;       1 = Magazine uses fixed pocket coding
             ;Bit 0..15 - Magazine 1..16
             ;6543210987654321
MP 7482    : %0000000000000000
             ;------------------------------------------------------------------
             ;MP7483 Using Tool-Number or Tool-Name in TOOL CALL/TOOL DEF
             ;Input: 0 = Tool-Name and Tool-Number allowed
             ;       1 = only Tool-Name allowed
             ;       2 = only Tool-Number allowed
MP 7483    : 0
             ;------------------------------------------------------------------
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
             ;------------------------------------------------------------------
             ;MP7485 Act upon usage time for tool selection
             ;Input: 0 to 100 [%]
MP 7485    : 10
             ;------------------------------------------------------------------
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
             ;3210
MP 7490    : %0001
             ;------------------------------------------------------------------
             ;MP 7492 Number of the axis in which the same datum is set
             ;        during datum setting
             ;        (e.g. parallel axes)
             ;        (with active preset table)
             ;Input:   0 to 22 [number of the axis]
             ;         -1 = Do not set a datum
MP 7492.0  : -1         ;Datum set in the 1. axis
MP 7492.1  : -1         ;Datum set in the 2. axis
MP 7492.2  : -1         ;Datum set in the 3. axis
MP 7492.3  : -1         ;Datum set in the 4. axis
MP 7492.4  : -1         ;Datum set in the 5. axis
MP 7492.5  : -1         ;Datum set in the 6. axis
MP 7492.6  : -1         ;Datum set in the 7. axis
MP 7492.7  : -1         ;Datum set in the 8. axis
MP 7492.8  : -1         ;Datum set in the 9. axis
MP 7492.9  : -1         ;Datum set in the 10. axis
MP 7492.10 : -1         ;Datum set in the 11. axis
MP 7492.11 : -1         ;Datum set in the 12. axis
MP 7492.12 : -1         ;Datum set in the 13. axis
MP 7492.13 : -1         ;Datum set in the 14. axis
MP 7492.14 : -1         ;Datum set in the 15. axis
MP 7492.15 : -1         ;Datum set in the 16. axis
MP 7492.16 : -1         ;Datum set in the 17. axis
MP 7492.17 : -1         ;Datum set in the 18. axis
MP 7492.18 : -1         ;Datum set in the 19. axis
MP 7492.19 : -1         ;Datum set in the 20. axis
MP 7492.20 : -1         ;Datum set in the 21. axis
MP 7492.21 : -1         ;Datum set in the 22. axis
             ;------------------------------------------------------------------
             ;MP7493 Tolerance at datum setting with M114;
             ;       Max. deviation of actual tool orientation to the tool
             ;       axis (of swivelt coordination system) at datum setting
             ;Input: 0 to 30,0000 [degrees]
MP 7493    : 0.005
             ;------------------------------------------------------------------
             ;MP7494 Axes that always require a precision stop when
             ;       positioning (with rotary axes even irrespective of
             ;       M134)
             ;Input: %987654321 bit-encoded
             ;         0 = No precision stop
             ;         1 = Precision stop
             ;2109876543210987654ZYX
MP 7494    : %0000000000000000000000
             ;------------------------------------------------------------------
             ;MP7502 M144 (M145) Include machine kinematics
             ;Input: %xxx
             ;Bit 0   0 = M144/M145 not active
             ;        1 = M144/M145 active
             ;Bit 1   M144/M145 in the automatic modes
             ;        0 = M144/M145 active
             ;        1 = M144 is activated automatically at the start of
             ;            an NC program
             ;Bit 2   M144/M145 in the manual modes
             ;        0 = M144/M145 not active
             ;        1 = M144/M145 active
             ;210
MP 7502    : %000
             ;------------------------------------------------------------------
             ;MP7503 Activating of FCL function "Virtual Tool-axis"
             ;       supports Moving/Repositioning in tool-axis direction
             ;Input: 0 = inactive
             ;       1 = active
MP 7503    : 1
             ;------------------------------------------------------------------
             ;MP7506 Selection of a kinematic while booting up the control
             ;Input: -1 = inactive
             ;       0 to 999
MP 7506    : -1
             ;------------------------------------------------------------------
             ;MP7507 Selection of a kinematic via softkey
             ;Input: %xx
             ;Bit 0   0 = Selection of a kinematic in editing operting modes
             ;            not possible
             ;        1 = Selection of a kinematic in editing operting modes
             ;            for simulation in program test mode possible
             ;Bit 1   0 = Selection of a kinematic in machining operting
             ;            modes not possible
             ;        1 = Selection of a kinematic in machining operting
             ;            modes for the real machine possible
             ;Bit 2   0 = Softkey 3D ROT not available in program test
             ;        1 = Softkey 3D ROT available in program test
             ;Bit 3   0 = Keyword KINEMATIC possible
             ;        1 = Keyword KINEMATIC disabled
             ;3210
MP 7507    : %0100
             ;==================================================================
             ;MP 7600 .. 7699
             ;Hardware
             ;==================================================================
             ;MP7600.0 Position controller cycle time (only for CC 422)
             ;Input: 3 to 20 [MP7600.0 * 0.6 ms]
             ;       >= 1.8 ms
MP 7600.0  : 5          ;3
             ;MP7600.1 PLC cycle time (only for CC 422)
             ;Input: 1 to 20 [MP7600.0 * MP7600.1 * 0.6 ms]
             ;       >= 10.8 ms
MP 7600.1  : 7          ;11
             ;------------------------------------------------------------------
             ;MP7602 PLC cycle time (only for CC 424, CC 61xx)
             ;Input: 0 to 60 [ms]
             ;         CC 424: 0 = 10,8 ms
             ;         CC 61xx: 0 = 12 ms
MP 7602    : 20
             ;------------------------------------------------------------------
             ;MP7610 Control loop speed
             ;       (only for CC 424, CC 61xx with MP2000)
             ;Input: 0 = Single speed
             ;       1 = Double speed
             ;3210
MP 7610.0  : %1100
             ;Bit 0 - 6 or 10 control loops: X51/X53; 8 control loops: X51/X53
             ;Bit 1 - 6 or 10 control loops: X52/X54; 8 control loops: X52/X54
             ;Bit 2 - 6 or 10 control loops: X55; 8 control loops: X55/X57
             ;Bit 3 - 6 or 10 control loops: X56; 8 control loops: X56/X58
             ;3210
MP 7610.1  : %1111
             ;Bit 0 - 10 control loops: X57; 12 control loops: X59/X61
             ;Bit 1 - 10 control loops: X58; 12 control loops: X60/X62
             ;Bit 2 - 10 control loops: X59; 12 control loops: X63
             ;Bit 3 - 10 control loops: X60; 12 control loops: X64
             ;------------------------------------------------------------------
             ;MP7620 Feed-rate override and spindle speed override
             ;Input: %xxxx
             ;Bit 0   Feed rate override if rapid traverse key in
             ;        Program Run mode is pressed
             ;        0 = Override not effective
             ;        1 = Override effective
             ;Bit 1   Non-functional
             ;Bit 2   Feed rate override if rapid traverse key and machine
             ;        direction buttons pressed in Manual operating mode
             ;        0 = Override not effective
             ;        1 = Override effective
             ;Bit 3   Feed rate override and spindle speed override in
             ;        1 % steps or along a non-linear curve
             ;        0 = 1 % steps
             ;        1 = Non-linear curve
             ;Bit 4   Non-functional
             ;Bit 5   TE with potentiometer for rapid traverse override
             ;        instead of spindle speed override
             ;        Please refer to the notes in the Technical Manual
             ;        0 = TE with spindle speed override
             ;        1 = TE with rapid traverse override
             ;Bit 6   Feedrate smoothing
             ;        0 = Not active
             ;        1 = Active
             ;Bit 7   Only with Functional Safety:
             ;        Adaption of 100% position of potentiometers to F-LIMIT
             ;        0 = Not active
             ;        1 = Adapt 100% position to F-LIMIT,
             ;            100% is equivalent to active SLS limit
             ;Bit 8   Error message if feed rate or rapid traverse override
             ;        at 0 %
             ;        0 = Not active
             ;        1 = Active
             ;Bit 9   HSCI keyboard with three override potentiometers (feed
             ;        rate-, spindle speed- and rapid traverse override)
             ;        0 = Not active
             ;        1 = Active
             ;9876543210
MP 7620    : %0000001111
             ;------------------------------------------------------------------
             ;MP7621 Increments of the spindle override by the "+"- and "-"-
             ;       keys (just for corresponding operating panels)
             ;Input: 1 to 10 [%]
             ;         0 = 5 %
MP 7621    : 0
             ;------------------------------------------------------------------
             ;MP7630 Time after EMERGENCY STOP-Test to be in standby again
             ;Input: 1 to 999 [ms]
             ;       0 = previous behaviour (up to and including 34049x-02)
MP 7630    : 0
             ;------------------------------------------------------------------
             ;MP7640 Handwheel
             ;       MP7640.0 for X23 of the last machine operating panel
             ;       in the HSCI chain
             ;       MP7640.1 to .3 reserved
             ;       MP7640.4 for X29 of the MC
             ;Input: 0 = No handwheel
             ;       1 = HR330 (evaluate all keys by NC)
             ;       2 = HR130 or HR330
             ;       3 = HR330 (evaluate  keys +, - and
             ;           "rapid traverse" by PLC)
             ;       4 = HR332 (evaluate all keys by PLC)
             ;       5 = Up to three HR150 via HRA110
             ;       6 = HR410, HR510
             ;       7 = reserved
             ;       8 = reserved
             ;       9 = HR410FS, HR 510FS
             ;      10 = reserved
             ;      11 = HR420/HR520 without supporting the LEDs
             ;      12 = HR550
             ;      13 = HR520 with supporting the LEDs
MP 7640.0  : 0
MP 7640.1  : 0
MP 7640.2  : 0
MP 7640.3  : 0
MP 7640.4  : 0
             ;------------------------------------------------------------------
             ;MP7641 Functions of the handwheel
             ;Input: %xxxxxxxxx
             ;Bit 0   Entry of the interpolation factor (not HR 420, HR 5xx)
             ;        0 = Through TNC keyboard
             ;        1 = Through PLC Module 9036
             ;Bit 1   Handwheel with detent positions
             ;        0 = Without detent positions
             ;        1 = With detent positions
             ;Bit 2   Axis direction keys and rapid traverse (HR 420, HR 5xx)
             ;        0 = By the NC
             ;        1 = By the PLC
             ;Bit 3   NC start/NC stop on the HR 420, HR 5xx
             ;        0 = By the NC
             ;        1 = By the PLC
             ;        Handwheel superimpositioning in tool-axis direction
             ;        can be activated
             ;Bit 4   0 = Handwheel superimpositioning in tool-axis direction
             ;            not possible
             ;        1 = Handwheel superimpositioning in tool-axis direction
             ;            can be activated in global settings
             ;        Inactive behaviour of the HR420, HR 5xx
             ;Bit 5   0 = Keys on HR only sent to PLC when HR is active
             ;        1 = Keys on HR sent to PLC also, when the HR is
             ;            inactive
             ;Bit 6   Selecting and traversing auxiliary axes
             ;        with HR 420, HR 5xx
             ;        0 = not possible
             ;        1 = possible
             ;Bit 7   TeachIn button on HR 550
             ;        0 = by NC
             ;        1 = by PLC
             ;Bit 8   CTRL button on HR 550
             ;        0 = by NC
             ;        1 = by PLC
             ;Bit 9   PLC softkeys and HR 420, HR 5xx active
             ;        Please refer to the notes in the Technical Manual
             ;        0 = PLC softkeys inactive if HR active
             ;        1 = PLC-Softkeys active if HR active
             ;Bit 10  Additional charging station HRA 551FS for HR 550 allowed
             ;        0 = not allowed
             ;        1 = allowed
             ;09876543210
MP 7641    : %00000010000
             ;------------------------------------------------------------------
             ;MP7642 Handwheel name
             ;       MP7642.0 for X23 of the last machine operating panel
             ;       in the HSCI chain
             ;       MP7642.1 to .3 reserved
             ;       MP7642.4 for X29 of the MC
             ;Input: Handwheel name, max 31 characters
MP 7642.0  : 0
MP 7642.1  : 0
MP 7642.2  : 0
MP 7642.3  : 0
MP 7642.4  : 0
             ;------------------------------------------------------------------
             ;MP7645.0 Handwheel initialization (HR332/HR410/HRA110)
             ;         MP7645.x for X23 of the last machine operating panel
             ;         in the HSCI chain
             ;Input:
             ;HR332: Evaluation of handwheel keys and LED
             ;       0 = Axis keys by NC (other keys by PLC)
             ;       1 = Keys by PLC
             ;HR410: Evaluation of handwheel keys and LED
             ;       0 = Keys by NC (function keys FCT A-C by PLC)
             ;       1 = Keys by PLC
             ;HRA110: Assignment of the 3rd handwheel axis via
             ;        axis selection switch
             ;        0 = Left stop = Z axis
             ;        1 = Left stop = X axis
             ;        2 = Switch setting 3 = Z axis
MP 7645.0  : 0
             ;------------------------------------------------------------------
             ;MP7645.1 HRA110 initialization: Assignment of the 3rd
             ;         handwheel axis via MP
             ;Input:   0 = Simulation of switch setting 1 (left stop)
             ;         of the axis selection switch. Assignment of the 3rd
             ;         handwheel via MP 7645.0 (input value 0 or 1)
             ;         1 = X axis
             ;         2 = Y axis
             ;         4 = Z axis
             ;         8 = Axis 4
             ;        16 = Axis 5
MP 7645.1  : 4
             ;------------------------------------------------------------------
             ;MP7645.2 HRA110 initialization: Axis selection via axis selection
             ;switch or MP 7645.1
             ;Input: 0 = Axis selection: Axis selection switch
             ;       1 = Axis selection through MP 7645.1
MP 7645.2  : 0
             ;------------------------------------------------------------------
             ;MP7645.3 HR550 Behaviour relais contact permissive keys
             ;Input:   0 = 2 closer
             ;         1 = 1 closer / 1 opener (for FS necessary)
             ;         HRA5xx: Interpretation of permissive buttons
             ;         0 = both relays as normally open contacts
             ;         1 = one relays as normally open contact
             ;             one relays as normally closed contact
MP 7645.3  : 0
             ;------------------------------------------------------------------
MP 7645.4  : 0          ;Reserved
MP 7645.5  : 0          ;Reserved
MP 7645.6  : 0          ;Reserved
MP 7645.7  : 0          ;Reserved
             ;------------------------------------------------------------------
             ;MP7646.x reserved
MP 7646.0  : 0          ;Reserved
MP 7646.1  : 0          ;Reserved
MP 7646.2  : 0          ;Reserved
MP 7646.3  : 0          ;Reserved
MP 7646.4  : 0          ;Reserved
MP 7646.5  : 0          ;Reserved
MP 7646.6  : 0          ;Reserved
MP 7646.7  : 0          ;Reserved
             ;------------------------------------------------------------------
             ;MP7647.x reserved
MP 7647.0  : 0          ;Reserved
MP 7647.1  : 0          ;Reserved
MP 7647.2  : 0          ;Reserved
MP 7647.3  : 0          ;Reserved
MP 7647.4  : 0          ;Reserved
MP 7647.5  : 0          ;Reserved
MP 7647.6  : 0          ;Reserved
MP 7647.7  : 0          ;Reserved
             ;------------------------------------------------------------------
             ;MP7648.x reserved
MP 7648.0  : 0          ;Reserved
MP 7648.1  : 0          ;Reserved
MP 7648.2  : 0          ;Reserved
MP 7648.3  : 0          ;Reserved
MP 7648.4  : 0          ;Reserved
MP 7648.5  : 0          ;Reserved
MP 7648.6  : 0          ;Reserved
MP 7648.7  : 0          ;Reserved
             ;------------------------------------------------------------------
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
             ;        0 = Left stop = Z axis
             ;        1 = Left stop = X axis
             ;        2 = Switch setting 3 = Z axis
MP 7649.0  : 0
             ;------------------------------------------------------------------
             ;MP7649.1 HRA110 initialization: Assignment of the 3rd
             ;         handwheel axis via MP
             ;Input:   0 = Simulation of switch setting 1 (left stop)
             ;         of the axis selection switch. Assignment of the 3rd
             ;         handwheel via MP 7649.0 (input value 0 or 1)
             ;         1 = X axis
             ;         2 = Y axis
             ;         4 = Z axis
             ;         8 = Axis 4
             ;        16 = Axis 5
MP 7649.1  : 4
             ;------------------------------------------------------------------
             ;MP7649.2 HRA110 initialization: Axis selection via axis selection
             ;switch or MP 7649.1
             ;Input: 0 = Axis selection: Axis selection switch
             ;       1 = Axis selection through MP 7645.1
MP 7649.2  : 0
             ;------------------------------------------------------------------
             ;MP7649.3 HRA5xx: Interpretation of permissive buttons
             ;Input: 0 = both relays as normally open contacts
             ;       1 = one relays as normally open contact
             ;           one relays as normally closed contact
MP 7649.3  : 0
MP 7649.4  : 0          ;Reserved
MP 7649.5  : 0          ;Reserved
MP 7649.6  : 0          ;Reserved
MP 7649.7  : 0          ;Reserved
             ;------------------------------------------------------------------
             ;MP7650 Counting direction for handwheel
             ;       MP7650.0 for X23 of the last machine operating panel
             ;       in the HSCI chain
             ;       MP7650.1 to .3 reserved
             ;       MP7650.4 for X29 of the MC
             ;Input: 0 = Negative counting direction
             ;       1 = Positive counting direction
             ;2109876543210987654ZYX
MP 7650.0  : %0000000000000000000000
MP 7650.1  : %0000000000000000000000
MP 7650.2  : %0000000000000000000000
MP 7650.3  : %0000000000000000000000
MP 7650.4  : %0000000000000000000000
             ;------------------------------------------------------------------
             ;MP7660 Threshold sensitivity for electronic handwheel
             ;Input: 0 to 65 535 [increments]
MP 7660    : 10
             ;-----------------------------------------------------------------
             ;MP7662 Filter for smoothing the movement during handwheel super-
             ;       imposition
             ;       (M118 or Global Program Settings)
             ;Input: 0.000 to 1.000  [seconds]
MP 7662    : 0.1
             ;------------------------------------------------------------------
             ;MP7670 Minimum interpolation factor for handwheel
             ;Input:    0 to 10
MP 7670.0  : 10         ;For low speed
MP 7670.1  : 3          ;For medium speed (only HR 410)
MP 7670.2  : 0          ;For high speed (only HR 410)
             ;------------------------------------------------------------------
             ;MP7671 Manual feed rate in Handwheel operating mode with HR 410
             ;Input: 0 to 1000 [% of MP1020]
MP 7671.0  : 10         ;Low speed
MP 7671.1  : 50         ;Medium speed
MP 7671.2  : 100        ;High speed
             ;------------------------------------------------------------------
             ;MP7672 For HR 410 a path for every detent position can be set
             ;       if MP7641 Bit1 = 1
             ;Input: 0,0000 to 1,0000 [mm]
MP 7672.0  : 0.001      ;Low speed
MP 7672.1  : 0.01       ;Medium speed
MP 7672.2  : 0.1        ;High speed
             ;------------------------------------------------------------------
             ;MP7674 HR 130, HR 330, HR 332 always and HR 410,
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
MP 7674.14 : 0
MP 7674.15 : 0
MP 7674.16 : 0
MP 7674.17 : 0
MP 7674.18 : 0
MP 7674.19 : 0
MP 7674.20 : 0
MP 7674.21 : 0
             ;------------------------------------------------------------------
             ;MP7675 HR 130, HR 330, HR 332 of no significance
             ;       HR 4xx, HR 5xx if MP7641 Bit1 = 1
             ;       (with detent positions) a maximum axis-specific
             ;       path for every detent position in [mm] can be set
             ;       HR 420, HR 5xx if MP7641 Bit1 = 0
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
MP 7675.14 : 0
MP 7675.15 : 0
MP 7675.16 : 0
MP 7675.17 : 0
MP 7675.18 : 0
MP 7675.19 : 0
MP 7675.20 : 0
MP 7675.21 : 0
             ;------------------------------------------------------------------
             ;MP7680 Machine parameter with multiple function
             ;Input: %xxxxxxxxxxxxxx
             ;Bit 0   Memory function for axis direction keys
             ;        0 = Not saved
             ;        1 = Saved
             ;Bit 1   Returning to the contour
             ;        0 = Not active
             ;        1 = Active
             ;Bit 2   Block scan
             ;        0 = Not active
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
             ;Bit 10  Compensated outside corner with spline instead of circle
             ;        (corners remain sharp with nominal-value filtering)
             ;        0 = Not active
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
             ;Bit 16  approach/depart movements of cycle 39
             ;        0 = curved approach/depart movements active
             ;        1 = curved approach/depart movements activ not active
             ;Bit 17  Behavior of markers M4175, M4176
             ;        0 = M4175, M4176 cleared at an internal stop
             ;        1 = M4176 remains set during RUNCANCEL macro
             ;Bit 18  Manual measurement of basic rotation with inclined
             ;        rotary axes
             ;        0 = Error message "Axis angle not equal to tilt angle"
             ;            is active
             ;        1 = Error message is not active
             ;Bit 19  Modified algorithm for M120 (radius-compensated
             ;       contour look-ahead)
             ;       0 = Not active
             ;        1 = Active
             ;Bit 20  Error message: Retraction with active limit plane
             ;        not allowed
             ;        0 = Error message is active
             ;        1 = Error message is not active
             ;Bit 21  Error message in Cycles 25, 276:
             ;        Approaching or departing block collides with programmed
             ;        contour
             ;        0 = Error message is active
             ;        1 = Error message is not active
             ;Bit 22  Selection option for returning to the contour:
             ;        0 = Not active, same behavior as before 
             ;            (up to and including 60642x-04 SP11)
             ;        1 = Selection dialog for direct returning to the contour 
             ;            or via block scan is displayed
             ;21098765432109876543210
MP 7680    : %00000100000010001111111
             ;------------------------------------------------------------------
             ;MP7681 M/S/T/Q transfer to the PLC during block scan
             ;Input: %xxxx
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
             ;Bit 5   0 = READ FROM PLC KEY and WRITE TO PLC KEY are
             ;            not executed during a block scan
             ;            (behavior as before)
             ;        1 = READ FROM PLC KEY and WRITE TO PLC KEY are also
             ;            active during a block scan
             ;543210
MP 7681    : %110111
             ;------------------------------------------------------------------
             ;MP7682 Machine parameter with multiple function
             ;Input:% xxxxxxxxxxxxx
             ;Bit 0   Incremental block after Tool Call
             ;        0 = With length compensation
             ;        1 = Without length compensation
             ;Bit 1   Set reference value for datum
             ;        (Use MP7500 bit 5, since nominal-to-actual value
             ;        transfer may change the datum)
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
             ;        0 = "tilt working plane"-state from NC-Program will
             ;             not be transfered to "Manual Operation" during
             ;             program interruption
             ;        1 = "tilt working plane"-state will be transfered
             ;             to "Manual Operation" during program interruption
             ;Bit 10  Peripheral Milling
             ;        0 = Peripheral Milling permitted
             ;        1 = Peripheral Milling inactive
             ;Bit 11  reserved
             ;Bit 12  Behaviour for R2 > R in tool table
             ;        0 = R2 > R yields to error message
             ;        1 = error message disabled
             ;Bit 13  No program interruption upon invalid TOOL DEF
             ;        0 = Error message and NC stop upon invalid TOOL DEF
             ;        1 = Only warning upon invalid TOOL DEF
             ;Bit 14  No insertion of the tool if TIME2 has expired
             ;        0 = Tool is inserted even if TIME2 has expired
             ;        1 = TOOL CALL for a tool where TIME2 has expired
             ;            leads to an error message and NC stop
             ;Bit 15  reserved
             ;Bit 16  reserved
             ;Bit 17  Behavior of control-in-operation signal in the log at
             ;        program end
             ;        0 = Control-in-operation signal is deleted at program
             ;        end and is set again by the NC software by executing
             ;        the cancel macro (behavior as up to 60642x-03)
             ;        1 = Control-in-operation signal is deleted by the
             ;        NC software only after execution of the cancel macro
             ;Bit 18  Search for replacement tool without tool usage file
             ;        0 = Search using the tool name
             ;            (behavior as before)
             ;        1 = Completely use up the tool
             ;        	  The tool that was last used will be used
             ;        	  (most recent date/time in the LAST_USE column)         
             ;Bit 19  Mirroring rotary axes if M128/FUNCTION TCPM is active
             ;        0 = Also affects rotary axis coordinates that were not exp
             ;            programmed in the NC program, but were calculated from
             ;            programmed tool direction vectors (TX, TY, TZ) or from
             ;            spatial angles (FUNCTION TCPM AXIS SPAT).
             ;            Applies to mirroring via CYCL DEF 8 and mirroring
             ;            via Global Program Settings.
             ;            (Behavior as before)
             ;        1 = Does not affect rotary axis coordinates calculated fro
             ;            tool direction vectors and/or spatial angles.
             ;            Mirroring in linear axes also affects (as before)
             ;            tool direction vectors and spatial angles;
             ;            mirroring the rotary axes then affects only rotary axi
             ;            coordinates directly programmed in the NC program.
             ;            (Same behavior as with TNC 640 as of SW 34059x-07))
             
             ;98765432109876543210
MP 7682    : %00000000001000000111
             ;------------------------------------------------------------------
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
             ;            Do not display Softkey EDIT PALETTE
             ;        1 = Active pallet table can be edited
             ;            Display Softkey EDIT PALETTE
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
             ;Bit 9   EDIT PALETTE soft key in pallet machining
             ;        0 = Do not display
             ;        1 = Display
             ;9876543210
MP 7683    : %0100111110
             ;------------------------------------------------------------------
             ;MP7684 Nominal position value filter and path control with M128
             ;Input: %xxxxxxxx
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
             ;        0 = Include jerk- and tolerance limits at curvature
             ;            changes
             ;        1 = Do not include jerk- and tolerance limits at
             ;            curvature changes
             ;Bit 5   reserved
             ;Bit 6   reserved
             ;Bit 7   reserved
             ;Bit 8   reserved
             ;Bit 9   Traversing with high rate of movement by rotation axes
             ;        in conjunction with M128 (movement smaller 2 degree
             ;        and M118 inactive
             ;        0 = previous behaviour
             ;        1 = active
             ;Bit 10  Modification of the calculation of contouring feed
             ;        rate at the beginning of a contour-element
             ;        0 = active
             ;        1 = previous behaviour (up to and including 340490-02)
             ;09876543210
MP 7684    : %01000000000
             ;------------------------------------------------------------------
             ;MP7690 Evaluation of the electronic ID labels
             ;Bit 0   HEIDENHAIN power modules
             ;        0 = Active
             ;        1 = Not active
             ;Bit 1   HEIDENHAIN synchronous motors
             ;        0 = Active
             ;        1 = Not active
             ;Bit 2   HEIDENHAIN power supply units
             ;        0 = Active
             ;        1 = Not active
             ;210
MP 7690    : %000
             ;------------------------------------------------------------------
             ;MP7691 Memory for diagnostics
             ;Input: 0 to 10 (function depends on diagnosis - see comment)
             ;       0 = inactive
MP 7691.0  : 0          ; operating system trace = volume of circular buffer in
             ;            Mbyte
MP 7691.1  : 0          ; TCPdump = 10 files with [MP7691.1] MByte
MP 7691.2  : 0          ; NC-console log = 10 files with 10*[MP7691.2] kByte
MP 7691.3  : 0          ; kernel output log = 10 files with 10*[MP7691.3] kByte
             ;==================================================================
             ;MP 13000 .. 13999
             ;Spindle control (2nd spindle)
             ;==================================================================
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
MP 13010   : 6
             ;------------------------------------------------------------------
             ;MP13011 Function of analog output s if MP13010 < 3
             ;Input: 0 = No special function (0 volt)
             ;       1 = Voltage proportional to the current machining feed
             ;           rate (depending on MP13012)
             ;       2 = Voltage is defined via the PLC module 9130
             ;       3 = Voltage is defined through M functions
             ;           (M200 to M204)
MP 13011   : 0
             ;------------------------------------------------------------------
             ;MP13012 Feed rate from output of an analog voltage of 10 V
             ;(MP13011 = 1)
             ;Input: 0 to 300 000 [mm/min]
             ;Laser power control M200-M204
MP 13012   : 0          ;0 to 30 000 m/min = 10 volts
             ;------------------------------------------------------------------
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
             ;------------------------------------------------------------------
             ;MP13014.0 to MP13014.11 Characteristic curve kink points for
             ;output of the analog voltage with M202
             ;Input: 0 to 9.999 [V]
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
             ;------------------------------------------------------------------
             ;MP13020 Definition of speed range of BCD spindle
             ;Input: 0 to 99 999
             ;Limitation of speed codes (no limitation with 01991)
             ;    1* increment (1..9)
             ;+  10* max. code (1..99)
             ;+1000* min. code (1..99)
MP 13020   : 1991
             ;------------------------------------------------------------------
             ;MP13030 Axis stop with a Tool Call in which only a
             ;spindle speed is transferred
             ;Input: %xx
             ;Bit 0   Axis stops with Tool Call
             ;        0 = Active
             ;        1 = Not active
             ;Bit 1   Regulate to speed 0 when switching to another
             ;        gear range
             ;        0 = Active
             ;        1 = Not active
             ;10
MP 13030   : %00
             ;------------------------------------------------------------------
             ;MP13120 Speed 0 permitted
             ;Input:  0: S = 0 permitted
             ;        1: S = 0 not permitted
MP 13120   : 1
             ;------------------------------------------------------------------
             ;MP13130 Polarity of the S analog voltage
             ;Input:  0 = M03 positive voltage; M04 negative voltage
             ;        1 = M03 negative voltage; M04 positive voltage
             ;        2 = M03 and M04 positive voltage
             ;        3 = M03 and M04 negative voltage
MP 13130   : 0
             ;------------------------------------------------------------------
             ;MP13140 Counting direction of the encoder signals for the spindle
             ;Input:  0 = Positive counting direction with M03
             ;        1 = Negative counting direction with M03
MP 13140   : 1
             ;------------------------------------------------------------------
             ;MP13142 Line count of the rotary encoder on the spindle
             ;Input:  100 to 100000 [lines]
MP 13142   : 2048
             ;------------------------------------------------------------------
             ;MP13143 Mounting configuration of the spindle position encoder
             ;Input: 0 = Direct
             ;       1 = Transmission (reference pulse from input X30)
             ;       2 = Transmission (reference end position of input X30 and
             ;           reference pulse from encoder, not possible if
             ;           transmission is located between ROD and spindle)
             ;       3 = Transmission (2nd reference pulse from input X30)
             ;       4 = Referencing via EnDat (not possible if
             ;           transmission is located between ROD and spindle)
             ;       5 = like input = 0, but evaluation of transmission
MP 13143   : 0
             ;------------------------------------------------------------------
             ;MP13210 S digital: Motor revolutions at rated speed
             ;Input:  0 to 100 [1000/min]
             ;
             ;         S analog : S analog voltage at rated speed
             ;         Input: 0 to 9.999 [V]
MP 13210.0 : 9          ;1st gear range
MP 13210.1 : 9          ;2nd gear range
MP 13210.2 : 0          ;3rd gear range
MP 13210.3 : 0          ;4th gear range
MP 13210.4 : 0          ;5th gear range
MP 13210.5 : 0          ;6th gear range
MP 13210.6 : 0          ;7th gear range
MP 13210.7 : 0          ;8th gear range
             ;------------------------------------------------------------------
             ;MP13240.1 S digital: Minimum possible output speed of motor
             ;Input:    0 to 9.999 [1000/min]
             ;
             ;          S analog : Minimum possible S analog output voltage
             ;          Input: 0 to 9.999 [V]
MP 13240.1 : 0.05
             ;------------------------------------------------------------------
             ;MP13240.2 S digital: Motor speed for gear changing
             ;Input:    0 to 9.999 [1000/min]
             ;
             ;          S analog : Jog voltage for gear changing
             ;          Input: 0 to 9.999 [V]
MP 13240.2 : 0.1
             ;------------------------------------------------------------------
             ;MP13310 S override limitation
             ;Input:  0 to 100 [%]
MP 13310.0 : 150        ;Maximum
MP 13310.1 : 75         ;Minimum
             ;------------------------------------------------------------------
             ;MP13350 Maximum permissible overshoot of the spindle rpm for
             ;        monitoring the spindle rpm, as a percentage of the
             ;        nominal speed value
             ;Input: 0 to 100 [%]
MP 13350   : 0
             ;------------------------------------------------------------------
             ;MP13351 Entry of an absolute value for the permissible
             ;        overshoot of the rpm. This value is only used if the
             ;        absolute value resulting from MP13350 is less than the
             ;        the value entered for MP13351.
             ;        (for low nominal speeds)
             ;Input: 0,001 to 100000,000 [rpm]
             ;       0 = Monitoring deactivated
MP 13351   : 0
             ;------------------------------------------------------------------
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
             ;------------------------------------------------------------------
             ;MP13412 Multiplier for MP13411
             ;Input:  0.000 to 1.999
MP 13412.0 : 1          ;With M03, M04, M05
MP 13412.1 : 1          ;For oriented spindle stop
MP 13412.2 : 1          ;For tapping
MP 13412.3 : 1          ;For rigid tapping
             ;------------------------------------------------------------------
             ;MP13415 Spindle overshoot behavior
             ;Input:  0 to 1 000 [ms]
MP 13415.0 : 100        ;With M03, M04, M05
MP 13415.1 : 100        ;For oriented spindle stop
MP 13415.2 : 100        ;For tapping
MP 13415.3 : 100        ;For rigid tapping
             ;------------------------------------------------------------------
             ;MP13420 Positioning window for the spindle
             ;Input:  0 to 360 [degrees]
MP 13420   : 0.5
             ;------------------------------------------------------------------
             ;MP13430 Deviation of the reference mark from the desired
             ;position (spindle preset)
             ;Input:  0 to 360 [degrees]
MP 13430   : 0
             ;------------------------------------------------------------------
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
             ;------------------------------------------------------------------
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
             ;------------------------------------------------------------------
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
             ;------------------------------------------------------------------
             ;MP13510 Rated speed for gear ranges
             ;Input: 0 to 99 999.999 [rpm]
MP 13510.0 : 3000       ;1st gear range
MP 13510.1 : 30000      ;2nd gear range
MP 13510.2 : 0          ;3rd gear range
MP 13510.3 : 0          ;4th gear range
MP 13510.4 : 0          ;5th gear range
MP 13510.5 : 0          ;6th gear range
MP 13510.6 : 0          ;7th gear range
MP 13510.7 : 0          ;8th gear range
             ;------------------------------------------------------------------
             ;MP13515 Maximum spindle speed
             ;Input: 0 to 99 999.999 [rpm]
MP 13515.0 : 3000       ;1st gear range
MP 13515.1 : 30000      ;2nd gear range
MP 13515.2 : 0          ;3rd gear range
MP 13515.3 : 0          ;4th gear range
MP 13515.4 : 0          ;5th gear range
MP 13515.5 : 0          ;6th gear range
MP 13515.6 : 0          ;7th gear range
MP 13515.7 : 0          ;8th gear range
             ;------------------------------------------------------------------
             ;MP13520.0 Speed activation through marker M4011
             ;Input: 0 to 99 999.999 [rpm]
MP 13520.0 : 10
             ;------------------------------------------------------------------
             ;MP13520.1 Spindle speed for oriented stop
             ;Input: 0 to 99 999.999 [rpm]
MP 13520.1 : 500
             ;------------------------------------------------------------------
             ;MP13530 Increased spindle power for roughing
             ;Input: 0 = not active
             ;       1 = increased spindle power for roughing
MP 13530   : 0
             ;------------------------------------------------------------------
             ;MP13540 Zulässige Drehzahlunterschreitung der
             ;       Spindel-Istdrehzahl über einen Prozent-Faktor
             ;       von der Solldrehzahl
             ;Eingabe: 0 bis 100 [%]
             ;         0 = Überwachung nicht aktiv
MP 13540   : 0
             ;------------------------------------------------------------------
             ;MP13542 Mindest-Spindeldrehzahl, ab der die Überwachung
             ;       in MP3540 aktiv wird
             ;Eingabe: 0,001 bis 99 999,999 [U/min]
             ;         0 = Überwachung nicht aktiv
MP 13542   : 0
             ;------------------------------------------------------------------
             ;MP13550 Verzögerung der NOT-AUS-Reaktion von Spindeln
             ;Eingabe: 0,001 bis 0,100 [s]
             ;         0 = Verzögerung nicht aktiv
MP 13550   : 0
