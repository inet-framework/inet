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

package inet.examples.obstacles;

import inet.networklayer.autorouting.ipv4.FlatNetworkConfigurator;
import inet.nodes.inet.AdhocHost;
import inet.world.annotations.AnnotationManager;
import inet.world.obstacles.ObstacleControl;
import inet.world.radio.ChannelControl;


network Net80211
{
    parameters:
        int numHosts;
        double playgroundSizeX;
        double playgroundSizeY;
        @display("bgb=818,436");
    submodules:
        host[numHosts]: AdhocHost {
            parameters:
                @display("r=,,#707070");
        }
        channelControl: ChannelControl {
            parameters:
                @display("p=60,50");
        }
        obstacles: ObstacleControl {
            @display("p=60,230");
        }
        annotations: AnnotationManager {
            @display("p=60,320");
        }
        configurator: FlatNetworkConfigurator {
            @display("p=60,140");
        }
}

