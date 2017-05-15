//
// Copyright (C) 2005 Andras Varga
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


package inet.linklayer.queue;

import inet.linklayer.IOutputQueue;


// Drop-tail queue, to be used in network interfaces.
// Conforms to the ~IOutputQueue interface.
// DTQueue - modified DroptailQueue with threshold CAC control
//
simple DataThrQueue like IOutputQueue
{
    parameters:
        int frameCapacity = default(100);
        string queueName = default("l2queue"); // name of the inner cQueue object, used in the 'q' tag of the display string
        @display("i=block/queue");
        @signal[rcvdPk](type=cPacket);
        @signal[enqueuePk](type=cPacket);
        @signal[dequeuePk](type=cPacket);
        @signal[dropPkByQueue](type=cPacket);
        @signal[queueingTime](type=simtime_t; unit=s);
        @signal[queueingTimeOfSyn](type=simtime_t; unit=s);
        @signal[queueLength](type=long);
        @signal[accepted_tosOfSYNPk](type=long);
        @signal[accepted_reqBwOfSYNPk](type=long);
        @signal[rejected_tosOfSYNPk](type=long);
        @signal[rejected_reqBwOfSYNPk](type=long);
        @signal[cumulative_accepted](type=long);
        @signal[cumulative_rejected](type=long);
        @signal[cumulative_revenue](type=long);
        @signal[hostSendInt](type=double);
        @signal[currSynLoad](type=long);
        
        @statistic[rcvdPk](title="received packets"; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[dropPk](title="dropped packets"; source=dropPkByQueue; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[queueingTime](title="queueing time"; record=histogram,vector; interpolationmode=none);
        @statistic[queueingTimeOfSyn](title="queueing time of accepted SYN"; record=histogram,vector; interpolationmode=none);
        @statistic[queueLength](title="queue length"; record=max,timeavg,vector; interpolationmode=sample-hold);
        @statistic[accepted_tosOfSYNPk](title="accpted ToS"; source=accepted_tosOfSYNPk; record=histogram,vector; interpolationmode=none);
        @statistic[accepted_reqBwOfSYNPk](title="accepted reqBw"; source=accepted_reqBwOfSYNPk; record=histogram,vector; interpolationmode=none);
        @statistic[rejected_tosOfSYNPk](title="rejected ToS"; source=rejected_tosOfSYNPk; record=histogram,vector; interpolationmode=none);
        @statistic[rejected_reqBwOfSYNPk](title="rejected reqBw"; source=rejected_reqBwOfSYNPk; record=histogram,vector; interpolationmode=none);
        @statistic[cumulative_accepted](title="cumulative accepted flows"; source=cumulative_accepted; record=count,vector; interpolationmode=none);
        @statistic[cumulative_rejected](title="cumulative rejected flows"; source=cumulative_rejected; record=count,vector; interpolationmode=none);
        @statistic[cumulative_revenue](title="cumulative sum of accepted ToS value"; source=cumulative_revenue; record= vector; interpolationmode=none);
        @statistic[hostSendInt](title="sendInterval"; source=hostSendInt; record=histogram, vector; interpolationmode=none);
        @statistic[currSynLoad](title="SYN host data rate"; source=currSynLoad; record=histogram, vector; interpolationmode=none);
        
    gates:
        input in;
        output out;
}

