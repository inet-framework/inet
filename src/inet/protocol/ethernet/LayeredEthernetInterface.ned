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

package inet.protocol.ethernet;

import inet.linklayer.contract.IEthernetInterface;
import inet.networklayer.common.InterfaceEntry;

module LayeredEthernetInterface extends InterfaceEntry like IEthernetInterface
{
    parameters:
        string protocol = default("ethernetmac");
        bool broadcast = default(true);
        bool multicast = default(true);
        string address = default("auto");
        double bitrate @unit(bps);
        string interfaceTableModule;
        *.bitrate = default(bitrate);
        @class(InterfaceEntry);
        @display("i=block/ifcard;bgb=700,300");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        inout phys;
    submodules:
        macLayer: <default("EthernetMacLayer")> like IEthernetMacLayer {
            @display("p=500,100");
        }
        phyLayer: <default("EthernetPhyLayer")> like IEthernetPhyLayer {
            @display("p=500,200");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> macLayer.upperLayerIn;
        macLayer.lowerLayerOut --> phyLayer.upperLayerIn;
        phyLayer.lowerLayerOut --> { @display("m=s"); } --> phys$o;

        phys$i --> { @display("m=s"); } --> phyLayer.lowerLayerIn;
        phyLayer.upperLayerOut --> macLayer.lowerLayerIn;
        macLayer.upperLayerOut --> { @display("m=n"); } --> upperLayerOut;
}
