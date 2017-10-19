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

cplusplus {{
#include "inet/common/INETDefs.h"
#include "inet/common/packet/chunk/FieldsChunk.h"
#include "inet/networklayer/contract/ipv4/IPv4Address.h"
}}

import inet.routing.bgpv4.BGPCommon;

namespace inet;

class noncobject IPv4Address;
class FieldsChunk extends cObject;

namespace inet::bgp;

enum BgpConstants{
    BGP_HEADER_OCTETS = 19;
    BGP_OPEN_OCTETS = 10;
    BGP_EMPTY_UPDATE_OCTETS = 4; // UnfeasibleRoutesLength (2) + TotalPathAttributeLength (2)
};

//
// Represents a BGPv4 AS_PATH path attribute (RFC 4271 Section 4.3)
//
class BGPASPathSegment
{
    unsigned char type @enum(BGPPathSegmentType);
    unsigned char length;
    unsigned short asValue[];
}


//
// BGPv4 message types
//
enum BGPType
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
class BGPHeader extends FieldsChunk
{
    chunkLength = B(BGP_HEADER_OCTETS);
    char type @enum(BGPType);
}

//
// Represents a BGPv4 KEEPALIVE message.
//
// KEEPALIVE messages are exchanged as often as necessary between systems
// to avoid exceeding the BGP Hold Timer (constant defined in BGPOpen).
//
class BGPKeepAliveMessage extends BGPHeader
{
    type = BGP_KEEPALIVE;
}

struct BGPParameterValues
{
    short authCode;
    long authenticationData;
}

struct BGPOptionalParameters
{
    short parameterType;
    unsigned short parameterLength;
    BGPParameterValues parameterValues;
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
class BGPOpenMessage extends BGPHeader
{
    @omitGetVerb(true);
    @fieldNameSuffix("_var");
    type = BGP_OPEN;
    chunkLength = B(BGP_HEADER_OCTETS + BGP_OPEN_OCTETS);

    char version = 4;
    unsigned short myAS;
    simtime_t holdTime;
    IPv4Address BGPIdentifier;
    BGPOptionalParameters optionalParameters[];
}

enum BGPUpdateAttributeTypeCode
{
    ORIGIN = 1;
    AS_PATH = 2;
    NEXT_HOP = 3;
    LOCAL_PREF = 5;
    ATOMIC_AGGREGATE = 6;
}

struct BGPUpdateAttributeFlags
{
    bool optionalBit;
    bool transitiveBit;
    bool partialBit;
    bool estendedLengthBit;
}

struct BGPUpdateAttributeType
{
    BGPUpdateAttributeFlags flags;
    unsigned char typeCode @enum(BGPUpdateAttributeTypeCode);
}

class BGPUpdatePathAttributes
{
    BGPUpdateAttributeType type; // 2 octets
    unsigned short length; // 2 octets
}

// ORIGIN (RFC4271 Section 4.3, Type code 1):
class BGPUpdatePathAttributesOrigin extends BGPUpdatePathAttributes
{
    int value @enum(BGPSessionType);
}

// AS_PATH (RFC4271 Section 4.3, Type code 2):
class BGPUpdatePathAttributesASPath extends BGPUpdatePathAttributes
{
    BGPASPathSegment value[];
}

//NEXT_HOP (RFC4271 Section 4.3, Type code 3):
class BGPUpdatePathAttributesNextHop extends BGPUpdatePathAttributes
{
    IPv4Address value;
}

//LOCAL_PREF (RFC4271 Section 4.3, Type code 5):
class BGPUpdatePathAttributesLocalPref extends BGPUpdatePathAttributes
{
    unsigned int value;
}

//ATOMIC_AGGREGATE (RFC4271 Section 4.3, Type code 6):
enum BGPUpdateAtomicAggregateValues
{
    NO_SPECIFIC_ROUTE = 0;
    SPECIFIC_ROUTE = 1;
}

class BGPUpdatePathAttributesAtomicAggregate extends BGPUpdatePathAttributes
{
    int value @enum(BGPUpdateAtomicAggregateValues);
}

struct BGPUpdateWithdrawnRoutes
{
    unsigned char length; // 1 octet
    IPv4Address prefix; // 4 octets
}

struct BGPUpdateNLRI
{
    unsigned char length; // 1 octet
    IPv4Address prefix; // 4 octets
}

//
// Represents the list of path attributes in a BGPv4 UPDATE message.
// See RFC 4271, section 4.3
//
class BGPUpdatePathAttributeList
{
    BGPUpdatePathAttributesOrigin origin; // mandatory attribute
    BGPUpdatePathAttributesASPath asPath[]; // mandatory attribute
    BGPUpdatePathAttributesNextHop nextHop; // mandatory attribute
    BGPUpdatePathAttributesLocalPref localPref[]; // optional attribute
    BGPUpdatePathAttributesAtomicAggregate atomicAggregate[]; // optional attribute
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
//    - prefix : variable size (contains the IP prefix; IPv4: 4 octets)
//
class BGPUpdateMessage extends BGPHeader
{
    @customize(true);
    type = BGP_UPDATE;
    chunkLength = B(BGP_HEADER_OCTETS + BGP_EMPTY_UPDATE_OCTETS);

    BGPUpdateWithdrawnRoutes withdrawnRoutes[];
    BGPUpdatePathAttributeList pathAttributeList[]; // optional field (size is either 0 or 1)
    BGPUpdateNLRI NLRI;
}

