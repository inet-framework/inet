//
// Copyright (C) 2004 Andras Varga
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
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

#if OMNETPP_VERSION >= 0x0600
#define getModuleByPath  findModuleByPath
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

// Around OMNeT++ 6.0 pre9, send() functions and arrived() function received
// an extra SendOptions& options argument.
#if OMNETPP_BUILDNUM >= 1504
#define SENDOPTIONS_ARG        , const SendOptions& options
#define SENDOPTIONS_ARG_NAME   , options
#else
#define SENDOPTIONS_ARG
#define SENDOPTIONS_ARG_NAME
#endif
} // namespace inet

// Around OMNeT++ 6.0 pre12, cVisitor::visit() return type changed from void to bool
#if OMNETPP_BUILDNUM >= 1513
#define VISIT_RETURNTYPE bool
#define VISIT_RETURN(x) return x
#else
#define VISIT_RETURNTYPE void
#define VISIT_RETURN(x)
#endif



#endif // ifndef __INET_COMPAT_H
