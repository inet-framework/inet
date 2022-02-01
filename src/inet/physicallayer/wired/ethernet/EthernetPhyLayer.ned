//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wired.ethernet;

import inet.protocolelement.common.InterpacketGapInserter;
import inet.protocolelement.transceiver.contract.IPacketReceiver;
import inet.protocolelement.transceiver.contract.IPacketTransmitter;
import inet.queueing.contract.IPacketFilter;
import inet.queueing.contract.IPacketFlow;

module EthernetPhyLayer like IEthernetPhyLayer
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
        phyHeaderInserter: <default("EthernetPhyHeaderInserter")> like IPacketFlow {
            @display("p=200,100");
        }
        interFrameGapInserter: InterpacketGapInserter {
            duration = default(96s / dropUnit(parent.bitrate));  // 96 bit / <bitrate> bps
            @display("p=200,200");
        }
        outboundEmitter: <default("PacketEmitter")> like IPacketFlow {
            @display("p=200,300");
        }
        transmitter: <default("PacketTransmitter")> like IPacketTransmitter {
            datarate = default(parent.bitrate);
            @display("p=200,400");
        }
        phyHeaderChecker: <default("EthernetPhyHeaderChecker")> like IPacketFilter {
            @display("p=500,100");
        }
        inboundEmitter: <default("PacketEmitter")> like IPacketFlow {
            @display("p=500,300");
        }
        receiver: <default("PacketReceiver")> like IPacketReceiver {
            datarate = default(parent.bitrate);
            @display("p=500,400");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> phyHeaderInserter.in;
        phyHeaderInserter.out --> interFrameGapInserter.in;
        interFrameGapInserter.out --> outboundEmitter.in;
        outboundEmitter.out --> transmitter.in;
        transmitter.out --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> receiver.in;
        receiver.out --> inboundEmitter.in;
        inboundEmitter.out --> phyHeaderChecker.in;
        phyHeaderChecker.out --> { @display("m=n"); } --> upperLayerOut;
}
