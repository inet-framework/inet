// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//
// @file finalTest.ned
// @date 21.10.2011
// @author: Tomas Prochazka (mailto:xproch21@stud.fit.vutbr.cz), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
// @brief Topology with PIM network

package inet.examples.pim.sm.basic;

import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.inet.MulticastRouter;
import inet.node.inet.StandardHost;


network PIM_SM_Network
{
    @display("bgb=567,467");

    submodules:
        DR_R1: MulticastRouter {
            parameters:
                @display("p=70,420");
            gates:
                ethg[3];
        }
        DR_R2: MulticastRouter {
            parameters:
                @display("p=70,302");
            gates:
                ethg[4];
        }
        RP: MulticastRouter {
            parameters:
                @display("p=394,302");
            gates:
                ethg[3];
        }
        DR_S2: MulticastRouter {
            parameters:
                @display("p=230,197");
            gates:
                ethg[4];
        }
        DR_S1: MulticastRouter {
            parameters:
                @display("p=420,143");
            gates:
                ethg[3];
        }
        Source2: StandardHost {
            parameters:
                @display("i=device/server;p=230,36");
            gates:
                ethg[1];
        }
        Source1: StandardHost {
            parameters:
                @display("i=device/server;p=536,29");
            gates:
                ethg[1];
        }
        Receiver1: StandardHost {
            parameters:
                @display("p=394,420");
            gates:
                ethg[1];
        }
        Receiver2: StandardHost {
            parameters:
                @display("p=70,85");
            gates:
                ethg[1];
        }
        configurator: IPv4NetworkConfigurator {
            @display("p=510,420");
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
