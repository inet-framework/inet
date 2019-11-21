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
// Video stream server. To be used with ~UDPVideoStreamCli.
//
// The server will wait for incoming "video streaming requests".
// When a request arrives, it draws a random video stream size
// using the videoSize parameter, and starts streaming to the client.
// During streaming, it will send UDP packets of size packetLen at every
// sendInterval, until videoSize is reached. The parameters packetLen
// and sendInterval can be set to constant values to create CBR traffic,
// or to random values (e.g. sendInterval=uniform(1e-6, 1.01e-6)) to
// accomodate jitter.
//
// The server can serve several clients, and several streams per client.
//
// Statistics:
//
// reqStreamBytes: statistic of bytelength of requested video streams.
// sentPkBytes: statistic of sent packets and sent bytes.
//
// @see ~UDPVideoStreamCli
//
simple UdpVideoStreamSvr2 like IApp
{
    parameters:        
        string interfaceTableModule;   // The path to the InterfaceTable module        
        int localPort; // port to listen on
        volatile double sendInterval @unit(s); // interval between sending video stream packets
        volatile int packetLen @unit(B);  // length of a video packet in bytes
        volatile int videoSize @unit(B);  // length of full a video stream in bytes
        volatile double stopTime @unit(s) = default(-1s);
        string traceFileName = default(""); // University arizona video trace format
        bool macroPackets = default(false); // it allows that several video frames can be grouped in a packet
        int maxSizeMacro @unit(B) = default(512B); // maximum size of a grouped packet
        bool videoBroadcast = default(false); // the server will send broadcast video frames 
        double startBroadcast @unit(s) = default(0s); // start time of the video broadcast 
        string broadcastInterface = default("wlan0"); // interface used to broadcast the video frames
        volatile double restartBroascast @unit(s) = default(-1s); // after finish the broadcast sequence the server will rester the broadcast ofter an interval 
        double stopOperationExtraTime @unit(s) = default(-1s);    // extra time after lifecycle stop operation finished
        double stopOperationTimeout @unit(s) = default(2s);    // timeout value for lifecycle stop operation
        @display("i=block/app");
        @signal[sentPk](type=cPacket);
        @signal[reqStreamBytes](type=long);
        @statistic[reqStreamBytes](title="requested stream bytes"; record=count,sum,vector; interpolationmode=none);
        @statistic[sentPk](title="packets sent"; source=sentPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
    gates:
        input socketIn @labels(UdpControlInfo/up);
        output socketOut @labels(UdpControlInfo/down);
}

