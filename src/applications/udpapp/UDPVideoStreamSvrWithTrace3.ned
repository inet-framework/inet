//
// Copyright (C) 2013 Kyeong Soo (Joseph) Kim
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
// Video streaming server based on trace files from ASU video trace library [1]
// for the source clock frequency recovery with jitter time-stamp approach [2].
// To be used with UDPVideoStreamCliWithSCFR.
//
// @see UDPVideoStreamSvrWithTrace, UDPVideoStreamSvrWithTrace2, UDPVideoStreamCliWithSCFR
//
// @note All generated RTP packets have the same payload size (including the last fragment).
//
// @par References:
// <ol>
// <li><a href="http://trace.eas.asu.edu/">Video trace library, Arizona State University</a>
// </li>
// <li>W. Su and I. F. Akyildiz,
//     "The jitter time-stamp approach for clock recovery of real-time variable
//     bit-rate traffic," IEEE/ACM Trans. Networking, vol. 9., no. 6,
//     Dec. 2001, pp. 746-754.
// </li>
// </ol>
//
simple UDPVideoStreamSvrWithTrace3 like UDPApp
{
    parameters:
        int serverPort; // port to listen on
        int fps = default(30);	// frames per second (fps) of the trace file
        int appOverhead @unit(byte) = default(12B);	// size of application layer overhead (e.g., RTP header)	
        int maxPayloadSize @unit(byte) = default(1460B);	// maximum size of payload to be carried in RTP packets;
															// default of 1460B is MTU of 1500B minus RTP (12B), UDP (8B), and IP (20B) headers.
		double clockFrequency @unit(Hz) = default(90000Hz);	// frequency of	a local clock for RTP timestamps
		int r = default(10);	// ratio bewteen reference clock frequency (i.e., 'f_r' in [2]) and packet transmission rate during ON period
        string traceFile;	// name of a video trace file in ASU format
		bool frameSpreading = default(false);	// if true, spread frame packets over a frame period
        @display("i=block/app");
    gates:
        input udpIn @labels(UDPControlInfo/up);
        output udpOut @labels(UDPControlInfo/down);
}
