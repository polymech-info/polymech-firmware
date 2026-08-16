0  BEGIN PGM M302 MM 
1  * - Compensation of V-Axis
2  WRITE TO PLC KEY "WG_Nummer_aktives_NC_Macro" =+302 ; NUMBER NC MACRO ON
3  FN 18: SYSREAD Q0 = ID240 NR8 IDX8
4  FN 32: AXIS1:+Q0
5  WRITE TO PLC KEY "WG_Nummer_aktives_NC_Macro" =-1 ; NC MACRO OFF
6  END PGM M302 MM 
