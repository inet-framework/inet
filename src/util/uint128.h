#ifndef Uint128_H
#define Uint128_H

/*
  Name: Uint128.hpp
  Copyright: Copyright (C) 2005, Jan Ringos
  Author: Jan Ringos, http://Tringi.Mx-3.cz

  Version: 1.1
  Modify Alfonso Ariza Quintana, 2008
*/


#include <exception>
#include <cstdlib>
#include <cstdio>
#include <new>
#include <IPv6Address.h>
#include <IPv4Address.h>
#include <MACAddress.h>


// CLASS

class Uint128
{
    // Binary correct representation of signed 128bit integer
  private:
    uint64_t lo;
    uint64_t hi;

    //inline Uint128 (const unsigned __int64 & a, const unsigned __int64 & b) throw ()
//            : lo (a), hi (b) {};
    inline Uint128 (const uint64_t & a, const uint64_t & b) throw ()  {lo = a; hi = b;}
  protected:
    // Some global operator functions must be friends
    friend bool operator <  (const Uint128 &, const Uint128 &) throw ();
    //friend bool operator >  (const Uint128 &, const Uint128 &) throw ();
    friend bool operator == (const Uint128 &, const uint32_t &) throw ();
    friend bool operator == (const Uint128 &, const int32_t &) throw ();
    friend bool operator == (const Uint128 &, const uint64_t &) throw ();
    friend bool operator == (const Uint128 &, const int64_t &) throw ();

    friend bool operator == (const uint32_t &, const Uint128 &) throw ();
    friend bool operator == (const int32_t &, const Uint128 &) throw ();
    friend bool operator == (const uint64_t &, const Uint128 &) throw ();
    friend bool operator == (const int64_t &, const Uint128 &) throw ();

    friend bool operator == (const Uint128 &, const Uint128 &) throw ();
    friend bool operator || (const Uint128 &, const Uint128 &) throw ();
    friend bool operator && (const Uint128 &, const Uint128 &) throw ();

    friend bool operator != (const Uint128 &, const uint32_t &) throw ();
    friend bool operator != (const Uint128 &, const int32_t &) throw ();
    friend bool operator != (const Uint128 &, const uint64_t &) throw ();
    friend bool operator != (const Uint128 &, const int64_t &) throw ();

    friend bool operator != (const uint32_t &, const Uint128 &) throw ();
    friend bool operator != (const int32_t &, const Uint128 &) throw ();
    friend bool operator != (const uint64_t &, const Uint128 &) throw ();
    friend bool operator != (const int64_t &, const Uint128 &) throw ();
    friend std::ostream& operator<<(std::ostream& os, const Uint128& );

#ifdef __GNUC__
//            friend Uint128 operator <? (const Uint128 &, const Uint128 &) throw ();
//            friend Uint128 operator >? (const Uint128 &, const Uint128 &) throw ();
#endif
  public:
    // Constructors
    Uint128 () throw ();
    Uint128 (const Uint128 & a) throw ();

    // Note: int / unsigned int operators conflict with other integer types with at least GCC and MSVC
    // inline Uint128 (const unsigned int & a) throw () : lo (a), hi (0ull) {};
    // inline Uint128 (const int & a) throw () : lo (a), hi (0ull) {};

    //   inline Uint128 (const unsigned __int64 & a) throw () : lo (a), hi (0ull) {};
    Uint128 (const int32_t & a) throw ();
    Uint128 (const uint32_t & a) throw ();
    Uint128 (const int64_t & a) throw ();
    Uint128 (const uint64_t & a) throw ();

    Uint128 (const float a) throw ();
    Uint128 (const double & a) throw ();
    Uint128 (const long double & a) throw ();

    Uint128 (const char * sz) throw ();

    // TODO: Consider creation of operator= to eliminate
    //       the need of intermediate objects during assignments.
    Uint128 & operator = (const Uint128 &other) {if (this==&other) return *this; lo = other.lo; hi = other.hi; return *this;}

