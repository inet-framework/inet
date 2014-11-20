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

import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EtherSwitch;
import inet.node.inet.MulticastRouter;
import inet.node.inet.StandardHost;


network Network
{
    @display("bgb=448,352,white");

    submodules:
        R1: MulticastRouter {
            @display("p=208,102");
        }
        R2: MulticastRouter {
            @display("p=143,163");
        }
        R3: MulticastRouter {
            @display("p=276,163");
        }

        Source: StandardHost {
            @display("p=208,33");
        }
        configurator: IPv4NetworkConfigurator {
            @display("p=44,41");
        }
        R4: MulticastRouter {
            @display("p=208,300");
        }
        Receiver: StandardHost {
            @display("p=301,300");
        }
        switch: EtherSwitch {
            @display("p=208,221");
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
