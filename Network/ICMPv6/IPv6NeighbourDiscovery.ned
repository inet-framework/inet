//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


//
// Implements \IPv6 Neighbour Discovery.
//
// An overview of the \IPv6 implementation in the INET Framework is
// provided <a href="ipv6overview.html">here</a>.
//
// @see IPv6, RoutingTable6, ICMPv6
//
simple IPv6NeighbourDiscovery
    gates:
        in: fromIPv6;
        out: toIPv6;
endsimple
