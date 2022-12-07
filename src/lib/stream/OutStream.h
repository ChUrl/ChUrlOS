/*****************************************************************************
 *                                                                           *
 *                           O U T S T R E A M                               *
 *                                                                           *
 *---------------------------------------------------------------------------*
 * Beschreibung:    Die Klasse OutStream enthaelt die Definition des         *
 *                  << Operators fuer die wichtigsten der vordefinierten     *
 *                  Datentypen und realisiert somit die bekannte Ausgabe-    *
 *                  funktion der C++ iO_Stream Bibliothek. Zur Zeit wird     *
 *                  die Darstellung von Zeichen, Zeichenketten und ganzen    *
 *                  Zahlen unterstuetzt. Ein weiterer << Operator erlaubt    *
 *                  die Verwendung von Manipulatoren.                        *
 *                                                                           *
 *                  Neben der Klasse OutStream sind hier auch die            *
 *                  Manipulatoren hex, dec, oct und bin fuer die Wahl der    *
 *                  Basis bei der Zahlendarstellung, sowie endl fuer den     *
 *                  Zeilenumbruch definiert.                                 *
 *                                                                           *
 * Autor:           Olaf Spinczyk, TU Dortmund                               *
 *                  Aenderungen von Michael Schoettner, HHU, 06.04.20        *
 *****************************************************************************/
#ifndef OutStream_include__
#define OutStream_include__

#include "StringBuffer.h"
#include "lib/string/String.h"
#include "lib/string/StringView.h"

// Some basic width formatting
class fillw {
public:
    constexpr explicit fillw(const unsigned int w) : w(w) {}

    const unsigned int w;
};

class fillc {
public:
    constexpr explicit fillc(const char c) : c(c) {}

    const char c;
};

class OutStream : public StringBuffer {
private:
    // Some stream formatting
    unsigned char fill_used;   // indicates how many characters are already used by the text internally
    unsigned char fill_width;  // If input is shorter than fill_width fill remaining space up with fill_char
    char fill_char;            // fill character for fixed width
    int base;                  // Basis des Zahlensystems: z.B. 2, 8, 10 oder 16

    void fill_use_char();  // recognizes that one char from the print width has been used up
    void fill_finalize();  // does the filling after text has been written to buffer

public:
    OutStream(const OutStream &copy) = delete;  // Verhindere Kopieren

    OutStream() : fill_used(0), fill_width(0), fill_char(' '), base(10) {}

//    ~OutStream() override = default;

    // Methode zur Ausgabe des Pufferinhalts der Basisklasse StringBuffer.
    void flush() override;

    void put(char c) override;

    // OPERATOR << : Umwandlung des angegebenen Datentypes in eine
    //               Zeichenkette.

    // NOTE: I changed the stream manipulators to templates to be usable with different streams.
    //       If a Stream derived from OutStream gets passed to a operator<< the type won't be "lowered".
    //       This allows chaining of operator<< of different streams.
    //       Needed because I added operator<< overloads to the CGA_Stream class to change color with manipulators.

    // Darstellung eines Zeichens (trivial)
    template<typename T>
    friend T &operator<<(T &os, char c) {
        os.put(c);
        if (c != '\n') {
            // endl() doesn't has access to StringBuffer::put(), so ignore \n here
            os.fill_finalize();
        }
        return os;
    }

    template<typename T>
    friend T &operator<<(T &os, unsigned char c) { return os << static_cast<char>(c); }

    // Darstellung einer nullterminierten Zeichenkette
    template<typename T>
    friend T &operator<<(T &os, const String::string_view string) {
        for (char current: string) {
            os.put(current);
        }
        os.fill_finalize();
        return os;
    }

    // Darstellung ganzer Zahlen im Zahlensystem zur Basis base
    template<typename T>
    friend T &operator<<(T &os, short ival) { return os << static_cast<long>(ival); }

    template<typename T>
    friend T &operator<<(T &os, unsigned short ival) { return os << static_cast<unsigned long>(ival); }

