//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//


package inet.examples.inet.routerperf;

import inet.applications.generic.IIpvxTrafficGenerator;
import inet.node.base.TransportLayerNodeBase;

//
// Definition of an IP node with a transport generator that connects
// to IP directly, without TCP or UDP.
//
module BurstHost extends TransportLayerNodeBase
{
    parameters:
        @networkNode();
        @display("i=device/pc3");
        hasUdp = false;
        hasTcp = false;
        *.routingTableModule = default("^.ipv4.routingTable");
    submodules:
        trafGen: <> like IIpvxTrafficGenerator {
            parameters:
                @display("p=329,81");
        }
    connections allowunconnected:
        // transport connections
        trafGen.ipOut --> tn.in++;
        trafGen.ipIn <-- tn.out++;
}

