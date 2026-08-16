import React, { useState, useEffect, ReactNode, useMemo } from 'react';
import { translations as generatedTranslations } from '../src/translations'; // Generated translations

// --- Language Configuration ---
// LangCode and DEFAULT_LANG are based on the fallback (manually managed) translations
// as this defines the core supported languages and structure.
type LangCode = 'en' | 'fr' | 'sw' | 'de' | 'es' | 'it' | 'ja' | 'ko' | 'pt' | 'ru' | 'tr' | 'zh' | 'nl';
const DEFAULT_LANG: LangCode = 'en';

// Define supported languages based on i18n.tsx (manually for now, can be dynamic later)
// We know these from reading i18n.tsx: sw, en, fr, de.
// Other languages in the LangCode type are: 'es' | 'it' | 'ja' | 'ko' | 'pt' | 'ru' | 'tr' | 'zh'
// but they don't have translation objects yet.
export const supportedLanguages = [
  { code: 'en', name: 'English' },
  { code: 'fr', name: 'Français' },
  { code: 'sw', name: 'Kiswahili' },
  { code: 'de', name: 'Deutsch' },
  { code: 'es', name: 'Español' },
  { code: 'nl', name: 'Nederlands' }
];

// Fallback translations (previously the main translations object)
const fallbackTranslations: Record<string, { [key: string]: string }> =
{
  sw:{
    "Name is required": "Jina linahitajika",
    "Duration must be at least 10 seconds (10000 ms)": "Muda lazima uwe angalau sekundi 10 (10000 ms)",
    "Download English Translations": "Pakua tafsiri za Kiingereza",
    "PolyMech - Cassandra": "PolyMech - Cassandra",
    "ONLINE": "MTANDAONI",
    "API URL": "Anwani ya API",
    "System Information": "Taarifa za mfumo",
    "Dashboard": "Dashibodi",
    "Plunger": "Kisukuma",
    "Reset": "Weka upya",
    "Disconnect": "Katisha muunganisho",
    "Cassandra HMI": "Cassandra HMI",
    "Cassandra Settings": "Mipangilio ya Cassandra",
    "Network Settings": "Mipangilio ya mtandao",
    "Signals": "Ishara",
    "Profiles": "Profaili",
    "Controller Chart": "Chati ya kidhibiti",
    "Sequential Heating Control": "Udhibiti wa kupasha joto mfululizo",
    "Cassandra Bank Alpha": "Cassandra Benki Alpha",
    "Cassandra Bank Beta (Auto-generated)": "Cassandra Benki Beta (imetolewa kiotomatiki)",
    "Global Settings": "Mipangilio ya jumla",
    "Show PV": "Onyesha PV",
    "Show SP": "Onyesha SP",
    "Profile SP": "SP ya profaili",
    "Window (min)": "Dirisha (dak)",
    "Visible Controllers": "Vidhibiti vinavyoonekana",
    "Clear Chart": "Futa chati",
    "Reset Zoom": "Weka upya kukuza",
    "Export to CSV": "Hamisha kama CSV",
    "Sequential Heating": "Kupasha joto mfululizo",
    "Heating Time": "Muda wa kupasha joto",
    "500ms - 2h": "500 ms – 2 h",
    "Max Simultaneous": "Kiasi cha juu kwa wakati mmoja",
    "Window Offset": "Uhamishaji wa dirisha",
    "Start Index": "Kielezo cha kuanza",
    "End Index": "Kielezo cha mwisho",
    "Current Status": "Hali ya sasa",
    "Idle": "Imesimama",
    "Carina": "Carina",
    "Stopped": "Imesitishwa",
    "PV": "PV",
    "T/C Err": "Hitilafu T/C",
    "SP": "SP",
    "N/A": "N/A",
    "Castor": "Castor",
    "Cetus": "Cetus",
    "Corona": "Corona",
    "Coma B": "Coma B",
    "Corvus": "Corvus",
    "Crater": "Crater",
    "Crux": "Crux",
    "Loading network settings...": "Inapakia mipangilio ya mtandao…",
    "Station (STA) Mode": "Hali ya Station (STA)",
    "STA SSID": "SSID ya STA",
    "STA Password (leave blank to keep unchanged)": "Nenosiri la STA (acha wazi kubaki kama lilivyo)",
    "STA IP Address": "Anwani ya IP ya STA",
    "STA Gateway": "Lango la STA",
    "STA Subnet Mask": "Maski ya sub-neti ya STA",
    "STA Primary DNS": "DNS ya msingi ya STA",
    "STA Secondary DNS": "DNS ya ziada ya STA",
    "Access Point (AP) Mode": "Hali ya Access Point (AP)",
    "AP SSID": "SSID ya AP",
    "AP Password (leave blank to keep unchanged)": "Nenosiri la AP (acha wazi kubaki kama lilivyo)",
    "AP IP Address": "Anwani ya IP ya AP",
    "AP Gateway": "Lango la AP",
    "AP Subnet Mask": "Maski ya sub-neti ya AP",
    "Save Network Settings": "Hifadhi mipangilio ya mtandao",
    "Associated Controllers:": "Vidhibiti vinavyohusishwa:",
    "Edit Profile": "Hariri profaili",
    "Start Profile": "Anzisha profaili",
    "Duplicate Profile": "Nakili profaili",
    "Delete Profile": "Futa profaili",
    "Disabled": "Imezimwa",
    "Total": "Jumla",
    "Max": "Max",
    "Start": "Anza",
    "Slave:": "Mtumwa:",
    "SP CMD Addr:": "Anwani CMD SP:",
    "E.g., Quick Ramp Up": "Mf., ongezeko la haraka",
    "Select a signal plot": "Chagua grafu ya ishara",
    "Describe the temperature profile": "Elezea wasifu wa joto",
    "Profile Name": "Jina la profaili",
    "Associated Signal Plot (Optional)": "Grafu ya ishara inayohusiana (hiari)",
    "Description": "Maelezo",
    "Duration (hh:mm:ss)": "Muda (hh:mm:ss)",
    "Temperature Curve": "Mchoro wa joto",
    "Clear Curve to Default Ramp": "Futa mchoro hadi rampu chaguomsingi",
    "Target Controllers (Registers)": "Vidhibiti lengwa (rejesta)",
    "Export JSON": "Hamisha JSON",
    "Import JSON": "Ingiza JSON",
    "Update Profile": "Sasisha profaili",
    "None": "Hakuna",
    "Slot:": "Slot:",
    "Signal Plot Editor": "Kihariri grafu ya ishara",
    "Download All JSON": "Pakua JSON zote",
    "Upload All JSON": "Pakia JSON zote",
    "No signal plots.": "Hakuna grafu za ishara.",
    "Signal plots loaded from API.": "Grafu za ishara zimepakuliwa kutoka API.",
    "Enable plot {name}": "Wezesha grafu {name}",
    "Total:": "Jumla:",
    "Add a set of sample control points to this plot": "Ongeza seti ya vidhibiti sampuli kwenye grafu hii",
    "Remove all control points from this plot": "Ondoa vidhibiti vyote kutoka grafu hii",
    "Download JSON for {name}": "Pakua JSON kwa {name}",
    "Upload JSON for {name}": "Pakia JSON kwa {name}",
    "Duration:": "Muda:",
    "Timeline:": "Muda wa matukio:",
    "Control Points": "Vidhibiti",
    "Properties:": "Sifa:",
    "Select a control point to see its properties.": "Chagua kidhibiti kuona sifa zake.",
    "Add Samples": "Ongeza sampuli",
    "Clear All CPs": "Futa CP zote",
    "Download Plot": "Pakua grafu",
    "Upload Plot": "Pakia grafu",
    "No control points to select.": "Hakuna vidhibiti vya kuchagua.",
    "Run this control point action now": "Endesha kitendo hiki sasa",
    "Select type": "Chagua aina",
    "Enter CP name": "Ingiza jina la CP",
    "Enter CP description": "Ingiza maelezo ya CP",
    "Or Select Known Register...": "Au chagua rejesta inayojulikana…",
    "Addr:": "Anwani:",
    "ID:": "Kitambulisho:",
    "Run Action": "Endesha kitendo",
    "Time (0-1000 scale):": "Muda (kipimo 0–1000):",
    "Actual:": "Halisi:",
    "State:": "Hali:",
    "Type:": "Aina:",
    "CP Name (Optional):": "Jina la CP (hiari):",
    "CP Description (Optional):": "Maelezo ya CP (hiari):",
    "Arguments:": "Hoja:",
    "Register Address (arg_0):": "Anwani ya rejesta (arg_0):",
    "Register Value (arg_1):": "Thamani ya rejesta (arg_1):",
    "Argument 2 (Optional):": "Hoja 2 (hiari):",
    "Or Select Known Coil...": "Au chagua koili inayojulikana…",
    "Coil Address (arg_0):": "Anwani ya koili (arg_0):",
    "Coil Value (arg_1):": "Thamani ya koili (arg_1):",
    "ON": "WASHI"
  },
  en: {
    "Name is required": "Name is required",
    "Duration must be at least 10 seconds (10000 ms)": "Duration must be at least 10 seconds (10000 ms)",
    "PolyMech - Cassandra": "PolyMech - Cassandra",
    "ONLINE": "ONLINE",
    "API URL": "API URL",
    "System Information": "System Information",
    "Dashboard": "Dashboard",
    "Plunger": "Plunger",
    "Reset": "Reset",
    "Disconnect": "Disconnect",
    "Cassandra HMI": "Cassandra HMI",
    "Cassandra Settings": "Cassandra Settings",
    "Network Settings": "Network Settings",
    "Signals": "Signals",
    "Profiles": "Profiles",
    "Associated Controllers:": "Associated Controllers:",
    "Edit Profile": "Edit Profile",
    "Start Profile": "Start Profile",
    "Duplicate Profile": "Duplicate Profile",
    "Delete Profile": "Delete Profile",
    "Disabled": "Disabled",
    "Total": "Total",
    "Idle": "Idle",
    "Max": "Max",
    "Start": "Start",
    "Slot:": "Slot:",
    "Enable plot {name}": "Enable plot {name}",
    "Total:": "Total:",
    "Add a set of sample control points to this plot": "Add a set of sample control points to this plot",
    "Remove all control points from this plot": "Remove all control points from this plot",
    "Download JSON for {name}": "Download JSON for {name}",
    "Upload JSON for {name}": "Upload JSON for {name}",
    "Signal Plot Editor": "Signal Plot Editor",
    "Download All JSON": "Download All JSON",
    "Upload All JSON": "Upload All JSON",
    "Duration:": "Duration:",
    "Timeline:": "Timeline:",
    "Control Points": "Control Points",
    "Properties:": "Properties:",
    "Select a control point to see its properties.": "Select a control point to see its properties.",
    "Add Samples": "Add Samples",
    "Clear All CPs": "Clear All CPs",
    "Download Plot": "Download Plot",
    "Upload Plot": "Upload Plot",
    "No control points to select.": "No control points to select.",
    "Signal plots loaded from API.": "Signal plots loaded from API.",
    "Controller Chart": "Controller Chart",
    "Global Settings": "Global Settings",
    "Show PV": "Show PV",
    "Show SP": "Show SP",
    "Profile SP": "Profile SP",
    "Window (min)": "Window (min)",
    "Visible Controllers": "Visible Controllers",
    "Cassandra Bank Alpha": "Cassandra Bank Alpha",
    "Cassandra Bank Beta (Auto-generated)": "Cassandra Bank Beta (Auto-generated)",
    "Clear Chart": "Clear Chart",
    "Reset Zoom": "Reset Zoom",
    "Export to CSV": "Export to CSV",
    "Sequential Heating Control": "Sequential Heating Control",
    "OFFLINE": "OFFLINE",
    "Connect": "Connect",
    "Connecting...": "Connecting...",
    "Sequential Heating": "Sequential Heating",
    "Heating Time": "Heating Time",
    "500ms - 2h": "500ms - 2h",
    "Max Simultaneous": "Max Simultaneous",
    "Window Offset": "Window Offset",
    "Start Index": "Start Index",
    "End Index": "End Index",
    "Current Status": "Current Status",
    "Carina": "Carina",
    "Stopped": "Stopped",
    "PV": "PV",
    "T/C Err": "T/C Err",
    "SP": "SP",
    "N/A": "N/A",
    "Castor": "Castor",
    "Cetus": "Cetus",
    "Corona": "Corona",
    "Loading network settings...": "Loading network settings...",
    "Station (STA) Mode": "Station (STA) Mode",
    "STA SSID": "STA SSID",
    "STA Password (leave blank to keep unchanged)": "STA Password (leave blank to keep unchanged)",
    "STA IP Address": "STA IP Address",
    "STA Gateway": "STA Gateway",
    "STA Subnet Mask": "STA Subnet Mask",
    "STA Primary DNS": "STA Primary DNS",
    "STA Secondary DNS": "STA Secondary DNS",
    "Access Point (AP) Mode": "Access Point (AP) Mode",
    "AP SSID": "AP SSID",
    "AP Password (leave blank to keep unchanged)": "AP Password (leave blank to keep unchanged)",
    "AP IP Address": "AP IP Address",
    "AP Gateway": "AP Gateway",
    "AP Subnet Mask": "AP Subnet Mask",
    "Save Network Settings": "Save Network Settings",
    "No signal plots.": "No signal plots.",
    "Slave:": "Slave:",
    "SP CMD Addr:": "SP CMD Addr:",
    "E.g., Quick Ramp Up": "E.g., Quick Ramp Up",
    "Select a signal plot": "Select a signal plot",
    "Describe the temperature profile": "Describe the temperature profile",
    "Profile Name": "Profile Name",
    "Associated Signal Plot (Optional)": "Associated Signal Plot (Optional)",
    "Description": "Description",
    "Duration (hh:mm:ss)": "Duration (hh:mm:ss)",
    "Temperature Curve": "Temperature Curve",
    "Clear Curve to Default Ramp": "Clear Curve to Default Ramp",
    "Target Controllers (Registers)": "Target Controllers (Registers)",
    "Export JSON": "Export JSON",
    "Import JSON": "Import JSON",
    "Update Profile": "Update Profile",
    "None": "None",
    "Run this control point action now": "Run this control point action now",
    "Select type": "Select type",
    "Enter CP name": "Enter CP name",
    "Enter CP description": "Enter CP description",
    "Or Select Known Coil...": "Or Select Known Coil...",
    "Addr:": "Addr:",
    "ID:": "ID:",
    "Run Action": "Run Action",
    "Time (0-1000 scale):": "Time (0-1000 scale):",
    "Actual:": "Actual:",
    "State:": "State:",
    "Type:": "Type:",
    "CP Name (Optional):": "CP Name (Optional):",
    "CP Description (Optional):": "CP Description (Optional):",
    "Arguments:": "Arguments:",
    "Coil Address (arg_0):": "Coil Address (arg_0):",
    "Coil Value (arg_1):": "Coil Value (arg_1):",
    "OFF": "OFF",
    "Argument 2 (Optional):": "Argument 2 (Optional):",
    "Download English Translations": "Download English Translations"
  },
  fr: {
    "Name is required": "Le nom est obligatoire",
    "Duration must be at least 10 seconds (10000 ms)": "La durée doit être d'au moins 10 secondes (10000 ms)",
    "Download English Translations": "Télécharger les traductions anglaises",
    "PolyMech - Cassandra": "PolyMech - Cassandra",
    "ONLINE": "EN LIGNE",
    "API URL": "URL de l'API",
    "System Information": "Informations système",
    "Dashboard": "Tableau de bord",
    "Plunger": "Poussoir",
    "Reset": "Réinitialiser",
    "Disconnect": "Déconnecter",
    "Cassandra HMI": "Cassandra HMI",
    "Cassandra Settings": "Paramètres Cassandra",
    "Network Settings": "Paramètres réseau",
    "Signals": "Signaux",
    "Profiles": "Profils",
    "Controller Chart": "Graphique du régulateur",
    "Sequential Heating Control": "Contrôle de chauffage séquentiel",
    "Cassandra Bank Alpha": "Cassandra Banque Alpha",
    "Cassandra Bank Beta (Auto-generated)": "Cassandra Banque Bêta (générée automatiquement)",
    "Global Settings": "Paramètres globaux",
    "Show PV": "Afficher PV",
    "Show SP": "Afficher SP",
    "Profile SP": "SP du profil",
    "Window (min)": "Fenêtre (min)",
    "Visible Controllers": "Régulateurs visibles",
    "Clear Chart": "Effacer le graphique",
    "Reset Zoom": "Réinitialiser le zoom",
    "Export to CSV": "Exporter en CSV",
    "Sequential Heating": "Chauffage séquentiel",
    "Heating Time": "Temps de chauffage",
    "500ms - 2h": "500 ms – 2 h",
    "Max Simultaneous": "Maximum simultané",
    "Window Offset": "Décalage de fenêtre",
    "Start Index": "Indice de début",
    "End Index": "Indice de fin",
    "Current Status": "État actuel",
    "Idle": "Inactif",
    "Carina": "Carina",
    "Stopped": "Arrêté",
    "PV": "PV",
    "T/C Err": "Err. T/C",
    "SP": "SP",
    "N/A": "N/D",
    "Castor": "Castor",
    "Cetus": "Cetus",
    "Corona": "Corona",
    "Coma B": "Coma B",
    "Corvus": "Corvus",
    "Crater": "Crater",
    "Crux": "Crux",
    "Loading network settings...": "Chargement des paramètres réseau…",
    "Station (STA) Mode": "Mode station (STA)",
    "STA SSID": "SSID STA",
    "STA Password (leave blank to keep unchanged)": "Mot de passe STA (laisser vide pour conserver)",
    "STA IP Address": "Adresse IP STA",
    "STA Gateway": "Passerelle STA",
    "STA Subnet Mask": "Masque de sous-réseau STA",
    "STA Primary DNS": "DNS primaire STA",
    "STA Secondary DNS": "DNS secondaire STA",
    "Access Point (AP) Mode": "Mode point d'accès (AP)",
    "AP SSID": "SSID AP",
    "AP Password (leave blank to keep unchanged)": "Mot de passe AP (laisser vide pour conserver)",
    "AP IP Address": "Adresse IP AP",
    "AP Gateway": "Passerelle AP",
    "AP Subnet Mask": "Masque de sous-réseau AP",
    "Save Network Settings": "Enregistrer les paramètres réseau",
    "Associated Controllers:": "Régulateurs associés :",
    "Edit Profile": "Modifier le profil",
    "Start Profile": "Démarrer le profil",
    "Duplicate Profile": "Dupliquer le profil",
    "Delete Profile": "Supprimer le profil",
    "Disabled": "Désactivé",
    "Total": "Total",
    "Max": "Max.",
    "Start": "Démarrer",
    "Slave:": "Esclave :",
    "SP CMD Addr:": "Adresse CMD SP :",
    "E.g., Quick Ramp Up": "Par ex., montée rapide",
    "Select a signal plot": "Sélectionner un graphique de signal",
    "Describe the temperature profile": "Décrire le profil de température",
    "Profile Name": "Nom du profil",
    "Associated Signal Plot (Optional)": "Graphique de signal associé (optionnel)",
    "Description": "Description",
    "Duration (hh:mm:ss)": "Durée (hh:mm:ss)",
    "Temperature Curve": "Courbe de température",
    "Clear Curve to Default Ramp": "Réinitialiser la courbe à la rampe par défaut",
    "Target Controllers (Registers)": "Régulateurs cibles (registres)",
    "Export JSON": "Exporter JSON",
    "Import JSON": "Importer JSON",
    "Update Profile": "Mettre à jour le profil",
    "None": "Aucun",
    "Slot:": "Emplacement :",
    "Signal Plot Editor": "Éditeur de graphique de signal",
    "Download All JSON": "Télécharger tous les JSON",
    "Upload All JSON": "Téléverser tous les JSON",
    "No signal plots.": "Aucun graphique de signal.",
    "Signal plots loaded from API.": "Graphiques de signal chargés depuis l'API.",
    "Enable plot {name}": "Activer le graphique {name}",
    "Total:": "Total :",
    "Add a set of sample control points to this plot": "Ajouter un ensemble de points de contrôle d'exemple à ce graphique",
    "Remove all control points from this plot": "Supprimer tous les points de contrôle de ce graphique",
    "Download JSON for {name}": "Télécharger JSON pour {name}",
    "Upload JSON for {name}": "Téléverser JSON pour {name}",
    "Duration:": "Durée :",
    "Timeline:": "Chronologie :",
    "Control Points": "Points de contrôle",
    "Properties:": "Propriétés :",
    "Select a control point to see its properties.": "Sélectionner un point de contrôle pour voir ses propriétés.",
    "Add Samples": "Ajouter des exemples",
    "Clear All CPs": "Effacer tous les CP",
    "Download Plot": "Télécharger le graphique",
    "Upload Plot": "Téléverser le graphique",
    "No control points to select.": "Aucun point de contrôle à sélectionner.",
    "Run this control point action now": "Exécuter cette action immédiatement",
    "Select type": "Sélectionner le type",
    "Enter CP name": "Entrer le nom CP",
    "Enter CP description": "Entrer la description CP",
    "Or Select Known Register...": "Ou sélectionner un registre connu…",
    "Addr:": "Adr. :",
    "ID:": "ID :",
    "Run Action": "Exécuter l'action",
    "Time (0-1000 scale):": "Temps (échelle 0–1000) :",
    "Actual:": "Réel :",
    "State:": "État :",
    "Type:": "Type :",
    "CP Name (Optional):": "Nom CP (optionnel) :",
    "CP Description (Optional):": "Description CP (optionnel) :",
    "Arguments:": "Arguments :",
    "Register Address (arg_0):": "Adresse registre (arg_0) :",
    "Register Value (arg_1):": "Valeur registre (arg_1) :",
    "Argument 2 (Optional):": "Argument 2 (optionnel) :",
    "Or Select Known Coil...": "Ou sélectionner une bobine connue…",
    "Coil Address (arg_0):": "Adresse bobine (arg_0) :",
    "Coil Value (arg_1):": "Valeur bobine (arg_1) :",
    "ON": "ON"
  },
  de: {
    "Modbus Connection": "PolyMech - Cassandra",
    "OFFLINE": "Offline",
    "API URL": "API-URL",
    "Connect": "Verbinden",
    "Connecting...": "Verbinde...",
    "ONLINE": "Online",
    "Queue": "Warteschlange",
    "Refresh": "Aktualisieren",
    "Disconnect": "Trennen",
    "Current Plunger State": "Aktueller Kolbenstatus",
    "N/A": "N/A",
    "Home": "Init.",
    "Plunge": "Eintauchen",
    "Stop": "Stopp",
    "Fill": "Füllen",
    "Info": "Info",
    "VFD Speed": "VFD-Geschwindigkeit",
    "Replay Duration": "Wiederholungsdauer",
    "Replay Now": "Wiederholen",
    "Adjust (mA):": "Anpassen (mA):",
    "Max Plunger Op. Time:": "Max. Kolbenbetriebszeit:",
    "VFD Control": "VFD-Steuerung",
    "Monitor and control Variable Frequency Drive.": "Frequenzumrichter überwachen und steuern.",
    "Running Freq:": "Aktuelle Frequenz:",
    "Set Freq (Mon):": "Frequenz einstellen (Mon):",
    "Current:": "Strom:",
    "Power:": "Leistung:",
    "Torque:": "Drehmoment:",
    "Fault Code:": "Fehlercode:",
    "Status:": "Status:",
    "Set Target Frequency (0-75 Int):": "Zielfrequenz festlegen (0-75 Int):",
    "Directional Commands": "Richtungsbefehle",
    "Sets target frequency then sends command to direction register.": "Legt die Zielfrequenz fest und sendet dann den Befehl an das Richtungsregister.",
    "Run Forward": "Vorwärts laufen",
    "Run Reverse": "Rückwärts laufen",
    "Stop VFD": "VFD stoppen",
    "Other Commands": "Weitere Befehle",
    "Reset Fault": "Fehler zurücksetzen",
    "None": "Keine",
    "Peak": "Max",
    "Loading plunger settings...": "Kolbeneinstellungen werden geladen...",
    "Plunger Settings": "Kolbeneinstellungen",
    "Download JSON": "JSON herunterladen",
    "Load Defaults": "Standardwerte laden",
    "Save Plunger Settings": "Kolbeneinstellungen speichern",
    "Load Settings from JSON": "Einstellungen aus JSON laden",
    "Load from JSON": "Aus JSON laden",
    "Speeds": "Geschwindigkeiten",
    "Slow": "Langsam",
    "Medium": "Mittel",
    "Fast": "Schnell",
    "Fill Plunge": "Füllen – Eintauchen",
    "Fill Home": "Füllen – Home",
    "Current Thresholds": "Stromgrenzwerte",
    "Jam Threshold": "Blockierschwelle",
    "Post Flow": "Nachfluss",
    "Post-Flow Configuration": "Nachfluss-Konfiguration",
    "Enable Post Flow": "Nachfluss aktivieren",
    "Duration": "Dauer",
    "Speed": "Geschwindigkeit",
    "Stopping Wait": "Wartezeit beim Stoppen",
    "Complete Wait": "Wartezeit bis Abschluss",
    "Timings & Durations": "Zeitpunkte & Dauern",
    "Jammed - Homing": "Blockiert – Referenzfahrt",
    "Jammed - Operation": "Blockiert – Betrieb",
    "Auto Mode Hold": "Automodus-Halten",
    "Max Universal Jam": "Max. Universalblockierung",
    "Fill Joystick Hold": "Füll-Joystick-Halten",
    "Fill Plunged Wait": "Wartezeit nach Füll-Eintauchen",
    "Fill Homed Wait": "Wartezeit nach Füll-Home",
    "Record Hold": "Aufnahme-Halten",
    "Max Record": "Max. Aufnahme",
    "Replay": "Wiedergabe",
    "Max Operation": "Max. Betrieb",
    "Plunger HMI": "Kolben-HMI",
    "Network Settings": "Netzwerkeinstellungen",
    "Station (STA) Mode": "Station (STA)-Modus",
    "STA SSID": "STA-SSID",
    "STA Password (leave blank to keep unchanged)": "STA-Passwort (leer lassen für unverändert)",
    "STA IP Address": "STA-IP-Adresse",
    "STA Gateway": "STA-Gateway",
    "STA Subnet Mask": "STA-Subnetzmaske",
    "STA Primary DNS": "STA-Primärer DNS",
    "STA Secondary DNS": "STA-Sekundärer DNS",
    "Access Point (AP) Mode": "Access-Point (AP)-Modus",
    "AP SSID": "AP-SSID",
    "AP Password (leave blank to keep unchanged)": "AP-Passwort (leer lassen für unverändert)",
    "AP IP Address": "AP-IP-Adresse",
    "AP Gateway": "AP-Gateway",
    "AP Subnet Mask": "AP-Subnetzmaske",
    "Save Network Settings": "Netzwerkeinstellungen speichern",
    "Loading network settings...": "Netzwerkeinstellungen werden geladen..."
  },
  es:{
    "Name is required": "El nombre es obligatorio",
    "Duration must be at least 10 seconds (10000 ms)": "La duración debe ser al menos de 10 segundos (10000 ms)",
    "Download English Translations": "Descargar traducciones al inglés",
    "PolyMech - Cassandra": "PolyMech - Cassandra",
    "ONLINE": "EN LÍNEA",
    "API URL": "URL de la API",
    "System Information": "Información del sistema",
    "Dashboard": "Panel",
    "Plunger": "Émbolo",
    "Reset": "Restablecer",
    "Disconnect": "Desconectar",
    "Cassandra HMI": "Cassandra HMI",
    "Cassandra Settings": "Configuraciones de Cassandra",
    "Network Settings": "Configuraciones de red",
    "Signals": "Señales",
    "Profiles": "Perfiles",
    "Controller Chart": "Gráfico del controlador",
    "Sequential Heating Control": "Control de calefacción secuencial",
    "Cassandra Bank Alpha": "Cassandra Banco Alfa",
    "Cassandra Bank Beta (Auto-generated)": "Cassandra Banco Beta (generado automáticamente)",
    "Global Settings": "Configuraciones globales",
    "Show PV": "Mostrar PV",
    "Show SP": "Mostrar SP",
    "Profile SP": "SP del perfil",
    "Window (min)": "Ventana (min)",
    "Visible Controllers": "Controladores visibles",
    "Clear Chart": "Limpiar gráfico",
    "Reset Zoom": "Restablecer zoom",
    "Export to CSV": "Exportar a CSV",
    "Sequential Heating": "Calefacción secuencial",
    "Heating Time": "Tiempo de calefacción",
    "500ms - 2h": "500 ms – 2 h",
    "Max Simultaneous": "Máx. simultáneos",
    "Window Offset": "Desfase de ventana",
    "Start Index": "Índice inicial",
    "End Index": "Índice final",
    "Current Status": "Estado actual",
    "Idle": "Inactivo",
    "Carina": "Carina",
    "Stopped": "Detenido",
    "PV": "PV",
    "T/C Err": "Err. T/C",
    "SP": "SP",
    "N/A": "N/D",
    "Castor": "Castor",
    "Cetus": "Cetus",
    "Corona": "Corona",
    "Coma B": "Coma B",
    "Corvus": "Corvus",
    "Crater": "Crater",
    "Crux": "Crux",
    "Loading network settings...": "Cargando configuraciones de red…",
    "Station (STA) Mode": "Modo estación (STA)",
    "STA SSID": "SSID STA",
    "STA Password (leave blank to keep unchanged)": "Contraseña STA (dejar en blanco para mantener)",
    "STA IP Address": "Dirección IP STA",
    "STA Gateway": "Puerta de enlace STA",
    "STA Subnet Mask": "Máscara de subred STA",
    "STA Primary DNS": "DNS primario STA",
    "STA Secondary DNS": "DNS secundario STA",
    "Access Point (AP) Mode": "Modo punto de acceso (AP)",
    "AP SSID": "SSID AP",
    "AP Password (leave blank to keep unchanged)": "Contraseña AP (dejar en blanco para mantener)",
    "AP IP Address": "Dirección IP AP",
    "AP Gateway": "Puerta de enlace AP",
    "AP Subnet Mask": "Máscara de subred AP",
    "Save Network Settings": "Guardar configuraciones de red",
    "Associated Controllers:": "Controladores asociados:",
    "Edit Profile": "Editar perfil",
    "Start Profile": "Iniciar perfil",
    "Duplicate Profile": "Duplicar perfil",
    "Delete Profile": "Eliminar perfil",
    "Disabled": "Deshabilitado",
    "Total": "Total",
    "Max": "Máx.",
    "Start": "Iniciar",
    "Slave:": "Esclavo:",
    "SP CMD Addr:": "Dirección CMD SP:",
    "E.g., Quick Ramp Up": "Ej., rampa rápida",
    "Select a signal plot": "Seleccionar un gráfico de señal",
    "Describe the temperature profile": "Describir el perfil de temperatura",
    "Profile Name": "Nombre del perfil",
    "Associated Signal Plot (Optional)": "Gráfico de señal asociado (opcional)",
    "Description": "Descripción",
    "Duration (hh:mm:ss)": "Duración (hh:mm:ss)",
    "Temperature Curve": "Curva de temperatura",
    "Clear Curve to Default Ramp": "Restablecer curva a rampa predeterminada",
    "Target Controllers (Registers)": "Controladores objetivo (registros)",
    "Export JSON": "Exportar JSON",
    "Import JSON": "Importar JSON",
    "Update Profile": "Actualizar perfil",
    "None": "Ninguno",
    "Slot:": "Ranura:",
    "Signal Plot Editor": "Editor de gráfico de señal",
    "Download All JSON": "Descargar todos los JSON",
    "Upload All JSON": "Subir todos los JSON",
    "No signal plots.": "No hay gráficos de señal.",
    "Signal plots loaded from API.": "Gráficos de señal cargados desde la API.",
    "Enable plot {name}": "Habilitar gráfico {name}",
    "Total:": "Total:",
    "Add a set of sample control points to this plot": "Agregar un conjunto de puntos de control de muestra a este gráfico",
    "Remove all control points from this plot": "Eliminar todos los puntos de control de este gráfico",
    "Download JSON for {name}": "Descargar JSON para {name}",
    "Upload JSON for {name}": "Subir JSON para {name}",
    "Duration:": "Duración:",
    "Timeline:": "Cronología:",
    "Control Points": "Puntos de control",
    "Properties:": "Propiedades:",
    "Select a control point to see its properties.": "Seleccione un punto de control para ver sus propiedades.",
    "Add Samples": "Agregar muestras",
    "Clear All CPs": "Limpiar todos los CP",
    "Download Plot": "Descargar gráfico",
    "Upload Plot": "Subir gráfico",
    "No control points to select.": "No hay puntos de control para seleccionar.",
    "Run this control point action now": "Ejecutar esta acción ahora",
    "Select type": "Seleccionar tipo",
    "Enter CP name": "Ingresar nombre CP",
    "Enter CP description": "Ingresar descripción CP",
    "Or Select Known Register...": "O seleccionar registro conocido…",
    "Addr:": "Dir.:",
    "ID:": "ID:",
    "Run Action": "Ejecutar acción",
    "Time (0-1000 scale):": "Tiempo (escala 0–1000):",
    "Actual:": "Actual:",
    "State:": "Estado:",
    "Type:": "Tipo:",
    "CP Name (Optional):": "Nombre CP (opcional):",
    "CP Description (Optional):": "Descripción CP (opcional):",
    "Arguments:": "Argumentos:",
    "Register Address (arg_0):": "Dirección de registro (arg_0):",
    "Register Value (arg_1):": "Valor de registro (arg_1):",
    "Argument 2 (Optional):": "Argumento 2 (opcional):",
    "Or Select Known Coil...": "O seleccionar bobina conocida…",
    "Coil Address (arg_0):": "Dirección de bobina (arg_0):",
    "Coil Value (arg_1):": "Valor de bobina (arg_1):",
    "ON": "ENCENDIDO"
  },
  nl:{
    "Name is required": "Naam is verplicht",
    "Duration must be at least 10 seconds (10000 ms)": "De duur moet minimaal 10 seconden zijn (10000 ms)",
    "Download English Translations": "Engelse vertalingen downloaden",
    "PolyMech - Cassandra": "PolyMech - Cassandra",
    "ONLINE": "ONLINE",
    "API URL": "API-URL",
    "System Information": "Systeeminformatie",
    "Dashboard": "Dashboard",
    "Plunger": "Plunjer",
    "Reset": "Resetten",
    "Disconnect": "Verbinding verbreken",
    "Cassandra HMI": "Cassandra HMI",
    "Cassandra Settings": "Cassandra-instellingen",
    "Network Settings": "Netwerkinstellingen",
    "Signals": "Signalen",
    "Profiles": "Profielen",
    "Controller Chart": "Controllergrafiek",
    "Sequential Heating Control": "Sequentiële verwarmingsturing",
    "Cassandra Bank Alpha": "Cassandra Bank Alpha",
    "Cassandra Bank Beta (Auto-generated)": "Cassandra Bank Beta (automatisch gegenereerd)",
    "Global Settings": "Algemene instellingen",
    "Show PV": "PV weergeven",
    "Show SP": "SP weergeven",
    "Profile SP": "Profiel-SP",
    "Window (min)": "Venster (min)",
    "Visible Controllers": "Zichtbare controllers",
    "Clear Chart": "Grafiek wissen",
    "Reset Zoom": "Zoom resetten",
    "Export to CSV": "Exporteren naar CSV",
    "Sequential Heating": "Sequentiële verwarming",
    "Heating Time": "Verwarmingstijd",
    "500ms - 2h": "500 ms – 2 u",
    "Max Simultaneous": "Max. gelijktijdig",
    "Window Offset": "Venster-offset",
    "Start Index": "Startindex",
    "End Index": "Eindindex",
    "Current Status": "Huidige status",
    "Idle": "Inactief",
    "Carina": "Carina",
    "Stopped": "Gestopt",
    "PV": "PV",
    "T/C Err": "T/C-fout",
    "SP": "SP",
    "N/A": "N/B",
    "Castor": "Castor",
    "Cetus": "Cetus",
    "Corona": "Corona",
    "Coma B": "Coma B",
    "Corvus": "Corvus",
    "Crater": "Crater",
    "Crux": "Crux",
    "Loading network settings...": "Netwerkinstellingen laden…",
    "Station (STA) Mode": "Station (STA)-modus",
    "STA SSID": "STA-SSID",
    "STA Password (leave blank to keep unchanged)": "STA-wachtwoord (leeg laten om ongewijzigd te laten)",
    "STA IP Address": "STA-IP-adres",
    "STA Gateway": "STA-gateway",
    "STA Subnet Mask": "STA-subnetmasker",
    "STA Primary DNS": "Primair STA-DNS",
    "STA Secondary DNS": "Secundair STA-DNS",
    "Access Point (AP) Mode": "Access Point (AP)-modus",
    "AP SSID": "AP-SSID",
    "AP Password (leave blank to keep unchanged)": "AP-wachtwoord (leeg laten om ongewijzigd te laten)",
    "AP IP Address": "AP-IP-adres",
    "AP Gateway": "AP-gateway",
    "AP Subnet Mask": "AP-subnetmasker",
    "Save Network Settings": "Netwerkinstellingen opslaan",
    "Associated Controllers:": "Gekoppelde controllers:",
    "Edit Profile": "Profiel bewerken",
    "Start Profile": "Profiel starten",
    "Duplicate Profile": "Profiel dupliceren",
    "Delete Profile": "Profiel verwijderen",
    "Disabled": "Uitgeschakeld",
    "Total": "Totaal",
    "Max": "Max.",
    "Start": "Starten",
    "Slave:": "Slave:",
    "SP CMD Addr:": "SP CMD-adres:",
    "E.g., Quick Ramp Up": "Bijv. snelle opwarming",
    "Select a signal plot": "Selecteer een signaalgrafiek",
    "Describe the temperature profile": "Beschrijf het temperatuurprofiel",
    "Profile Name": "Profielfnaam",
    "Associated Signal Plot (Optional)": "Geassocieerde signaalgrafiek (optioneel)",
    "Description": "Beschrijving",
    "Duration (hh:mm:ss)": "Duur (hh:mm:ss)",
    "Temperature Curve": "Temperatuurcurve",
    "Clear Curve to Default Ramp": "Curve wissen naar standaardramp",
    "Target Controllers (Registers)": "Doelcontrollers (registers)",
    "Export JSON": "JSON exporteren",
    "Import JSON": "JSON importeren",
    "Update Profile": "Profiel bijwerken",
    "None": "Geen",
    "Slot:": "Sleuf:",
    "Signal Plot Editor": "Signaalgrafiek-editor",
    "Download All JSON": "Alle JSON downloaden",
    "Upload All JSON": "Alle JSON uploaden",
    "No signal plots.": "Geen signaalgrafieken.",
    "Signal plots loaded from API.": "Signaalgrafieken geladen vanuit API.",
    "Enable plot {name}": "Grafiek {name} inschakelen",
    "Total:": "Totaal:",
    "Add a set of sample control points to this plot": "Voeg een set voorbeeldregelpunten toe aan deze grafiek",
    "Remove all control points from this plot": "Verwijder alle regelpunten uit deze grafiek",
    "Download JSON for {name}": "JSON downloaden voor {name}",
    "Upload JSON for {name}": "JSON uploaden voor {name}",
    "Duration:": "Duur:",
    "Timeline:": "Tijdlijn:",
    "Control Points": "Regelpunten",
    "Properties:": "Eigenschappen:",
    "Select a control point to see its properties.": "Selecteer een regelpunt om zijn eigenschappen te zien.",
    "Add Samples": "Voorbeelden toevoegen",
    "Clear All CPs": "Alle CP's wissen",
    "Download Plot": "Grafiek downloaden",
    "Upload Plot": "Grafiek uploaden",
    "No control points to select.": "Geen regelpunten om te selecteren.",
    "Run this control point action now": "Voer deze regelpuntactie nu uit",
    "Select type": "Selecteer type",
    "Enter CP name": "Voer CP-naam in",
    "Enter CP description": "Voer CP-beschrijving in",
    "Or Select Known Register...": "Of kies een bekend register…",
    "Addr:": "Adr.:",
    "ID:": "ID:",
    "Run Action": "Actie uitvoeren",
    "Time (0-1000 scale):": "Tijd (0–1000-schaal):",
    "Actual:": "Actueel:",
    "State:": "Status:",
    "Type:": "Type:",
    "CP Name (Optional):": "CP-naam (optioneel):",
    "CP Description (Optional):": "CP-beschrijving (optioneel):",
    "Arguments:": "Argumenten:",
    "Register Address (arg_0):": "Registeradres (arg_0):",
    "Register Value (arg_1):": "Registerwaarde (arg_1):",
    "Argument 2 (Optional):": "Argument 2 (optioneel):",
    "Or Select Known Coil...": "Of kies een bekende spoel…",
    "Coil Address (arg_0):": "Spoeladres (arg_0):",
    "Coil Value (arg_1):": "Spoelwaarde (arg_1):",
    "ON": "AAN"
  }
};

