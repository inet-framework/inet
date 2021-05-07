//
// Copyright (C) 2012 Opensim Ltd.
// Author: Tamas Borbely
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.networklayer.diffserv;

import inet.queueing.common.PacketMultiplexer;
import inet.queueing.filter.RedDropper;
import inet.queueing.queue.PacketQueue;

//
// This is an example queue, that implements
// one class of the Assured Forwarding PHB group (RFC 2597).
//
// Packets with the same AFx class, but different drop priorities
// arrive at the afx1In, afx2In, and afx3In gates. The received
// packets are stored in the same queue. Before the packet
// is enqueued, a RED dropping algorithm may decide to selectively
// drop them, based on the average length of the queue and the RED parameters
// of the drop priority of the packet.
//
// The afxyMinth, afxyMaxth, and afxyMaxp parameters must have values that
// ensures that packets with lower drop priorities are dropped with lower
// or equal probability than packets with higher drop priorities.
//
// @see ~DiffservQueue
//
module AFxyQueue
{
    parameters:
        double wq = default(0.002); // smoothing factor, i.e.  the weight of the current queue length in the averaged queue length

        double afx1Minth = default(50);  // minimum queue length thresholds for dropping packets with drop priority 1
        double afx1Maxth = default(100); // maximum queue length thresholds for dropping packets with drop priority 1
        double afx1Maxp = default(0.3);  // maximum probability of drop when the queue length is between thresholds for drop priority 1

        double afx2Minth = default(30); // minimum queue length thresholds for dropping packets with drop priority 2
        double afx2Maxth = default(60); // maximum queue length thresholds for dropping packets with drop priority 2
        double afx2Maxp = default(0.6); // maximum probability of drop when the queue length is between thresholds for drop priority 2

        double afx3Minth = default(10); // minimum queue length thresholds for dropping packets with drop priority 3
        double afx3Maxth = default(40); // maximum queue length thresholds for dropping packets with drop priority 3
        double afx3Maxp = default(0.9); // maximum probability of drop when the queue length is between thresholds for drop priority 3

        @display("i=block/queue;q=l2queue");

    gates:
        input afx1In;
        input afx2In;
        input afx3In;
        output out;
    submodules:
        red1: RedDropper {
            collectionModule = "^.queue";
            wq = parent.wq;
            minth = parent.afx1Minth;
            maxth = parent.afx1Maxth;
            maxp = parent.afx1Maxp;
            @display("p=100,100");
        }
        red2: RedDropper {
            collectionModule = "^.queue";
            wq = parent.wq;
            minth = parent.afx2Minth;
            maxth = parent.afx2Maxth;
            maxp = parent.afx2Maxp;
            @display("p=100,200");
        }
        red3: RedDropper {
            collectionModule = "^.queue";
            wq = parent.wq;
            minth = parent.afx3Minth;
            maxth = parent.afx3Maxth;
            maxp = parent.afx3Maxp;
            @display("p=100,300");
        }
        mux: PacketMultiplexer {
            @display("p=200,200");
        }
        queue: PacketQueue {
            @display("p=300,200");
        }
    connections:
        afx1In --> { @display("m=w"); } --> red1.in;
        afx2In --> { @display("m=w"); } --> red2.in;
        afx3In --> { @display("m=w"); } --> red3.in;
        red1.out --> { @display("m=m,100,20,0,50"); } --> mux.in++;
        red2.out --> { @display("m=m,100,50,0,50"); } --> mux.in++;
        red3.out --> { @display("m=m,100,80,0,50"); } --> mux.in++;
        mux.out --> queue.in;
        queue.out --> { @display("m=e"); } --> out;
}

