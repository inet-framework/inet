//
// (C) 2005 Vojtech Janota
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

namespace inet;

cplusplus {{
typedef std::vector<struct TeLinkStateInfo> TeLinkStateInfoVector;
}}

//
// Represents a link in the ~Ted and in ~LinkStateMsg packets.
//
struct TeLinkStateInfo
{
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


