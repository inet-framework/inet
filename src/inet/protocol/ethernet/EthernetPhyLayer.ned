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
import inet.queueing.contract.IPacketFilter;
import inet.queueing.contract.IPacketFlow;

module EthernetPhyLayer like IEthernetPhyLayer
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
        preambleInserter: <default("EthernetPreambleInserter")> like IPacketFlow {
            @display("p=200,100");
        }
        interFrameGapInserter: InterPacketGap {
            duration = default(96s / dropUnit(bitrate));
            @display("p=200,200");
        }
        outboundEmitter: <default("PacketEmitter")> like IPacketFlow {
            @display("p=200,300");
        }
        transmitter: <default("PreemptibleTransmitter")> like IPacketTransmitter {
            datarate = default(bitrate);
            @display("p=200,400");
        }
        preambleChecker: <default("EthernetPreambleChecker")> like IPacketFilter {
            @display("p=500,100");
        }
        inboundEmitter: <default("PacketEmitter")> like IPacketFlow {
            @display("p=500,300");
        }
        receiver: <default("DestreamingReceiver")> like IPacketReceiver {
            datarate = default(bitrate);
            @display("p=500,400");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> preambleInserter.in;
        preambleInserter.out --> interFrameGapInserter.in;
        interFrameGapInserter.out --> outboundEmitter.in;
        outboundEmitter.out --> transmitter.in;
        transmitter.out --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> receiver.in;
        receiver.out --> inboundEmitter.in;
        inboundEmitter.out --> preambleChecker.in;
        preambleChecker.out --> { @display("m=n"); } --> upperLayerOut;
}
