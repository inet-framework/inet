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
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package inet.examples.ethernet.gatepreemption;

import inet.networks.WiredNetworkBase;
import inet.node.ethernet.Eth100M;
import inet.node.inet.StandardHost;
import inet.protocol.common.PreemptableStreamer;
import inet.protocol.ethernet.EthernetMacLayer;
import inet.protocol.fragmentation.FragmentTagBasedDefragmenter;
import inet.queueing.contract.IPacketBuffer;
import inet.queueing.contract.IPacketClassifier;
import inet.queueing.contract.IPacketGate;
import inet.queueing.contract.IPacketQueue;
import inet.queueing.contract.IPacketScheduler;
import inet.queueing.queue.CompoundPacketQueue;

module StreamableSelectiveQueue extends CompoundPacketQueue
{
    parameters:
        int numQueues;
        @display("bgb=1014,338");
    submodules:
        buffer: <default("PriorityBuffer")> like IPacketBuffer if typename != "" {
            parameters:
                @display("p=100,225");
        }
        classifier: <default("PacketClassifier")> like IPacketClassifier {
            parameters:
                @display("p=100,100");
                classifierClass = default("inet::queueing::PacketUserPriorityIndClassifier");
        }
        queue[numQueues]: <default("PacketQueue")> like IPacketQueue {
            parameters:
                bufferModule = default(exists(buffer) ? "^.buffer" : "");
                @display("p=300,100,column,125");
        }
        streamer[numQueues]: PreemptableStreamer {
            @display("p=500,100,column,125");
            minPacketLength = 64B;
        }
        gate[numQueues]: <default("PeriodicGate")> like IPacketGate {
            parameters:
                @display("p=700,100,column,125");
        }
        scheduler: <default("PriorityScheduler")> like IPacketScheduler {
            parameters:
                @display("p=900,100");
        }
    connections:
        in --> { @display("m=w"); } --> classifier.in;
        for i=0..sizeof(queue)-1 {
            classifier.out++ --> queue[i].in;
            queue[i].out --> streamer[i].in;
            streamer[i].out --> gate[i].in;
            gate[i].out --> scheduler.in++;
        }
        scheduler.out --> { @display("m=e"); } --> out;
}

module CustomMacLayer extends EthernetMacLayer
{
    parameters:
        server.typename = default("PreemptingServer");
        streamer.typename = default("OmittedPacketFlow"); // Packet streaming begins in queue module
        receiver.typename = default("StreamingReceiver");
        transmitter.typename = default("StreamingTransmitter");
        fcsInserter.typename = default("EthernetFragmentFcsInserter");
        preambleInserter.typename = default("EthernetFragmentPreambleInserter");
        fcsChecker.typename = default("EthernetFragmentFcsChecker");
        preambleChecker.typename = default("EthernetFragmentPreambleChecker");
        volatile double bitrate @unit(bps);
        server.datarate = bitrate;
    submodules:
        defragmenter: FragmentTagBasedDefragmenter {
            @display("p=500,300");
        }
    connections:
        fcsChecker.out --> { @reconnect; } --> defragmenter.in;
        defragmenter.out --> { @reconnect;@display("m=n"); } --> upperLayerOut;
}

network GatePreemptionTest extends WiredNetworkBase
{
    parameters:
        string highTsOperation;
        @display("bgb=424,405");
    submodules:
        host1: StandardHost {
            @display("p=250,100");
        }
        host2: StandardHost {
            @display("p=350,100");
        }
    connections:
        host1.ethg++ <--> Eth100M <--> host2.ethg++;
}

