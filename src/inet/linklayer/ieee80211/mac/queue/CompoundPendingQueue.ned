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

package inet.linklayer.ieee80211.mac.queue;

import inet.common.queueing.PacketClassifier;
import inet.common.queueing.CompoundPacketQueue;
import inet.common.queueing.PacketQueue;
import inet.common.queueing.PriorityScheduler;

module CompoundPendingQueue extends CompoundPacketQueue
{
    parameters:
        frameCapacity = default(100); // -1 means unlimited
    submodules:
        classifier: PacketClassifier {
            parameters:
                classifierClass = default("inet::ieee80211::MgmtOverDataClassifier"); // MgmtOverDataClassifier, MgmtOverMulticastOverUnicastClassifier, ...
                @display("p=100,200");
        }
        managementQueue: PacketQueue {
            parameters:
                @display("p=300,100");
        }
        multicastQueue: PacketQueue {
            parameters:
                @display("p=300,200");
        }
        unicastQueue: PacketQueue {
            parameters:
                @display("p=300,300");
        }
        prioritizer: PriorityScheduler {
            parameters:
                @display("p=500,200");
        }
    connections:
        in --> classifier.in;
        classifier.out++ --> managementQueue.in;
        classifier.out++ --> multicastQueue.in;
        classifier.out++ --> unicastQueue.in;
        managementQueue.out --> prioritizer.in++;
        multicastQueue.out --> prioritizer.in++;
        unicastQueue.out --> prioritizer.in++;
        prioritizer.out --> out;
}

