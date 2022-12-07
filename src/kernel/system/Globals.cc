/*****************************************************************************
 *                                                                           *
 *                               G L O B A L S                               *
 *                                                                           *
 *---------------------------------------------------------------------------*
 * Beschreibung:    Globale Variablen des Systems.                           *
 *                                                                           *
 * Autor:           Michael Schoettner, 30.7.16                              *
 *****************************************************************************/

#include "Globals.h"

CGA_Stream kout;  // Ausgabe-Strom fuer Kernel
const BIOS& bios = BIOS::instance();        // Schnittstelle zum 16-Bit BIOS
VESA vesa;        // VESA-Treiber

PIC pic;               // Interrupt-Controller
IntDispatcher intdis;  // Unterbrechungsverteilung
PIT pit(10000);        // 10000
PCSPK pcspk;           // PC-Lautsprecher
Keyboard kb;           // Tastatur

// BumpAllocator allocator;
LinkedListAllocator allocator;
// TreeAllocator allocator;

Scheduler scheduler;

KeyEventManager kevman;
SerialOut serial;

unsigned int total_mem;  // RAM total
uint64_t systime = 0;
