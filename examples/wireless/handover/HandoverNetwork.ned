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

import inet.networklayer.autorouting.ipv4.IPv4NetworkConfigurator;
import inet.nodes.inet.WirelessHost;
import inet.nodes.wireless.AccessPoint;
import inet.world.radio.ChannelControl;


network HandoverNetwork
{
    parameters:
    submodules:
        host: WirelessHost {
            @display("p=50,130;r=,,#707070");
        }
        ap1: AccessPoint {
            @display("p=100,200;r=,,#707070");
        }
        ap2: AccessPoint {
            @display("p=500,200;r=,,#707070");
        }
        channelControl: ChannelControl {
            @display("p=60,50");
        }
        configurator: IPv4NetworkConfigurator {
            @display("p=140,50");
        }
}

