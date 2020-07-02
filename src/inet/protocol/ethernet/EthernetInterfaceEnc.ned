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
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
//

package inet.protocol.ethernet;

import inet.common.MessageMultiplexer;
import inet.common.PacketCommandClassifier;
import inet.linklayer.contract.IEthernetInterface;
import inet.linklayer.ethernet.IEtherEncap;
import inet.networklayer.common.InterfaceEntry;
import inet.protocol.common.InterPacketGap;
import inet.protocol.common.PacketSerializer;
import inet.protocol.common.PacketTrimmer;
import inet.protocol.common.PaddingInserter;
import inet.protocol.common.ProtocolChecker;
import inet.protocol.connectionmanager.ConnectionManager;
import inet.protocol.ieee8021q.IIeee8021qTagger;
import inet.protocol.ieee8022.IIeee8022LlcChecker;
import inet.protocol.ieee8022.IIeee8022LlcInserter;
import inet.protocol.transceiver.StreamingReceiver;
import inet.protocol.transceiver.StreamingTransmitter;
import inet.queueing.classifier.PacketClassifier;
import inet.queueing.common.PacketMultiplexer;
import inet.queueing.contract.IPacketFilter;
import inet.queueing.contract.IPacketQueue;
import inet.queueing.contract.ITrafficConditioner;
import inet.queueing.server.InstantServer;


module EtherEncapModularized like IEtherEncap
{
    parameters:
        //        string interfaceTableModule;   // The path to the InterfaceTable module
        bool useSNAP = default(false);  // create EtherFrameWithSNAP frames instead of EthernetIIFrame
        //        string fcsMode @enum("declared", "computed");
        //        bool registerProtocol = default(false);     //FIXME //KLUDGE should redesign place of EtherEncap and LLC modules
        //        @lifecycleSupport;
        //        double stopOperationExtraTime @unit(s) = default(-1s);    // extra time after lifecycle stop operation finished
        //        double stopOperationTimeout @unit(s) = default(2s);    // timeout value for lifecycle stop operation
        //        @display("i=block/square");
        llcInserter.useSNAP = useSNAP;
    gates:
        input upperLayerIn @labels(Ieee802Ctrl/down); // higher layer
        output upperLayerOut @labels(Ieee802Ctrl/up); // higher layer
        input lowerLayerIn @labels(EtherFrame);   // to Ethernet MAC
        output lowerLayerOut @labels(EtherFrame); // to Ethernet MAC
    submodules:
        ethPauseCommandProcessor: EthernetPauseCommandProcessor {
            @display("p=301,78");
        }
        ethSocketCommandProcessor: EthernetSocketCommandProcessor {
            socketTableModule = "^.ethernetSocketTable";
            @display("p=301,168");
        }
        llcSocketCommandProcessor: LlcSocketCommandProcessor {
            socketTableModule = "^.llcSocketTable";
            @display("p=301,250");
        }
        llcInserter: <default("Ieee8022LlcInserter")> like IIeee8022LlcInserter {
            @display("p=200,400");
        }
        typeOrLengthInserter: EthernetTypeOrLengthInserter {
            @display("p=200,500");
        }
        ctagInserter: <default("OmittedIeee8021qTagger")> like IIeee8021qTagger {
            vlanTagType = "c";
            @display("p=200,600");
        }
        stagInserter: <default("OmittedIeee8021qTagger")> like IIeee8021qTagger {
            vlanTagType = "s";
            @display("p=200,700");
        }
        addressInserter: EthernetAddressInserter {
            @display("p=200,800");
        }
        addressChecker: EthernetAddressChecker {
            @display("p=500,800");
        }
        stagChecker: <default("OmittedIeee8021qTagger")> like IIeee8021qTagger {
            vlanTagType = "s";
            @display("p=500,700");
        }
        ctagChecker: <default("OmittedIeee8021qTagger")> like IIeee8021qTagger {
            vlanTagType = "c";
            @display("p=500,600");
        }
        typeOrLengthChecker: EthernetTypeOrLengthChecker {
            @display("p=500,500");
        }
        llcClassifier: PacketClassifier {
            classifierClass = default("inet::EthernetLlcClassifier");
            @display("p=500,400");
        }
        llcChecker: <default("Ieee8022LlcChecker")> like IIeee8022LlcChecker {
            @display("p=600,350");
        }
        protocolChecker: ProtocolChecker {
            @display("p=500,100");
        }
        m2: MessageMultiplexer {
            @display("p=500,20");
        }
        ethSocketPacketProcessor: EthernetSocketPacketProcessor {
            socketTableModule = "^.ethernetSocketTable";
            @display("p=400,450");
        }
        m1: PacketMultiplexer {
            @display("p=500,200");
        }
        llcSocketPacketProcessor: LlcSocketPacketProcessor {
            socketTableModule = "^.llcSocketTable";
            @display("p=600,250");
        }
        ethernetSocketTable: EthernetSocketTable {
            @display("p=100,150");
        }
        llcSocketTable: LlcSocketTable {
            @display("p=100,250");
        }
        c1: PacketCommandClassifier {
            @display("p=200,24");
        }
        m3: PacketMultiplexer {
            @display("p=200,311");
        }
    connections:
        upperLayerIn --> c1.in;
        llcInserter.out --> typeOrLengthInserter.in;
        typeOrLengthInserter.out --> ctagInserter.in;
        ctagInserter.out --> stagInserter.in;
        stagInserter.out --> addressInserter.in;
        addressInserter.out --> lowerLayerOut;
        ethSocketCommandProcessor.cmdOut --> m2.in++;
        llcSocketCommandProcessor.cmdOut --> m2.in++;

