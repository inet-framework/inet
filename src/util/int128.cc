#include "int128.h"

/*
  Name: int128.cpp
  Copyright: Copyright (C) 2005, Jan Ringos
  Author: Jan Ringos, http://Tringi.Mx-3.cz

  Version: 1.1
*/

#include <memory>
#include <cmath>

// IMPLEMENTATION
const int128 int128::INT128_MAX(UINT64_MAX, INT64_MAX);
const int128 int128::INT128_MIN(0, INT64_MIN);


const char *int128::toString(uint32_t radix) const
{
    if (!*this) return "0";
    if (radix < 2 || radix > 37) return "(invalid radix)";

    static char sz[256];
    memset(sz, 0, 256);

    int128 r;
    int128 ii = (*this < 0) ? -*this : *this;
    int i = 255;
    int128 aux = radix;

    while (!!ii && i)
    {
        ii = ii.div(aux, r);
        unsigned int c = r.toInt();
        sz[--i] = c + ((c > 9) ? 'A' - 10 : '0');
    };

    if (*this < 0)
        sz[--i] = '-';

    return &sz[i];
};

void int128::set(const char *sz)
{
    lo = 0u;
    hi = 0;

    if (!sz) return;
    if (!sz[0]) return;

    uint32_t radix = 10;
    uint32_t i = 0;
    bool minus = false;

    if (sz[i] == '-')
    {
        ++i;
        minus = true;
    }

    if (sz[i] == '0')
    {
        radix = 8;
        ++i;
        if (sz[i] == 'x')
        {
            radix = 16;
            ++i;
        }
    }

    for (; i < strlen(sz); ++i)
    {
        uint32_t n = 0;
        if (sz[i] >= '0' && sz[i] <= '9' && sz[i] < '0' + (int) radix)
            n = sz[i] - '0';
        else if (sz[i] >= 'a' && sz[i] <= 'a' + (int) radix - 10)
            n = sz[i] - 'a' + 10;
        else if (sz[i] >= 'A' && sz[i] <= 'A' + (int) radix - 10)
            n = sz[i] - 'A' + 10;
        else
            break;

        (*this) *= radix;
        (*this) += n;
    }

    if (minus)
        *this = int128(0) - *this;

    return;
};

int128::int128(const float a)
        : lo((uint64_t) fmodf(a, 18446744073709551616.0f)),
        hi((int64_t) (a / 18446744073709551616.0f)) {};

int128::int128(const double & a)
        : lo((uint64_t) fmod(a, 18446744073709551616.0)),
        hi((int64_t) (a / 18446744073709551616.0)) {};

int128::int128(const long double & a)
        : lo((uint64_t) fmodl(a, 18446744073709551616.0l)),
        hi((int64_t) (a / 18446744073709551616.0l)) {};

float int128::toFloat() const
{
    return (float) hi * 18446744073709551616.0f
           + (float) lo;
};

double int128::toDouble() const
{
    return (double) hi * 18446744073709551616.0
           + (double) lo;
};


int128 & int128::operator=(const float &a)
{
    lo = ((uint64_t) fmodf(a, 18446744073709551616.0f));
    hi = ((int64_t) (a / 18446744073709551616.0f));
    return *this;
}

int128 & int128::operator=(const double & a)
{
    lo = ((uint64_t) fmod(a, 18446744073709551616.0));
    hi = ((int64_t) (a / 18446744073709551616.0));
    return *this;
}

int128 & int128::operator=(const long double & a)
{
    lo = ((uint64_t) fmodl(a, 18446744073709551616.0l));
    hi = ((int64_t) (a / 18446744073709551616.0l));
    return *this;
}


long double int128::toLongDouble() const
{
    return (long double) hi * 18446744073709551616.0l
           + (long double) lo;
};

int128 int128::operator-() const
{
    if (lo == 0)
        return int128(0ull, -hi);
    else
        return int128(-lo, ~hi);
};

int128 & int128::operator++()
{
    ++lo;
    if (!lo)
        ++hi;

    return *this;
};

int128 & int128::operator--()
{
    if (!lo)
        --hi;
    --lo;

    return *this;
};

