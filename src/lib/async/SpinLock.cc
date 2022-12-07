/*****************************************************************************
 *                                                                           *
 *                              S P I N L O C K                              *
 *                                                                           *
 *---------------------------------------------------------------------------*
 * Beschreibung:    Implementierung eines Spinlocks mithilfe der cmpxchg     *
 *                  Instruktion.                                             *
 *                                                                           *
 * Autor:           Michael Schoettner, 2.2.2018                             *
 *****************************************************************************/

#include "SpinLock.h"

/*****************************************************************************
 * Methode:         CAS                                                      *
 *---------------------------------------------------------------------------*
 * Parameter:       *ptr    Adresse der Variable des Locks                   *
 *                   old    Wert gegen den verglichen wird                   *
 *                  _new    Wert der gesetzt werden soll                     *
 *                                                                           *
 * Beschreibung:    Semantik der Funktion CAS = Cmompare & Swap:             *
 *                      if old == *ptr then                                  *
 *                          *ptr := _new                                     *
 *                      return prev                                          *
 *****************************************************************************/
static inline uint32_t CAS(const uint32_t* ptr) {
    uint32_t prev;

    /*
        AT&T/UNIX assembly syntax

        The 'volatile' keyword after 'asm' indicates that the instruction 
        has important side-effects. GCC will not delete a volatile asm if 
        sit is reachable.
     */
    asm volatile("lock;"                           // prevent race conditions with other cores
                 "cmpxchg %1, %2;"                 // %1 = _new; %2 = *ptr
                                                   // constraints
                 : "=a"(prev)                      // output: =a: RAX -> prev (%0))
                 : "r"(1), "m"(*ptr), "a"(0)  // input = %1, %2, %3 (r=register, m=memory, a=accumlator = eax
                 : "memory");                      // ensures assembly block will not be moved by gcc

    return prev;  // return pointer instead of prev to prevent unnecessary second call
}

/*****************************************************************************
 * Methode:         SpinLock::acquire                                        *
 *---------------------------------------------------------------------------*
 * Beschreibung:    Lock belegen.                                            *
 *****************************************************************************/
void SpinLock::acquire() {
    // If lock == 0 the SpinLock can be aquired without waiting
    // If lock == 1 the  while loop blocks until aquired
    while (CAS(ptr) != 0) {}
}

/*****************************************************************************
 * Methode:         SpinLock::release                                        *
 *---------------------------------------------------------------------------*
 * Beschreibung:    Lock freigeben.                                          *
 *****************************************************************************/
void SpinLock::release() {
    lock = 0;
}
