*-----------------------------------------------------------------------------
*  Macro Bibliothek
*-----------------------------------------------------------------------------
*  Macro's für die Fehlermeldungen
*
*-----------------------------------------------------------------------------

#define     KL_max_Nummer_Fehler_Queue               K +4
#define     KL_nicht_benutzt                         K +0



#define /m    M_Fehler_Hirth_Soll_Position  L  PN_Fehler_Hirth_Soll_Position \
  R      PN_Fehler_Hirth_Soll_Position                                       \
  IFT                                                                        \
    PS     NP_M4203_Fehler_Modul_9xxx                                        \
    PS     K &PN_Fehler_Hirth_Soll_Position - 4800                           \
    LX                                                                       \
    +      K &SG_Kennung_Achse                                               \
    PSW                                                                      \
    LX                                                                       \
    +      K+1                                                               \
    PSW                                                                      \
    PS     KL_nicht_benutzt                                                  \
    CM     9084                                                              \                        ;PLC-Fehlermeldung anzeigen
    PL     NP_M4203_Fehler_Modul_9xxx                                        \
  ENDI



#define /m    M_Fehler_Hirth_Position  L  PN_Fehler_Hirth_Position          \
  R      PN_Fehler_Hirth_Position                                           \
  IFT                                                                       \
    PS     NP_M4203_Fehler_Modul_9xxx                                       \
    PS     K &PN_Fehler_Hirth_Position - 4800                               \
    LX                                                                      \
    +      K &SG_Kennung_Achse                                              \
    PSW                                                                     \
    LX                                                                      \
    +      K+1                                                              \
    PSW                                                                     \
    PS     KL_nicht_benutzt                                                 \
    CM     9084                                                             \                        ;PLC-Fehlermeldung anzeigen
    PL     NP_M4203_Fehler_Modul_9xxx                                       \
  ENDI



#define /m    M_Fehler_Motor_Tmax  L  PN_Fehler_Motor_Tmax                  \
  R      PN_Fehler_Motor_Tmax                                               \
  IFT                                                                       \
    PS     NP_M4203_Fehler_Modul_9xxx                                       \
    PS     K &PN_Fehler_Motor_Tmax - 4800                                   \
    LX                                                                      \
    +      K &SG_Kennung_Achse                                              \
    PSW                                                                     \
    LX                                                                      \
    +      K+1                                                              \
    PSW                                                                     \
    PS     KL_nicht_benutzt                                                 \
    CM     9084                                                             \                        ;PLC-Fehlermeldung anzeigen
    PL     NP_M4203_Fehler_Modul_9xxx                                       \
  ENDI



#define /m    M_Fehler_Leistungteil_Tmax  L  PN_Fehler_Leistungteil_Tmax    \
  R      PN_Fehler_Leistungteil_Tmax                                        \
  IFT                                                                       \
    PS     NP_M4203_Fehler_Modul_9xxx                                       \
    PSX                                                                     \
    L      K-1                                                              \
    =X                                                                      \
    REPEAT                                                                  \
      INCX                                                                  \
      L      MG_Leistungsteil_X_Tmax[X]                                     \
    UNTILT                                                                  \
    PS     K &PN_Fehler_Leistungteil_Tmax - 4800                            \
    LX                                                                      \
    +      K &SG_Kennung_Achse                                              \
    PSW                                                                     \
    LX                                                                      \
    +      K+1                                                              \
    PSW                                                                     \
    PS     KL_nicht_benutzt                                                 \
    CM     9084                                                             \                        ;PLC-Fehlermeldung anzeigen
    PLX    \
    PL     NP_M4203_Fehler_Modul_9xxx\
  ENDI



#define /m    M_Fehler_I2t_Vorwarnung  L  PN_Fehler_I2t_Vorwarnung          \
  R      PN_Fehler_I2t_Vorwarnung                                           \
  IFT                                                                       \
    PS     NP_M4203_Fehler_Modul_9xxx                                       \
    PSX                                                                     \
    L      K-1                                                              \
    =X                                                                      \
    REPEAT                                                                  \
      INCX                                                                  \
      L      MG_I2t_Vorwarnung[X]                                           \
    UNTILT                                                                  \
    PS     K &PN_Fehler_I2t_Vorwarnung - 4800                               \
    LX                                                                      \
    +      K &SG_Kennung_Achse                                              \
    PSW                                                                     \
    LX                                                                      \
    +      K+1                                                              \
    PSW                                                                     \
    PS     KL_nicht_benutzt                                                 \
    CM     9084                                                             \                        ;PLC-Fehlermeldung anzeigen
    PLX    \
    PL     NP_M4203_Fehler_Modul_9xxx\
  ENDI



