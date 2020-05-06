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

package inet.examples.ethernet.interface;

import inet.networks.WiredNetworkBase;
import inet.node.ethernet.Eth100M;
import inet.node.inet.StandardHost;

network EthernetInterfaceConfigurationExample extends WiredNetworkBase
{
    parameters:
        string highTsOperation;
    submodules:
        host1: StandardHost {
            @display("p=250,100");
        }
        host2: StandardHost {
            @display("p=350,100");
        }
    connections:
        host1.ethg++ <--> Eth100M <--> host2.ethg++;
}

