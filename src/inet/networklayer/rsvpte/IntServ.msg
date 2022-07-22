//
// Copyright (C) 2005 Vojtech Janota
// Copyright (C) 2003 Xuan Thang Nguyen
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

//
// This file defines data structures used in IntServ services
//
// TODO *_t is reserved for C++ standard
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
    @packetData;
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
    @packetData;
    Ipv4Address Next_Hop_Address;
    Ipv4Address Logical_Interface_Handle;
}

//
// Intserv/RSVP: Sender Template Structure
//
struct SenderTemplateObj
{
    @packetData;
    Ipv4Address SrcAddress;
    int Lsp_Id = -1;
}

//
// Intserv/RSVP: Sender Tspec Structure
//
struct SenderTspecObj
{
    @packetData;
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
    @packetData;
    // request;
    int prot;
}

//
// Intserv/RSVP: Sender Descriptor Structure
//
struct SenderDescriptor_t
{
    @packetData;
    SenderTemplateObj Sender_Template_Object;
    SenderTspecObj Sender_Tspec_Object;
}

//
// Intserv/RSVP: Explicit Routing Object Structure
//
struct EroObj
{
    @packetData;
    bool L = false;
    Ipv4Address node;
}

//
// Intserv/RSVP: Flow Descriptor Structure
//
struct FlowDescriptor_t
{
    @packetData;
    FlowSpecObj Flowspec_Object;
    FilterSpecObj Filter_Spec_Object;
    Ipv4AddressVector RRO;
    int label;
}
