//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_INTRUSIVEPTR_H
#define __INET_INTRUSIVEPTR_H

#include <iosfwd>

#include "inet/common/INETDefs.h"

#define INET_ALWAYS_INLINE    __attribute__((always_inline)) inline

namespace inet {

/**
 * A smart pointer that uses intrusive reference counting.
 *
 * Relies on unqualified calls to
 *     void intrusivePtrCounterIncrement(T *p);
 *     void intrusivePtrCounterDecrement(T *p);
 * The object is responsible for destroying itself.
 */
template<class T> class IntrusivePtr
{
  private:
    T *p;

  public:
    INET_ALWAYS_INLINE constexpr IntrusivePtr() noexcept : p(nullptr) {}

    INET_ALWAYS_INLINE constexpr IntrusivePtr(std::nullptr_t) : p(nullptr) {}

    INET_ALWAYS_INLINE explicit IntrusivePtr(T *p) : p(p) {
        if (p != 0) intrusivePtrCounterIncrement(p);
    }

    template<class U>
    INET_ALWAYS_INLINE IntrusivePtr(IntrusivePtr<U> const& ptr) : p(ptr.get()) {
        if (p != 0) intrusivePtrCounterIncrement(p);
    }

    INET_ALWAYS_INLINE IntrusivePtr(IntrusivePtr const& ptr) : p(ptr.p) {
        if (p != 0) intrusivePtrCounterIncrement(p);
    }

    INET_ALWAYS_INLINE ~IntrusivePtr() {
        if (p != 0) intrusivePtrCounterDecrement(p);
    }

    template<class U>
    INET_ALWAYS_INLINE IntrusivePtr& operator=(IntrusivePtr<U> const& ptr) {
        IntrusivePtr(ptr).swap(*this);
        return *this;
    }

    INET_ALWAYS_INLINE IntrusivePtr(IntrusivePtr&& ptr) noexcept : p(ptr.p) {
        ptr.p = 0;
    }

    INET_ALWAYS_INLINE IntrusivePtr& operator=(IntrusivePtr&& ptr) noexcept {
        IntrusivePtr(static_cast<IntrusivePtr &&>(ptr)).swap(*this);
        return *this;
    }

    template<class U> friend class IntrusivePtr;

    template<class U>
    INET_ALWAYS_INLINE IntrusivePtr(IntrusivePtr<U>&& ptr) : p(ptr.p) {
        ptr.p = 0;
    }

    template<class U>
    INET_ALWAYS_INLINE IntrusivePtr& operator=(IntrusivePtr<U>&& ptr) noexcept {
        IntrusivePtr(static_cast<IntrusivePtr<U> &&>(ptr)).swap(*this);
        return *this;
    }

    INET_ALWAYS_INLINE IntrusivePtr& operator=(IntrusivePtr const& ptr) {
        IntrusivePtr(ptr).swap(*this);
        return *this;
    }

    INET_ALWAYS_INLINE IntrusivePtr& operator=(T *ptr) {
        IntrusivePtr(ptr).swap(*this);
        return *this;
    }

    INET_ALWAYS_INLINE void reset() {
        IntrusivePtr().swap(*this);
    }

    INET_ALWAYS_INLINE void reset(T *ptr) {
        IntrusivePtr(ptr).swap(*this);
    }

    INET_ALWAYS_INLINE void reset(T *ptr, bool add_ref) {
        IntrusivePtr(ptr, add_ref).swap(*this);
    }

    INET_ALWAYS_INLINE T *get() const noexcept { return p; }

    INET_ALWAYS_INLINE T *detach() noexcept {
        T *ret = p;
        p = 0;
        return ret;
    }

    INET_ALWAYS_INLINE T& operator*() const noexcept { return *p; }

    INET_ALWAYS_INLINE T *operator->() const noexcept { return p; }

    INET_ALWAYS_INLINE explicit operator bool() const noexcept { return p != 0; }

    INET_ALWAYS_INLINE void swap(IntrusivePtr& ptr) noexcept {
        T *tmp = p;
        p = ptr.p;
        ptr.p = tmp;
    }

    INET_ALWAYS_INLINE unsigned int use_count() const noexcept { return p->use_count(); }
};

template<class T, class U>
INET_ALWAYS_INLINE bool operator==(IntrusivePtr<T> const& a, IntrusivePtr<U> const& b) noexcept
{
    return a.get() == b.get();
}

template<class T, class U>
INET_ALWAYS_INLINE bool operator!=(IntrusivePtr<T> const& a, IntrusivePtr<U> const& b) noexcept
{
    return a.get() != b.get();
}

template<class T, class U>
INET_ALWAYS_INLINE bool operator==(IntrusivePtr<T> const& a, U *b) noexcept
{
    return a.get() == b;
}

template<class T, class U>
INET_ALWAYS_INLINE bool operator!=(IntrusivePtr<T> const& a, U *b) noexcept
{
    return a.get() != b;
}

template<class T, class U>
INET_ALWAYS_INLINE bool operator==(T *a, IntrusivePtr<U> const& b) noexcept
{
    return a == b.get();
}

template<class T, class U>
INET_ALWAYS_INLINE bool operator!=(T *a, IntrusivePtr<U> const& b) noexcept
{
    return a != b.get();
}

template<class T>
INET_ALWAYS_INLINE bool operator==(IntrusivePtr<T> const& p, std::nullptr_t) noexcept
{
    return p.get() == 0;
}

template<class T>
INET_ALWAYS_INLINE bool operator==(std::nullptr_t, IntrusivePtr<T> const& p) noexcept
{
    return p.get() == 0;
}

template<class T>
INET_ALWAYS_INLINE bool operator!=(IntrusivePtr<T> const& p, std::nullptr_t) noexcept
{
    return p.get() != 0;
}

template<class T>
INET_ALWAYS_INLINE bool operator!=(std::nullptr_t, IntrusivePtr<T> const& p) noexcept
{
    return p.get() != 0;
}

template<class T>
INET_ALWAYS_INLINE bool operator<(IntrusivePtr<T> const& a, IntrusivePtr<T> const& b) noexcept
{
    return std::less<T *>()(a.get(), b.get());
}

template<class T>
INET_ALWAYS_INLINE void swap(IntrusivePtr<T>& lhs, IntrusivePtr<T>& ptr) noexcept
{
    lhs.swap(ptr);
}

template<class T>
INET_ALWAYS_INLINE T *get_pointer(IntrusivePtr<T> const& p) noexcept
{
    return p.get();
}

template<class T, class U>
INET_ALWAYS_INLINE IntrusivePtr<T> static_pointer_cast(IntrusivePtr<U> const& p)
{
    return IntrusivePtr<T>(static_cast<T *>(p.get()));
}

template<class T, class U>
INET_ALWAYS_INLINE IntrusivePtr<T> const_pointer_cast(IntrusivePtr<U> const& p)
{
    return IntrusivePtr<T>(const_cast<T *>(p.get()));
}

template<class T, class U>
INET_ALWAYS_INLINE IntrusivePtr<T> dynamic_pointer_cast(IntrusivePtr<U> const& p)
{
    return IntrusivePtr<T>(dynamic_cast<T *>(p.get()));
}

template<class Y>
INET_ALWAYS_INLINE std::ostream& operator<<(std::ostream& os, IntrusivePtr<Y> const& p)
{
    os << p.get();
    return os;
}

template<typename T>
class IntrusivePtrCounter;

template<typename T>
INET_ALWAYS_INLINE void intrusivePtrCounterIncrement(const IntrusivePtrCounter<T> *p) noexcept;
template<typename T>
INET_ALWAYS_INLINE void intrusivePtrCounterDecrement(const IntrusivePtrCounter<T> *p) noexcept;

template<typename T>
class IntrusivePtrCounter
{
  private:
    mutable unsigned int c;

