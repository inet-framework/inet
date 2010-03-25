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
// an access point (AP). It relies on a connected Ieee80211Mac for actual
// reception and transmission of frames.
//
// This management module variant does not implement beacons, probes,
// authentication and association -- it simply treats all stations
// as associated all the time. This also means that you cannot use this
// module in a simulation that involves handovers; for that you need
// to use the full version, Ieee80211MgmtAP.
//
// When simulating an AP that has other (Ethernet or wireless) interfaces
// as well, this module can be connected to a MACRelayUnit which performs
// bridging of Ethernet frames. This module will perform conversion from/to
// EtherFrame for MACRelayUnit.
//
// This module never switches channels, that is, it will operate on the channel
// the physical layer is configured for (see channelNumber in Ieee80211Radio).
//
// @author Andras Varga
//
simple Ieee80211MgmtAPSimplified like Ieee80211Mgmt
{
    parameters:
        int frameCapacity = default(100);
        @display("i=block/cogwheel");
    gates:
        input uppergateIn; // from MACRelayUnit; can be left unconnected
        output uppergateOut; // to MACRelayUnit; can be left unconnected
        input macIn @labels(Ieee80211Frame); // from Ieee80211Mac
        output macOut @labels(Ieee80211Frame); // to Ieee80211Mac
}
