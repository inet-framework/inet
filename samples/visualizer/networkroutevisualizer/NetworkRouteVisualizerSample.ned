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
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
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

//
// TODO documentation
//
network NetworkRouteComplex
{
    @display("bgb=1000,650");
    submodules:
        routeVisualizer1: NetworkRouteVisualizer {
            parameters:
                @display("p=100,50");
        }
        routeVisualizer2: NetworkRouteVisualizer {
            parameters:
                @display("p=100,150");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                @display("p=100,250");
        }
        visualizer: IntegratedVisualizer {
            parameters:
                @display("p=100,350");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,450");
        }
        source1: StandardHost {
            @display("p=250,50");
        }
        destination1: StandardHost {
            @display("p=930,550");
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
        router3: Router {
            @display("p=512.04,481.91998");
        }
        accessPoint0: AccessPoint {
            @display("p=730,50");
        }
        accessPoint1: AccessPoint {
            @display("p=286.13998,480.66498");
        }
        source2: WirelessHost {
            @display("p=919.915,114.204994");
        }
        destination2: WirelessHost {
            @display("p=163.15,549.69");
        }
    connections:
        source1.ethg++ <--> Eth100M <--> router0.ethg++;
        router0.pppg++ <--> Eth100M <--> router1.pppg++;
        router1.pppg++ <--> Eth100M <--> router2.pppg++;
        router2.ethg++ <--> Eth100M <--> destination1.ethg++;
        router0.pppg++ <--> Eth100M <--> router3.pppg++;
        router3.pppg++ <--> Eth100M <--> router2.pppg++;
        router1.ethg++ <--> Eth100M <--> accessPoint0.ethg++;
        router3.ethg++ <--> Eth100M <--> accessPoint1.ethg++;
}

