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
#include <omnetpp.h>
#include "IntServ.h"

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
bool find(const IPAddressVector& vec, IPAddress addr); // use TEMPLATE

/**
 * TODO documentation
 */
void append(std::vector<int>& dest, const std::vector<int>& src);

/**
 * TODO documentation
 */
int find(const EroVector& ERO, IPAddress node);

/**
 * XXX function appears to be unused
 */
cModule *getPayloadOwner(cPacket *msg);

//void prepend(EroVector& dest, const EroVector& src, bool reverse);


#endif
