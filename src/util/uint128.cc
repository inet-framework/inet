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


#include "uint128.h"

const Uint128 Uint128::UINT128_MAX(UINT64_MAX, UINT64_MAX);
const Uint128 Uint128::UINT128_MIN(0, 0);


const char *Uint128::toString(unsigned int radix) const
{
    if (!*this) return "0";
    if (radix < 2 || radix > 37) return "(invalid radix)";

    static char sz[256];
    memset(sz, 0, 256);

    Uint128 r;
    Uint128 ii(*this);
    Uint128 aux(radix);
    int i = 255;

    while (!!ii && i)
    {
        ii = ii.div(aux, r);
        unsigned int c = r.toUint();
        sz[--i] = c + ((c > 9) ? 'A' - 10 : '0');
    }

    return &sz[i];
}

void Uint128::set(const char *sz)
{
    hi = 0;
    lo = 0;

    if (!sz) return;
    if (!sz [0]) return;

    unsigned int radix = 10;
    unsigned int i = 0;
    bool minus = false;

    if (sz [i] == '-')
    {
        ++i;
        minus = true;
    }

    if (sz [i] == '0')
    {
        radix = 8;
        ++i;
        if (sz [i] == 'x')
        {
            radix = 16;
            ++i;
        }
    }

    for (; i < strlen(sz); ++i)
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
    }

    if (minus)
    {
        *this = Uint128(0) - *this;
    }
}

Uint128& Uint128::operator=(const float& a)
{
    lo = ((uint64_t) fmodf(a, 18446744073709551616.0f));
    hi = ((uint64_t) (a / 18446744073709551616.0f));
    return *this;
}

Uint128& Uint128::operator=(const double& a)
{
    lo = ((uint64_t) fmod(a, 18446744073709551616.0));
    hi = ((uint64_t) (a / 18446744073709551616.0));
    return *this;
}

Uint128& Uint128::operator=(const long double& a)
{
    lo = ((uint64_t) fmodl(a, 18446744073709551616.0l));
    hi = ((uint64_t) (a / 18446744073709551616.0l));
    return *this;
}


Uint128::Uint128(const float a)
        : lo((uint64_t) fmodf(a, 18446744073709551616.0f)),
        hi((uint64_t) (a / 18446744073709551616.0f)) {}

Uint128::Uint128(const double& a)
        : lo((uint64_t) fmod(a, 18446744073709551616.0)),
        hi((uint64_t) (a / 18446744073709551616.0)) {}

Uint128::Uint128(const long double& a)
        : lo((uint64_t) fmodl(a, 18446744073709551616.0l)),
        hi((uint64_t) (a / 18446744073709551616.0l)) {}

float Uint128::toFloat() const
{
    return (float) hi * 18446744073709551616.0f
           + (float) lo;
}

double Uint128::toDouble() const
{
    return (double) hi * 18446744073709551616.0
           + (double) lo;
}

long double Uint128::toLongDouble() const
{
    return (long double) hi * 18446744073709551616.0l
           + (long double) lo;
}

Uint128 Uint128::operator-() const
{
    if (lo == 0)
        return Uint128(0ull, -hi);
    else
        return Uint128(-lo, ~hi);
}

Uint128 Uint128::operator ~ () const
{
    return Uint128(~lo, ~hi);
}

Uint128& Uint128::operator++()
{
    ++lo;
    if (!lo)
        ++hi;

    return *this;
}

Uint128& Uint128::operator--()
{
    if (!lo)
        --hi;
    --lo;

    return *this;
}

Uint128 Uint128::operator++(int)
{
    Uint128 b(*this);
    ++ *this;

    return b;
}

Uint128 Uint128::operator--(int)
{
    Uint128 b(*this);
    -- *this;

    return b;
}

Uint128& Uint128::operator+=(const Uint128& b)
{
    uint64_t old_lo = lo;

    lo += b.lo;
    hi += b.hi;
    if(lo < old_lo)
        ++hi;

    return *this;
}

Uint128& Uint128::operator*=(const Uint128& b)
{
    if (!b)
        return *this = 0u;
    if (b == 1u)
        return *this;

    Uint128 a = *this;
    Uint128 t = b;

    lo = 0ull;
    hi = 0ull;

    for (unsigned int i = 0; i < 128; ++i)
    {
        if (t.lo & 1)
            *this += a << i;

        t >>= 1;
    }

    return *this;
}


Uint128 Uint128::div(const Uint128& ds, Uint128& remainder) const
{
    if (!ds)
        return 1u / (unsigned int) ds.lo;

    Uint128 dd = *this;

    // only remainder
    if (ds > dd)
    {
        remainder = *this;
        return (Uint128)0;
    }

    Uint128 r = (Uint128) 0;
    Uint128 q = (Uint128) 0;
//    while (dd >= ds) { dd -= ds; q += 1; } // extreme slow version

    uint32_t b = 127;
    while (r < ds)
    {
        r <<= 1;
        if (dd.bit(b--))
            r.lo |= 1;
    }
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
        }

    remainder = r;
    return q;
}

bool Uint128::bit(unsigned int n) const
{
    if (n < 64)
        return lo & (1ull << n);
    if (n < 128)
        return hi & (1ull << (n - 64));
    return false;
}

void Uint128::bit(unsigned int n, bool val)
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
    }
}


Uint128& Uint128::operator>>=(unsigned int n)
{
    if (n >= 128)
    {
        lo = hi = 0ull;
        return *this;
    }

    if (n >= 64)
    {
        lo = hi >> (n-64);
        hi = 0ull;
        return *this;
    }

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
    }

    return *this;
}

Uint128& Uint128::operator<<=(unsigned int n)
{
    if (n >= 128)
    {
        lo = hi = 0ull;
        return *this;
    }

    if (n >= 64)
    {
        hi = lo << (n-64);
        lo = 0ull;
        return *this;
    }

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
    }

    return *this;
}



bool operator!=(const Uint128& a, const uint32_t& b)
{
    return  !(a == b);
}

bool operator!=(const uint32_t& b, const Uint128& a)
{
    return  !(a == b);
}

bool operator!=(const Uint128& a, const uint64_t& b)
{
    return  !(a == b);
}

bool operator!=(const uint64_t& b, const Uint128& a)
{
    return  !(a == b);
}


bool operator!=(const Uint128& a, const int32_t& b)
{
    return  !(a == b);
}

bool operator!=(const int32_t& b, const Uint128& a)
{
    return  !(a == b);
}

bool operator!=(const Uint128& a, const int64_t& b)
{
    return  !(a == b);
}

bool operator!=(const int64_t& b, const Uint128& a)
{
    return  !(a == b);
}

