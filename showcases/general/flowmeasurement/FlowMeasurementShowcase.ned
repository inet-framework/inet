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

package inet.showcases.general.flowmeasurement;

import inet.networks.WiredNetworkBase;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.INetworkNode;
import inet.node.inet.StandardHost;
import inet.protocolelement.measurement.IMeasurementLayer;


module MyStandardHost extends StandardHost
{
    submodules:
        measurementLayer: <default("")> like IMeasurementLayer {
            @display("p=450,250");
        }
    connections:
        udp.ipOut --> { @reconnect; } --> measurementLayer.upperLayerIn if hasUdp;
        measurementLayer.lowerLayerOut --> { @reconnect; } --> tn.in++ if hasUdp;
        udp.ipIn <-- { @reconnect; } <-- measurementLayer.upperLayerOut if hasUdp;
        measurementLayer.lowerLayerIn <-- { @reconnect; } <-- tn.out++ if hasUdp;
}

network FlowMeasurementShowcase extends WiredNetworkBase
{
    submodules:
        client1: <default("StandardHost")> like INetworkNode {
            @display("p=300,150");
        }
        client2: <default("StandardHost")> like INetworkNode {
            @display("p=300,350");
        }
        switch1: EthernetSwitch {
            @display("p=500,250");
        }
        switch2: EthernetSwitch {
            @display("p=700,250");
        }
        server1: <default("StandardHost")> like INetworkNode {
            @display("p=900,150");
        }
        server2: <default("StandardHost")> like INetworkNode {
            @display("p=900,350");
        }
    connections:
        client1.ethg++ <--> Eth100M <--> switch1.ethg++;
        client2.ethg++ <--> Eth100M <--> switch1.ethg++;
        switch2.ethg++ <--> Eth100M <--> server1.ethg++;
        switch2.ethg++ <--> Eth100M <--> server2.ethg++;
        switch1.ethg++ <--> Eth100M <--> switch2.ethg++;
}

