//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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






package org.omnetpp.inet.examples.INET.RouterPerf;

import org.omnetpp.inet.Network.AutoRouting.FlatNetworkConfigurator;
import org.omnetpp.inet.Nodes.INET.BurstHost;
import org.omnetpp.inet.Nodes.INET.Router;
import org.omnetpp.inet.World.ChannelInstaller;


network RouterPerfNetwork
{
    parameters:
        int nodeNo;
    submodules:
        channelInstaller: ChannelInstaller {
            parameters:
                channelClass = "ThruputMeteringChannel";
                channelAttrs = "format=u";
                @display("p=59,211");
        }
        configurator: FlatNetworkConfigurator {
            parameters:
                moduleTypes = "Router BurstHost";
                nonIPModuleTypes = "";
                networkAddress = "145.236.0.0";
                netmask = "255.255.0.0";
                @display("p=61,163");
        }
        sender[nodeNo]: BurstHost {
            parameters:
                @display("p=100,300,row,100");
        }
        recip[nodeNo]: BurstHost {
            parameters:
                @display("p=100,80,row,100");
        }
        router: Router {
            parameters:
                @display("p=210,190");
        }
    connections allowunconnected:
        for i=0..nodeNo-1 {
            sender[i].pppg++ <--> {  datarate = 1Mbps; } <--> router.pppg++;
            recip[i].pppg++ <--> {  datarate = 1Mbps; } <--> router.pppg++;
        }
}

