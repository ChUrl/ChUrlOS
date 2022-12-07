/*****************************************************************************
 *                                                                           *
 *                            H E A P D E M O                                *
 *                                                                           *
 *---------------------------------------------------------------------------*
 * Beschreibung:    Demonstration der dynamischen Speicherverwaltung.        *
 *                                                                           *
 * Autor:           Michael Schoettner, HHU, 27.12.2016                      *
 *****************************************************************************/

#include "HeapDemo.h"

void HeapDemo::run() {
    kout.lock();
    kout.clear();
    kout << "HEAP_DEMO ===================================================================" << endl;

    /* hier muss Code eingefuegt werden */

    allocator.dump_free_memory();

    // Rounding to word border
    kout << "ROUNDING ====================================================================" << endl;
    void* alloc = allocator.alloc(1);  // 1 Byte
    allocator.dump_free_memory();
    allocator.free(alloc);
    allocator.dump_free_memory();

    // Some objects and forward/backward merging
    kout << "SOME OBJECTS ================================================================" << endl;
    MyObj* a = new MyObj(5);
    allocator.dump_free_memory();
    MyObj* b = new MyObj(10);
    allocator.dump_free_memory();
    MyObj* c = new MyObj(15);
    allocator.dump_free_memory();
    delete b;  // No merge
    allocator.dump_free_memory();
    delete a;  // Merge forward BUG: Bluescreen
    allocator.dump_free_memory();
    delete c;
    allocator.dump_free_memory();

    // Allocate too whole heap
    // void* ptr = allocator.alloc(1024 * 1024 - 24);
    // allocator.dump_free_memory();
    // allocator.free(ptr);
    // allocator.dump_free_memory();

    // Allocate too much
    kout << "TOO MUCH ====================================================================" << endl;
    allocator.alloc(1024 * 1024);  // should fail as only 1024 * 1024 - Headersize bytes are available
    allocator.dump_free_memory();

    // A lot of allocations
    // MyObj* objs[1024];
    // for (unsigned int i = 0; i < 1024; ++i) {
    //     objs[i] = new MyObj(5);
    // }
    // allocator.dump_free_memory();
    // waitForReturn();
    // for (unsigned int i = 0; i < 1024; ++i) {
    //     delete objs[i];
    // }
    // allocator.dump_free_memory();

    // Array allocation
    kout << "ARRAY =======================================================================" << endl;
    MyObj* objs = new MyObj[1024];
    allocator.dump_free_memory();
    delete[] objs;
    allocator.dump_free_memory();

    kout << "HEAP_DEMO END ===============================================================" << endl;

    kout.unlock();
    scheduler.exit();
}
