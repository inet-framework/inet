//
// Copyright (C) 2010 Alfonso Ariza
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//
package inet.examples.manetrouting.multiradio;

import inet.networklayer.autorouting.ipv4.IPv4NetworkConfigurator;
import inet.nodes.inet.AdhocHost;
import inet.world.radio.ChannelControl;


//
// Models a network with several hosts. Each host may contain one or
// more radios. Nodes are using adhoc routing to pass information.
// It is necessary to use ChannelControlExtended instead of
// ChannelControl because only the extended version handles multiple radios.
//
network MultiRadio
{
    parameters:
        int numHosts;
    submodules:
        channelControl: ChannelControl {
            @display("p=83,15");
        }
        networkConfigurator: IPv4NetworkConfigurator {
            @display("p=253,15");
        }
        mobileHost[numHosts]: AdhocHost {
            @display("p=100,100;r=,,#000040");
        }
    connections allowunconnected:
}
