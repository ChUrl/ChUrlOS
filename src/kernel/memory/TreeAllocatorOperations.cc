#include "TreeAllocator.h"

// RBT code taken from https://github.com/Bibeknam/algorithmtutorprograms

// START copy from algorithmtutorprograms
void TreeAllocator::rbt_transplant(tree_block_t* a, tree_block_t* b) {
    if (a->parent == nullptr) {
        free_start = b;
    } else if (a == a->parent->left) {
        a->parent->left = b;
    } else {
        a->parent->right = b;
    }
    b->parent = a->parent;
}

// insert the key to the tree in its appropriate position
// and fix the tree
void TreeAllocator::rbt_insert(tree_block_t* node) {
    // Ordinary Binary Search Insertion
    node->parent = nullptr;
    node->left = nullptr;
    node->right = nullptr;
    node->red = true;  // new node must be red

    tree_block_t* y = nullptr;
    tree_block_t* x = free_start;

    while (x != nullptr) {
        y = x;
        if (get_size(node) < get_size(x)) {
            x = x->left;
        } else {
            x = x->right;
        }
    }

    // y is parent of x
    node->parent = y;
    if (y == nullptr) {
        free_start = node;
    } else if (get_size(node) < get_size(y)) {
        y->left = node;
    } else {
        y->right = node;
    }

    // if new node is a root node, simply return
    if (node->parent == nullptr) {
        node->red = false;
        return;
    }

    // if the grandparent is null, simply return
    if (node->parent->parent == nullptr) {
        return;
    }

    // Fix the tree
    rbt_fix_insert(node);
}

// fix the red-black tree
void TreeAllocator::rbt_fix_insert(tree_block_t* k) {
    tree_block_t* u;
    while (k->parent->red) {
        if (k->parent == k->parent->parent->right) {
            u = k->parent->parent->left;  // uncle
            if (u->red) {
                // case 3.1
                u->red = false;
                k->parent->red = false;
                k->parent->parent->red = true;
                k = k->parent->parent;
            } else {
                if (k == k->parent->left) {
                    // case 3.2.2
                    k = k->parent;
                    rbt_rot_r(k);
                }
                // case 3.2.1
                k->parent->red = false;
                k->parent->parent->red = true;
                rbt_rot_l(k->parent->parent);
            }
        } else {
            u = k->parent->parent->right;  // uncle

            if (u->red) {
                // mirror case 3.1
                u->red = false;
                k->parent->red = false;
                k->parent->parent->red = true;
                k = k->parent->parent;
            } else {
                if (k == k->parent->right) {
                    // mirror case 3.2.2
                    k = k->parent;
                    rbt_rot_l(k);
                }
                // mirror case 3.2.1
                k->parent->red = false;
                k->parent->parent->red = true;
                rbt_rot_r(k->parent->parent);
            }
        }
        if (k == free_start) {
            break;
        }
    }
    free_start->red = false;
}

// rotate left at node x
void TreeAllocator::rbt_rot_l(tree_block_t* x) {
    tree_block_t* y = x->right;
    x->right = y->left;
    if (y->left != nullptr) {
        y->left->parent = x;
    }
    y->parent = x->parent;
    if (x->parent == nullptr) {
        free_start = y;
    } else if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }
    y->left = x;
    x->parent = y;
}

// rotate right at node x
void TreeAllocator::rbt_rot_r(tree_block_t* x) {
    tree_block_t* y = x->left;
    x->left = y->right;
    if (y->right != nullptr) {
        y->right->parent = x;
    }
    y->parent = x->parent;
    if (x->parent == nullptr) {
        free_start = y;
    } else if (x == x->parent->right) {
        x->parent->right = y;
    } else {
        x->parent->left = y;
    }
    y->right = x;
    x->parent = y;
}

// find the node with the minimum key
tree_block_t* TreeAllocator::rbt_minimum(tree_block_t* node) {
    while (node->left != nullptr) {
        node = node->left;
    }
    return node;
}

void TreeAllocator::rbt_remove(tree_block_t* z) {
    tree_block_t* x;
    tree_block_t* y;

    y = z;
    bool y_original_red = y->red;
    if (z->left == nullptr) {
        x = z->right;
        rbt_transplant(z, z->right);
    } else if (z->right == nullptr) {
        x = z->left;
        rbt_transplant(z, z->left);
    } else {
        y = rbt_minimum(z->right);
        y_original_red = y->red;
        x = y->right;
        if (y->parent == z) {
            x->parent = y;
        } else {
            rbt_transplant(y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }

        rbt_transplant(z, y);
        y->left = z->left;
        y->left->parent = y;
        y->red = z->red;
    }
    if (!y_original_red) {
        rbt_fix_remove(x);
    }
}

// fix the rb tree modified by the delete operation
void TreeAllocator::rbt_fix_remove(tree_block_t* x) {
    tree_block_t* s;
    while (x != free_start && !x->red) {
        if (x == x->parent->left) {
            s = x->parent->right;
            if (s->red) {
                // case 3.1
                s->red = false;
                x->parent->red = true;
                rbt_rot_l(x->parent);
                s = x->parent->right;
            }

            if (!s->left->red && !s->right->red) {
                // case 3.2
                s->red = true;
                x = x->parent;
            } else {
                if (!s->right->red) {
                    // case 3.3
                    s->left->red = false;
                    s->red = true;
                    rbt_rot_r(s);
                    s = x->parent->right;
                }

                // case 3.4
                s->red = x->parent->red;
                x->parent->red = false;
                s->right->red = false;
                rbt_rot_l(x->parent);
                x = free_start;
            }
        } else {
            s = x->parent->left;
            if (s->red) {
                // case 3.1
                s->red = false;
                x->parent->red = true;
                rbt_rot_r(x->parent);
                s = x->parent->left;
            }

            if (!s->right->red) {
                // case 3.2
                s->red = true;
                x = x->parent;
            } else {
                if (!s->left->red) {
                    // case 3.3
                    s->right->red = false;
                    s->red = true;
                    rbt_rot_l(s);
                    s = x->parent->left;
                }

                // case 3.4
                s->red = x->parent->red;
                x->parent->red = false;
                s->left->red = false;
                rbt_rot_r(x->parent);
                x = free_start;
            }
        }
    }
    x->red = false;
}
// END copy from algorithmtutorprograms

// This is recursive and depends on luck
tree_block_t* TreeAllocator::rbt_search_bestfit(tree_block_t* node, unsigned int req_size) {
    if (node == nullptr) {
        return nullptr;
    }

    if (req_size < get_size(node)) {
        if (node->left != nullptr && get_size(node->left) >= req_size) {
            return rbt_search_bestfit(node->left, req_size);
        }

        return node;
    }

    if (req_size > get_size(node)) {
        if (node->right != nullptr && get_size(node->right) >= req_size) {
            return rbt_search_bestfit(node->right, req_size);
        }

        // Block doesn't fit
        return nullptr;
    }

    // Perfect fit
    return node;
}

// DLL code
void TreeAllocator::dll_insert(list_block_t* previous, list_block_t* node) {
    previous->next->previous = node;
    node->next = previous->next;
    node->previous = previous;
    previous->next = node;
}

void TreeAllocator::dll_remove(list_block_t* node) {
    node->previous->next = node->next;
    node->next->previous = node->previous;
}
