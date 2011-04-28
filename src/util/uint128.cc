
#include "uint128.h"

/*
  Name: Uint128.cpp
  Copyright: Copyright (C) 2005, Jan Ringos
  Author: Jan Ringos, http://Tringi.Mx-3.cz

  Version: 1.1
  Alfonso Ariza Quintana 2010, adaptation to inetmanet
*/

#include <memory>
#include <cmath>

// IMPLEMENTATION
const Uint128 Uint128::UINT128_MAX(UINT64_MAX,UINT64_MAX);
const Uint128 Uint128::UINT128_MIN(0,0);

Uint128::Uint128 () throw ()
{
    lo = 0ull;
    hi = 0ull;
}

Uint128::Uint128 (const Uint128 & a) throw ()
{
    lo = a.lo;
    hi = a.hi;
}

Uint128::Uint128  (const int32_t & a) throw ()
{
    lo = a;
    hi =0ull;
}

Uint128::Uint128 (const uint32_t & a) throw ()
{
    lo = a;
    hi =0ull;
}

Uint128::Uint128 (const int64_t & a) throw ()
{
    lo =a;
    hi =0ull;
}

Uint128::Uint128 (const uint64_t & a) throw ()
{
    lo =a ;
    hi =0ull;
}


const char * Uint128::toString (unsigned int radix) const throw ()
{
    if (!*this) return "0";
    if (radix < 2 || radix > 37) return "(invalid radix)";

    static char sz [256];
    memset (sz, 0, 256);

    Uint128 r;
    Uint128 ii = *this;
    Uint128 aux = radix;
    int i = 255;

    while (!!ii && i)
    {
        ii = ii.div (aux, r);
        sz [--i] = r.toUint () + ((r.toUint () > 9) ? 'A' - 10 : '0');
    };

    return &sz [i];
}

Uint128 & Uint128::operator= (const char * sz) throw ()
{

    hi=0;
    lo=0;

    if (!sz) return *this;
    if (!sz [0]) return *this;

    unsigned int radix = 10;
    unsigned int i = 0;
    bool minus = false;

    if (sz [i] == '-')
    {
        ++i;
        minus = true;
    };

    if (sz [i] == '0')
    {
        radix = 8;
        ++i;
        if (sz [i] == 'x')
        {
            radix = 16;
            ++i;
        };
    };

    for (; i < strlen (sz); ++i)
    {
        unsigned int n = 0;
        if ((sz [i] >= '0') && (sz [i] <= '9'))
        {
            if (radix == 8 && (sz [i] >= '9'))
                break;
            n = sz [i] - '0';
        }
        else if ((sz [i] >= 'a') && (sz [i] <= 'a' + (int) radix - 10))
            n = sz [i] - 'a' + 10;
        else if (sz [i] >= 'A' && sz [i] <= 'A' + (int) radix - 10)
            n = sz [i] - 'A' + 10;
        else
            break;
        (*this) *= Uint128(radix);
        (*this) += Uint128(n);
    };

    if (minus)
    {
        *this = Uint128 (0) - *this;
    }
    return *this;
}

Uint128::Uint128 (const char * sz) throw ()
        : lo (0u), hi (0u)
{

    if (!sz) return;
    if (!sz [0]) return;

    unsigned int radix = 10;
    unsigned int i = 0;
    bool minus = false;

    if (sz [i] == '-')
    {
        ++i;
        minus = true;
    };

    if (sz [i] == '0')
    {
        radix = 8;
        ++i;
        if (sz [i] == 'x')
        {
            radix = 16;
            ++i;
        };
    };

    for (; i < strlen (sz); ++i)
    {
        unsigned int n = 0;
        if ((sz [i] >= '0') && (sz [i] <= '9'))
        {
            if (radix == 8 && (sz [i] >= '9'))
                break;
            n = sz [i] - '0';
        }
        else if (sz [i] >= 'a' && sz [i] <= 'a' + (int) radix - 10)
            n = sz [i] - 'a' + 10;
        else if (sz [i] >= 'A' && sz [i] <= 'A' + (int) radix - 10)
            n = sz [i] - 'A' + 10;
        else
            break;

        (*this) *= Uint128(radix);
        (*this) += Uint128(n);
    };

    if (minus)
    {
        *this = Uint128(0) - *this;
    }
    return;
};


Uint128 & Uint128::operator= (const float &a) throw ()
{
    lo = ((uint64_t) fmodf (a, 18446744073709551616.0f));
    hi = ((uint64_t) (a / 18446744073709551616.0f));
    return *this;
}

Uint128 & Uint128::operator= (const double & a) throw ()
{
    lo = ((uint64_t) fmod (a, 18446744073709551616.0));
    hi = ((uint64_t) (a / 18446744073709551616.0));
    return *this;
}

Uint128 & Uint128::operator= (const long double & a) throw ()
{
    lo = ((uint64_t) fmodl (a, 18446744073709551616.0l));
    hi = ((uint64_t) (a / 18446744073709551616.0l));
    return *this;
}


