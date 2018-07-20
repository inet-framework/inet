//
// Copyright (C) 2009 Thomas Reschka
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



package inet.common.queue;

import inet.common.IHook;


//
// Ordinal Based Duplicator module.
//
simple OrdinalBasedDuplicator like IHook
{
    parameters:
        string duplicatesVector = default(""); // the duplicatesVector format is "<packetNumber>;<packetNumber>;...;"
                                               // packet numbers in ascending order e.g. "5;6;12;13;"
        @display("i=block/control");
        @signal[packetReceived](type=cMessage);
        @signal[packetSent](type=cMessage);
        @signal[duplPk](type=cMessage);
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[packetSent](title="packets sent"; source=packetSent; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[duplPk](title="packets duplicated"; source=duplPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
    gates:
        input in;
        output out;
}
