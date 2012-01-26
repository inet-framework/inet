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
// License along with this program; if not, see <http://www.gnu.org/licenses/>.
//



package inet.networklayer.icmpv6;

//
// ICMPv6 implementation.
//
// To send an ICMPv6 Echo Request, send any message to port pingIn with ~IPv6ControlInfo
// attached. The message will be used as ping payload (encapsulated into an ~ICMPv6Message).
// When an ICMPv6 Echo Reply is received, the payload message will be sent to pingOut.
//
// @see ~IPv6, ~RoutingTable6, ~IPv6NeighbourDiscovery
//#FIXME gate descriptions needed...
//
simple ICMPv6
{
    parameters:
        @display("i=block/control");
    gates:
        input ipv6In;
        output ipv6Out;
        input pingIn[]; // ping requests from app
        output pingOut[];
        output errorOut;  // for errors
}


