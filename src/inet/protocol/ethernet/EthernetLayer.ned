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

import inet.protocol.ieee8021q.IIeee8021qChecker;
import inet.protocol.ieee8021q.IIeee8021qInserter;
import inet.queueing.common.PacketMultiplexer;
import inet.queueing.contract.IPacketClassifier;
import inet.queueing.contract.IPacketFilter;
import inet.queueing.contract.IPacketFlow;

module SimpleEthernetLayer
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
            @display("p=700,100");
        }
        addressChecker: <default("EthernetAddressChecker")> like IPacketFilter {
            @display("p=700,200");
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

module EthernetLayer
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
        ctagInserter: <default("OmittedIeee8021qInserter")> like IIeee8021qInserter {
            vlanTagType = "c";
            @display("p=100,200");
        }
        stagInserter: <default("OmittedIeee8021qInserter")> like IIeee8021qInserter {
            vlanTagType = "s";
            @display("p=300,200");
        }
        typeOrLengthInserter: <default("EthernetTypeOrLengthInserter")> like IPacketFlow {
            @display("p=200,400");
        }
        addressInserter: <default("EthernetAddressInserter")> like IPacketFlow {
            @display("p=200,500");
        }
        paddingInserter: <default("PaddingInserter")> like IPacketFlow {
            @display("p=200,600");
        }
        typeOrLengthChecker: <default("EthernetTypeOrLengthChecker")> like IPacketFilter {
            @display("p=700,100");
        }
        ctagChecker: <default("OmittedIeee8021qChecker")> like IIeee8021qChecker {
            vlanTagType = "c";
            @display("p=800,300");
        }
        stagChecker: <default("OmittedIeee8021qChecker")> like IIeee8021qChecker {
            vlanTagType = "s";
            @display("p=600,300");
        }
        addressChecker: <default("EthernetAddressChecker")> like IPacketFilter {
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
        addressInserter.out --> paddingInserter.in;
        typeOrLengthInserter.out --> addressInserter.in;
        typeOrLengthChecker.out --> upperLayerOut;
        paddingInserter.out --> lowerLayerOut;
        lowerLayerIn --> { @display("m=s"); } --> addressChecker.in;
        c1.out++ --> ctagInserter.in;
        c1.out++ --> stagInserter.in;
        ctagInserter.out --> m1.in++;
        c1.out++ --> typeOrLengthInserter.in;
        stagInserter.out --> m1.in++;
        m1.out --> c1.in;
        upperLayerIn --> m1.in++;
        addressChecker.out --> m2.in++;
        m2.out --> c2.in;
        c2.out++ --> stagChecker.in;
        stagChecker.out --> m2.in++;
        ctagChecker.out --> m2.in++;
        c2.out++ --> typeOrLengthChecker.in;
        c2.out++ --> ctagChecker.in;
}
