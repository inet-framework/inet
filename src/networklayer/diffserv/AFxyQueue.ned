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

import inet.linklayer.queue.FIFOQueue;
import inet.linklayer.queue.REDDropper;


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
        fifoQueue: FIFOQueue {
            @display("p=251,102");
        }
        redDropper: REDDropper {
            numGates = 3;
            wq = wq;
            minths = string(afx1Minth) + " " + string(afx2Minth) + " " + string(afx3Minth);
            maxths = string(afx1Maxth) + " " + string(afx2Maxth) + " " + string(afx3Maxth);
            maxps = string(afx1Maxp) + " " + string(afx2Maxp) + " " + string(afx3Maxp);
            @display("p=105,102");
        }
    connections:
        afx1In --> { @display("m=w"); } --> redDropper.in[0];
        afx2In --> { @display("m=w"); } --> redDropper.in[1];
        afx3In --> { @display("m=w"); } --> redDropper.in[2];
        redDropper.out[0] --> { @display("m=m,100,20,0,50"); } --> fifoQueue.in++;
        redDropper.out[1] --> { @display("m=m,100,50,0,50"); } --> fifoQueue.in++;
        redDropper.out[2] --> { @display("m=m,100,80,0,50"); } --> fifoQueue.in++;
        fifoQueue.out --> { @display("m=e"); } --> out;
}
