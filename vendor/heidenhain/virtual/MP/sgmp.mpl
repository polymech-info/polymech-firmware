             ; Maschinenparameter für iTNC530 mit integrierten
             ; Personenschutz
             ; ID 340422/340490
             ; Stand:  12.12.2016
             
             ;===================================
             ;= PARAMETER FÜR SICHERE STEUERUNG =
             ;===================================

             ; Bezeichnung der Achsen
             ; Verfahrbereich 1, 2, 3
MP 100.0    :
MP 100.1    :
MP 100.2    :

             ; Zuordnung der Achsen zu den Regler-Basisplatinen
             ;      (nur CC 6xxx)
             ;Eingabe: 0..4 = Nummer der Regler-Basisplatine im HSCI-Strang
             ;                Nur die Reihenfolge der Regler-Basisplatinen
             ;                im HSCI-System wirkt sich hier auf 0..4 aus
             ;                1. Regler-Basisplatine erh„lt Adr. 0
MP 108.0   :
MP 108.1   :
MP 108.2   :
MP 108.3   :
MP 108.4   :
MP 108.5   :
MP 108.6   :
MP 108.7   :
MP 108.8   :
MP 108.9   :
MP 108.10  :
MP 108.11  :
MP 108.12  :
MP 108.13  :
MP 108.14  :
MP 108.15  :
MP 108.16  :
MP 108.17  :
MP 108.18  :
MP 108.19  :
MP 108.20  :
MP 108.21  :


             ;--------------------------------------------------------------
             ; Zuordnung der Spindeln zu den Regler-Basisplatinen
             ;        (nur CC 6xxx)
             ;Eingabe: 0..4 = Nummer der Regler-Basisplatine im HSCI-Strang
             ;                Nur die Reihenfolge der Regler-Basisplatinen
             ;                im HSCI-System wirkt sich hier auf 0..4 aus
             ;                1. Regler-Basisplatine erh„lt Adr. 0
MP 109.0   :
MP 109.1   :

             ; Zuordnung Lagemesssytem-Eingänge zu Achsen
             ; Eingabe: 0        = kein Lagemesssystem-Eingang
             ;          1..6     = Lagemesssystem-Eingang X1..X6
             ;          35..38   = Lagemesssystem-Eingang X35..X38
             ;          201..210 = Lagemesssystem-Eingang X201..X210
MP 110.0   :
MP 110.1   :
MP 110.2   :
MP 110.3   :
MP 110.4   :
MP 110.5   :
MP 110.6   :
MP 110.7   :
MP 110.8   :
MP 110.9   :
MP 110.10  :
MP 110.11  :
MP 110.12  :
MP 110.13  :
MP 110.14  :
MP 110.15  :
MP 110.16  :
MP 110.17  :
MP 110.18  :
MP 110.19  :
MP 110.20  :
MP 110.21  :

   
             ;MP111 Zuordnung Lagemesssytem-Eingänge für Spindeln
             ;Eingabe: 0        = kein Lagemesssystem-Eingang
             ;         1..6     = Lagemesssystem-Eingang X1..X6
             ;         35..38   = Lagemesssystem-Eingang X35..X38
             ;         201..210 = Lagemesssystem-Eingang X201..X210
MP 111.0   :
MP 111.1   :

             ;MP112 Zuordnung Drehzahl-Messsytem-Eingänge zu Achsen
             ;      (nur bei CC 422)
             ;Eingabe: 0      = kein Drehzahl-Messsystem-Eingang
             ;         15..20 = Drehzahl-Messsystem-Eingang X15..X20
             ;         80..85 = Drehzahl-Messsystem-Eingang X80..X85
MP 112.0   :
MP 112.1   :
MP 112.2   :
MP 112.3   :
MP 112.4   :
MP 112.5   :
MP 112.6   :
MP 112.7   :
MP 112.8   :
MP 112.9   :
MP 112.10  :
MP 112.11  :
MP 112.12  :
MP 112.13  :
MP 112.14  :
MP 112.15  :
MP 112.16  :
MP 112.17  :
MP 112.18  :
MP 112.19  :
MP 112.20  :
MP 112.21  :

             ;Bit4=1:sicherheits-info aus Lagegeber Bit5=1:Sicherheits-info aus Motorgeber
MP 118.0   :
MP 118.1   :
MP 118.2   :
MP 118.3   :
MP 118.4   :
MP 118.5   :
MP 118.6   :
MP 118.7   :
MP 118.8   :
MP 118.9   :
MP 118.10  :
MP 118.11  :
MP 118.12  :
MP 118.13  :
MP 118.14  :
MP 118.15  :
MP 118.16  :
MP 118.17  :
MP 118.18  :
MP 118.19  :
MP 118.20  :
MP 118.21  :

             ;MP113 Zuordnung Drehzahl-Messsytem-Eingänge für Spindeln
             ;Eingabe: 0      = kein Drehzahl-Messsystem
             ;         15..20 = Drehzahl-Messsystem-Eingang X15..X20
             ;         80..85 = Drehzahl-Messsystem-Eingang X80..X85
MP 113.0   :
MP 113.1   :

             ;MP120 Zuordnung Drehzahl-Sollwert-Ausgänge zu Achsen
             ;Achtung Zuordnung ist fest vorgegeben (.0 -> x1)
             ;Eingabe: 0      = keine geregelte Achse
             ;         1..6   = analog-Ausgänge X8 1..6
             ;         7..13  = analog-Ausgänge X9 7..13
             ;         51..62 = digitale-Ausgänge X51..X62
MP 120.0   :
MP 120.1   :
MP 120.2   :
MP 120.3   :
MP 120.4   :
MP 120.5   :
MP 120.6   :
MP 120.7   :
MP 120.8   :
MP 120.9   :
MP 120.10  :
MP 120.11  :
MP 120.12  :
MP 120.13  :
MP 120.14  :
MP 120.15  :
MP 120.16  :
MP 120.17  :
MP 120.18  :
MP 120.19  :
MP 120.20  :
MP 120.21  :

             ;MP121 Zuordnung Drehzahl-Sollwert-Ausgänge zu Spindeln
             ;Eingabe: 0      = keine geregelte Spindel
             ;         1..6   = analog-Ausgänge X8 1..6
             ;         7..13  = analog-Ausgänge X9 7..13
             ;         51..62 = digitale-Ausgänge X51..X62
