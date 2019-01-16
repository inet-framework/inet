//
// (C) 2005 Vojtech Janota
//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//

#ifndef __INET_UTILS_H
#define __INET_UTILS_H

#include <vector>

#include "inet/common/INETDefs.h"
#include "inet/networklayer/rsvpte/IntServ_m.h"

namespace inet {

EroVector routeToEro(const Ipv4AddressVector& rro);
std::string vectorToString(const Ipv4AddressVector& vec);
std::string vectorToString(const Ipv4AddressVector& vec, const char *delim);
std::string vectorToString(const EroVector& vec);
std::string vectorToString(const EroVector& vec, const char *delim);

/**
 * TODO documentation
 */
void removeDuplicates(std::vector<int>& vec);

/**
 * TODO documentation
 */
bool find(std::vector<int>& vec, int value);

/**
 * TODO documentation
 */
bool find(const Ipv4AddressVector& vec, Ipv4Address addr);    // use TEMPLATE

/**
 * TODO documentation
 */
void append(std::vector<int>& dest, const std::vector<int>& src);

/**
 * TODO documentation
 */
int find(const EroVector& ERO, Ipv4Address node);

/**
 * XXX function appears to be unused
 */
cModule *getPayloadOwner(cPacket *msg);

//void prepend(EroVector& dest, const EroVector& src, bool reverse);

} // namespace inet

#endif // ifndef __INET_UTILS_H

