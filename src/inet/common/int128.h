/**
 * Copyright (c) 2005 Jan Ringo, www.ringos.cz
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the author be held liable for any damages arising from the
 * use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * The origin of this software must not be misrepresented; you must not claim
 * that you wrote the original software. If you use this software in a
 * product, an acknowledgment in the product documentation would be
 * appreciated but is not required. Altered source versions must be plainly
 * marked as such, and must not be misrepresented as being the original
 * software. This notice may not be removed or altered from any source
 * distribution.
 */

#ifndef __INET_INT128_H
#define __INET_INT128_H

/*
   Name: int128_t.h
   Author: Jan Ringos, http://Tringi.Mx-3.cz
   Source: http://mx-3.cz/tringi/www/int128_t
   Version: 1.1

   Modifications: Alfonso Ariza Quintana, Zoltan Bojthe et al.
 */

#include "inet/common/INETDefs.h"

namespace inet {

// CLASS
class INET_API Int128_t
{
  private:
    // Binary correct representation of signed 128bit integer
    uint64_t lo;
    int64_t hi;

  protected:
    // Some global operator functions must be friends
    friend bool operator<(const Int128_t&, const Int128_t&);
    friend bool operator==(const Int128_t&, const Int128_t&);
    friend bool operator||(const Int128_t&, const Int128_t&);
    friend bool operator&&(const Int128_t&, const Int128_t&);

#ifdef __GNUC__
    //   friend Int128_t operator <? (const Int128_t&, const Int128_t&);
    //   friend Int128_t operator >? (const Int128_t&, const Int128_t&);
#endif // ifdef __GNUC__

  public:
    void set(const char *sz);

    // Constructors
    inline Int128_t() {}
    inline Int128_t(const Int128_t& a) : lo(a.lo), hi(a.hi) {}

    inline Int128_t(const uint32_t& a) : lo(a), hi(0ll) {}
    inline Int128_t(const int32_t& a) : lo(a), hi(0ll)
    {
        if (a < 0) hi = -1ll;
    }

    inline Int128_t(const uint64_t& a) : lo(a), hi(0ll) {}
    inline Int128_t(const int64_t& a) : lo(a), hi(0ll)
    {
        if (a < 0) hi = -1ll;
    }

    Int128_t(const float a);
    Int128_t(const double& a);
    Int128_t(const long double& a);

    Int128_t(const char *sz) { set(sz); }

    // TODO: Consider creation of operator= to eliminate
    //       the need of intermediate objects during assignments.

    Int128_t& operator=(const Int128_t& other) { lo = other.lo; hi = other.hi; return *this; }
    Int128_t& operator=(const int32_t& a) { lo = a; hi = 0; return *this; }
    Int128_t& operator=(const uint32_t& a) { lo = a; hi = 0; return *this; }

    Int128_t& operator=(const int64_t& a) { lo = a; hi = 0; return *this; }
    Int128_t& operator=(const uint64_t& a) { lo = a; hi = 0; return *this; }

    Int128_t& operator=(const char *sz) { set(sz); return *this; }
    Int128_t& operator=(const float& a);
    Int128_t& operator=(const double& a);
    Int128_t& operator=(const long double& a);

  private:
    // Special internal constructors
    Int128_t(const uint64_t& a, const int64_t& b)
        : lo(a), hi(b) {}

  public:
    // Operators
    bool operator!() const { return !(hi || lo); }

    Int128_t operator-() const;
    Int128_t operator~() const { return Int128_t(~lo, ~hi); }

    Int128_t& operator++();
    Int128_t& operator--();
    Int128_t operator++(int);
    Int128_t operator--(int);

    Int128_t& operator+=(const Int128_t& b);
    Int128_t& operator*=(const Int128_t& b);

    Int128_t& operator>>=(unsigned int n);
    Int128_t& operator<<=(unsigned int n);