MP 121.0   :
MP 121.1   :
            
             ; Index y der Maschinen-Parameter MP2xxx.y für die Achsen
             ; Eingabe: 0 bis 17
MP 130.0   :
MP 130.1   :
MP 130.2   :
MP 130.3   :
MP 130.4   :
MP 130.5   :
MP 130.6   :
MP 130.7   :
MP 130.8   :
MP 130.9   :
MP 130.10  :
MP 130.11  :
MP 130.12  :
MP 130.13  :
MP 130.14  :
MP 130.15  :
MP 130.16  :
MP 130.17  :
MP 130.18  :
MP 130.19  :
MP 130.20  :
MP 130.21  :

             ; Index y der Maschinen-Parameter MP2xxx.y für die Spindel
             ; in Betriebsart 0
             ; Eingabe: 0 bis 17
MP 131.0   :
MP 131.1   :

             ; MP132 Index y der Maschinen-Parameter MP2xxx.y für die Spindel
             ; in Betriebsart 1
             ; Eingabe: 0 bis 17
MP 132.0   :
MP 132.1   :

             ;Signalperiode wird von der TNC automatisch berechnet:
             ;Signalperiode = MP331 / MP332
             ;MP331 = Strecke für Zählimpulse aus MP332
             ;Eingabe: 0 bis 99 999,9999 [mm/Grad]
MP 331.0   :
MP 331.1   :
MP 331.2   :
MP 331.3   :
MP 331.4   :
MP 331.5   :
MP 331.6   :
MP 331.7   :
MP 331.8   :
MP 331.9   :
MP 331.10  :
MP 331.11  :
MP 331.12  :
MP 331.13  :
MP 331.14  :
MP 331.15  :
MP 331.16  :
MP 331.17  :
MP 331.18  :
MP 331.19  :
MP 331.20  :
MP 331.21  :

             ;MP332 Anzahl der Zählimpulse auf der Strecke aus MP331
             ;Eingabe: 1 bis 16 777 215
MP 332.0   :
MP 332.1   :
MP 332.2   :
MP 332.3   :
MP 332.4   :
MP 332.5   :
MP 332.6   :
MP 332.7   :
MP 332.8   :
MP 332.9   :
MP 332.10  :
MP 332.11  :
MP 332.12  :
MP 332.13  :
MP 332.14  :
MP 332.15  :
MP 332.16  :
MP 332.17  :
MP 332.18  :
MP 332.19  :
MP 332.20  :
MP 332.21  :
          
             ; max. Zeit für Test der Abschaltkanäle ( Impuls löschen )
             ; Eingabe: 1..4320 [min] (4320min = 72h)
MP 511     :
             ; Datensatz-ID fuer globalen Datensatz (Index 0), SPLC-Datensatz (Index 1) und dann
             ; abwechselnd Achsdatensatz und Motordatensatz für alle Achsen
             ; Dieser Parameter ist nur für SPLC und wird von der NC gesetzt
MP 513.0   :
MP 513.1   :
MP 513.2   :
MP 513.3   :
MP 513.4   :
MP 513.5   :
MP 513.6   :
MP 513.7   :
MP 513.8   :
MP 513.9   :
MP 513.10  :
MP 513.11  :
MP 513.12  :
MP 513.13  :
MP 513.14  :
MP 513.15  :
MP 513.16  :
MP 513.17  :
MP 513.18  :
MP 513.19  :
MP 513.20  :
MP 513.21  :
MP 513.22  :
MP 513.23  :
MP 513.24  :
MP 513.25  :
MP 513.26  :
MP 513.27  :
MP 513.28  :
MP 513.29  :
MP 513.30  :
MP 513.31  :
MP 513.32  :
MP 513.33  :
MP 513.34  :
MP 513.35  :
MP 513.36  :
MP 513.37  :
MP 513.38  :
MP 513.39  :
MP 513.40  :
MP 513.41  :
MP 513.42  :
MP 513.43  :
MP 513.44  :
MP 513.45  :
MP 513.46  :
MP 513.47  :
MP 513.48  :
MP 513.49  :
             ; Datensatz-CRC fuer globalen Datensatz (Index 0), SPLC-Datensatz (Index 1) und dann
             ; abwechselnd Achsdatensatz und Motordatensatz für alle Achsen
             ; Dieser Parameter wird von der NC gesetzt
MP 514.0   :
MP 514.1   :
MP 514.2   :
MP 514.3   :
MP 514.4   :
MP 514.5   :
MP 514.6   :
MP 514.7   :
MP 514.8   :
MP 514.9   :
MP 514.10  :
MP 514.11  :
MP 514.12  :
MP 514.13  :
MP 514.14  :
MP 514.15  :
MP 514.16  :
MP 514.17  :
MP 514.18  :
MP 514.19  :
MP 514.20  :
MP 514.21  :
MP 514.22  :
MP 514.23  :
MP 514.24  :
MP 514.25  :
MP 514.26  :
MP 514.27  :
MP 514.28  :
MP 514.29  :
MP 514.30  :
MP 514.31  :
MP 514.32  :
MP 514.33  :
MP 514.34  :
MP 514.35  :
MP 514.36  :
MP 514.37  :
MP 514.38  :
MP 514.39  :
MP 514.40  :
MP 514.41  :
MP 514.42  :
MP 514.43  :
MP 514.44  :
MP 514.45  :
MP 514.46  :
MP 514.47  :
MP 514.48  :
MP 514.49  :
             ; Datensatz abgenommen fuer globalen Datensatz (Index 0), SPLC-Datensatz (Index 1) und dann
             ; abwechselnd Achsdatensatz und Motordatensatz für alle Achsen
             ; Dieser Parameter ist nur für SPLC und wird von der NC gesetzt
             ; 0 = Datensatz nicht abgenommen
             ; 1 = Datensatz ist abgenommen
