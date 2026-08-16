0  BEGIN PGM TO_CLAMP MM 
1  WRITE TO PLC KEY "WG_Nummer_aktives_NC_Macro" =+1005 ; NUMBER NC MACRO ON
2  *
3  * - Safety high
4  *
5  FN 18: SYSREAD Q11 = ID510 NR5 IDX3 ; Safety high Z programmed
6  FN 9: IF +Q11 EQU +0 GOTO LBL 1
7  FN 18: SYSREAD Q14 = ID510 NR6 IDX3 ; Safety high Z
8  L  Z+Q14 FMAX
9  LBL 1
10 FN 18: SYSREAD Q12 = ID510 NR5 IDX2 ; Safety high Y programmed
11 FN 9: IF +Q12 EQU +0 GOTO LBL 2
12 FN 18: SYSREAD Q15 = ID510 NR6 IDX2 ; Safety high Y
13 L  Y+Q15 FMAX
14 LBL 2
15 FN 18: SYSREAD Q13 = ID510 NR5 IDX1 ; Safety high X programmed
16 FN 9: IF +Q13 EQU +0 GOTO LBL 3
17 FN 18: SYSREAD Q16 = ID510 NR6 IDX1 ; Safety high X
18 L  X+Q16 FMAX
19 LBL 3
20 *
21 * - Next position fixture
22 *
23 ; L IA+90 FMAX
24 WRITE TO PLC KEY "WG_Nummer_aktives_NC_Macro" =-1 ; NC MACRO OFF
25 END PGM TO_CLAMP MM 
