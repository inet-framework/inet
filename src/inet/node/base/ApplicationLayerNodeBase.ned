//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.node.base;

import inet.applications.contract.IApp;
import inet.common.MessageDispatcher;

//
// An extension of the transport layer node base that adds support for applications.
// Provides the highest level of abstraction for network nodes, completing the full
// network stack from physical to application layer.
//
// Adds an application layer with configurable application modules and
// a message dispatcher that connects applications to the transport protocols.
// It serves as the foundation for end-user devices like hosts and specialized nodes
// that need application functionality.
//
// @see ~TransportLayerNodeBase, ~NetworkLayerNodeBase, ~StandardHost
//
module ApplicationLayerNodeBase extends TransportLayerNodeBase
{
    parameters:
        int numApps = default(0);
        @figure[applicationLayer](type=rectangle; pos=250,5; size=1000,137; lineColor=#808080; cornerRadius=5; fillColor=#ffff00; fillOpacity=0.1);
        @figure[applicationLayer.title](type=text; pos=1245,10; anchor=ne; text="application layer");
    submodules:
        app[numApps]: <> like IApp {
            @display("p=375,75,row,150");
        }
        at: MessageDispatcher {
            @display("p=750,150;b=1000,5,,,,1");
        }
    connections allowunconnected:
        for i=0..numApps-1 {
            app[i].socketOut --> at.in++;
            app[i].socketIn <-- at.out++;
        }

        at.out++ --> udp.appIn if hasUdp;
        at.in++ <-- udp.appOut if hasUdp;

        at.out++ --> tcp.appIn if hasTcp;
        at.in++ <-- tcp.appOut if hasTcp;

        at.out++ --> sctp.appIn if hasSctp;
        at.in++ <-- sctp.appOut if hasSctp;

        at.out++ --> tn.in++;
        at.in++ <-- tn.out++;
}

