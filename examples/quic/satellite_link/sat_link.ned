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
// Ekaterina Volodina uni-due.de 
// 
//  Medium public satellite broadband access:
//  Satellite downlink path: 50 Mbps
//  Satellite uplink path: 10 Mbps
//  No emulated packet loss
//  RTT: 650 ms
//  Buffer size : BDP

package inet.examples.quic.satellite_link;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;

network sat_link
{
    parameters:
        double bottleneckPer = default(0);
        double bottleneckDelay @unit(s) = default(1ms);
        double bottleneckDatarate @unit(bps) = default(100Mbps);
        double bottleneckDownlinkDatarate @unit(bps) = default(100Mbps);
        double bottleneckUplinkDatarate @unit(bps) = default(100Mbps);

        @display("bgb=1158,584");
        
    types:
        channel etherline extends ned.DatarateChannel
        {
            delay = 0ms;
            datarate = 0bps;
        }

        channel bottleneck extends ned.DatarateChannel
        {
            datarate = bottleneckDatarate;
            delay = bottleneckDelay;
            per = bottleneckPer;
        }
        channel bottleneck_downlink extends ned.DatarateChannel
        {
            datarate = bottleneckDownlinkDatarate;//10Mbps;//bottleneckDatarate; 8.5Mbps; 50Mbps; 250
            delay = 300ms;//bottleneckDelay; 300ms
            per = bottleneckPer;
        }

        channel bottleneck_uplink extends ned.DatarateChannel
        {
            datarate = bottleneckUplinkDatarate;//2Mbps;//bottleneckDatarate/10; // uplink 10% of downlink //1.5Mbps //10Mbps //3
            delay = 300ms;//bottleneckDelay; //300ms
            per = bottleneckPer;
        }
    submodules:
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=222,32");              
        }

        router1: Router {
            parameters:
                @display("p=350.0525,174.3");
        }

        router2: Router {
            parameters:
                @display("p=691.39,174.3");
        }

        router3: Router {
            parameters:
                @display("p=350.0525,419.7725");
        }

        router4: Router {
            parameters:
                @display("p=691.39,419.7725");
        }

        router5: Router {
            parameters:
                @display("p=187.3725,284.69");
        }

        router6: Router {
            parameters:
                @display("p=852.6175,284.69");
        }

        client: StandardHost {
            parameters:
                @display("i=device/laptop;p=50.8375,286.1425");
        }


        server: StandardHost {
            parameters:
                @display("p=1048.705,286.1425;i=device/server_l");
        }

    connections:
        client.pppg++ <--> etherline <--> router5.pppg++;
        server.pppg++ <--> etherline <--> router6.pppg++;

        router1.pppg++ <--> etherline <--> router5.pppg++;
        router2.pppg++ <--> etherline <--> router6.pppg++;

        router3.pppg++ <--> etherline <--> router5.pppg++;
        router4.pppg++ <--> etherline <--> router6.pppg++;

        router1.pppg++ <--> bottleneck_downlink <--> router2.pppg++;
        router3.pppg++ <--> bottleneck_uplink <--> router4.pppg++;


}