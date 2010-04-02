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
// Video streaming client to be used with UDPVideoStreamSvrWithTrace.
//
// @see UDPVideoStreamSvrWithTrace
//
simple UDPVideoStreamCliWithTrace like UDPApp
{
    parameters:
        int localPort;
        string serverAddress;
        int serverPort;
        double startTime @unit("s") = default(1s);        
        int fps = default(30);	// frames per second (fps) of the trace file
        int appOverhead @unit(byte) = default(12B);	// size of application layer overhead (e.g., RTP header)
													// default of 1460B is MTU of 1500B minus RTP (12B), UDP (8B), and IP (20B) headers.
		double startupDelay @unit(s) = default(5s);	// start-up delay resulting from both decoding and networking (e.g., jitter, depacketization)
        @display("i=block/app");
    gates:
        input udpIn @labels(UDPControlInfo/up);
        output udpOut @labels(UDPControlInfo/down);
}
