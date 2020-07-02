//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.node.base;

import inet.common.MessageDispatcher;
import inet.common.packet.recorder.PcapRecorder;
import inet.linklayer.contract.IEthernetInterface;
import inet.linklayer.contract.ILoopbackInterface;
import inet.linklayer.contract.IPppInterface;
import inet.linklayer.contract.ITunnelInterface;
import inet.linklayer.contract.IVlanInterface;
import inet.linklayer.contract.IWirelessInterface;
import inet.linklayer.ethernet.switch.MacRelayUnit;
import inet.networklayer.common.InterfaceTable;
import inet.protocol.ethernet.SimpleEthernetLayer;
import inet.protocol.ieee8021q.IIeee8021qChecker;
import inet.protocol.ieee8021q.IIeee8021qInserter;
import inet.queueing.common.PacketMultiplexer;
import inet.queueing.contract.IPacketClassifier;
import inet.queueing.contract.IPacketFilter;
import inet.queueing.contract.IPacketFlow;


module EthernetLayerSplitted
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
            @display("p=200,199");
        }
        ctagInserter: <default("OmittedIeee8021qInserter")> like IIeee8021qInserter {
            vlanTagType = "c";
            @display("p=50,199");
        }
        stagInserter: <default("OmittedIeee8021qInserter")> like IIeee8021qInserter {
            vlanTagType = "s";
            @display("p=350,199");
        }
        addressInserter: <default("EthernetAddressInserter")> like IPacketFlow {
            @display("p=200,400");
        }
        paddingInserter: <default("PaddingInserter")> like IPacketFlow {
            @display("p=200,500");
        }
        typeOrLengthChecker: <default("EthernetTypeOrLengthChecker")> like IPacketFilter {
            @display("p=500,300");
        }
        ctagChecker: <default("OmittedIeee8021qChecker")> like IIeee8021qChecker {
            vlanTagType = "c";
            @display("p=650,300");
        }
        stagChecker: <default("OmittedIeee8021qChecker")> like IIeee8021qChecker {
            vlanTagType = "s";
            @display("p=350,300");
        }
        addressChecker: <default("EthernetAddressChecker")> like IPacketFilter {
            @display("p=500,500");
        }
        messageDispatcher: MessageDispatcher {
            @display("p=500,199;b=10,10");
        }
        packetMultiplexer: PacketMultiplexer {
            @display("p=500,400");
        }
        classifier: <default("PacketClassifier")> like IPacketClassifier {
            @display("p=200,298");
        }
        packetMultiplexer1: PacketMultiplexer {
            @display("p=200,90");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> packetMultiplexer1.in++;
        addressInserter.out --> paddingInserter.in;
        paddingInserter.out --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> addressChecker.in;
        typeOrLengthChecker.out --> messageDispatcher.in++;
        messageDispatcher.out++ --> stagChecker.in;
        messageDispatcher.out++ --> ctagChecker.in;
        messageDispatcher.out++ --> upperLayerOut;
        ctagChecker.out --> packetMultiplexer.in++;
        stagChecker.out --> packetMultiplexer.in++;
        addressChecker.out --> packetMultiplexer.in++;
        packetMultiplexer.out --> typeOrLengthChecker.in;
        typeOrLengthInserter.out --> classifier.in;
        classifier.out++ --> addressInserter.in;
        classifier.out++ --> ctagInserter.in;
        classifier.out++ --> stagInserter.in;
        packetMultiplexer1.out --> typeOrLengthInserter.in;
        ctagInserter.out --> packetMultiplexer1.in++;
        stagInserter.out --> packetMultiplexer1.in++;
}

