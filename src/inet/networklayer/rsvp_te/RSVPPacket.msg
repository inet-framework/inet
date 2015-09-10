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
#include "inet/networklayer/rsvp_te/IntServ_m.h"

#define PATH_MESSAGE   1
#define RESV_MESSAGE   2
#define PTEAR_MESSAGE  3
#define RTEAR_MESSAGE  4
#define PERROR_MESSAGE 5
#define RERROR_MESSAGE 6
#define HELLO_MESSAGE   7
}}

namespace inet;

struct SessionObj_t;

struct RsvpHopObj_t;

struct SenderTemplateObj_t;

struct SenderTspecObj_t;

struct FlowSpecObj_t;

struct FilterSpecObj_t;

struct LabelRequestObj_t;

struct SenderDescriptor_t;

struct FlowDescriptor_t;

struct EroObj_t;



//
// RSVP message common part
//
packet RSVPMessage
{
    @fieldNameSuffix("_var"); //TODO remove when OMNeT++ 4.x compatibility is no longer required
    int rsvpKind;
}

//
// FIXME missing documentation
//
packet RSVPPacket extends RSVPMessage
{
    @customize(true);
    SessionObj_t session;
    bool checksumValid = true;
}