const translationCache: { [lang: string]: { [key: string]: string } } = {};
let translationsLoadedForLang: { [lang: string]: boolean } = {};

const getCurrentLang = (): LangCode => {
  if (typeof window !== 'undefined') {
    const params = new URLSearchParams(window.location.search);
    const langParam = params.get('lang') as LangCode;
    if (langParam && Object.prototype.hasOwnProperty.call(fallbackTranslations, langParam)) {
      return langParam;
    }

    const browserLangs = navigator.languages || [navigator.language];
    for (const lang of browserLangs) {
      const shortLang = lang.split('-')[0] as LangCode;
      if (shortLang && Object.prototype.hasOwnProperty.call(fallbackTranslations, shortLang)) {
        return shortLang;
      }
    }
  }
  return DEFAULT_LANG;
};

interface TProps {
  children: ReactNode;
}

const T: React.FC<TProps> = ({ children }) => {
  const [translatedText, setTranslatedText] = useState<ReactNode>(null);
  const currentLang = getCurrentLang();

  const textKey = useMemo(() => {
    if (typeof children === 'string') return children;
    if (React.isValidElement(children) && typeof children.props.children === 'string') {
      return children.props.children;
    }
    return null;
  }, [children]);

  useEffect(() => {
    let isMounted = true;

    if (!textKey) {
      setTranslatedText(children);
      return;
    }

    // Simplified translateAndCache (synchronous)
    const getTranslationValue = (key: string, lang: LangCode): string => {
      // 1. Try generated translations for the current language
      if (generatedTranslations[lang] && generatedTranslations[lang][key] !== undefined) {
        return generatedTranslations[lang][key];
      }
      // 2. Try fallback translations for the current language
      if (fallbackTranslations[lang] && fallbackTranslations[lang][key] !== undefined) {
        return fallbackTranslations[lang][key];
      }
      // 3. If not found and lang is not default, try generated for default
      if (lang !== DEFAULT_LANG) {
        if (generatedTranslations[DEFAULT_LANG] && generatedTranslations[DEFAULT_LANG][key] !== undefined) {
          return generatedTranslations[DEFAULT_LANG][key];
        }
        // 4. Try fallback for default
        if (fallbackTranslations[DEFAULT_LANG] && fallbackTranslations[DEFAULT_LANG][key] !== undefined) {
          return fallbackTranslations[DEFAULT_LANG][key];
        }
      }
      // 5. Ultimate fallback: return the key itself
      return key;
    };
    
    // Attempt to get from cache first
    if (translationCache[currentLang] && translationCache[currentLang][textKey]) {
        const cachedResult = translationCache[currentLang][textKey];
        if (React.isValidElement(children) && typeof children.props.children === 'string') {
            setTranslatedText(React.cloneElement(children, {}, cachedResult));
        } else {
            setTranslatedText(cachedResult);
        }
    } else {
        const result = getTranslationValue(textKey, currentLang);
        // Populate cache
        if (!translationCache[currentLang]) {
            translationCache[currentLang] = {};
        }
        translationCache[currentLang][textKey] = result;

        if (isMounted) {
            if (React.isValidElement(children) && typeof children.props.children === 'string') {
                setTranslatedText(React.cloneElement(children, {}, result));
            } else {
                setTranslatedText(result);
            }
        }
    }

    return () => {
      isMounted = false;
    };
  }, [textKey, currentLang, children]);

  // Render logic: Show original if textKey is null or translatedText is not yet set.
  // Otherwise, show translatedText.
  if (!textKey) { // If children is not a simple string (e.g. a React component without translatable string)
    return <>{children}</>;
  }
  
  if (translatedText !== null) {
    return <>{translatedText}</>;
  }

  // Fallback to showing the original children (or key) while loading or if no translation.
  // This avoids blank spaces.
  return <>{children}</>; 
};

