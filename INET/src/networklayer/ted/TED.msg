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


cplusplus {{
#include "IPAddress.h"

#define LINK_STATE_MESSAGE      1

typedef std::vector<struct TELinkStateInfo> TELinkStateInfoVector;
}}


class noncobject IPAddress;

//
// Represents a link in the TED and in LinkStateMsg packets.
//
struct TELinkStateInfo
{
    IPAddress advrouter; // routerId of originator of this link state entry
    IPAddress linkid;    // identifies link within advrouter; set to peer's address
    IPAddress local;     // address of local interface of this link
    IPAddress remote;    // address of "gateway" to peer
    double metric;       // link metric
    double MaxBandwidth; // maximum bandwidth (bps)
    double UnResvBandwidth[8]; // unreserved bandwidths --FIXME indexed by what?

    simtime_t timestamp;    // time of originating this entry
    unsigned int sourceId;  // FIXME looks like this is the same as advrouter -- really needed?
    unsigned int messageId; // id assigned to originating LinkStateMsg (FIXME or?)
    bool state;             // false = down, true = up
}


//
// Data structure supplied with NF_TED_CHANGE NotificationBoard notifications.
// This triggers the link state protocol to send out up-to-date link state info
// about the given links.
//
class TEDChangeInfo
{
    int tedLinkIndices[];
}


