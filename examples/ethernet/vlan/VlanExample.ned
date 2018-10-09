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

package inet.examples.ethernet.vlan;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EtherSwitch;
import inet.node.inet.StandardHost;

network VlanExample
{
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,100;is=s");
        }
        host1: StandardHost {
            @display("p=250,100");
        }
        switch1: EtherSwitch {
            @display("p=350,100");
        }
        switch2: EtherSwitch {
            @display("p=450,100");
        }
        host2: StandardHost {
            @display("p=550,100");
        }
    connections:
        host1.ethg++ <--> Eth100M <--> switch1.ethg++;
        host2.ethg++ <--> Eth100M <--> switch2.ethg++;
        switch1.ethg++ <--> Eth100M <--> switch2.ethg++;
}