// Helper function to get a translated string directly
export const translate = (textKey: string, langParam?: LangCode): string => {
  const langToUse = langParam || getCurrentLang();

  // Check cache first
  if (translationCache[langToUse] && translationCache[langToUse][textKey] !== undefined) {
    return translationCache[langToUse][textKey];
  }

  let resolvedTranslation: string | undefined;

  // 1. Try generated translations for the current language
  if (generatedTranslations[langToUse] && generatedTranslations[langToUse][textKey] !== undefined) {
    resolvedTranslation = generatedTranslations[langToUse][textKey];
  }
  // 2. Try fallback translations for the current language
  else if (fallbackTranslations[langToUse] && fallbackTranslations[langToUse][textKey] !== undefined) {
    resolvedTranslation = fallbackTranslations[langToUse][textKey];
  }
  // 3. If not found and lang is not default, try generated for default
  else if (langToUse !== DEFAULT_LANG) {
    if (generatedTranslations[DEFAULT_LANG] && generatedTranslations[DEFAULT_LANG][textKey] !== undefined) {
      resolvedTranslation = generatedTranslations[DEFAULT_LANG][textKey];
    }
    // 4. Try fallback for default
    else if (fallbackTranslations[DEFAULT_LANG] && fallbackTranslations[DEFAULT_LANG][textKey] !== undefined) {
      resolvedTranslation = fallbackTranslations[DEFAULT_LANG][textKey];
    }
  }

  const result = resolvedTranslation !== undefined ? resolvedTranslation : textKey;

  // Populate cache
  if (!translationCache[langToUse]) {
    translationCache[langToUse] = {};
  }
  translationCache[langToUse][textKey] = result;
  
  return result;
};

export const getTranslationCache = (lang?: LangCode): Record<string, string> | Record<string, { [key: string]: string }> => {
  if (lang) {
    return translationCache[lang] || {};
  }
  return translationCache;
};

if (typeof window !== 'undefined') {
  (window as any).getTranslationCache = getTranslationCache;
  (window as any).translate = translate; // For debugging
  (window as any).getCurrentLang = getCurrentLang; // For debugging
  (window as any).generatedTranslations = generatedTranslations; // For debugging
  (window as any).fallbackTranslations = fallbackTranslations; // For debugging
}

export { T, getCurrentLang, fallbackTranslations, generatedTranslations };
