MCURSES - MINI CURSES LIBRARY - Version 2.1.0                                            10.11.2014
====================================================================================================

Copyright (c) 2011-2014 Frank Meyer - frank(at)fli4l.de

mcurses ist eine Mini-Curses-Programmbibliothek, die weniger auf Effizienz sondern eher
auf möglichst wenig Speicherplatz-Verbrauch getrimmt ist. So ist diese Bibliothek durchaus
auf Mikrocontrollern wie z.B. ATmegas lauffähig.

Die mcurses-Funktionen lehnen sich so nah wie möglich an das Original CURSES bzw.
NCURSES an, trotzdem müssen jedoch teilweise Abstriche gemacht werden. Als Terminal
wird lediglich VT200 unterstützt. Ein optimal passendes Terminal-Emulationsprogramm
ist PuTTY.

----------------------------------------------------------------------------------------------------
Konfigurierbare Konstanten in mcurses-config.h:

        mcurses nutzt den AVR-UART. Standard-Baudrate ist 19200 Bd. Diese Konstante kann hier
        an die verwendete Baudrate angepasst werden:

            #define MCURSES_BAUD    19200L                                  // baudrate

        Die Anzahlen der Zeilen und Spalten sind fest eingestellt, nämlich auf
        Standard-VT200-Größe:

            #define MCURSES_LINES   24                                      // 24 lines
            #define MCURSES_COLS    80                                      // 80 columns

        Die Werte können hier an die eingestellte Größe der verwendeten Terminal-Emulation
        angepasst werden.

----------------------------------------------------------------------------------------------------
Konstanten in mcurses.h:

  Konstante    Wert   Bemerkung
  --------------------------------------------------------------------------------------------------
  TRUE           1    Zu verwenden für Funktionen, die Booleans erwarten, z.B. nodelay()
  FALSE          0    s.a. TRUE
  OK             0    Returnwert Okay: Nur aus Kompatibilitätsgründen zu (n)curses enthalten, n.u.
  ERR          255    Returnwert Error: Siehe getch()   
  LINES         24    Ist konfigurierbar über MCURSES_LINES, siehe mcurses-config.h
  COLS          80    Ist konfigurierbar über MCURSES_COLS, siehe mcurses-config.h

----------------------------------------------------------------------------------------------------

mcurses-Funktionen:

void    initscr (void)

        Initialisierung mcurses, siehe auch endwin()

        Beispiel: initscr ();

----------------------------------------------------------------------------------------------------
void    move (uint8_t y, uint8_t x)

        Cursor auf Zeile y, Spalte x positionieren, beginnend bei 0

        Beispiel: move (10, 20);

----------------------------------------------------------------------------------------------------
void    attrset (uint16_t attr)

        Attribute setzen, können per '|' kombiniert werden

        Mögliche Attribute:

        A_NORMAL        // normal
        A_UNDERLINE     // underline
        A_REVERSE       // reverse
        A_BLINK         // blink
        A_BOLD          // bold
        A_DIM           // dim
        A_STANDOUT      // standout (same as bold)

        F_BLACK         // foreground black
        F_RED           // foreground red
        F_GREEN         // foreground green
        F_BROWN         // foreground brown
        F_BLUE          // foreground blue
        F_MAGENTA       // foreground magenta
        F_CYAN          // foreground cyan
        F_WHITE         // foreground white
        F_YELLOW        // same as F_BROWN

        B_BLACK         // background black
        B_RED           // background red
        B_GREEN         // background green
        B_BROWN         // background brown
        B_BLUE          // background blue
        B_MAGENTA       // background magenta
        B_CYAN          // background cyan
        B_WHITE         // background white
        B_YELLOW        // same as B_BROWN

        Beispiel: attrset (A_REVERSE | A_UNDERLINE | F_RED);

