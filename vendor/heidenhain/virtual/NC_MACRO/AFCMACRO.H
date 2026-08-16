0  BEGIN PGM AFCMACRO MM 
1  WRITE TO PLC KEY "WG_Nummer_aktives_NC_Macro" =+1004 ; NUMBER NC MACRO ON
2  ;aktuelle Toolnummer lesen
3  FN 18: SYSREAD Q1 = ID20 NR1
4  ;maximale Standzeit bei Tool Call lesen
5  FN 18: SYSREAD Q2 = ID50 NR10 IDXQ1
6  ;aktuelle Standzeit auf maximal setzen
7  FN 17: SYSWRITE ID 50 NR11 IDXQ1 =+Q2
8  ;Schwesterwerkzeug einwechseln, anpositionieren
9  FN 17: SYSWRITE ID 990 NR11 =+0
10 WRITE TO PLC KEY "WG_Nummer_aktives_NC_Macro" =-1 ; NC MACRO OFF
11 END PGM AFCMACRO MM 
