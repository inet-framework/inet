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

package inet.linklayer.ethernet.modular;

import inet.linklayer.ethernet.contract.IEthernetMacLayer;
import inet.queueing.server.PreemptingServer;
import inet.queueing.classifier.PacketClassifier;
import inet.queueing.common.PacketMultiplexer;
import inet.queueing.contract.IPacketQueue;
import inet.queueing.scheduler.PriorityScheduler;

module EthernetPreemptingMacLayer like IEthernetMacLayer
{
    parameters:
        volatile double bitrate @unit(bps);
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        queue: <default("OmittedPacketQueue")> like IPacketQueue if typename != "" {
            @display("p=200,100");
        }
        outboundClassifier: PacketClassifier {
            classifierClass = default("inet::PacketVlanReqClassifier");
            @display("p=200,200");
        }
        expressMacLayer: <default("EthernetStreamingMacLayer")> like IEthernetMacLayer {
            @display("p=300,300");
        }
        preemptableMacLayer: <default("EthernetFragmentingMacLayer")> like IEthernetMacLayer {
            @display("p=500,300");
        }
        scheduler: PriorityScheduler {
            @display("p=200,400");
        }
        preemptingServer: PreemptingServer {
            datarate = default(bitrate);
            @display("p=200,500");
        }
        inboundClassifier: PacketClassifier {
            classifierClass = default("inet::PacketFragmentTagClassifier");
            @display("p=600,400");
        }
        multiplexer: PacketMultiplexer {
            @display("p=600,200");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> queue.in;
        queue.out --> outboundClassifier.in;
        outboundClassifier.out++ --> expressMacLayer.upperLayerIn;
        outboundClassifier.out++ --> preemptableMacLayer.upperLayerIn;
        expressMacLayer.lowerLayerOut --> scheduler.in++;
        preemptableMacLayer.lowerLayerOut --> scheduler.in++;
        scheduler.out --> preemptingServer.in;
        preemptingServer.out --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> inboundClassifier.in;
        inboundClassifier.out++ --> expressMacLayer.lowerLayerIn;
        inboundClassifier.out++ --> preemptableMacLayer.lowerLayerIn;
        expressMacLayer.upperLayerOut --> multiplexer.in++;
        preemptableMacLayer.upperLayerOut --> multiplexer.in++;
        multiplexer.out --> { @display("m=n"); } --> upperLayerOut;
}
