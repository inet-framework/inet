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

package inet.examples.bgpv4.BgpEx3_3routers;

import inet.common.misc.ThruputMeteringChannel;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.bgp.BgpRouter;
import inet.node.inet.StandardHost;

import inet.common.scenario.ScenarioManager;

import inet.node.bgp.BgpRouter6;
import inet.networklayer.configurator.ipv6.Ipv6FlatNetworkConfigurator;

network BGPTest3
{
    types:
        channel LINK_100 extends ThruputMeteringChannel
        {
            parameters:
                delay = 0s;
                datarate = 100Mbps;
                thruputDisplayFormat = "#N";
        }
    submodules:
        R1: BgpRouter6 {
            parameters:
                @display("p=100,100");
            gates:
                ethg[1];
        }
        R2: BgpRouter6 {
            parameters:
                @display("p=300,100");
            gates:
                ethg[3];
        }
        R3: BgpRouter6 {
            parameters:
                @display("p=450,100");
            gates:
                ethg[3];
        }
        R4: BgpRouter6 {
            parameters:
                @display("p=650,100");
            gates:
                ethg[1];
        }
        R5: BgpRouter6 {
            parameters:
                @display("p=375,200");
            gates:
                ethg[3];
        }
        R6: BgpRouter6 {
            parameters:
                @display("p=375,300");
            gates:
                ethg[1];
        }


        configurator: Ipv4NetworkConfigurator {
            @display("p=50,20;is=s");
         	config = xmldoc("IPv4Config.xml");
            addStaticRoutes = false;
            addDefaultRoutes = false;
            addSubnetRoutes = false;
        }
        
         scenarioManager: ScenarioManager {
            @display("p=150,20;is=s");
        }

 //       configurator6: Ipv6FlatNetworkConfigurator {
//            @display("p=150,20;is=s");
//        }

    connections:
        R1.ethg[0] <--> LINK_100 <--> R2.ethg[0];

        R2.ethg[1] <--> LINK_100 <--> R3.ethg[1];
        R2.ethg[2] <--> LINK_100 <--> R5.ethg[1];

        R3.ethg[0] <--> LINK_100 <--> R4.ethg[0];
        R3.ethg[2] <--> LINK_100 <--> R5.ethg[2];
        
        R5.ethg[0] <--> LINK_100 <--> R6.ethg[0];



}