MP 515.0   :
MP 515.1   :
MP 515.2   :
MP 515.3   :
MP 515.4   :
MP 515.5   :
MP 515.6   :
MP 515.7   :
MP 515.8   :
MP 515.9   :
MP 515.10  :
MP 515.11  :
MP 515.12  :
MP 515.13  :
MP 515.14  :
MP 515.15  :
MP 515.16  :
MP 515.17  :
MP 515.18  :
MP 515.19  :
MP 515.20  :
MP 515.21  :
MP 515.22  :
MP 515.23  :
MP 515.24  :
MP 515.25  :
MP 515.26  :
MP 515.27  :
MP 515.28  :
MP 515.29  :
MP 515.30  :
MP 515.31  :
MP 515.32  :
MP 515.33  :
MP 515.34  :
MP 515.35  :
MP 515.36  :
MP 515.37  :
MP 515.38  :
MP 515.39  :
MP 515.40  :
MP 515.41  :
MP 515.42  :
MP 515.43  :
MP 515.44  :
MP 515.45  :
MP 515.46  :
MP 515.47  :
MP 515.48  :
MP 515.49  :
             ; Maschinen-ID für Datensatz fuer globalen Datensatz (Index 0), SPLC-Datensatz (Index 1) und dann
             ; abwechselnd Achsdatensatz und Motordatensatz für alle Achsen
             ; Maschinentyp für Datensatz nur SPLC
             ; Dieser Parameter ist nur für SPLC und wird ab Index 1 von der NC gesetzt
MP 516.0   :
MP 516.1   :
MP 516.2   :
MP 516.3   :
MP 516.4   :
MP 516.5   :
MP 516.6   :
MP 516.7   :
MP 516.8   :
MP 516.9   :
MP 516.10  :
MP 516.11  :
MP 516.12  :
MP 516.13  :
MP 516.14  :
MP 516.15  :
MP 516.16  :
MP 516.17  :
MP 516.18  :
MP 516.19  :
MP 516.20  :
MP 516.21  :
MP 516.22  :
MP 516.23  :
MP 516.24  :
MP 516.25  :
MP 516.26  :
MP 516.27  :
MP 516.28  :
MP 516.29  :
MP 516.30  :
MP 516.31  :
MP 516.32  :
MP 516.33  :
MP 516.34  :
MP 516.35  :
MP 516.36  :
MP 516.37  :
MP 516.38  :
MP 516.39  :
MP 516.40  :
MP 516.41  :
MP 516.42  :
MP 516.43  :
MP 516.44  :
MP 516.45  :
MP 516.46  :
MP 516.47  :
MP 516.48  :
MP 516.49  :

             ; Stillsetz-Zeiten Stopp1 für Achsen achsspzifisch nur SPLC
             ; Eingabe 0.000...50.000 [sec]
MP 525.0   :
MP 525.1   :
MP 525.2   :
MP 525.3   :
MP 525.4   :
MP 525.5   :
MP 525.6   :
MP 525.7   :
MP 525.8   :
MP 525.9   :
MP 525.10  :
MP 525.11  :
MP 525.12  :
MP 525.13  :
MP 525.14  :
MP 525.15  :
MP 525.16  :
MP 525.17  :
MP 525.18  :
MP 525.19  :
MP 525.20  :
MP 525.21  :

             ; Stillsetz-Zeiten Stopp1 für Spindeln nur SPLC
             ; Eingabe 0.000...50.000 [sec]
MP 526     :

             ; Stillsetzzeiten Stopp2 für Achsen achsspzifisch nur SPLC
             ; Eingabe 0.000...2.000 [sec]
MP 527.0   :
MP 527.1   :
MP 527.2   :
MP 527.3   :
MP 527.4   :
MP 527.5   :
MP 527.6   :
MP 527.7   :
MP 527.8   :
MP 527.9   :
MP 527.10  :
MP 527.11  :
MP 527.12  :
MP 527.13  :
MP 527.14  :
MP 527.15  :
MP 527.16  :
MP 527.17  :
MP 527.18  :
MP 527.19  :
MP 527.20  :
MP 527.21  :

             ; Stillsetzzeiten Stopp2 für Spindeln nur SPLC
             ; Eingabe 0.000...10.000 [sec]
MP 528     :

             ; Verzögerungszeit für Fehlerauslösung durch 
             ; dv/dt-Überwachung, wenn Achse nicht abbremst.              
             ; Eingabe: 0,000 bis 10,000 [s]
MP 530.0   :
MP 530.1   :
MP 530.2   :
MP 530.3   :
MP 530.4   :
MP 530.5   :
MP 530.6   :
MP 530.7   :
MP 530.8   :
MP 530.9   :
MP 530.10  :
MP 530.11  :
MP 530.12  :
MP 530.13  :
MP 530.14  :
MP 530.15  :
MP 530.16  :
MP 530.17  :
MP 530.18  :
MP 530.19  :
MP 530.20  :
MP 530.21  :

             ; Zeit für SPLC-Timer (T0 bis T16 in s) nur fuer SPLC
MP 535.0   :
MP 535.1   :
MP 535.2   :
MP 535.3   :
MP 535.4   :
MP 535.5   :
MP 535.6   :
MP 535.7   :
MP 535.8   :
MP 535.9   :
MP 535.10  :
MP 535.11  :
MP 535.12  :
MP 535.13  :
MP 535.14  :
MP 535.15  :
MP 535.18  :
MP 535.19  :
MP 535.20  :
MP 535.21  :
 
             ; Sicher reduzierte Geschwindigkeit SLS_3 für Achsen in den
             ; Betriebsarten BA3/BA4
             ; Eingabe: 0...5000 [mm/min]
MP 540.0   :
MP 540.1   :
MP 540.2   :
MP 540.3   :
MP 540.4   :
MP 540.5   :
MP 540.6   :
MP 540.7   :
MP 540.8   :
MP 540.9   :
MP 540.10  :
MP 540.11  :
MP 540.12  :
MP 540.13  :
MP 540.14  :
MP 540.15  :
MP 540.16  :
MP 540.17  :
MP 540.18  :
MP 540.19  :
MP 540.20  :
MP 540.21  :

             ; Sicher reduzierte Spindeldrehzahl SLS_3 in den
             ; Betriebsarten BA3/BA4
             ; Eingabe: 0...6000 [U/min]
