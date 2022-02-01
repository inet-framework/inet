//
// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

// @file finalTest.ned
// @date 21.10.2011
// @author: Tomas Prochazka (mailto:xproch21@stud.fit.vutbr.cz), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
// @brief Topology with PIM network

package inet.examples.pim.sm.basic;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.inet.MulticastRouter;
import inet.node.inet.StandardHost;


network PIM_SM_Network
{
    submodules:
        DR_R1: MulticastRouter {
            parameters:
                @display("p=400,190");
            gates:
                ethg[3];
        }
        DR_R2: MulticastRouter {
            parameters:
                @display("p=500,280");
            gates:
                ethg[4];
        }
        RP: MulticastRouter {
            parameters:
                @display("p=600,100");
            gates:
                ethg[3];
        }
        DR_S2: MulticastRouter {
            parameters:
                @display("p=700,280");
            gates:
                ethg[4];
        }
        DR_S1: MulticastRouter {
            parameters:
                @display("p=800,190");
            gates:
                ethg[3];
        }
        Source2: StandardHost {
            parameters:
                @display("i=device/server;p=800,370");
            gates:
                ethg[1];
        }
        Source1: StandardHost {
            parameters:
                @display("i=device/server;p=950,190");
            gates:
                ethg[1];
        }
        Receiver1: StandardHost {
            parameters:
                @display("p=250,190");
            gates:
                ethg[1];
        }
        Receiver2: StandardHost {
            parameters:
                @display("p=400,370");
            gates:
                ethg[1];
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,100;is=s");
        }
    connections allowunconnected:
        DR_R2.ethg[2] <--> Eth100M <--> RP.ethg[1];
        DR_R2.ethg[1] <--> Eth100M <--> DR_S2.ethg[1];
        RP.ethg[0] <--> Eth100M <--> DR_S2.ethg[2];
        DR_S2.ethg[0] <--> Eth100M <--> DR_S1.ethg[1];
        DR_S1.ethg[0] <--> Eth100M <--> Source1.ethg[0];
        DR_R1.ethg[1] <--> Eth100M <--> DR_R2.ethg[0];
        Receiver1.ethg[0] <--> Eth100M <--> DR_R1.ethg[0];
        Receiver2.ethg[0] <--> Eth100M <--> DR_R2.ethg[3];
        DR_S2.ethg[3] <--> Eth100M <--> Source2.ethg[0];
}
