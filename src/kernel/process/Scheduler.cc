/*****************************************************************************
 *                                                                           *
 *                          S C H E D U L E R                                *
 *                                                                           *
 *---------------------------------------------------------------------------*
 * Beschreibung:    Implementierung eines einfachen Zeitscheiben-Schedulers. *
 *                  Rechenbereite Threads werden in 'readQueue' verwaltet.   *
 *                                                                           *
 *                  Der Scheduler wird mit 'schedule' gestartet. Neue Threads*
 *                  können mit 'ready' hinzugefügt werden. Ein Thread muss   *
 *                  die CPU::freiwillig mit 'yield' abgeben, damit andere auch*
 *                  rechnen koennen. Ein Thread kann sich selbst mit 'exit'  *
 *                  terminieren. Ein Thread kann einen anderen Thread mit    *
 *                  'kill' beenden. Ein erzwungener Threadwechsel erfolgt    *
 *                  mit der Funktion 'preempt', welche von der Timer-ISR     *
 *                  aufgerufen wird.                                         *
 *                                                                           *
 *                  Zusaetzlich gibt es nun fuer die Semaphore zwei neue     *
 *                  Funktionen 'block' und 'deblock'.                        *
 *                                                                           *
 * Autor:           Michael, Schoettner, HHU, 23.11.2018                     *
 *****************************************************************************/

#include "Scheduler.h"
#include "IdleThread.h"
#include <utility>

