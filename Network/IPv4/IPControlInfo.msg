//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


enum IPProtocolFieldId;
class noncobject IPAddress;

cplusplus {{
#include "IPAddress.h"
#include "IPDatagram_m.h"
}}


//
// Control information for sending/receiving packets over IP.
//
// To send a packet over IP, fill in an IPControlInfo object,
// attach it to the packet with the C++ method setControlInfo(),
// the send it to the IP module.
//
// When IP delivers packets to higher layers, it also attaches
// similar IPControlInfo objects to them.
//
// When sending, the following fields are required:
// - protocol: a value from IPProtocolFieldId
// - destAddr
//
// Optional fields:
// - srcAddr: it will be set to the address of the outgoing interface
// - codepoint: used in DS_Field (RFC 2474) instead of TOS (default: 0)
// - timeToLive: default defined as module parameter
// - dontFragment: default: false
//
// Not supported:
// - options: IP Options currently not used
// - Identifier is currently always chosen by IP layer
//
class IPControlInfo
{
    properties:
        omitGetVerb = true;
    fields:
        IPAddress destAddr;
        IPAddress srcAddr;
        short protocol enum(IPProtocolFieldId);
        unsigned char diffServCodePoint;
        short timeToLive;
        bool dontFragment;
};


//
// Control info used for routing purposes within IPProcessing.
// OutputPort and nextHopAddr are filled in by IPRouting.
//
// Next hop address is used on a LAN to determine the MAC destination
// address (and it may be used on other multicast networks for similar
// addressing purpose).
//
class IPRoutingDecision
{
    properties:
        omitGetVerb = true;
    fields:
        int inputPort = -1;
        int outputPort = -1;
        IPAddress nextHopAddr;
};


