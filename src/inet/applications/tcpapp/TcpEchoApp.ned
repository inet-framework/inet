//
// Copyright (C) 2004 Andras Varga
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

package inet.applications.tcpapp;

import inet.applications.contract.IApp;

//
// Accepts any number of incoming TCP connections, and sends back the
// messages that arrive on them, The lengths of the messages are
// multiplied by echoFactor before sending them back (echoFactor=1 will
// result in sending back the same message unmodified.) The reply can also be
// delayed by a constant time (echoDelay parameter).
//
// Compatible with both ~Ipv4 and ~Ipv6.
//
simple TcpEchoApp extends TcpServerHostApp
{
    parameters:
        @class(TcpEchoApp);
        double echoFactor = default(1);
        double echoDelay @unit(s) = default(0s);
        serverThreadModuleType = default("inet.applications.tcpapp.TcpEchoAppThread"); // module type of "thread" module to launch on incoming connections
        @signal[packetSent](type=inet::Packet);
        @signal[packetReceived](type=inet::Packet);
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[packetSent](title="packets sent"; source=packetSent; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[endToEndDelay](title="end-to-end delay"; source="dataAge(packetReceived)"; unit=s; record=histogram,weightedHistogram,vector; interpolationmode=none);
}

// Thread for ~TcpEchoApp
simple TcpEchoAppThread
{
}

