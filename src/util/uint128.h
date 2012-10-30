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

#ifndef __INET_UINT128_H
#define __INET_UINT128_H

/*
  Name: int128.h
  Author: Jan Ringos, http://Tringi.Mx-3.cz
  Source: http://mx-3.cz/tringi/www/int128
  Version: 1.1

  Modifications: Alfonso Ariza Quintana, Zoltan Bojthe et al.
*/


#include "INETDefs.h"


// CLASS

class Uint128
{
    // Binary correct representation of signed 128bit integer
  private:
    uint64_t lo;
    uint64_t hi;

    //inline Uint128 (const unsigned __int64 & a, const unsigned __int64 & b)
//            : lo (a), hi (b) {}
    inline Uint128(const uint64_t& a, const uint64_t& b)  {lo = a; hi = b;}
  protected:
    // Some global operator functions must be friends
    friend bool operator<(const Uint128&, const Uint128&);
    //friend bool operator >  (const Uint128&, const Uint128&);
    friend bool operator==(const Uint128&, const uint32_t&);
    friend bool operator==(const Uint128&, const int32_t&);
    friend bool operator==(const Uint128&, const uint64_t&);
    friend bool operator==(const Uint128&, const int64_t&);

    friend bool operator==(const uint32_t&, const Uint128&);
    friend bool operator==(const int32_t&, const Uint128&);
    friend bool operator==(const uint64_t&, const Uint128&);
    friend bool operator==(const int64_t&, const Uint128&);

    friend bool operator==(const Uint128&, const Uint128&);
    friend bool operator||(const Uint128&, const Uint128&);
    friend bool operator&&(const Uint128&, const Uint128&);

    friend bool operator!=(const Uint128&, const uint32_t&);
    friend bool operator!=(const Uint128&, const int32_t&);
    friend bool operator!=(const Uint128&, const uint64_t&);
    friend bool operator!=(const Uint128&, const int64_t&);

    friend bool operator!=(const uint32_t&, const Uint128&);
    friend bool operator!=(const int32_t&, const Uint128&);
    friend bool operator!=(const uint64_t&, const Uint128&);
    friend bool operator!=(const int64_t&, const Uint128&);
    friend std::ostream& operator<<(std::ostream& os, const Uint128& );

#ifdef __GNUC__
//            friend Uint128 operator <? (const Uint128&, const Uint128&);
//            friend Uint128 operator >? (const Uint128&, const Uint128&);
#endif
  public:
    void set( const char *sz);

    // Constructors
    Uint128() : lo(0ull), hi(0ull) {}

    Uint128(const Uint128& a) : lo(a.lo), hi(a.hi) {}

    // Note: int / unsigned int operators conflict with other integer types with at least GCC and MSVC
    // inline Uint128 (const unsigned int & a) : lo (a), hi (0ull) {}
    // inline Uint128 (const int & a) : lo (a), hi (0ull) {}

    //   inline Uint128 (const unsigned __int64 & a) : lo (a), hi (0ull) {}
    Uint128(const int32_t& a) : lo(a), hi(0) {}
    Uint128(const uint32_t& a) : lo(a), hi(0) {}
    Uint128(const int64_t& a) : lo(a), hi(0) {}
    Uint128(const uint64_t& a) : lo(a), hi(0) {}

    Uint128(const float a);
    Uint128(const double& a);
    Uint128(const long double& a);

    explicit Uint128(const char *sz) {set(sz);}

    // TODO: Consider creation of operator= to eliminate
    //       the need of intermediate objects during assignments.
    Uint128& operator=(const Uint128& other) {lo = other.lo; hi = other.hi; return *this;}

    // Note: int / unsigned int operators conflict with other integer types with at least GCC and MSVC
    // Uint128& operator = (const int &a) {lo = a; hi = 0; return *this;}
    // Uint128& operator = (const unsigned int &a) {lo = a; hi = 0; return *this;}

    Uint128& operator=(const int32_t& a) {lo = a; hi = 0; return *this;}
    Uint128& operator=(const uint32_t& a) {lo = a; hi = 0; return *this;}
    Uint128& operator=(const int64_t& a) {lo = a; hi = 0; return *this;}
    Uint128& operator=(const uint64_t& a) {lo = a; hi = 0; return *this;}

    Uint128& operator=( const char *sz) {set(sz); return *this;}
    Uint128& operator=(const float& a);
    Uint128& operator=(const double& a);
    Uint128& operator=(const long double& a);

    // Operators
    bool operator!() const { return !(hi || lo); }
    Uint128 operator-() const;
    Uint128 operator ~ () const;
    Uint128& operator++();
    Uint128& operator--();
    Uint128 operator++(int);
    Uint128 operator--(int);

    Uint128& operator+=(const Uint128& b);
    Uint128& operator*=(const Uint128& b);

    Uint128& operator>>=(unsigned int n);
    Uint128& operator<<=(unsigned int n);
    Uint128& operator|=(const Uint128& b) { hi |= b.hi; lo |= b.lo; return *this; }


    Uint128& operator&=(const Uint128& b) { hi &= b.hi; lo &= b.lo; return *this; }
    Uint128& operator^=(const Uint128& b) { hi ^= b.hi; lo ^= b.lo; return *this; }

