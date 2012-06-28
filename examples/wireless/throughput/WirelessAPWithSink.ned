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



package inet.examples.wireless.throughput;

import inet.mobility.models.StationaryMobility;
import inet.linklayer.ieee80211.Ieee80211Nic;
import inet.base.Sink;
import inet.base.NotificationBoard;


//
// Well, this models a 802.11 Access Point with a Sink.
//
module WirelessAPWithSink
{
    parameters:
        @node();
        @labels(node);
        @display("i=device/accesspoint");
    gates:
        input radioIn @directIn;
    submodules:
        sink: Sink {
            parameters:
                @display("p=169,124");
        }
        wlan: Ieee80211Nic {
            parameters:
                @display("p=89,225;q=queue");
                mgmtType = default("Ieee80211MgmtAPSimplified");
                mgmt.encapDecap = "false";
        }
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=78,60");
        }
        mobility: StationaryMobility {
            parameters:
                @display("p=71,121");
        }
    connections allowunconnected:
        radioIn --> { @display("m=s"); } --> wlan.radioIn;
        wlan.upperLayerOut --> sink.in++;
}

