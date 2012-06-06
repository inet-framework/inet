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
// This classifier contains a list of filters that identifies
// the flows and determines their classes.
// Each filter can match the source and destination address,
// IP protocol number, source and destination ports, or ToS
// of the datagram. The first matching filter determines the
// index of the out gate. If no matching filter is found,
// then the packet will be sent through the defaultOut gate.
//
// See RFC 2475 2.3.1, RFC 3290 4.2.2
//
simple MultiFieldClassifier
{
    parameters:
        xml filters = default(xml("<filters/>"));
        @display("i=block/classifier");
        
        @signal[pkClass](type=integer);
        @statistic[pkClass](title="packet class"; source=pkClass; record=vector; interpolationmode=none);
    gates:
        input in;
        output outs[];
        output defaultOut;
}