    // Inline simple operators
    inline const Uint128& operator+() const { return *this; }

    // Rest of inline operators
    inline Uint128& operator-=(const Uint128& b)
    {
        return *this += (-b);
    }
    inline Uint128& operator/=(const Uint128& b)
    {
        Uint128 dummy;
        *this = this->div(b, dummy);
        return *this;
    }
    inline Uint128& operator%=(const Uint128& b)
    {
        this->div(b, *this);
        return *this;
    }

    // Common methods
    unsigned int toUint() const {return (unsigned int) lo;}
    uint64_t toUint64() const {return lo;}

    const char *toString(unsigned int radix = 10) const;
    float toFloat() const;
    double toDouble() const;
    long double toLongDouble() const;

    // Arithmetic methods
    Uint128  div(const Uint128&, Uint128&) const;

    // Bit operations
    bool    bit(unsigned int n) const;
    void    bit(unsigned int n, bool val);
    uint64_t getLo() const {return lo;}
    uint64_t getHi() const {return hi;}
    static const Uint128 UINT128_MAX;
    static const Uint128 UINT128_MIN;
}
#ifdef __GNUC__
__attribute__((__aligned__(16), __packed__))
#endif
;


// GLOBAL OPERATORS

bool operator<(const Uint128& a, const Uint128& b);

inline bool operator==(const Uint128& a, const Uint128& b)
{
    return a.hi == b.hi && a.lo == b.lo;
}


inline bool operator||(const Uint128& a, const Uint128& b)
{
    return (a.hi || a.lo) || (b.hi || b.lo);
}

inline bool operator&&(const Uint128& a, const Uint128& b)
{
    return (a.hi || a.lo) && (b.hi || b.lo);
}


#ifdef __GNUC__
//    inline Uint128 operator <? (const Uint128& a, const Uint128& b) {
//        return (a < b) ? a : b; }
//    inline Uint128 operator >? (const Uint128& a, const Uint128& b) {
//        return (a < b) ? b : a; }
#endif

// GLOBAL OPERATOR INLINES

inline bool operator<(const Uint128& a, const Uint128& b)
{
    return (a.hi < b.hi) || ((a.hi == b.hi) && (a.lo < b.lo));
}

inline bool operator==(const Uint128& a, const uint32_t& b)
{
    return (a.hi == 0) && (a.lo == (uint64_t)b);
}

inline bool operator==(const uint32_t& b, const Uint128& a)
{
    return (a.hi == 0) && (a.lo == (uint64_t)b);
}

inline bool operator==(const Uint128& a, const uint64_t& b)
{
    return (a.hi == 0) && (a.lo == b);
}

inline bool operator==(const uint64_t& b, const Uint128& a)
{
    return (a.hi == 0) && (a.lo == b);
}

inline bool operator==(const Uint128& a, const int32_t& b)
{
    return (b >= 0) && (a.hi == 0) && (a.lo == (uint64_t)b);
}

inline bool operator==(const int32_t& b, const Uint128& a)
{
    return (b >= 0) && (a.hi == 0) && (a.lo == (uint64_t)b);
}

inline bool operator==(const Uint128& a, const int64_t& b)
{
    return (b >= 0) && (a.hi == 0) && (a.lo == (uint64_t)b);
}

inline bool operator==(const int64_t& b, const Uint128& a)
{
    return (b >= 0) && (a.hi == 0) && (a.lo == (uint64_t)b);
}

inline Uint128 operator+(const Uint128& a, const Uint128& b)
{
    return Uint128(a) += b;
}
inline Uint128 operator-(const Uint128& a, const Uint128& b)
{
    return Uint128(a) -= b;
}
inline Uint128 operator*(const Uint128& a, const Uint128& b)
{
    return Uint128(a) *= b;
}
inline Uint128 operator/(const Uint128& a, const Uint128& b)
{
    return Uint128(a) /= b;
}
inline Uint128 operator%(const Uint128& a, const Uint128& b)
{
    return Uint128(a) %= b;
}

inline Uint128 operator>>(const Uint128& a, unsigned int n)
{
    return Uint128(a) >>= n;
}
inline Uint128 operator<<(const Uint128& a, unsigned int n)
{
    return Uint128(a) <<= n;
}

inline Uint128 operator&(const Uint128& a, const Uint128& b)
{
    return Uint128(a) &= b;
}
inline Uint128 operator|(const Uint128& a, const Uint128& b)
{
    return Uint128(a) |= b;
}
inline Uint128 operator^(const Uint128& a, const Uint128& b)
{
    return Uint128(a) ^= b;
}

inline bool operator>(const Uint128& a, const Uint128& b)
{
    return   b < a;
}
inline bool operator<=(const Uint128& a, const Uint128& b)
{
    return !(b < a);
}
inline bool operator>=(const Uint128& a, const Uint128& b)
{
    return !(a < b);
}

inline bool operator!=(const Uint128& a, const Uint128& b)
{
    return !(a == b);
}

inline std::ostream& operator<<(std::ostream& os, const Uint128& val)
{
    return os << val.toString();
}

// MISC

//typedef Uint128 __Uint128;

#endif
