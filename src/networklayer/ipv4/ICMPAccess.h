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

#ifndef __INET_ICMPACCESS_H
#define __INET_ICMPACCESS_H

//  Cleanup and rewrite: Andras Varga, 2004

#include <omnetpp.h>
#include "ModuleAccess.h"
#include "ICMP.h"


/**
 * Gives access to ICMP.
 */
class INET_API ICMPAccess : public ModuleAccess<ICMP>
{
    public:
        ICMPAccess() : ModuleAccess<ICMP>("icmp") {}
};

#endif

