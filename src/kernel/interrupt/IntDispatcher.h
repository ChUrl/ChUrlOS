/*****************************************************************************
 *                                                                           *
 *                         I N T D I S P A T C H E R                         *
 *                                                                           *
 *---------------------------------------------------------------------------*
 * Beschreibung:    Zentrale Unterbrechungsbehandlungsroutine des Systems.   *
 *                  Der Parameter gibt die Nummer des aufgetretenen          *
 *                  Interrupts an. Wenn eine Interrupt Service Routine (ISR) *
 *                  in der Map registriert ist, so wird diese aufgerufen.    *
 *                                                                           *
 * Autor:           Michael Schoettner, 30.7.16                              *
 *****************************************************************************/
#ifndef IntDispatcher_include__
#define IntDispatcher_include__

#include "ISR.h"
#include "lib/container//Array.h"
#include "lib/stream/Logger.h"

namespace Kernel {

class IntDispatcher {
public:
    // Vektor-Nummern
    enum Vector {
        TIMER = 32,
        KEYBOARD = 33,
        COM1 = 36
    };

public:
    IntDispatcher() = default;

    IntDispatcher(const IntDispatcher &copy) = delete;  // Verhindere Kopieren

    // Registrierung einer ISR. (Rueckgabewert: 0 = Erfolg, -1 = Fehler)
    int assign(uint8_t vector, ISR &isr);

    // ISR fuer 'vector' ausfuehren
    int dispatch(uint8_t vector);

private:
    Container::Array<ISR *, 256> handlerMap = {nullptr};

    static NamedLogger log;
};

}

#endif
