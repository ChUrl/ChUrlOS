#ifndef BlueScreenDemo_include__
#define BlueScreenDemo_include__

#include "kernel/system/Globals.h"
#include "kernel/process/Thread.h"
#include "kernel/event/KeyEventListener.h"

class PagingDemo : public Kernel::Thread {
private:
    void writeprotect_page();

    void free_page();

    void notpresent_page();

    Kernel::KeyEventListener listener;

    static NamedLogger log;

public:
    PagingDemo(const PagingDemo &copy) = delete;

    PagingDemo() : listener(tid) {
        Kernel::kevman.subscribe(listener);
    }

    ~PagingDemo() override {
        Kernel::kevman.unsubscribe(listener);
    }

    void run() override;
};

#endif