MP 541     :

             ; Stillstandsfenster für Überwachung im Zustand SOS
             ; (Sicherer Betriebshalt) und bei Notaus.
             ; Eingabe: 0,0010...30,0000 [mm bzw. Grad]
MP 545.0   :
MP 545.1   :
MP 545.2   :
MP 545.3   :
MP 545.4   :
MP 545.5   :
MP 545.6   :
MP 545.7   :
MP 545.8   :
MP 545.9   :
MP 545.10  :
MP 545.11  :
MP 545.12  :
MP 545.13  :
MP 545.14  :
MP 545.15  :
MP 545.16  :
MP 545.17  :
MP 545.18  :
MP 545.19  :
MP 545.20  :
MP 545.21  :

             ; Achsspezifischer Grenzwert fuer Sicher begrenztes Schrittmass
MP 547.0   :
MP 547.1   :
MP 547.2   :
MP 547.3   :
MP 547.4   :
MP 547.5   :
MP 547.6   :
MP 547.7   :
MP 547.8   :
MP 547.9   :
MP 547.10  :
MP 547.11  :
MP 547.12  :
MP 547.13  :
MP 547.14  :
MP 547.15  :
MP 547.16  :
MP 547.17  :
MP 547.18  :
MP 547.19  :
MP 547.20  :
MP 547.21  :

             ; Achsgruppenspezifische Konfiguration fuer die SS2-Reaktion: SOS oder STO
             ; (Anwendung zum Beispiel bei Drehbearbeitung).
             ;      0: Achsen in SOS und Spindeln in STO nach SS2 default
             ;      1: Die Achsgruppe ist in STO nach SS2
             ;      2: Die Achsgruppe ist in SOS nach SS2
MP 549.0   :
MP 549.1   :
MP 549.2   :
MP 549.3   :
MP 549.4   :
MP 549.5   :
MP 549.6   :
MP 549.7   :

             ; Stillstandsüberwachung für Hilfsachsen
             ; Eingabe: 0,0010...30,0000 [mm bzw. Grad]
MP 550.0   :
MP 550.1   :
MP 550.2   :
MP 550.3   :
MP 550.4   :
MP 550.5   :
MP 550.6   :
MP 550.7   :
MP 550.8   :
MP 550.9   :
MP 550.10  :
MP 550.11  :
MP 550.12  :
MP 550.13  :
MP 550.14  :
MP 550.15  :
MP 550.16  :
MP 550.17  :
MP 550.18  :
MP 550.19  :
MP 550.20  :
MP 550.21  :
    
             ; Sonderbetrieb Spindel: Sicher reduzierte Spindeldrehzahl SLS_4
             ; in der Betriebsart BA4
             ; Eingabe: 0...6000 [U/min]
MP 551     :

             ; Sonderbetrieb Achsen: Sicher reduzierte Geschwindigkeit SLS_4
             ; in der Betriebsart BA4
             ; Eingabe: 0...5000 [mm/min] bzw. [Grad/min]
MP 552.0   :
MP 552.1   :
MP 552.2   :
MP 552.3   :
MP 552.4   :
MP 552.5   :
MP 552.6   :
MP 552.7   :
MP 552.8   :
MP 552.9   :
MP 552.10  :
MP 552.11  :
MP 552.12  :
MP 552.13  :
MP 552.14  :
MP 552.15  :
MP 552.16  :
MP 552.17  :
MP 552.18  :
MP 552.19  :
MP 552.20  :
MP 552.21  :

             ; Anzeige-Modus für Drehachsen und PLC-Hilfsachsen
             ; Eingabe: 0,0000...99.999,9999 [Grad]
             ;          0 = kein Modulo
             ;          Software-Endschalter aktiv
             ;          >0 = Modulo-Wert für Anzeige
             ;          Software-Endschalter inaktiv
MP 555.0   :
MP 555.1   :
MP 555.2   :
MP 555.3   :
MP 555.4   :
MP 555.5   :
MP 555.6   :
MP 555.7   :
MP 555.8   :
MP 555.9   :
MP 555.10  :
MP 555.11  :
MP 555.12  :
MP 555.13  :
MP 555.14  :
MP 555.15  :
MP 555.16  :
MP 555.17  :
MP 555.18  :
MP 555.19  :
MP 555.20  :
MP 555.21  :


             ; Maximalwert für die Stillstandsüberwachung bei SS2 für Spindel 1 in der Betriebsart SOM_2
             ; Eingabe: 1…100 [Umdrehungen]
             ; Defaultwert: 2 [Umdrehungen]
MP 556     :

             ; Maximalwert für die Stillstandsüberwachung bei SS2 für Spindel 1 in der Betriebsart SOM_3
             ; Eingabe: 1…100 [Umdrehungen]
             ; Defaultwert: 5 [Umdrehungen]
MP 557     :

             ; Maximalwert für die Stillstandsüberwachung bei SS2 für Spindel 1 in der Betriebsart SOM_4
             ; Eingabe: 1…100 [Umdrehungen]
             ; Defaultwert: 5 [Umdrehungen]
