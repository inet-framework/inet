//
// Copyright (C) 2005 Andras Varga
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

#ifndef __INET_INETUTILS_H
#define __INET_INETUTILS_H

#include <string>

#include "inet/common/INETDefs.h"

namespace inet {

namespace utils {

/**
 *  Converts an integer to string.
 */
INET_API std::string ltostr(long i);    //XXX make an ultostr as well, to be consistent with atoul

/**
 *  Converts a double to string
 */
INET_API std::string dtostr(double d);

/**
 *  Converts string to double
 */
INET_API double atod(const char *s);

/**
 *  Converts string to unsigned long
 */
INET_API unsigned long atoul(const char *s);

/**
 * Removes non-alphanumeric characters from the given string.
 */
INET_API std::string stripnonalnum(const char *s);

/**
 * Accepts a printf-like argument list, and returns the result in a string.
 * The limit is 1024 chars.
 */
INET_API std::string stringf(const char *fmt, ...);

/**
 * Accepts a vprintf-like argument list, and returns the result in a string.
 * The limit is 1024 chars.
 */
INET_API std::string vstringf(const char *fmt, va_list& args);

/**
 * Rounding up to the nearest multiple of a number.
 */
inline int roundUp(int numToRound, int multiple) { return ((numToRound + multiple -1) / multiple) * multiple; }

/**
 * Like cObjectFactory::createOneIfClassIsKnown(), except it starts searching for the class in the given namespace
 */
INET_API cObject *createOneIfClassIsKnown(const char *className, const char *defaultNamespace = getSimulation()->getContext()->getClassName());

/**
 * Like cObjectFactory::createOne(), except it starts searching for the class in the given namespace
 */
INET_API cObject *createOne(const char *className, const char *defaultNamespace = getSimulation()->getContext()->getClassName());

/**
 * Duplicate a packet together with its control info. (cPacket's dup() ignores the control info,
 * it will be nullptr in the returned copy).
 */
template<typename T>
T *dupPacketAndControlInfo(T *packet) {
    T *copy  = packet->dup();
    if (cObject *ctrl = packet->getControlInfo())
        copy->setControlInfo(ctrl->dup());
    return copy;
}

} // namespace utils

} // namespace inet

#endif // ifndef __INET_OPP_UTILS_H

