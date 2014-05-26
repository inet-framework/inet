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
// Video streaming client with source clock frequency recovery (SCFR) capability.
//
// It records as an output vector the values of timestamps of received RTP packets
// togetehr with their arrival times for further processing with external programs.
//
// @see UDPVideoStreamSvrWithTrace3
//
simple UDPVideoStreamCliWithSCFR2 like UDPApp
{
    parameters:
        @display("i=block/app");
		// statistics: common
        @statistic[departureTime](title="departure (creation) times of RTP packets"; record=vector; interpolationmode=none);
        @statistic[timestamp](title="received timestamps of RTP packets"; record=vector; interpolationmode=none);
        @statistic[fragmentStart](title="fragment start indicators of RTP packets"; record=vector; interpolationmode=none);
        int localPort;
        string serverAddress;
        int serverPort;
        double startTime @unit("s") = default(1s);
    gates:
        input udpIn @labels(UDPControlInfo/up);
        output udpOut @labels(UDPControlInfo/down);
}