MP 558     :

             ;MP560 Zusatzfunktionen
             ;Eingabe: %xxxxxxxxxxxxx
             ;Bit0     Sonderbetriebsart (Zustimmtaste nur für Spindelstart;
             ;         über MP551 einstellbare Spindeldrehzahl und über
             ;         MP552.x einstellbarer Vorschub)
             ;         0 = Nicht aktiv
             ;         1 = Aktiv
             ;Bit1     Abschalttest nach NETZ EIN
             ;         0 = Schutztüren müssen geschlossen sein
             ;         1 = Schutztüren müssen nicht geschlossen sein
             ;Bit2     Nach extern Stopp, S = 0 und Schliessen der
             ;         Schutztüre
             ;         0 = NC-Start durch Tastendruck
             ;         1 = NC-Start durch Tastendruck oder PLC
             ;Bit3     Test der Motorbremse auf Schluss 0 V und 24 V
             ;         0 = Test der Motorbremse aktiv
             ;         1 = Test der Motorbremse inaktiv
             ;         (T.BRK nicht verdrahtet)
             ;Bit4     Reaktion beim Öffnen der Schutztüre, wenn ein
             ;         Abschalttest durchgeführt wird
             ;         0 = Stop3
             ;         1 = Kein Stop3
             ;Bit5     Strommessung aller Antriebe vor Pruefung auf Impuls-
             ;         löschung im Abschalttest
             ;         0 = Strommessung wird durchgeführt
             ;         1 = keine Strommessung
             ;Bit6     Abschalttest achsspezifisch durchführen
             ;         0 = Test achsspezifisch durchführen
             ;         1 = alle Achsen gemeinsam abschalten und testen
             ;Bit7     1 Kein ständiges Drücken der Zustimmtaste am Handrad notwendig
             ;Bit8     1 Verdrahtung der Sicherheitsrelais nicht vorhanden
             ;Bit9     1 In der Maschinenbetriebsart Handrad ist ein gleichzeitiges verfahren von mehreren
             ;           Achsen möglich z.B. Ausgleichsbewegung (nur in SOM3 und SOM4 relevant).
             ;Bit10    1 No test of STO.A.PIC at test cutout channels
             ;           (necessary for DRIVE-CLiq inverters)
             ;Bit11    1 No dv/dt-monitoring for spindle drives at SS1 switch off
             ;Bit12    1 Test WD.x.SPL is omited
MP 560     :

             ;	Zulässige Zeit für Ungleichheit von sicheren Eingängen
	     ;  Eingabe: 0,1 … 30[s]
	     ;  Defaultwert: 5[s]
MP 584     :

             ; SPLC-Eingaenge mit inverser logik fuer Kanal A (MC) (z.B. NC-Stoptaste) nur fuer SPLC
MP 585.0   :
MP 585.1   :
MP 585.2   :
MP 585.3   :
MP 585.4   :
MP 585.5   :
MP 585.6   :
MP 585.7   :

             ; SPLC-Eingaenge mit inverser logik fuer Kanal B (CC) (z.B. NC-Stoptaste) nur fuer SPLC
MP 586.0   :
MP 586.1   :
MP 586.2   :
MP 586.3   :
MP 586.4   :
MP 586.5   :
MP 586.6   :
MP 586.7   :

             ; SPLC-Eingaenge, die von einem Testausgang versorgt werden (fuer dynamische Tests) nur fuer SPLC
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

             ; Sicher reduzierte Geschwindigkeit SLS_2 für Achsen in der
             ; Betriebsart BA2
             ; Eingabe: 0...2000 [mm/min]
MP 590.0   :
MP 590.1   :
MP 590.2   :
MP 590.3   :
MP 590.4   :
MP 590.5   :
MP 590.6   :
MP 590.7   :
MP 590.8   :
MP 590.9   :
MP 590.10  :
MP 590.11  :
MP 590.12  :
MP 590.13  :
MP 590.14  :
MP 590.15  :
MP 590.16  :
MP 590.17  :
MP 590.18  :
MP 590.19  :
MP 590.20  :
MP 590.21  :

             ; Sicher reduzierte Spindeldrehzahl SLS_2 in der
             ; Betriebsart BA2
             ; Eingabe: 0...2000 [U/min]
MP 591     :

             ; Zuordnung von NC-Achsen und Hilfsachsen zu Achsgruppen. nur fuer SPLC
             ; Eingabe 0...7 max. 8 Achsgruppen für sichere Achsen möglich
             ; Eingabe -1 d.h. Achse ist nicht sicher
MP 600.0   :
MP 600.1   :
MP 600.2   :
MP 600.3   :
MP 600.4   :
MP 600.5   :
MP 600.6   :
MP 600.7   :
MP 600.8   :
MP 600.9   :
MP 600.10  :
MP 600.11  :
MP 600.12  :
MP 600.13  :
MP 600.14  :
MP 600.15  :
MP 600.16  :
MP 600.17  :
MP 600.18  :
MP 600.19  :
MP 600.20  :
MP 600.21  :

             ; Zuordnung von Spindeln zu Achsgruppen. nur fuer SPLC
             ; Eingabe 0...7 max. 8 Achsgruppen für sichere Spindeln möglich
             ; Eingabe -1 d.h. Spindel ist nicht sicher
MP 601.0   :
MP 601.1   :

             ; Bremsreihenfolge der Achsgruppen nur fuer SPLC
             ; Eingabe bitcodiert in der jeweiligen Achsgruppe
MP 610.0   :
MP 610.1   :
MP 610.2   :
MP 610.3   :
MP 610.4   :
MP 610.5   :
MP 610.6   :
MP 610.7   :

            ; Zeit bis zum nächsten Bremsentest
            ; Eingabe: 0 bis 60000 [min]
MP 620.0   :
MP 620.1   :
MP 620.2   :
MP 620.3   :
MP 620.4   :
MP 620.5   :
MP 620.6   :
MP 620.7   :
MP 620.8   :
MP 620.9   :
MP 620.10  :
MP 620.11  :
MP 620.12  :
MP 620.13  :
MP 620.14  :
MP 620.15  :
MP 620.16  :
MP 620.17  :
MP 620.18  :
MP 620.19  :
MP 620.20  :
MP 620.21  :


            ; Maximale Drehzahlabweichung in % von der Solldrehzahl für Achsen
            ; Soll-IstwertÜberwachung mit Drehzahlwerten
            ; Eingabe: 0 bis 20 [%] von der aktuellen Solldrehzahl
MP 630.0   :
MP 630.1   :
MP 630.2   :
MP 630.3   :
MP 630.4   :
MP 630.5   :
MP 630.6   :
MP 630.7   :
MP 630.8   :
MP 630.9   :
MP 630.10  :
MP 630.11  :
MP 630.12  :
MP 630.13  :
MP 630.14  :
MP 630.15  :
MP 630.16  :
MP 630.17  :
MP 630.18  :
MP 630.19  :
MP 630.20  :
MP 630.21  :

            ; Maximale Drehzahlabweichung in % von der Solldrehzahl für Spindel
            ; Soll-IstwertÜberwachung mit Drehzahlwerten
            ; Eingabe: 0 bis 20 [%] von der aktuellen Solldrehzahl
