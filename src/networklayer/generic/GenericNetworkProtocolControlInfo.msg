//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004 Andras Varga
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


enum IPProtocolId;

class noncobject Address;

cplusplus {{
#include "Address.h"
#include "IPProtocolId_m.h"
}}


//
// Control information for sending/receiving packets over the generic network protocol.
//
class GenericNetworkProtocolControlInfo  //TODO rename!!!!
{
    @customize(true);
    Address destinationAddress @getter(_getDestinationAddress);   // destination address
    Address sourceAddress @getter(_getSourceAddress);    // source address
    int interfaceId = -1; // interface on which the datagram was received, or
                          // should be sent (see ~InterfaceTable)
    short protocol enum(IPProtocolId);  // encapsulated protocol
    short hopLimit;     // maximum hop count
}


//
// Control info attached to packets sent from ~GenericNetworkProtocol to ~GlobalARP.
//
// Next hop address is used on a LAN to determine the MAC destination
// address (and it may be used on other multicast networks for similar
// addressing purpose).
//
class GenericRoutingDecision
{
    int interfaceId = -1; 
    Address nextHop;
}
