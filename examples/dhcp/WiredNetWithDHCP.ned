//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.dhcp;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth10M;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.StandardHost;


network WiredNetWithDHCP
{
    parameters:
        int numHosts;
    submodules:
        server: StandardHost {
            @display("p=400,225");
        }
        switch: EthernetSwitch {
            parameters:
                @display("p=450,350");
        }
        client[numHosts]: StandardHost {
            @display("p=150,100,ri,300,250");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                config = xml("<config><interface hosts='server' address='192.168.1.1' netmask='255.255.255.0' /></config>");
                @display("p=100,100;is=s");
        }
    connections:
        server.ethg++ <--> Eth10M <--> switch.ethg++;
        for i=0..numHosts-1 {
            client[i].ethg++ <--> Eth10M <--> switch.ethg++;
        }
}

