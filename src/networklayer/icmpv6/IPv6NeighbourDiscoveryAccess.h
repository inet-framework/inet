//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, see <http://www.gnu.org/licenses/>.
//


#ifndef IPV6NEIGHBOURDISCOVERYACCESS_H
#define IPV6NEIGHBOURDISCOVERYACCESS_H

#include <omnetpp.h>
#include "ModuleAccess.h"
#include "IPv6NeighbourDiscovery.h"


/**
 * Gives access to IPv6NeighbourDiscovery
 */
class INET_API IPv6NeighbourDiscoveryAccess : public ModuleAccess<IPv6NeighbourDiscovery>
{
    public:
        IPv6NeighbourDiscoveryAccess() : ModuleAccess<IPv6NeighbourDiscovery>("neighbourDiscovery") {}
};

#endif
