//
// Copyright (C) 2008 Feng Chen
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

package inet.examples.wpan.csma802154;

import inet.networklayer.autorouting.FlatNetworkConfigurator;
import inet.nodes.adhoc.MobileHost;
import inet.experimental.nodes.adhoc.MobileManetRoutingMesh;
import inet.experimental.nodes.adhoc.BasicMobileManetMesh;
import inet.world.ChannelControl;
import inet.examples.wpan.csma802154.nodecsma802154;


network csma802454net
{
    parameters:
        int numHosts;
        int numFixHosts;
        int numBasic;
        double nodeSeparation =default (40);
        double playgroundSizeX;
        double playgroundSizeY;
    submodules:
        fixhost[numFixHosts]: nodecsma802154 {
            parameters:
              y =  (floor(index/sqrt(numFixHosts))*nodeSeparation)+(nodeSeparation);
              x = ((index%floor(sqrt(numFixHosts)))*nodeSeparation)+(nodeSeparation);
              @display("i=device/pocketpc_s;r=,,#707070");
        }
        channelcontrol: ChannelControl {
            parameters:
                playgroundSizeX = playgroundSizeX;
                playgroundSizeY = playgroundSizeY;
                @display("p=60,50;i=misc/sun");
        }
        configurator: FlatNetworkConfigurator {
            parameters:
                networkAddress = "145.236.0.0";
                netmask = "255.255.0.0";
                @display("p=140,50;i=block/cogwheel_s");
        }
    connections allowunconnected:
}


