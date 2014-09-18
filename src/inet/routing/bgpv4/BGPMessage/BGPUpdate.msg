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
#include "inet/routing/bgpv4/BGPMessage/BGPHeader_m.h"
#include "inet/routing/bgpv4/BGPMessage/BGPUpdatePathAttributes_m.h"
#include "inet/networklayer/contract/ipv4/IPv4Address.h"

namespace inet {
const int BGP_EMPTY_UPDATE_OCTETS = 4; // UnfeasibleRoutesLength (2) + TotalPathAttributeLength (2)
} // namespace inet

}}


namespace inet::bgp;

class BGPHeader;

class noncobject BGPUpdatePathAttributesOrigin;
class noncobject BGPUpdatePathAttributesASPath;
class noncobject BGPUpdatePathAttributesNextHop;
class noncobject BGPUpdatePathAttributesLocalPref;
class noncobject BGPUpdatePathAttributesAtomicAggregate;
class noncobject IPv4Address;

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
packet BGPUpdateMessage extends BGPHeader
{
    @customize(true);
    type = BGP_UPDATE;
    byteLength = BGP_HEADER_OCTETS + BGP_EMPTY_UPDATE_OCTETS;

    BGPUpdateWithdrawnRoutes withdrawnRoutes[];
    BGPUpdatePathAttributeList pathAttributeList[]; // optional field (size is either 0 or 1)
    BGPUpdateNLRI NLRI;
}