        lowerLayerIn --> addressChecker.in;
        addressChecker.out --> stagChecker.in;
        stagChecker.out --> ctagChecker.in;
        ctagChecker.out --> typeOrLengthChecker.in;
        typeOrLengthChecker.out --> ethSocketPacketProcessor.in;
        m1.out --> protocolChecker.in;
        protocolChecker.out --> m2.in++;
        ethPauseCommandProcessor.out --> ethSocketCommandProcessor.in;
        ethSocketCommandProcessor.out --> llcSocketCommandProcessor.in;
        m3.out --> llcInserter.in;
        m2.out --> upperLayerOut;
        ethSocketPacketProcessor.out --> llcClassifier.in;
        ethSocketPacketProcessor.socketOut --> m2.in++;
        llcClassifier.out++ --> m1.in++;
        llcClassifier.out++ --> llcChecker.in;
        llcChecker.out --> llcSocketPacketProcessor.in;
        llcSocketPacketProcessor.out --> m1.in++;
        llcSocketPacketProcessor.socketOut --> m2.in++;
        c1.messageOut --> ethPauseCommandProcessor.in;
        llcSocketCommandProcessor.out --> m3.in++;
        c1.packetOut --> m3.in++;
}

module EtherQEncapModularized like IEtherEncap
{
    gates:
        input upperLayerIn @labels(Ieee802Ctrl/down); // higher layer
        output upperLayerOut @labels(Ieee802Ctrl/up); // higher layer
        input lowerLayerIn @labels(EtherFrame);   // to Ethernet MAC
        output lowerLayerOut @labels(EtherFrame); // to Ethernet MAC
    submodules:
        addressremover: EthernetAddressChecker {
//            indication = false;
            promiscuous = true;
            @display("p=200,200");
        }
        ctagInserter: <default("OmittedIeee8021qTagger")> like IIeee8021qTagger {
            vlanTagType = "c";
            @display("p=200,300");
        }
        stagInserter: <default("OmittedIeee8021qTagger")> like IIeee8021qTagger {
            vlanTagType = "s";
            @display("p=200,400");
        }
        addressInserter: EthernetAddressInserter {
            @display("p=200,600");
        }
        addressChecker: EthernetAddressChecker {
            promiscuous = true;
            @display("p=500,600");
        }
        stagChecker: <default("OmittedIeee8021qTagger")> like IIeee8021qTagger {
            vlanTagType = "s";
            @display("p=500,500");
        }
        ctagChecker: <default("OmittedIeee8021qTagger")> like IIeee8021qTagger {
            vlanTagType = "c";
            @display("p=500,400");
        }
        addressReInserter: EthernetAddressInserter {
//            request = false;
            @display("p=500,200");
        }

