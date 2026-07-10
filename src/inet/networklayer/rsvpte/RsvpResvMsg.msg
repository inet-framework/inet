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

    // RFC 2205 Section 3.1.1 TIME_VALUES C-Type 1: refresh period R, in
    // milliseconds. Same representation-gap fix as RsvpPathMsg.refreshPeriod
    // (Workstream E, Phase 2 commit 3) -- B6's computeResvMessageLength()
    // already budgets TIME_VALUES_OBJECT_BYTES (8) for Resv, but no field
    // existed to carry the value. Defaulted to RsvpTe.ned's refreshInterval
    // default (30s); not wired to the module parameter (refreshResv()/RsvpTe.cc
    // are unchanged).
    int refreshPeriod = 30000;

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
    FlowDescriptorVector flowDescriptor;
    rsvpKind = RERROR_MESSAGE;
}

cplusplus(RsvpResvError) {{
  public:
    inline Ipv4Address getNHOP() { return getHop().Next_Hop_Address; }
    inline Ipv4Address getLIH() { return getHop().Logical_Interface_Handle; }
}}
