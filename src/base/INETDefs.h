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

#ifndef __INET_INETDEFS_H
#define __INET_INETDEFS_H

//
// General definitions.
//

#include <omnetpp.h>

#if OMNETPP_VERSION < 0x0402
#  error At least OMNeT++/OMNEST version 4.2 required
#endif

#if defined(INET_EXPORT)
#  define INET_API OPP_DLLEXPORT
#elif defined(INET_IMPORT)
#  define INET_API OPP_DLLIMPORT
#else
#  define INET_API
#endif

typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;


//
// Macro to prevent executing ev<< statements in Express mode.
// Compare ev/sec values with code compiled with #define EV ev.
//
#define EV ev.isDisabled()?ev:ev

// used at several places as
#define SPEED_OF_LIGHT 299792458.0

//
// Macro to protect expressions like gate("out")->getToGate()->getToGate()
// from crashing if something in between returns NULL.
// The above expression should be changed to
//    CHK(CHK(gate("out"))->getToGate())->getToGate()
// which is uglier but doesn't crash, just stops with a nice
// error message if something goes wrong.
//
template <class T>
T *__checknull(T *p, const char *expr, const char *file, int line)
{
    if (!p)
        opp_error("Expression %s returned NULL at %s:%d", expr, file, line);
    return p;
}
#define CHK(x) __checknull((x), #x, __FILE__, __LINE__)


#define PK(msg)  check_and_cast<cPacket *>(msg)    /*XXX temp def*/


#ifdef _MSC_VER
// complementary error function, not in MSVC
double INET_API erfc(double x);

// ISO C99 function, not in MSVC
inline long lrint(double x)
{
    return (long)floor(x+0.5);
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
#endif


#endif
