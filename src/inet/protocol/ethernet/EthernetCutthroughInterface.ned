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

module EthernetCutthroughInterface extends InterfaceEntry like IEthernetInterface
{
    parameters:
        bool broadcast = default(true);
        bool multicast = default(true);
        string address = default("auto");
        double bitrate @unit(bps);
        string interfaceTableModule;
        string macTableModule;
        *.bitrate = default(bitrate);
        *.macTableModule = default(macTableModule);
        phyLayer.receiver.typename = default("StreamThroughReceiver");
        @class(EthernetCutthroughInterface);
        @display("i=block/ifcard;bgb=700,400");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input cutthroughIn;
        output cutthroughOut;
        inout phys;
    submodules:
        macLayer: EthernetMacLayer {
            @display("p=500,100");
        }
        cutthroughLayer: EthernetCutthroughLayer {
            @display("p=500,200");
        }
        phyLayer: EthernetPhyLayer {
            @display("p=500,300");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> macLayer.upperLayerIn;
        macLayer.lowerLayerOut --> cutthroughLayer.upperLayerIn;
        cutthroughLayer.lowerLayerOut --> phyLayer.upperLayerIn;
        phyLayer.lowerLayerOut --> { @display("m=s"); } --> phys$o;

        phys$i --> { @display("m=s"); } --> phyLayer.lowerLayerIn;
        phyLayer.upperLayerOut --> cutthroughLayer.lowerLayerIn;
        cutthroughLayer.upperLayerOut --> macLayer.lowerLayerIn;
        macLayer.upperLayerOut --> { @display("m=n"); } --> upperLayerOut;

        cutthroughLayer.cutthroughOut --> { @display("m=e"); } --> cutthroughOut;
        cutthroughIn --> { @display("m=e"); } --> cutthroughLayer.cutthroughIn;
}
