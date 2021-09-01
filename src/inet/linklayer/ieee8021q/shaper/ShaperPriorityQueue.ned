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

package inet.linklayer.ieee8021q.shaper;

import inet.queueing.contract.IPacketClassifier;
import inet.queueing.contract.IPacketScheduler;
import inet.queueing.contract.IPacketShaper;
import inet.queueing.queue.CompoundPacketQueueBase;

module ShaperPriorityQueue extends CompoundPacketQueueBase
{
    parameters:
        int numShapers;
    submodules:
        classifier: <default("PriorityToTrafficClassClassifier")> like IPacketClassifier {
            @display("p=100,150");
        }
        shaper[numShapers]: <default("PacketShaper")> like IPacketShaper {
            @display("p=300,150,column,150");
        }
        scheduler: <default("PriorityScheduler")> like IPacketScheduler {
            @display("p=500,150");
        }
    connections:
        in --> classifier.in;
        for i=0..numShapers-1 {
            classifier.out++ --> shaper[i].in;
            shaper[i].out --> scheduler.in++;
        }
        scheduler.out --> out;
}
