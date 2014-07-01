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
//

#include <omnetpp.h>
#include "Compat.h"

#if OMNETPP_VERSION < 0x0403
#  error At least OMNeT++/OMNEST version 4.3 required
#endif // if OMNETPP_VERSION < 0x0403

#if defined(INET_EXPORT)
#  define INET_API    OPP_DLLEXPORT
#elif defined(INET_IMPORT)
#  define INET_API    OPP_DLLIMPORT
#else // if defined(INET_EXPORT)
#  define INET_API
#endif // if defined(INET_EXPORT)

#include "InitStages.h"

namespace inet {
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

// used at several places as
#define SPEED_OF_LIGHT    299792458.0

//
// Macro to protect expressions like gate("out")->getToGate()->getToGate()
// from crashing if something in between returns NULL.
// The above expression should be changed to
//    CHK(CHK(gate("out"))->getToGate())->getToGate()
// which is uglier but doesn't crash, just stops with a nice
// error message if something goes wrong.
//
template<class T>
T *__checknull(T *p, const char *expr, const char *file, int line)
{
    if (!p)
        throw cRuntimeError("Expression %s returned NULL at %s:%d", expr, file, line);
    return p;
}

#define CHK(x)     __checknull((x), #x, __FILE__, __LINE__)

#define PK(msg)    check_and_cast<cPacket *>(msg)    /*XXX temp def*/

/*
   template<typename T, typename A>
   inline std::ostream& operator<<(std::ostream& out, const std::vector<T,A>& vec )
   {
    out << '{';
    for(typename std::vector<T,A>::const_iterator it = vec.begin(); it != vec.end(); ++it)
    {
        if (it != vec.begin())
            out << ",";
        out << *it;
    }
    out << '}';
    return out;
   }
 */

/**
 * Shortcut to cObjectFactory::createOneIfClassIsKnown().
 */
inline cObject *createOneIfClassIsKnown(const char *classname)
{
    cObject *ret = cObjectFactory::createOneIfClassIsKnown(classname);
    if (!ret) {
        char buf[200];    //FIXME
        snprintf(buf, 200, "inet::%s", classname);
        ret = cObjectFactory::createOneIfClassIsKnown(buf);
    }
    return ret;
}

inline cObject *createOne(const char *classname)
{
    cObject *ret = createOneIfClassIsKnown(classname);
    if (!ret)
        throw cRuntimeError("Class [inet::]\"%s\" not found -- perhaps its code was not linked in, "
                            "or the class wasn't registered with Register_Class(), or in the case of "
                            "modules and channels, with Define_Module()/Define_Channel()", classname);
    return ret;
}
} // namespace inet

#endif // ifndef __INET_INETDEFS_H

