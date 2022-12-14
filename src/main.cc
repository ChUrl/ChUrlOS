/*****************************************************************************
 *                                                                           *
 *                                M A I N                                    *
 *                                                                           *
 *---------------------------------------------------------------------------*
 * Beschreibung:    Startroutine, wird direkt vom Bootlader angesprungen,    *
 *                  nachdem dieser in den Protected Mode geschaltet hat und  *
 *                  die GDT und IDT initalisiert hat.                        *
 *                                                                           *
 * Autor:           Michael Schoettner, HHU, 11.11.2016                      *
 *****************************************************************************/

#include "kernel/system/Globals.h"
#include "lib/demo/MainMenu.h"
#include "lib/stream/Logger.h"
#include "device/cpu/CPU.h"
#include "kernel/memory/Paging.h"
#include "lib/util/System.h"
#include "kernel/system/System.h"
#include "kernel/service/InterruptService.h"
#include "kernel/service/SchedulerService.h"

// TODO: Read from file
void print_startup_message() {
    Util::System::out.lock();
    Util::System::out.clear();
    Util::System::out << "BSEos 1.0\n"
                      << "=========\n"
                      << "Unterstuetzte Funktionen:\n"
                      << "   - Bildschirmausgaben\n"
                      << "   - Sound ueber den PC-Lautsprecher\n"
                      << "   - Tastatureingaben per Abfrage\n"
                      << "   - Einfache Heapverwaltung\n"
                      << "   - Tastatureingaben per Interrupt\n"
                      << "   - Kooperative Threads\n"
                      << "   - VESA Graphics Mode\n"
                      << "   - Einfaches Paging\n"
                      << "   - Preemptive Threads\n"
                      << "   - Einfache Synchronisierung\n"

                      << "   - Einfache (Tastatur-)Eventverwaltung\n"
                      << "   - Serial Output Logging\n"
                      << "\nPress Enter to continue\n"
                      << endl;
    Util::System::out.unlock();
}

