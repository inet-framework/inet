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

package inet.linklayer.ethernet;

import inet.linklayer.contract.IEthernetInterface;
import inet.linklayer.ethernet.contract.IEthernetMacLayer;
import inet.networklayer.common.NetworkInterface;
import inet.physicallayer.wired.ethernet.IEthernetPhyLayer;
import inet.protocolelement.measurement.IMeasurementLayer;
import inet.protocolelement.trafficconditioner.ITrafficConditionerLayer;

//
// This module implements an Ethernet network interface.
//
module LayeredEthernetInterface extends NetworkInterface like IEthernetInterface
{
    parameters:
        string protocol = default("ethernetmac");
        bool broadcast = default(true);
        bool multicast = default(true);
        string address = default("auto");
        double bitrate @unit(bps);
        string interfaceTableModule;
        *.bitrate = default(bitrate);
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input cutthroughIn @loose;
        output cutthroughOut @loose;
        inout phys;
    submodules:
        trafficConditionerLayer: <default("OmittedTrafficConditionerLayer")> like ITrafficConditionerLayer {
            parameters:
                @display("p=400,100");
        }
        measurementLayer: <default("OmittedMeasurementLayer")> like IMeasurementLayer {
            parameters:
                @display("p=400,200");
        }
        macLayer: <default("EthernetMacLayer")> like IEthernetMacLayer {
            parameters:
                @display("p=400,300");
        }
        phyLayer: <default("EthernetPhyLayer")> like IEthernetPhyLayer {
            parameters:
                @display("p=400,400");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> trafficConditionerLayer.upperLayerIn;
        trafficConditionerLayer.lowerLayerOut --> measurementLayer.upperLayerIn;
        measurementLayer.lowerLayerOut --> macLayer.upperLayerIn;
        macLayer.lowerLayerOut --> phyLayer.upperLayerIn;
        phyLayer.lowerLayerOut --> { @display("m=s"); } --> phys$o;

        phys$i --> { @display("m=s"); } --> phyLayer.lowerLayerIn;
        phyLayer.upperLayerOut --> macLayer.lowerLayerIn;
        macLayer.upperLayerOut --> measurementLayer.lowerLayerIn;
        measurementLayer.upperLayerOut --> trafficConditionerLayer.lowerLayerIn;
        trafficConditionerLayer.upperLayerOut --> { @display("m=n"); } --> upperLayerOut;
}

