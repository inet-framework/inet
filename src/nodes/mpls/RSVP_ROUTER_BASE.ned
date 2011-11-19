//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
package inet.nodes.mpls;



//
// The Base module for an RSVP-TE capable router. See ~RSVP_LSR, ~RSVP_FAILED
//
// RSVP occupies the Transport layer; however, it is not a transport protocol
// itself. RSVP uses transport protocols to route packets. ~TED is used
// to calculate shortest paths.
//
module RSVP_ROUTER_BASE
{
    parameters:
        @node();
        @labels(node,mpls-node);
        @display("i=abstract/router");
        string peers;
        string routerId = default("auto");
        string routingFile = default("");
    gates:
        inout pppg[] @labels(PPPFrame-conn);
}

