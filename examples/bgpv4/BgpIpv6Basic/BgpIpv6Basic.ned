//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.bgpv4.BgpIpv6Basic;

import inet.networklayer.configurator.ipv6.Ipv6NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.inet.Router;
import inet.node.ipv6.StandardHost6;

//
// Minimal IPv6 EBGP topology: two single-router autonomous systems peering over
// IPv6, each advertising the IPv6 network behind it (MP-BGP, RFC 4760).
//
//   hostA --- rA (AS 65001) === rB (AS 65002) --- hostB
//
network BgpIpv6Basic
{
    submodules:
        configurator: Ipv6NetworkConfigurator {
            parameters:
                @display("p=100,100;is=s");
                config = xmldoc("Ipv6Config.xml");
                // no auto static routes: routers get only the on-link routes configured
                // manually below, so the inter-AS routes can only come from BGP
                addStaticRoutes = false;
        }
        rA: Router {
            parameters:
                @display("p=300,200");
        }
        rB: Router {
            parameters:
                @display("p=500,200");
        }
        hostA: StandardHost6 {
            parameters:
                @display("p=300,320;i=device/pc");
        }
        hostB: StandardHost6 {
            parameters:
                @display("p=500,320;i=device/pc");
        }
    connections:
        hostA.ethg++ <--> Eth100M <--> rA.ethg++;
        rA.ethg++ <--> Eth100M <--> rB.ethg++;
        rB.ethg++ <--> Eth100M <--> hostB.ethg++;
}
