//
// Copyright (C) 2010 Helene Lageber
//    2019 Adrian Novak - multi address-family support
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
import inet.networklayer.contract.ipv6.Ipv6Address;
import inet.networklayer.contract.ipv4.Ipv4Address;
import inet.routing.bgpv4.BgpCommon;

namespace inet::bgp;

cplusplus {{
const B BGP_HEADER_OCTETS = B(19);
const B BGP_OPEN_OCTETS = B(18);
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
// SAME for IPv4 and IPv6 multi address-family
class BgpKeepAliveMessage extends BgpHeader
{
    type = BGP_KEEPALIVE;
}
//xnovak1j - multiprotocol extension - rfc4760 for OPEN message Ipv4 and Ipv6
enum BgpParameterValuesTypeCode
{
	Multiprotocol_extension_capability = 1;
}
//AFI type code does difference  between Ipv4 and Ipv6
enum AFITypes
{
    Ipv4 = 1;
    Ipv6 = 2;
}

enum SAFITypes
{
 	unicast = 1;   
}

enum BgpOptionalParametersTypeCode 
{
	Capability = 2;
}

struct BgpCapabilityValue
{
	AFITypes AFI;	//2 octets
    short Reserved = 0; //1 octet
    SAFITypes SAFI;	//1 0ctet
}

struct BgpParameterValue
{
    //short authCode;
    //long authenticationData;
    BgpParameterValuesTypeCode capabilityCode; //1 octet
    unsigned short capabilityLength; //1 octet
    BgpCapabilityValue capabilityValue;
    
}

struct BgpOptionalParameters
{
    BgpOptionalParametersTypeCode parameterType; //1 octet
    unsigned short parameterLength;	//1 octet
    BgpParameterValue parameterValue;	
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
//		- Type 
//		- Length 
//		- Values
//			-AFI 2 octets
//			- Reserved 1 octet
//			- SAFI 1 octet
		
//    //- Auth code
//    //- Authentication Data
//
class BgpOpenMessage extends BgpHeader
{
    type = BGP_OPEN;
    chunkLength = BGP_HEADER_OCTETS + BGP_OPEN_OCTETS;

    char version = 4;
    unsigned short myAS;
    simtime_t holdTime;
    Ipv4Address BGPIdentifier;
    unsigned short optionalParametersLength = 8;
    BgpOptionalParameters optionalParameters[];
}
//BGP Update
enum BgpUpdateAttributeTypeCode
{
    ORIGIN = 1;
    AS_PATH = 2;
    NEXT_HOP = 3;
    LOCAL_PREF = 5;
    ATOMIC_AGGREGATE = 6;
    MP_REACH_NLRI = 14;
    MP_UNREACH_NLRI = 15;
}

struct BgpUpdateAttributeFlags
{
    bool optionalBit;
    bool transitiveBit;
    bool partialBit;
    bool extendedLengthBit;
}

//struct BgpUpdateAttributeType
//{
//    BgpUpdateAttributeFlags flags;
//    BgpUpdateAttributeTypeCode typeCode;
//}

class BgpUpdatePathAttributes extends cObject
{
    //BgpUpdateAttributeType type; // 2 octets
    //unsigned short length; // 2 octets
}

// ORIGIN (RFC4271 Section 4.3, Type code 1):
class BgpUpdatePathAttributesOrigin extends BgpUpdatePathAttributes
{
    BgpUpdateAttributeFlags flags;
    BgpUpdateAttributeTypeCode type = ORIGIN;
    unsigned char length = 1;
    BgpSessionType value;
}

// AS_PATH (RFC4271 Section 4.3, Type code 2):
class BgpUpdatePathAttributesAsPath extends BgpUpdatePathAttributes
{
    BgpUpdateAttributeFlags flags;
    BgpUpdateAttributeTypeCode type = AS_PATH;
    unsigned char length;
    BgpAsPathSegment value[];
}

//NEXT_HOP (RFC4271 Section 4.3, Type code 3):
class BgpUpdatePathAttributesNextHop6 extends BgpUpdatePathAttributes
{
    BgpUpdateAttributeFlags flags;
    BgpUpdateAttributeTypeCode type = NEXT_HOP;
	unsigned char length = 16;
    Ipv6Address value;
}

class BgpUpdatePathAttributesNextHop extends BgpUpdatePathAttributes
{
    BgpUpdateAttributeFlags flags;
    BgpUpdateAttributeTypeCode type = NEXT_HOP;
    unsigned char length = 4;
    Ipv4Address value;
}

//LOCAL_PREF (RFC4271 Section 4.3, Type code 5):
class BgpUpdatePathAttributesLocalPref extends BgpUpdatePathAttributes
{	
    BgpUpdateAttributeFlags flags;
    BgpUpdateAttributeTypeCode type = LOCAL_PREF;
	unsigned char length;
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
    BgpUpdateAttributeFlags flags;
    BgpUpdateAttributeTypeCode type = ATOMIC_AGGREGATE;
	unsigned char length;
    BgpUpdateAtomicAggregateValues value;
}

struct BgpUpdateWithdrawnRoutes6
{
    unsigned char length; // 1 octet
    Ipv6Address prefix; // 16 octets
}

struct BgpUpdateWithdrawnRoutes
{
    unsigned char length; // 1 octet
    Ipv4Address prefix; // 4 octets
}

struct BgpUpdateNlri6
{
    unsigned char length; // 1 octet
    Ipv6Address prefix; // 16 octets
}

struct BgpUpdateNlri
{
    unsigned char length; // 1 octet
    Ipv4Address prefix; // 4 octets
}

class BgpUpdateMpReachNlriValue extends cObject 
{
	AFITypes AFI = Ipv6;	//2 octets
    SAFITypes SAFI = unicast;
    unsigned char lengthOfNextHop = 17;
    BgpUpdatePathAttributesNextHop6 nextHop;
    short Reserved = 0; //1 octet
    BgpUpdateNlri6 NLRI;
}

class BgpUpdateMpUnreachNlriValue extends cObject 
{
	AFITypes AFI = Ipv6;	//2 octets
    SAFITypes SAFI = unicast;
    BgpUpdateWithdrawnRoutes6 NLRI;
}

class BgpUpdatePathAttributeMpReachNlri extends BgpUpdatePathAttributes
{
    BgpUpdateAttributeFlags flags;
    BgpUpdateAttributeTypeCode type = MP_REACH_NLRI;
	unsigned char length;
	BgpUpdateMpReachNlriValue mpReachNlriValue;
}

class BgpUpdatePathAttributeMpUnreachNlri extends BgpUpdatePathAttributes
{
    BgpUpdateAttributeFlags flags;
    BgpUpdateAttributeTypeCode type = MP_UNREACH_NLRI;
	unsigned char length;
	BgpUpdateMpUnreachNlriValue mpUnreachNlriValue;
}

//
// Represents the list of path attributes in a BGPv4 UPDATE message.
// See RFC 4271, section 4.3
//
class BgpUpdatePathAttributeList6 extends cObject
{
    BgpUpdatePathAttributesOrigin origin; // mandatory attribute
    BgpUpdatePathAttributesAsPath asPath[]; // mandatory attribute
    //BgpUpdatePathAttributesNextHop6 nextHop; // mandatory attribute
    BgpUpdatePathAttributesLocalPref localPref[]; // optional attribute
    BgpUpdatePathAttributesAtomicAggregate atomicAggregate[]; // optional attribute
    BgpUpdatePathAttributeMpReachNlri mpReachNlri; //optional attribute
    BgpUpdatePathAttributeMpUnreachNlri mpUnreachNlri; //optional attribute
    
}

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
class BgpUpdateMessage6 extends BgpHeader
{
    @customize;     // update chunkLength when changed   //FIXME setNLRI() always increment length with 5 bytes
    type = BGP_UPDATE;
    chunkLength = BGP_HEADER_OCTETS + BGP_EMPTY_UPDATE_OCTETS;
	
	unsigned short withdrawnRoutesLength = 0; // 2 octets
    BgpUpdateWithdrawnRoutes6 withdrawnRoutes;
    unsigned short pathAttributeLength = 1; // 2 octets
    BgpUpdatePathAttributeList6 pathAttributeList[]; // optional field (size is either 0 or 1)
}

class BgpUpdateMessage extends BgpHeader
{
    @customize;     // update chunkLength when changed   //FIXME setNLRI() always increment length with 17 bytes
    type = BGP_UPDATE;
    chunkLength = BGP_HEADER_OCTETS + BGP_EMPTY_UPDATE_OCTETS;

	unsigned short withdrawnRoutesLength = 0; // 2 octets
    BgpUpdateWithdrawnRoutes withdrawnRoutes; //withdrawn nlri
    unsigned short pathAttributeLength = 0; // 2 octets
    BgpUpdatePathAttributeList pathAttributeList[]; // optional field (size is either 0 or 1)
    BgpUpdateNlri NLRI;
}

