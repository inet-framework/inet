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
// ICMPv6 implementation.
//
// To send ICMPv6 Echo Request, send any message to port pingIn with IPv6ControlInfo
// attached. The message will be used as ping payload (encapsulated into an ICMPv6Message).
// When ICMPv6 Echo Reply is received, the payload message will be sent to pingOut.
//
// @see IPv6, RoutingTable6, IPv6NeighbourDiscovery
//#FIXME gate descriptions needed...
//
simple ICMPv6
    gates:
        in: fromIPv6;
        out: toIPv6;
        in: pingIn; // ping requests from app
        out: pingOut;
	out: errorOut;  // for errors
endsimple


