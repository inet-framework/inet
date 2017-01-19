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

package inet.samples.visualizer.networkroutevisualizer;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.inet.StandardHost;
import inet.visualizer.integrated.IntegratedVisualizer;
import inet.visualizer.networklayer.NetworkRouteVisualizer;


//
// TODO documentation
//
network NetworkRouteSimple
{
    @display("bgb=891,488");
    submodules:
        routeVisualizer: NetworkRouteVisualizer {
            parameters:
                @display("p=100,50");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                @display("p=100,150");
        }
        visualizer: IntegratedVisualizer {
            parameters:
                @display("p=100,250");
        }
        source: StandardHost {
            @display("p=300,150");
        }
        destination: StandardHost {
            @display("p=700,150");
        }

    connections:
        source.ethg++ <--> Eth100M <--> destination.ethg++;
}

