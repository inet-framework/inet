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
   Name: int128.h
   Author: Jan Ringos, http://Tringi.Mx-3.cz
   Source: http://mx-3.cz/tringi/www/int128
   Version: 1.1

   Modifications: Alfonso Ariza Quintana, Zoltan Bojthe et al.
 */

#include "inet/common/INETDefs.h"

namespace inet {

// TODO optimize *= and other operators/functions
class INET_API Int128
{
  private:
    // Binary correct representation of signed 128bit integer
    uint64_t lo;
    int64_t hi;

  protected:
    // Some global operator functions must be friends
    friend bool operator<(const Int128&, const Int128&);
    friend bool operator==(const Int128&, const Int128&);
    friend bool operator||(const Int128&, const Int128&);
    friend bool operator&&(const Int128&, const Int128&);

#ifdef __GNUC__
//    friend Int128 operator <? (const Int128&, const Int128&);
//    friend Int128 operator >? (const Int128&, const Int128&);
#endif // ifdef __GNUC__

  public:
    void set(const char *sz);

    // Constructors
    inline Int128() {}
    inline Int128(const Int128& a) : lo(a.lo), hi(a.hi) {}

    inline Int128(const uint32_t& a) : lo(a), hi(0ll) {}
    inline Int128(const int32_t& a) : lo(a), hi(0ll)
    {
        if (a < 0) hi = -1ll;
    }

    inline Int128(const uint64_t& a) : lo(a), hi(0ll) {}
    inline Int128(const int64_t& a) : lo(a), hi(0ll)
    {
        if (a < 0) hi = -1ll;
    }

    Int128(const float a);
    Int128(const double& a);
    Int128(const long double& a);

    Int128(const char *sz) { set(sz); }

    // TODO Consider creation of operator= to eliminate
    //       the need of intermediate objects during assignments.

    Int128& operator=(const Int128& other) { lo = other.lo; hi = other.hi; return *this; }
    Int128& operator=(const int32_t& a) { lo = a; hi = 0; return *this; }
    Int128& operator=(const uint32_t& a) { lo = a; hi = 0; return *this; }

    Int128& operator=(const int64_t& a) { lo = a; hi = 0; return *this; }
    Int128& operator=(const uint64_t& a) { lo = a; hi = 0; return *this; }

    Int128& operator=(const char *sz) { set(sz); return *this; }
    Int128& operator=(const float& a);
    Int128& operator=(const double& a);
    Int128& operator=(const long double& a);

  private:
    // Special internal constructors
    Int128(const uint64_t& a, const int64_t& b)
        : lo(a), hi(b) {}

  public:
    // Operators
    bool operator!() const { return !(hi || lo); }

    Int128 operator-() const;
    Int128 operator~() const { return Int128(~lo, ~hi); }

    Int128& operator++();
    Int128& operator--();
    Int128 operator++(int);
    Int128 operator--(int);

    Int128& operator+=(const Int128& b);
    Int128& operator*=(const Int128& b);

    Int128& operator>>=(unsigned int n);
    Int128& operator<<=(unsigned int n);

    Int128& operator|=(const Int128& b) { hi |= b.hi; lo |= b.lo; return *this; }
    Int128& operator&=(const Int128& b) { hi &= b.hi; lo &= b.lo; return *this; }
    Int128& operator^=(const Int128& b) { hi ^= b.hi; lo ^= b.lo; return *this; }

    // Inline simple operators
    inline const Int128& operator+() const { return *this; }

    // Rest of inline operators
    inline Int128& operator-=(const Int128& b)
    {
        return *this += (-b);
    }

    inline Int128& operator/=(const Int128& b)
    {
        Int128 dummy;
        *this = this->div(b, dummy);
        return *this;
    }

    inline Int128& operator%=(const Int128& b)
    {
        this->div(b, *this);
        return *this;
    }

    // Common methods
    int toInt() const { return (int)lo; }
    int64_t toInt64() const { return (int64_t)lo; }
    float toFloat() const;
    double toDouble() const;
    long double toLongDouble() const;

    std::string str(int radix = 10) const;

    // Arithmetic methods
    Int128 div(const Int128&, Int128&) const;

    // Bit operations
    bool bit(unsigned int n) const;
    void bit(unsigned int n, bool val);

    operator double() { return toDouble(); }
    operator int() { return toInt(); }
    static const Int128 INT128_MAX;
    static const Int128 INT128_MIN;
}
#ifdef __GNUC__
__attribute__((__aligned__(16), __packed__))
#endif // ifdef __GNUC__
;

// GLOBAL OPERATORS

bool operator<(const Int128& a, const Int128& b);

inline bool operator==(const Int128& a, const Int128& b)
{
    return a.hi == b.hi && a.lo == b.lo;
}

inline bool operator&&(const Int128& a, const Int128& b)
{
    return (a.hi || a.lo) && (b.hi || b.lo);
}

inline bool operator||(const Int128& a, const Int128& b)
{
    return (a.hi || a.lo) || (b.hi || b.lo);
}

#ifdef __GNUC__
// inline Int128 operator <? (const Int128& a, const Int128& b) {
//     return (a < b) ? a : b; }
// inline Int128 operator >? (const Int128& a, const Int128& b) {
//     return (a < b) ? b : a; }
#endif // ifdef __GNUC__

// GLOBAL OPERATOR INLINES

inline Int128 operator+(const Int128& a, const Int128& b)
{
    return Int128(a) += b;
}

inline Int128 operator-(const Int128& a, const Int128& b)
{
    return Int128(a) -= b;
}

inline Int128 operator*(const Int128& a, const Int128& b)
{
    return Int128(a) *= b;
}

inline Int128 operator/(const Int128& a, const Int128& b)
{
    return Int128(a) /= b;
}

inline Int128 operator%(const Int128& a, const Int128& b)
{
    return Int128(a) %= b;
}

inline Int128 operator>>(const Int128& a, unsigned int n)
{
    return Int128(a) >>= n;
}

inline Int128 operator<<(const Int128& a, unsigned int n)
{
    return Int128(a) <<= n;
}

inline Int128 operator&(const Int128& a, const Int128& b)
{
    return Int128(a) &= b;
}

inline Int128 operator|(const Int128& a, const Int128& b)
{
    return Int128(a) |= b;
}

inline Int128 operator^(const Int128& a, const Int128& b)
{
    return Int128(a) ^= b;
}

inline bool operator>(const Int128& a, const Int128& b)
{
    return b < a;
}

inline bool operator<=(const Int128& a, const Int128& b)
{
    return !(b < a);
}

inline bool operator>=(const Int128& a, const Int128& b)
{
    return !(a < b);
}

inline bool operator!=(const Int128& a, const Int128& b)
{
    return !(a == b);
}

// MISC

// typedef Int128 __int128;

} // namespace inet

#endif