    // Note: int / unsigned int operators conflict with other integer types with at least GCC and MSVC
    // Uint128 & operator = (const int &a) {lo = a; hi = 0; return *this;}
    // Uint128 & operator = (const unsigned int &a) {lo = a; hi = 0; return *this;}

    Uint128 & operator = (const int32_t &a) {lo = a; hi = 0; return *this;}
    Uint128 & operator = (const uint32_t &a) {lo = a; hi = 0; return *this;}
    Uint128 & operator = (const int64_t &a) {lo = a; hi = 0; return *this;}
    Uint128 & operator = (const uint64_t &a) {lo = a; hi = 0; return *this;}

    Uint128 & operator = ( const char * sz) throw ();
    Uint128 & operator= (const float &a) throw ();
    Uint128 & operator= (const double & a) throw ();
    Uint128 & operator= (const long double & a) throw ();

    Uint128 & operator= (const IPv4Address &) throw ();
    Uint128 & operator= (const MACAddress &) throw ();
    Uint128 & operator= (const IPv6Address &) throw ();

    inline Uint128 (const IPv4Address & a) throw ()  {*this=a;}
    inline Uint128 (const MACAddress & a) throw ()  {*this=a;}
    inline Uint128 (const IPv6Address & a) throw ()  {*this=a;}


    // Operators
    bool operator ! () const throw ();
    Uint128 operator - () const throw ();
    Uint128 operator ~ () const throw ();
    Uint128 & operator ++ ();
    Uint128 & operator -- ();
    Uint128 operator ++ (int);
    Uint128 operator -- (int);

    Uint128 & operator += (const Uint128 & b) throw ();
    Uint128 & operator *= (const Uint128 & b) throw ();

    Uint128 & operator >>= (unsigned int n) throw ();
    Uint128 & operator <<= (unsigned int n) throw ();
    Uint128 & operator |= (const Uint128 & b) throw ();

    Uint128 & operator &= (const Uint128 & b) throw ();
    Uint128 & operator ^= (const Uint128 & b) throw ();

    // Inline simple operators
    inline const Uint128 & operator + () const throw () { return *this; };

    // Rest of inline operators
    inline Uint128 & operator -= (const Uint128 & b) throw ()
    {
        return *this += (-b);
    };
    inline Uint128 & operator /= (const Uint128 & b) throw ()
    {
        Uint128 dummy;
        *this = this->div (b, dummy);
        return *this;
    };
    inline Uint128 & operator %= (const Uint128 & b) throw ()
    {
        this->div (b, *this);
        return *this;
    };
    // Common methods
    unsigned int toUint () const throw () {return (unsigned int) this->lo;};
    uint64_t toUint64 () const throw () {return this->lo;};
    const char * toString (unsigned int radix = 10) const throw ();
    float toFloat () const throw ();
    double toDouble () const throw ();
    long double toLongDouble () const throw ();
    operator double() const {return toDouble(); }
    operator int() const {return toUint();}
    operator uint32_t() const { return toUint();}
    operator uint64_t() const {return toUint64();}
    operator int64_t() const {return toUint64();}

    operator bool()
    {
        if (lo==0 && hi == 0)
            return false;
        return true;
    }

    operator IPv4Address() const {IPv4Address add(toUint()); return add;}
    operator MACAddress() const
    {
        MACAddress add;
        add.setAddressByte(0, (lo>>40)&0xff);
        add.setAddressByte(1, (lo>>32)&0xff);
        add.setAddressByte(2, (lo>>24)&0xff);
        add.setAddressByte(3, (lo>>16)&0xff);
        add.setAddressByte(4, (lo>>8)&0xff);
        add.setAddressByte(5, lo&0xff);
        return add;
    }

    operator IPv6Address() const
    {
        uint32_t d[4];
        d[0]=lo & 0xffffffff;
        d[1]=(lo>>32) & 0xffffffff;
        d[2]=hi & 0xffffffff;
        d[3]=(hi>>32) & 0xffffffff;
        IPv6Address add(d[0],d[1],d[2],d[3]);
        return add;
    }

