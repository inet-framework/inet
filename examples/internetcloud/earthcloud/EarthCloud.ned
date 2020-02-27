//
// Copyright (C) 2012 OpenSim Ltd.
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.examples.internetcloud.earthcloud;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.EtherLink;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.node.internetcloud.InternetCloud;
import ned.DatarateChannel;


network SimpleContinentCloud
{
    parameters:
        int numClients;
        *.eth[*].bitrate = 100 Mbps;
        @display("i=device/lan-ring");
    gates:
        inout pppg[];
    submodules:
        router: Router {
            @display("p=250,100");
        }
        client[numClients]: StandardHost {
            @display("p=400,100,col,100");
        }
        server: StandardHost {
            @display("p=100,100");
        }
    connections:
        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> router.pppg++;
        }
        router.ethg++ <--> EtherLink <--> server.ethg++;
        for i=0..numClients-1 {
            router.ethg++ <--> EtherLink <--> client[i].ethg++;
        }
}

network EarthCloud
{
    parameters:
        *.eth[*].bitrate = 1 Gbps;
        @display("bgi=maps/world;bgb=875,414");
    types:
        channel ppplink extends DatarateChannel
        {
            delay = 2 ms;
            datarate = 100 Mbps;
        }
    submodules:
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,200;is=s");
        }

        internet: InternetCloud {
            @display("p=387,185;is=vl");
        }
        africa: SimpleContinentCloud {
            @display("p=465,288");
        }
        asia: SimpleContinentCloud {
            @display("p=662,121");
        }
        australia: SimpleContinentCloud {
            @display("p=759,299;");
        }
        europe: SimpleContinentCloud {
            @display("p=446,77");
        }
        northAmerica: SimpleContinentCloud {
            @display("p=170,120");
        }
        southAmerica: SimpleContinentCloud {
            @display("p=260,270");
        }
    connections:
        africa.pppg++ <--> ppplink <--> internet.pppg++;
        asia.pppg++ <--> ppplink <--> internet.pppg++;
        australia.pppg++ <--> ppplink <--> internet.pppg++;
        europe.pppg++ <--> ppplink <--> internet.pppg++;
        northAmerica.pppg++ <--> ppplink <--> internet.pppg++;
        southAmerica.pppg++ <--> ppplink <--> internet.pppg++;
}

