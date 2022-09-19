//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
INET_API std::string ltostr(long i); // TODO make an ultostr as well, to be consistent with atoul

/**
 *  Converts a double to string
 */
INET_API std::string dtostr(double d);

INET_API std::string hex(int16_t l);
INET_API std::string hex(uint16_t l);
INET_API std::string hex(int32_t l);
INET_API std::string hex(uint32_t l);
INET_API std::string hex(int64_t l);
INET_API std::string hex(uint64_t l);

INET_API long hex(const char *s);

INET_API unsigned long uhex(const char *s);

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
inline int roundUp(int numToRound, int multiple) { return ((numToRound + multiple - 1) / multiple) * multiple; }

/**
 * Like cObjectFactory::createOneIfClassIsKnown(), except it starts searching for the class in the given namespace
 */
INET_API cObject *createOneIfClassIsKnown(const char *className, const char *defaultNamespace = nullptr);

/**
 * Like cObjectFactory::createOne(), except it starts searching for the class in the given namespace
 */
INET_API cObject *createOne(const char *className, const char *defaultNamespace = nullptr);

/**
 * Duplicate a packet together with its control info. (cPacket's dup() ignores the control info,
 * it will be nullptr in the returned copy).
 */
template<typename T>
T *dupPacketAndControlInfo(T *packet) {
    T *copy = packet->dup();
    if (cObject *ctrl = packet->getControlInfo())
        copy->setControlInfo(ctrl->dup());
    return copy;
}

INET_API bool fileExists(const char *pathname);
INET_API void splitFileName(const char *pathname, std::string& dir, std::string& fnameonly);
INET_API void makePath(const char *pathname);
INET_API void makePathForFile(const char *filename);

} // namespace utils

} // namespace inet

#endif

