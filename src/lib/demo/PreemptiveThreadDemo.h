#ifndef preemptive_thread_include__
#define preemptive_thread_include__

#include "kernel/system/Globals.h"
#include "kernel/process/Thread.h"
#include "lib/async/Semaphore.h"

class PreemptiveLoopThread : public Kernel::Thread {
private:
    int id;

public:
    PreemptiveLoopThread(const PreemptiveLoopThread &copy) = delete;  // Verhindere Kopieren

    // Gibt der Loop einen Stack und eine Id.
    PreemptiveLoopThread(int i) : id(i) {}

    // Zaehlt einen Zaehler hoch und gibt ihn auf dem Bildschirm aus.
    void run() override;
};

class PreemptiveThreadDemo : public Kernel::Thread {
private:
    unsigned int number_of_threads;

public:
    PreemptiveThreadDemo(const PreemptiveThreadDemo &copy) = delete;  // Verhindere Kopieren

    PreemptiveThreadDemo(unsigned int n) : number_of_threads(n) {}

    // Thread-Startmethode
    void run() override;
};

#endif
