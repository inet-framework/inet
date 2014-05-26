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
// It measures arrival times of video stream packets using a 48-bit counter
// driven by a high-speed local clock (~ MHz) and estimates the ratio of source
// and receiver clock frequency together with RTP 32-bit timestamps, which is
// recorded as an output vector and used by SCFR algorithms.
//
// @see UDPVideoStreamSvrWithTrace2
//
simple UDPVideoStreamCliWithSCFR like UDPApp
{
    parameters:
        @display("i=block/app");
		// statistics: common
        @statistic[fragmentStart](title="indicators for start of fragment"; record=vector; interpolationmode=none);
		// statistics: aperiodic (i.e., for all packets)
        @statistic[endToEndDelayAperiodic](title="end-to-end delay (based on reference clock) of all packets"; record=vector; interpolationmode=none);
        @statistic[interArrivalTimeAperiodic](title="interarrival times (based on receiver clock) of all packets"; record=vector; interpolationmode=none);
        @statistic[interDepartureTimeAperiodic](title="interdeparture times (based on source clock) of all packets"; record=vector; interpolationmode=none);
        @statistic[clockFrequencyRatioAperiodic](title="measured clock frequency ratio between receiver and source based on all packets"; record=vector; interpolationmode=none);
		// statistics: periodic (i.e., for the first packets of frames)
        @statistic[endToEndDelayPeriodic](title="end-to-end delay (based on reference clock) of first packets of frames"; record=vector; interpolationmode=none);
        @statistic[interArrivalTimePeriodic](title="interarrival times (based on receiver clock) of first packets of frames"; record=vector; interpolationmode=none);
        @statistic[interDepartureTimePeriodic](title="interdeparture times (based on source clock) of first packets of frames"; record=vector; interpolationmode=none);
        @statistic[clockFrequencyRatioPeriodic](title="measured clock ratio between receiver and source based on first packets of frames"; record=vector; interpolationmode=none);
        int localPort;
        string serverAddress;
        int serverPort;
        double startTime @unit("s") = default(1s);
        double clockFrequency @unit("Hz") = default(16MHz); // frequency of a local clock; default from Arduino board
    gates:
        input udpIn @labels(UDPControlInfo/up);
        output udpOut @labels(UDPControlInfo/down);
}

