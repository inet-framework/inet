//
// Copyright (C) 2017 OpenSim Ltd.
//

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.tutorials.ethernet;

import inet.linklayer.configurator.L2NetworkConfigurator;
import inet.node.ethernet.EtherHost;
import inet.node.ethernet.Eth10M;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EtherSwitch;


//
// Sample Ethernet LAN: two hosts directly connected to each other
// via twisted pair.
//
network TwoHosts
{
    submodules:
        hostA: EtherHost {
            parameters:
                @display("p=60,200");
        }
        hostB: EtherHost {
            parameters:
                @display("p=200,200");
        }
    connections:
        hostA.ethg <--> Eth100M {  length = 5m; } <--> hostB.ethg;
}

//
// Sample Ethernet LAN: three hosts connected to one switch
// via twisted pair.
//
network HostsWithSwitch
{
    submodules:
        hostA: EtherHost {
            parameters:
                @display("p=60,200");
        }
        hostB: EtherHost {
            parameters:
                @display("p=200,200");
        }
        hostC: EtherHost {
            parameters:
                @display("p=360,200");
        }
        switch: EtherSwitch {
            parameters:
                @display("p=200,360");
        }
    connections:
        hostA.ethg <--> Eth100M { length = 5m; } <--> switch.ethg++;
        hostB.ethg <--> Eth10M { length = 5m; } <--> switch.ethg++;
        hostC.ethg <--> Eth100M { length = 5m; } <--> switch.ethg++;
}

//
// Sample Ethernet LAN: three hosts and switch pair directly connected to each other
// via twisted pair.
//
network HostsWithSwitches
{
    submodules:
        hostA: EtherHost {
            parameters:
                @display("p=60,200");
        }
        hostB: EtherHost {
            parameters:
                @display("p=200,200");
        }
        hostC: EtherHost {
            parameters:
                @display("p=360,200");
        }
        switchA: EtherSwitch {
            parameters:
                @display("p=60,360");
        }
        switchB: EtherSwitch {
            parameters:
                @display("p=200,280");
        }
        switchC: EtherSwitch {
            parameters:
                @display("p=360,360");
        }
    connections:
        switchA.ethg++ <--> Eth100M { length = 5m; } <--> switchB.ethg++;
        switchA.ethg++ <--> Eth100M { length = 5m; } <--> switchC.ethg++;
        switchB.ethg++ <--> Eth100M { length = 5m; } <--> switchC.ethg++;
        hostA.ethg <--> Eth100M { length = 5m; } <--> switchA.ethg++;
        hostB.ethg <--> Eth10M { length = 5m; } <--> switchB.ethg++;
        hostC.ethg <--> Eth100M { length = 5m; } <--> switchC.ethg++;
}

//
// Sample Ethernet LAN: two hosts directly connected to each other
// via twisted pair .
//
network HostsWithSwitchesSTP extends HostsWithSwitches
{
    submodules:
        l2NetworkConfigurator: L2NetworkConfigurator {
            parameters:
                @display("p=80,60");
        }
}