Uint128::Uint128 (const float a) throw ()
        : lo ((uint64_t) fmodf (a, 18446744073709551616.0f)),
        hi ((uint64_t) (a / 18446744073709551616.0f)) {};

Uint128::Uint128 (const double & a) throw ()
        : lo ((uint64_t) fmod (a, 18446744073709551616.0)),
        hi ((uint64_t) (a / 18446744073709551616.0)) {};

Uint128::Uint128 (const long double & a) throw ()
        : lo ((uint64_t) fmodl (a, 18446744073709551616.0l)),
        hi ((uint64_t) (a / 18446744073709551616.0l)) {};

float Uint128::toFloat () const throw ()
{
    return (float) this->hi * 18446744073709551616.0f
           + (float) this->lo;
};

double Uint128::toDouble () const throw ()
{
    return (double) this->hi * 18446744073709551616.0
           + (double) this->lo;
};

long double Uint128::toLongDouble () const throw ()
{
    return (long double) this->hi * 18446744073709551616.0l
           + (long double) this->lo;
};

Uint128 Uint128::operator - () const throw ()
{
    if (!this->hi && !this->lo)
        // number is 0, just return 0
        return *this;
    else
        // non 0 number
        return Uint128 (-this->lo, ~this->hi);
};

Uint128 Uint128::operator ~ () const throw ()
{
    return Uint128 (~this->lo, ~this->hi);
};

Uint128 & Uint128::operator ++ ()
{
    ++this->lo;
    if (!this->lo)
        ++this->hi;

    return *this;
};

Uint128 & Uint128::operator -- ()
{
    if (!this->lo)
        --this->hi;
    --this->lo;

    return *this;
};

Uint128 Uint128::operator ++ (int)
{
    Uint128 b = *this;
    ++ *this;

    return b;
};

Uint128 Uint128::operator -- (int)
{
    Uint128 b = *this;
    -- *this;

    return b;
};

Uint128 & Uint128::operator += (const Uint128 & b) throw ()
{
    uint64_t old_lo = this->lo;

    this->lo += b.lo;
    this->hi += b.hi + (this->lo < old_lo);

    return *this;
};

Uint128 & Uint128::operator *= (const Uint128 & b) throw ()
{
    if (!b)
        return *this = 0u;
    if (b == 1u)
        return *this;

    Uint128 a = *this;
    Uint128 t = b;

    this->lo = 0ull;
    this->hi = 0ull;

    for (unsigned int i = 0; i < 128; ++i)
    {
        if (t.lo & 1)
            *this += a << i;

        t >>= 1;
    };

    return *this;
};


Uint128 Uint128::div (const Uint128 & ds, Uint128 & remainder) const throw ()
{
    if (!ds)
        return 1u / (unsigned int) ds.lo;

    Uint128 dd = *this;

    // only remainder
    if (ds > dd)
    {
        remainder = *this;
        return (Uint128)0;
    };

    Uint128 r = (Uint128) 0;
    Uint128 q = (Uint128) 0;
//    while (dd >= ds) { dd -= ds; q += 1; }; // extreme slow version

    uint32_t b = 127;
    while (r < ds)
    {
        r <<= 1;
        if (dd.bit (b--))
            r.lo |= 1;
    };
    ++b;

    while (true)
        if (r < ds)
        {
            if (!(b--)) break;

            r <<= 1;
            if (dd.bit (b))
                r.lo |= 1;

        }
        else
        {
            r -= ds;
            q.bit (b, true);
        };

    remainder = r;
    return q;
};

bool Uint128::bit (unsigned int n) const throw ()
{
    n &= 0x7F;

    if (n < 64)
        return this->lo & (1ull << n);
    else
        return this->hi & (1ull << (n - 64));
};

void Uint128::bit (unsigned int n, bool val) throw ()
{
    n &= 0x7F;

    if (val)
    {
        if (n < 64) this->lo |= (1ull << n);
        else this->hi |= (1ull << (n - 64));
    }
    else
    {
        if (n < 64) this->lo &= ~(1ull << n);
        else this->hi &= ~(1ull << (n - 64));
    };
};


Uint128 & Uint128::operator >>= (unsigned int n) throw ()
{
    n &= 0x7F;

    if (n > 63)
    {
        n -= 64;
        this->lo = this->hi;
        this->hi = 0ull;
    };

    if (n)
    {
        // shift low qword
        this->lo >>= n;

        // get lower N bits of high qword
        uint64_t mask = 0ull;
        for (unsigned int i = 0; i < n; ++i) mask |= (1 << i);

        // and add them to low qword
        this->lo |= (this->hi & mask) << (64 - n);

        // and finally shift also high qword
        this->hi >>= n;
    };

    return *this;
};

