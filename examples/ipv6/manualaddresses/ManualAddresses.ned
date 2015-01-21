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

package inet.examples.ipv6.manualaddresses;

import inet.node.ethernet.Eth1G;
import inet.node.ethernet.EtherSwitch;
import inet.node.ipv6.Router6;
import inet.node.ipv6.StandardHost6;


//
// TODO Auto-generated network
//
network ManualAddresses
{
    @display("bgb=649,384");
    submodules:
        switch: EtherSwitch {
            @display("p=294,212");
        }

        router1: Router6 {
            @display("p=207,213");
        }
        net1host1: StandardHost6 {
            @display("p=60,122");
        }
        net1host2: StandardHost6 {
            @display("p=60,301");
        }

        router2: Router6 {
            @display("p=296,124");
        }
        net2host1: StandardHost6 {
            @display("p=294,30");
        }

        router3: Router6 {
            @display("p=368,213");
        }
        server: StandardHost6 {
            @display("p=446,213");
        }

        net1switch: EtherSwitch {
            @display("p=133,212");
        }
    connections:
        router1.ethg++ <--> Eth1G <--> switch.ethg++;
        router2.ethg++ <--> Eth1G <--> switch.ethg++;
        router3.ethg++ <--> Eth1G <--> switch.ethg++;

        net2host1.ethg++ <--> Eth1G <--> router2.ethg++;
        server.ethg++ <--> Eth1G <--> router3.ethg++;
        net1switch.ethg++ <--> Eth1G <--> router1.ethg++;
        net1host1.ethg++ <--> Eth1G <--> net1switch.ethg++;
        net1host2.ethg++ <--> Eth1G <--> net1switch.ethg++;
}
