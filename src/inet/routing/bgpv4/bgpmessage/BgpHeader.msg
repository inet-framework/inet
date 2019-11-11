//
// Copyright (C) 2010 Helene Lageber
// Copyright (C) 2019 OpenSim Ltd.
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

import inet.common.INETDefs;
import inet.common.Units;
import inet.common.packet.chunk.Chunk;
import inet.networklayer.contract.ipv4.Ipv4Address;
import inet.routing.bgpv4.BgpCommon;

namespace inet::bgp;

cplusplus {{
const B BGP_HEADER_OCTETS = B(19);
const B BGP_OPEN_OCTETS = B(10);
const B BGP_EMPTY_UPDATE_OCTETS = B(4); // UnfeasibleRoutesLength (2) + TotalPathAttributeLength (2)
}}

//
// Represents a BGPv4 AS_PATH path attribute (RFC 4271 Section 4.3)
//
class BgpAsPathSegment extends cObject
{
    BgpPathSegmentType type;
    uint8_t length;
    uint16_t asValue[];
}


//
// BGPv4 message types
//
enum BgpType
{
    BGP_OPEN = 1;
    BGP_UPDATE = 2;
    BGP_NOTIFICATION = 3;
    BGP_KEEPALIVE = 4;
}

//
// Represents a BGPv4 message header.
//
// Header fields modeled:
//   - Marker: 16 octets (authentication)
//   - Length: 2 octets  (total size of the message)
//   - Type: 1 octet
//
class BgpHeader extends FieldsChunk
{
    chunkLength = BGP_HEADER_OCTETS;
    uint8_t marker[16] = 0xFF;
    uint16_t totalLength = B(BGP_HEADER_OCTETS).get();
    BgpType type = static_cast<BgpType>(-1);
}

//
// Represents a BGPv4 KEEPALIVE message.
//
// KEEPALIVE messages are exchanged as often as necessary between systems
// to avoid exceeding the BGP Hold Timer (constant defined in BgpOpen).
//
class BgpKeepAliveMessage extends BgpHeader
{
    type = BGP_KEEPALIVE;
}

class BgpOptionalParameterBase
{
    @packetData;
    short parameterType;
    unsigned short parameterValueLength;
}

//TODO add accepted BgpOptionalParameter variants

class BgpOptionalParameterRaw extends BgpOptionalParameterBase
{
    char value[];
}

//
// Represents a BGPv4 OPEN message.
//
class BgpOpenMessage extends BgpHeader
{
    type = BGP_OPEN;
    chunkLength = BGP_HEADER_OCTETS + BGP_OPEN_OCTETS;
    totalLength = B(BGP_HEADER_OCTETS + BGP_OPEN_OCTETS).get();

    uint8_t version = 4;    // BGP Version, 1 octet
    uint16_t myAS;    // My Autonomous system, 2 octets
    simtime_t holdTime;    // Hold Time in seconds, 2 octets
    Ipv4Address BGPIdentifier;    // 4 octets
    unsigned short optionalParametersLength;    // 1 octet, total length (in bytes) of optionalParameters[] (if = 0, no optional parameters)
    BgpOptionalParameterBase *optionalParameter[] @owned;
}

enum BgpUpdateAttributeTypeCode
{
    ORIGIN = 1;
    AS_PATH = 2;
    NEXT_HOP = 3;
    MULTI_EXIT_DISC = 4;
    LOCAL_PREF = 5;
    ATOMIC_AGGREGATE = 6;
    AGGREGATOR = 7;
}

class BgpUpdatePathAttributes extends cObject
{
    bool optionalBit = false;    // 1 bit, Optional bit. It defines whether the attribute is optional (if set to 1) or well-known (if set to 0).
    bool transitiveBit = false;    // 1 bit, Transitive bit. It defines whether an optional attribute is transitive (if set to 1) or non-transitive (if set to 0). Transitive bit MUST be set to 1 for well-known attributes
    bool partialBit = false;    // 1 bit, Partial bit. It defines whether the information contained in the optional transitive attribute is partial (if set to 1) or complete (if set to 0).  For well-known attributes and for optional non-transitive attributes, the Partial bit MUST be set to 0.
    bool extendedLengthBit = false;    // 1 bit, Extended Length bit. It defines whether the Attribute Length is one octet (if set to 0) or two octets (if set to 1).
    unsigned short reserved = 0;    // 4 bit
    BgpUpdateAttributeTypeCode typeCode;    // 1 octet
    unsigned short length; // 2 octets or just 1 depending on the value of flags.extendedLengthBit
}

