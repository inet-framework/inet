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

package inet.linklayer.ethernet.layered;

import inet.linklayer.ieee8021ae.IIeee8021aeTagEpdHeaderChecker;
import inet.linklayer.ieee8021ae.IIeee8021aeTagEpdHeaderInserter;
import inet.linklayer.ieee8021q.IIeee8021qTagEpdHeaderChecker;
import inet.linklayer.ieee8021q.IIeee8021qTagEpdHeaderInserter;
import inet.linklayer.ieee8021q.IIeee8021qTagTpidHeaderChecker;
import inet.linklayer.ieee8021q.IIeee8021qTagTpidHeaderInserter;
import inet.queueing.common.PacketMultiplexer;
import inet.queueing.contract.IPacketClassifier;
import inet.queueing.contract.IPacketFilter;
import inet.queueing.contract.IPacketFlow;

module EthernetLayer like IEthernetLayer
{
    parameters:
        string interfaceTableModule;
        *.interfaceTableModule = default(interfaceTableModule);
        paddingInserter.minLength = default(60B);
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        cTagHeaderInserter: <default("OmittedIeee8021qTagEpdHeaderInserter")> like IIeee8021qTagEpdHeaderInserter {
            vlanTagType = "c";
            @display("p=200,100");
        }
        sTagHeaderInserter: <default("OmittedIeee8021qTagEpdHeaderInserter")> like IIeee8021qTagEpdHeaderInserter {
            vlanTagType = "s";
            @display("p=200,200");
        }
        macHeaderInserter: <default("EthernetMacHeaderInserter")> like IPacketFlow {
            @display("p=200,300");
        }
        paddingInserter: <default("PaddingInserter")> like IPacketFlow {
            @display("p=200,400");
        }
        cTagHeaderChecker: <default("OmittedIeee8021qTagEpdHeaderChecker")> like IIeee8021qTagEpdHeaderChecker {
            vlanTagType = "c";
            @display("p=500,100");
        }
        sTagHeaderChecker: <default("OmittedIeee8021qTagEpdHeaderChecker")> like IIeee8021qTagEpdHeaderChecker {
            vlanTagType = "s";
            @display("p=500,200");
        }
        macHeaderChecker: <default("EthernetMacHeaderChecker")> like IPacketFilter {
            @display("p=500,300");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> cTagHeaderInserter.in;
        cTagHeaderInserter.out --> sTagHeaderInserter.in;
        sTagHeaderInserter.out --> macHeaderInserter.in;
        macHeaderInserter.out --> paddingInserter.in;
        paddingInserter.out --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> macHeaderChecker.in;
        macHeaderChecker.out --> sTagHeaderChecker.in;
        sTagHeaderChecker.out --> cTagHeaderChecker.in;
        cTagHeaderChecker.out --> { @display("m=n"); } --> upperLayerOut;
}
