//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.pim.dm.assert;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.MulticastRouter;
import inet.node.inet.StandardHost;


network Network
{
    submodules:
        R1: MulticastRouter {
            @display("p=400,200");
        }
        R2: MulticastRouter {
            @display("p=550,100");
        }
        R3: MulticastRouter {
            @display("p=550,300");
        }

        Source: StandardHost {
            @display("p=250,200");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,100;is=s");
        }
        R4: MulticastRouter {
            @display("p=850,199");
        }
        Receiver: StandardHost {
            @display("p=1000,199");
        }
        switch: EthernetSwitch {
            @display("p=700,199");
        }
    connections allowunconnected:
        R1.ethg++ <--> Eth100M <--> R2.ethg++;
        R1.ethg++ <--> Eth100M <--> R3.ethg++;
        R1.ethg++ <--> Eth100M <--> Source.ethg++;

        switch.ethg++ <--> Eth100M <--> R2.ethg++;
        switch.ethg++ <--> Eth100M <--> R3.ethg++;
        R4.ethg++ <--> Eth100M <--> switch.ethg++;
        R4.ethg++ <--> Eth100M <--> Receiver.ethg++;
}
