//
// Copyright (C) 2006 Andras Varga
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


package inet.linklayer.ieee80211.mgmt;

//
// Used in 802.11 infrastructure mode: handles management frames for a station (STA).
//
// Relies on the MAC layer (~Ieee80211Mac) for reception and transmission of frames.
//
// @author Andras Varga
//
simple Ieee80211MgmtSTA like IIeee80211Mgmt
{
    parameters:
        int frameCapacity = default(100); // maximum queue length
        @display("i=block/cogwheel");
        @signal[enqueuePk](type=cMessage);
        @signal[dequeuePk](type=cMessage);
        @signal[dropPkByQueue](type=cMessage);
        @signal[queueingTime](type=simtime_t; unit=s);
        @signal[dataQueueLen](type=long);  // number of stored packets
        @statistic[dataQueueLen](title="data queue length"; record=max,timeavg,vector; interpolationmode=sample-hold);
        @statistic[dropPkByQueue](title="packets dropped by queue"; source=dropPkByQueue; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input macIn @labels(Ieee80211Frame);
        output macOut @labels(Ieee80211Frame);
        input agentIn;   // used for attaching an agent algorithm
        output agentOut; // used for attaching an agent algorithm
}

