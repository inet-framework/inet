//
// Copyright (C) 2004 Andras Varga
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



package org.omnetpp.inet.examples.INET.REDTest;

import org.omnetpp.inet.Network.AutoRouting.FlatNetworkConfigurator;
import org.omnetpp.inet.Nodes.INET.Router;
import org.omnetpp.inet.Nodes.INET.StandardHost;
import org.omnetpp.inet.World.ChannelInstaller;
import org.omnetpp.inet.World.ScenarioManager;


network REDTest
{
    submodules:
        channelInstaller: ChannelInstaller {
            parameters:
                channelClass = "ThruputMeteringChannel";
                channelAttrs = "format=b (u) #N";
                @display("p=359,40");
        }
        configurator: FlatNetworkConfigurator {
            parameters:
                moduleTypes = "Router StandardHost";
                nonIPModuleTypes = "";
                networkAddress = "145.236.0.0";
                netmask = "255.255.0.0";
                @display("p=179,40");
        }
        scenarioManager: ScenarioManager {
            parameters:
                @display("p=262,40");
        }
        r1: Router {
            parameters:
                @display("p=147,143");
        }
        r2: Router {
            parameters:
                @display("p=317,143");
        }
        s1: StandardHost {
            parameters:
                @display("p=68,64;i=device/laptop");
        }
        s2: StandardHost {
            parameters:
                @display("p=65,193;i=device/laptop");
        }
        s3: StandardHost {
            parameters:
                @display("p=421,83;i=device/laptop");
        }
        s4: StandardHost {
            parameters:
                @display("p=420,195;i=device/laptop");
        }
    connections allowunconnected:
        r1.pppg++ <--> {  datarate = 1.5Mbps; delay = 20ms; } <--> r2.pppg++;
        s1.pppg++ <--> {  datarate = 10Mbps; delay = 2ms; } <--> r1.pppg++;
        s2.pppg++ <--> {  datarate = 10Mbps; delay = 3ms; } <--> r1.pppg++;
        s3.pppg++ <--> {  datarate = 10Mbps; delay = 4ms; } <--> r2.pppg++;
        s4.pppg++ <--> {  datarate = 10Mbps; delay = 5ms; } <--> r2.pppg++;
}

