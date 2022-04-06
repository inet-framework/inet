//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

import inet.linklayer.ethernet.contract.IEthernetMacLayer;
import inet.protocolelement.contract.IPacketLifeTimer;
import inet.protocolelement.fragmentation.FragmentTagBasedDefragmenter;
import inet.queueing.contract.IPacketFilter;
import inet.queueing.contract.IPacketFlow;
import inet.queueing.contract.IPacketQueue;

module EthernetFragmentingMacLayer like IEthernetMacLayer
{
    parameters:
        double bitrate @unit(bps);
        *.bitrate = default(this.bitrate);
        lifetimer.collectionModule = default("^.queue");
        streamer.minPacketLength = 60B;
        outboundEmitter.signalName = "packetSentToLower";
        outboundEmitter.direction = "outbound";
        inboundEmitter.signalName = "packetReceivedFromLower";
        inboundEmitter.direction = "inbound";
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        lifetimer: <default("")> like IPacketLifeTimer if typename != "" {
            @display("p=100,100;is=s");
        }
        queue: <default("PacketQueue")> like IPacketQueue {
            @display("p=200,100");
        }
        fcsRemover: <default("EthernetFcsChecker")> like IPacketFilter {
            @display("p=200,200");
        }
        streamer: <default("PreemptableStreamer")> like IPacketFlow {
            @display("p=200,300");
        }
        fragmentFcsInserter: <default("EthernetFragmentFcsInserter")> like IPacketFlow {
            @display("p=200,400");
        }
        outboundEmitter: <default("PacketEmitter")> like IPacketFlow {
            @display("p=200,500");
        }
        fcsInserter: <default("EthernetFcsInserter")> like IPacketFlow {
            @display("p=500,200");
        }
        defragmenter: FragmentTagBasedDefragmenter {
            @display("p=500,300");
        }
        fragmentFcsChecker: <default("EthernetFragmentFcsChecker")> like IPacketFilter {
            @display("p=500,400");
        }
        inboundEmitter: <default("PacketEmitter")> like IPacketFlow {
            @display("p=500,500");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> queue.in;
        queue.out --> fcsRemover.in;
        fcsRemover.out --> streamer.in;
        streamer.out --> fragmentFcsInserter.in;
        fragmentFcsInserter.out --> outboundEmitter.in;
        outboundEmitter.out --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> inboundEmitter.in;
        inboundEmitter.out --> fragmentFcsChecker.in;
        fragmentFcsChecker.out --> defragmenter.in;
        defragmenter.out --> fcsInserter.in;
        fcsInserter.out --> { @display("m=n"); } --> upperLayerOut;
}
