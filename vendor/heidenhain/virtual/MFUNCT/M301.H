0  BEGIN PGM M301 MM 
1  * - Compensation of U-Axis
2  WRITE TO PLC KEY "WG_Nummer_aktives_NC_Macro" =+301 ; NUMBER NC MACRO ON
3  FN 18: SYSREAD Q0 = ID240 NR8 IDX7
4  FN 32: AXIS0:+Q0
5  WRITE TO PLC KEY "WG_Nummer_aktives_NC_Macro" =-1 ; NC MACRO OFF
6  END PGM M301 MM 
