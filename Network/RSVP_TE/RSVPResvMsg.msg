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
#include "IntServ.h"
#include "RSVPPacket.h"
}}

struct RsvpHopObj_t;

class noncobject IPAddress;
class noncobject FlowDescriptorVector;

class RSVPPacket;

//
// FIXME missing documentation
//
message RSVPResvMsg extends RSVPPacket
{
    properties:
        customize=true;
    fields:
        RsvpHopObj_t hop;
        FlowDescriptorVector flowDescriptor;
        int rsvpKind = RESV_MESSAGE;
}

//
// FIXME missing documentation
//
message RSVPResvTear extends RSVPPacket
{
    properties:
        customize=true;
    fields:
        RsvpHopObj_t hop;
        FlowDescriptorVector flowDescriptor;
        int rsvpKind = RTEAR_MESSAGE;
};

//
// FIXME missing documentation
//
message RSVPResvError extends RSVPPacket
{
    properties:
        customize=true;
    fields:
        RsvpHopObj_t hop;
        IPAddress errorNode;
        int errorCode;
        int rsvpKind = RERROR_MESSAGE;
};


