#pragma once

#include <map>
#include <string>

enum class ProLogixCmd {
    ADDR,        // Syntax: ++addr [<PAD> [<SAD>]] | PAD: Primary Address (0-30), SAD: Secondary Address (96-126)
    AUTO,        // Syntax: ++auto [0|1] | 0: Off, 1: Read-after-write (autom. Talk-Addressierung nach Schreibvorgang)
    CLR,         // Syntax: ++clr | Sendet Selected Device Clear (SDC) an die aktuelle Adresse
    EOI,         // Syntax: ++eoi [0|1] | 0: Deaktiviert EOI-Assertion beim letzten Byte, 1: Aktiviert
    EOS,         // Syntax: ++eos [0|1|2|3] | 0: CR+LF, 1: CR, 2: LF, 3: Kein Terminator (binärfreundlich)
    EOT_ENABLE,  // Syntax: ++eot_enable [0|1] | 1: Fügt eot_char an USB-Output an, wenn EOI detektiert wird
    EOT_CHAR,    // Syntax: ++eot_char [<char>] | <char>: Dezimaler ASCII-Wert (z.B. 42 für '*') für eot_enable
    IFC,         // Syntax: ++ifc | Setzt Interface Clear Signal für 150us (macht Prologix zum Controller-In-Charge)
    LLO,         // Syntax: ++llo | Local Lockout: Sperrt die Frontpanel-Bedienung des adressierten Geräts
    LOC,         // Syntax: ++loc | Local: Aktiviert die Frontpanel-Bedienung des adressierten Geräts
    LON,         // Syntax: ++lon [0|1] | Listen-only Mode (nur im Device Mode verfügbar)
    MODE,        // Syntax: ++mode [0|1] | 1: Controller Mode, 0: Device Mode (Peripherie-Emulation)
    READ,        // Syntax: ++read [eoi|<char>] | Liest Daten bis EOI, spezifischem Zeichen oder Timeout
    READ_TMO_MS, // Syntax: ++read_tmo_ms <time> | Inter-Character Timeout in ms (1-3000)
    RST,         // Syntax: ++rst | Führt einen Power-on Reset des Controllers durch (Dauer ca. 5s)
    SAVECFG,     // Syntax: ++savecfg [0|1] | 1: Automatisches Speichern der Parameter im EEPROM (Vorsicht: Verschleiß)
    SPOLL,       // Syntax: ++spoll [<PAD> [<SAD>]] | Führt einen Serial Poll durch
    SRQ,         // Syntax: ++srq | Abfrage des aktuellen SRQ-Signalzustands (1: asserted/low, 0: high)
    STATUS,      // Syntax: ++status [0-255] | Setzt das Status-Byte für Serial Polls (nur Device Mode)
    TRG,         // Syntax: ++trg [<PAD1> ... <PAD15>] | Sendet Group Execute Trigger (GET) an bis zu 15 Adressen
    VER,         // Syntax: ++ver | Gibt die Firmware-Versionszeichenfolge zurück
    HELP         // Syntax: ++help | Gibt eine kurze Zusammenfassung der Befehle aus
};

static const std::map<ProLogixCmd, std::string> ProLogixCmdLookup = {
    {ProLogixCmd::ADDR, "++addr"},
    {ProLogixCmd::AUTO, "++auto"},
    {ProLogixCmd::CLR, "++clr"},
    {ProLogixCmd::EOI, "++eoi"},
    {ProLogixCmd::EOS, "++eos"},
    {ProLogixCmd::EOT_ENABLE, "++eot_enable"},
    {ProLogixCmd::EOT_CHAR, "++eot_char"},
    {ProLogixCmd::IFC, "++ifc"},
    {ProLogixCmd::LLO, "++llo"},
    {ProLogixCmd::LOC, "++loc"},
    {ProLogixCmd::LON, "++lon"},
    {ProLogixCmd::MODE, "++mode"},
    {ProLogixCmd::READ, "++read"},
    {ProLogixCmd::READ_TMO_MS, "++read_tmo_ms"},
    {ProLogixCmd::RST, "++rst"},
    {ProLogixCmd::SAVECFG, "++savecfg"},
    {ProLogixCmd::SPOLL, "++spoll"},
    {ProLogixCmd::SRQ, "++srq"},
    {ProLogixCmd::STATUS, "++status"},
    {ProLogixCmd::TRG, "++trg"},
    {ProLogixCmd::VER, "++ver"},
    {ProLogixCmd::HELP, "++help"}
};

