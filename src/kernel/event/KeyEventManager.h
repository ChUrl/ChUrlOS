#ifndef KeyEventManager_Include_H_
#define KeyEventManager_Include_H_

#include "kernel/event/KeyEventListener.h"
#include "kernel/log/Logger.h"
#include "lib/util/Vector.h"

// NOTE: Could do this more generally but we only have key events
//       Also pretty limited: One thread can have one listener as identification is done over tid

class KeyEventManager {
private:
    NamedLogger log;

    bse::vector<KeyEventListener*> listeners;

public:
    KeyEventManager(const KeyEventManager& copy) = delete;

    KeyEventManager() : log("KEvMan"), listeners(true) {}

    void init() {
        listeners.reserve();
    }

    void subscribe(KeyEventListener& sub);
    void unsubscribe(KeyEventListener& unsub);
    void broadcast(char c);  // Unblocks all input waiting threads, I don't have a method to direct input
};

#endif
