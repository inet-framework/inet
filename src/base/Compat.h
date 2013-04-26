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

#ifndef EV_FATAL
// TODO: #if OMNETPP_VERSION < 0x0500
// FIXME: eventually remove these forward compatibility macros
#define EV_FATAL EV
#define EV_ERROR EV
#define EV_WARN  EV
#define EV_INFO  EV
#define EV_DEBUG EV
#define EV_TRACE EV

#define EV_FATAL_S EV
#define EV_ERROR_S EV
#define EV_WARN_S  EV
#define EV_INFO_S  EV
#define EV_DEBUG_S EV
#define EV_TRACE_S EV

#define EV_FATAL_P ev.printf
#define EV_ERROR_P ev.printf
#define EV_WARN_P  ev.printf
#define EV_INFO_P  ev.printf
#define EV_DEBUG_P ev.printf
#define EV_TRACE_P ev.printf

#define EV_FATAL_PS ev.printf
#define EV_ERROR_PS ev.printf
#define EV_WARN_PS  ev.printf
#define EV_INFO_PS  ev.printf
#define EV_DEBUG_PS ev.printf
#define EV_TRACE_PS ev.printf

#define EV_S EV

#define EV_GLOBAL_STREAM

class cLogStream : public std::ostream
{
    public:
        static cLogStream globalStream;
};

#endif

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
#endif

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

#endif
