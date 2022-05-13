//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_INETDEFS_H
#define __INET_INETDEFS_H

// precompiled headers must be included first
#ifdef NDEBUG
#include "inet/common/precompiled_release.h"
#else
#include "inet/common/precompiled_debug.h"
#endif

// important INET_WITH_* macros defined by OMNET
#include "inet/opp_defines.h"

// feature defines generated based on the actual feature enablement
#include "inet/features.h"

//
// General definitions.
//

#include "inet/common/Compat.h"

namespace inet {
using namespace omnetpp;
} // namespace inet

#if OMNETPP_VERSION < 0x0600 || OMNETPP_BUILDNUM < 1531
#  error At least OMNeT++/OMNEST version 6.0 required
#endif

#define INET_VERSION        0x0404
#define INET_PATCH_LEVEL    0x00

#if defined(INET_EXPORT)
#define INET_API          OPP_DLLEXPORT
#elif defined(INET_IMPORT)
#define INET_API          OPP_DLLIMPORT
#else // if defined(INET_EXPORT)
#define INET_API
#endif // if defined(INET_EXPORT)

#include "inet/common/InitStages.h"

// main namespace of INET framework
namespace inet {

typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

// used at several places as
#define SPEED_OF_LIGHT    299792458.0

template<class...>
using void_t = void;

//
// Macro to protect expressions like gate("out")->getToGate()->getToGate()
// from crashing if something in between returns nullptr.
// The above expression should be changed to
// CHK(CHK(gate("out"))->getToGate())->getToGate()
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

#define RNGCONTEXT                           (cSimulation::getActiveSimulation()->getContext())->

#define FINGERPRINT_ADD_EXTRA_DATA(x)        { if (cFingerprintCalculator * fpc = getSimulation()->getFingerprintCalculator()) fpc->addExtraData(x); }
#define FINGERPRINT_ADD_EXTRA_DATA2(x, y)    { if (cFingerprintCalculator * fpc = getSimulation()->getFingerprintCalculator()) fpc->addExtraData(x, y); }

#define CHK(x)                               __checknull((x), #x, __FILE__, __LINE__)

#define PK(msg)                              check_and_cast<cPacket *>(msg)    /*TODO temp def*/

inline void printElapsedTime(const char *name, long startTime)
{
    EV_DEBUG << "Time spent in " << name << ": " << (static_cast<double>(clock() - startTime) / CLOCKS_PER_SEC) << "s" << endl;
}

#define TIME(CODE)                            { long startTime = clock(); CODE; printElapsedTime( #CODE, startTime); }

#define GET_3TH_ARG(arg1, arg2, arg3, ...)    arg3

extern INET_API int evFlags;
#define EV_FORMAT_STYLE(format)               (evFlags & IPrintableObject::PRINT_FLAG_FORMATTED ? format : "")
#define EV_NORMAL                             EV_FORMAT_STYLE("\x1b[0m")
#define EV_BOLD                               EV_FORMAT_STYLE("\x1b[1m")
#define EV_FAINT                              EV_FORMAT_STYLE("\x1b[2m")
#define EV_ITALIC                             EV_FORMAT_STYLE("\x1b[3m")
#define EV_UNDERLINE                          EV_FORMAT_STYLE("\x1b[4m")
#define EV_FORMAT_OBJECT(object)              printToStringIfPossible(object, evFlags)

#define EV_FIELD_1(field)                     EV_FIELD_2(field, field)
#define EV_FIELD_2(field, value)              ", " << EV_BOLD << #field << EV_NORMAL << " = " << EV_FORMAT_OBJECT(value)
#define EV_FIELD_CHOOSER(...)                 GET_3TH_ARG(__VA_ARGS__, EV_FIELD_2, EV_FIELD_1, )
#define EV_FIELD(...)                         EV_FIELD_CHOOSER(__VA_ARGS__)(__VA_ARGS__)

#define EV_ENDL                               "." << endl
#define EV_LOC                                EV_FAINT << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << "()" << EV_NORMAL

} // namespace inet

#ifdef INET_WITH_SELFDOC
#include "inet/common/selfdoc/SelfDoc.h"
#endif

#endif

