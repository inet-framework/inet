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
// Used in 802.11 infrastructure mode: handles management frames for
// an access point (AP). It relies on a connected ~Ieee80211Mac for actual
// reception and transmission of frames.
//
// This management module variant does not implement beacons, probes,
// authentication and association -- it simply treats all stations
// as associated all the time. This also means that you cannot use this
// module in a simulation that involves handovers; for that you need
// to use the full version, ~Ieee80211MgmtAP.
//
// When simulating an AP that has other (Ethernet or wireless) interfaces
// as well, this module can be connected to a ~IMACRelayUnit which performs
// bridging of Ethernet frames. This module will perform conversion from/to
// ~EtherFrame for ~IMACRelayUnit.
//
// This module never switches channels, that is, it will operate on the channel
// the physical layer is configured for (see channelNumber in ~Ieee80211Radio).
//
// This module checks if there is an other module connected above the management
// module (e.g. RelayUnit) and sends a copy of the frame there and at the same time
// sends the frame back to the wirless interface. The relay unit will send it out
// on the interface which is towards the destination or will broadcast it on all
// interfaces (except on which it was received).
//
// @author Andras Varga
//
simple Ieee80211MgmtAPSimplified like IIeee80211Mgmt
{
    parameters:
        int frameCapacity = default(100);
        string encapDecap = default("eth") @enum("true", "false", "eth");   // if "eth", frames sent up are converted to EthernetIIFrame
        @display("i=block/cogwheel");
        @signal[enqueuePk](type=cPacket);
        @signal[dequeuePk](type=cPacket);
        @signal[dropPkByQueue](type=cPacket);
        @signal[queueingTime](type=simtime_t);
        @signal[dataQueueLen](type=long);  // number of stored packets
        @signal[rcvdPk](type=cPacket);
        @statistic[dataQueueLen](title="data queue length"; record=max,timeavg,vector; interpolationmode=sample-hold);
        @statistic[dropPkByQueue](title="packets dropped by queue"; source=dropPkByQueue; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
    gates:
        input upperLayerIn; // from ~IMACRelayUnit; can be left unconnected
        output upperLayerOut; // to ~IMACRelayUnit; can be left unconnected
        input macIn @labels(Ieee80211Frame); // from ~Ieee80211Mac
        output macOut @labels(Ieee80211Frame); // to ~Ieee80211Mac
        input agentIn @loose;   // unused dummy to statisfy the IIeee80211Mgmt interface
        output agentOut @loose; // unused dummy to statisfy the IIeee80211Mgmt interface
}