Uint128 & Uint128::operator <<= (unsigned int n) throw ()
{
    n &= 0x7F;

    if (n > 63)
    {
        n -= 64;
        this->hi = this->lo;
        this->lo = 0ull;
    };

    if (n)
    {
        // shift high qword
        this->hi <<= n;

        // get higher N bits of low qword
        uint64_t mask = 0ull;
        for (unsigned int i = 0; i < n; ++i) mask |= (1 << (63 - i));

        // and add them to high qword
        this->hi |= (this->lo & mask) >> (64 - n);

        // and finally shift also low qword
        this->lo <<= n;
    };

    return *this;
};

bool Uint128::operator ! () const throw ()
{
    return !(this->hi || this->lo);
};

Uint128 & Uint128::operator |= (const Uint128 & b) throw ()
{
    this->hi |= b.hi;
    this->lo |= b.lo;

    return *this;
};

Uint128 & Uint128::operator &= (const Uint128 & b) throw ()
{
    this->hi &= b.hi;
    this->lo &= b.lo;

    return *this;
};

Uint128 & Uint128::operator ^= (const Uint128 & b) throw ()
{
    this->hi ^= b.hi;
    this->lo ^= b.lo;

    return *this;
};

bool operator <  (const Uint128 & a, const Uint128 & b) throw ()
{
    return (a.hi == b.hi) ? (a.lo < b.lo) : (a.hi < b.hi);
};

bool operator == (const Uint128 & a, const uint32_t & b) throw ()
{
    if (a.hi != 0) return false;
    uint64_t aux = b;
    return  a.lo == aux;
};

bool operator == (const uint32_t & b, const Uint128 & a) throw ()
{
    if (a.hi != 0) return false;
    uint64_t aux = b;
    return  a.lo == aux;
};

bool operator == (const Uint128 & a, const uint64_t & b) throw ()
{
    if (a.hi != 0) return false;
    return  a.lo == b;
};

bool operator == (const uint64_t & b, const Uint128 & a) throw ()
{
    if (a.hi != 0) return false;
    return  a.lo == b;
};


bool operator == (const Uint128 & a, const int32_t & b) throw ()
{
    if (a.hi != 0) return false;
    int64_t aux = b;
    return  a.lo == (uint64_t)aux;
};

bool operator == (const int32_t & b, const Uint128 & a) throw ()
{
    if (a.hi != 0) return false;
    int64_t aux = b;
    if (b < 0) return false;
    return  a.lo == (uint64_t) aux;
};

bool operator == (const Uint128 & a, const int64_t & b) throw ()
{
    if (a.hi != 0) return false;
    if (b < 0) return false;
    return  a.lo == (uint64_t)b;
};

bool operator == (const int64_t & b, const Uint128 & a) throw ()
{
    if (a.hi != 0) return false;
    if (b < 0) return false;
    return  a.lo == (uint64_t) b;
};

bool operator == (const Uint128 & a, const Uint128 & b) throw ()
{
    return a.hi == b.hi && a.lo == b.lo;
};


bool operator && (const Uint128 & a, const Uint128 & b) throw ()
{
    return (a.hi || a.lo) && (b.hi || b.lo);
};

bool operator || (const Uint128 & a, const Uint128 & b) throw ()
{
    return (a.hi || a.lo) || (b.hi || b.lo);
};


Uint128 & Uint128::operator= (const IPAddress &add) throw ()
{
    hi=0;
    lo=add.getInt();
    return *this;
}

Uint128 & Uint128::operator= (const MACAddress &add) throw ()
{
    hi=0;
    lo=0;
    uint64_t aux;
    for (int i=0; i<MAC_ADDRESS_BYTES; i++)
    {
        aux  = add.getAddressByte(MAC_ADDRESS_BYTES-i-1);
        aux <<= 8*i;
        lo  |= aux ;
    }
    return *this;
}

Uint128 & Uint128::operator= (const IPv6Address &add) throw ()
{
    uint32 *w = const_cast<IPv6Address&>(add).words();
    uint64_t aux[4];
    aux[0]=w[0];
    aux[1]=w[1];
    aux[2]=w[2];
    aux[3]=w[3];

    lo = aux[0]&(aux[1]<<32);
    hi = aux[2]&(aux[3]<<32);
    return *this;
}


bool operator != (const Uint128 & a, const uint32_t & b) throw ()
{
    return  !(a == b);
}

bool operator != (const uint32_t & b, const Uint128 & a) throw ()
{
    return  !(a == b);
}

bool operator != (const Uint128 & a, const uint64_t & b) throw ()
{
    return  !(a == b);
}

bool operator != (const uint64_t & b, const Uint128 & a) throw ()
{
    return  !(a == b);
}


bool operator != (const Uint128 & a, const int32_t & b) throw ()
{
    return  !(a == b);
}

bool operator != (const int32_t & b, const Uint128 & a) throw ()
{
    return  !(a == b);
}

bool operator != (const Uint128 & a, const int64_t & b) throw ()
{
    return  !(a == b);
}

bool operator != (const int64_t & b, const Uint128 & a) throw ()
{
    return  !(a == b);
}

