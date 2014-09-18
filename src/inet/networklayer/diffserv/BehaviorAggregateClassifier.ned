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

//
// This module reads the DSCP (lower six bits of ToS/TrafficClass) from the
// received datagram, and forwards the datagram to the corresponding output gate.
//
// DSCPs are enumerated in the dscps parameter, first value is for out[0], second
// for out[1], etc. If the packet has no DSCP, or its value is not enumerated
// in the dscps parameter, it will be forwarded via the defaultOut gate.
//
simple BehaviorAggregateClassifier
{
    parameters:
        string dscps = default(""); // space separated dscp values of the gates, both names (e.g. AF11, EF) and numbers (0x0A,0b101110) can be used
        @display("i=block/classifier");

        @signal[pkClass](type=long);
        @statistic[pkClass](title="packet class"; source=pkClass; record=vector; interpolationmode=none);
    gates:
        input in;
        output outs[];
        output defaultOut;
}