  public:
    INET_ALWAYS_INLINE IntrusivePtrCounter() noexcept : c(0) {}
    INET_ALWAYS_INLINE IntrusivePtrCounter(IntrusivePtrCounter const&) noexcept : c(0) {}

    INET_ALWAYS_INLINE unsigned int use_count() const noexcept { return c; }

    INET_ALWAYS_INLINE IntrusivePtrCounter& operator=(IntrusivePtrCounter const&) noexcept { return *this; }

    INET_ALWAYS_INLINE IntrusivePtr<T> shared_from_this() { return IntrusivePtr<T>(static_cast<T *>(this)); }

  protected:
    INET_ALWAYS_INLINE ~IntrusivePtrCounter() = default;

    friend void intrusivePtrCounterIncrement<T>(const IntrusivePtrCounter<T> *p) noexcept;
    friend void intrusivePtrCounterDecrement<T>(const IntrusivePtrCounter<T> *p) noexcept;
};

template<typename T>
INET_ALWAYS_INLINE void intrusivePtrCounterIncrement(const IntrusivePtrCounter<T> *p) noexcept
{
    ++(p->c);
}

template<typename T>
INET_ALWAYS_INLINE void intrusivePtrCounterDecrement(const IntrusivePtrCounter<T> *p) noexcept
{
    if (--(p->c) == 0)
        delete static_cast<const T *>(p);
}

} // namespace inet

#endif

