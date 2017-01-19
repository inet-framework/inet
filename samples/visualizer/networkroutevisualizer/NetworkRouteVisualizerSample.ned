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
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.visualizer.integrated.IntegratedVisualizer;
import inet.visualizer.networklayer.NetworkRouteVisualizer;


//
// TODO documentation
//
network NetworkRouteSimple
{
    @display("bgb=800,300");
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

//
// TODO documentation
//
network NetworkRouteExtended
{
    @display("bgb=1000,600");
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
            @display("p=250,50");
        }
        destination: StandardHost {
            @display("p=950,550");
        }

        router0: Router {
            @display("p=383.73126,279.6875");
        }
        router1: Router {
            @display("p=661.1813,194.6625");
        }
        router2: Router {
            @display("p=787.60004,404.98752");
        }
    connections:
        source.ethg++ <--> Eth100M <--> router0.ethg++;
        router0.pppg++ <--> Eth100M <--> router1.pppg++;
        router1.pppg++ <--> Eth100M <--> router2.pppg++;
        router2.ethg++ <--> Eth100M <--> destination.ethg++;
}

