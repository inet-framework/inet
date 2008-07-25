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

import inet.networklayer.autorouting.FlatNetworkConfigurator;
import inet.nodes.wireless.WirelessAPSimplified;
import inet.nodes.wireless.WirelessHostSimplified;
import inet.world.ChannelControl;


network Lan80211
{
    parameters:
        int numHosts;
        int playgroundSizeX;
        int playgroundSizeY;
    submodules:
        host[numHosts]: WirelessHostSimplified {
            @display("r=,,#707070");
        }
        ap: WirelessAPSimplified {
            @display("p=213,174;r=,,#707070");
        }
        channelcontrol: ChannelControl {
            playgroundSizeX = playgroundSizeX;
            playgroundSizeY = playgroundSizeY;
            numChannels = 2;
            @display("p=61,46");
        }
        configurator: FlatNetworkConfigurator {
            networkAddress = "145.236.0.0";
            netmask = "255.255.0.0";
            @display("p=140,50");
        }
}
