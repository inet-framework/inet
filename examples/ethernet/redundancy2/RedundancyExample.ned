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
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

package inet.examples.ethernet.redundancy2;

import inet.linklayer.configurator.StreamRedundancyConfigurator;
import inet.linklayer.configurator.TsnConfigurator;
import inet.networks.WiredNetworkBase;
import inet.node.ethernet.Eth1G;
import inet.node.ethernet.EthernetSwitch;

network RedundancyExample1 extends WiredNetworkBase
{
    submodules:
        streamRedundancyConfigurator: StreamRedundancyConfigurator {
            @display("p=100,400;is=s");
        }
        tsnConfigurator: TsnConfigurator {
            @display("p=100,500;is=s");
        }
        a: EthernetSwitch {
            @display("p=500,200");
        }
        b: EthernetSwitch {
            @display("p=700,100");
        }
        d: EthernetSwitch {
            @display("p=900,100");
        }
        c: EthernetSwitch {
            @display("p=700,300");
        }
        e: EthernetSwitch {
            @display("p=900,300");
        }
        f: EthernetSwitch {
            @display("p=1100,200");
        }
    connections:
        a.ethg++ <--> Eth1G <--> c.ethg++;
        a.ethg++ <--> Eth1G <--> b.ethg++;
        b.ethg++ <--> Eth1G <--> d.ethg++;
        b.ethg++ <--> Eth1G <--> e.ethg++;
        c.ethg++ <--> Eth1G <--> e.ethg++;
        d.ethg++ <--> Eth1G <--> f.ethg++;
        e.ethg++ <--> Eth1G <--> f.ethg++;
}

network RedundancyExample2 extends WiredNetworkBase
{
    submodules:
        streamRedundancyConfigurator: StreamRedundancyConfigurator {
            @display("p=100,400;is=s");
        }
        tsnConfigurator: TsnConfigurator {
            @display("p=100,500;is=s");
        }
        a: EthernetSwitch {
            @display("p=489,267");
        }
        d: EthernetSwitch {
            @display("p=410,199");
        }
        d1: EthernetSwitch {
            @display("p=271,233");
        }
        d2: EthernetSwitch {
            @display("p=358,114");
        }
        b: EthernetSwitch {
            @display("p=604,175");
        }
        b1: EthernetSwitch {
            @display("p=760,250");
        }
        b2: EthernetSwitch {
            @display("p=721,114");
        }
        c: EthernetSwitch {
            @display("p=546,375");
        }
        c1: EthernetSwitch {
            @display("p=416,515");
        }
        c2: EthernetSwitch {
            @display("p=585,521");
        }
    connections:
        a.ethg++ <--> Eth1G <--> b.ethg++;
        a.ethg++ <--> Eth1G <--> b1.ethg++;
        a.ethg++ <--> Eth1G <--> c.ethg++;
        a.ethg++ <--> Eth1G <--> c1.ethg++;
        a.ethg++ <--> Eth1G <--> d.ethg++;
        a.ethg++ <--> Eth1G <--> d1.ethg++;
        b.ethg++ <--> Eth1G <--> b1.ethg++;
        b.ethg++ <--> Eth1G <--> b2.ethg++;
        b1.ethg++ <--> Eth1G <--> b2.ethg++;
        d.ethg++ <--> Eth1G <--> d1.ethg++;
        d.ethg++ <--> Eth1G <--> d2.ethg++;
        d1.ethg++ <--> Eth1G <--> d2.ethg++;
        c1.ethg++ <--> Eth1G <--> c2.ethg++;
        c.ethg++ <--> Eth1G <--> c2.ethg++;
        c.ethg++ <--> Eth1G <--> c1.ethg++;
}

