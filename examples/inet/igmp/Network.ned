//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.inet.igmp;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;


//
// TODO documentation
//
network Network
{
    submodules:
        Alice: StandardHost {
            @display("p=250,100");
        }
        FITgate1: Router {
            @display("p=550,100");
        }
        FITgate2: Router {
            @display("p=550,200");
        }
        Bob: StandardHost {
            @display("p=250,200");
        }
        Carlos: StandardHost {
            @display("p=550,300");
        }
        etherSwitch: EthernetSwitch {
            @display("p=400,150");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,100;is=s");
            config = xmldoc("config.xml");
        }
    connections allowunconnected:
        Alice.ethg++ <--> Eth100M <--> etherSwitch.ethg++;
        Bob.ethg++ <--> Eth100M <--> etherSwitch.ethg++;
        FITgate1.ethg++ <--> Eth100M <--> etherSwitch.ethg++;
        FITgate2.ethg++ <--> Eth100M <--> etherSwitch.ethg++;
        Carlos.ethg++ <--> Eth100M <--> FITgate2.ethg++;
}

