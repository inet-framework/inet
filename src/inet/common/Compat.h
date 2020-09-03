//
// Copyright (C) 2004 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#ifndef __INET_COMPAT_H
#define __INET_COMPAT_H

#include <iostream>
#include <omnetpp.h>

namespace inet {

    typedef uint64_t uint64;
    typedef int64_t  int64;
    typedef uint32_t uint32;
    typedef int32_t  int32;
    typedef uint16_t uint16;
    typedef int16_t  int16;
    typedef uint8_t  uint8;

#  define EVSTREAM                      EV

#if OMNETPP_BUILDNUM < 1503
#define findModuleByPath(x)    getModuleByPath(x)
#endif

#ifdef _MSC_VER
// complementary error function, not in MSVC
double INET_API erfc(double x);

// ISO C99 function, not in MSVC
inline long lrint(double x)
{
    return (long)floor(x + 0.5);
}

// ISO C99 function, not in MSVC
inline double fmin(double a, double b)
{
    return a < b ? a : b;
}

// ISO C99 function, not in MSVC
inline double fmax(double a, double b)
{
    return a > b ? a : b;
}

#endif    // _MSC_VER

} // namespace inet

#endif

