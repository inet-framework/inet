//
// Copyright (C) 2012 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program. If not, see http://www.gnu.org/licenses/.
//
//package inet.examples.manetrouting.multiradio;

package inet.examples.manetrouting.hna;

import inet.networklayer.autorouting.ipv4.IPv4NetworkConfigurator;
import inet.nodes.ethernet.Eth100M;
import inet.nodes.inet.AdhocHost;
import inet.nodes.inet.StandardHost;
import inet.world.radio.ChannelControl;
import inet.util.ThruputMeteringChannel;

network AdhocNet
{
    parameters:
        int netSize;
        @display("bgs=0.01");
    types:

        channel C extends ThruputMeteringChannel
        {
            delay = 0.1us;
            datarate = 100Mbps;
            thruputDisplayFormat = "#N";
        }

    submodules:
        channelControl: ChannelControl {
            @display("p=83,15");
        }

        networkConfigurator: IPv4NetworkConfigurator {
            @display("p=253,15");
            dumpAddresses = true;
            dumpTopology = true;
            dumpRoutes = true;
           // addDefaultRoutes = false;
           // addStaticRoutes = false;
           // addSubnetRoutes = false;
        }
        host1: StandardHost {
            @display("p=3100,6200");
        }
        gwHost1: AdhocHost {
            @display("p=7600,6200");
        }
        mobileHost[netSize * netSize]: AdhocHost {
            @display("p=15800,8500;r=,,#000040");
        }
        gwHost2: AdhocHost {
            @display("p=14800,3800");
        }
        host2: StandardHost {
            @display("p=21800,5000");
        }

    connections:
        host1.ethg++ <--> Eth100M <--> gwHost1.ethg++;
        host2.ethg++ <--> Eth100M <--> gwHost2.ethg++;
}
