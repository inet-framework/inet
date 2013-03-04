//
// Copyright (C) 2010 Kyeong Soo (Joseph) Kim
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

//
// Video streaming server based on trace files from ASU video trace library [1].
// To be used with UDPVideoStreamCliWithTrace.
//
// The server will wait for incoming "video streaming requests".
// When a request arrives, it starts streaming to the client using a trace file.
// During streaming, it will send \UDP packets whose sizes and intervals are
// calculated based on the frame size and the frame period from the trace file.
//
// The server can serve several clients, and several streams per client.
//
// @see UDPVideoStreamCliWithTrace
//
// @par References:
// <ol>
// <li><a href="http://trace.eas.asu.edu/">Video trace library, Arizona State University</a>
// </li>
// </ol>
//
simple UDPVideoStreamSvrWithTrace like UDPApp
{
    parameters:
        int serverPort; // port to listen on
        int fps = default(30);	// frames per second (fps) of the trace file
        int appOverhead @unit(byte) = default(12B);	// size of application layer overhead (e.g., RTP header)	
        int maxPayloadSize @unit(byte) = default(1460B);	// maximum size of payload to be carried in RTP packets;
															// default of 1460B is MTU of 1500B minus RTP (12B), UDP (8B), and IP (20B) headers.											
		bool frameSpreading = default(true);				// if true, spread frame packets over a frame period
        string traceFile;	// name of a video trace file in ASU format
        @display("i=block/app");
    gates:
        input udpIn @labels(UDPControlInfo/up);
        output udpOut @labels(UDPControlInfo/down);
}

