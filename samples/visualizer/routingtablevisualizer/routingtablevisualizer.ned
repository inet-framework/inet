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

package inet.samples.visualizer.routingtablevisualizer;

import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.aodv.AODVRouter;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EtherSwitch;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedVisualizer;

network RoutingTableVisualizerA
{
    @display("bgb=822,636");
    submodules:
        hostA: StandardHost {
            @display("p=125.44,247.04");
        }
        hostB: StandardHost {
            @display("p=510.72,247.04");
        }
        visualizer: inet.visualizer.networklayer.RoutingTableVisualizer {
            @display("p=701.44,168.96");
        }
        configurator: IPv4NetworkConfigurator {
            @display("p=701.44,81.92");
        }
    connections:
        hostA.ethg++ <--> Eth100M <--> hostB.ethg++;
}

network RoutingTableVisualizerB
{
    @display("bgb=822,636");

    submodules:
        host1: StandardHost {
            @display("p=136.96,225.28");
        }
        host2: StandardHost {
            @display("p=136.96,355.84");
        }
        host3: StandardHost {
            @display("p=634.88,225.28");
        }
        host4: StandardHost {
            @display("p=636.16,363.52");
        }
        switch: EtherSwitch {
            @display("p=299.52,302.08");
        }
        router: Router {
            @display("p=476.16,302.08");
        }
        visualizer: IntegratedVisualizer {
            @display("p=759.04,51.2");
        }
        configurator: IPv4NetworkConfigurator {
            @display("p=759.04,124.16");
        }
    connections:
        host1.ethg++ <--> Eth100M <--> switch.ethg++;
        host2.ethg++ <--> Eth100M <--> switch.ethg++;
        switch.ethg++ <--> Eth100M <--> router.ethg++;
        router.ethg++ <--> Eth100M <--> host3.ethg++;
        router.ethg++ <--> Eth100M <--> host4.ethg++;
}

network RoutingTableVisualizerC
{
    @display("bgb=822,636");
    submodules:
        sourceHost: AODVRouter {
            @display("p=120,57");
        }
        host1: AODVRouter {
            @display("p=119.04,148.48");
        }
        host2: AODVRouter {
            @display("p=119.04,241.92");
        }
        host3: AODVRouter {
            @display("p=119.04,341.76");
        }
        host4: AODVRouter {
            @display("p=119.04,451.84");
        }
        host5: AODVRouter {
            @display("p=119.04,554.24");
        }
        destinationHost: AODVRouter {
            @display("p=272.64,56.32");
        }
        configurator: IPv4NetworkConfigurator {
            @display("p=556,101");
        }
        visualizer: IntegratedVisualizer {
            @display("p=556,179");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=560,255");
        }
}




