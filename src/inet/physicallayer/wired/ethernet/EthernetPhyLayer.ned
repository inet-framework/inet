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
        *.bitrate = default(bitrate);
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
            duration = default(96s / dropUnit(bitrate));
            @display("p=200,200");
        }
        outboundEmitter: <default("PacketEmitter")> like IPacketFlow {
            @display("p=200,300");
        }
        transmitter: <default("PacketTransmitter")> like IPacketTransmitter {
            datarate = default(bitrate);
            @display("p=200,400");
        }
        phyHeaderChecker: <default("EthernetPhyHeaderChecker")> like IPacketFilter {
            @display("p=500,100");
        }
        inboundEmitter: <default("PacketEmitter")> like IPacketFlow {
            @display("p=500,300");
        }
        receiver: <default("PacketReceiver")> like IPacketReceiver {
            datarate = default(bitrate);
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
