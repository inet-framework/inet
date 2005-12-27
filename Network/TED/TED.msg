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
    fields:
        IPAddress advrouter; // routerId of originator of this link state entry
        IPAddress linkid;    // FIXME what's this?
        IPAddress local;     // address of local interface of this link
        IPAddress remote;    // address of remote interface of this link
        double metric;       // link metric
        double MaxBandwidth; // maximum bandwidth (bps)
        double UnResvBandwidth[8]; // unreserved bandwidths --FIXME indexed by what?

        double timestamp;    // time of originating this entry
        unsigned int sourceId;  // FIXME what's this?
        unsigned int messageId; // id assigned to originating LinkStateMsg (FIXME or?)
        bool state;          // FIXME values?
};


//
// Packet for disseminating link state information (TELinkStateInfo[]) by the 
// LinkStateRouting module.
//
message LinkStateMsg
{
    fields:
        TELinkStateInfo linkInfo[];

        bool request = false; // if true, receiver is expected to send back its own link state database to the sender --FIXME really needed?
        // bool ack = false; -- apparently unused, removed -- TODO check with Vojta

        int command = LINK_STATE_MESSAGE;
};


//
// Data structure supplied with NF_TED_CHANGE NotificationBoard notifications.
// This triggers the link state protocol to send out up-to-date link state info
// about the given links.
//
class TEDChangeInfo
{
    fields:
        int tedLinkIndices[];
};


