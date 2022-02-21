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
    double metric;       // link metric
    double MaxBandwidth; // maximum bandwidth (bps)
    double UnResvBandwidth[8]; // unreserved bandwidths --FIXME indexed by what?

    simtime_t timestamp;    // time of originating this entry
    unsigned int sourceId;  // FIXME looks like this is the same as advrouter -- really needed?
    unsigned int messageId; // id assigned to originating ~LinkStateMsg (FIXME or?)
    bool state;             // false = down, true = up
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
