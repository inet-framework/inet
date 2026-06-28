//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// Babel routing protocol (RFC 6126) ported from the ANSAINET project.
// Original authors: Vit Rek, Vladimir Vesely (Brno University of Technology).
//

package inet.routing.babel;

import inet.applications.contract.IApp;
import inet.common.SimpleModule;

//
// Babel is a loop-free distance-vector routing protocol (RFC 6126), robust on
// lossy and wireless links. It is dual-stack: a single Babel process routes
// both IPv4 and IPv6. Routers exchange TLV-based control messages over UDP port
// 6696 (IPv4 multicast 224.0.0.111 and IPv6 link-local multicast ff02::1:6),
// using a feasibility condition plus sequence numbers to guarantee loop
// freedom, and a pluggable per-link cost (e.g. ETX). The module installs the
// selected routes into the IPv4 and/or IPv6 routing tables. It slots into a
// host/router as an application (`like IApp`).
//
// NOTE: scaffold — neighbour management, the route-selection algorithm and the
// data plane are not yet implemented.
//
simple Babel extends SimpleModule like IApp
{
    parameters:
        @class(Babel);
        @display("i=block/network2");
        string interfaceTableModule = default("^.interfaceTable");
        string ipv4RoutingTableModule = default("^.ipv4.routingTable");
        string ipv6RoutingTableModule = default("^.ipv6.routingTable");
        int udpPort = default(6696);
        xml configData = default(xml("<Babel/>"));
    gates:
        input socketIn;
        output socketOut;
}
