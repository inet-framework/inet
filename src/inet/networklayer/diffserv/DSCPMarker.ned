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
// This module sets the DSCP field (lower six bit of Tos/TrafficClass) of IP datagrams
// to the value specified by the dscps parameter.
//
// The dscps parameter is a space separated list of DSCP values.
// Packets arrived at gate in[i] are marked with the ith value. If there are fewer
// dscp values given than input gates, then the last one is repeated.
//
simple DSCPMarker
{
    parameters:
        string dscps; // space separated list if dscp values; both names (e.g. AF11, EF) and numbers (0x0A,0b101110) can be used

        @display("i=block/star");

        @signal[markPk](type=cPacket);
        @statistic[markPk](title="packets marked"; source=markPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
    gates:
        input in[];
        output out;
}