//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wired.ethernet;

import inet.protocolelement.common.InterpacketGapInserter;
import inet.protocolelement.transceiver.contract.IPacketReceiver;
import inet.protocolelement.transceiver.contract.IPacketTransmitter;
import inet.queueing.classifier.PacketClassifier;
import inet.queueing.common.PacketMultiplexer;
import inet.queueing.contract.IPacketFilter;
import inet.queueing.contract.IPacketFlow;

module EthernetPreemptingPhyLayer like IEthernetPhyLayer
{
    parameters:
        volatile double bitrate @unit(bps);
        outboundEmitter.signalName = "packetSentToLower";
        outboundEmitter.direction = "outbound";
        inboundEmitter.signalName = "packetReceivedFromLower";
        inboundEmitter.direction = "inbound";
        *.bitrate = default(this.bitrate);
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        c1: PacketClassifier {
            classifierClass = default("inet::PacketFragmentTagClassifier");
            @display("p=200,100");
        }
        phyHeaderInserter: <default("EthernetPhyHeaderInserter")> like IPacketFlow {
            @display("p=100,200");
        }
        fragmentPhyHeaderInserter: <default("EthernetFragmentPhyHeaderInserter")> like IPacketFlow {
            @display("p=300,200");
        }
        m1: PacketMultiplexer {
            @display("p=200,300");
        }
        interFrameGapInserter: InterpacketGapInserter {
            duration = default(96s / dropUnit(parent.bitrate));
            @display("p=200,400");
        }
        outboundEmitter: <default("PacketEmitter")> like IPacketFlow {
            @display("p=200,500");
        }
        transmitter: <default("StreamThroughTransmitter")> like IPacketTransmitter {
            datarate = default(parent.bitrate);
            @display("p=200,600");
        }
        m2: PacketMultiplexer {
            @display("p=600,100");
        }
        fragmentPhyHeaderChecker: <default("EthernetFragmentPhyHeaderChecker")> like IPacketFilter {
            @display("p=700,200");
        }
        phyHeaderChecker: <default("EthernetPhyHeaderChecker")> like IPacketFilter {
            @display("p=500,200");
        }
        c2: PacketClassifier {
            classifierClass = default("inet::PacketEthernetPreambleTypeClassifier");
            @display("p=600,300");
        }
        inboundEmitter: <default("PacketEmitter")> like IPacketFlow {
            @display("p=600,500");
        }
        receiver: <default("DestreamingReceiver")> like IPacketReceiver {
            datarate = default(parent.bitrate);
            @display("p=600,600");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> c1.in;
        c1.out++ --> phyHeaderInserter.in;
        c1.out++ --> fragmentPhyHeaderInserter.in;
        phyHeaderInserter.out --> m1.in++;
        fragmentPhyHeaderInserter.out --> m1.in++;
        m1.out --> interFrameGapInserter.in;
        interFrameGapInserter.out --> outboundEmitter.in;
        outboundEmitter.out --> transmitter.in;
        transmitter.out --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> receiver.in;
        receiver.out --> inboundEmitter.in;
        inboundEmitter.out --> c2.in;
        c2.out++ --> phyHeaderChecker.in;
        c2.out++ --> fragmentPhyHeaderChecker.in;
        phyHeaderChecker.out --> m2.in++;
        fragmentPhyHeaderChecker.out --> m2.in++;
        m2.out --> { @display("m=n"); } --> upperLayerOut;
}
