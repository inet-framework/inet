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
#include "BGPHeader_m.h"
#include "BGPUpdatePathAttributes_m.h"
#include "IPAddress.h"
}}


class BGPHeader;

class noncobject BGPUpdatePathAttributesOrigin;
class noncobject BGPUpdatePathAttributesASPath;
class noncobject BGPUpdatePathAttributesNextHop;
class noncobject BGPUpdatePathAttributesLocalPref;
class noncobject BGPUpdatePathAttributesAtomicAggregate;
class noncobject IPAddress;

struct BGPUpdateWithdrawnRoutes
{
    unsigned char length; // 1 octet
    IPAddress prefix; // 4 octets
}

struct BGPUpdateNLRI
{
    unsigned char length; // 1 octet
    IPAddress prefix; // 4 octets
}

class BGPUpdatePathAttributesContent
{
    BGPUpdatePathAttributesOrigin origin;
    BGPUpdatePathAttributesASPath asPath[];
    BGPUpdatePathAttributesNextHop nextHop;
    BGPUpdatePathAttributesLocalPref localPref[]; // optional
    BGPUpdatePathAttributesAtomicAggregate atomicAggregate[]; // optional
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
packet BGPUpdate extends BGPHeader
{
    @customize(true);
    unsigned short unfeasibleRoutesLength = 0;
    BGPUpdateWithdrawnRoutes withdrawnRoutes[]; // if unfeasibleRoutesLength != 0

    unsigned short totalPathAttributeLength = 0;
    BGPUpdatePathAttributesContent pathAttributesContent[]; // if totalPathAttributesLengh != 0

    BGPUpdateNLRI NLRI;
}

