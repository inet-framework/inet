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
import inet.node.ethernet.Eth100M;
import inet.node.inet.StandardHost;


network Simplest
{
    @display("bgb=822,636");
    submodules:
        hostA: StandardHost {
            @display("p=125.44,247.04");
        }
        hostB: StandardHost {
            @display("p=510.72,247.04");
        }
        visualizer: RoutingTableVisualizer {
            @display("p=701.44,168.96");
        }
        configurator: IPv4NetworkConfigurator {
            @display("p=701.44,81.92");
        }
    connections:
        hostA.ethg++ <--> Eth100M <--> hostB.ethg++;
}
