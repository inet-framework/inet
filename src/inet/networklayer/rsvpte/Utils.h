//
// Copyright (C) 2005 Vojtech Janota
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_UTILS_H
#define __INET_UTILS_H

#include <vector>

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
void append(std::vector<int>& dest, const std::vector<int>& src);

/**
 * TODO function appears to be unused
 */
cModule *getPayloadOwner(cPacket *msg);

//void prepend(EroVector& dest, const EroVector& src, bool reverse);

} // namespace inet

#endif

