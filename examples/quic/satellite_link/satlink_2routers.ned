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
// Ekaterina Volodina
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package inet.examples.quic.satellite_link;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;

network satlink_2routers
{
    parameters:
        double uplinkPer = default(0);
        double downlinkPer = default(0);
        double linkDelay @unit(s) = default(0ms);
        double linkDatarate @unit(bps) = default(0Mbps);
        double downlinkDatarate @unit(bps) = default(100Mbps);
        double uplinkDatarate @unit(bps) = default(100Mbps);

    types:
        
        channel uplink extends ned.DatarateChannel
        {
            datarate = uplinkDatarate;
            delay = linkDelay;
            per = uplinkPer;
        }
        channel downlink extends ned.DatarateChannel
        {
            datarate = downlinkDatarate;
            delay = linkDelay;
            per = downlinkPer;
        }
        channel idealLink extends ned.DatarateChannel
        {
            datarate = 0Mbps;
            delay = 0ms;
            per = 0;
        }
    submodules:
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=222,32");
                // route packets from router1 to router2 over uplink 
                config = xml("<config> <interface hosts=\"**\" address=\"10.x.x.x\" netmask=\"255.x.x.x\"/> \
                			  <interface hosts='client' names='ppp0' address='10.0.0.1'/> \
                              <interface hosts='server' names='ppp0' address='10.0.0.14'/> \
                			  <route hosts=\"router1\" destination=\"10.0.0.14\" netmask=\"255.255.255.255\" interface=\"ppp1\"/> \
                			  <route hosts=\"router2\" destination=\"10.0.0.1\" netmask=\"255.255.255.255\" interface=\"ppp2\"/> \
                			  </config>");
        }

        client: StandardHost {
            parameters:
                @display("i=device/laptop;p=57,137");
        }


        router1: Router {
            parameters:
                @display("p=146,137");
        }

        router2: Router {
            parameters:
                @display("p=305,137");
        }

        server: StandardHost {
            parameters:
                @display("p=390,137;i=device/server_l");
        }

    connections:
        client.pppg++ <--> idealLink <--> router1.pppg++;

        router2.pppg++ <--> idealLink <--> server.pppg++;
        
        router1.pppg++ <--> downlink <--> router2.pppg++;
        router1.pppg++ <--> uplink <--> router2.pppg++;
}

