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

package inet.examples.quic.shared_link;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;

network shared_link
{
    parameters:
        double bottleneckPer = default(0);
        double bottleneckDelay @unit(s) = default(1ms);
        double bottleneckDatarate @unit(bps) = default(100Mbps);

    types:
        channel ideal extends ned.DatarateChannel
        {
            delay = 0ms;
            datarate = 1000Gbps;
        }
        channel bottleneck extends ned.DatarateChannel
        {
            datarate = parent.bottleneckDatarate;
            delay = parent.bottleneckDelay;
            per = parent.bottleneckPer;
        }
    submodules:
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=222,32");
        }

        router1: Router {
            parameters:
                @display("p=146,137");
        }

        router2: Router {
            parameters:
                @display("p=305,137");
        }

        sender1: StandardHost {
            parameters:
                @display("p=57,25");
        }

        sender2: StandardHost {
            parameters:
                @display("p=57,100");
        }

        sender3: StandardHost {
            parameters:
                @display("p=57,175");
        }

        trafficGenerator: StandardHost {
            parameters:
                @display("p=57,250");
        }

        receiver1: StandardHost {
            parameters:
                @display("p=390,25");
        }

        receiver2: StandardHost {
            parameters:
                @display("p=390,100");
        }

        receiver3: StandardHost {
            parameters:
                @display("p=390,175");
        }

        trafficConsumer: StandardHost {
            parameters:
                @display("p=390,250");
        }

    connections:
        sender1.pppg++ <--> ideal <--> router1.pppg++;
        sender2.pppg++ <--> ideal <--> router1.pppg++;
        sender3.pppg++ <--> ideal <--> router1.pppg++;
        trafficGenerator.pppg++ <--> ideal <--> router1.pppg++;
        
        router2.pppg++ <--> ideal <--> receiver1.pppg++;
        router2.pppg++ <--> ideal <--> receiver2.pppg++;
        router2.pppg++ <--> ideal <--> receiver3.pppg++;
        router2.pppg++ <--> ideal <--> trafficConsumer.pppg++;

        router1.pppg++ <--> bottleneck <--> router2.pppg++;

        
}