#define /m    M_Fehler_I2t_Begrenzung  L  PN_Fehler_I2t_Begrenzung          \
  R      PN_Fehler_I2t_Begrenzung                                           \
  IFT                                                                       \
    PS     NP_M4203_Fehler_Modul_9xxx                                       \
    PSX                                                                     \
    L      K-1                                                              \
    =X                                                                      \
    REPEAT                                                                  \
      INCX                                                                  \
      L      MG_I2t_Begrenzung[X]                                           \
    UNTILT                                                                  \
    PS     K &PN_Fehler_I2t_Begrenzung - 4800                               \
    LX                                                                      \
    +      K &SG_Kennung_Achse                                              \
    PSW                                                                     \
    LX                                                                      \
    +      K+1                                                              \
    PSW                                                                     \
    PS     KL_nicht_benutzt                                                 \
    CM     9084                                                             \                        ;PLC-Fehlermeldung anzeigen
    PLX    \
    PL     NP_M4203_Fehler_Modul_9xxx\
  ENDI



#define /m    M_Fehler_Antriebe_betriebsbereit  L  PN_Fehler_Antriebe_betriebsbereit \
  R      PN_Fehler_Antriebe_betriebsbereit                                           \
  IFT                                                                                \
    PS     NP_M4203_Fehler_Modul_9xxx                                                \
    PSX                                                                              \
    L      K-1                                                                       \
    =X                                                                               \
    REPEAT                                                                           \
      INCX                                                                           \
      L      DG_Antriebe_bereit_MC                                                   \
      A      DG_digitale_Antriebe                                                    \
      XO     DG_Strom_Drehzahl_Regler_ein                                            \
      BTX                                                                            \
      O[                                                                             \
        LX                                                                           \
        >=     KG_Spindel                                                            \
      ]                                                                              \
    UNTILT                                                                           \
    PS     K &PN_Fehler_Antriebe_betriebsbereit - 4800                               \
    LN     I_Antriebe_betriebsbereit                                                 \
    IFT                                                                              \
      PS     K &SG_Kennung_Achsen_Limitx                                             \
      PS     K+0                                                                     \
    ELSE                                                                             \
      LX                                                                             \
      +      K &SG_Kennung_Achse                                                     \
      PSW                                                                            \
      LX                                                                             \
      +      K+1                                                                     \
      PSW                                                                            \
    ENDI                                                                             \
    PS     KL_nicht_benutzt                                                          \
    CM     9084                                                                      \                        ;PLC-Fehlermeldung anzeigen
    PLX    \
    PL     NP_M4203_Fehler_Modul_9xxx\
  ENDI



#define /m    M_Fehler_Achs_Klemmung     L     PN_Fehler_Klemmung          \
  R      PN_Fehler_Klemmung                                                \
  =      MG_Fehler_Klemmung[X]                                             \
  L      MG_Fehler_Klemmung[X]                                             \
  AN     MG_Bremsen_Test                                                   \
  IFT                                                                      \
    PS     NP_M4203_Fehler_Modul_9xxx                                      \
    PS     K &PN_Fehler_Klemmung - 4800                                    \
    LX                                                                     \
    +      K &SG_Kennung_Achse                                             \
    PSW                                                                    \
    LX                                                                     \
    +      K+1                                                             \
    PSW                                                                    \
    PS     KL_nicht_benutzt                                                \
    CM     9084                                                            \                        ;PLC-Fehlermeldung anzeigen
    PL     NP_M4203_Fehler_Modul_9xxx                                      \
  ENDI



#define /m    M_Fehler_CC_Abschaltung     L     PN_Fehler_CC_Abschaltung   \
  R      PN_Fehler_CC_Abschaltung                                          \
  IFT                                                                      \
    PS     NP_M4203_Fehler_Modul_9xxx                                      \
    PS     K &PN_Fehler_CC_Abschaltung - 4800                              \
    LX                                                                     \
    +      K &SG_Kennung_Achse                                             \
    PSW                                                                    \
    LX                                                                     \
    +      K+1                                                             \
    PSW                                                                    \
    PS     KL_nicht_benutzt                                                \
    CM     9084                                                            \                        ;PLC-Fehlermeldung anzeigen
    PL     NP_M4203_Fehler_Modul_9xxx                                      \
  ENDI



