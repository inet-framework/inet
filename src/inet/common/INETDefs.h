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

// precompiled headers must be included first
#ifdef NDEBUG
#include "inet/common/precompiled_release.h"
#else
#include "inet/common/precompiled_debug.h"
#endif

// important WITH_* macros defined by OMNET
#include "inet/opp_defines.h"

// feature defines generated based on the actual feature enablement
#include "inet/features.h"

//
// General definitions.
//

#include "inet/common/Compat.h"

namespace inet {
using namespace omnetpp;
}

#if OMNETPP_VERSION < 0x0504 || OMNETPP_BUILDNUM < 1020
#  error At least OMNeT++/OMNEST version 5.4.1 required
#endif // if OMNETPP_VERSION < 0x0504

#define INET_VERSION  0x0402
#define INET_PATCH_LEVEL 0x05

#if OMNETPP_VERSION < 0x0600
#define OMNETPP5_CODE(x) x
typedef long intval_t;
typedef unsigned long uintval_t;
#else
#define OMNETPP5_CODE(x)
#endif // if OMNETPP_VERSION < 0x0600

#if OMNETPP_VERSION >= 0x0600
#define OMNETPP6_CODE(x) x
#else
#define OMNETPP6_CODE(x)
#endif // if OMNETPP_VERSION >= 0x0600


#if defined(INET_EXPORT)
#  define INET_API    OPP_DLLEXPORT
#elif defined(INET_IMPORT)
#  define INET_API    OPP_DLLIMPORT
#else // if defined(INET_EXPORT)
#  define INET_API
#endif // if defined(INET_EXPORT)

#include "inet/common/InitStages.h"

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

//@}

#define RNGCONTEXT  (cSimulation::getActiveSimulation()->getContext())->

#define FINGERPRINT_ADD_EXTRA_DATA(x)  { if (cFingerprintCalculator *fpc = getSimulation()->getFingerprintCalculator()) fpc->addExtraData(x); }
#define FINGERPRINT_ADD_EXTRA_DATA2(x,y)  { if (cFingerprintCalculator *fpc = getSimulation()->getFingerprintCalculator()) fpc->addExtraData(x, y); }

#define CHK(x)     __checknull((x), #x, __FILE__, __LINE__)

#define PK(msg)    check_and_cast<cPacket *>(msg)    /*XXX temp def*/

inline void printElapsedTime(const char *name, long startTime)
{
    EV_DEBUG << "Time spent in " << name << ": " << (static_cast<double>(clock() - startTime) / CLOCKS_PER_SEC) << "s" << endl;
}

#define TIME(CODE)    { long startTime = clock(); CODE; printElapsedTime( #CODE, startTime); }

} // namespace inet

#endif // ifndef __INET_INETDEFS_H