----------------------------------------------------------------------------------------------------
void    addch (uint8_t ch)

        Zeichen ausgeben

        Neben den alphanumerischen Zeichen können noch folgende Sonderzeichen ausgegeben werden:

            ACS_LRCORNER            DEC graphic 0x6a: lower right corner    Rechteck: Untere rechte Ecke
            ACS_URCORNER            DEC graphic 0x6b: upper right corner    Rechteck: Obere rechte Ecke
            ACS_ULCORNER            DEC graphic 0x6c: upper left corner     Rechteck: Obere linke Ecke
            ACS_LLCORNER            DEC graphic 0x6d: lower left corner     Rechteck: Untere linke Ecke
            ACS_PLUS                graphic 0x6e: crossing lines            Rechteck: Kreuz
            ACS_HLINE               DEC graphic 0x71: horizontal line       Rechteck: Waagerechte Linie
            ACS_LTEE                DEC graphic 0x74: left tee              Rechteck: Linkes T
            ACS_RTEE                DEC graphic 0x75: right tee             Rechteck: Rechtes T
            ACS_BTEE                DEC graphic 0x76: bottom tee            Rechteck: Unteres T
            ACS_TTEE                DEC graphic 0x77: top tee               Rechteck: Oberes T
            ACS_VLINE               DEC graphic 0x78: vertical line         Rechteck: Senkrechte Linie

            ACS_DIAMOND             DEC graphic 0x60: diamond               Diamond
            ACS_CKBOARD             DEC graphic 0x61: checker board         Schachbrettmuster
            ACS_DEGREE              DEC graphic 0x66: degree symbol         Grad-Symbol: °
            ACS_PLMINUS             DEC graphic 0x66: plus/minus            Plus/Minus

            ACS_S1                  DEC graphic 0x6f: scan line 1           Waagerechte Linie 1
            ACS_S3                  DEC graphic 0x70: scan line 3           Waagerechte Linie 3
            ACS_S5                  DEC graphic 0x71: scan line 5           Waagerechte Linie 5
            ACS_S7                  DEC graphic 0x72: scan line 7           Waagerechte Linie 7
            ACS_S9                  DEC graphic 0x73: scan line 9           Waagerechte Linie 9
            ACS_LEQUAL              DEC graphic 0x79: less/equal            Kleiner/gleich
            ACS_GEQUAL              DEC graphic 0x7a: greater/equal         Größer/gleich
            ACS_PI                  DEC graphic 0x7b: Pi                    Pi
            ACS_NEQUAL              DEC graphic 0x7c: not equal             Ungleich
            ACS_STERLING            DEC graphic 0x7d: uk pound sign         Britisches Pfund
            ACS_BULLET              DEC graphic 0x7e: bullet                Bullet (Punkt auf mittlerer Höhe)

        Beispiele:

            addch ('H');
            addch ('e');
            addch ('l');
            addch ('l');
            addch ('o');
            
            addch (ACS_ULCORNER);
            addch (ACS_HLINE);
            ....
            addch (ACS_HLINE);
            addch (ACS_URCORNER);

----------------------------------------------------------------------------------------------------
void    addstr (char * str)
void    addstr_P (const PROGMEM char * str)

        Zeichenkette ausgeben

        Beispiel: addstr ("Hello, World");

----------------------------------------------------------------------------------------------------
void    setscrreg (uint8_t t, uint8_t b)

        Scrolling-Region setzen: t = top, b = bottom

        Beispiel: setscrreg (1, LINES - 2);

----------------------------------------------------------------------------------------------------
void    deleteln (void)

        Eine Zeile an der aktuellen Position löschen, beachte Scrolling-Region!

        Beispiel: deleteln ();

----------------------------------------------------------------------------------------------------
void    insertln (void)

        Eine Zeile an der aktuellen Position einfügen, beachte Scrolling-Region!

        Beispiel: insertln ();

----------------------------------------------------------------------------------------------------
void    scroll (void)

        Eine Zeile hochrollen, beachte Scrolling-Region!

        Beispiel: scroll ();

----------------------------------------------------------------------------------------------------
void    clear (void)

        Bildschirm löschen

        Beispiel: clear ();

----------------------------------------------------------------------------------------------------
void    clrtobot (void)

        Bildschirm ab der aktuellen Zeile bis unten löschen

        Beispiel: clrtobot ();

----------------------------------------------------------------------------------------------------
void    clrtoeol (void)

        Zeile ab der aktuellen Spalte bis zum Zeilenende löschen

        Beispiel: clrtoeol ();

----------------------------------------------------------------------------------------------------
void    delch (void)

        Zeichen an der aktuellen Position löschen

        Beispiel: delch ();

----------------------------------------------------------------------------------------------------
void    insch (uint8_t)

        Zeichen an der aktuellen Position einfügen

        Beispiel: insch ('X');

----------------------------------------------------------------------------------------------------
void    nodelay (uint8_t)

        Nodelay-Flag setzen/löschen

        Ist das Nodelay-Flag gesetzt, liefert getch() bei nicht gedrückter Taste ERR zurück.

        Beispiel: nodelay (TRUE);

----------------------------------------------------------------------------------------------------
uint8_t getch ()

        Lesen von alphanumerischen Tasten und Funktionstasten

        Ist das Nodelay-Flag gesetzt, liefert getch() bei nicht gedrückter Taste ERR zurück.
	Ausser den alphanumerischen Zeichen kann getch() folgende Konstanten für
        Funktionstasten etc. zurückliefern:

            '\t'                   Tabulatortaste
            KEY_DOWN               Cursor runter
            KEY_UP                 Cursor hoch
            KEY_LEFT               Cursor links
            KEY_RIGHT              Cursor rechts
            KEY_HOME               HOME bzw. Pos1
            KEY_DC                 DEL bzw. Entf.
            KEY_IC                 INS bzw. Einfg.
            KEY_NPAGE              PAGE UP bzw. Bild hoch
            KEY_PPAGE              PAGE DOWN bzw. BILD runter
            KEY_END                END bzw. Ende
            KEY_BTAB               SH+TAB bzw. Umschalttaste + Tabulatortaste
            KEY_F(n)               Funktionstasten, z.B. F(2)

        Beispiel:
                  uint8_t ch = getch ();

                  if (ch == KEY_DOWN)
                  {
                       do_something ();
                  }