// ORIGIN (RFC4271 Section 4.3, Type code 1):
class BgpUpdatePathAttributesOrigin extends BgpUpdatePathAttributes
{
    typeCode = ORIGIN;
    optionalBit = false;
    transitiveBit = true;
    length = 1;
    BgpSessionType value;
}

// AS_PATH (RFC4271 Section 4.3, Type code 2):
class BgpUpdatePathAttributesAsPath extends BgpUpdatePathAttributes
{
    typeCode = AS_PATH;
    optionalBit = false;
    transitiveBit = true;
    length = 0;
    BgpAsPathSegment value[];
}

//NEXT_HOP (RFC4271 Section 4.3, Type code 3):
class BgpUpdatePathAttributesNextHop extends BgpUpdatePathAttributes
{
    typeCode = NEXT_HOP;
    optionalBit = false;
    transitiveBit = true;
    length = 4;
    Ipv4Address value;
}

//MULTI_EXIT_DISC (RFC4271 Section 4.3, Type code 4):
class BgpUpdatePathAttributesMultiExitDisc extends BgpUpdatePathAttributes
{
    typeCode = MULTI_EXIT_DISC;
    optionalBit = true;
    transitiveBit = false;
    length = 4;
    uint32_t value;
}

//LOCAL_PREF (RFC4271 Section 4.3, Type code 5):
class BgpUpdatePathAttributesLocalPref extends BgpUpdatePathAttributes
{
    typeCode = LOCAL_PREF;
    optionalBit = false;
    transitiveBit = true;
    length = 4;
    uint32_t value;
}

//ATOMIC_AGGREGATE (RFC4271 Section 4.3, Type code 6):
class BgpUpdatePathAttributesAtomicAggregate extends BgpUpdatePathAttributes
{
    typeCode = ATOMIC_AGGREGATE;
    optionalBit = false;
    transitiveBit = true;
    length = 0;
}

class BgpUpdatePathAttributesAggregator extends BgpUpdatePathAttributes
{
    typeCode = AGGREGATOR;
    optionalBit = true;
    transitiveBit = true;
    length = 6;
    uint16_t asNumber;
    Ipv4Address bgpSpeaker;
}

struct BgpUpdateWithdrawnRoutes
{
    unsigned char length; // 1 octet
    Ipv4Address prefix; // 4 octets
}

struct BgpUpdateNlri
{
    // uint32_t pathIdentifier      // specified in RFC 7911, optional 4 octets, wireshark detect existing of this field with heuristical algorithm
    uint8_t length = 0; // length of prefix in bits, 1 octet
    Ipv4Address prefix; // 1-4 octets, based on length ((length + 7) / 8)
}

//
// Represents a BGPv4 UPDATE message.
//
// see RFC 4271 Section 4.3
//
class BgpUpdateMessage extends BgpHeader
{
    type = BGP_UPDATE;
    chunkLength = BGP_HEADER_OCTETS + BGP_EMPTY_UPDATE_OCTETS;
    totalLength = B(BGP_HEADER_OCTETS + BGP_EMPTY_UPDATE_OCTETS).get();

    uint16_t withDrawnRoutesLength = 0;    // This 2-octets unsigned integer indicates the total length of
                                           // the Withdrawn Routes field in octets.  Its value allows the
                                           // length of the Network Layer Reachability Information field to
                                           // be determined, as specified below.
                                           //
                                           // A value of 0 indicates that no routes are being withdrawn from
                                           // service, and that the WITHDRAWN ROUTES field is not present in
                                           // this UPDATE message.

    BgpUpdateWithdrawnRoutes withdrawnRoutes[];
    uint16_t totalPathAttributeLength = 0;    // This 2-octets unsigned integer indicates the total length of
                                              // Path Attributes field in octets.  Its value allows the length
                                              // of the Network Layer Reachability field to be determined as
                                              // specified below.
                                              //
                                              // A value of 0 indicates that neither the Network Layer
                                              // Reachability Information field nor the Path Attribute field is
                                              // present in this UPDATE message.
    BgpUpdatePathAttributes* pathAttributes[] @owned @allowReplace;
    BgpUpdateNlri NLRI[];
}

