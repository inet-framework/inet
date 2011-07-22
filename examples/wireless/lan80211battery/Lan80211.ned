//
// Copyright (C) 2005 Andras Varga
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

package inet.examples.wireless.lan80211battery;

import inet.battery.models.SimpleBattery;
import inet.networklayer.autorouting.ipv4.FlatNetworkConfigurator;
import inet.nodes.inet.WirelessHost;
import inet.nodes.wireless.AccessPoint;
import inet.world.radio.ChannelControl;


module WirelessHostBattery extends WirelessHost
{
    @display("i=device/wifilaptop");
    numRadios = default(1);

    submodules:
        battery: SimpleBattery {
            @display("p=565,298");
        }
}

network Lan80211Battery
{
    parameters:
        int numHosts;
    submodules:
        host[numHosts]: WirelessHostBattery {
            @display("r=,,#707070");
            wlan[*].mgmtType = "Ieee80211MgmtSTASimplified";
        }
        ap: AccessPoint {
            @display("p=213,174;r=,,#707070");
            wlan[*].mgmtType = "Ieee80211MgmtAPSimplified";
        }
        channelControl: ChannelControl {
            numChannels = 2;
            @display("p=61,46");
        }
        configurator: FlatNetworkConfigurator {
            networkAddress = "145.236.0.0";
            netmask = "255.255.0.0";
            @display("p=140,50");
        }
}
