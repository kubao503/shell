#ifndef WRAPPER_H
#define WRAPPER_H

#include <memory>

// Wrapper around std::unique_ptr which behaves mostly like the std::unique_ptr
// Implements a copy constructor which in fact behaves like a move constructor
template <typename T>
struct Wrapper
{
    mutable std::unique_ptr<T> ptr{nullptr};

    template <typename U>
    Wrapper(const Wrapper<U> &other) : ptr{std::move(other.ptr)} {} // Actually move-ctor
    Wrapper(const Wrapper<T> &other) : ptr{std::move(other.ptr)} {}
    Wrapper(std::unique_ptr<T> ptr = nullptr) : ptr{std::move(ptr)} {}

    template <typename U>
    Wrapper &operator=(const Wrapper<U> &other)
    {
        ptr = std::move(other.ptr);
        return *this;
    }
    Wrapper &operator=(const Wrapper &other)
    {
        ptr = std::move(other.ptr);
        return *this;
    }

    T *operator->() { return ptr.get(); }
    const T *get() const { return ptr.get(); }
};

template <typename T, typename... Args>
Wrapper<T> make_wrapped(Args &&...args)
{
    return Wrapper<T>(std::make_unique<T>(std::forward<Args>(args)...));
}

#endif
