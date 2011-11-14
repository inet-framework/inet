#ifndef INT128_H
#define INT128_H

/*
  Name: int128.h
  Copyright: Copyright (C) 2005, Jan Ringos
  Author: Jan Ringos, http://Tringi.Mx-3.cz

  Version: 1.1
*/

#include <exception>
#include <cstdlib>
#include <cstdio>
#include <new>
#include <IPv4Address.h>

// CLASS

class int128
{
  private:
    // Binary correct representation of signed 128bit integer
    uint64_t lo;
    int64_t  hi;

  protected:
    // Some global operator functions must be friends
    friend bool operator<(const int128 &, const int128 &);
    friend bool operator==(const int128 &, const int128 &);
    friend bool operator||(const int128 &, const int128 &);
    friend bool operator&&(const int128 &, const int128 &);

#ifdef __GNUC__
    //   friend int128 operator <? (const int128 &, const int128 &);
    //   friend int128 operator >? (const int128 &, const int128 &);
#endif

  public:
    void set(const char *sz);

    // Constructors
    inline int128() {};
    inline int128(const int128 & a) : lo(a.lo), hi(a.hi) {};

    inline int128(const uint32_t & a) : lo(a), hi(0ll) {};
    inline int128(const int32_t & a) : lo(a), hi(0ll)
    {
        if (a < 0) hi = -1ll;
    };

    inline int128(const uint64_t & a) : lo(a), hi(0ll) {};
    inline int128(const int64_t & a) : lo(a), hi(0ll)
    {
        if (a < 0) hi = -1ll;
    };

    int128(const float a);
    int128(const double & a);
    int128(const long double & a);

    int128(const char *sz) { set(sz); }

    // TODO: Consider creation of operator= to eliminate
    //       the need of intermediate objects during assignments.

    int128 & operator=(const int128 &other) {lo = other.lo; hi = other.hi; return *this;}
    int128 & operator=(const int32_t &a) {lo = a; hi = 0; return *this;}
    int128 & operator=(const uint32_t &a) {lo = a; hi = 0; return *this;}

    int128 & operator=(const int64_t &a) {lo = a; hi = 0; return *this;}
    int128 & operator=(const uint64_t &a) {lo = a; hi = 0; return *this;}

    int128 & operator=( const char *sz) {set(sz); return *this;}
    int128 & operator=(const float &a);
    int128 & operator=(const double & a);
    int128 & operator=(const long double & a);

  private:
    // Special internal constructors
    int128(const uint64_t & a, const int64_t & b)
            : lo(a), hi(b) {};

  public:
    // Operators
    bool operator!() const;

    int128 operator-() const;
    int128 operator ~ () const;

    int128 & operator++();
    int128 & operator--();
    int128 operator++(int);
    int128 operator--(int);

    int128 & operator+=(const int128 & b);
    int128 & operator*=(const int128 & b);

    int128 & operator>>=(unsigned int n);
    int128 & operator<<=(unsigned int n);

    int128 & operator|=(const int128 & b);
    int128 & operator&=(const int128 & b);
    int128 & operator^=(const int128 & b);

    // Inline simple operators
    inline const int128 & operator+() const { return *this; };

    // Rest of inline operators
    inline int128 & operator-=(const int128 & b)
    {
        return *this += (-b);
    };
    inline int128 & operator/=(const int128 & b)
    {
        int128 dummy;
        *this = this->div(b, dummy);
        return *this;
    };
    inline int128 & operator%=(const int128 & b)
    {
        this->div(b, *this);
        return *this;
    };

    // Common methods
    int toInt() const {return (int) lo; };
    int64_t toInt64() const {  return (int64_t) lo; };

    const char *toString(uint32_t radix = 10) const;
    float toFloat() const;
    double toDouble() const;
    long double toLongDouble() const;

    // Arithmetic methods
    int128  div(const int128 &, int128 &) const;

    // Bit operations
    bool    bit(unsigned int n) const;
    void    bit(unsigned int n, bool val);


    operator double() { return toDouble(); }
    operator int() { return toInt();}
    operator IPv4Address() { IPv4Address add(toInt()); return add;}
    static const int128 INT128_MAX;
    static const int128 INT128_MIN;
}
#ifdef __GNUC__
__attribute__((__aligned__(16), __packed__))
#endif
;


// GLOBAL OPERATORS

bool operator<(const int128 & a, const int128 & b);
bool operator==(const int128 & a, const int128 & b);
bool operator||(const int128 & a, const int128 & b);
bool operator&&(const int128 & a, const int128 & b);

#ifdef __GNUC__
// inline int128 operator <? (const int128 & a, const int128 & b) {
//     return (a < b) ? a : b; };
// inline int128 operator >? (const int128 & a, const int128 & b) {
//     return (a < b) ? b : a; };
#endif

// GLOBAL OPERATOR INLINES

inline int128 operator+(const int128 & a, const int128 & b)
{
    return int128(a) += b;
};
inline int128 operator-(const int128 & a, const int128 & b)
{
    return int128(a) -= b;
};
inline int128 operator*(const int128 & a, const int128 & b)
{
    return int128(a) *= b;
};
inline int128 operator/(const int128 & a, const int128 & b)
{
    return int128(a) /= b;
};
inline int128 operator%(const int128 & a, const int128 & b)
{
    return int128(a) %= b;
};

inline int128 operator>>(const int128 & a, unsigned int n)
{
    return int128(a) >>= n;
};
inline int128 operator<<(const int128 & a, unsigned int n)
{
    return int128(a) <<= n;
};

inline int128 operator&(const int128 & a, const int128 & b)
{
    return int128(a) &= b;
};
inline int128 operator|(const int128 & a, const int128 & b)
{
    return int128(a) |= b;
};
inline int128 operator^(const int128 & a, const int128 & b)
{
    return int128(a) ^= b;
};

inline bool operator>(const int128 & a, const int128 & b)
{
    return   b < a;
};
inline bool operator<=(const int128 & a, const int128 & b)
{
    return !(b < a);
};
inline bool operator>=(const int128 & a, const int128 & b)
{
    return !(a < b);
};
inline bool operator!=(const int128 & a, const int128 & b)
{
    return !(a == b);
};


// MISC

//typedef int128 __int128;

#endif
