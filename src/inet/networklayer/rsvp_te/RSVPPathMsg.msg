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
import inet.networklayer.contract.ipv4.Ipv4Address;
import inet.networklayer.rsvp_te.IntServ;
import inet.networklayer.rsvp_te.RsvpPacket;

cplusplus {{
#include "inet/networklayer/rsvp_te/RsvpPacket.h"
}}

namespace inet;

//
// FIXME missing documentation
//
class RsvpPathMsg extends RsvpPacket
{
    @customize;
    RsvpHopObj hop;
    LabelRequestObj label_request;
    SenderDescriptor_t sender_descriptor;
    EroVector ERO;
    int color;

    rsvpKind = PATH_MESSAGE;
}

//
// FIXME missing documentation
//
class RsvpPathTear extends RsvpPacket
{
    @customize;
    RsvpHopObj hop;
    SenderTemplateObj senderTemplate;
    bool force;
    rsvpKind = PTEAR_MESSAGE;
}


//
// FIXME missing documentation
//
class RsvpPathError extends RsvpPacket
{
    @customize;
    Ipv4Address errorNode;
    int errorCode;
    SenderDescriptor_t sender_descriptor;

    rsvpKind = PERROR_MESSAGE;
}

