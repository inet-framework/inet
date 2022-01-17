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

package inet.networks.ethernet;

import inet.networks.base.WiredNetworkBase;
import inet.node.contract.IEthernetNetworkNode;
import inet.node.ethernet.EthernetLink;

network LinearNetwork extends WiredNetworkBase
{
    parameters:
        *.eth[*].bitrate = default(100Mbps);
    submodules:
        client: <default("StandardHost")> like IEthernetNetworkNode {
            @display("p=350,200");
        }
        switch: <default("EthernetSwitch")> like IEthernetNetworkNode {
            @display("p=550,200");
        }
        server: <default("StandardHost")> like IEthernetNetworkNode {
            @display("p=750,200");
        }
    connections:
        client.ethg++ <--> EthernetLink <--> switch.ethg++;
        switch.ethg++ <--> EthernetLink <--> server.ethg++;
}

