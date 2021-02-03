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
// along with this program.  If not, see <https://www.gnu.org/licenses/>.//

package inet.examples.ethernet.clocksynchronization;

import inet.networks.WiredNetworkBase;
import inet.node.ethernet.Eth1G;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.StandardHost;
import inet.queueing.contract.IPacketClassifier;
import inet.queueing.contract.IPacketGate;
import inet.queueing.contract.IPacketQueue;
import inet.queueing.contract.IPacketScheduler;
import inet.queueing.queue.CompoundPacketQueueBase;

module ClockSynchronizationExampleQueue extends CompoundPacketQueueBase
{
    parameters:
        int numQueues;
        double bitrate @unit(bps);
        *.bitrate = default(bitrate);
        *.guardBand = replaceUnit(96.0 / dropUnit(bitrate), "s");
    submodules:
        classifier: <default("ContentBasedClassifier")> like IPacketClassifier {
            parameters:
                @display("p=100,100");
        }
        queue[numQueues]: <default("PacketQueue")> like IPacketQueue {
            parameters:
                @display("p=300,100,column,125");
        }
        gate[numQueues]: <default("PeriodicGate")> like IPacketGate {
            parameters:
                @display("p=500,100,column,125");
        }
        scheduler: <default("PriorityScheduler")> like IPacketScheduler {
            parameters:
                @display("p=700,100");
        }
    connections:
        in --> { @display("m=w"); } --> classifier.in;
        for i=0..sizeof(queue)-1 {
            classifier.out++ --> queue[i].in;
            queue[i].out --> gate[i].in;
            gate[i].out --> scheduler.in++;
        }
        scheduler.out --> { @display("m=e"); } --> out;
}

network ClockSynchronizationExampleNetwork extends WiredNetworkBase
{
    submodules:
        source1: StandardHost {
            @display("p=300,100");
        }
        source2: StandardHost {
            @display("p=300,300");
        }
        switch1: EthernetSwitch {
            @display("p=500,200");
        }
        switch2: EthernetSwitch {
            @display("p=700,200");
        }
        sink1: StandardHost {
            @display("p=900,100");
        }
        sink2: StandardHost {
            @display("p=900,300");
        }
    connections:
        switch1.ethg++ <--> Eth1G <--> switch2.ethg++;
        source1.ethg++ <--> Eth1G <--> switch1.ethg++;
        source2.ethg++ <--> Eth1G <--> switch1.ethg++;
        switch2.ethg++ <--> Eth1G <--> sink1.ethg++;
        switch2.ethg++ <--> Eth1G <--> sink2.ethg++;
}
