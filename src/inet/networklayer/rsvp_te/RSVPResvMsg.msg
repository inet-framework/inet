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


cplusplus {{
#include "inet/networklayer/rsvp_te/IntServ.h"
#include "inet/networklayer/rsvp_te/RSVPPacket.h"
}}

namespace inet;

struct RsvpHopObj_t;


class noncobject IPv4Address;

class noncobject FlowDescriptorVector;

class RSVPPacket;


//
// FIXME missing documentation
//
packet RSVPResvMsg extends RSVPPacket
{
    @customize(true);
    RsvpHopObj_t hop;
    FlowDescriptorVector flowDescriptor;
    int rsvpKind = RESV_MESSAGE;
}

//
// FIXME missing documentation
//
packet RSVPResvTear extends RSVPPacket
{
    @customize(true);
    RsvpHopObj_t hop;
    FlowDescriptorVector flowDescriptor;
    int rsvpKind = RTEAR_MESSAGE;
}

//
// FIXME missing documentation
//
packet RSVPResvError extends RSVPPacket
{
    @customize(true);
    RsvpHopObj_t hop;
    IPv4Address errorNode;
    int errorCode;
    int rsvpKind = RERROR_MESSAGE;
}


