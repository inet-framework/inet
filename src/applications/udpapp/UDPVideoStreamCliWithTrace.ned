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
// It reports packet-level as well as frame-level (meaningful only with
// trace files in <i>verbose</i> format) statistics at the end of simulation.
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
        double startupDelay @unit(s) = default(5s);	// due to play-out buffering; 5s is default value for Windows Media Player
        int fps = default(30);	// frames per second (fps) of the trace file
        int appOverhead @unit(byte) = default(12B);	// size of application layer overhead (e.g., RTP header)
													// default of 1460B is MTU of 1500B minus RTP (12B), UDP (8B), and IP (20B) headers.
		// parameters for trace statistics (default values are for 'Terminator 2'
		int numTraceFrames = default(17998);	// number of frames in the trace file (needed to handle wrap around of frames by the server)
		int gopSize = default(12);	// number of frames per GOP
		int numBFrames = default(2);	// number of B frames between two consecutive I or P frames
        @display("i=block/app");
    gates:
        input udpIn @labels(UDPControlInfo/up);
        output udpOut @labels(UDPControlInfo/down);
}
