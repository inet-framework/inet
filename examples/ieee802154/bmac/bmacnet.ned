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

package inet.examples.wpan.bmac;

import inet.networklayer.autorouting.FlatNetworkConfigurator;
import inet.nodes.adhoc.MobileHost;
import inet.experimental.nodes.adhoc.MobileManetRoutingMesh;
import inet.experimental.nodes.adhoc.BasicMobileManetMesh;
import inet.world.ChannelControl;
import inet.examples.wpan.bmac.nodebmac;


network bmacnet
{
    parameters:
        int numNodes;
        double nodeSeparation = default(40);
        double playgroundSizeX;
        double playgroundSizeY;
    submodules:
        node[numNodes]: nodebmac {
            parameters:
                y = (floor(index/sqrt(numNodes))*nodeSeparation)+(nodeSeparation);
                x = ((index%floor(sqrt(numNodes)))*nodeSeparation)+(nodeSeparation);
                @display("i=device/pocketpc_s;r=,,#707070");
        }
        node1: nodebmac {
            parameters:
                y = 100;
                x = 50;
                @display("i=device/pocketpc_s;r=,,#707070;p=270,100");
        }
        node2: nodebmac {
            parameters:
                y = 100;
                x = 90;
                @display("i=device/pocketpc_s;r=,,#707070;p=518,100");
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


