//
// Copyright (C) 2005 Andras Varga
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
// Video streaming client.
//
// This module sends one "video streaming request" to serverAddress:serverPort at time startTime
// and receives stream from ~UDPVideoStreamSvr server.
//
// Statistics:
//
// rcvdPkBytes: statistic of received packets and received bytes.
// endToEndDelay: statistic of delays of incoming packets.
//
// @see ~UDPVideoStreamSvr
//
simple UDPVideoStreamCli like IUDPApp
{
    parameters:
        int localPort = default(-1);  // local port (-1: use ephemeral port)
        string serverAddress;  // server address
        int serverPort;  // server port
        double startTime @unit(s) = default(1s);
        @display("i=block/app");
        @signal[rcvdPk](type=cPacket);
        @statistic[rcvdPk](title="packets received"; source=rcvdPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[endToEndDelay](title="end-to-end delay"; source="messageAge(rcvdPk)"; unit=s; record=histogram,vector; interpolationmode=none);
    gates:
        input udpIn @labels(UDPControlInfo/up);
        output udpOut @labels(UDPControlInfo/down);
}