        packetTrimmer: PacketTrimmer {
            @display("p=500,300");
        }
        packetSerializer: PacketSerializer {
            @display("p=500,100");
        }
        packetTrimmer1: PacketTrimmer {
            @display("p=200,500");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> addressremover.in;
        addressremover.out --> ctagInserter.in;
        ctagInserter.out --> stagInserter.in;
        stagInserter.out --> packetTrimmer1.in;
        addressInserter.out --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> addressChecker.in;
        addressChecker.out --> stagChecker.in;
        stagChecker.out --> ctagChecker.in;
        ctagChecker.out --> packetTrimmer.in;
        packetSerializer.out --> { @display("m=n"); } --> upperLayerOut;
        packetTrimmer.out --> addressReInserter.in;
        addressReInserter.out --> packetSerializer.in;
        packetTrimmer1.out --> addressInserter.in;
}

module EthFakeMac
{
    parameters:
    gates:
        input in;
        output out;
    submodules:
        queue: <default("EtherQueue")> like IPacketQueue {
            parameters:
                packetCapacity = default(1000);
                @display("p=100,100");
        }
    connections:
        in --> { @display("m=n"); } --> queue.in;
        queue.out --> { @display("m=s"); } --> out;
}

module EthernetInterfaceEnc extends InterfaceEntry like IEthernetInterface
{
    parameters:
        @class(InterfaceEntry);
        @lifecycleSupport;
        @display("i=block/ifcard");
        string interfaceTableModule;
        string address = default("auto");
        bool broadcast = default(true);
        bool multicast = default(true);
        bool promiscuous = default(false);  // if true, all packets are received, otherwise only the
                                            // ones with matching destination MAC address
        double bitrate @unit(bps) = default(nan bps);
        int mtu @unit(B) = default(1500B);
        string fcsMode @enum("declared","computed") = default("declared");
        padding.minLength = 60B;
        ifg.duration = 96s/dropUnit(bitrate);
        transmitter.datarate = bitrate;
        receiver.datarate = bitrate;
        encap.addressChecker.promiscuous = promiscuous;
        encap.fcsMode = fcsMode;
        encap.*.interfaceTableModule = default(absPath(interfaceTableModule));
        ingressTC.interfaceTableModule = default(absPath(interfaceTableModule));
        egressTC.interfaceTableModule = default(absPath(interfaceTableModule));
    gates:
        input upperLayerIn;
        output upperLayerOut;
        inout phys;
    submodules:
        egressTC: <default("OmittedTrafficConditioner")> like ITrafficConditioner {
            @display("p=200,100");
        }
        ingressTC: <default("OmittedTrafficConditioner")> like ITrafficConditioner {
            @display("p=500,100");
        }
        encap: <default("EtherEncapModularized")> like IEtherEncap {
            parameters:
                @display("p=350,150");
        }
        mac: EthFakeMac {
            parameters:
                @display("p=200,200");
        }
        server: InstantServer {
            parameters:
                @display("p=200,300");
        }
        packetTrimmer: PacketTrimmer {
            @display("p=200,400");
        }
        padding: PaddingInserter {
            parameters:
                @display("p=200,500");
        }
        fcsInserter: EthernetFcsInserter {
            parameters:
                @display("p=200,600");
        }
        preambleInserter: EthernetPreambleInserter {
            parameters:
                @display("p=200,700");
        }
        serializer: PacketSerializer {
            parameters:
                @display("p=200,800");
        }
        ifg: InterPacketGap {
            parameters:
                @display("p=200,900");
        }
        transmitter: StreamingTransmitter {
            parameters:
                @display("p=200,1000");
        }

        cm: ConnectionManager {
            parameters:
                @display("p=350,1100");
        }

        fcsChecker: EthernetFcsChecker {
            parameters:
                @display("p=500,800");
        }
        preambleChecker: EthernetPreambleChecker {
            parameters:
                @display("p=500,900");
        }
        receiver: StreamingReceiver {
            parameters:
                @display("p=500,1000");
        }

        channelDatarateReader: ChannelDatarateReader {
            @display("p=100,1100");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> egressTC.in;
        egressTC.out --> encap.upperLayerIn;

        encap.upperLayerOut --> ingressTC.in;
        ingressTC.out --> { @display("m=n"); } --> upperLayerOut;

        encap.lowerLayerOut --> mac.in;

        mac.out --> server.in;
        server.out --> packetTrimmer.in;
        packetTrimmer.out --> padding.in;
        padding.out --> fcsInserter.in;
        fcsInserter.out --> preambleInserter.in;
        preambleInserter.out --> serializer.in;
        serializer.out --> ifg.in;
        ifg.out --> transmitter.in;
        transmitter.out --> cm.in;
        cm.physOut --> { @display("m=s"); } --> phys$o;

        fcsChecker.out --> encap.lowerLayerIn;
        preambleChecker.out --> fcsChecker.in;
        receiver.out --> preambleChecker.in;
        cm.out --> receiver.in;
        phys$i --> { @display("m=s"); } --> cm.physIn;
}

