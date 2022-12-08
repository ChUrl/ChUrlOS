#ifndef VECTOR_INCLUDE_H_
#define VECTOR_INCLUDE_H_

// NOTE: I decided to implement this because I wanted some sort of dynamic array (for example for the keyeventmanager).
//       Also I wanted to template the Queue (for the scheduler) but with this I can just replace the Queue and use the
//       ArrayList instead

#include "Iterator.h"
#include <utility>

// https://en.cppreference.com/w/cpp/container/vector
namespace Container {

/**
 * This class implements a dynamically allocated array list.
 *
 * @tparam T The type of the held objects
 */
template<typename T>
class Vector {
public:
    using iterator = ContinuousIterator<T>;

private:
    static constexpr const std::size_t default_cap = 16;  // Arbitrary but very small
    static constexpr const std::size_t min_cap = 8;       // Slots to allocate extra when array full

    T *buf = nullptr;  // Heap allocated as size needs to change during runtime
    std::size_t sz = 0; // The current size of the buffer (marks the slot where to insert a new element)
    std::size_t buf_cap = 0; // The current capacity of the buffer

    void init(std::size_t cap = Vector::default_cap) {
        if (buf != nullptr) {
            return;
        }
        buf = new T[cap];
        buf_cap = cap;
    }

    [[nodiscard]] std::size_t get_rem_cap() const {
        return buf_cap - size();
    }

    // Enlarges the buffer if we run out of space
    void min_expand() {
        // Init if necessary
        if (buf == nullptr) {
            init();
            return;  // Dont have to realloc after init
        }

        // Since we only ever add single elements this should never get below zero
        if (get_rem_cap() < min_cap) {
            switch_buf(buf_cap + min_cap);
        }
    }

    // 1. Allocates new buffer
    // 2. Moves stuff to new buffer
    // 3. Deletes old buffer
    // 4. Sets new pos/cap
    void switch_buf(std::size_t cap) {
        // Alloc new array
        T *new_buf = new T[cap];

        // Swap current elements to new array
        for (std::size_t i = 0; i < size(); ++i) {
            new_buf[i] = std::move(buf[i]);
            buf[i].~T();  // TODO: I think delete[] buf calls these, verify that
        }

        // Move new array to buf, deleting the old array
        delete[] buf;
        buf = new_buf;
        buf_cap = cap;
    }

    // Index is location where space should be made
    void copy_right(std::size_t i) {
        if (i >= size()) {
            return; // We don't need to copy anything as space is already there
        }

        // TODO: Delete instead of ~T()?
        for (std::size_t idx = size(); idx > i; --idx) {
            buf[idx].~T(); // Delete previously contained element that will be overridden
            buf[idx] = std::move(buf[idx - 1]); // This leaves a "shell" of the old object that has to be deleted
            buf[idx - 1].~T(); // Delete element in moved-out state
        }
    }

    // Index is the location that will be removed
    void copy_left(std::size_t i) {
        if (i >= size()) {
            return; // We don't need to copy anything as nothing will be overridden
        }

        // TODO: Delete instead of ~T()?
        for (std::size_t idx = i; idx < size(); ++idx) {
            buf[idx].~T();  // Delete the element that will be overwritten
            buf[idx] = std::move(buf[idx + 1]);
            buf[idx + 1].~T();
        }
    }

public:
    explicit Vector(bool lazy = false) {
        if (!lazy) {
            // I added this as a workaround, the scheduler can't initialize the queues right
            // away because when the scheduler is started the allocator is not ready
            init();
        }
    };

    // Initialize like this: bse::vector<int> vec {1, 2, 3, 4, 5};
    /**
     * Initialize the Vector with initial values provided by an initializer list.
     * The Vector will have as many slots as initial values are provided.
     *
     * @param list The initial values
     */
    Vector(std::initializer_list<T> list) : sz(list.size()), buf_cap(sz), buf(new T[buf_cap]) {
        // TODO: Exception on wrong size? Can't static_assert on list.size()
        typename std::initializer_list<T>::iterator it = list.begin();

        for (unsigned int i = 0; i < sz; ++i) {
            buf[i] = *it;
            ++it;
        }
    }

    Vector(const Vector &copy) : sz(copy.sz), buf_cap(copy.buf_cap), buf(new T[buf_cap]) {
        for (unsigned int i = 0; i < sz; ++i) {
            buf[i] = copy[i]; // Does a copy since copy is marked const reference
        }
    }