int main() {
    Logger::set_level(Logger::TRACE);
    Logger::enable_serial(); // TODO: This is shitty, should pass an output stream to the logger instead

    // Speicherverwaltung initialisieren
    Kernel::allocator.init(); // TODO: MemoryService
    Kernel::kevman.init(); // TODO: EventService

    Kernel::System::registerService<Kernel::InterruptService>();
    Kernel::System::registerService<Kernel::SchedulerService>();

    // Tastatur-Unterbrechungsroutine 'einstoepseln'
    Kernel::kb.plugin();
    Kernel::pit.plugin();

    // Interrupts erlauben (Tastatur, PIT)
    Device::CPU::enable_int();

    // Activate paging
    // This has to happen after the allocator is initialized but before the scheduler is started
    Kernel::pg_init(); // TODO: MemoryService

    // Startmeldung
    print_startup_message();

    auto &schedulerService = Kernel::System::getService<Kernel::SchedulerService>();
    schedulerService.ready<MainMenu>();
    schedulerService.startScheduling();

    // NOTE: Pre-Post ToDo's
    // DONE: Reorganize the project similar to hhuOS:
    //       - DONE: Use CMake
    //       - DONE: Copy file structure: build, cmake, src
    //       - DONE: Translate the src/Makefile and boot/Makefile
    // DONE: Switch char/short/int/long to uint_ sized types where appropriate (I will find you...and then I will kill you)
    // DONE: Namespace that shit (Except for OutStream)
    // TODO: Change Globals.cpp/.h to System class
    // TODO: Constructors are missing in many places (Move, Copy, Moveass, Copyass, Destructor)
    // TODO: Change filenames to .cpp (ugh)
    // TODO: Check data types of register/port read/write functions
    // TODO: Investigate: C++20 (I wanted to use reference optionals somewhere...)
    // TODO: Rearrange code inside classes (public structs, public functions, private structs, private functions, private variables)
    // TODO: Write documentation comments
    // TODO: Investigate what stuff has to be synchronized...

    // NOTE: Post ToDo's
    // TODO: Investigate the memory cleanup (esp. with threads), I guess the whole thing is a memory leak currently lol
    // TODO: Singleton class wrapper? Or investigate usual singleton pattern for devices once again...
    // TODO: Use Optional type (e.g. for addresses), can implement a very simple version that handles vals/ptrs/refs
    // TODO: Introduce kernel services and abstract time, memory, scheduler, interrupt etc. (like in hhuOS)
    // TODO: Generalize key event system to eventservice (where custom events can be registered)
    // TODO: Use linear framebuffer to draw output/shell/text instead of CGA
    // TODO: Add worker threads that execute a function and return a value?
    // TODO: Manage exited threads in scheduler
    // TODO: Query thread state in scheduler
    // TODO: Bitfield lib class that has defined behavior regarding ordering/packing (for easy register access)?
    // TODO: Port my APIC code from hhuOS
    // TODO: Exceptions
    // TODO: The streams need to be completely redone, they are absolute chaos right now (copy C++ stream interfaces)
    //       I wouldn't touch them in their current state, just redo...
    //       There should be in/out/err system streams, stream for serial, with filesystem also filestreams
    //       Investigate StreamWriter, StreamReader, Buffered, Synchronized etc...
    //       Objects (e.g. Logger) should be able to just work on streams that will be passed in
    //       Also change Logger to lib/util Util:: namespace and streams to lib/stream Stream::

    // NOTE: Large post ToDo's
    // TODO: Add a RB-Tree datastructure and use it for memory management?
    // TODO: Enable full paging (check hhuOS for implementation example)

    // NOTE: Filesystem enable ToDo's (huge)
    // TODO: Very simple floppy disk device driver (floppy is common and simple)
    // TODO: Very simple FAT8 filesystem driver (or something similar that is extremely simple)
    // TODO: Need the infastructure to mount filesystems (check hhuOS for implementation example)
    // TODO: Write a very simple program loader to load binaries stored on the filesystem to memory and execute them
    // TODO: Add simple Unix-like binaries that get compiled for BSEos during build time (check hhuOS)
    // TODO: Add simple shell (check hhuOS)

    // NOTE: Insane ToDo's
    // TODO: Write a very simple text editor that can save files to the filesystem (maybe I could port something...)
    // TODO: Small interpreter to run some source code on the system
    // TODO: User/Kernel mode separation with syscalls

    // NOTE: Enforced ToDo's (needed)
    // DONE: Rewrite demos for threads
    // DONE: Make menu for demos
    // DONE: Thread switching stops after a while
    // DONE: Threads are not cleanup after exit, use managed pointer?
    // DONE: Fix PCSKP/Keyboard Demos: Keyboard Thread gets missing after blocking waiting for input, kevman doesn't wake up...

    // NOTE: Main ToDo's (extra)
    // DONE: Basic event management for keyboard events so threads can utilize interrupt based inputs
    //       This also works well with a blocked-queue in the scheduler for threads waiting for input
    // DONE: Serial output
    // CANCELED: Output graphviz stuff over serial?
    // CANCELED: Fix the damn TreeAllocator: Allow root deletion without bluescreen
    //           Maybe just remove the red black tree stuff and replace with usual binary search tree?
    //           I can just balance this tree unefficiantly by reinserting all nodes
    // CANCELED: Implement BST data structure with Tree interface?
    // CANCELED: Implement RBT tree interface implementation?
    // CANCELED: Switch treealloc so the underlying tree can be swapped easily
    // CANCELED: Implement realloc so ArrayList can realloc instead of newly allocate bigger block
    // DONE: Array wrapper
    // DONE: Rewrite Logging with a basic logger
    // DONE: Static Logger
    // DONE: String wrapper
    // DONE: Linked List
    // DONE: Iterator support for structures
    // DONE: Implement own basic managed pointers
    // DONE: Scheduler should own threads
    // DONE: Remove CoroutineState/ThreadState and just use pusha/popa, start/switch methods should
    //       just get esp as argument
    // CANCELED: Use singleton pattern for some device classes/classes used only in globals (needs stdlib for exita)
    // DONE: Introduce name to threads?
    // DONE: Allow to gracefully kill threads
    // CANCELED: Request thread status from scheduler (don't need it)

    // NOTE: Cleanup
    // DONE: Use templates for queue so threads don't have to be casted down from chain
    // DONE: Only use references instead of pointers where possible
    // DONE: Unify debug output format
    // DONE: Cleanup: Remove I added this... Notes, just leave explanations
    // DONE: Remove Math "lib" or do something with it
    // CANCELED: Add some fixed point math like sine approximation or fractions?
    // DONE: Cleanup imports: Only import stuff in implementation when only needed there
    // CANCELED: Switch cpu_disableint() to semaphore etc (Spinlock in the scheduler?)
    // CANCELED: Change mylib types to not use T* but T and call with memcpy<Type*> instead of memcpy<Type>?
    // DONE: Make more stuff const and static and static constexpr const
    // DONE: Remove ArrayList init and do this inside ArrayList when an operation on the list is done
    // DONE: Kevman unsubscribe is needed, because exited threads will still be woken up by kevman
    //       Or check if thread is still running
    // DONE: Delete copy constructors that weren't already deleted
    // DONE: Switch out semaphore Queue with ArrayList? Or switch back Scheduler to Queue?
    // CANCELED: Add virtual destructors and make sure to call them with delete when objects are removed
    // DONE: Replace empty constructors/destructors with default keyword
    // DONE: Synchronize the outstream
    // DONE: Remove Iterator from List.h
    // DONE: Move Array/ArrayList/LinkedList/List to bse namespace
    // DONE: Remove the Input.h file and replace functionality with kevman
    // DONE: Replace C style casts with C++ casts
    // DONE: Add Move/Copy/Assignment stuff to vector, array etc (all where it's missing)

    // Scheduler doesn't return
    return 0;
}
