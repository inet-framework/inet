//
// Copyright (C) 2005 Vojtech Janota
// Copyright (C) 2003 Xuan Thang Nguyen
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.networklayer.contract.ipv4.Ipv4Address;
import inet.networklayer.rsvpte.IntServ;
import inet.networklayer.rsvpte.RsvpPacket;

namespace inet;

//
// FIXME missing documentation
//
class RsvpResvMsg extends RsvpPacket
{
    RsvpHopObj hop;
    FlowDescriptorVector flowDescriptor;
    rsvpKind = RESV_MESSAGE;
}

cplusplus(RsvpResvMsg) {{
  public:
    inline Ipv4Address getNHOP() const { return getHop().Next_Hop_Address; }
    inline Ipv4Address getLIH() const { return getHop().Logical_Interface_Handle; }
}}

//
// FIXME missing documentation
//
class RsvpResvTear extends RsvpPacket
{
    RsvpHopObj hop;
    FlowDescriptorVector flowDescriptor;
    rsvpKind = RTEAR_MESSAGE;
}

cplusplus(RsvpResvTear) {{
  public:
    inline Ipv4Address getNHOP() { return getHop().Next_Hop_Address; }
    inline Ipv4Address getLIH() { return getHop().Logical_Interface_Handle; }
}}

//
// FIXME missing documentation
//
class RsvpResvError extends RsvpPacket
{
    RsvpHopObj hop;
    Ipv4Address errorNode;
    int errorCode;
    rsvpKind = RERROR_MESSAGE;
}

cplusplus(RsvpResvError) {{
  public:
    inline Ipv4Address getNHOP() { return getHop().Next_Hop_Address; }
    inline Ipv4Address getLIH() { return getHop().Logical_Interface_Handle; }
}}
