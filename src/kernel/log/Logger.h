#ifndef Logger_Include_H_
#define Logger_Include_H_

#include "device/graphics/CGA.h"
#include "lib/stream/OutStream.h"
#include "lib/async/SpinLock.h"
#include "lib/util/String.h"
#include "lib/util/StringView.h"

class Logger : public OutStream {
    friend class NamedLogger;  // Allow NamedLogger to lock/unlock

public:
    static Logger& instance() {
        static Logger log;
        return log;
    }

private:
    Logger() = default;

    static bool kout_enabled;
    static bool serial_enabled;

    void log(bse::string_view message, CGA::color col) const;

    SpinLock sem;              // Semaphore would be a cyclic include
    static void lock() { Logger::instance().sem.acquire(); }
    static void unlock() { Logger::instance().sem.release(); }
    // static void lock() {}
    // static void unlock() {}

public:
//    ~Logger() override = default;

    Logger(const Logger& copy) = delete;
    void operator=(const Logger& copy) = delete;

    enum LogLevel {
        TRACE,
        DEBUG,
        ERROR,
        INFO
    };
    static LogLevel level;
    LogLevel current_message_level = Logger::INFO;  // Use this to log with manipulators

    void flush() override;

    void trace(bse::string_view message) const;
    void debug(bse::string_view message) const;
    void error(bse::string_view message) const;
    void info(bse::string_view message) const;

    // TODO: Make lvl change accessible over menu
    static void set_level(LogLevel lvl) {
        Logger::level = lvl;
    }

    static bse::string_view level_to_string(LogLevel lvl) {
        switch (lvl) {
        case Logger::TRACE:
            return "TRC";
        case Logger::DEBUG:
            return "DBG";
        case Logger::ERROR:
            return "ERR";
        case Logger::INFO:
            return "INF";
        }
    }

    static void enable_kout() {
        Logger::kout_enabled = true;
    }
    static void disable_kout() {
        Logger::kout_enabled = false;
    }
    static void enable_serial() {
        Logger::serial_enabled = true;
    }
    static void disable_serial() {
        Logger::serial_enabled = false;
    }
};

// Manipulatoren
Logger& TRACE(Logger& log);
Logger& DEBUG(Logger& log);
Logger& ERROR(Logger& log);
Logger& INFO(Logger& log);

class NamedLogger {
private:
    const char* name;

public:
    explicit NamedLogger(const char* name) : name(name) {}

    Logger& trace() {
        Logger::lock();
        return Logger::instance() << TRACE << name << " :: ";
    }

    Logger& debug() {
        Logger::lock();
        return Logger::instance() << DEBUG << name << " :: ";
    }

    Logger& error() {
        Logger::lock();
        return Logger::instance() << ERROR << name << " :: ";
    }

    Logger& info() {
        Logger::lock();
        return Logger::instance() << INFO << name << " :: ";
    }
};

#endif
