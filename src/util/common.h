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

#ifndef __INET_INETCOMMON_H
#define __INET_INETCOMMON_H

#include <omnetpp.h>
#include <string>

#include "IntServ.h" // IPAddressVector, EroVector
#include "IPvXAddress.h"

std::string intToString(int i);
std::string vectorToString(IPAddressVector vec);
std::string vectorToString(IPAddressVector vec, const char *delim);
std::string vectorToString(EroVector vec);
std::string vectorToString(EroVector vec, const char *delim);

EroVector routeToEro(IPAddressVector rro);
uint32 getLevel(IPvXAddress addr);

#endif