enum class ScpiCmd {
    // Common/Universal
    IDN,                // Syntax: *IDN? | Abfrage von Hersteller, Modell und Firmware
    CLR,                // Syntax: *CLS | Löscht Status-Register und Fehlerschlange
    RST,                // Syntax: *RST | Setzt das Gerät auf Werkseinstellungen zurück
    WAI,                // Syntax: *WAI | Wartet mit der Ausführung nachfolgender Befehle bis aktuelle fertig sind
    OPC,                // Syntax: *OPC? | Gibt '1' zurück, wenn alle Operationen abgeschlossen sind

    // Frequency Commands
    FREQ_CENT,          // Syntax: FREQ:CENT <freq> | Setzt die Mittenfrequenz (z.B. 1GHZ)
    FREQ_CENT_QUERY,    // Syntax: FREQ:CENT? | Abfrage der Mittenfrequenz
    FREQ_SPAN,          // Syntax: FREQ:SPAN <freq> | Setzt die Frequenzspanne
    FREQ_SPAN_QUERY,    // Syntax: FREQ:SPAN? | Abfrage der Frequenzspanne
    FREQ_STAR,          // Syntax: FREQ:STAR <freq> | Setzt die Startfrequenz
    FREQ_STAR_QUERY,    // Syntax: FREQ:STAR? | Abfrage der Startfrequenz
    FREQ_STOP,          // Syntax: FREQ:STOP <freq> | Setzt die Stoppfrequenz
    FREQ_STOP_QUERY,    // Syntax: FREQ:STOP? | Abfrage der Stoppfrequenz

    // Markers/Calculations
    CALC_MARK_MAX,      // Syntax: CALC:MARK:MAX | Setzt den Marker auf den höchsten Peak
    CALC_MARK1_Y,       // Syntax: CALC:MARK1:Y? | Abfrage des Pegels (Amplitude) an Marker 1
    CALC_MARK1_X,       // Syntax: CALC:MARK1:X? | Abfrage der Frequenz an Marker 1

    // Bandwidth
    BAND_RES,           // Syntax: BAND:RES <freq> | Auflösebandbreite (RBW)
    BAND_VID_AUTO,      // Syntax: BAND:VID:AUTO [ON|OFF] | Automatische Videobandbreite
    BAND_RATIO,         // Syntax: BAND:RAT <num> | Verhältnis von RBW zu VBW
    BAND_TYPE,          // Syntax: BAND:TYPE [NORM|GAUS|FLAT] | Filtercharakteristik

    // Display/Trace
    DISP_TRAC_Y_RLEV,   // Syntax: DISP:TRAC:Y:RLEV <val> | Referenzpegel (meist in dBm)
    DISP_TRAC_Y_RLEV_QUERY, // Syntax: DISP:TRAC:Y:RLEV? | Abfrage des Referenzpegels
    DISP_TRAC_Y_OFFS,   // Syntax: DISP:TRAC:Y:RLEV:OFFS <val> | Referenzpegel-Offset
    DISP_TRAC_MODE,     // Syntax: DISP:TRAC:MODE [WRIT|MAXH|MINH|VIEW|BLANK] | Trace-Modus

    // Sweep Settings
    SWE_POIN,           // Syntax: SWE:POIN <val> | Anzahl der Messpunkte pro Sweep
    SWE_COUN,           // Syntax: SWE:COUN <val> | Anzahl der Sweeps für Average/MaxHold
    SWE_TIME,           // Syntax: SWE:TIME <time> | Manuelle Einstellung der Sweep-Zeit
    SWE_TIME_QUERY,     // Syntax: SWE:TIME? | Abfrage der aktuellen Sweep-Zeit

