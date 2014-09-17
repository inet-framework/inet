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

cplusplus {{
#include "inet/networklayer/contract/ipv4/IPv4Address.h"
#include "inet/networklayer/common/IPProtocolId_m.h"
#include "inet/linklayer/common/MACAddress.h"
}}

namespace inet;

enum IPProtocolId;

class noncobject IPv4Address;
class noncobject MACAddress;

//
// Control information for sending/receiving packets over IPv4.
//
// To send a packet over IPv4, fill in an ~IPv4ControlInfo object,
// attach it to the packet with the C++ method setControlInfo(),
// the send it to the ~IPv4 module.
//
// When sending, the following fields are required:
// - protocol: a value from ~IPProtocolId
// - destAddr
//
// Optional fields:
// - srcAddr: it will be set to the address of the outgoing interface
// - typeOfService: (default: 0)
// - timeToLive: default defined as module parameter
// - dontFragment: default: false
//
// Not supported:
// - options: IPv4 Options currently not used
// - Identifier is currently always chosen by the IPv4 layer
//
// When ~IPv4 delivers a packet to higher layers, it also attaches an ~IPv4ControlInfo
// to it. It fills in the following fields:
//  - srcAddr, destAddr, protocol, TypeOfService: values from the original datagram
//  - interfaceId: the interface on which the datagram arrived, or -1 if it was
//    created locally
//
// ~IPv4 also puts the original datagram object into the control info, because
// it may be needed by higher layers (for example, by ~ICMP for error reporting).
//
class IPv4ControlInfo
{
    @customize(true);
    IPv4Address destAddr;   // destination IPv4 address
    IPv4Address srcAddr;    // source IPv4 address
    int interfaceId = -1; // interface on which the datagram was received, or
                          // should be sent (see ~InterfaceTable)
    bool multicastLoop;   // if true, sent multicast datagrams will be looped back
    short protocol @enum(IPProtocolId);  // encapsulated protocol
    unsigned char typeOfService;  // Type of Service byte (low 6 bits: DSCP)
    abstract int diffServCodePoint;  // maps to bits 0-5 of trafficClass
    abstract int explicitCongestionNotification;  // maps to bits 6-7 of trafficClass
    short timeToLive;     // maximum hop count
    bool dontFragment;    // "don't fragment" bit
}

