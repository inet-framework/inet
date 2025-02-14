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

package inet.examples.quic.flow_control_limited;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;

network bottleneck_link
{
    parameters:
        double linkDelay @unit(s) = default(0ms);

    types:
        channel link extends ned.DatarateChannel
        {
            datarate = 100Gbps;
            delay = parent.linkDelay;
            per = 0;
        }
        channel idealLink extends ned.DatarateChannel
        {
            datarate = 1000Gbps;
            delay = 0ms;
            per = 0;
        }
    submodules:
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=222,32");
        }

        sender: StandardHost {
            parameters:
                @display("p=57,137");
        }

        router1: Router {
            parameters:
                @display("p=146,137");
        }

        router2: Router {
            parameters:
                @display("p=305,137");
        }

        receiver: StandardHost {
            parameters:
                @display("p=390,137");
        }

    connections:
        sender.pppg++ <--> idealLink <--> router1.pppg++;

        router2.pppg++ <--> idealLink <--> receiver.pppg++;

        router1.pppg++ <--> link <--> router2.pppg++;
}
