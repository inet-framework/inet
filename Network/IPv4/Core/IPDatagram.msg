//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


//import "IPInterfacePacket.msg";

enum IPProtocolFieldId;

class noncobject IPAddress;

cplusplus {{

#include "IPAddress.h"
#include "IPInterfacePacket.h"   // enum IPProtocolFieldId

// default IP header length: 20 bytes
const int DEFAULT_HEADER_LENGTH = 20;

// maximum IP header length: 64 bytes (TO BE CHECKED)
const int MAX_HEADER_LENGTH = 64;

// option entry number
const int MAX_IPADDR_OPTION_ENTRIES = 9;
const int MAX_TIMESTAMP_OPTION_ENTRIES = 4;

}};


//
// IP options. Beware: these are not the "official" codes!!!
//
enum IPOption
{
    IPOPTION_NO_OPTION = 0;
    IPOPTION_RECORD_ROUTE = 1;
    IPOPTION_TIMESTAMP = 2;
    IPOPTION_LOOSE_SOURCE_ROUTING = 3;
    IPOPTION_STRICT_SOURCE_ROUTING = 4;
};

//
// The timestamp flag uses the same numeric values as the IP Protocol
//
enum TimestampFlag
{
    IP_TIMESTAMP_TIMESTAMP_ONLY = 0;
    IP_TIMESTAMP_WITH_ADDRESS = 1;
    IP_TIMESTAMP_SENDER_INIT_ADDRESS = 3;
};


//
// Option structure: Record Route
//
class IPRecordRouteOption
{
    fields:
        IPAddress recordAddress[MAX_IPADDR_OPTION_ENTRIES];
        short nextAddressPtr;
};

//
// Option structure: Timestamp
//
class IPTimestampOption
{
    fields:
        int flag enum(TimestampFlag);
        short overflow;
        short nextAddressPtr;

        // use either up to 4 addresses with timestamps or
        // only up to 9 timestamps, according to the flag
        IPAddress recordAddress[MAX_TIMESTAMP_OPTION_ENTRIES];
        simtime_t recordTimestamp[MAX_IPADDR_OPTION_ENTRIES];
};

//
// Option Structure: Source Routing
//
class IPSourceRoutingOption
{
    fields:
        IPAddress recordAddress[MAX_IPADDR_OPTION_ENTRIES];
        short nextAddressPtr;
        short lastAddressPtr;
};


//
//  used existing fields:
//  length () / setLength(err)  in bits
//  hasBitError() / setBitError()
//  protocol() / setProtocol(short p) is always PR_IP
//  kind() always set to MK_PACKET
//  timestamp() / setTimestamp (simtime) used in timestamp option
//
//  void *context / contextPointer() / setContextPointer (*void)
//  currently optional
//
//  transportProtocol: use of enumeration in protocol.h?
//    ICMP?
//
//  additional length fields defined in this class
//  use the unit bytes (totalLength()=length()/8 and header_length) or
//  8 bytes (fragment_offset)
//
//# FIXME add method lengthBytes() {return length()/8;} to some base class
//
class IPDatagram extends cPacket
{
    properties:
        omitGetVerb = true;
    fields:
        short version = 4;
        short headerLength = DEFAULT_HEADER_LENGTH;  // FIXME set automatically!
        unsigned char diffServCodePoint;

        // FIXME protocol: PR_IP

        int fragmentId;
        bool moreFragments;
        bool dontFragment;
        int fragmentOffset;

        short timeToLive;
        int transportProtocol enum(IPProtocolFieldId) = IP_PROT_NONE;  // FIXME needed?
        IPAddress srcAddress;
        IPAddress destAddress;

        // option fields in IP header -- FIXME only one of the option fields can exist at a time
        int optionCode enum(IPOption) = IPOPTION_NO_OPTION;  // FIXME modify header length when options are present
        IPRecordRouteOption recordRoute;
        IPTimestampOption timestampOption;
        IPSourceRoutingOption sourceRoutingOption; // optionCode determines if strict or loose source routing

        // internal fields
        int outputPort = -1;
        int inputPort = -1;
};

//
// Some old code regarding options and header length
//
// header length is always set automatically
//short headerLength() { return header_length; }
//
// fields for options
//IPOption optionType() { return option_code; }
//
//    if no option exists, return NULL,
//    otherwise, return pointer to appropriate structure;
//    conversion needs to be done by calling function */
//void *optionField();

// existing option is overwritten when setXXXOption is called
//void setRecordRoute (const RecordRouteOptionField&);
//void setTimestampOption (const TimestampOptionField&);
//void setSourceRoutingOption(bool looseSourceRouting, const SourceRoutingOptionField&);
//
// private function
//void IPDatagram::setOptionHeaderLength()
//{
//
//	// HeaderLength is set automatically to maximum with option
//	setLength(length() - headerLength()*8);
//	header_length = MAX_HEADER_LENGTH;
//	setLength(length() + headerLength()*8);
//}
//
