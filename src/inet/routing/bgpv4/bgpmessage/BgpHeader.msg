//
// Copyright (C) 2010 Helene Lageber
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
    unsigned char length;
    unsigned short asValue[];
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
    BgpType type;
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

struct BgpParameterValues
{
    short authCode;
    long authenticationData;
}

struct BgpOptionalParameters
{
    short parameterType;
    unsigned short parameterLength;
    BgpParameterValues parameterValues;
}


//
// Represents a BGPv4 OPEN message.
//
// Fields modelled:
// - Version: 1 octet (BGP version)
// - My Autonomous system: 2 octets  (AS of the sender)
// - Hold Time: 2 octets (nbr de secondes que l'emetteur propose pour le compteur de renetue)
// - BGP Identifier: 4 octets
// - Optional Param Length: 1 octet (if = 0, no optional parameters)
// - Optional Parameters:
//    - Parameter Type
//    - Parameter Length
//    - Parameter Values (variable size)
//    - Auth code
//    - Authentication Data
//
class BgpOpenMessage extends BgpHeader
{
    type = BGP_OPEN;
    chunkLength = BGP_HEADER_OCTETS + BGP_OPEN_OCTETS;

    char version = 4;
    unsigned short myAS;
    simtime_t holdTime;
    Ipv4Address BGPIdentifier;
    BgpOptionalParameters optionalParameters[];
}

enum BgpUpdateAttributeTypeCode
{
    ORIGIN = 1;
    AS_PATH = 2;
    NEXT_HOP = 3;
    LOCAL_PREF = 5;
    ATOMIC_AGGREGATE = 6;
}

struct BgpUpdateAttributeFlags
{
    bool optionalBit;
    bool transitiveBit;
    bool partialBit;
    bool estendedLengthBit;
}

struct BgpUpdateAttributeType
{
    BgpUpdateAttributeFlags flags;
    BgpUpdateAttributeTypeCode typeCode;
}

class BgpUpdatePathAttributes extends cObject
{
    BgpUpdateAttributeType type; // 2 octets
    unsigned short length; // 2 octets
}

// ORIGIN (RFC4271 Section 4.3, Type code 1):
class BgpUpdatePathAttributesOrigin extends BgpUpdatePathAttributes
{
    BgpSessionType value;
}

// AS_PATH (RFC4271 Section 4.3, Type code 2):
class BgpUpdatePathAttributesAsPath extends BgpUpdatePathAttributes
{
    BgpAsPathSegment value[];
}

//NEXT_HOP (RFC4271 Section 4.3, Type code 3):
class BgpUpdatePathAttributesNextHop extends BgpUpdatePathAttributes
{
    Ipv4Address value;
}

//LOCAL_PREF (RFC4271 Section 4.3, Type code 5):
class BgpUpdatePathAttributesLocalPref extends BgpUpdatePathAttributes
{
    unsigned int value;
}

//ATOMIC_AGGREGATE (RFC4271 Section 4.3, Type code 6):
enum BgpUpdateAtomicAggregateValues
{
    NO_SPECIFIC_ROUTE = 0;
    SPECIFIC_ROUTE = 1;
}

class BgpUpdatePathAttributesAtomicAggregate extends BgpUpdatePathAttributes
{
    BgpUpdateAtomicAggregateValues value;
}

struct BgpUpdateWithdrawnRoutes
{
    unsigned char length; // 1 octet
    Ipv4Address prefix; // 4 octets
}

struct BgpUpdateNlri
{
    unsigned char length; // 1 octet
    Ipv4Address prefix; // 4 octets
}

//
// Represents the list of path attributes in a BGPv4 UPDATE message.
// See RFC 4271, section 4.3
//
class BgpUpdatePathAttributeList extends cObject
{
    BgpUpdatePathAttributesOrigin origin; // mandatory attribute
    BgpUpdatePathAttributesAsPath asPath[]; // mandatory attribute
    BgpUpdatePathAttributesNextHop nextHop; // mandatory attribute
    BgpUpdatePathAttributesLocalPref localPref[]; // optional attribute
    BgpUpdatePathAttributesAtomicAggregate atomicAggregate[]; // optional attribute
}

//
// Represents a BGPv4 UPDATE message.
//
// BGP UPDATE fields modelled:
//   - Unfeasible routes length: 2 octets (number of withdrawn routes)
//   - Withdrawn routes : variable size  (list of IP prefixes of unfeasible routes)
//   - Total Path Attributes Length: 1 octet (if = 0, no Path Attributes)
//   - Path Attributes:
//     - Attribute Type (2 octets)
//     - Attribute Length
//     - Attribute Values (variable size)
// - Network Layer Reachability Information: (variable size)
//    - Length : 1 octet
//    - prefix : variable size (contains the IP prefix; Ipv4: 4 octets)
//
class BgpUpdateMessage extends BgpHeader
{
    @customize;     // update chunkLength when changed   //FIXME setNLRI() always increment length with 5 bytes
    type = BGP_UPDATE;
    chunkLength = BGP_HEADER_OCTETS + BGP_EMPTY_UPDATE_OCTETS;

    BgpUpdateWithdrawnRoutes withdrawnRoutes[];
    BgpUpdatePathAttributeList pathAttributeList[]; // optional field (size is either 0 or 1)
    BgpUpdateNlri NLRI;
}

