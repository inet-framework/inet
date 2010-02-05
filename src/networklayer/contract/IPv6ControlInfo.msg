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


enum IPProtocolId;

class noncobject IPv6Address;

cplusplus {{
#include "IPv6Address.h"
#include "IPProtocolId_m.h"
}}



//
// Control information for sending/receiving packets over \IPv6.
//
// To send a packet over \IPv6, fill in an IPv6ControlInfo object,
// attach it to the packet with the C++ method setControlInfo(),
// the send it to the IPv6 module.
//
// When sending, the following fields are required:
// - protocol: a value from IPProtocolId
// - destAddr
//
// Optional fields:
// - srcAddr: it will be set to the address of the outgoing interface
// - timeToLive
//
// When IPv6 delivers a packet to higher layers, it also attaches an 
// IPv6ControlInfo to it. It fills in the following fields: 
//  - srcAddr, destAddr, protocol, hopLimit: values from the original datagram
//  - interfaceId: the interface on which the datagram arrived, or -1 if it was
//    created locally
//
// IPv6 also puts the original datagram object into the control info, because
// it may be needed by higher layers.
//
class IPv6ControlInfo
{
    @customize(true);
    int protocol enum(IPProtocolId);  // transport layer protocol
    IPv6Address destAddr; // destination IPv6 address
    IPv6Address srcAddr;  // source IPv6 address
    short hopLimit;       // hop limit
    int interfaceId = -1; // interface on which the datagram was received, or
                          // should be sent (see InterfaceTable)
}


