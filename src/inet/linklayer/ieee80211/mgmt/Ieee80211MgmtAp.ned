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
// @author Andras Varga
//

package inet.linklayer.ieee80211.mgmt;

//
// Used in 802.11 infrastructure mode: handles management frames for
// an access point (AP). It relies on a connected ~Ieee80211Mac for actual
// reception and transmission of frames.
//
// When simulating an AP that has other (Ethernet or wireless) interfaces
// as well, this module can be connected to a ~IMacRelayUnit which performs
// bridging of Ethernet frames. This module will perform conversion from/to
// ~EtherFrame for ~IMacRelayUnit.
//
// This module never switches channels, that is, it will operate on the channel
// the physical layer is configured for (see channelNumber in ~Ieee80211Radio).
//
// This module checks if there is an other module connected above the management
// module (e.g. RelayUnit) and sends the frame there if the frame's destination
// is not on the associated station's list. Multicast frames are broadcasted on
// the wireless card and a copy sent also up to the relay unit so it can broadcast
// it on other interfaces.
//
simple Ieee80211MgmtAp like IIeee80211Mgmt
{
    parameters:
        string ssid = default("SSID");
        double beaconInterval @unit(s) = default(100ms);
        int numAuthSteps = default(4); // use 2 for Open System auth, 4 for WEP
        //dataRate: numeric; XXX TBD
        string interfaceTableModule;
        string radioModule = default("^.radio");   // The path to the Radio module  //FIXME remove default value
        string mibModule;
        string macModule;               // The path to the MAC module
        @display("i=block/cogwheel");
        @signal[l2ApAssociated](type="inet::ieee80211::Ieee80211MgmtAp::NotificationInfoSta");
        @signal[l2ApDisassociated](type="inet::ieee80211::Ieee80211MgmtAp::NotificationInfoSta");
    gates:
        input macIn @labels(Ieee80211MacHeader); // from ~Ieee80211Mac
        output macOut @labels(Ieee80211MacHeader); // to ~Ieee80211Mac
        input agentIn @loose;   // unused dummy to statisfy the IIeee80211Mgmt interface
        output agentOut @loose; // unused dummy to statisfy the IIeee80211Mgmt interface
}

