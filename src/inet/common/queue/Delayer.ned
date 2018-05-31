//
// Copyright (C) 2012 OpenSim Ltd
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
// @author: Zoltan Bojthe
//


package inet.common.queue;

import inet.common.IHook;


//
// Delayer module.
//
simple Delayer like IHook
{
    parameters:
        volatile double delay @unit(s) = default(0s);    // delay [seconds] e.g. uniform(20ms,40ms)
        @display("i=block/timer");
        @signal[packetReceived](type=cMessage);
        @signal[packetSent](type=cMessage);
        @signal[delay](type=simtime_t);     // delay in seconds, emitted at packet receiving
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[packetSent](title="packets sent"; source=packetSent; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[delay](title="packet delay"; source=delay; record=vector,stats; interpolationmode=none);
    gates:
        input in;
        output out;
}
