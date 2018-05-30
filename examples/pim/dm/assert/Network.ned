// Copyright (C) 2013 Andras Varga
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
// 

package inet.examples.pim.dm.assert;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EtherSwitch;
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
        switch: EtherSwitch {
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
