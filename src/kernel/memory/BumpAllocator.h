/*****************************************************************************
 *                                                                           *
 *                        B U M P A L L O C A T O R                          *
 *                                                                           *
 *---------------------------------------------------------------------------*
 * Beschreibung:    Eine sehr einfache Heap-Verwaltung, welche freigegebenen *
 *                  Speicher nicht mehr nutzen kann.                         *
 *                                                                           *
 * Autor:           Michael Schoettner, HHU, 3.3.2022                        *
 *****************************************************************************/

#ifndef BumpAllocator_include__
#define BumpAllocator_include__

#include "Allocator.h"
#include "lib/stream/Logger.h"

namespace Kernel {

class BumpAllocator : Allocator {
private:
    uint8_t *next;
    uint32_t allocations;

    NamedLogger log;

public:
    BumpAllocator(Allocator &copy) = delete;  // Verhindere Kopieren

    BumpAllocator() : log("BMP-Alloc") {};  // Allocator() called implicitely in C++

//    ~BumpAllocator() override = default;

    void init() override;

    void dump_free_memory() override;

    void *alloc(uint32_t req_size) override;

    void free(void *ptr) override;
};

}

#endif
