//
// Copyright (C) 2012 Kyeong Soo (Joseph) Kim
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
// Video streaming client with source clock frequency recovery (SCFR) capability.
//
// It measures arrival times of video stream packets with RTP timestamps and
// estimates the ratio of source and receiver clock frequency, which is recorded
// as an output vector and used by SCFR algorithms.
//
// @see UDPVideoStreamSvrWithTrace2
//
simple UDPVideoStreamCliWithSCFR like UDPApp
{
    parameters:
        int localPort;
        string serverAddress;
        int serverPort;
        double startTime @unit("s") = default(1s);
        double clockFrequency @unit("Hz");	// frequency of a local clock
        @display("i=block/app");
    gates:
        input udpIn @labels(UDPControlInfo/up);
        output udpOut @labels(UDPControlInfo/down);
}

