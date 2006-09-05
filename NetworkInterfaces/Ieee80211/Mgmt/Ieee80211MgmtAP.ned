//
// Copyright (C) 2006 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


//
// Used in 802.11 infrastructure mode: handles management frames for
// an access point (AP). It relies on a connected Ieee80211Mac for actual
// reception and transmission of frames.
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
simple Ieee80211MgmtAP
    parameters:
        ssid: string,
        beaconInterval: numeric const,
        frameCapacity: numeric const, // maximum queue length
        numAuthSteps: numeric const; // use 2 for Open System auth, 4 for WEP
        //dataRate: numeric; XXX TBD
    gates:
        in: uppergateIn; // from MACRelayUnit; can be left unconnected
        out: uppergateOut; // to MACRelayUnit; can be left unconnected
        in: macIn; // from Ieee80211Mac
        out: macOut; // to Ieee80211Mac
endsimple

