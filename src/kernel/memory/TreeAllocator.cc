#include "TreeAllocator.h"
#include "kernel/system/Globals.h"

void TreeAllocator::init() {
    free_start = reinterpret_cast<tree_block_t*>(heap_start);
    free_start->allocated = false;
    free_start->left = nullptr;
    free_start->right = nullptr;
    free_start->parent = nullptr;
    free_start->red = false;  // The root is always black
    free_start->next = reinterpret_cast<list_block_t*>(free_start);
    free_start->previous = reinterpret_cast<list_block_t*>(free_start);

    log.info() << "Initialized Tree Allocator" << endl;
}

void TreeAllocator::dump_free_memory() {
    kout << "Free Memory:" << endl;
    list_block_t* current = reinterpret_cast<list_block_t*>(heap_start);
    do {
        if (!current->allocated) {
            kout << " - Free Block at " << reinterpret_cast<unsigned int>(current) << ", Size: "
                 << reinterpret_cast<unsigned int>(current->next) - reinterpret_cast<unsigned int>(current)
                 << endl;
        }
        current = current->next;
    } while (reinterpret_cast<unsigned int>(current) != heap_start);
}

void* TreeAllocator::alloc(unsigned int req_size) {
    log.debug() << "Requested " << dec << req_size << " Bytes" << endl;

    // Round to word borders + tree_block size
    unsigned int rreq_size = req_size;
    if (rreq_size < sizeof(tree_block_t) - sizeof(list_block_t)) {
        // the list_block_t is part of every block, but when freeing
        // memory we need enough space to store the rbt metadata
        rreq_size = sizeof(tree_block_t) - sizeof(list_block_t);
        log.trace() << " - Increased block size for rbt metadata" << endl;
    }
    unsigned int req_size_diff = (BASIC_ALIGN - rreq_size % BASIC_ALIGN) % BASIC_ALIGN;
    rreq_size = rreq_size + req_size_diff;
    if (req_size_diff > 0) {
        log.trace() << " - Rounded to word border (+" << dec << req_size_diff << " bytes)" << endl;
    }

    // Finds smallest block that is large enough
    tree_block_t* best_fit = rbt_search_bestfit(rreq_size);
    if (best_fit == nullptr) {
        log.error() << " - No block found" << endl;
        return nullptr;
    }
    if (best_fit->allocated) {
        // Something went really wrong
        log.error() << " - Block already allocated :(" << endl;
        return nullptr;
    }
    best_fit->allocated = true;
    unsigned int size = get_size(best_fit);
    log.trace() << " - Found best-fit: " << hex << reinterpret_cast<unsigned int>(best_fit) << endl;

    // HACK: I didn't want to handle situations with only one block (where the tree root would
    //       get removed), so I make sure there are always at least 2 blocks by inserting a dummy
    //       block. This is not fast at all but it was fast to code...
    //       I should change this so it only happens when only one block exists in the freelist
    tree_block_t dummy;
    dummy.allocated = false;
    rbt_insert(&dummy);  // I use the address of the stack allocated struct because it is
                               // removed before exiting the function

    rbt_remove(best_fit);  // BUG: Can trigger bluescreen
    if (size > HEAP_MIN_FREE_BLOCK_SIZE + rreq_size + sizeof(list_block_t)) {
        // Block can be cut
        log.trace() << " - Allocating " << dec << rreq_size << " Bytes with cutting" << endl;

        // [best_fit_start | sizeof(list_block_t) | rreq_size | new_block_start]
        tree_block_t* new_block
          = reinterpret_cast<tree_block_t*>(reinterpret_cast<char*>(best_fit) + sizeof(list_block_t) + rreq_size);
        new_block->allocated = false;
        dll_insert(best_fit, new_block);
        rbt_insert(new_block);
    } else {
        // Don't cut block
        // The block is already correctly positioned in the linked list so we only
        // need to remove it from the freelist, which is done for both cases
        log.trace() << " - Allocating " << dec << rreq_size << " Bytes without cutting" << endl;
    }

    // HACK: Remove the dummy element
    rbt_remove(&dummy);

    log.trace() << " - Returned address " << hex
                << reinterpret_cast<unsigned int>(reinterpret_cast<char*>(best_fit) + sizeof(list_block_t))
                << endl;
    return reinterpret_cast<void*>(reinterpret_cast<char*>(best_fit) + sizeof(list_block_t));
}

void TreeAllocator::free(void* ptr) {
    log.info() << "Freeing " << hex << reinterpret_cast<unsigned int>(ptr) << endl;

    list_block_t* block = reinterpret_cast<list_block_t*>(reinterpret_cast<char*>(ptr) - sizeof(list_block_t));
    if (!block->allocated) {
        // Block already free
        return;
    }
    block->allocated = false;  // If the block is merged backwards afterwards this is unnecessary

    list_block_t* previous = block->previous;
    list_block_t* next = block->next;

    if (next->allocated && previous->allocated) {
        // No merge
        rbt_insert(reinterpret_cast<tree_block_t*>(block));
        return;
    }

    // HACK: Same as when allocating
    tree_block_t dummy;
    dummy.allocated = false;
    rbt_insert(&dummy);  // I use the address of the stack allocated struct because it is

    if (!next->allocated) {
        // Merge forward
        log.trace() << " - Merging forward" << endl;

        // Remove the next block from all lists as it is now part of our freed block
        dll_remove(next);
        rbt_remove(reinterpret_cast<tree_block_t*>(next));  // BUG: Bluescreen if next is the only block in the freelist
        if (previous->allocated) {
            // Don't insert if removed later because of backward merge
            rbt_insert(reinterpret_cast<tree_block_t*>(block));
        }
    }

    if (!previous->allocated) {
        // Merge backward
        log.trace() << " - Merging backward" << endl;

        // Remove the current block from all lists as it is now part of the previous block
        // It doesn't have to be removed from rbt as it wasn't in there as it was allocated before
        dll_remove(block);
        rbt_remove(reinterpret_cast<tree_block_t*>(previous));
        rbt_insert(reinterpret_cast<tree_block_t*>(previous));  // Reinsert with new size
    }

    // HACK: Same as when allocating
    rbt_remove(&dummy);
}

unsigned int TreeAllocator::get_size(list_block_t* block) const {
    if (block->next == block) {
        // Only one block exists
        return heap_end - (reinterpret_cast<unsigned int>(block) + sizeof(list_block_t));
    }

    if (reinterpret_cast<unsigned int>(block->next) > reinterpret_cast<unsigned int>(block)) {
        // Next block is placed later in memory
        return reinterpret_cast<unsigned int>(block->next) - (reinterpret_cast<unsigned int>(block) + sizeof(list_block_t));
    }

    // Next block is placed earlier in memory which means block is at memory end
    return reinterpret_cast<unsigned int>(heap_end) - (reinterpret_cast<unsigned int>(block) + sizeof(list_block_t));
}
