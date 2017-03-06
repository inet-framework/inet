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

package inet.samples.visualizer.transportroutevisualizer;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.inet.StandardHost;
import inet.visualizer.transportlayer.TransportRouteVisualizer;


//
// TODO documentation
//
network TransportRouteSimple
{
    @display("bgb=500,200");
    submodules:
        source: StandardHost {
            @display("p=200,80");
        }
        destination: StandardHost {
            @display("p=400,80");
        }
        routeVisualizer: TransportRouteVisualizer {
            @display("p=80,50");
        }
        configurator: IPv4NetworkConfigurator {
            @display("p=80,120");
        }
    connections:
        source.ethg++ <--> Eth100M <--> destination.ethg++;
}