    // Initialization
    INIT_CONT,          // Syntax: INIT:CONT [ON|OFF] | Kontinuierlicher Sweep ein/aus
    INIT_IMM,           // Syntax: INIT:IMM | Löst einen einzelnen Sweep aus (Single-Shot)

    // Data/Format
    FORM_DATA,          // Syntax: FORM:DATA [ASC|REAL] | Datenformat (ASCII oder Binär)
    FORM_BORD,          // Syntax: FORM:BORD [NORM|SWAP] | Byte-Reihenfolge (Little/Big Endian)
    FORM_ASC,           // Syntax: FORM:ASC | Schaltet explizit auf ASCII-Format um

    // Trace
    TRAC_DATA,          // Syntax: TRAC:DATA? TRACE1 | Abfrage der rohen Messdaten von Trace 1
};

static const std::map<ScpiCmd, std::string> ScpiCmdLookup = {
    {ScpiCmd::IDN, "*IDN?"},
    {ScpiCmd::CLR, "*CLS"},
    {ScpiCmd::RST, "*RST"},
    {ScpiCmd::WAI, "*WAI"},
    {ScpiCmd::OPC, "*OPC?"},
    {ScpiCmd::FREQ_CENT, "FREQ:CENT"},
    {ScpiCmd::FREQ_CENT_QUERY, "FREQ:CENT?"},
    {ScpiCmd::FREQ_SPAN, "FREQ:SPAN"},
    {ScpiCmd::FREQ_SPAN_QUERY, "FREQ:SPAN?"},
    {ScpiCmd::FREQ_STAR, "FREQ:STAR"},
    {ScpiCmd::FREQ_STAR_QUERY, "FREQ:STAR?"},
    {ScpiCmd::FREQ_STOP, "FREQ:STOP"},
    {ScpiCmd::FREQ_STOP_QUERY, "FREQ:STOP?"},
    {ScpiCmd::CALC_MARK_MAX, "CALC:MARK:MAX"},
    {ScpiCmd::CALC_MARK1_Y, "CALC:MARK1:Y?"},
    {ScpiCmd::CALC_MARK1_X, "CALC:MARK1:X?"},
    {ScpiCmd::BAND_RES, "BAND:RES"},
    {ScpiCmd::BAND_VID_AUTO, "BAND:VID:AUTO"},
    {ScpiCmd::BAND_RATIO, "BAND:RAT"},
    {ScpiCmd::BAND_TYPE, "BAND:TYPE"},
    {ScpiCmd::DISP_TRAC_Y_RLEV, "DISP:TRAC:Y:RLEV"},
    {ScpiCmd::DISP_TRAC_Y_RLEV_QUERY, "DISP:TRAC:Y:RLEV?"},
    {ScpiCmd::DISP_TRAC_Y_OFFS, "DISP:TRAC:Y:RLEV:OFFS"},
    {ScpiCmd::DISP_TRAC_MODE, "DISP:TRAC:MODE"},
    {ScpiCmd::SWE_POIN, "SWE:POIN"},
    {ScpiCmd::SWE_COUN, "SWE:COUN"},
    {ScpiCmd::SWE_TIME, "SWE:TIME"},
    {ScpiCmd::SWE_TIME_QUERY, "SWE:TIME?"},
    {ScpiCmd::INIT_CONT, "INIT:CONT"},
    {ScpiCmd::INIT_IMM, "INIT:IMM"},
    {ScpiCmd::FORM_DATA, "FORM:DATA"},
    {ScpiCmd::FORM_BORD, "FORM:BORD"},
    {ScpiCmd::FORM_ASC, "FORM:ASC"},
    {ScpiCmd::TRAC_DATA, "TRAC:DATA?"}
};