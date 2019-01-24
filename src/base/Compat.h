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

#ifndef __INET_COMPAT_H_
#define __INET_COMPAT_H_

#include <iostream>
#include <omnetpp.h>
#if OMNETPP_VERSION >= 0x504
#include "inet_features.h"
#endif

#if OMNETPP_VERSION >= 0x500
    typedef uint64_t uint64;
    typedef int64_t  int64;
    typedef uint32_t uint32;
    typedef int32_t  int32;
    typedef uint16_t uint16;
    typedef int16_t  int16;
    typedef uint8_t  uint8;

#if OMNETPP_VERSION >= 0x600
    typedef cHistogram cDoubleHistogram;
#endif // OMNETPP_VERSION >= 0x600

#if OMNETPP_VERSION >= 0x504
#define opp_isEmpty   isEmpty
#define opp_error throw cRuntimeError
#define longValue    intValue
#define simulation   (*getSimulation())
#define ev           EV
#define info()       str()
#define MAXTIME      SIMTIME_MAX
#define RNGCONTEXT  (cSimulation::getActiveSimulation()->getContext())->
#define STR_SIMTIME(x)   SimTime::parse(x)
    typedef cObject cPolymorphic;
    inline long genk_intrand(int k,long r)  { return cSimulation::getActiveSimulation()->getContext()->getRNG(k)->intRand(r); }
    inline int64_t SIMTIME_RAW(simtime_t t)  { return t.raw(); }
#define FINGERPRINT_ADD_EXTRA_DATA(x)  { if (cFingerprintCalculator *fpc = getSimulation()->getFingerprintCalculator()) fpc->addExtraData(x); }
#define FINGERPRINT_ADD_EXTRA_DATA2(x,y)  { if (cFingerprintCalculator *fpc = getSimulation()->getFingerprintCalculator()) fpc->addExtraData(x, y); }
#else
#define FINGERPRINT_ADD_EXTRA_DATA(mos)  { if (cHasher *hasher = simulation.getHasher()) hasher->add(mos); }
#define opp_isEmpty   empty
#define hasGui()   ev.isGui()
#define getEnvir()   (*ev)
#define isExpressMode() isDisabled()
#endif  // OMNETPP_VERSION >= 0x504

#endif  // OMNETPP_VERSION >= 0x600

#if OMNETPP_VERSION < 0x0500
//cClassDescriptor compatibility
#define getFieldValueAsString(a, b, c)         getFieldAsString((a), (b), (c))
#define getFieldProperty(a, b)                 getFieldProperty(nullptr, (a), (b))
#define getFieldName(a)                        getFieldName(nullptr, (a))
#define getFieldIsCObject(a)                   getFieldIsCObject(nullptr, (a))
#define getFieldCount()                        getFieldCount(nullptr)
#define getFieldTypeString(a)                  getFieldTypeString(nullptr, (a))
#define getFieldIsArray(a)                     getFieldIsArray(nullptr, (a))
#define getFieldArraySize(a, b)                getArraySize((a), (b))
#define getFieldStructValuePointer(a, b, c)    getFieldStructPointer((a), (b), (c))
#define getFieldTypeFlags(a)                   getFieldTypeFlags(nullptr, (a))
#define getFieldStructName(a)                  getFieldStructName(nullptr, (a))
#endif    // OMNETPP_VERSION < 0x0500

#if OMNETPP_VERSION < 0x500
#  define EV_FATAL  EV << "FATAL: "
#  define EV_ERROR  EV << "ERROR: "
#  define EV_WARN   EV << "WARN: "
#  define EV_INFO   EV
#  define EV_DETAIL EV << "DETAIL: "
#  define EV_DEBUG  EV << "DEBUG: "
#  define EV_TRACE  EV << "TRACE: "

#  define EV_FATAL_C(category)  EV << "[" << category << "] FATAL: "
#  define EV_ERROR_C(category)  EV << "[" << category << "] ERROR: "
#  define EV_WARN_C(category)   EV << "[" << category << "] WARN: "
#  define EV_INFO_C(category)   EV << "[" << category << "] "
#  define EV_DETAIL_C(category) EV << "[" << category << "] DETAIL: "
#  define EV_DEBUG_C(category)  EV << "[" << category << "] DEBUG: "
#  define EV_TRACE_C(category)  EV << "[" << category << "] TRACE: "

#  define EV_STATICCONTEXT  /* Empty */

#endif  // OMNETPP_VERSION < 0x500

#if OMNETPP_VERSION < 0x404
#  define Register_Abstract_Class(x)    /* nothing */
#endif

#if OMNETPP_VERSION < 0x500
#  define EVSTREAM  getEnvir()->getOStream()
#else
#  define EVSTREAM  EV
#endif  // OMNETPP_VERSION < 0x500

// Around OMNeT++ 5.0 beta 3, signal listeners and result filters/recorders received
// an extra cObject *details argument.
#if OMNETPP_BUILDNUM >= 1005
#define DETAILS_ARG        ,cObject *details
#define DETAILS_ARG_NAME   ,details
#else
#define DETAILS_ARG
#define DETAILS_ARG_NAME
#endif

// Around OMNeT++ 5.3, packet printers received an extra const Options *options argument.
#if OMNETPP_VERSION >= 0x503
#define PACKETPRINTEROPTIONS_ARG        ,const Options *options
#define PACKETPRINTEROPTIONS_ARG_NAME   ,options
#define PACKETPRINTEROPTIONS_ARG_NULL   ,nullptr
#else
#define PACKETPRINTEROPTIONS_ARG
#define PACKETPRINTEROPTIONS_ARG_NAME
#define PACKETPRINTEROPTIONS_ARG_NULL
#endif

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
#endif  // _MSC_VER

#if OMNETPP_VERSION < 0x0500
/**
 * A check_and_cast<> that accepts pointers other than cObject*, too.
 * For compatibility; OMNeT++ 5.0 and later already contain this.
 */
template<class T, class P>
T check_and_cast(P *p)
{
    if (!p)
        throw cRuntimeError("check_and_cast(): cannot cast NULL pointer to type '%s'",opp_typename(typeid(T)));
    T ret = dynamic_cast<T>(p);
    if (!ret) {
        const cObject *o = dynamic_cast<const cObject *>(p);
        if (o)
            throw cRuntimeError("check_and_cast(): cannot cast (%s *)%s to type '%s'",o->getClassName(),o->getFullPath().c_str(),opp_typename(typeid(T)));
        else
            throw cRuntimeError("check_and_cast(): cannot cast %s to type '%s'",opp_typename(typeid(P)),opp_typename(typeid(T)));
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
        throw cRuntimeError("check_and_cast(): cannot cast NULL pointer to type '%s'",opp_typename(typeid(T)));
    T ret = dynamic_cast<T>(p);
    if (!ret) {
        const cObject *o = dynamic_cast<const cObject *>(p);
        if (o)
            throw cRuntimeError("check_and_cast(): cannot cast (%s *)%s to type '%s'",o->getClassName(),o->getFullPath().c_str(),opp_typename(typeid(T)));
        else
            throw cRuntimeError("check_and_cast(): cannot cast %s to type '%s'",opp_typename(typeid(P)),opp_typename(typeid(T)));
    }
    return ret;
}

template<class T, class P>
T check_and_cast_nullable(P *p)
{
    if (!p)
        return NULL;
    return check_and_cast<T>(p);
}

#endif  // OMNETPP_VERSION < 0x0500

#endif  // __INET_COMPAT_H_

