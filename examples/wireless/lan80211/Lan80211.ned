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

package inet.examples.wireless.lan80211;

import inet.networklayer.autorouting.ipv4.IPv4NetworkConfigurator;
import inet.nodes.inet.WirelessHost;
import inet.nodes.wireless.AccessPoint;
import inet.world.radio.ChannelControl;


network Lan80211
{
    parameters:
        int numHosts;
    submodules:
        host[numHosts]: WirelessHost {
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
        configurator: IPv4NetworkConfigurator {
            config=xml("<config><interface hosts='*' address='145.236.x.x' netmask='255.255.0.0'/></config>");
            @display("p=140,50");
        }
}
