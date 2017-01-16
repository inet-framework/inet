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
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedVisualizer;


network RoutingTableVisualizer
{
    @display("bgb=822,636");
    submodules:
        source: AODVRouter {
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
        destination: AODVRouter {
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
