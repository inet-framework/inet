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
#include <IPAddress.h>

// CLASS

class int128
{
  private:
    // Binary correct representation of signed 128bit integer
    uint64_t    lo;
    int64_t    hi;

  protected:
    // Some global operator functions must be friends
    friend bool operator <  (const int128 &, const int128 &) throw ();
    friend bool operator == (const int128 &, const int128 &) throw ();
    friend bool operator || (const int128 &, const int128 &) throw ();
    friend bool operator && (const int128 &, const int128 &) throw ();

#ifdef __GNUC__
    //   friend int128 operator <? (const int128 &, const int128 &) throw ();
    //   friend int128 operator >? (const int128 &, const int128 &) throw ();
#endif

  public:
    // Constructors
    inline int128 () throw () {};
    inline int128 (const int128 & a) throw () : lo (a.lo), hi (a.hi) {};

#ifndef __GNUC__
    inline int128 (const unsigned int & a) throw () : lo (a), hi (0ll) {};
    inline int128 (const signed int & a) throw () : lo (a), hi (0ll)
    {
        if (a < 0) this->hi = -1ll;
    };
#endif
    inline int128 (const uint32_t & a) throw () : lo (a), hi (0ll) {};
    inline int128 (const int32_t & a) throw () : lo (a), hi (0ll)
    {
        if (a < 0) this->hi = -1ll;
    };

    inline int128 (const uint64_t & a) throw () : lo (a), hi (0ll) {};
    inline int128 (const int64_t & a) throw () : lo (a), hi (0ll)
    {
        if (a < 0) this->hi = -1ll;
    };

    int128 (const float a) throw ();
    int128 (const double & a) throw ();
    int128 (const long double & a) throw ();

    int128 (const char * sz) throw ();

    // TODO: Consider creation of operator= to eliminate
    //       the need of intermediate objects during assignments.

    int128 & operator = (const int128 &other) {if (this==&other) return *this; lo = other.lo; hi = other.hi; return *this;}
#ifndef __GNUC__
    int128 & operator = (const int &a) {lo = a; hi = 0; if (a < 0) this->hi = -1ll; return *this;}
    int128 & operator = (const unsigned int &a) {lo = a; hi = 0; return *this;}
#endif
    int128 & operator = (const int32_t &a) {lo = a; hi = 0; return *this;}
    int128 & operator = (const uint32_t &a) {lo = a; hi = 0; return *this;}

    int128 & operator = (const int64_t &a) {lo = a; hi = 0; return *this;}
    int128 & operator = (const uint64_t &a) {lo = a; hi = 0; return *this;}

    int128 & operator = ( const char * sz) throw ();
    int128 & operator= (const float &a) throw ();
    int128 & operator= (const double & a) throw ();
    int128 & operator= (const long double & a) throw ();

  private:
    // Special internal constructors
    int128 (const uint64_t & a, const int64_t & b) throw ()
            : lo (a), hi (b) {};

  public:
    // Operators
    bool operator ! () const throw ();

    int128 operator - () const throw ();
    int128 operator ~ () const throw ();

    int128 & operator ++ ();
    int128 & operator -- ();
    int128 operator ++ (int);
    int128 operator -- (int);

    int128 & operator += (const int128 & b) throw ();
    int128 & operator *= (const int128 & b) throw ();

    int128 & operator >>= (unsigned int n) throw ();
    int128 & operator <<= (unsigned int n) throw ();

    int128 & operator |= (const int128 & b) throw ();
    int128 & operator &= (const int128 & b) throw ();
    int128 & operator ^= (const int128 & b) throw ();

    // Inline simple operators
    inline const int128 & operator + () const throw () { return *this; };

    // Rest of inline operators
    inline int128 & operator -= (const int128 & b) throw ()
    {
        return *this += (-b);
    };
    inline int128 & operator /= (const int128 & b) throw ()
    {
        int128 dummy;
        *this = this->div (b, dummy);
        return *this;
    };
    inline int128 & operator %= (const int128 & b) throw ()
    {
        this->div (b, *this);
        return *this;
    };

    // Common methods
    int toInt () const throw () {return (int) this->lo; };
    int64_t toInt64 () const throw () {  return (int64_t) this->lo; };

    const char * toString (uint32_t radix = 10) const throw ();
    float toFloat () const throw ();
    double toDouble () const throw ();
    long double toLongDouble () const throw ();

    // Arithmetic methods
    int128  div (const int128 &, int128 &) const throw ();

    // Bit operations
    bool    bit (unsigned int n) const throw ();
    void    bit (unsigned int n, bool val) throw ();


    operator double() { return toDouble(); }
    operator int() { return toInt();}
    operator IPAddress() { IPAddress add(toInt()); return add;}
    static const int128 INT128_MAX;
    static const int128 INT128_MIN;
}
#ifdef __GNUC__
__attribute__ ((__aligned__ (16), __packed__))
#endif
;


// GLOBAL OPERATORS

bool operator <  (const int128 & a, const int128 & b) throw ();
bool operator == (const int128 & a, const int128 & b) throw ();
bool operator || (const int128 & a, const int128 & b) throw ();
bool operator && (const int128 & a, const int128 & b) throw ();

#ifdef __GNUC__
// inline int128 operator <? (const int128 & a, const int128 & b) throw () {
//     return (a < b) ? a : b; };
// inline int128 operator >? (const int128 & a, const int128 & b) throw () {
//     return (a < b) ? b : a; };
#endif

// GLOBAL OPERATOR INLINES

inline int128 operator + (const int128 & a, const int128 & b) throw ()
{
    return int128 (a) += b;
};
inline int128 operator - (const int128 & a, const int128 & b) throw ()
{
    return int128 (a) -= b;
};
inline int128 operator * (const int128 & a, const int128 & b) throw ()
{
    return int128 (a) *= b;
};
inline int128 operator / (const int128 & a, const int128 & b) throw ()
{
    return int128 (a) /= b;
};
inline int128 operator % (const int128 & a, const int128 & b) throw ()
{
    return int128 (a) %= b;
};

inline int128 operator >> (const int128 & a, unsigned int n) throw ()
{
    return int128 (a) >>= n;
};
inline int128 operator << (const int128 & a, unsigned int n) throw ()
{
    return int128 (a) <<= n;
};

inline int128 operator & (const int128 & a, const int128 & b) throw ()
{
    return int128 (a) &= b;
};
inline int128 operator | (const int128 & a, const int128 & b) throw ()
{
    return int128 (a) |= b;
};
inline int128 operator ^ (const int128 & a, const int128 & b) throw ()
{
    return int128 (a) ^= b;
};

inline bool operator >  (const int128 & a, const int128 & b) throw ()
{
    return   b < a;
};
inline bool operator <= (const int128 & a, const int128 & b) throw ()
{
    return !(b < a);
};
inline bool operator >= (const int128 & a, const int128 & b) throw ()
{
    return !(a < b);
};
inline bool operator != (const int128 & a, const int128 & b) throw ()
{
    return !(a == b);
};


// MISC

//typedef int128 __int128;

#endif
