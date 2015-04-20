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


cplusplus {{
#include "inet/networklayer/contract/ipv6/IPv6Address.h"
#include "inet/networklayer/common/IPProtocolId_m.h"
}}

namespace inet;

enum IPProtocolId;

class noncobject IPv6Address;
class noncobject IPv6ExtensionHeaderPtr;

cplusplus {{
class IPv6ExtensionHeader;
typedef IPv6ExtensionHeader *IPv6ExtensionHeaderPtr;
std::ostream& operator<<(std::ostream& os, IPv6ExtensionHeaderPtr eh);

#ifndef WITH_IPv6
inline std::ostream& operator<<(std::ostream& os, IPv6ExtensionHeaderPtr eh) {return os;}
#endif
}}



//
// Control information for sending/receiving packets over IPv6.
//
// To send a packet over IPv6, fill in an ~IPv6ControlInfo object,
// attach it to the packet with the C++ method setControlInfo(),
// the send it to the ~IPv6 module.
//
// When sending, the following fields are required:
// - protocol: a value from ~IPProtocolId
// - destAddr
//
// Optional fields:
// - srcAddr: it will be set to the address of the outgoing interface
// - timeToLive
//
// When ~IPv6 delivers a packet to higher layers, it also attaches an
// ~IPv6ControlInfo to it. It fills in the following fields:
//  - srcAddr, destAddr, protocol, hopLimit: values from the original datagram
//  - interfaceId: the interface on which the datagram arrived, or -1 if it was
//    created locally
//
// ~IPv6 also puts the original datagram object into the control info, because
// it may be needed by higher layers.
//
class IPv6ControlInfo
{
    @customize(true);
    @fieldNameSuffix("_var"); //TODO remove when OMNeT++ 4.x compatibility is no longer required
    short protocol @enum(IPProtocolId);  // transport layer protocol
    IPv6Address destAddr; // destination IPv6 address
    IPv6Address srcAddr;  // source IPv6 address
    unsigned char trafficClass;  // Traffic Class byte (low 6 bits: DSCP; highest 2 bits: ECN)
    abstract int diffServCodePoint;  // maps to bits 0-5 of trafficClass
    abstract int explicitCongestionNotification;  // maps to bits 6-7 of trafficClass
    short hopLimit;       // hop limit
    int interfaceId = -1; // interface on which the datagram was received, or
                          // should be sent (see ~InterfaceTable)
    bool multicastLoop;   // if true, sent multicast datagrams will be looped back
    abstract IPv6ExtensionHeaderPtr extensionHeader[]; // array of extension headers, subclassed from ~IPv6ExtensionHeader
}
