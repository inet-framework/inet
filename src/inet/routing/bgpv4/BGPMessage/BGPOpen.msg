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
#include "inet/networklayer/contract/ipv4/IPv4Address.h"

namespace inet { namespace bgp { const int BGP_OPEN_OCTETS = 10; } }

}}


namespace inet::bgp;

class BGPHeader;

class noncobject IPv4Address;

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
packet BGPOpenMessage extends BGPHeader
{
    @omitGetVerb(true);
    @fieldNameSuffix("_var");
    type = BGP_OPEN;
    byteLength = BGP_HEADER_OCTETS + BGP_OPEN_OCTETS;

    char version = 4;
    unsigned short myAS;
    simtime_t holdTime;
    IPv4Address BGPIdentifier;
    BGPOptionalParameters optionalParameters[];
}

