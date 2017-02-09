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
import inet.common.lifecycle.LifecycleController;
import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EtherSwitch;
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
    @display("bgb=1100,800");
    submodules:
        visualizer: IntegratedVisualizer {
            parameters:
                @display("p=100,50");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                @display("p=100,150");
        }
        standardHost0: StandardHost {
            @display("p=250,50");
        }
        standardHost4: StandardHost {
            @display("p=980.15497,557.22");
        }
        router0: Router {
            @display("p=451.8,144.325");
        }
        router4: Router {
            @display("p=535.885,292.41498");
        }
        router3: Router {
            @display("p=535.885,445.525");
        }
        router1: Router {
            @display("p=749.235,230.92");
        }
        router2: Router {
            @display("p=249.745,346.38");
        }
        standardHost2: StandardHost {
            @display("p=90.36,534.63");
        }
        standardHost1: StandardHost {
            @display("p=947.52496,110.439995");
        }
        standardHost3: StandardHost {
            @display("p=749.235,611.185");
        }
        etherSwitch0: EtherSwitch {
            @display("p=230.92,143.06999");
        }
        etherSwitch1: EtherSwitch {
            @display("p=947.52496,229.665");
        }
        etherSwitch2: EtherSwitch {
            @display("p=279.865,515.805");
        }
        etherSwitch3: EtherSwitch {
            @display("p=731.665,448.03497");
        }
    connections:
        router0.pppg++ <--> Eth100M <--> router4.pppg++;
        router4.pppg++ <--> Eth100M <--> router3.pppg++;
        standardHost0.ethg++ <--> Eth100M <--> etherSwitch0.ethg++;
        etherSwitch0.ethg++ <--> Eth100M <--> router0.ethg++;
        router3.ethg++ <--> Eth100M <--> etherSwitch3.ethg++;
        etherSwitch3.ethg++ <--> Eth100M <--> standardHost3.ethg++;
        etherSwitch3.ethg++ <--> Eth100M <--> standardHost4.ethg++;
        router4.pppg++ <--> Eth100M <--> router1.pppg++;
        etherSwitch1.ethg++ <--> Eth100M <--> router1.ethg++;
        etherSwitch1.ethg++ <--> Eth100M <--> standardHost1.ethg++;
        router2.pppg++ <--> Eth100M <--> router0.pppg++;
        router2.ethg++ <--> Eth100M <--> etherSwitch2.ethg++;
        standardHost2.ethg++ <--> Eth100M <--> etherSwitch2.ethg++;
}

//
// TODO documentation
//
network NetworkRouteComplex
{
    @display("bgb=1100,800");
    submodules:
        visualizer: IntegratedVisualizer {
            parameters:
                @display("p=100,50");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                @display("p=100,150");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,250");
        }
        scenarioManager: ScenarioManager {
            parameters:
                @display("p=100,350");
        }
        lifecycleController: LifecycleController {
            @display("p=100,450");
        }
        wiredSource: StandardHost {
            @display("p=240.672,389.784");
        }
        wiredDestination: StandardHost {
            @display("p=941.76,290.376");
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
            @display("p=383.244,143.87999");
        }
        accessPoint1: AccessPoint {
            @display("p=787.41595,514.044");
        }
        wirelessSource: WirelessHost {
            @display("p=383.244,49.704");
        }
        wirelessDestination: WirelessHost {
            @display("p=787.41595,604.29596");
        }

    connections:
        wiredSource.ethg++ <--> Eth100M <--> router0.ethg++;
        router0.pppg++ <--> Eth100M <--> router1.pppg++;
        router1.pppg++ <--> Eth100M <--> router2.pppg++;
        router2.ethg++ <--> Eth100M <--> wiredDestination.ethg++;
        router0.pppg++ <--> Eth100M <--> router3.pppg++;
        router3.pppg++ <--> Eth100M <--> router2.pppg++;
        router0.ethg++ <--> Eth100M <--> accessPoint0.ethg++;
        router2.ethg++ <--> Eth100M <--> accessPoint1.ethg++;
}

