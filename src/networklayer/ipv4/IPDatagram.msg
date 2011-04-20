//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004, 2009 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//


class noncobject IPAddress;

enum IPProtocolId;

cplusplus {{
#include "IPAddress.h"
#include "IPProtocolId_m.h"

// default IP header length: 20 bytes
const int IP_HEADER_BYTES = 20;

// maximum IP header length (base+options): 60 = 4 * 15 bytes
const int IP_MAX_HEADER_BYTES = 60;

// option entry number
const unsigned int MAX_IPADDR_OPTION_ENTRIES = 9;
const unsigned int MAX_TIMESTAMP_OPTION_ENTRIES = 4;
}}


//
// IP options class
//
enum IPOptionClass
{
    IPOPTION_CLASS_CONTROL = 0;
    IPOPTION_CLASS_RESERVED = 1;
    IPOPTION_CLASS_DEBUGGING = 2;
    IPOPTION_CLASS_RESERVED2 = 3;
};

//
// IP option numbers
//
enum IPOption
{
    IPOPTION_END_OF_OPTIONS = 0;
    IPOPTION_NO_OPTION = 1;
    IPOPTION_SECURITY = 2;
    IPOPTION_LOOSE_SOURCE_ROUTING = 3;
    IPOPTION_TIMESTAMP = 4;
    IPOPTION_RECORD_ROUTE = 7;
    IPOPTION_STREAM_ID = 8;
    IPOPTION_STRICT_SOURCE_ROUTING = 9;
    IPOPTION_ROUTER_ALERT = 20;
};


//
// The timestamp flag uses the same numeric values as the IP Protocol
//
enum TimestampFlag
{
    IP_TIMESTAMP_TIMESTAMP_ONLY = 0;
    IP_TIMESTAMP_WITH_ADDRESS = 1;
    IP_TIMESTAMP_SENDER_INIT_ADDRESS = 3;
}


//
// Option structure: Record Route
//
class IPRecordRouteOption
{
    IPAddress recordAddress[MAX_IPADDR_OPTION_ENTRIES];
    short nextAddressPtr;
}

//
// Option structure: Timestamp
//
class IPTimestampOption
{
    int flag enum(TimestampFlag);
    short overflow;
    short nextAddressPtr;

    // use either up to 4 addresses with timestamps or
    // only up to 9 timestamps, according to the flag
    IPAddress recordAddress[MAX_TIMESTAMP_OPTION_ENTRIES];
    simtime_t recordTimestamp[MAX_IPADDR_OPTION_ENTRIES];
}

//
// Option Structure: Source Routing
//
class IPSourceRoutingOption
{
    IPAddress recordAddress[MAX_IPADDR_OPTION_ENTRIES];
    short nextAddressPtr;
    short lastAddressPtr;
}


//
// Represents an IP datagram.
//
// Uses the following cPacket fields:
//    - getByteLength() / setByteLength() to represent total datagram length
//    - hasBitError() / setBitError() to represent datagram correctness
//    - getTimestamp() / setTimestamp (simtime) used in timestamp option
//
// Additional length fields defined in this class are in bytes.
//
// Only one of the option fields can exist at a time.
//
packet IPDatagram
{
    short version = 4;
    short headerLength = IP_HEADER_BYTES;

    IPAddress srcAddress;
    IPAddress destAddress;

    int transportProtocol enum(IPProtocolId) = IP_PROT_NONE;
    short timeToLive;
    int identification;
    bool moreFragments;
    bool dontFragment;
    int fragmentOffset;
    unsigned char diffServCodePoint;

    int optionCode enum(IPOption) = IPOPTION_NO_OPTION;  //#FIXME modify header length when options are present
                                                         //#FIXME also: the RFC says that more than one IP-Option is allowed
    IPRecordRouteOption recordRoute;
    IPTimestampOption timestampOption;
    IPSourceRoutingOption sourceRoutingOption; // optionCode determines if strict or loose source routing
    
    // Virtual value, used by the new fragment mechanism
    unsigned int totalPayloadLength;
}


