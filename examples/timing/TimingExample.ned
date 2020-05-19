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

package inet.examples.timing;

import inet.networks.WiredNetworkBase;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EtherSwitch;
import inet.node.inet.StandardHost;

network TimingExample extends WiredNetworkBase
{
    submodules:
        client1: StandardHost {
            @display("p=300,150");
        }
        client2: StandardHost {
            @display("p=300,350");
        }
        switch1: EtherSwitch {
            @display("p=500,250");
        }
        switch2: EtherSwitch {
            @display("p=700,250");
        }
        server1: StandardHost {
            @display("p=900,150");
        }
        server2: StandardHost {
            @display("p=900,350");
        }
    connections:
        client1.ethg++ <--> Eth100M <--> switch1.ethg++;
        client2.ethg++ <--> Eth100M <--> switch1.ethg++;
        switch2.ethg++ <--> Eth100M <--> server1.ethg++;
        switch2.ethg++ <--> Eth100M <--> server2.ethg++;
        switch1.ethg++ <--> Eth100M <--> switch2.ethg++;
}
