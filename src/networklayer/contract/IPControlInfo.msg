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

class noncobject IPAddress;

cplusplus {{
#include "IPAddress.h"
#include "IPProtocolId_m.h"
}}



//
// Control information for sending/receiving packets over \IP.
//
// To send a packet over \IP, fill in an IPControlInfo object,
// attach it to the packet with the C++ method setControlInfo(),
// the send it to the IP module.
//
// When sending, the following fields are required:
// - protocol: a value from IPProtocolId
// - destAddr
//
// Optional fields:
// - srcAddr: it will be set to the address of the outgoing interface
// - diffServCodePoint: used in DS_Field (RFC 2474) instead of TOS (default: 0)
// - timeToLive: default defined as module parameter
// - dontFragment: default: false
//
// Not supported:
// - options: \IP Options currently not used
// - Identifier is currently always chosen by the \IP layer
//
// When IP delivers a packet to higher layers, it also attaches an IPControlInfo 
// to it. It fills in the following fields: 
//  - srcAddr, destAddr, protocol, diffServCodePoint: values from the original datagram
//  - interfaceId: the interface on which the datagram arrived, or -1 if it was
//    created locally
//
// IP also puts the original datagram object into the control info, because
// it may be needed by higher layers (for example, by ICMP for error reporting).
//
class IPControlInfo
{
    @customize(true);
    IPAddress destAddr;   // destination IP address
    IPAddress srcAddr;    // source IP address
    int interfaceId = -1; // interface on which the datagram was received, or
                          // should be sent (see InterfaceTable)
    short protocol enum(IPProtocolId);  // encapsulated protocol
    unsigned char diffServCodePoint;  // for QoS
    short timeToLive;     // maximum hop count
    bool dontFragment;    // "don't fragment" bit
}


//
// Control info attached to packets sent from IP to ARP.
//
// Next hop address is used on a LAN to determine the MAC destination
// address (and it may be used on other multicast networks for similar
// addressing purpose).
//
class IPRoutingDecision
{
    int interfaceId = -1; // interface on which dgram should be sent (see InterfaceTable)
    IPAddress nextHopAddr;
}


