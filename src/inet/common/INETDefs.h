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

// feature defines generated based on the actual feature enablement
#include "inet/features.h"

//
// General definitions.
//

#include <omnetpp.h>
#include "inet/common/Compat.h"

using namespace omnetpp;

#if OMNETPP_VERSION < 0x0406
#  error At least OMNeT++/OMNEST version 4.6 required
#endif // if OMNETPP_VERSION < 0x0406

// OMNETPP_BUILDNUM was introduced around OMNeT++ 5.0beta2, with the initial value of 1001.
// The following lines fake a build number for earlier versions.
#ifndef OMNETPP_BUILDNUM
#  if OMNETPP_VERSION < 0x0500
#    define OMNETPP_BUILDNUM 0
#  else
#    define OMNETPP_BUILDNUM 1000
#  endif
#endif

#define INET_VERSION  0x0302
#define INET_PATCH_LEVEL 0x04

#if defined(INET_EXPORT)
#  define INET_API    OPP_DLLEXPORT
#elif defined(INET_IMPORT)
#  define INET_API    OPP_DLLIMPORT
#else // if defined(INET_EXPORT)
#  define INET_API
#endif // if defined(INET_EXPORT)

#include "inet/common/InitStages.h"

// cObject::parsimPack() became const around build #1001
#if OMNETPP_BUILDNUM >= 1001
#define PARSIMPACK_CONST const
#else
#define PARSIMPACK_CONST
#endif

#if OMNETPP_BUILDNUM <= 1002
#define doParsimPacking doPacking
#define doParsimUnpacking doUnpacking
#endif

// main namespace of INET framework
namespace inet {

typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

// used at several places as
#define SPEED_OF_LIGHT    299792458.0

//
// Macro to protect expressions like gate("out")->getToGate()->getToGate()
// from crashing if something in between returns nullptr.
// The above expression should be changed to
//    CHK(CHK(gate("out"))->getToGate())->getToGate()
// which is uglier but doesn't crash, just stops with a nice
// error message if something goes wrong.
//
template<class T>
T *__checknull(T *p, const char *expr, const char *file, int line)
{
    if (!p)
        throw cRuntimeError("Expression %s returned nullptr at %s:%d", expr, file, line);
    return p;
}

#define CHK(x)     __checknull((x), #x, __FILE__, __LINE__)

#define PK(msg)    check_and_cast<cPacket *>(msg)    /*XXX temp def*/

inline void printElapsedTime(const char *name, long startTime)
{
    EV_DEBUG << "Time spent in " << name << ": " << ((double)(clock() - startTime) / CLOCKS_PER_SEC) << "s" << endl;
}

#define TIME(CODE)    { long startTime = clock(); CODE; printElapsedTime( #CODE, startTime); }

} // namespace inet

#endif // ifndef __INET_INETDEFS_H

