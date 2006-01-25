//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#ifndef _INETDEFS_H__
#define _INETDEFS_H__

//
// General definitions.
// Andras Varga
//

#include <omnetpp.h>

#if OMNETPP_VERSION < 0x0302
#  error At least OMNeT++/OMNEST version 3.2 required
#endif

#ifdef BUILDING_INET
#  define INET_API  OPP_DLLEXPORT
#else
#  define INET_API  OPP_DLLIMPORT
#endif

typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned int uint32;


//
// Macro to prevent executing ev<< statements in Express mode.
// Compare ev/sec values with code compiled with #define EV ev.
//
#define EV ev.disabled()?std::cout:ev


//
// Macro to protect expressions like gate("out")->toGate()->toGate()
// from crashing if something in between returns NULL.
// The above expression should be changed to
//    CHK(CHK(gate("out"))->toGate())->toGate()
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

#endif
