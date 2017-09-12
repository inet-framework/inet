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

#include <memory>

//
// General definitions.
//

#include "inet/common/Compat.h"

using namespace omnetpp;

#if OMNETPP_VERSION < 0x0501 || OMNETPP_BUILDNUM < 1010
#  error At least OMNeT++/OMNEST version 5.1 required
#endif // if OMNETPP_VERSION < 0x0501

#define INET_VERSION  0x0363
#define INET_PATCH_LEVEL 0x00

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

// The following macro and template definitions allow the user to replace the
// standard shared pointer implementation in the Packet API with another one.
// The standard shared pointer implementation is thread safe if the threading
// library is used. Thread safety requires additional synchronization primitives
// which results in unnecessary performance penalty. In general, thread safety
// is not a requirement for OMNeT++/INET simulations
#define INET_STD_SHARED_PTR 1
#define INET_PTR_IMPLEMENTATION INET_STD_SHARED_PTR
#if INET_PTR_IMPLEMENTATION == INET_STD_SHARED_PTR

template<class T>
using Ptr = std::shared_ptr<T>;

template<class T, typename... Args>
Ptr<T> makeShared(Args&&... args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}

template<class T, class U>
Ptr<T> staticPtrCast(const Ptr<U>& r)
{
    return std::static_pointer_cast<T>(r);
}

template<class T, class U>
Ptr<T> dynamicPtrCast(const Ptr<U>& r)
{
    return std::dynamic_pointer_cast<T>(r);
}

template<class T, class U>
Ptr<T> constPtrCast(const Ptr<U>& r)
{
    return std::const_pointer_cast<T>(r);
}

#else
#error "Unknown Ptr implementation"
#endif

//@}

template<class T>
Ptr<T> __checknull(const Ptr<T>& p, const char *expr, const char *file, int line)
{
    if (p == nullptr)
        throw cRuntimeError("Expression %s returned nullptr at %s:%d", expr, file, line);
    return p;
}

#define RNGCONTEXT  (cSimulation::getActiveSimulation()->getContext())->

#define FINGERPRINT_ADD_EXTRA_DATA(x)  { if (cFingerprintCalculator *fpc = getSimulation()->getFingerprintCalculator()) fpc->addExtraData(x); }
#define FINGERPRINT_ADD_EXTRA_DATA2(x,y)  { if (cFingerprintCalculator *fpc = getSimulation()->getFingerprintCalculator()) fpc->addExtraData(x, y); }

#define CHK(x)     __checknull((x), #x, __FILE__, __LINE__)

#define PK(msg)    check_and_cast<cPacket *>(msg)    /*XXX temp def*/

inline void printElapsedTime(const char *name, long startTime)
{
    EV_DEBUG << "Time spent in " << name << ": " << ((double)(clock() - startTime) / CLOCKS_PER_SEC) << "s" << endl;
}

#define TIME(CODE)    { long startTime = clock(); CODE; printElapsedTime( #CODE, startTime); }

} // namespace inet

#endif // ifndef __INET_INETDEFS_H
