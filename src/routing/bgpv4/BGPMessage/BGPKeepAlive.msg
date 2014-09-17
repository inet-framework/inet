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
}}

namespace inet::bgp;

class BGPHeader;

//
// Represents a BGPv4 KEEPALIVE message.
//
// KEEPALIVE messages are exchanged as often as necessary between systems
// to avoid exceeding the BGP Hold Timer (constant defined in BGPOpen).
//
packet BGPKeepAliveMessage extends BGPHeader
{
    type = BGP_KEEPALIVE;
}


