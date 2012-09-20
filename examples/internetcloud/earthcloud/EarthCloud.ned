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

import inet.networklayer.autorouting.ipv4.IPv4NetworkConfigurator;
import inet.nodes.ethernet.Eth100M;
import inet.nodes.inet.Router;
import inet.nodes.inet.StandardHost;
import inet.nodes.internetcloud.InternetCloud;
import ned.DatarateChannel;


network SimpleContinentCloud
{
    parameters:
        int numClients;
        @display("i=device/lan-ring");
    gates:
        inout pppg[];
    submodules:
        router: Router;
        client[numClients]: StandardHost;
        server: StandardHost;
    connections:
        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> router.pppg++;
        }
        router.ethg++ <--> Eth100M <--> server.ethg++;
        for i=0..numClients-1 {
            router.ethg++ <--> Eth100M <--> client[i].ethg++;
        }
}

network EarthCloud
{
    @display("bgi=maps/world;bgb=875,414");
    submodules:
        configurator: IPv4NetworkConfigurator {
            parameters:
                @display("p=75,315");
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
            @display("p=153,90");
        }
        southAmerica: SimpleContinentCloud {
            @display("p=260,270");
        }
    connections:
        africa.pppg++ <--> {  datarate = 100Mbps; delay = 2ms; } <--> internet.pppg++;
        asia.pppg++ <--> {  datarate = 100Mbps; delay = 2ms; } <--> internet.pppg++;
        australia.pppg++ <--> {  datarate = 100Mbps; delay = 2ms; } <--> internet.pppg++;
        europe.pppg++ <--> {  datarate = 100Mbps; delay = 2ms; } <--> internet.pppg++;
        northAmerica.pppg++ <--> {  datarate = 100Mbps; delay = 2ms; } <--> internet.pppg++;
        southAmerica.pppg++ <--> {  datarate = 100Mbps; delay = 2ms; } <--> internet.pppg++;
}

