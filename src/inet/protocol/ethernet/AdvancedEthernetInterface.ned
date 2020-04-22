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

import inet.linklayer.contract.IEthernetInterface;
import inet.networklayer.common.InterfaceEntry;
import inet.protocol.common.InterPacketGap;
import inet.protocol.common.PacketEmitter;
import inet.protocol.common.PaddingInserter;
import inet.protocol.common.ProtocolChecker;
import inet.protocol.fragmentation.FragmentTagBasedDefragmenter;
import inet.protocol.ieee8021q.IIeee8021qTagger;
import inet.protocol.ieee8022.IIeee8022LlcChecker;
import inet.protocol.ieee8022.IIeee8022LlcInserter;
import inet.protocol.server.PreemptingServer;
import inet.protocol.transceiver.PreemptibleTransmitter;
import inet.protocol.transceiver.StreamingReceiver;
import inet.queueing.classifier.PacketClassifier;
import inet.queueing.common.PacketMultiplexer;
import inet.queueing.contract.ITrafficConditioner;
import inet.queueing.queue.PacketQueue;
import inet.queueing.scheduler.PriorityScheduler;

// TODO: add VLAN priority queueing (e.g. https://erg.abdn.ac.uk/users/gorry/course/lan-pages/vlan.html)
// TODO: add periodically open/closed gates to compound priority queues
// TODO: add socket handling
// TODO: dispatch for LLC/ethertype
// TODO: connect/disconnect handling
module AdvancedEthernetInterface extends InterfaceEntry like IEthernetInterface
{
    parameters:
        bool broadcast = default(true);
        bool multicast = default(true);
        string address = default("auto");
        double bitrate @unit(bps);
        string interfaceTableModule;
        @class(InterfaceEntry);
        @display("i=block/ifcard");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        inout phys;
    submodules:
        ingressConditioner: <default("OmittedTrafficConditioner")> like ITrafficConditioner {
            @display("p=300,100");
        }
        llcInserter: <default("OmittedIeee8022LlcInserter")> like IIeee8022LlcInserter {
            @display("p=300,200");
        }
        typeOrLengthInserter: EthernetTypeOrLengthInserter {
            @display("p=300,300");
        }
        ctagInserter: <default("OmittedIeee8021qTagger")> like IIeee8021qTagger {
            vlanTagType = "c";
            @display("p=300,400");
        }
        stagInserter: <default("OmittedIeee8021qTagger")> like IIeee8021qTagger {
            vlanTagType = "s";
            @display("p=300,500");
        }
        addressInserter: EthernetAddressInserter {
            @display("p=300,600");
        }
        paddingInserter: PaddingInserter {
            minLength = default(60B);
            @display("p=300,700");
        }
        priorityClassifier1: PacketClassifier {
            classifierClass = default("inet::EthernetExpressOverNormalClassifier");
            @display("p=300,800");
        }
        expressQueue: PacketQueue {
            @display("p=100,900");
        }
        preemptedQueue: PacketQueue {
            @display("p=300,900");
        }
        normalQueue: PacketQueue {
            @display("p=500,900");
        }
        priorityScheduler: PriorityScheduler {
            @display("p=200,1000");
        }
        preemptingServer: PreemptingServer {
            minPacketLength = 60B;
            @display("p=300,1100");
        }
        priorityClassifier2: PacketClassifier {
            classifierClass = default("inet::EthernetExpressOverNormalClassifier");
            @display("p=300,1200");
        }
        expressFcsInserter: EthernetFcsInserter {
            @display("p=150,1300");
        }
        expressPreambleInserter: EthernetPreambleInserter {
            @display("p=150,1400");
        }
        fragmentFcsInserter: EthernetFragmentFcsInserter {
            @display("p=450,1300");
        }
        fragmentPreambleInserter: EthernetFragmentPreambleInserter {
            @display("p=450,1400");
        }
        m1: PacketMultiplexer {
            @display("p=300,1500");
        }
        outboundEmitter: PacketEmitter {
            signalName = "packetSentToLower";
            direction = "outbound";
            protocolName = "ethernetphy";
            @display("p=300,1600");
        }
        interFrameGapInserter: InterPacketGap {
            duration = default(96s / dropUnit(bitrate));
            @display("p=300,1700");
        }
        transmitter: PreemptibleTransmitter {
            datarate = default(bitrate);
            @display("p=300,1800");
        }
        protocolChecker: ProtocolChecker {
            @display("p=950,100");
        }
        egressConditioner: <default("OmittedTrafficConditioner")> like ITrafficConditioner {
            @display("p=950,200");
        }
        llcChecker: <default("OmittedIeee8022LlcChecker")> like IIeee8022LlcChecker {
            @display("p=950,300");
        }
        typeOrLengthChecker: EthernetTypeOrLengthChecker {
            @display("p=950,400");
        }
        ctagChecker: <default("OmittedIeee8021qTagger")> like IIeee8021qTagger {
            vlanTagType = "c";
            @display("p=950,500");
        }
        stagChecker: <default("OmittedIeee8021qTagger")> like IIeee8021qTagger {
            vlanTagType = "s";
            @display("p=950,600");
        }
        addressChecker: EthernetAddressChecker {
            @display("p=950,700");
        }
        m2: PacketMultiplexer {
            @display("p=950,1200");
        }
        expressFcsChecker: EthernetFcsChecker {
            @display("p=800,1400");
        }
        expressPreambleChecker: EthernetPreambleChecker {
            @display("p=800,1500");
        }
        defragmenter: FragmentTagBasedDefragmenter {
            @display("p=1100,1300");
        }
        fragmentFcsChecker: EthernetFragmentFcsChecker {
            @display("p=1100,1400");
        }
        fragmentPreambleChecker: EthernetFragmentPreambleChecker {
            @display("p=1100,1500");
        }
        preambleClassifier: PacketClassifier {
            classifierClass = default("inet::EthernetPreambleClassifier");
            @display("p=950,1600");
        }
        inboundEmitter: PacketEmitter {
            signalName = "packetReceivedFromLower";
            direction = "inbound";
            protocolName = "ethernetphy";
            @display("p=950,1700");
        }
        receiver: StreamingReceiver {
            datarate = default(bitrate);
            @display("p=950,1800");
        }
    connections:
        upperLayerIn --> ingressConditioner.in;
        ingressConditioner.out --> llcInserter.in;
        llcInserter.out --> typeOrLengthInserter.in;
        typeOrLengthInserter.out --> ctagInserter.in;
        ctagInserter.out --> stagInserter.in;
        stagInserter.out --> addressInserter.in;
        addressInserter.out --> paddingInserter.in;
        paddingInserter.out --> priorityClassifier1.in;
        priorityClassifier1.out++ --> normalQueue.in;
        priorityClassifier1.out++ --> expressQueue.in;
        expressQueue.out --> priorityScheduler.in++;
        preemptedQueue.out --> priorityScheduler.in++;
        normalQueue.out --> priorityScheduler.in++;
        priorityScheduler.out --> preemptingServer.in;
        preemptingServer.preemptedOut --> preemptedQueue.in;
        preemptingServer.out --> priorityClassifier2.in;
        priorityClassifier2.out++ --> fragmentFcsInserter.in;
        priorityClassifier2.out++ --> expressFcsInserter.in;
        expressFcsInserter.out --> expressPreambleInserter.in;
        expressPreambleInserter.out --> m1.in++;
        fragmentFcsInserter.out --> fragmentPreambleInserter.in;
        fragmentPreambleInserter.out --> m1.in++;
        m1.out --> outboundEmitter.in;
        outboundEmitter.out --> interFrameGapInserter.in;
        interFrameGapInserter.out --> transmitter.in;
        transmitter.out --> phys$o;

        phys$i --> receiver.in;
        receiver.out --> inboundEmitter.in;
        inboundEmitter.out --> preambleClassifier.in;
        preambleClassifier.out++ --> expressPreambleChecker.in;
        preambleClassifier.out++ --> fragmentPreambleChecker.in;
        expressPreambleChecker.out --> expressFcsChecker.in;
        expressFcsChecker.out --> m2.in++;
        fragmentPreambleChecker.out --> fragmentFcsChecker.in;
        fragmentFcsChecker.out --> defragmenter.in;
        defragmenter.out --> m2.in++;
        m2.out --> addressChecker.in;
        addressChecker.out --> stagChecker.in;
        stagChecker.out --> ctagChecker.in;
        ctagChecker.out --> typeOrLengthChecker.in;
        typeOrLengthChecker.out --> llcChecker.in;
        llcChecker.out --> egressConditioner.in;
        egressConditioner.out --> protocolChecker.in;
        protocolChecker.out --> upperLayerOut;
}
