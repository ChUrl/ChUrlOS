/*****************************************************************************
 *                                                                           *
 *                                   P I T                                   *
 *                                                                           *
 *---------------------------------------------------------------------------*
 * Beschreibung:    Programmable Interval Timer.                             *
 *                                                                           *
 * Autor:           Michael Schoettner, 3.7.2022                             *
 *****************************************************************************/

#include "PIT.h"
#include "kernel/system/Globals.h"

const IOport PIT::control(0x43);
const IOport PIT::data0(0x40);

/*****************************************************************************
 * Methode:         PIT::interval                                            *
 *---------------------------------------------------------------------------*
 * Beschreibung:    Zeitinervall programmieren.                              *
 *                                                                           *
 * Parameter:                                                                *
 *      us:         Zeitintervall in Mikrosekunden, nachdem periodisch ein   * 
 *                  Interrupt erzeugt werden soll.                           *
 *****************************************************************************/
void PIT::interval(uint32_t us) {

    /* hier muss Code eingefuegt werden */

    control.outb(0x36);  // Zähler 0 Mode 3

    uint32_t cntStart = static_cast<unsigned int>((1193180.0 / 1000000.0) * us);  // 1.19Mhz PIT

    data0.outb(cntStart & 0xFF);  // Zaehler-0 laden (Lobyte)
    data0.outb(cntStart >> 8);    // Zaehler-0 laden (Hibyte)
}

/*****************************************************************************
 * Methode:         PIT::plugin                                              *
 *---------------------------------------------------------------------------*
 * Beschreibung:    Unterbrechungen fuer den Zeitgeber erlauben. Ab sofort   *
 *                  wird bei Ablauf des definierten Zeitintervalls die       *
 *                  Methode 'trigger' aufgerufen.                            *
 *****************************************************************************/
void PIT::plugin() {

    /* hier muss Code eingefuegt werden */

    intdis.assign(IntDispatcher::timer, *this);
    PIC::allow(PIC::timer);
}

/*****************************************************************************
 * Methode:         PIT::trigger                                             *
 *---------------------------------------------------------------------------*
 * Beschreibung:    ISR fuer den Zeitgeber. Wird aufgerufen, wenn der        * 
 *                  Zeitgeber eine Unterbrechung ausloest. Anzeige der Uhr   *
 *                  aktualisieren und Thread wechseln durch Setzen der       *
 *                  Variable 'forceSwitch', wird in 'int_disp' behandelt.    *
 *****************************************************************************/
void PIT::trigger() {

    /* hier muss Code eingefuegt werden */

    // log << TRACE << "Incrementing systime" << endl;

    // alle 10ms, Systemzeit weitersetzen
    systime++;

    // Bei jedem Tick einen Threadwechsel ausloesen.
    // Aber nur wenn der Scheduler bereits fertig intialisiert wurde
    // und ein weiterer Thread rechnen moechte

    /* hier muss Code eingefuegt werden */

    // Indicator
    if (systime - last_indicator_refresh >= 10) {
        indicator_pos = (indicator_pos + 1) % 4;
        CGA::show(79, 0, indicator[indicator_pos]);
        last_indicator_refresh = systime;
    }

    // Preemption
    if (scheduler.preemption_enabled()) {
        // log << TRACE << "Preemption" << endl;
        scheduler.preempt();
    }
}
