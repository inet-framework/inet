//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.diffserv.simple_;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import ned.DatarateChannel;


//
// This network contains a router with an 10Mbps Ethernet interface,
// and with a 128kbps dialup connection to a server.
//
// The dialup connection is the bottleneck of the traffic from the clients
// to the server. Without QoS, high traffic can cause random packet drops
// at the router Ppp interface.
//
network DiffservNetwork
{
    parameters:
        int numClients = default(1);
    types:
        channel dialup extends DatarateChannel
        {
            delay = normal(0.004s, 0.0018s);
            datarate = 128kbps;
        }
        channel ethernetline extends DatarateChannel
        {
            delay = 0.1us;
            datarate = 10Mbps;
        }
    submodules:
        router: Router {
            @display("p=550,100");
        }
        client[numClients]: StandardHost {
            @display("p=250,100,col,100");
        }
        server: StandardHost {
            @display("p=700,100");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("is=s;p=100,100");
        }
        etherSwitch: EthernetSwitch {
            @display("p=400,100");
        }
    connections:
        router.pppg++ <--> dialup <--> server.pppg++;
        etherSwitch.ethg++ <--> ethernetline <--> router.ethg++;
        for i=0..numClients-1 {
            client[i].ethg++ <--> ethernetline <--> etherSwitch.ethg++;
        }
}
