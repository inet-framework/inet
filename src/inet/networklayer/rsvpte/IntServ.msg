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
// RFC 2210 Section 3.1 (SENDER_TSPEC) / 3.2.1 (FLOWSPEC, Controlled-Load): the
// Int-Serv token bucket is (r, b, p, m, M) -- token rate, bucket depth (burst
// size), peak rate, minimum policed unit, and maximum packet size. This model's
// admission control (RsvpTe::doCACCheck()) only ever consults "req_bandwidth"
// (the rate, r); "burst"/"peakRate"/"minPolicedUnit"/"maxPacketSize" (b/p/m/M,
// Workstream C11) are carried on the wire and propagated/recorded end to end
// (RsvpTeSerializer, PathStateBlock/ResvStateBlock) but NEVER enforced -- this
// model has no per-LSP policing/shaping (see RsvpTe.ned's doc comment). Default
// 0 for all four (a real Tspec would typically have "peakRate" >= "req_bandwidth"
// and nonzero "minPolicedUnit"/"maxPacketSize", but since nothing in this model
// validates or acts on them, an all-zero default is harmless and keeps every
// existing traffic.xml/add-session XML -- none of which sets these attributes --
// unaffected).
struct SenderTspecObj
{
    @packetData;
    double req_bandwidth = 0; // token rate [r]
    double burst = 0; // bucket depth / burst size [b]
    double peakRate = 0; // peak rate [p]
    int minPolicedUnit = 0; // minimum policed unit [m]
    int maxPacketSize = 0; // maximum packet size [M]
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
    // RFC 3209 Section 4.3.1 LABEL_REQUEST C-Type 1: 2 reserved bytes + a
    // 16-bit L3PID identifying the layer-3 protocol using the requested label
    // (0x0800 = IPv4, the only payload type this model forwards over MPLS).
    // Representation gap fix (Workstream E, Phase 2 commit 3): this field did
    // not exist before; the wire serializer needs something concrete to write
    // for the object's already-budgeted 4-byte body (see B6's
    // LABEL_REQUEST_OBJECT_BYTES in RsvpTe.cc). Not wired to any per-request
    // state -- IPv4 is the model's only payload type, so the value never varies.
    int l3pid = 0x0800;
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
