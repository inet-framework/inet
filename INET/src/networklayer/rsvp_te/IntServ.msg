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


//
// This file defines data structures used in IntServ services
//

cplusplus {{
#include "IPAddress.h"
typedef std::vector<IPAddress> IPAddressVector;
typedef std::vector<struct EroObj_t> EroVector;
typedef std::vector<struct FlowDescriptor_t> FlowDescriptorVector;
}}


class noncobject IPAddress;

class noncobject IPAddressVector;

class noncobject EroVector;

//
// Intserv/RSVP: Session Structure
//
struct SessionObj_t
{
    int Tunnel_Id;
    int Extended_Tunnel_Id;
    IPAddress DestAddress;
    int setupPri;
    int holdingPri;
}

//
// Intserv/RSVP: RSVP HOP Structure
//
struct RsvpHopObj_t
{
    IPAddress Next_Hop_Address;
    IPAddress Logical_Interface_Handle;
}

//
// Intserv/RSVP: Sender Template Structure
//
struct SenderTemplateObj_t
{
    IPAddress SrcAddress;
    int Lsp_Id = -1;
}


//
// Intserv/RSVP: Sender Tspec Structure
//
struct SenderTspecObj_t
{
    double req_bandwidth = 0;
}

//
// Intserv/RSVP: Sender Tspec Structure
//
struct FlowSpecObj_t extends SenderTspecObj_t
{
}

//
// Intserv/RSVP: Sender Template Structure
//
struct FilterSpecObj_t extends SenderTemplateObj_t
{
}

//
// Intserv/RSVP: Label Request Object Structure
//
struct LabelRequestObj_t
{
    // request;
    int prot;
}

//
// Intserv/RSVP: Sender Descriptor Structure
//
struct SenderDescriptor_t
{
    SenderTemplateObj_t Sender_Template_Object;
    SenderTspecObj_t Sender_Tspec_Object;
}

//
// Intserv/RSVP: Explicit Routing Object Structure
//
struct EroObj_t
{
    bool L = false;
    IPAddress node;
}

//
// Intserv/RSVP: Flow Descriptor Structure
//
struct FlowDescriptor_t
{
    FlowSpecObj_t Flowspec_Object;
    FilterSpecObj_t Filter_Spec_Object;
    IPAddressVector RRO;
    int label;
}
