0  BEGIN PGM TO_PALLET MM 
1  WRITE TO PLC KEY "WG_Nummer_aktives_NC_Macro" =+1007 ; NUMBER NC MACRO ON
2  ;Dummy for pallet status in column W-STATUS
3  ;Test MP7480 >= 3
4  FN 18: SYSREAD Q1 = ID1000 NR7480 IDX0
5  FN 11: IF +Q1 GT +2 GOTO LBL "TC_ACTIVE"
6  FN 14: ERROR= 902
7  LBL "TC_ACTIVE"
8  ;In case a real pallet changer is existing FN19 and M60 necessary
9  ;FN 18: SYSREAD Q0 = ID510 NR2 IDX0 ; PAL No
10 ;FN 19: PLC =+Q0 / +0
11 ;M60
12 ;
13 WRITE TO PLC KEY "WG_Nummer_aktives_NC_Macro" =-1 ; NC MACRO OFF
14 END PGM TO_PALLET MM 
