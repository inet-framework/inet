//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.inet.configurator;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth1G;
import inet.node.inet.StandardHost;

network SimpleConfiguratorNetwork
{
    @display("bgb=450,300");
    submodules:
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,100;is=s");
        }
        host11: StandardHost {
            parameters:
                @display("p=250,100");
        }
        host21: StandardHost {
            parameters:
                @display("p=400,100");
        }
        host12: StandardHost {
            parameters:
                @display("p=250,250");
        }
        host22: StandardHost {
            parameters:
                @display("p=400,250");
        }
    connections allowunconnected:
        host11.ethg++ <--> Eth1G <--> host21.ethg++;
        host11.ethg++ <--> Eth1G <--> host12.ethg++;
        host21.ethg++ <--> Eth1G <--> host22.ethg++;
        host12.ethg++ <--> Eth1G <--> host22.ethg++;
}
