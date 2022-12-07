#ifndef VectorDemo_include__
#define VectorDemo_include__

#include "kernel/system/Globals.h"
#include "kernel/process/Thread.h"
#include "lib/util/Vector.h"

class VectorDemo : public Thread {
public:
    VectorDemo(const VectorDemo& copy) = delete;

    VectorDemo() : Thread("VectorDemo") {}

    void run() override;
};

#endif
