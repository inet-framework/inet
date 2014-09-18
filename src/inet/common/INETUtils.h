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

#ifndef __INET_OPP_UTILS_H
#define __INET_OPP_UTILS_H

#include <string>

#include "inet/common/INETDefs.h"

namespace inet {

namespace utils {

/**
 *  Converts an integer to string.
 */
std::string ltostr(long i);    //XXX make an ultostr as well, to be consistent with atoul

/**
 *  Converts a double to string
 */
std::string dtostr(double d);

/**
 *  Converts string to double
 */
double atod(const char *s);

/**
 *  Converts string to unsigned long
 */
unsigned long atoul(const char *s);

/**
 * Removes non-alphanumeric characters from the given string.
 */
std::string stripnonalnum(const char *s);

/**
 * Accepts a printf-like argument list, and returns the result in a string.
 * The limit is 1024 chars.
 */
std::string stringf(const char *fmt, ...);

/**
 * Accepts a vprintf-like argument list, and returns the result in a string.
 * The limit is 1024 chars.
 */
std::string vstringf(const char *fmt, va_list& args);

/**
 * Like cObjectFactory::createOneIfClassIsKnown(), except it starts searching for the class in the given namespace
 */
cObject *createOneIfClassIsKnown(const char *className, const char *defaultNamespace = simulation.getContext()->getClassName());

/**
 * Like cObjectFactory::createOne(), except it starts searching for the class in the given namespace
 */
cObject *createOne(const char *className, const char *defaultNamespace = simulation.getContext()->getClassName());

} // namespace utils

} // namespace inet

#endif // ifndef __INET_OPP_UTILS_H