#define /m    M_Fehler_X15x_Antriebs_Freigabe(Achsgruppe) L PN_Fehler_X15x_Antriebs_Freigabe \
  R      PN_Fehler_X15x_Antriebs_Freigabe                                                    \
  IFT                                                                                        \
    PS     NP_M4203_Fehler_Modul_9xxx                                                        \
    PS     K &PN_Fehler_X15x_Antriebs_Freigabe - 4800                                        \
    PS     S""                                                                               \
    L      (Achsgruppe)                                                                      \
    INCW                                                                                     \
    PSW                                                                                      \
    PS     KL_nicht_benutzt                                                                  \
    CM     9084                                                                              \                        ;PLC-Fehlermeldung anzeigen
    PL     NP_M4203_Fehler_Modul_9xxx                                                        \
  ENDI



#define /m    M_Modul_Fehler_Tabelle(Zeile,Datei_Name) L  NP_M4203_Fehler_Modul_9xxx        \
  IFT                                                                                       \
    PS     NP_M4203_Fehler_Modul_9xxx                                                       \
    PSX                                                                                     \
    PS     K &PN_Fehler_Modul_9255_Tab - 4800                                               \
    PS     S"(Datei_Name)"                                                                  \
    PS     NP_W1016_Nummer_fehlerhaftes_Modul                                               \
    PS     (Zeile)                                                                          \
    CM     9084                                                                             \                        ;PLC-Fehlermeldung anzeigen
    PLX    \
    PL     NP_M4203_Fehler_Modul_9xxx\
  ENDI



#define /m    M_Modul_Fehler_anzeigen L  NP_M4203_Fehler_Modul_9xxx            \
  IFT                                                                          \
    L      BG_Modul_Fehler_Queue                                               \
    <      KL_max_Nummer_Fehler_Queue                                          \
    IFT                                                                        \
      S      MG_Fehler_Modul_9xxx_Submit                                       \
      INC    BG_Modul_Fehler_Queue                                             \
      PS     NP_M4203_Fehler_Modul_9xxx                                        \
      PSX                                                                      \
      L      K &PN_Fehler_Modul_9xxx_1 - 4801                                  \
      +      BG_Modul_Fehler_Queue                                             \
      PSW                                                                      \
      L      NP_W1022_Modul_Fehlerstatus                                       \
      =X                                                                       \
      L      S#D900[X]                                                         \
      =      SG_Modul_Fehler_Status                                            \
      PS     K &SG_Modul_Fehler_Status                                         \
      PS     NP_W1016_Nummer_fehlerhaftes_Modul                                \
      PS     NP_W1022_Modul_Fehlerstatus                                       \
      CM     9084                                                              \                      ;PLC-Fehlermeldung anzeigen
      PLX    \
      PL     NP_M4203_Fehler_Modul_9xxx\
    ENDI     \
  ENDI



#define /m    M_Modul_Fehler_Win_anzeigen(M_Win) L  NP_M4203_Fehler_Modul_9xxx \
  IFT                                                                          \
    L      BG_Modul_Fehler_Queue                                               \
    <      KL_max_Nummer_Fehler_Queue                                          \
    IFT                                                                        \
      INC    BG_Modul_Fehler_Queue                                             \
      PS     NP_M4203_Fehler_Modul_9xxx                                        \
      PSX                                                                      \
      L      K &PN_Fehler_Modul_9xxx_1 - 4801                                  \
      +      BG_Modul_Fehler_Queue                                             \
      PSW                                                                      \
      L      NP_W1022_Modul_Fehlerstatus                                       \
      =X                                                                       \
      L      S#D900[X]                                                         \
      =      SG_Modul_Fehler_Status                                            \
      PS     K &SG_Modul_Fehler_Status                                         \
      PS     NP_W1016_Nummer_fehlerhaftes_Modul                                \
      PS     NP_W1022_Modul_Fehlerstatus                                       \
      CM     9084                                                              \                      ;PLC-Fehlermeldung anzeigen
      L      MG_Eins_Merker                                                    \
      S      (M_Win)                                                           \
      PLX                                                                      \
      PL     NP_M4203_Fehler_Modul_9xxx                                        \
    ENDI                                                                       \
  ENDI


#define /m    M_Pos_Flanke(ISig,OImp,Aux)                                   \
  L      (ISig)                                                             \
  AN     (Aux)                                                              \
  =      (OImp)                                                             \
  L      (ISig)                                                             \
  =      (Aux)


#define /m    M_Neg_Flanke(ISig,OImp,Aux)                                   \
  LN     (ISig)                                                             \
  AN     (Aux)                                                              \
  =      (OImp)                                                             \
  LN     (ISig)                                                             \
  =      (Aux)


