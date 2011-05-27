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
// The Base module for LDP Routers, see ~LDP_LSR, ~LDP_FAILED
//
module LDP_ROUTER_BASE
{
    parameters:
        @node();
        @labels(node);
        @display("i=abstract/router");
        double holdTime @unit("s") = default(15s);
        double helloInterval @unit("s") = default(5s);
        string peers;
        string routerId = default("auto");
        string routingFile = default("");
        string tcpType = default(firstAvailable("TCP","TCP_lwIP","TCP_NSC","TCP_None"));  // tcp implementation (e.g. ~TCP, ~TCP_lwIP, ~TCP_NSC) or ~TCPSpoof
    gates:
        inout pppg[] @labels(PPPFrame-conn);
}
