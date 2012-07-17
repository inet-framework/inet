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


package inet.util;

import inet.base.IHook;


//
// Delayer module.
//
simple Delayer like IHook
{
    parameters:
        volatile double delay @unit(s) = default(0s);    // delay [seconds] e.g. uniform(20ms,40ms)
        @display("i=status/lightning");
        @signal[rcvdPk](type=cMessage);
        @signal[sentPk](type=cMessage);
        @signal[delay](type=simtime_t);     // delay in seconds, emitted at packet receiving
        @statistic[rcvdPk](title="packets received"; source=rcvdPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[sentPk](title="packets sent"; source=sentPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[delay](title="packet delay"; source=delay; record=vector,stats; interpolationmode=none);
    gates:
        input in;
        output out;
}