    Int128_t& operator|=(const Int128_t& b) { hi |= b.hi; lo |= b.lo; return *this; }
    Int128_t& operator&=(const Int128_t& b) { hi &= b.hi; lo &= b.lo; return *this; }
    Int128_t& operator^=(const Int128_t& b) { hi ^= b.hi; lo ^= b.lo; return *this; }

    // Inline simple operators
    inline const Int128_t& operator+() const { return *this; }

    // Rest of inline operators
    inline Int128_t& operator-=(const Int128_t& b)
    {
        return *this += (-b);
    }

    inline Int128_t& operator/=(const Int128_t& b)
    {
        Int128_t dummy;
        *this = this->div(b, dummy);
        return *this;
    }

    inline Int128_t& operator%=(const Int128_t& b)
    {
        this->div(b, *this);
        return *this;
    }

    // Common methods
    int toInt() const { return (int)lo; }
    int64_t toInt64() const { return (int64_t)lo; }

    const char *toString(uint32_t radix = 10) const;
    float toFloat() const;
    double toDouble() const;
    long double toLongDouble() const;

    // Arithmetic methods
    Int128_t div(const Int128_t&, Int128_t&) const;

    // Bit operations
    bool bit(unsigned int n) const;
    void bit(unsigned int n, bool val);

    operator double() { return toDouble(); }
    operator int() { return toInt(); }
    static const Int128_t INT128_MAX;
    static const Int128_t INT128_MIN;
}
#ifdef __GNUC__
__attribute__((__aligned__(16), __packed__))
#endif // ifdef __GNUC__
;

// GLOBAL OPERATORS

bool operator<(const Int128_t& a, const Int128_t& b);

inline bool operator==(const Int128_t& a, const Int128_t& b)
{
    return a.hi == b.hi && a.lo == b.lo;
}

inline bool operator&&(const Int128_t& a, const Int128_t& b)
{
    return (a.hi || a.lo) && (b.hi || b.lo);
}

inline bool operator||(const Int128_t& a, const Int128_t& b)
{
    return (a.hi || a.lo) || (b.hi || b.lo);
}

#ifdef __GNUC__
// inline Int128_t operator <? (const Int128_t& a, const Int128_t& b) {
//     return (a < b) ? a : b; }
// inline Int128_t operator >? (const Int128_t& a, const Int128_t& b) {
//     return (a < b) ? b : a; }
#endif // ifdef __GNUC__

// GLOBAL OPERATOR INLINES

inline Int128_t operator+(const Int128_t& a, const Int128_t& b)
{
    return Int128_t(a) += b;
}

inline Int128_t operator-(const Int128_t& a, const Int128_t& b)
{
    return Int128_t(a) -= b;
}

inline Int128_t operator*(const Int128_t& a, const Int128_t& b)
{
    return Int128_t(a) *= b;
}

inline Int128_t operator/(const Int128_t& a, const Int128_t& b)
{
    return Int128_t(a) /= b;
}

inline Int128_t operator%(const Int128_t& a, const Int128_t& b)
{
    return Int128_t(a) %= b;
}

inline Int128_t operator>>(const Int128_t& a, unsigned int n)
{
    return Int128_t(a) >>= n;
}

inline Int128_t operator<<(const Int128_t& a, unsigned int n)
{
    return Int128_t(a) <<= n;
}

inline Int128_t operator&(const Int128_t& a, const Int128_t& b)
{
    return Int128_t(a) &= b;
}

inline Int128_t operator|(const Int128_t& a, const Int128_t& b)
{
    return Int128_t(a) |= b;
}

inline Int128_t operator^(const Int128_t& a, const Int128_t& b)
{
    return Int128_t(a) ^= b;
}

inline bool operator>(const Int128_t& a, const Int128_t& b)
{
    return b < a;
}

inline bool operator<=(const Int128_t& a, const Int128_t& b)
{
    return !(b < a);
}

inline bool operator>=(const Int128_t& a, const Int128_t& b)
{
    return !(a < b);
}

inline bool operator!=(const Int128_t& a, const Int128_t& b)
{
    return !(a == b);
}

// MISC

//typedef Int128_t __int128;

} // namespace inet

#endif

