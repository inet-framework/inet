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
#define LINK_NOTIFY_MESSAGE     2

typedef std::vector<struct TELinkStateInfo> TELinkStateInfoVector;
}}

class noncobject IPAddress;


//
// FIXME missing documentation
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
// FIXME missing documentation
//
struct TELink
{
    fields:
        IPAddress advrouter;
        IPAddress linkid;
};


//
// FIXME missing documentation
//
message TEDMsg
{
    fields:
        int command = 0;
};


//
// FIXME missing documentation
//
message LinkStateMsg extends TEDMsg
{
    fields:
        TELinkStateInfo linkInfo[];

        bool request = false;
        bool ack = false;

        int command = LINK_STATE_MESSAGE;
};


//
// FIXME missing documentation
//
message LinkNotifyMsg extends TEDMsg
{
    fields:
        TELink link[];

        int command = LINK_NOTIFY_MESSAGE;
};


