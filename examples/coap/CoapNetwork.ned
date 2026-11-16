//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.coap;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.networklayer.configurator.ipv6.Ipv6FlatNetworkConfigurator;
import inet.node.ethernet.EthernetLink;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.StandardHost;

//
// IPv4 CoAP example topology connecting client, updater, and server through
// an Ethernet switch.
//
network CoapNetwork
{
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=80,50");
        }
        switch: EthernetSwitch {
            @display("p=250,150");
        }
        client: StandardHost {
            @display("p=120,250");
        }
        updater: StandardHost {
            @display("p=250,280");
        }
        server: StandardHost {
            @display("p=400,150");
        }
    connections:
        client.ethg++ <--> EthernetLink { datarate = 100Mbps; delay = 0.5ms; } <--> switch.ethg++;
        updater.ethg++ <--> EthernetLink { datarate = 100Mbps; delay = 0.5ms; } <--> switch.ethg++;
        switch.ethg++ <--> EthernetLink { datarate = 100Mbps; delay = 0.5ms; } <--> server.ethg++;
}

//
// IPv6 CoAP example topology with autoconfigured client and server hosts.
//
network CoapIpv6Network
{
    submodules:
        configurator: Ipv6FlatNetworkConfigurator {
            @display("p=80,50");
        }
        client: StandardHost {
            @display("p=150,150");
            hasIpv4 = false;
            hasIpv6 = true;
        }
        server: StandardHost {
            @display("p=400,150");
            hasIpv4 = false;
            hasIpv6 = true;
        }
    connections:
        client.ethg++ <--> EthernetLink { datarate = 100Mbps; delay = 0.5ms; } <--> server.ethg++;
}
