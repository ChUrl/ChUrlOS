/*****************************************************************************
 *                                                                           *
 *                                  P I C                                    *
 *                                                                           *
 *---------------------------------------------------------------------------*
 * Beschreibung:    Mit Hilfe des PICs koennen Hardware-Interrupts (IRQs)    *
 *                  einzeln zugelassen oder unterdrueckt werden. Auf diese   *
 *                  Weise wird also bestimmt, ob die Unterbrechung eines     *
 *                  Geraetes ueberhaupt an den Prozessor weitergegeben wird. *
 *                  Selbst dann erfolgt eine Aktivierung der Unterbrechungs- *
 *                  routine nur, wenn der Prozessor bereit ist, auf Unter-   *
 *                  brechungen zu reagieren. Dies kann mit Hilfe der Klasse  *
 *                  CPU festgelegt werden.                                   *
 *                                                                           *
 * Autor:           Olaf Spinczyk, TU Dortmund                               *
 *****************************************************************************/
#ifndef PIC_include__
#define PIC_include__

#include <cstdint>
#include "device/port/IOport.h"

namespace Device {

class PIC {
private:
    static const IOport IMR1;  // interrupt mask register von PIC 1
    static const IOport IMR2;  // interrupt mask register von PIC 2

public:
    PIC(const PIC &copy) = delete;  // Verhindere Kopieren

    PIC() = default;

    // Can't make static because atexit

    // IRQ-Nummern von Geraeten
    enum Irq {
        TIMER = 0,     // Programmable Interrupt Timer (PIT)
        KEYBOARD = 1,  // Tastatur
        COM1 = 4
    };

    // Freischalten der Weiterleitung eines IRQs durch den PIC an die CPU
    static void allow(uint8_t irq);

    // Unterdruecken der Weiterleitung eines IRQs durch den PIC an die CPU
    static void forbid(uint8_t irq);

    // Abfragen, ob die Weiterleitung fuer einen bestimmten IRQ unterdrueckt ist
    static bool status(uint8_t interrupt_device);
};

}

#endif
