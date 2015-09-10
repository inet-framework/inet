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
#include "inet/networklayer/contract/ipv4/IPv4Address.h"
#include "inet/routing/bgpv4/BGPMessage/BGPASPathSegment_m.h"
#include "inet/routing/bgpv4/BGPCommon_m.h"
}}

namespace inet::bgp;

enum BGPSessionType;

class noncobject IPv4Address;
class noncobject BGPASPathSegment;


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

