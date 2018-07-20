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

import inet.common.INETDefs;
import inet.networklayer.contract.ipv4.Ipv4Address;

namespace inet;

cplusplus {{
typedef std::vector<struct EroObj> EroVector;
typedef std::vector<struct FlowDescriptor_t> FlowDescriptorVector;
}}


class EroVector { @existingClass; }

class FlowDescriptorVector { @existingClass; }

//
// Intserv/RSVP: Session Structure
//
struct SessionObj
{
    int Tunnel_Id;
    int Extended_Tunnel_Id;
    Ipv4Address DestAddress;
    int setupPri;
    int holdingPri;
}

//
// Intserv/RSVP: RSVP HOP Structure
//
struct RsvpHopObj
{
    Ipv4Address Next_Hop_Address;
    Ipv4Address Logical_Interface_Handle;
}

//
// Intserv/RSVP: Sender Template Structure
//
struct SenderTemplateObj
{
    Ipv4Address SrcAddress;
    int Lsp_Id = -1;
}


//
// Intserv/RSVP: Sender Tspec Structure
//
struct SenderTspecObj
{
    double req_bandwidth = 0;
}

//
// Intserv/RSVP: Sender Tspec Structure
//
struct FlowSpecObj extends SenderTspecObj
{
}

//
// Intserv/RSVP: Sender Template Structure
//
struct FilterSpecObj extends SenderTemplateObj
{
}

//
// Intserv/RSVP: Label Request Object Structure
//
struct LabelRequestObj
{
    // request;
    int prot;
}

//
// Intserv/RSVP: Sender Descriptor Structure
//
struct SenderDescriptor_t
{
    SenderTemplateObj Sender_Template_Object;
    SenderTspecObj Sender_Tspec_Object;
}

//
// Intserv/RSVP: Explicit Routing Object Structure
//
struct EroObj
{
    bool L = false;
    Ipv4Address node;
}

//
// Intserv/RSVP: Flow Descriptor Structure
//
struct FlowDescriptor_t
{
    FlowSpecObj Flowspec_Object;
    FilterSpecObj Filter_Spec_Object;
    Ipv4AddressVector RRO;
    int label;
}
