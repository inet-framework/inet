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

namespace omnetpp { }  // so "using namespace omnetpp" in INETDefs.h doesn't cause error for OMNeT++ 4.x

namespace inet {

#if OMNETPP_VERSION >= 0x500
    typedef uint64_t uint64;
    typedef int64_t  int64;
    typedef uint32_t uint32;
    typedef int32_t  int32;
    typedef uint16_t uint16;
    typedef int16_t  int16;
    typedef uint8_t  uint8;
#endif  // OMNETPP_VERSION >= 0x500

#if OMNETPP_VERSION < 0x500
#  define EV_FATAL                 EV << "FATAL: "
#  define EV_ERROR                 EV << "ERROR: "
#  define EV_WARN                  EV << "WARN: "
#  define EV_INFO                  EV
#  define EV_DETAIL                EV << "DETAIL: "
#  define EV_DEBUG                 EV << "DEBUG: "
#  define EV_TRACE                 EV << "TRACE: "
#  define EV_FATAL_C(category)     EV << "[" << category << "] FATAL: "
#  define EV_ERROR_C(category)     EV << "[" << category << "] ERROR: "
#  define EV_WARN_C(category)      EV << "[" << category << "] WARN: "
#  define EV_INFO_C(category)      EV << "[" << category << "] "
#  define EV_DETAIL_C(category)    EV << "[" << category << "] DETAIL: "
#  define EV_DEBUG_C(category)     EV << "[" << category << "] DEBUG: "
#  define EV_TRACE_C(category)     EV << "[" << category << "] TRACE: "
#  define EV_STATICCONTEXT         /* Empty */
#endif    // OMNETPP_VERSION < 0x500

#if OMNETPP_VERSION < 0x404
#  define Register_Abstract_Class(x)    /* nothing */
#endif // if OMNETPP_VERSION < 0x404

#if OMNETPP_VERSION < 0x500
#  define EVSTREAM                      getEnvir()->getOStream()
#else // if OMNETPP_VERSION < 0x500
#  define EVSTREAM                      EV
#endif    // OMNETPP_VERSION < 0x500

// Around OMNeT++ 5.0 beta 2, the "ev" and "simulation" macros were eliminated, and replaced
// by the functions/methods getEnvir() and getSimulation(), the INET codebase updated.
// The following lines let the code compile with earlier OMNeT++ versions as well.
#ifdef ev
inline cEnvir *getEnvir() {return cSimulation::getActiveEnvir();}
inline cSimulation *getSimulation() {return cSimulation::getActiveSimulation();}
inline bool hasGUI() {return cSimulation::getActiveEnvir()->isGUI();}
#endif  //ev

// Around OMNeT++ 5.0 beta 2, random variate generation functions like exponential() became
// members of cComponent. By prefixing calls with the following macro you can make the code
// compile with earlier OMNeT++ versions as well.
#if OMNETPP_BUILDNUM >= 1002
#define RNGCONTEXT  (cSimulation::getActiveSimulation()->getContext())->
#else
#define RNGCONTEXT
#endif

// Around OMNeT++ 5.0 beta 3, signal listeners and result filters/recorders received
// an extra cObject *details argument.
#if OMNETPP_BUILDNUM >= 1005
#define DETAILS_ARG        ,cObject *details
#define DETAILS_ARG_NAME   ,details
#else
#define DETAILS_ARG
#define DETAILS_ARG_NAME
#endif

// Around OMNeT++ 5.0 beta 3, fingerprint computation has been changed.
#if OMNETPP_BUILDNUM >= 1006
#define FINGERPRINT_ADD_EXTRA_DATA(x)  { if (cFingerprintCalculator *fpc = getSimulation()->getFingerprintCalculator()) fpc->addExtraData(x); }
#define FINGERPRINT_ADD_EXTRA_DATA2(x,y)  { if (cFingerprintCalculator *fpc = getSimulation()->getFingerprintCalculator()) fpc->addExtraData(x, y); }
#elif OMNETPP_BUILDNUM >= 1005
#define FINGERPRINT_ADD_EXTRA_DATA(x)  { if (cFingerprint *fingerprint = getSimulation()->getFingerprint()) fingerprint->addExtraData(x); }
#define FINGERPRINT_ADD_EXTRA_DATA2(x,y)  { if (cFingerprint *fingerprint = getSimulation()->getFingerprint()) fingerprint->addExtraData(x, y); }
#else
#define FINGERPRINT_ADD_EXTRA_DATA(x)  { if (cHasher *hasher = getSimulation()->getHasher()) hasher->add(x); }
#define FINGERPRINT_ADD_EXTRA_DATA2(x,y)  { if (cHasher *hasher = getSimulation()->getHasher()) hasher->add(x, y); }
#endif

// Around OMNeT++ 5.0 beta 3, MAXTIME was renamed to SIMTIME_MAX
#if OMNETPP_BUILDNUM < 1005
#define SIMTIME_MAX MAXTIME
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

#if OMNETPP_VERSION < 0x0500

NAMESPACE_BEGIN

/**
 * A check_and_cast<> that accepts pointers other than cObject*, too.
 * For compatibility; OMNeT++ 5.0 and later already contain this.
 */
template<class T, class P>
T check_and_cast(P *p)
{
    if (!p)
        throw cRuntimeError("check_and_cast(): cannot cast nullptr pointer to type '%s'", opp_typename(typeid(T)));
    T ret = dynamic_cast<T>(p);
    if (!ret) {
        const cObject *o = dynamic_cast<const cObject *>(p);
        if (o)
            throw cRuntimeError("check_and_cast(): cannot cast (%s *)%s to type '%s'", o->getClassName(), o->getFullPath().c_str(), opp_typename(typeid(T)));
        else
            throw cRuntimeError("check_and_cast(): cannot cast %s to type '%s'", opp_typename(typeid(P)), opp_typename(typeid(T)));
    }
    return ret;
}

/**
 * A const version of check_and_cast<> that accepts pointers other than cObject*, too.
 * For compatibility; OMNeT++ 5.0 and later already contain this.
 */
template<class T, class P>
T check_and_cast(const P *p)
{
    if (!p)
        throw cRuntimeError("check_and_cast(): cannot cast nullptr pointer to type '%s'", opp_typename(typeid(T)));
    T ret = dynamic_cast<T>(p);
    if (!ret) {
        const cObject *o = dynamic_cast<const cObject *>(p);
        if (o)
            throw cRuntimeError("check_and_cast(): cannot cast (%s *)%s to type '%s'", o->getClassName(), o->getFullPath().c_str(), opp_typename(typeid(T)));
        else
            throw cRuntimeError("check_and_cast(): cannot cast %s to type '%s'", opp_typename(typeid(P)), opp_typename(typeid(T)));
    }
    return ret;
}

template<class T, class P>
T check_and_cast_nullable(P *p)
{
    if (!p)
        return nullptr;
    return check_and_cast<T>(p);
}

NAMESPACE_END

#endif    // OMNETPP_VERSION < 0x0500

#endif // ifndef __INET_COMPAT_H