----------------------------------------------------------------------------------------------------
void    getnstr (char * str, uint8_t maxlen)

        Einlesen eines Strings mit maximal maxlen Zeichen.

        Die Funktion hat einen Mini-Editor eingebaut. Die Edit-Tasten sind Einfg, Entf,
        Pos1, Ende und die Cursor-Tasten.

        Beispiel:

        char    buf[80];

	move (10, 10);                            // Cursor positionieren
        getnstr (buf, 80);                        // Einlesen eines String mit 79 Zeichen + '\0'
        mvaddstr (11, 10, buf);                   // Ausgabe des Strings eine Zeile darunter

----------------------------------------------------------------------------------------------------
void    refresh (void)

        Aktualisieren des Bildschirminhaltes (Flush von evtl. noch auszugebenden Zeichen)

        Bemerkung: Die Funktion getch() macht implizit einen Aufruf von refresh(), um den
        Bildschirminhalt vor der Tastatur-Eingabe zu aktualisieren. Daher ist ein manueller
        Aufruf unmittelbar vor getch() nicht notwendig.

        Beispiel: refresh ();

----------------------------------------------------------------------------------------------------
void    endwin ()

        mcurses beenden, siehe auch initscr()

        Beispiel: endwin ();

----------------------------------------------------------------------------------------------------

Funktionsähnliche Makros 

----------------------------------------------------------------------------------------------------
void    erase ()

        siehe clear()

        Beispiel: erase ();

----------------------------------------------------------------------------------------------------
void    mvaddch (uint8_t y, uint8_t x, uint8_t c)

        Zur Position (y,x), dann Zeichen ausgeben

        Beispiel: mvaddch (10, 10, 'X');

----------------------------------------------------------------------------------------------------
void    mvaddstr (uint8_t y, uint8_t x, char * s)
void    mvaddstr_P (uint8_t y, uint8_t x, const PROGMEM char * s)

        Zur Position (y,x), dann Zeichenkette ausgeben

        Beispiel: mvaddstr (10, 10, "Hello World");

----------------------------------------------------------------------------------------------------
void    mvinsch (uint8_t y, uint8_t x, uint8_t c)

        Zur Position (y,x), dann Zeichen einfügen

        Beispiel: mvinsch (10, 10, 'X');

----------------------------------------------------------------------------------------------------
void    mvdelch(uint8_t y, uint8_t x)

        Zur Position (y,x), dann Zeichen löschen

        Beispiel: mvdelch(10, 10);

----------------------------------------------------------------------------------------------------
void    mvgetnstr(uint8_t y, uint8_t x, char * str, uint8_t maxlen)

        Zur Position (y,x), dann String einlesen

        Beispiel: mvgetnstr (10, 10, buf, 80);

----------------------------------------------------------------------------------------------------
void    getyx(uint8_t y, uint8_t x)

        Speichern der aktuellen Cursorposition in eigenen Variablen

        Beispiel: getyx(mycursor_y, mycursor_x);

----------------------------------------------------------------------------------------------------

Versionshistorie:

01.08.2011: Version 1.0.0

    * Erste Version 

01.08.2011: Version 1.0.2

    * Demo-Programm ausgebaut
    * addstr_P() und mvaddstr_P() zur Dokumentation hinzugefügt 

02.08.2011: Version 1.0.4

    * mcurses-Library + Demo-Programm nun auch unter Linux lauffähig 

02.08.2011: Version 1.0.6

    * Kosmetische Änderungen: Linux- und AVR-spezifischer Code besser isoliert. 

05.08.2011: Version 1.1.0

    * Unterstützung von Farben
    * Unterstützung von Graphikzeichen (Zeichnen von Rechtecken etc)
    * Neue Funktionen/Makros: nodelay(), refresh() und getyx()
    * Ringbuffer für effizientere Ein- und Ausgabe (UART)
    * AVR- und Linux-spezifische I/O-Funktionen besser modularisiert
    * Dokumentation ausgebaut
    * Demoprogramm ausgebaut (Farbe, Türme von Hanoi etc)

20.09.2011: Version 1.2.0

    * Key-Mapping an Linux/Console angepasst, insb. Edit keys
    * Backspace-Key vereinheitlicht
    * KEY_CR als RETURN-Taste

05.11.2014: Version 1.3.0

    * Neue Funktion getnstr() mit Mini-Editor
    * Kleine Korrekturen in endwin()

06.11.2014: Version 2.0.0

    * Portierung von mcurses auf SDCC Z80 compiler (noch im Test)
