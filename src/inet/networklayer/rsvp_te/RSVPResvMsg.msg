//
// (C) 2005 Vojtech Janota
// (C) 2003 Xuan Thang Nguyen
//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//

import inet.common.INETDefs;
import inet.networklayer.contract.ipv4.IPv4Address;

cplusplus {{
#include "inet/networklayer/rsvp_te/IntServ.h"
#include "inet/networklayer/rsvp_te/RSVPPacket.h"
}}

namespace inet;

struct RsvpHopObj_t;
class noncobject FlowDescriptorVector;
class RSVPPacket extends cObject;

//
// FIXME missing documentation
//
class RSVPResvMsg extends RSVPPacket
{
    @customize(true);
    RsvpHopObj_t hop;
    FlowDescriptorVector flowDescriptor;
    rsvpKind = RESV_MESSAGE;
}

//
// FIXME missing documentation
//
class RSVPResvTear extends RSVPPacket
{
    @customize(true);
    RsvpHopObj_t hop;
    FlowDescriptorVector flowDescriptor;
    rsvpKind = RTEAR_MESSAGE;
}

//
// FIXME missing documentation
//
class RSVPResvError extends RSVPPacket
{
    @customize(true);
    RsvpHopObj_t hop;
    IPv4Address errorNode;
    int errorCode;
    rsvpKind = RERROR_MESSAGE;
}
