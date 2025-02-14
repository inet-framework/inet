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

package inet.examples.quic.ptb;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.common.scenario.ScenarioManager;

network Bottleneck3_
{
    parameters:
        double oneWayDelay @unit(s) = default(10ms);
        double link1Datarate @unit(bps) = default(1Gbps);
        double link1Delay @unit(s) = default(oneWayDelay/3);
		double link1Per = default(0);
        double link2Datarate @unit(bps) = default(100Mbps);
        double link2Delay @unit(s) = default(oneWayDelay/3);
		double link2Per = default(0);
        double link2_Datarate @unit(bps) = default(100Mbps);
        double link2_Delay @unit(s) = default(oneWayDelay/3);
		double link2_Per = default(0);
        double link3Datarate @unit(bps) = default(1Gbps);
        double link3Delay @unit(s) = default(oneWayDelay/3);
		double link3Per = default(0);
    types:
        channel link1 extends ned.DatarateChannel
        {
            datarate = parent.link1Datarate;
            delay = parent.link1Delay;
            per = parent.link1Per;
        }
        channel link2 extends ned.DatarateChannel
        {
            datarate = parent.link2Datarate;
            delay = parent.link2Delay;
            per = parent.link2Per;
        }
        channel link2_ extends ned.DatarateChannel
        {
            datarate = parent.link2_Datarate;
            delay = parent.link2_Delay;
            per = parent.link2_Per;
        }
        channel link3 extends ned.DatarateChannel
        {
            datarate = parent.link3Datarate;
            delay = parent.link3Delay;
            per = parent.link3Per;
        }
    submodules:
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=222,32");
            	config = xmldoc("Bottleneck3_config.xml");
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

        scenarioManager: ScenarioManager {
            @display("p=103,24;is=s");
        }

    connections:
        sender.pppg++ <--> link1 <--> router1.pppg++;
        router1.pppg++ <--> link2 <--> router2.pppg++;
        router1.pppg++ <--> link2_ <--> router2.pppg++;
        router2.pppg++ <--> link3 <--> receiver.pppg++;
}