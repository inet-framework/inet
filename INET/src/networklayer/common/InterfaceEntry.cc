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
// License along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <algorithm>
#include <sstream>

#include "InterfaceEntry.h"
#include "InterfaceTable.h"


InterfaceEntry::InterfaceEntry()
{
    ownerp = NULL;

    nwLayerGateIndex = -1;
    nodeOutputGateId = -1;
    nodeInputGateId = -1;
    peernamid = -1;

    mtu = 0;

    down = false;
    broadcast = false;
    multicast = false;
    pointToPoint= false;
    loopback = false;
    datarate = 0;

    ipv4data = NULL;
    ipv6data = NULL;
    protocol3data = NULL;
    protocol4data = NULL;
}

std::string InterfaceEntry::info() const
{
    std::stringstream out;
    out << (!ifname.empty() ? getName() : "*");
    if (getNetworkLayerGateIndex()==-1)
        out << "  on:-";
    else
        out << "  on:nwLayer.ifOut[" << getNetworkLayerGateIndex() << "]";
    out << "  MTU:" << getMTU();
    if (isDown()) out << " DOWN";
    if (isBroadcast()) out << " BROADCAST";
    if (isMulticast()) out << " MULTICAST";
    if (isPointToPoint()) out << " POINTTOPOINT";
    if (isLoopback()) out << " LOOPBACK";
    out << "  macAddr:";
    if (getMacAddress().isUnspecified())
        out << "n/a";
    else
        out << getMacAddress();

    if (ipv4data)
        out << " " << ((cPolymorphic*)ipv4data)->info(); // Khmm...
    if (ipv6data)
        out << " " << ((cPolymorphic*)ipv6data)->info(); // Khmm...
    if (protocol3data)
        out << " " << protocol3data->info();
    if (protocol4data)
        out << " " << protocol4data->info();
    return out.str();
}

std::string InterfaceEntry::detailedInfo() const
{
    std::stringstream out;
    out << "name:" << (!ifname.empty() ? getName() : "*");
    if (getNetworkLayerGateIndex()==-1)
        out << "  on:-";
    else
        out << "  on:nwLayer.ifOut[" << getNetworkLayerGateIndex() << "]";
    out << "MTU: " << getMTU() << " \t";
    if (isDown()) out << "DOWN ";
    if (isBroadcast()) out << "BROADCAST ";
    if (isMulticast()) out << "MULTICAST ";
    if (isPointToPoint()) out << "POINTTOPOINT ";
    if (isLoopback()) out << "LOOPBACK ";
    out << "\n";
    out << "  macAddr:";
    if (getMacAddress().isUnspecified())
        out << "n/a";
    else
        out << getMacAddress();
    out << "\n";
    if (ipv4data)
        out << " " << ((cPolymorphic*)ipv4data)->info() << "\n"; // Khmm...
    if (ipv6data)
        out << " " << ((cPolymorphic*)ipv6data)->info() << "\n"; // Khmm...
    if (protocol3data)
        out << " " << protocol3data->info() << "\n";
    if (protocol4data)
        out << " " << protocol4data->info() << "\n";

    return out.str();
}

void InterfaceEntry::configChanged()
{
    if (ownerp)
        ownerp->interfaceConfigChanged(this);
}

void InterfaceEntry::stateChanged()
{
    if (ownerp)
        ownerp->interfaceStateChanged(this);
}