    IPv4Address  getIPAddress () const {IPv4Address add(toUint()); return add;}
    MACAddress getMACAddress() const
    {
        MACAddress add;
        add.setAddressByte(0, (lo>>40)&0xff);
        add.setAddressByte(1, (lo>>32)&0xff);
        add.setAddressByte(2, (lo>>24)&0xff);
        add.setAddressByte(3, (lo>>16)&0xff);
        add.setAddressByte(4, (lo>>8)&0xff);
        add.setAddressByte(5, lo&0xff);
        return add;
    }

    IPv6Address getIPv6Address () const
    {
        uint32_t d[4];
        d[0]=lo & 0xffffffff;
        d[1]=(lo>>32) & 0xffffffff;
        d[2]=hi & 0xffffffff;
        d[3]=(hi>>32) & 0xffffffff;
        IPv6Address add(d[0],d[1],d[2],d[3]);
        return add;
    }

    // Arithmetic methods
    Uint128  div (const Uint128 &, Uint128 &) const throw ();

    // Bit operations
    bool    bit (unsigned int n) const throw ();
    void    bit (unsigned int n, bool val) throw ();
    uint64_t getLo() const {return lo;}
    uint64_t getHi() const {return hi;}
    static const Uint128 UINT128_MAX;
    static const Uint128 UINT128_MIN;
}
#ifdef __GNUC__
__attribute__ ((__aligned__ (16), __packed__))
#endif
;


// GLOBAL OPERATORS

bool operator <  (const Uint128 & a, const Uint128 & b) throw ();
bool operator == (const Uint128 & a, const Uint128 & b) throw ();
bool operator || (const Uint128 & a, const Uint128 & b) throw ();
bool operator && (const Uint128 & a, const Uint128 & b) throw ();

#ifdef __GNUC__
//    inline Uint128 operator <? (const Uint128 & a, const Uint128 & b) throw () {
//        return (a < b) ? a : b; };
//    inline Uint128 operator >? (const Uint128 & a, const Uint128 & b) throw () {
//        return (a < b) ? b : a; };
#endif

// GLOBAL OPERATOR INLINES

inline Uint128 operator + (const Uint128 & a, const Uint128 & b) throw ()
{
    return Uint128 (a) += b;
};
inline Uint128 operator - (const Uint128 & a, const Uint128 & b) throw ()
{
    return Uint128 (a) -= b;
};
inline Uint128 operator * (const Uint128 & a, const Uint128 & b) throw ()
{
    return Uint128 (a) *= b;
};
inline Uint128 operator / (const Uint128 & a, const Uint128 & b) throw ()
{
    return Uint128 (a) /= b;
};
inline Uint128 operator % (const Uint128 & a, const Uint128 & b) throw ()
{
    return Uint128 (a) %= b;
};

inline Uint128 operator >> (const Uint128 & a, unsigned int n) throw ()
{
    return Uint128 (a) >>= n;
};
inline Uint128 operator << (const Uint128 & a, unsigned int n) throw ()
{
    return Uint128 (a) <<= n;
};

inline Uint128 operator & (const Uint128 & a, const Uint128 & b) throw ()
{
    return Uint128 (a) &= b;
};
inline Uint128 operator | (const Uint128 & a, const Uint128 & b) throw ()
{
    return Uint128 (a) |= b;
};
inline Uint128 operator ^ (const Uint128 & a, const Uint128 & b) throw ()
{
    return Uint128 (a) ^= b;
};

inline bool operator >  (const Uint128 & a, const Uint128 & b) throw ()
{
    return   b < a;
};
inline bool operator <= (const Uint128 & a, const Uint128 & b) throw ()
{
    return !(b < a);
};
inline bool operator >= (const Uint128 & a, const Uint128 & b) throw ()
{
    return !(a < b);
};

inline bool operator != (const Uint128 & a, const Uint128 & b) throw ()
{
    return !(a == b);
};

inline std::ostream& operator<<(std::ostream& os, const Uint128& val)
{
    return os << val.toString();
}

// MISC

//typedef Uint128 __Uint128;

#endif
