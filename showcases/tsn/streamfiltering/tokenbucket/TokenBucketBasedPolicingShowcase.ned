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

package inet.showcases.tsn.streamfiltering.tokenbucket;

import inet.networks.TsnNetworkBase;
import inet.node.ethernet.EthernetLink;
import inet.node.tsn.TsnDevice;
import inet.node.tsn.TsnSwitch;

network TokenBucketBasedPolicingShowcase extends TsnNetworkBase
{
    submodules:
        client1: TsnDevice {
            @display("p=300,150");
        }
        client2: TsnDevice {
            @display("p=300,350");
        }
        switch1: TsnSwitch {
            @display("p=500,250");
        }
        switch2: TsnSwitch {
            @display("p=700,250");
        }
        server1: TsnDevice {
            @display("p=900,150");
        }
        server2: TsnDevice {
            @display("p=900,350");
        }
    connections:
        client1.ethg++ <--> EthernetLink <--> switch1.ethg++;
        client2.ethg++ <--> EthernetLink <--> switch1.ethg++;
        switch2.ethg++ <--> EthernetLink <--> server1.ethg++;
        switch2.ethg++ <--> EthernetLink <--> server2.ethg++;
        switch1.ethg++ <--> EthernetLink <--> switch2.ethg++;
}

