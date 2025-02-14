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

package inet.examples.quic.blackhole;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.common.scenario.ScenarioManager;

network Bottleneck
{
    parameters:
        double linkDatarate @unit(bps) = default(1Gbps);
        double linkDelay @unit(s) = default(0ms);
        double linkPer = default(0);
        double bottleneckDatarate @unit(bps) = default(100Mbps);
        double bottleneckDelay @unit(s) = default(10ms);
        double bottleneckPer = default(0);
        @display("bgb=451,350");
    types:
        channel link extends ned.DatarateChannel
        {
            datarate = parent.linkDatarate;
            delay = parent.linkDelay;
            per = parent.linkPer;
        }
        channel bottleneckLink extends ned.DatarateChannel
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
                @display("p=144,190");
        }

        router2: Router {
            parameters:
                @display("p=303,190");
        }

        sender: StandardHost {
            parameters:
                @display("p=55,190");
        }

        receiver: StandardHost {
            parameters:
                @display("p=388,190");
        }

        sender1: StandardHost {
            parameters:
                @display("p=55,77");
        }

        receiver1: StandardHost {
            parameters:
                @display("p=388,77");
        }

        sender2: StandardHost {
            parameters:
                @display("p=55,133");
        }

        receiver2: StandardHost {
            parameters:
                @display("p=388,133");
        }

        sender3: StandardHost {
            parameters:
                @display("p=55,248");
        }

        receiver3: StandardHost {
            parameters:
                @display("p=388,248");
        }

        sender4: StandardHost {
            parameters:
                @display("p=55,305");
        }

        receiver4: StandardHost {
            parameters:
                @display("p=388,305");
        }

        scenarioManager: ScenarioManager {
            @display("p=103,24;is=s");
        }

    connections:
        sender.pppg++ <--> link <--> router1.pppg++;
        router2.pppg++ <--> link <--> receiver.pppg++;

        router1.pppg++ <--> bottleneckLink <--> router2.pppg++;

        sender1.pppg++ <--> link <--> router1.pppg++;
        router2.pppg++ <--> link <--> receiver1.pppg++;
        sender2.pppg++ <--> link <--> router1.pppg++;
        router2.pppg++ <--> link <--> receiver2.pppg++;
        sender3.pppg++ <--> link <--> router1.pppg++;
        router2.pppg++ <--> link <--> receiver3.pppg++;
        sender4.pppg++ <--> link <--> router1.pppg++;
        router2.pppg++ <--> link <--> receiver4.pppg++;
}
