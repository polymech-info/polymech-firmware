0  BEGIN PGM 332 MM 
1  * - Tuning
2  ; Assignment in PLC:\MP\MPFragment\MPFragment.A
3  ;
4  * - Tuning Mode
5  ; Q395=0 - Tuning off (Standard)
6  ; Q395=1 - Exact
7  ; Q395=2 - Smooth
8  ; Q395=3 - Rough
9  ;
10 ; Q396= Tolerance T
11 ;
12 ; Q397= Angle Tolerance TA
13 ;
14 * - Tuning Mode off (Standard)
15 FN 9: IF +Q395 EQU +0 GOTO LBL "STANDARD"
16 ;
17 * - Tuning Mode Exact
18 FN 9: IF +Q395 EQU +1 GOTO LBL "EXACT"
19 ;
20 * - Tuning Mode Smooth
21 FN 9: IF +Q395 EQU +2 GOTO LBL "SMOOTH"
22 ;
23 * - Tuning Mode Rough
24 FN 9: IF +Q395 EQU +3 GOTO LBL "ROUGH"
25 FN 9: IF +0 EQU +0 GOTO LBL "MP1202"
26 ;
27 * - Activate PLC:\MP\MPFragment\Tuning\Standard.a
28 LBL "STANDARD"
29 FN 17: SYSWRITE ID 1020 NR1 =+0
30 FN 19: PLC =+99993 / +0
31 FN 9: IF +0 EQU +0 GOTO LBL "MP1202"
32 ;
33 * - Activate PLC:\MP\MPFragment\Tuning\Exact.a
34 LBL "EXACT"
35 FN 17: SYSWRITE ID 1020 NR1 =+1
36 FN 19: PLC =+99993 / +1
37 FN 9: IF +0 EQU +0 GOTO LBL "MP1202"
38 ;
39 * - Activate PLC:\MP\MPFragment\Tuning\Smooth.a
40 LBL "SMOOTH"
41 FN 17: SYSWRITE ID 1020 NR1 =+2
42 FN 19: PLC =+99993 / +2
43 FN 9: IF +0 EQU +0 GOTO LBL "MP1202"
44 ;
45 * - Activate PLC:\MP\MPFragment\Tuning\Rough.a
46 LBL "ROUGH"
47 FN 17: SYSWRITE ID 1020 NR1 =+3
48 FN 19: PLC =+99993 / +3
49 FN 9: IF +0 EQU +0 GOTO LBL "MP1202"
50 ;
51 * - Read MP1202.0 Cycle 32 3D milling - predefined tolerance
52 LBL "MP1202"
53 FN 10: IF +Q396 NE +0 GOTO LBL "NO_MP1202"
54 FN 18: SYSREAD Q396 = ID1000 NR1202 IDX0
55 LBL "NO_MP1202"
56 ;
57 * - define Tolerance Angle TA
58 FN 10: IF +Q397 NE +0 GOTO LBL "TA"
59 Q397 = 0.1
60 LBL "TA"
61 ;
62 * - Tuning Mode Rough
63 FN 9: IF +Q395 EQU +3 GOTO LBL "HSC1"
64 ;
65 * - Tuning Mode Standard, Smooth, Exact
66 FN 10: IF +Q395 NE +3 GOTO LBL "HSC0"
67 ;
68 * - Cycle 32 for Tuning Mode Rough
69 LBL "HSC1"
70 CYCL DEF 32.0 TOLERANZ
71 CYCL DEF 32.1 TQ396
72 CYCL DEF 32.2 HSC-MODE:1 TAQ397
73 FN 9: IF +0 EQU +0 GOTO LBL "END"
74 ;
75 * - Cycle 32 for Tuning Mode Standard, Exact, Smooth
76 LBL "HSC0"
77 CYCL DEF 32.0 TOLERANZ
78 CYCL DEF 32.1 TQ396
79 CYCL DEF 32.2 HSC-MODE:0 TAQ397
80 FN 9: IF +0 EQU +0 GOTO LBL "END"
81 ;
82 LBL "END"
83 END PGM 332 MM 
