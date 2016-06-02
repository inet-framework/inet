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


package inet.applications.udpapp;

import inet.applications.contract.IApp;

//
// Consumes and prints packets received from the ~UDP module.
//
simple UDPSink like IApp
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        double startTime @unit(s) = default(0s); // application start time
        double stopTime @unit(s) = default(-1s);  // time of finishing receiving
        int localPort; // local port
        bool receiveBroadcast = default(false); // if true, makes the socket receive broadcast packets
        string multicastGroup = default(""); // if non-empty, makes the socket receive multicast packets sent to that group
        string multicastSources = default(""); // if non-empty, than the socket receives multicast packets from those sources only

        @display("i=block/sink");
        @signal[rcvdPk](type=cPacket);
        @statistic[rcvdPk](title="packets received"; source=rcvdPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[endToEndDelay](title="end-to-end delay"; source="messageAge(rcvdPk)"; unit=s; record=histogram,vector; interpolationmode=none);
    gates:
        input socketIn @labels(UDPControlInfo/up);
        output socketOut @labels(UDPControlInfo/down);
}

