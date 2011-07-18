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



package inet.util;

import inet.base.IHook;


//
// Duplicate generator module.
//
simple DuplicatesGenerator like IHook
{
    parameters:
        @node();
        string duplicatesVector = default(""); // the duplicatesVector format is "<packetNumber>;<packetNumber>;...;"
                                               // packet numbers in ascending order e.g. "5;6;12;13;"
        @display("i=status/busy");
        @signal[rcvdPk](type=cMessage);
        @signal[sentPk](type=cMessage);
        @signal[duplPk](type=cMessage);
        @statistic[rcvdPkBytes](title="Received packet bytes"; source="packetBytes(rcvdPk)"; unit=B; record=count,sum,vector; interpolationmode=none);
        @statistic[sentPkBytes](title="Sent packet bytes"; source="packetBytes(sentPk)"; unit=B; record=count,sum,vector; interpolationmode=none);
        @statistic[duplPkBytes](title="Duplicated packet bytes"; source="packetBytes(duplPk)"; unit=B; record=count,sum,vector; interpolationmode=none);
    gates:
        input in;
        output out;
}