    Vector &operator=(const Vector &copy) {
        if (this != &copy) {
            ~Vector();

            buf_cap = copy.buf_cap;
            sz = copy.sz;
            buf = new T[buf_cap];
            for (unsigned int i = 0; i < sz; ++i) {
                buf[i] = copy[i];
            }
        }
        return *this;
    }

    Vector(Vector &&move) noexcept: buf(move.buf), sz(move.sz), buf_cap(move.buf_cap) {
        move.buf_cap = 0;
        move.sz = 0;
        move.buf = nullptr;
    }

    Vector &operator=(Vector &&move) noexcept {
        if (this != &move) {
            buf_cap = move.buf_cap;
            sz = move.sz;
            buf = move.buf;

            move.buf_cap = 0;
            move.sz = 0;
            move.buf = nullptr;
        }
        return *this;
    }

    ~Vector() {
        if (buf == nullptr) {
            return;
        }

        for (std::size_t i = 0; i < size(); ++i) {
            buf[i].~T();  // TODO: I think delete[] buf calls these, verify that
        }
        delete[] buf;
    }

    // Iterator
    iterator begin() { return iterator(&buf[0]); }

    iterator begin() const { return iterator(&buf[0]); }

    iterator end() { return iterator(&buf[size()]); }

    iterator end() const { return iterator(&buf[size()]); }

    // Add elements
    // https://en.cppreference.com/w/cpp/container/vector/push_back
    void push_back(const T &copy) {
        buf[size()] = copy;
        ++sz;
        min_expand();
    }

    void push_back(T &&move) {
        buf[size()] = std::move(move);
        ++sz;
        min_expand();
    }

    // https://en.cppreference.com/w/cpp/container/vector/insert
    // The element will be inserted before the pos iterator, pos can be the end() iterator
    iterator insert(iterator pos, const T &copy) {
        std::size_t idx = distance(begin(), pos);  // begin() does init if necessary
        copy_right(idx);                           // nothing will be done if pos == end()
        buf[idx] = copy;
        ++sz;
        min_expand();
        return iterator(&buf[idx]);
    }

    iterator insert(iterator pos, T &&move) {
        std::size_t idx = distance(begin(), pos);  // begin() does init if necessary
        copy_right(idx);
        buf[idx] = std::move(move);
        ++sz;
        min_expand();
        return iterator(&buf[idx]);
    }

    // Remove elements
    // https://en.cppreference.com/w/cpp/container/vector/erase
    // Returns the iterator after the removed element, pos can't be end() iterator
    iterator erase(iterator pos) {
        std::size_t idx = distance(begin(), pos);
        copy_left(idx);
        --sz;
        // shrink();
        return iterator(&buf[idx]);
    }

    // Access
    T &front() {
        return buf[0];
    }

    const T &front() const {
        return buf[0];
    }

    T &back() {
        return buf[sz - 1];
    }

    const T &back() const {
        return buf[sz - 1];
    }

    T &operator[](std::size_t pos) {
        return buf[pos];
    }

    const T &operator[](std::size_t pos) const {
        return buf[pos];
    }

    // Misc
    [[nodiscard]] bool empty() const {
        return !static_cast<bool>(sz);
    }

    [[nodiscard]] std::size_t size() const {
        return sz;
    }

    void clear() {
        while (sz > 0) {
            --sz;
            buf[sz].~T();
        }
    }

    void reserve(std::size_t cap = Vector::default_cap) {
        // The first reserve could allocate double if cap != default_cap
        if (buf == nullptr) {
            // Directly init with correct size
            init(cap);
            return;
        }

        if (cap == buf_cap) {
            // Would change nothing
            return;
        }

        switch_buf(cap);
    }

    [[nodiscard]] bool initialized() const {
        return buf != nullptr;
    }
};

// Erase all elements that match a predicate
// NOTE: pred is no real predicate as one would need closures for this, but we don't have <functional> available
//       This means the result has to be passed separately and the function differs from the c++20 std::erase_if
template<typename T, typename arg>
std::size_t erase_if(Vector<T> &vec, arg (*pred)(const T &), arg result) {
    std::size_t erased_els = 0;
    for (typename Vector<T>::Iterator it = vec.begin(); it != vec.end(); /*Do nothing*/) {
        if (pred(*it) == result) {
            it = vec.erase(it);  // erase returns the iterator to the next element
            ++erased_els;
        } else {
            ++it;  // move forward when nothing was deleted
        }
    }
    return erased_els;
}

}  // namespace Container

#endif