MP 631     :

            ; Delay-Time für Soll-/Istwert-Überwachung mit Drehzahlwerten für Achsen
            ; Eingabe: 0...60 [s]
            ; Defaultwert: 10 [s]
MP 632.0   :
MP 632.1   :
MP 632.2   :
MP 632.3   :
MP 632.4   :
MP 632.5   :
MP 632.6   :
MP 632.7   :
MP 632.8   :
MP 632.9   :
MP 632.10  :
MP 632.11  :
MP 632.12  :
MP 632.13  :
MP 632.14  :
MP 632.15  :
MP 632.16  :
MP 632.17  :
MP 632.18  :
MP 632.19  :
MP 632.20  :
MP 632.21  :

            ; Delay-Time für Soll-/Istwert-Überwachung mit Drehzahlwerten für Spindel
            ; Eingabe: 0...60 [s]
            ; Defaultwert: 10 [s]
MP 633     :

             ; maximal zulässige Soll-Istwert-Abweichung bei offener Schutztuere
             ; Eingabe: 0,0000...+30,0000 [mm] bzw. [Grad]
MP 641.0   :
MP 641.1   :
MP 641.2   :
MP 641.3   :
MP 641.4   :
MP 641.5   :
MP 641.6   :
MP 641.7   :
MP 641.8   :
MP 641.9   :
MP 641.10  :
MP 641.11  :
MP 641.12  :
MP 641.13  :
MP 641.14  :
MP 641.15  :
MP 641.16  :
MP 641.17  :
MP 641.18  :
MP 641.19  :
MP 641.20  :
MP 641.21  :

             ; Max. Positionsabweichung beim REF-Fahren zwischen MCU und CCU
             ; Eingabe: 0,000...30,000 [mm]
MP 642.0   :
MP 642.1   :
MP 642.2   :
MP 642.3   :
MP 642.4   :
MP 642.5   :
MP 642.6   :
MP 642.7   :
MP 642.8   :
MP 642.9   :
MP 642.10  :
MP 642.11  :
MP 642.12  :
MP 642.13  :
MP 642.14  :
MP 642.15  :
MP 642.16  :
MP 642.17  :
MP 642.18  :
MP 642.19  :
MP 642.20  :
MP 642.21  :

             ; Position an der vom Bediener die Übereinstimmung von
             ; tatsächlicher Position und intern verwendeten Lagewerten
             ; geprüft werden kann.
             ; Eingabe: -30.000,000...+30.000,000 [mm] bzw. [Grad]
MP 646.0   :
MP 646.1   :
MP 646.2   :
MP 646.3   :
MP 646.4   :
MP 646.5   :
MP 646.6   :
MP 646.7   :
MP 646.8   :
MP 646.9   :
MP 646.10  :
MP 646.11  :
MP 646.12  :
MP 646.13  :
MP 646.14  :
MP 646.15  :
MP 646.16  :
MP 646.17  :
MP 646.18  :
MP 646.19  :
MP 646.20  :
MP 646.21  :
            
             ; Pos. absoluter Lagegrenzwert
             ; Eingabe: -30.000,000...+30.000,000 [mm] bzw. [Grad]
MP 650.0   :
MP 650.1   :
MP 650.2   :
MP 650.3   :
MP 650.4   :
MP 650.5   :
MP 650.6   :
MP 650.7   :
MP 650.8   :
MP 650.9   :
MP 650.10  :
MP 650.11  :
MP 650.12  :
MP 650.13  :
MP 650.14  :
MP 650.15  :
MP 650.16  :
MP 650.17  :
MP 650.18  :
MP 650.19  :
MP 650.20  :
MP 650.21  :

             ; Neg. absoluter Lagegrenzwert
             ; Eingabe: -30.000,000...+30.000,000 [mm] bzw. [Grad]
MP 670.0   :
MP 670.1   :
MP 670.2   :
MP 670.3   :
MP 670.4   :
MP 670.5   :
MP 670.6   :
MP 670.7   :
MP 670.8   :
MP 670.9   :
MP 670.10  :
MP 670.11  :
MP 670.12  :
MP 670.13  :
MP 670.14  :
MP 670.15  :
MP 670.16  :
MP 670.17  :
MP 670.18  :
MP 670.19  :
MP 670.20  :
MP 670.21  :

             ; Inbetriebnahme-Flag: 1: keine CRC Pruefung des SPLC-Programms nur fuer SPLC
MP 690     :

             ; CRC-Summen des SPLC-Programms: CRC intermediate code,
             ; CRC MC code, CRC CC code nur fuer SPLC
MP 691.0   :
MP 691.1   :
MP 691.2   :

             ; Version der SPLC-API, die zur Erstellung des SPLCProgramms verwendet wurde
MP 693     :

             ;Parameter für zwei Bremsen an einer Achse
             ;Bremsentest: Art der Ansteuerung -2=umrichter -1=pl-modul 0..n pl-modul mit 0V-Ausgang
             ;1. Bremse
MP 830.0   :
MP 830.1   :
MP 830.2   :
MP 830.3   :
MP 830.4   :
MP 830.5   :
MP 830.6   :
MP 830.7   :
MP 830.8   :
MP 830.9   :
MP 830.10  :
MP 830.11  :
MP 830.12  :
MP 830.13  :
MP 830.14  :
MP 830.15  :
MP 830.16  :
MP 830.17  :
MP 830.18  :
MP 830.19  :
MP 830.20  :
MP 830.21  :
             ;2. Bremse
MP 831.0   :
MP 831.1   :
MP 831.2   :
MP 831.3   :
MP 831.4   :
MP 831.5   :
MP 831.6   :
MP 831.7   :
MP 831.8   :
MP 831.9   :
MP 831.10  :
MP 831.11  :
MP 831.12  :
MP 831.13  :
MP 831.14  :
MP 831.15  :
MP 831.16  :
MP 831.17  :
MP 831.18  :
MP 831.19  :
MP 831.20  :
MP 831.21  :

             ;Bremsentest: multiplikator fuer Strom
