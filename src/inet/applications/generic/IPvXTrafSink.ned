//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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


package inet.applications.generic;

//
// Consumes and prints packets received from the IP module.
// Compatible with both ~IPv4 and ~IPv6.
//
simple IPvXTrafSink like IIPvXTrafficGenerator
{
    parameters:
        int protocol; // value for ~IPv4ControlInfo / ~IPv6ControlInfo protocol field
        @display("i=block/sink");
        @signal[rcvdPk](type=cPacket);
        @statistic[rcvdPk](title="packets received"; source=rcvdPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[endToEndDelay](title="end-to-end delay"; source="messageAge(rcvdPk)"; unit=s; record=histogram,vector; interpolationmode=none);
    gates:
        input ipIn @labels(IPv4ControlInfo/up,IPv6ControlInfo/up);
        output ipOut @labels(IPv4ControlInfo/down,IPv6ControlInfo/down);
}

