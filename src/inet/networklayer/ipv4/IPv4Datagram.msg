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


cplusplus {{
#include "inet/common/TLVOption.h"
#include "inet/networklayer/contract/ipv4/IPv4Address.h"
#include "inet/networklayer/common/IPProtocolId_m.h"
}}

namespace inet;

class noncobject TLVOptionBase;
class noncobject TLVOptions;

class noncobject IPv4Address;

enum IPProtocolId;

cplusplus {{
// default IPv4 header length: 20 bytes
const int IP_HEADER_BYTES = 20;

// maximum IPv4 header length (base+options): 60 = 4 * 15 bytes
const int IP_MAX_HEADER_BYTES = 60;

// option type fields
const unsigned char IPOPTION_COPY_MASK   = 0x80;
const unsigned char IPOPTION_CLASS_MASK  = 0x60;
const unsigned char IPOPTION_NUMBER_MASK = 0x1F;

// option entry number
const unsigned int MAX_IPADDR_OPTION_ENTRIES = 9;
const unsigned int MAX_TIMESTAMP_OPTION_ENTRIES = 4;

}}


//
// IPv4 options class
//
enum IPOptionClass
{
    IPOPTION_CLASS_CONTROL = 0;
    IPOPTION_CLASS_RESERVED = 1;
    IPOPTION_CLASS_DEBUGGING = 2;
    IPOPTION_CLASS_RESERVED2 = 3;
};

//
// IPv4 option types
//
enum IPOption
{
    IPOPTION_END_OF_OPTIONS = 0;
    IPOPTION_NO_OPTION = 1;
    IPOPTION_STREAM_ID = 8;

    IPOPTION_TIMESTAMP = 68;

    IPOPTION_SECURITY = 130;
    IPOPTION_LOOSE_SOURCE_ROUTING = 131;
    IPOPTION_RECORD_ROUTE = 136;
    IPOPTION_STRICT_SOURCE_ROUTING = 137;
    IPOPTION_ROUTER_ALERT = 148;
};

//
// The timestamp flag uses the same numeric values as the IPv4 Protocol
//
enum TimestampFlag
{
    IP_TIMESTAMP_TIMESTAMP_ONLY = 0;
    IP_TIMESTAMP_WITH_ADDRESS = 1;
    IP_TIMESTAMP_SENDER_INIT_ADDRESS = 3;
}

class IPv4Option extends TLVOptionBase
{
}

class IPv4OptionNop extends IPv4Option
{
    type = IPOPTION_NO_OPTION;
    length = 1;
}

class IPv4OptionEnd extends IPv4Option
{
    type = IPOPTION_END_OF_OPTIONS;
    length = 1;
}

//
// Option structure: Record Route
//
class IPv4OptionRecordRoute extends IPv4Option
{
    // type = IPOPTION_RECORD_ROUTE;
    // type = IPOPTION_LOOSE_SOURCE_ROUTING
    // type = IPOPTION_STRICT_SOURCE_ROUTING
    // length = 3 + 4 * getRecordAddressArraySize();
    short nextAddressIdx;
    // pointer = 4 + 4 * nextAddressIdx
    IPv4Address recordAddress[];    // max size is 9
}

//
// Option structure: Timestamp
//
class IPv4OptionTimestamp extends IPv4Option
{
    type = IPOPTION_TIMESTAMP;
    // length = 4 + 4 * getRecordAddressArraySize() + 4 * getRecordTimestampArraySize();
    int flag @enum(TimestampFlag);
    short overflow;
    short nextIdx;
    // pointer = 5 + (flag == IP_TIMESTAMP_TIMESTAMP_ONLY ? 4 : 8) * nextIdx

    // use either up to 4 addresses with timestamps or
    // only up to 9 timestamps, according to the flag
    IPv4Address recordAddress[];
    simtime_t recordTimestamp[];
}

//
// Option Structure: Stream ID
//
class IPv4OptionStreamId extends IPv4Option
{
    type = IPOPTION_STREAM_ID;
    length = 4;
    short streamId;
}

class IPv4OptionUnknown extends IPv4Option
{
    // type = any
    // length = 2 + getBytesArraySize()
    uint8_t bytes[];
}


//
// Represents an IPv4 datagram.
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
packet IPv4Datagram
{
    @customize(true);
    @fieldNameSuffix("_var"); //TODO remove when OMNeT++ 4.x compatibility is no longer required

    short version = 4;
    short headerLength = IP_HEADER_BYTES;

    IPv4Address srcAddress;
    IPv4Address destAddress;

    int transportProtocol @enum(IPProtocolId) = IP_PROT_NONE;
    short timeToLive;
    int identification;
    bool moreFragments;
    bool dontFragment;
    int fragmentOffset;         // bytes, must be multiple of 8
    int totalLengthField = -1;       // -1 means: getTotalLengthField() uses getByteLength()
    unsigned char typeOfService;  // ToS byte
    abstract int diffServCodePoint;  // maps to bits 0-5 of ToS
    abstract int explicitCongestionNotification;  // maps to bits 6-7 of ToS

    TLVOptions options; // array of option pointers, option pointers owned by datagram
}

