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

package inet.showcases.general.timing;

import inet.networklayer.contract.INetworkLayer;
import inet.networklayer.ipv4.Ipv4NetworkLayer;
import inet.networks.WiredNetworkBase;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.StandardHost;
import inet.protocolelement.measurement.IMeasurementLayer;
import inet.protocolelement.measurement.OmittedMeasurementLayer;


module MyIpv4NetworkLayer like INetworkLayer
{
    parameters:
        //        *.hasIpv4 = hasIpv4;
        //        *.hasIpv6 = hasIpv6;
        //        *.hasGn = hasGn;
        bool forwarding = default(false);
        bool multicastForwarding = default(false);
        string interfaceTableModule;
        string displayStringTextFormat = default("%i");
        *.forwarding = this.forwarding;
        *.multicastForwarding = this.multicastForwarding;
        *.interfaceTableModule = default(absPath(this.interfaceTableModule));
        *.displayStringTextFormat = displayStringTextFormat;
        *.routingTableModule = default(absPath(".routingTable"));
        *.arpModule = default(absPath(".arp"));
        *.icmpModule = default(absPath(".icmp"));
        @class(::inet::Ipv4NetworkLayer);
        @display("i=block/fork");
    gates:
        input ifIn @labels(INetworkHeader);
        output ifOut @labels(INetworkHeader);
        input transportIn @labels(Ipv4ControlInfo/down);
        output transportOut @labels(Ipv4ControlInfo/up);
    submodules:
        ipv4: Ipv4NetworkLayer {
            parameters:
                @display("p=100,100");
        }
        measurementLayer: <default("OmittedMeasurementLayer")> like IMeasurementLayer {
            parameters:
                @display("p=100,200");
        }

    connections:
//        ipv4.ifIn <-- measurementLayer.upperLayerOut;
//        ipv4.ifOut --> measurementLayer.upperLayerIn;
//        transportIn --> ipv4.transportIn;
//        transportOut <-- ipv4.transportOut;
//        measurementLayer.lowerLayerIn <-- transportIn;
//        measurementLayer.lowerLayerOut --> transportOut;

        transportIn --> ipv4.transportIn;
        transportOut <-- ipv4.transportOut;
        ipv4.ifOut --> measurementLayer.upperLayerIn;
        ipv4.ifIn <-- measurementLayer.upperLayerOut;
        measurementLayer.lowerLayerOut --> ifOut;
        measurementLayer.lowerLayerIn <-- ifIn;

}

module MyInheritedIpv4NetworkLayer extends Ipv4NetworkLayer
{
    submodules:
        measurementLayer: <default("OmittedMeasurementLayer")> like IMeasurementLayer {
            @display("p=603,607");
        }
    connections:
        lp.out++ --> { @reconnect; } --> measurementLayer.upperLayerIn;
        measurementLayer.lowerLayerOut --> { @reconnect; } --> ifOut;
        lp.in++ <-- { @reconnect; } <-- measurementLayer.upperLayerOut;
        measurementLayer.lowerLayerIn <-- { @reconnect; } <-- ifIn;
}

network TimingShowcase extends WiredNetworkBase
{
    submodules:
        client1: StandardHost {
            @display("p=300,150");
        }
        client2: StandardHost {
            @display("p=300,350");
        }
        switch1: EthernetSwitch {
            @display("p=500,250");
        }
        switch2: EthernetSwitch {
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

