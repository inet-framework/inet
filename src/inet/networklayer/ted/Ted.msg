//
// Copyright (C) 2005 Vojtech Janota
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.networklayer.contract.ipv4.Ipv4Address;

namespace inet;

cplusplus {{
typedef std::vector<struct TeLinkStateInfo> TeLinkStateInfoVector;
}}

//
// Represents a link in the ~Ted and in ~LinkStateMsg packets.
//
struct TeLinkStateInfo
{
    @packetData;
    Ipv4Address advrouter; // routerId of originator of this link state entry
    Ipv4Address linkid;    // identifies link within advrouter; set to peer's address
    Ipv4Address local;     // address of local interface of this link
    Ipv4Address remote;    // address of "gateway" to peer
    double metric;       // link metric (IGP metric)
    double MaxBandwidth; // maximum bandwidth (bps)
    double UnResvBandwidth[8]; // unreserved bandwidths --FIXME indexed by what?

    simtime_t timestamp;    // time of originating this entry
    unsigned int sourceId;  // FIXME looks like this is the same as advrouter -- really needed?
    unsigned int messageId; // id assigned to originating ~LinkStateMsg (FIXME or?)
    bool state;             // false = down, true = up

    // --- TE attribute extension (D3) -------------------------------------
    // These attributes are not consumed by anything in this workstream; they
    // are populated (see Ted::initializeTED()'s "linkAttributes" XML param)
    // and exposed via query helpers (Ted::getTeMetric(), Ted::matchesAffinity())
    // for CSPF-with-affinities (Workstream C6) and TI-LFA (Workstream F2) to
    // consume later. Field names deliberately mirror RFC 3630 sub-TLVs.
    double teMetric = 0;    // TE metric (RFC 3630 Section 2.5.5); 0 means "fall back to `metric`" -- see Ted::getTeMetric()
    uint32_t adminGroup = 0; // administrative group / color bits (RFC 3630 Section 2.5.9); 0 = no group membership
    // Shared Risk Link Group ids for this link (RFC 3630-style). `struct` msg
    // types (unlike `class` ones) don't support true dynamic arrays, so this
    // is a fixed-size array capped at Ted::MAX_SRLGS (keep the literal below
    // in sync with that constant -- see Ted.h) plus an explicit valid-count;
    // only srlgs[0..srlgsCount) are meaningful, the rest are always 0.
    unsigned int srlgsCount = 0; // number of valid entries in srlgs[] (0..MAX_SRLGS)
    uint32_t srlgs[8];           // MAX_SRLGS = 8; unused slots are 0
}

//
// Data structure supplied with NF_TED_CHANGE signal.
// This triggers the link state protocol to send out up-to-date link state info
// about the given links.
//
class TedChangeInfo extends cObject
{
    int tedLinkIndices[];
}
