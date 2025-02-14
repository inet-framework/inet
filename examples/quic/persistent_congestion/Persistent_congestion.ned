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

package inet.examples.quic.persistent_congestion;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.common.scenario.ScenarioManager;

network Persistent_congestion
{
    parameters:

        @display("bgb=451,190");
    types:
        channel ideal extends ned.DatarateChannel
        {
            delay = 1ms;
            datarate = 100Gbps;
        }
        channel bottleneck extends ned.DatarateChannel
        {
            delay = 10ms;
            datarate = 100Mbps;
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

        sender: StandardHost {
            parameters:
                @display("p=57,137");
        }


        receiver: StandardHost {
            parameters:
                @display("p=390,137");
        }

        scenarioManager: ScenarioManager {
            @display("p=103,24;is=s");
        }

    connections:
        sender.pppg++ <--> ideal <--> router1.pppg++;

        router1.pppg++ <--> bottleneck <--> router2.pppg++;

        router2.pppg++ <--> ideal <--> receiver.pppg++;
}
