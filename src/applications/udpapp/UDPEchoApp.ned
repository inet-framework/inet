//
// Copyright (C) 2005 Andras Babos
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


package inet.applications.udpapp;

import inet.applications.IUDPApp;


//
// Similar to ~UDPBasicApp, but it sends back the packet after reception.
//
simple UDPEchoApp like IUDPApp
{
    parameters:
        int localPort = default(-1);  // local port (-1: use ephemeral port)
        string destAddresses = default(""); // list of destination IP addresses, separated by spaces
        int destPort;  // destination port 
        volatile int messageLength @unit(B); // length of messages to generate, int bytes
        double startTime @unit(s) = default(1s); // application start time (start of the first packet)
        double stopTime @unit(s) = default(0);  // time of finish sending, 0 means forever
        volatile double sendInterval @unit(s); // should usually be a random value, e.g. exponential(1)
        @display("i=block/app");
        @signal[sentPk](type=cPacket);
        @signal[rcvdPk](type=cPacket);
        @signal[roundTripTime](type=simtime_t);
        @statistic[rcvdPkBytes](title="Received packet bytes"; source="packetBytes(rcvdPk)"; unit=B; record=count,sum,vector; interpolationmode=none);
        @statistic[sentPkBytes](title="Sent packet bytes"; source="packetBytes(sentPk)"; unit=B; record=count,sum,vector; interpolationmode=none);
        @statistic[roundTripTime](title="Round trip time"; unit=s; record=histogram,stats,vector);
    gates:
        input udpIn @labels(UDPControlInfo/up);
        output udpOut @labels(UDPControlInfo/down);
}