MP 835.0   :
MP 835.1   :
MP 835.2   :
MP 835.3   :
MP 835.4   :
MP 835.5   :
MP 835.6   :
MP 835.7   :
MP 835.8   :
MP 835.9   :
MP 835.10  :
MP 835.11  :
MP 835.12  :
MP 835.13  :
MP 835.14  :
MP 835.15  :
MP 835.16  :
MP 835.17  :
MP 835.18  :
MP 835.19  :
MP 835.20  :
MP 835.21  :
             ;2. Bremse
MP 836.0   :
MP 836.1   :
MP 836.2   :
MP 836.3   :
MP 836.4   :
MP 836.5   :
MP 836.6   :
MP 836.7   :
MP 836.8   :
MP 836.9   :
MP 836.10  :
MP 836.11  :
MP 836.12  :
MP 836.13  :
MP 836.14  :
MP 836.15  :
MP 836.16  :
MP 836.17  :
MP 836.18  :
MP 836.19  :
MP 836.20  :
MP 836.21  :
             ;Bremsentest: zulaessiger Weg
MP 840.0   :
MP 840.1   :
MP 840.2   :
MP 840.3   :
MP 840.4   :
MP 840.5   :
MP 840.6   :
MP 840.7   :
MP 840.8   :
MP 840.9   :
MP 840.10  :
MP 840.11  :
MP 840.12  :
MP 840.13  :
MP 840.14  :
MP 840.15  :
MP 840.16  :
MP 840.17  :
MP 840.18  :
MP 840.19  :
MP 840.20  :
MP 840.21  :
             ;2. Bremse
MP 841.0   :
MP 841.1   :
MP 841.2   :
MP 841.3   :
MP 841.4   :
MP 841.5   :
MP 841.6   :
MP 841.7   :
MP 841.8   :
MP 841.9   :
MP 841.10  :
MP 841.11  :
MP 841.12  :
MP 841.13  :
MP 841.14  :
MP 841.15  :
MP 841.16  :
MP 841.17  :
MP 841.18  :
MP 841.19  :
MP 841.20  :
MP 841.21  :
                
             ;MP1054 Weg bei einer Motorumdrehung
             ;Eingabe: 0 bis maximal 15 Ziffern [mm]
MP 1054.0  :
MP 1054.1  :
MP 1054.2  :
MP 1054.3  :
MP 1054.4  :
MP 1054.5  :
MP 1054.6  :
MP 1054.7  :
MP 1054.8  :
MP 1054.9  :
MP 1054.10 :
MP 1054.11 :
MP 1054.12 :
MP 1054.13 :
MP 1054.14 :
MP 1054.15 :
MP 1054.16 :
MP 1054.17 :
MP 1054.18  :
MP 1054.19  :
MP 1054.20  :
MP 1054.21  :

             ; Stillstands-Überwachung
             ; Eingabe: 0,0010 bis 30,0000 [mm]
MP 1110.0  :
MP 1110.1  :
MP 1110.2  :
MP 1110.3  :
MP 1110.4  :
MP 1110.5  :
MP 1110.6  :
MP 1110.7  :
MP 1110.8  :
MP 1110.9  :
MP 1110.10 :
MP 1110.11 :
MP 1110.12 :
MP 1110.13 :
MP 1110.14 :
MP 1110.15 :
MP 1110.16 :
MP 1110.17 :
MP 1110.18  :
MP 1110.19  :
MP 1110.20  :
MP 1110.21  :
            
             ; Typ des Leistungsteils für die Achsen
             ; Name des ausgewählten Leistungsteils
             ; (wird von der TNC eingetragen)
MP 2100.0  :
MP 2100.1  :
MP 2100.2  :
MP 2100.3  :
MP 2100.4  :
MP 2100.5  :
MP 2100.6  :
MP 2100.7  :
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
MP 2100.18  :
MP 2100.19  :
MP 2100.20  :
MP 2100.21  :

             ; Zeit für Watchdog WD1.1 (3s)
             ; Eingabe: 1...6[s] 0=3[s]
MP 2172    :

             ; Motortyp für die Achsen
             ; Name des ausgewählten Motors
             ; (wird von der TNC eingetragen)
MP 2200.0  :
MP 2200.1  :
MP 2200.2  :
MP 2200.3  :
MP 2200.4  :
MP 2200.5  :
MP 2200.6  :
MP 2200.7  :
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
MP 2200.18  :
MP 2200.19  :
MP 2200.20  :
MP 2200.21  :

             ; Faktor fuer Stillstandsdauerstrom
             ; beim Motorbremsentest
MP 2230.0  :
MP 2230.1  :
MP 2230.2  :
MP 2230.3  :
MP 2230.4  :
MP 2230.5  :
MP 2230.6  :
MP 2230.7  :
MP 2230.8  :
MP 2230.9  :
MP 2230.10 :
MP 2230.11 :
MP 2230.12 :
MP 2230.13 :
MP 2230.14 :
MP 2230.15 :
MP 2230.16 :
MP 2230.17 :
MP 2230.18  :
MP 2230.19  :
MP 2230.20  :
MP 2230.21  :

             ; max. zulaessiger Weg beim
             ; Motorbremsentest
MP 2232.0  :
MP 2232.1  :
MP 2232.2  :
MP 2232.3  :
MP 2232.4  :
MP 2232.5  :
MP 2232.6  :
MP 2232.7  :
MP 2232.8  :
MP 2232.9  :
MP 2232.10 :
MP 2232.11 :
MP 2232.12 :
MP 2232.13 :
MP 2232.14 :
MP 2232.15 :
MP 2232.16 :
MP 2232.17 :
MP 2232.18  :
MP 2232.19  :
MP 2232.20  :
MP 2232.21  :

             ; Ansteuerung der Bremsen durch PWM-Ausgang verhindern
             ; Eingabe: Bit0=0/1
