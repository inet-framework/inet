//
// Copyright (C) 2001-2006  Sergio Andreozzi
// Copyright (C) 2009 A. Ariza Universidad de Malaga
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


package inet.networklayer.queue;

//
// Queue that implements Weighter Fair Queueing (WFQ), to be used in
// network interfaces.
//
// @see ~REDQueue, ~DropTailQoSQueue, ~DropTailQueue
//
simple WeightedFairQueue like IOutputQueue
{
    parameters:
        int frameCapacity = default(100);  // per-subqueue capacity
        string classifierClass;  // class that inherits from IQoSClassifier
        double bandwidth @unit(bps) = default(1e6bps);
        string queueWeight = default(""); // by default the queues have the same weight
        bool useRED = default(false); // whether to use RED policy
        double wq = default(0.002);  // queue weight
        double minth = default(5);  // minimum threshold for avg queue length
        double maxth = default(50);  // maximum threshold for avg queue length (=buffer capacity)
        double maxp = default(0.02);  // maximum value for pb
        double pkrate = default(150);  // arrivals per sec (see comment above)
        @display("i=block/queue");
        @signal[queueLength](type=long);
        @signal[earlyDropPkByQueue](type=cMessage);
        @statistic[earlyDropPk](title="early dropped packets"; source=earlyDropPkByQueue; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[queueLength](title="queue length"; record=max,timeavg,vector; interpolationmode=sample-hold);

    gates:
        input in;
        output out;
}



