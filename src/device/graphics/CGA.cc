/*****************************************************************************
 *                                                                           *
 *                                    C G A                                  *
 *                                                                           *
 *---------------------------------------------------------------------------*
 * Beschreibung:    Mit Hilfe dieser Klasse kann man auf den Bildschirm des  *
 *                  PCs zugreifen. Der Zugriff erfolgt direkt auf der Hard-  *
 *                  wareebene, d.h. ueber den Bildschirmspeicher und den     *
 *                  I/O-Ports der Grafikkarte.                               *
 *                                                                           *
 * Autor:           Olaf Spinczyk, TU Dortmund                               *
 *                  Aenderungen von Michael Schoettner, HHU, 21.8.2016       *
 *****************************************************************************/
#include "CGA.h"
#include "lib/mem/Memory.h"

const IOport CGA::index_port(0x3d4);
const IOport CGA::data_port(0x3d5);

const bse::span<CGA::cga_char_t, CGA::ROWS * CGA::COLUMNS> CGA::SCREEN{reinterpret_cast<CGA::cga_char_t*>(0xb8000U)};
const bse::span<CGA::cga_line_t, CGA::ROWS> CGA::SCREEN_ROWS{reinterpret_cast<CGA::cga_line_t*>(0xb8000U)};
CGA::cga_page_t* const CGA::SCREEN_PAGE {reinterpret_cast<CGA::cga_page_t*>(0xb8000U)};

/*****************************************************************************
 * Methode:         CGA::setpos                                              *
 *---------------------------------------------------------------------------*
 * Beschreibung:    Setzen des Cursors in Spalte x und Zeile y.              *
 *****************************************************************************/
void CGA::setpos(unsigned int x, unsigned int y) {

    /* Hier muess Code eingefuegt werden */

    // NOTE: The cursor addresses positions on screen, not bytes
    unsigned short pos = x + y * COLUMNS;
    unsigned char cursor_low = pos & 0xFF;
    unsigned char cursor_high = (pos >> 8) & 0xFF;

    index_port.outb(0xF);  // Cursor(low)
    data_port.outb(cursor_low);

    index_port.outb(0xE);  // Cursor(high)
    data_port.outb(cursor_high);
}

/*****************************************************************************
 * Methode:         CGA::getpos                                              *
 *---------------------------------------------------------------------------*
 * Beschreibung:    Abfragem der Cursorposition                              *
 *                                                                           *
 * Rückgabewerte:   x und y                                                  *
 *****************************************************************************/
void CGA::getpos(unsigned int& x, unsigned int& y) {

    /* Hier muess Code eingefuegt werden */

    index_port.outb(0xF);  // Cursor(low)
    unsigned char cursor_low = data_port.inb();

    index_port.outb(0xE);  // Cursor(high)
    unsigned char cursor_high = data_port.inb();

    unsigned short cursor =
      (cursor_low & 0xFF) | ((cursor_high << 8) & 0xFF00);

    x = cursor % COLUMNS;
    y = (cursor / COLUMNS);
}

/*****************************************************************************
 * Methode:         CGA::show                                                *
 *---------------------------------------------------------------------------*
 * Beschreibung:    Anzeige eines Zeichens mit Attribut an einer bestimmten  *
 *                  Stelle auf dem Bildschirm.                               *
 *                                                                           *
 * Parameter:                                                                *
 *      x,y         Position des Zeichens                                    *
 *      character   Das auszugebende Zeichen                                 *
 *      attrib      Attributbyte fuer das Zeichen                            *
 *****************************************************************************/
void CGA::show(unsigned int x, unsigned int y, char character, unsigned char attrib) {

    /* Hier muess Code eingefuegt werden */

    if (x >= COLUMNS || y >= ROWS) {
        // Out of bounds
        return;
    }

    cga_char_t* pos = SCREEN[x + y * COLUMNS];
    pos->cga_char = character;
    pos->cga_attribute = attrib;
}

/*****************************************************************************
 * Methode:         CGA::print                                               *
 *---------------------------------------------------------------------------*
 * Beschreibung:    Anzeige mehrerer Zeichen ab der aktuellen Cursorposition *
 *                  '\n' fuer Zeilenvorschub.                                *
 *                                                                           *
 * Parameter:                                                                *
 *      substring      Auszugebende Zeichenkette                                *
 *      n           Laenger der Zeichenkette                                 *
 *      attrib      Attributbyte fuer alle Zeichen der Zeichenkette          *
 *****************************************************************************/
void CGA::print(const bse::string_view string, unsigned char attrib) const {

    /* Hier muess Code eingefuegt werden */

    unsigned int cursor_x = 0;
    unsigned int cursor_y = 0;  // Don't poll registers every stroke
    getpos(cursor_x, cursor_y);

    for (char current : string) {
        if (current == '\n') {
            cursor_x = 0;
            cursor_y = cursor_y + 1;

            if (cursor_y >= ROWS) {
                // Bottom of screen reached
                scrollup();
                cursor_y = cursor_y - 1;
            }

            continue;
        }

        if (current == '\0') {
            // Don't need to run to end if null terminated
            break;
        }

        show(cursor_x, cursor_y, current, attrib);
        cursor_x = cursor_x + 1;

        if (cursor_x >= COLUMNS) {
            // Right of screen reached
            cursor_x = 0;
            cursor_y = cursor_y + 1;

            if (cursor_y >= ROWS) {
                // Bottom of screen reached
                scrollup();
                cursor_y = cursor_y - 1;
            }
        }
    }

    setpos(cursor_x, cursor_y);
}

/*****************************************************************************
 * Methode:         CGA::scrollup                                            *
 *---------------------------------------------------------------------------*
 * Beschreibung:    Verschiebt den Bildschirminhalt um eine Zeile nach oben. *
 *                  Die neue Zeile am unteren Bildrand wird mit Leerzeichen  *
 *                  gefuellt.                                                *
 *****************************************************************************/
void CGA::scrollup() const {

    /* Hier muss Code eingefuegt werden */

    // Move up
    bse::memcpy<cga_line_t>(SCREEN_ROWS[0], SCREEN_ROWS[1], ROWS - 1);

    // Clear last line
    bse::zero<cga_line_t>(SCREEN_ROWS[ROWS - 1]);
}

/*****************************************************************************
 * Methode:         CGA::clear                                               *
 *---------------------------------------------------------------------------*
 * Beschreibung:    Lösche den Textbildschirm.                               *
 *****************************************************************************/
void CGA::clear() {

    /* Hier muess Code eingefuegt werden */

    bse::zero<cga_page_t>(SCREEN_PAGE);
    setpos(0, 0);
}

/*****************************************************************************
 * Methode:         CGA::attribute                                           *
 *---------------------------------------------------------------------------*
 * Beschreibung:    Hilfsfunktion zur Erzeugung eines Attribut-Bytes aus     *
 *                  Hintergrund- und Vordergrundfarbe und der Angabe, ob das *
 *                  Zeichen blinkend darzustellen ist.                       *
 *                                                                           *
 * Parameter:                                                                *
 *      bg          Background color                                         *
 *      fg          Foreground color                                         *
 *      blink       ywa/no                                                   *
 *****************************************************************************/
unsigned char CGA::attribute(CGA::color bg, CGA::color fg, bool blink) {

    /* Hier muess Code eingefuegt werden */

    return static_cast<int>(blink) << 7  // B0000000
         | (bg & 0x7) << 4  // 0HHH0000 (Hintergrund)
         | (fg & 0xF);      // 0000VVVV (Vordergrund)
}
