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

package inet.examples.wireless.handover;

import inet.networklayer.autorouting.FlatNetworkConfigurator;
import inet.nodes.wireless.WirelessAP;
import inet.nodes.wireless.WirelessHost;
import inet.world.ChannelControl;


network HandoverNetwork
{
    parameters:
        double playgroundSizeX;
        double playgroundSizeY;
    submodules:
        host: WirelessHost {
            @display("p=50,130;r=,,#707070");
        }
        ap1: WirelessAP {
            @display("p=100,200;r=,,#707070");
        }
        ap2: WirelessAP {
            @display("p=500,200;r=,,#707070");
        }
        channelcontrol: ChannelControl {
            playgroundSizeX = playgroundSizeX;
            playgroundSizeY = playgroundSizeY;
            @display("p=60,50");
        }
        configurator: FlatNetworkConfigurator {
            @display("p=140,50");
        }
}

