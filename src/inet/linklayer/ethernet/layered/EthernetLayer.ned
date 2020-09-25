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

module SimpleEthernetLayer like IEthernetLayer
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
        typeOrLengthInserter: <default("EthernetTypeOrLengthInserter")> like IPacketFlow {
            @display("p=200,100");
        }
        addressInserter: <default("EthernetAddressInserter")> like IPacketFlow {
            @display("p=200,200");
        }
        paddingInserter: <default("PaddingInserter")> like IPacketFlow {
            @display("p=200,300");
        }
        typeOrLengthChecker: <default("EthernetTypeOrLengthChecker")> like IPacketFilter {
            @display("p=500,100");
        }
        addressChecker: <default("EthernetAddressChecker")> like IPacketFilter {
            @display("p=500,200");
        }
    connections:
        addressInserter.out --> paddingInserter.in;
        typeOrLengthInserter.out --> addressInserter.in;
        typeOrLengthChecker.out --> upperLayerOut;
        paddingInserter.out --> lowerLayerOut;
        lowerLayerIn --> { @display("m=s"); } --> addressChecker.in;
        addressChecker.out --> typeOrLengthChecker.in;
        upperLayerIn --> typeOrLengthInserter.in;
}

module EthernetLayerWithQTags like IEthernetLayer
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
        cTagHeaderInserter: <default("Ieee8021qTagEpdHeaderInserter")> like IIeee8021qTagEpdHeaderInserter {
            vlanTagType = "c";
            @display("p=200,100");
        }
        sTagHeaderInserter: <default("Ieee8021qTagEpdHeaderInserter")> like IIeee8021qTagEpdHeaderInserter {
            @display("p=200,200");
        }
        macHeaderInserter: <default("EthernetMacHeaderInserter")> like IPacketFlow {
            @display("p=200,300");
        }
        paddingInserter: <default("PaddingInserter")> like IPacketFlow {
            @display("p=200,400");
        }
        cTagHeaderChecker: <default("Ieee8021qTagEpdHeaderChecker")> like IIeee8021qTagEpdHeaderChecker {
            vlanTagType = "c";
            @display("p=500,100");
        }
        sTagHeaderChecker: <default("Ieee8021qTagEpdHeaderChecker")> like IIeee8021qTagEpdHeaderChecker {
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
        c1: <default("PacketClassifier")> like IPacketClassifier {
            @display("p=200,300");
        }
        cTagHeaderInserter: <default("OmittedIeee8021qTagTpidHeaderInserter")> like IIeee8021qTagTpidHeaderInserter {
            vlanTagType = "c";
            @display("p=100,200");
        }
        sTagHeaderInserter: <default("OmittedIeee8021qTagTpidHeaderInserter")> like IIeee8021qTagTpidHeaderInserter {
            vlanTagType = "s";
            @display("p=300,200");
        }
        typeOrLengthInserter: <default("EthernetTypeOrLengthInserter")> like IPacketFlow {
            @display("p=200,400");
        }
        macHeaderInserter: <default("EthernetMacHeaderInserter")> like IPacketFlow {
            @display("p=200,500");
        }
        paddingInserter: <default("PaddingInserter")> like IPacketFlow {
            @display("p=200,600");
        }
        cTagHeaderChecker: <default("OmittedIeee8021qTagTpidHeaderChecker")> like IIeee8021qTagTpidHeaderChecker {
            vlanTagType = "c";
            @display("p=800,300");
        }
        sTagHeaderChecker: <default("OmittedIeee8021qTagTpidHeaderChecker")> like IIeee8021qTagTpidHeaderChecker {
            vlanTagType = "s";
            @display("p=600,300");
        }
        macHeaderChecker: <default("EthernetMacHeaderChecker")> like IPacketFilter {
            @display("p=700,500");
        }
        m1: PacketMultiplexer {
            @display("p=200,100");
        }
        c2: <default("PacketClassifier")> like IPacketClassifier {
            @display("p=700,200");
        }
        m2: PacketMultiplexer {
            @display("p=700,400");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> m1.in++;
        macHeaderInserter.out --> paddingInserter.in;
        typeOrLengthInserter.out --> macHeaderInserter.in;
        c2.out++ --> upperLayerOut;
        paddingInserter.out --> lowerLayerOut;
        lowerLayerIn --> { @display("m=s"); } --> macHeaderChecker.in;
        c1.out++ --> cTagHeaderInserter.in;
        c1.out++ --> sTagHeaderInserter.in;
        cTagHeaderInserter.out --> m1.in++;
        c1.out++ --> typeOrLengthInserter.in;
        sTagHeaderInserter.out --> m1.in++;
        m1.out --> c1.in;
        macHeaderChecker.out --> m2.in++;
        m2.out --> c2.in;
        c2.out++ --> sTagHeaderChecker.in;
        sTagHeaderChecker.out --> m2.in++;
        cTagHeaderChecker.out --> m2.in++;
        c2.out++ --> cTagHeaderChecker.in;
}
