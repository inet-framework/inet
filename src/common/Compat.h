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

#if OMNETPP_VERSION < 0x500
#  define EV_FATAL                         EV << "FATAL: "
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
        #  define EVSTREAM                      ev.getOStream()
    #else // if OMNETPP_VERSION < 0x500
        #  define EVSTREAM                      EV
    #endif    // OMNETPP_VERSION < 0x500

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

#if OMNETPP_VERSION < 0x0500
/**
 * A check_and_cast<> that accepts pointers other than cObject*, too.
 * For compatibility; OMNeT++ 5.0 and later already contain this.
 */
template<class T, class P>
T check_and_cast(P *p)
{
    if (!p)
        throw cRuntimeError("check_and_cast(): cannot cast NULL pointer to type '%s'", opp_typename(typeid(T)));
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
        throw cRuntimeError("check_and_cast(): cannot cast NULL pointer to type '%s'", opp_typename(typeid(T)));
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
        return NULL;
    return check_and_cast<T>(p);
}

#endif    // OMNETPP_VERSION < 0x0500

} // namespace inet

#endif // ifndef __INET_COMPAT_H

