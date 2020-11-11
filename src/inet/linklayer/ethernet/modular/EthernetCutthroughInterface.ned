//
// Copyright (C) 2020 OpenSim Ltd.
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

package inet.linklayer.ethernet.modular;

import inet.linklayer.contract.IEthernetInterface;
import inet.linklayer.ethernet.contract.IEthernetMacLayer;
import inet.networklayer.common.NetworkInterface;
import inet.physicallayer.wired.ethernet.IEthernetPhyLayer;

//
// This module represents an Ethernet network interface with cut-through support.
//
module EthernetCutthroughInterface extends NetworkInterface like IEthernetInterface
{
    parameters:
        string protocol = default("ethernetmac");
        bool broadcast = default(true);
        bool multicast = default(true);
        string address = default("auto");
        double bitrate @unit(bps);
        string interfaceTableModule;
        string macTableModule;
        *.bitrate = default(bitrate);
        *.macTableModule = default(macTableModule);
        @class(EthernetCutthroughInterface);
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input cutthroughIn;
        output cutthroughOut;
        inout phys;
    submodules:
        macLayer: <default("EthernetMacLayer")> like IEthernetMacLayer {
            parameters:
                @display("p=400,100");
        }
        cutthroughLayer: EthernetCutthroughLayer {
            parameters:
                @display("p=400,200");
        }
        phyLayer: <default("EthernetStreamThroughPhyLayer")> like IEthernetPhyLayer {
            parameters:
                @display("p=400,300");
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
        cutthroughIn --> { @display("m=w"); } --> cutthroughLayer.cutthroughIn;
}

