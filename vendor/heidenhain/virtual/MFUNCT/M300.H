0  BEGIN PGM M300 MM 
1  * - Reset Compensation of U,V,W-Axes
2  WRITE TO PLC KEY "WG_Nummer_aktives_NC_Macro" =+300 ; NUMBER NC MACRO ON
3  FN 32: AXIS0:+0 AXIS1:+0 AXIS2:+0
4  WRITE TO PLC KEY "WG_Nummer_aktives_NC_Macro" =-1 ; NC MACRO OFF
5  END PGM M300 MM 
