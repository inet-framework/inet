//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TRACED_H
#define __INET_TRACED_H

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * @brief A template class that wraps a variable of type T (usually an primitive type like int,
 * double, enum, etc.), and monitors its value for changes. On each change, the callback function
 * is called with the old and the new value. This class is particularly useful for automatic
 * logging or recording of state variables in a module.
 */
template<typename T>
class Traced {
  private:
    T value_;
    std::function<void(T,T)> callback_;

    void setValue(const T& newValue) {
        T oldValue = value_;
        value_ = newValue;
        if (callback_)
            callback_(oldValue, newValue);
    }

  public:
    Traced() = default;

    Traced(const T& value) : value_(value) {}

    Traced(const T& value, std::function<void(T,T)> callback) : value_(value), callback_(callback) {}

    // Set callback function
    void addCallback(std::function<void(T,T)> callback) {
        if (!callback_)
            callback_ = callback;
        else
            // chaining
            // NOTE: callback_=callback_ is needed instead of 'this' to eliminate different warnings on both std=c++17 and std=c++20
            callback_ = [=, callback_=callback_] (T oldValue, T newValue) {
                callback_(oldValue, newValue);
                callback(oldValue, newValue);
            };
    }

    // Set up to emit signal on changes
    void addEmitCallback(cComponent *component, simsignal_t signal) {
        auto callback = [=](T oldValue, T newValue) {
            if (oldValue != newValue)
                component->emit(signal, newValue);
        };
        addCallback(callback);
    }

    void removeCallbacks() {
        callback_ = nullptr;
    }

    // Overload assignment operator
    Traced& operator=(const T& value) {
        setValue(value);
        return *this;
    }

    // Overload for += operator
    Traced& operator+=(const T& rhs) {
        setValue(value_ + rhs);
        return *this;
    }

    // Overload for -= operator
    Traced& operator-=(const T& rhs) {
        setValue(value_ - rhs);
        return *this;
    }

    // Overload for *= operator
    Traced& operator*=(const T& rhs) {
        setValue(value_ * rhs);
        return *this;
    }

    // Overload for /= operator
    Traced& operator/=(const T& rhs) {
        setValue(value_ / rhs);
        return *this;
    }

    // Overload for %= operator (only for integral types)
    template<typename U = T>
    typename std::enable_if<std::is_integral<U>::value, Traced&>::type operator%=(const T& rhs) {
        setValue(value_ % rhs);
        return *this;
    }

    // Overload for &= operator (only for integral types)
    template<typename U = T>
    typename std::enable_if<std::is_integral<U>::value, Traced&>::type operator&=(const T& rhs) {
        setValue(value_ & rhs);
        return *this;
    }

    // Overload for |= operator (only for integral types)
    template<typename U = T>
    typename std::enable_if<std::is_integral<U>::value, Traced&>::type operator|=(const T& rhs) {
        setValue(value_ | rhs);
        return *this;
    }

    // Overload for ^= operator (only for integral types)
    template<typename U = T>
    typename std::enable_if<std::is_integral<U>::value, Traced&>::type operator^=(const T& rhs) {
        setValue(value_ ^ rhs);
        return *this;
    }

    // Overload for <<= operator (only for integral types)
    template<typename U = T>
    typename std::enable_if<std::is_integral<U>::value, Traced&>::type operator<<=(const T& rhs) {
        setValue(value_ << rhs);
        return *this;
    }

    // Overload for >>= operator (only for integral types)
    template<typename U = T>
    typename std::enable_if<std::is_integral<U>::value, Traced&>::type operator>>=(const T& rhs) {
        setValue(value_ >> rhs);
        return *this;
    }

    // Overload type conversion operator to allow transparent use as T
    operator T() const {
        return value_;
    }

};

} // namespace inet

#endif