module EthernetLayerCurrent
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
            @display("p=200,76");
        }
        ctagInserter: <default("OmittedIeee8021qInserter")> like IIeee8021qInserter {
            vlanTagType = "c";
            @display("p=61,253");
        }
        stagInserter: <default("OmittedIeee8021qInserter")> like IIeee8021qInserter {
            vlanTagType = "s";
            @display("p=329,253");
        }
        addressInserter: <default("EthernetAddressInserter")> like IPacketFlow {
            @display("p=200,400");
        }
        paddingInserter: <default("PaddingInserter")> like IPacketFlow {
            @display("p=200,500");
        }
        typeOrLengthChecker: <default("EthernetTypeOrLengthChecker")> like IPacketFilter {
            @display("p=500,82");
        }
        ctagChecker: <default("OmittedIeee8021qChecker")> like IIeee8021qChecker {
            vlanTagType = "c";
            @display("p=649,347");
        }
        stagChecker: <default("OmittedIeee8021qChecker")> like IIeee8021qChecker {
            vlanTagType = "s";
            @display("p=360,347");
        }
        addressChecker: <default("EthernetAddressChecker")> like IPacketFilter {
            @display("p=500,500");
        }
        classifier: <default("PacketClassifier")> like IPacketClassifier {
            @display("p=200,298");
        }
        packetMultiplexer1: PacketMultiplexer {
            @display("p=200,199");
        }
        classifier1: <default("PacketClassifier")> like IPacketClassifier {
            @display("p=500,300");
        }
        packetMultiplexer: PacketMultiplexer {
            @display("p=500,400");
        }
        macRelayUnit: MacRelayUnit {
            @display("p=344,153");
        }
    connections:
        addressInserter.out --> paddingInserter.in;
        paddingInserter.out --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> addressChecker.in;
        classifier.out++ --> addressInserter.in;
        classifier.out++ --> ctagInserter.in;
        classifier.out++ --> stagInserter.in;
        ctagInserter.out --> packetMultiplexer1.in++;
        stagInserter.out --> packetMultiplexer1.in++;
        upperLayerIn --> typeOrLengthInserter.in;
        typeOrLengthInserter.out --> packetMultiplexer1.in++;
        packetMultiplexer1.out --> classifier.in;
        typeOrLengthChecker.out --> upperLayerOut;
        classifier1.out++ --> stagChecker.in;
        classifier1.out++ --> ctagChecker.in;
        addressChecker.out --> packetMultiplexer.in++;
        packetMultiplexer.out --> classifier1.in;
        stagChecker.out --> packetMultiplexer.in++;
        ctagChecker.out --> packetMultiplexer.in++;
        classifier1.out++ --> typeOrLengthChecker.in;
}

module EthernetLayerShifted
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
            @display("p=200,300");
        }
        ctagInserter: <default("OmittedIeee8021qInserter")> like IIeee8021qInserter {
            vlanTagType = "c";
            @display("p=61,134");
        }
        stagInserter: <default("OmittedIeee8021qInserter")> like IIeee8021qInserter {
            vlanTagType = "s";
            @display("p=335,134");
        }
        addressInserter: <default("EthernetAddressInserter")> like IPacketFlow {
            @display("p=200,400");
        }
        paddingInserter: <default("PaddingInserter")> like IPacketFlow {
            @display("p=200,500");
        }
        typeOrLengthChecker: <default("EthernetTypeOrLengthChecker")> like IPacketFilter {
            @display("p=500,400");
        }
        ctagChecker: <default("OmittedIeee8021qChecker")> like IIeee8021qChecker {
            vlanTagType = "c";
            @display("p=622,300");
        }
        stagChecker: <default("OmittedIeee8021qChecker")> like IIeee8021qChecker {
            vlanTagType = "s";
            @display("p=394,300");
        }
        addressChecker: <default("EthernetAddressChecker")> like IPacketFilter {
            @display("p=500,500");
        }
        classifier: <default("PacketClassifier")> like IPacketClassifier {
            @display("p=200,188");
        }
        packetMultiplexer1: PacketMultiplexer {
            @display("p=200,82");
        }
        messageDispatcher: MessageDispatcher {
            @display("p=500,134;b=10,10");
        }
    connections:
        addressInserter.out --> paddingInserter.in;
        paddingInserter.out --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> addressChecker.in;
        ctagInserter.out --> packetMultiplexer1.in++;
        stagInserter.out --> packetMultiplexer1.in++;
        classifier.out++ --> typeOrLengthInserter.in;
        classifier.out++ --> ctagInserter.in;
        classifier.out++ --> stagInserter.in;
        upperLayerIn --> packetMultiplexer1.in++;
        packetMultiplexer1.out --> classifier.in;
        typeOrLengthInserter.out --> addressInserter.in;
        addressChecker.out --> typeOrLengthChecker.in;
        typeOrLengthChecker.out --> messageDispatcher.in++;
        messageDispatcher.out++ --> stagChecker.in;
        messageDispatcher.out++ --> ctagChecker.in;
        messageDispatcher.out++ --> upperLayerOut;
        ctagChecker.out --> messageDispatcher.in++;
        stagChecker.out --> messageDispatcher.in++;
}
