//
// Copyright (C) 2005 Andras Varga
// Copyright (C) 2015 A. Ariza (Malaga University)
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
simple UdpVideoStreamCli2 like IApp
{
    parameters:
        int localPort = default(-1);  // local port (-1: use ephemeral port)
        string serverAddress;  // server address
        int serverPort;  // server port
        double startTime @unit(s) = default(1s); // start time of the client
        volatile double reintent @unit(s) = default(60s); // The client will request a new sequence if no video frame has arrived after this period, if a video frame has been received, the client can request other sequences only if the parameter multipleRequest is true   
        bool multipleRequest = default(false);  // if true the client request other sequence after finish the current 
        double timeOut @unit(s) = default(10s); // maximum time without receive a frame that the client use to determine a fail of request or end of sequence. If it is a end of sequence and multipleRequest = false the client doesn't request other sequence, if the client has never received a video frame, it will request other sequence
        double limitDelay @unit(s) = default(0.5s); // maximum delay in a video frame to discard it
        @display("i=block/app");
        @signal[rcvdPk](type=cPacket);
        @statistic[rcvdPk](title="packets received"; source=rcvdPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[endToEndDelay](title="end-to-end delay"; source="messageAge(rcvdPk)"; unit=s; record=histogram,vector; interpolationmode=none);
    gates:
        input socketIn @labels(UdpControlInfo/up);
        output socketOut @labels(UdpControlInfo/down);
}