int128 int128::operator++(int)
{
    int128 b(*this);
    ++ *this;

    return b;
};

int128 int128::operator--(int)
{
    int128 b(*this);
    -- *this;

    return b;
};

int128 & int128::operator+=(const int128 & b)
{
    uint64_t old_lo = lo;

    lo += b.lo;
    hi += b.hi;
    if (lo < old_lo)
        ++hi;

    return *this;
};

int128 & int128::operator*=(const int128 & b)
{
    if (!b)
        return *this = 0u;
    if (b == 1u)
        return *this;

    int128 a(*this);
    int128 t(b);

    lo = 0ull;
    hi = 0ll;

    for (unsigned int i = 0; i < 128; ++i)
    {
        if (t.lo & 1)
            *this += a << i;

        t >>= 1;
    };

    return *this;
};


int128 int128::div(const int128 & divisor, int128 & remainder) const
{
    if (!divisor)
        return 1u / (unsigned int) divisor.lo;
    // or RaiseException (EXCEPTION_INT_DIVIDE_BY_ZERO,
    //                    EXCEPTION_NONCONTINUABLE, 0, NULL);

    int128 ds = (divisor < 0) ? -divisor : divisor;
    int128 dd = (*this < 0) ? -*this : *this;

    // only remainder
    if (ds > dd)
    {
        remainder = *this;
        return (int128)0;
    };

    int128 r = (int128)0;
    int128 q = (int128)0;
//    while (dd >= ds) { dd -= ds; q += 1; }; // extreme slow version

    unsigned int b = 127;
    while (r < ds)
    {
        r <<= 1;
        if (dd.bit(b--))
            r.lo |= 1;
    };
    ++b;

    while (true)
        if (r < ds)
        {
            if (!(b--)) break;

            r <<= 1;
            if (dd.bit(b))
                r.lo |= 1;

        }
        else
        {
            r -= ds;
            q.bit(b, true);
        };

    // correct
    if ((divisor < 0) ^ (*this < 0)) q = - q;
    if (*this < 0) r = - r;

    remainder = r;
    return q;
};

bool int128::bit(unsigned int n) const
{
    if (n >= 128)
        return hi < 0;

    if (n < 64)
        return lo & (1ull << n);
    else
        return hi & (1ull << (n - 64));
};

void int128::bit(unsigned int n, bool val)
{
    if (n >= 128)
        return;

    if (val)
    {
        if (n < 64) lo |= (1ull << n);
        else hi |= (1ull << (n - 64));
    }
    else
    {
        if (n < 64) lo &= ~(1ull << n);
        else hi &= ~(1ull << (n - 64));
    };
};


int128 & int128::operator>>=(unsigned int n)
{
    if (n >= 128)
    {
        hi = (hi < 0) ? -1ll : 0ll;
        lo = hi;
        return *this;
    }

    if (n >= 64)
    {
        lo = hi >> (n-64);
        hi = (hi < 0) ? -1ll : 0ll;
        return *this;
    };

    if (n)
    {
        // shift low qword
        lo >>= n;

        // get lower N bits of high qword
        uint64_t mask = (1ull << n) - 1;

        // and add them to low qword
        lo |= (hi & mask) << (64 - n);

        // and finally shift also high qword
        hi >>= n;
    };

    return *this;
};

int128 & int128::operator<<=(unsigned int n)
{
    if (n >= 128)
    {
        lo = hi = 0;
        return *this;
    }

    if (n >= 64)
    {
        hi = lo << (n-64);
        lo = 0ull;
        return *this;
    };

    if (n)
    {
        // shift high qword
        hi <<= n;

        // get higher N bits of low qword
        uint64_t mask = ~((1ull << (64 - n)) - 1);

        // and add them to high qword
        hi |= (lo & mask) >> (64 - n);

        // and finally shift also low qword
        lo <<= n;
    };

    return *this;
};

bool operator<(const int128 & a, const int128 & b)
{
    if (a.hi == b.hi)
    {
        if (a.hi < 0)
            return (int64_t) a.lo < (int64_t) b.lo;
        else
            return a.lo < b.lo;
    }
    else
        return a.hi < b.hi;
};

