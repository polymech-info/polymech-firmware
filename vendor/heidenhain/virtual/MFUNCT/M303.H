0  BEGIN PGM M303 MM 
1  * - Compensation of W-Axis
2  WRITE TO PLC KEY "WG_Nummer_aktives_NC_Macro" =+303 ; NUMBER NC MACRO ON
3  FN 18: SYSREAD Q0 = ID240 NR8 IDX9
4  FN 32: AXIS2:+Q0
5  WRITE TO PLC KEY "WG_Nummer_aktives_NC_Macro" =-1 ; NC MACRO OFF
6  END PGM M303 MM 