    template<typename T>
    friend T &operator<<(T &os, int ival) { return os << static_cast<long>(ival); }

    template<typename T>
    friend T &operator<<(T &os, unsigned int ival) { return os << static_cast<unsigned long>(ival); }

    template<typename T>
    friend T &operator<<(T &os, long ival) {
        // Bei negativen Werten wird ein Minuszeichen ausgegeben.
        if (ival < 0) {
            os.put('-');
            ival = -ival;
        }
        // Dann wird der Absolutwert als vorzeichenlose Zahl ausgegeben.
        return os << static_cast<unsigned long>(ival);
    }

    template<typename T>
    friend T &operator<<(T &os, unsigned long ival) {
        unsigned long div;
        char digit;

        if (os.base == 8) {
            os.put('0');  // oktale Zahlen erhalten eine fuehrende Null
        } else if (os.base == 16) {
            os.put('0');  // hexadezimale Zahlen ein "0x"
            os.put('x');
        }

        // Bestimmung der groessten Potenz der gewaehlten Zahlenbasis, die
        // noch kleiner als die darzustellende Zahl ist.
        for (div = 1; ival / div >= static_cast<unsigned long>(os.base); div *= os.base) {}

        // ziffernweise Ausgabe der Zahl
        for (; div > 0; div /= static_cast<unsigned long>(os.base)) {
            digit = ival / div;
            if (digit < 10) {
                os.put('0' + digit);
            } else {
                os.put('a' + digit - 10);
            }
            ival %= div;
        }
        os.fill_finalize();
        return os;
    }

    // Darstellung eines Zeigers als hexadezimale ganze Zahl
    template<typename T>
    friend T &operator<<(T &os, void *ptr) {
        int oldbase = os.base;
        os.base = 16;
        os << reinterpret_cast<unsigned long>(ptr);
        os.base = oldbase;
        return os;
    }

    // Aufruf einer Manipulatorfunktion
    template<typename T>
    friend T &operator<<(T &os, T &(*f)(T &)) { return f(os); }

    // For stream formatting
    template<typename T>
    friend T &operator<<(T &os, const fillw &w) {
        os.flush();  // Flush the buffer to not modify previous output
        os.fill_width = w.w;
        return os;
    }

    template<typename T>
    friend T &operator<<(T &os, const fillc &c) {
        os.flush();
        os.fill_char = c.c;
        return os;
    }

    // Allow access to base member
    template<typename T>
    friend T &endl(T &os);

    template<typename T>
    friend T &bin(T &os);

    template<typename T>
    friend T &oct(T &os);

    template<typename T>
    friend T &dec(T &os);

    template<typename T>
    friend T &hex(T &os);
};

//
// Manipulatorfunktionen
//
// Die folgenden Funktionen erhalten und liefern jeweils eine Referenz auf
// ein OutStream Objekt. Da die Klasse OutStream einen Operator << fuer
// derartige Funktionen definiert, koennen sie mit Hilfe dieses Operators
// aufgerufen und sogar in weitere Eingaben eingebettet werden.
// Aufgabe der Manipulatoren ist, die Darstellung der nachfolgenden Ausgaben
// zu beeinflussen, z.B durch die Wahl des Zahlensystems.

// Zeilenumbruch in Ausgabe einfuegen.
template<typename T>
T &endl(T &os) {
    // os << '\r';
    os << '\n';
    os.flush();
    return os;
}

// Waehle binaeres Zahlensystem aus.
template<typename T>
T &bin(T &os) {
    os.base = 2;
    return os;
}

// Waehle oktales Zahlensystem aus.
template<typename T>
T &oct(T &os) {
    os.base = 8;
    return os;
}

// Waehle dezimales Zahlensystem aus.
template<typename T>
T &dec(T &os) {
    os.base = 10;
    return os;
}

// Waehle hexadezimales Zahlensystem aus.
template<typename T>
T &hex(T &os) {
    os.base = 16;
    return os;
}

#endif
