#ifndef SerialOut_Include_H_
#define SerialOut_Include_H_

#include "IOport.h"
#include "lib/util/String.h"
#include "lib/util/StringView.h"

// NOTE: I took this code from https://wiki.osdev.org/Serial_Ports

class SerialOut {
private:
    static const IOport com1;

    static int serial_received();
    static int is_transmit_empty();

public:
    SerialOut();
    SerialOut(const SerialOut& copy) = delete;

    // Can't make singleton because atexit

    static char read();
    static void write(char a);
    static void write(const bse::string_view a);
};

#endif
