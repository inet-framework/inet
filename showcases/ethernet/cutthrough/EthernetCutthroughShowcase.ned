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

package inet.showcases.ethernet.cutthrough;

import inet.networks.WiredNetworkBase;
import inet.node.ethernet.Eth1G;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.StandardHost;


network EthernetCutthroughShowcase extends WiredNetworkBase
{
    submodules:
        host1: StandardHost {
            @display("p=250,100");
        }
        switch1: EthernetSwitch {
            @display("p=350,100");
        }
        switch2: EthernetSwitch {
            @display("p=450,100");
        }
        host2: StandardHost {
            @display("p=550,100");
        }
    connections:
        host1.ethg++ <--> Eth1G <--> switch1.ethg++;
        switch1.ethg++ <--> Eth1G <--> switch2.ethg++;
        switch2.ethg++ <--> Eth1G <--> host2.ethg++;
}

