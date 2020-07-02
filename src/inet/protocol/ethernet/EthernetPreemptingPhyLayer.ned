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

import inet.protocol.common.InterPacketGap;
import inet.protocol.transceiver.contract.IPacketReceiver;
import inet.protocol.transceiver.contract.IPacketTransmitter;
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
        outboundEmitter.protocolName = "ethernetphy";
        inboundEmitter.signalName = "packetReceivedFromLower";
        inboundEmitter.direction = "inbound";
        inboundEmitter.protocolName = "ethernetphy";
        *.bitrate = default(bitrate);
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
        preambleInserter: <default("EthernetPreambleInserter")> like IPacketFlow {
            @display("p=100,200");
        }
        fragmentPreambleInserter: <default("EthernetFragmentPreambleInserter")> like IPacketFlow {
            @display("p=300,200");
        }
        m1: PacketMultiplexer {
            @display("p=200,300");
        }
        interFrameGapInserter: InterPacketGap {
            duration = default(96s / dropUnit(bitrate));
            @display("p=200,400");
        }
        outboundEmitter: <default("PacketEmitter")> like IPacketFlow {
            @display("p=200,500");
        }
        transmitter: <default("PreemptibleTransmitter")> like IPacketTransmitter {
            datarate = default(bitrate);
            @display("p=200,600");
        }
        m2: PacketMultiplexer {
            @display("p=600,100");
        }
        fragmentPreambleChecker: <default("EthernetFragmentPreambleChecker")> like IPacketFilter {
            @display("p=700,200");
        }
        preambleChecker: <default("EthernetPreambleChecker")> like IPacketFilter {
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
            datarate = default(bitrate);
            @display("p=600,600");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> c1.in;
        c1.out++ --> preambleInserter.in;
        c1.out++ --> fragmentPreambleInserter.in;
        preambleInserter.out --> m1.in++;
        fragmentPreambleInserter.out --> m1.in++;
        m1.out --> interFrameGapInserter.in;
        interFrameGapInserter.out --> outboundEmitter.in;
        outboundEmitter.out --> transmitter.in;
        transmitter.out --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> receiver.in;
        receiver.out --> inboundEmitter.in;
        inboundEmitter.out --> c2.in;
        c2.out++ --> preambleChecker.in;
        c2.out++ --> fragmentPreambleChecker.in;
        preambleChecker.out --> m2.in++;
        fragmentPreambleChecker.out --> m2.in++;
        m2.out --> { @display("m=n"); } --> upperLayerOut;
}
