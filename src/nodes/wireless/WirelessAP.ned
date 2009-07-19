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




package inet.nodes.wireless;

import inet.base.NotificationBoard;
import inet.mobility.NullMobility;
import inet.linklayer.ieee80211.Ieee80211NicAP;


//
// Models a 802.11 Access Point. It can be used in conjunction with
// WirelessHost, or any other host model containing WirelessNicSTA.
//
// @see WirelessAP, WirelessAPSimplified, WirelessAPWithEth, WirelessAPWithEthSimplified
// @see WirelessHost, WirelessHostSimplified
// @see MobileHost, MFMobileHost
//
module WirelessAP
{
    parameters:
        @node();
        @labels(node,wireless-node);
        @display("i=device/accesspoint");
    gates:
        input radioIn @directIn;
    submodules:
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=79,74");
        }
        wlan: Ieee80211NicAP { // see also Ieee80211NicAPSimplified
            parameters:
                @display("p=110,179;q=queue");
        }
        mobility: NullMobility {
            parameters:
                @display("p=144,70");
        }
    connections allowunconnected:
        radioIn --> { @display("m=s"); } --> wlan.radioIn;
}