MP 2234.0  :
MP 2234.1  :
MP 2234.2  :
MP 2234.3  :
MP 2234.4  :
MP 2234.5  :
MP 2234.6  :
MP 2234.7  :
MP 2234.8  :
MP 2234.9  :
MP 2234.10 :
MP 2234.11 :
MP 2234.12 :
MP 2234.13 :
MP 2234.14 :
MP 2234.15 :
MP 2234.16 :
MP 2234.17 :
MP 2234.18  :
MP 2234.19  :
MP 2234.20  :
MP 2234.21  :

             ; Zeit zwischen Ausgabe des Bremssignals -BRK
             ; und dem Abschalten des Reglers (Überlappungszeit)
             ; Eingabe: 0,001 bis 0,5 [s]

MP 2308.0  :
MP 2308.1  :
MP 2308.2  :
MP 2308.3  :
MP 2308.4  :
MP 2308.5  :
MP 2308.6  :
MP 2308.7  :
MP 2308.8  :
MP 2308.9  :
MP 2308.10 :
MP 2308.11 :
MP 2308.12 :
MP 2308.13 :
MP 2308.14 :
MP 2308.15 :
MP 2308.16 :
MP 2308.17 :
MP 2308.18  :
MP 2308.19  :
MP 2308.20  :
MP 2308.21  :

             ; Maximale Bremsleistung beim Abbremsen bei Not-Aus
             ; Eingabe: 0,001 bis 3000,000 [kW]
             ;          0 = Funktion inaktiv
MP 2390.0  :
MP 2390.1  :
MP 2390.2  :
MP 2390.3  :
MP 2390.4  :
MP 2390.5  :
MP 2390.6  :
MP 2390.7  :
MP 2390.8  :
MP 2390.9  :
MP 2390.10 :
MP 2390.11 :
MP 2390.12 :
MP 2390.13 :
MP 2390.14 :
MP 2390.15 :
MP 2390.16 :
MP 2390.17 :
MP 2390.18  :
MP 2390.19  :
MP 2390.20  :
MP 2390.21  :

             ; Maximale Bremsleistung beim Abbremsen bei Netzausfall
             ; Eingabe: 0,001 bis 3000,000 [kW]
             ;          0 = Funktion inaktiv
MP 2394.0  :
MP 2394.1  :
MP 2394.2  :
MP 2394.3  :
MP 2394.4  :
MP 2394.5  :
MP 2394.6  :
MP 2394.7  :
MP 2394.8  :
MP 2394.9  :
MP 2394.10 :
MP 2394.11 :
MP 2394.12 :
MP 2394.13 :
MP 2394.14 :
MP 2394.15 :
MP 2394.16 :
MP 2394.17 :
MP 2394.18  :
MP 2394.19  :
MP 2394.20  :
MP 2394.21  :

             ; Rampe für Bremsen bei Not-Aus
             ; Eingabe: 0,1 bis 999,9 [U/min/ms]
             ;                        (U/min ist Motordrehzahl)
             ;          0 = Funktion inaktiv
MP 2590.0  :
MP 2590.1  :
MP 2590.2  :
MP 2590.3  :
MP 2590.4  :
MP 2590.5  :
MP 2590.6  :
MP 2590.7  :
MP 2590.8  :
MP 2590.9  :
MP 2590.10 :
MP 2590.11 :
MP 2590.12 :
MP 2590.13 :
MP 2590.14 :
MP 2590.15 :
MP 2590.16 :
MP 2590.17 :
MP 2590.18  :
MP 2590.19  :
MP 2590.20  :
MP 2590.21  :

             ;MP3142 Strichzahl des Drehgebers an der Hauptspindel
             ;Eingabe: 100 bis 9999 [Striche]
MP 3142    :
             ;--------------------------------------------------------------
             ;MP3143 Montageart Lagemesssystem Spindel
             ;Eingabe: 0 = direkt
             ;         1 = Getriebe (Ref-Impuls vom Eingang X30)
             ;         2 = Getriebe (Ref-Endlage von Eingang X30 und Ref-
             ;             Impuls vom Messsytem, nicht möglich wenn Getriebe
             ;             zwischen Messsytem und Spindel)
             ;         3 = Getriebe (2. Ref-Impuls vom Eingang X30)
MP 3143    :

             ;MP3210 S digital: Motor-Umdrehungen bei Nenn-Drehzahl
             ;         Eingabe: 0 bis 100 [1000/min]
             ;
             ;       S analog : S-Analogspannung bei Nenn-Drehzahl
             ;         Eingabe: 0 bis 9,999 [V]
MP 3210.0  : ;1. Stufe
MP 3210.1  : ;2. Stufe
MP 3210.2  : ;3. Stufe
MP 3210.3  : ;4. Stufe
MP 3210.4  : ;5. Stufe
MP 3210.5  : ;6. Stufe
MP 3210.6  : ;7. Stufe
MP 3210.7  : ;8. Stufe

             ;MP3450 Umdrehungen Spindel-Motor
             ;Eingabe: 0 bis 65535 [1/min]
             ;         0 keine Übersetzung
MP 3450.0  : ;1. Stufe
MP 3450.1  : ;2. Stufe
MP 3450.2  : ;3. Stufe
MP 3450.3  : ;4. Stufe
MP 3450.4  : ;5. Stufe
MP 3450.5  : ;6. Stufe
MP 3450.6  : ;7. Stufe
MP 3450.7  : ;8. Stufe

             ;MP3451 Umdrehungen Spindel
             ;Eingabe: 0 bis 65535 [1/min]
             ;         0 keine Übersetzung
MP 3451.0  : ;1. Stufe
MP 3451.1  : ;2. Stufe
MP 3451.2  : ;3. Stufe
MP 3451.3  : ;4. Stufe
MP 3451.4  : ;5. Stufe
MP 3451.5  : ;6. Stufe
MP 3451.6  : ;7. Stufe
MP 3451.7  : ;8. Stufe

             ;MP3510 Nenn-Drehzahl für Getriebestufen
             ;Eingabe: 0 bis 99 999,999 [U/min]
MP 3510.0  : ;1. Stufe
MP 3510.1  : ;2. Stufe
MP 3510.2  : ;3. Stufe
MP 3510.3  : ;4. Stufe
MP 3510.4  : ;5. Stufe
MP 3510.5  : ;6. Stufe
MP 3510.6  : ;7. Stufe
MP 3510.7  : ;8. Stufe
