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
// Models Telnet sessions with a specific user behaviour.
// The server app should be ~TcpGenericServerApp.
// Compatible with both ~Ipv4 and ~Ipv6.
//
// NOTE: This module emulates a very specific user behaviour, and as such,
// it should be viewed as an example rather than a generic Telnet model.
// If you want to model realistic Telnet traffic, you are encouraged
// to gather statistics from packet traces on a real network, and
// write your model accordingly.
//
simple TelnetApp like IApp
{
    parameters:
        string localAddress = default(""); // local address or empty ("")
        int localPort; // local port number
        string connectAddress;  // server address (may be symbolic)
        int connectPort = default(1000); // port number to connect to
        double startTime @unit(s) = default(1s); // time first session begins
        double stopTime @unit(s) = default(-1s);  // time of finishing sending, negative values mean forever
        volatile int numCommands = default(int(exponential(10))); // user types this many commands in a session
        volatile int commandLength @unit(B) = default(10B); // commands are this many characters (plus Enter)
        volatile double keyPressDelay @unit(s) = default(exponential(0.1s)); // delay between keypresses
        volatile int commandOutputLength @unit(B) = default(intWithUnit(exponential(1000B))); // commands produce this much output
        volatile double thinkTime @unit(s) = default(exponential(10s)); // user waits this much before starting to type new command
        volatile double idleInterval @unit(s) = default(exponential(300s)); // time gap between sessions
        volatile double reconnectInterval @unit(s) = default(30s);  // if connection breaks, user waits this much before trying to reconnect
        @display("i=block/telnet");
        @signal[packetSent](type=Packet);
        @signal[packetReceived](type=Packet);
        @signal[connect](type=long);  // 1 for open, -1 for close
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[packetSent](title="packets sent"; source=packetSent; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[endToEndDelay](title="end-to-end delay"; source="dataAge(packetReceived)"; unit=s; record=histogram,vector; interpolationmode=none);
        @statistic[numActiveSessions](title="number of active sessions"; source="sum(connect)"; record=max,timeavg,vector; interpolationmode=sample-hold;);
        @statistic[numSessions](title="total number of sessions"; source="sum(connect+1)/2"; record=last);

    gates:
        input socketIn @labels(TcpCommand/up);
        output socketOut @labels(TcpCommand/down);
}

