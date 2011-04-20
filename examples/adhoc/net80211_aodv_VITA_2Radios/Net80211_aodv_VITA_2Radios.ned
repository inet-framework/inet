//
// Copyright (C) 2010 Alfonso Ariza
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

package inet.examples.adhoc.net80211_aodv_VITA_2Radios;


import inet.world.ChannelControlExtended;
import inet.networklayer.autorouting.ipv4.NetworkConfigurator;


//
// Example network for testing aodv routing using WiMAX (802.16e). 
// Contains fixed and mobile nodes. Mobile nodes are not moving,
// but are scattered randomly on the playground.
//
network Net80211_aodv_VITA_2Radios
{
    parameters:
        int numHosts;
        int numFixHosts;
        double playgroundSizeX;
        double playgroundSizeY;
        @display("bgb=480,402");
    submodules:
        WiMAX_Basisstation[numFixHosts]: FixManetRoutingHostVITA {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=94,345");
        }
        vita[numHosts]: MobileManetRoutingHostVITA {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=250,176");
        }
        channelcontrol: ChannelControlExtended {
            parameters:
                playgroundSizeX = playgroundSizeX;
                playgroundSizeY = playgroundSizeY;
                @display("p=54,34;i=misc/sun");
        }
        configurator: NetworkConfigurator {
            parameters:
                //                networkAddress = "145.236.0.0";
                //                netmask = "255.255.0.0";
                @display("p=140,50;i=block/cogwheel_s");
        }
    connections allowunconnected:
}