namespace Kernel {

constexpr const bool INSANE_TRACE = false;

void Scheduler::start(Container::Vector<Memory::unique_ptr<Thread>>::iterator next) {
    active = next;
    if (active >= ready_queue.end()) {
        active = ready_queue.begin();
        log.debug() << "Scheduler::start started different thread than passed" << endl;
    }
    if constexpr (INSANE_TRACE) {
        log.trace() << "Starting Thread with id: " << dec << (*active)->tid << endl;
    }
    (*active)->start();  // First dereference the Iterator, then the unique_ptr to get Thread
}

void Scheduler::switch_to(Thread *prev_raw, Container::Vector<Memory::unique_ptr<Thread>>::iterator next) {
    active = next;
    if (active >= ready_queue.end()) {
        active = ready_queue.begin();
        // log.debug() << "Scheduler::switch_to started different thread than passed" << endl;
    }
    if constexpr (INSANE_TRACE) {
        log.trace() << "Switching to Thread with id: " << dec << (*active)->tid << endl;
    }
    prev_raw->switchTo(**active);
}

/*****************************************************************************
 * Methode:         Scheduler::schedule                                      *
 *---------------------------------------------------------------------------*
 * Beschreibung:    Scheduler starten. Wird nur einmalig aus main.cc gerufen.*
 *****************************************************************************/
void Scheduler::schedule() {

    /* hier muss Code eingefuegt werden */

    // We need to start the idle thread first as this one sets the scheduler to initialized
    // and enables preemption.
    // Otherwise preemption will be blocked and nothing will happen if the first threads
    // run() function is blocking

    ready_queue.push_back(Memory::make_unique<IdleThread>());
    log.info() << "Starting scheduling: starting thread with id: " << dec << (*(ready_queue.end() - 1))->tid << endl;
    start(ready_queue.end() - 1);
}

/*****************************************************************************
 * Methode:         Scheduler::ready                                         *
 *---------------------------------------------------------------------------*
 * Beschreibung:    Thread in readyQueue eintragen.                          *
 *****************************************************************************/
void Scheduler::ready(Memory::unique_ptr<Thread> &&thread) {
    Device::CPU::disable_int();
    log.debug() << "Adding to ready_queue, ID: " << dec << thread->tid << endl;
    ready_queue.push_back(std::move(thread));
    Device::CPU::enable_int();
}

/*****************************************************************************
 * Methode:         Scheduler::exit                                          *
 *---------------------------------------------------------------------------*
 * Beschreibung:    Thread ist fertig und terminiert sich selbst. Hier muss  *
 *                  nur auf den naechsten Thread mithilfe des Dispatchers    *
 *                  umgeschaltet werden. Der aktuell laufende Thread ist     *
 *                  nicht in der readyQueue.                                 *
 *****************************************************************************/
void Scheduler::exit() {

    /* hier muss Code eingefuegt werden */

    // Thread-Wechsel durch PIT verhindern
    Device::CPU::disable_int();

    if (ready_queue.size() == 1) {
        log.error() << "Can't exit last thread, active ID: " << dec << (*active)->tid << endl;
        Device::CPU::enable_int();
        return;
    }

    log.debug() << "Exiting thread, ID: " << dec << (*active)->tid << endl;
    start(ready_queue.erase(active));  // erase returns the next iterator after the erased element
    // cannot use switch_to here as the previous thread no longer
    // exists (was deleted by erase)

    // Interrupts werden in Thread_switch in Thread.asm wieder zugelassen
    // dispatch kehr nicht zurueck
}

/*****************************************************************************
 * Methode:         Scheduler::kill                                          *
 *---------------------------------------------------------------------------*
 * Beschreibung:    Thread mit 'Gewalt' terminieren. Er wird aus der         *
 *                  readyQueue ausgetragen und wird dann nicht mehr aufge-   *
 *                  rufen. Der Aufrufer dieser Methode muss ein anderer      *
 *                  Thread sein.                                             *
 *                                                                           *
 * Parameter:                                                                *
 *      that        Zu terminierender Thread                                 *
 *****************************************************************************/
void Scheduler::kill(uint32_t tid, Memory::unique_ptr<Thread> *ptr) {
    Device::CPU::disable_int();

    uint32_t prev_tid = (*active)->tid;

    // Block queue, can always kill
    for (Container::Vector<Memory::unique_ptr<Thread>>::iterator it = block_queue.begin();
         it != block_queue.end(); ++it) {
        if ((*it)->tid == tid) {
            // Found thread to kill

            if (ptr != nullptr) {
                // Move old thread out of queue to return it
                uint32_t pos = distance(block_queue.begin(), it);
                *ptr = std::move(block_queue[pos]);  // Return the killed thread
            }

            // Just erase from queue, do not need to switch
            block_queue.erase(it);
            log.info() << "Killed thread from block_queue with id: " << tid << endl;

            Device::CPU::enable_int();
            return;
        }
    }

    // Ready queue, can't kill last one
    if (ready_queue.size() == 1) {
        log.error() << "Kill: Can't kill last thread in ready_queue with id: " << tid << endl;
        Device::CPU::enable_int();
        return;
    }

    for (Container::Vector<Memory::unique_ptr<Thread>>::iterator it = ready_queue.begin();
         it != ready_queue.end(); ++it) {
        if ((*it)->tid == tid) {
            // Found thread to kill

            if (ptr != nullptr) {
                // Move old thread out of queue to return it
                uint32_t pos = distance(ready_queue.begin(), it);
                *ptr = std::move(ready_queue[pos]);  // Return the killed thread
            }

            if (tid == prev_tid) {
                // If we killed the active thread we need to switch to another one
                log.info() << "Killed active thread from ready_queue with id: " << tid << endl;

                // Switch to current active after old active was removed
                start(ready_queue.erase(it));
            }

            // Just erase from queue, do not need to switch
            ready_queue.erase(it);
            log.info() << "Killed thread from ready_queue with id: " << tid << endl;

            Device::CPU::enable_int();
            return;
        }
    }

    log.error() << "Kill: Couldn't find thread with id: " << tid << " in ready- or block-queue" << endl;
    log.error() << "Mabe it already exited itself?" << endl;
    Device::CPU::enable_int();
}

// TODO: Can't retrive the thread right now because it's not clear when it's finished,
//       maybe introduce a exited_queue and get it from there
void Scheduler::nice_kill(uint32_t tid, Memory::unique_ptr<Thread> *ptr) {
    Device::CPU::disable_int();

    for (Memory::unique_ptr<Thread> &thread: block_queue) {
        if (thread->tid == tid) {
            thread->suicide();
            log.info() << "Nice killed thread in block_queue with id: " << tid << endl;
            deblock(tid);
            Device::CPU::enable_int();
            return;
        }
    }

    for (Memory::unique_ptr<Thread> &thread: ready_queue) {
        if (thread->tid == tid) {
            thread->suicide();
            log.info() << "Nice killed thread in ready_queue with id: " << tid << endl;
            Device::CPU::enable_int();
            return;
        }
    }

    log.error() << "Can't nice kill thread (not found) with id: " << tid << endl;
    log.error() << "Mabe it already exited itself?" << endl;
    Device::CPU::enable_int();
}

/*****************************************************************************
 * Methode:         Scheduler::yield                                         *
 *---------------------------------------------------------------------------*
 * Beschreibung:    CPU::freiwillig abgeben und Auswahl des naechsten Threads.*
 *                  Naechsten Thread aus der readyQueue holen, den aktuellen *
 *                  in die readyQueue wieder eintragen. Das Umschalten soll  *
 *                  mithilfe des Dispatchers erfolgen.                       *
 *                                                                           *
 *                  Achtung: Falls nur der Idle-Thread läuft, so ist die     *
 *                           readyQueue leer.                                *
 *****************************************************************************/
void Scheduler::yield() {

    /* hier muss Code eingefuegt werden */

    // Thread-Wechsel durch PIT verhindern
    Device::CPU::disable_int();

    if (ready_queue.size() == 1) {
        if constexpr (INSANE_TRACE) {
            log.trace() << "Skipping yield as no thread is waiting, active ID: " << dec << (*active)->tid << endl;
        }
        Device::CPU::enable_int();
        return;
    }
    if constexpr (INSANE_TRACE) {
        log.trace() << "Yielding, ID: " << dec << (*active)->tid << endl;
    }
    switch_to((*active).get(), active + 1);  // prev_raw is valid since no thread was killed/deleted
}

/*****************************************************************************
 * Methode:         Scheduler::preempt                                       *
 *---------------------------------------------------------------------------*
 * Beschreibung:    Diese Funktion wird aus der ISR des PITs aufgerufen und  *
 *                  schaltet auf den naechsten Thread um, sofern einer vor-  *
 *                  handen ist.                                              *
 *****************************************************************************/
void Scheduler::preempt() {

    /* Hier muss Code eingefuegt werden */

    Device::CPU::disable_int();
    yield();
}

/*****************************************************************************
 * Methode:         Scheduler::block                                         *
 *---------------------------------------------------------------------------*
 * Beschreibung:    Aufrufer ist blockiert. Es soll auf den naechsten Thread *
 *                  umgeschaltet werden. Der Aufrufer soll nicht in die      *
 *                  readyQueue eingefuegt werden und wird extern verwaltet.  *
 *                  Wird bei uns nur fuer Semaphore verwendet. Jede Semaphore*
 *                  hat eine Warteschlange wo der Thread dann verwaltet wird.*
 *                  Die Methode kehrt nicht zurueck, sondern schaltet um.    *
 *****************************************************************************/
void Scheduler::block() {

    /* hier muss Code eingefuegt werden */

    Device::CPU::disable_int();

    if (ready_queue.size() == 1) {
        log.error() << "Can't block last thread, active ID: " << dec << (*active)->tid << endl;
        Device::CPU::enable_int();
        return;
    }

    Thread *prev_raw = (*active).get();
    std::size_t pos = distance(ready_queue.begin(), active);
    block_queue.push_back(std::move(ready_queue[pos]));

    if constexpr (INSANE_TRACE) {
        log.trace() << "Blocked thread with id: " << prev_raw->tid << endl;
    }

    switch_to(prev_raw, ready_queue.erase(active));  // prev_raw is valid as thread was moved before vector erase
}

/*****************************************************************************
 * Methode:         Scheduler::deblock                                       *
 *---------------------------------------------------------------------------*
 * Beschreibung:    Thread 'that' deblockieren. 'that' wird nur in die       *
 *                  readyQueue eingefuegt und dann zurueckgekehrt. In der    *
 *                  einfachsten Form entspricht diese Funktion exakt 'ready' *
 *                  Man koennte alternativ aber den deblockierten Thread auch*
 *                  am Anfang der readyQueue einfuegen, um ihn zu beorzugen. *
 *                                                                           *
 * Parameter:       that:  Thread der deblockiert werden soll.               *
 *****************************************************************************/
void Scheduler::deblock(uint32_t tid) {

    /* hier muss Code eingefuegt werden */

    Device::CPU::disable_int();

    for (Container::Vector<Memory::unique_ptr<Thread>>::iterator it = block_queue.begin();
         it != block_queue.end(); ++it) {
        if ((*it)->tid == tid) {
            // Found thread with correct tid

            std::size_t pos = distance(block_queue.begin(), it);
            ready_queue.insert(active + 1, std::move(block_queue[pos]));  // We insert the thread after the active
            // thread to prefer deblocked threads
            block_queue.erase(it);
            if constexpr (INSANE_TRACE) {
                log.trace() << "Deblocked thread with id: " << tid << endl;
            }
            Device::CPU::enable_int();
            return;
        }
    }

    log.error() << "Couldn't deblock thread with id: " << tid << endl;
    Device::CPU::enable_int();
}

}
