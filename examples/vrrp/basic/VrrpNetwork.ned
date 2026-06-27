//
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.vrrp.basic;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;

//
// Two VRRP routers (gw1, gw2) share the virtual gateway address 192.168.1.254
// on a switched LAN; a host uses that address as its default gateway.
//
network VrrpNetwork
{
    @display("bgb=400,300");
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=50,50");
        }
        gw1: Router {
            hasVrrp = true;
            @display("p=120,80");
        }
        gw2: Router {
            hasVrrp = true;
            @display("p=280,80");
        }
        switch: EthernetSwitch {
            @display("p=200,170");
        }
        host: StandardHost {
            @display("p=200,250");
        }
    connections:
        gw1.ethg++ <--> Eth100M <--> switch.ethg++;
        gw2.ethg++ <--> Eth100M <--> switch.ethg++;
        host.ethg++ <--> Eth100M <--> switch.ethg++;
}
