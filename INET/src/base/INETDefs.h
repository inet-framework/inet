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
// Andras Varga
//

#include <omnetpp.h>

#if OMNETPP_VERSION < 0x0400
#  error At least OMNeT++/OMNEST version 4.0 required
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
        opp_error("Expression %s returned NULL at %s:%d",expr,file,line);
    return p;
}
#define CHK(x) __checknull((x), #x, __FILE__, __LINE__)


#define PK(msg)  check_and_cast<cPacket *>(msg)    /*XXX temp def*/


#ifdef _MSC_VER
//
// Implementation of the error function, from the Mobility Framework
//
// Unfortunately the windows math library does not provide an
// implementation of the error function, so we use an own
// implementation (Thanks to Jirka Klaue)
// Author Jirka Klaue
//
static double erfc(double x)
{
    double t, u, y;

    if (x <= -6)
        return 2;
    if (x >= 6)
        return 0;

    t = 3.97886080735226 / (fabs(x) + 3.97886080735226);
    u = t - 0.5;
    y = (((((((((0.00127109764952614092 * u + 1.19314022838340944e-4) * u -
        0.003963850973605135) * u - 8.70779635317295828e-4) * u +
        0.00773672528313526668) * u + 0.00383335126264887303) * u -
        0.0127223813782122755) * u - 0.0133823644533460069) * u +
        0.0161315329733252248) * u + 0.0390976845588484035) * u +
        0.00249367200053503304;
    y = ((((((((((((y * u - 0.0838864557023001992) * u -
        0.119463959964325415) * u + 0.0166207924969367356) * u +
        0.357524274449531043) * u + 0.805276408752910567) * u +
        1.18902982909273333) * u + 1.37040217682338167) * u +
        1.31314653831023098) * u + 1.07925515155856677) * u +
        0.774368199119538609) * u + 0.490165080585318424) * u +
        0.275374741597376782) * t * exp(-x * x);

    return x < 0 ? 2 - y : y;
}
#endif

#endif
