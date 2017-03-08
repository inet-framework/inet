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
import inet.node.aodv.AODVRouter;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EtherSwitch;
import inet.node.inet.AdhocHost;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedVisualizer;
import inet.visualizer.transportlayer.TransportRouteVisualizer;


//
// TODO documentation
//
network TransportRouteSimpleWired
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


//
// TODO documentation
//
network TransportRouteSimpleWireless
{
    @display("bgb=500,260");
    submodules:
        source: AdhocHost {
            @display("p=200,120");
        }
        destination: AdhocHost {
            @display("p=400,120");
        }
        routeVisualizer: TransportRouteVisualizer {
            @display("p=80,50");
        }
        configurator: IPv4NetworkConfigurator {
            @display("p=80,120");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=80,190");
        }
}

//
// TODO documentation
//
network TransportRouteExtended
{
    @display("bgb=1100,800");
    submodules:
        visualizer: IntegratedVisualizer {
            parameters:
                @display("p=80,50");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                @display("p=80,120");
        }
        source1: StandardHost {
            @display("p=308.736,75.576004");
        }
        router0: Router {
            @display("p=633.552,162.408");
        }
        router4: Router {
            @display("p=633.552,403.608");
        }
        router3: Router {
            @display("p=445.41602,520.992");
        }
        router1: Router {
            @display("p=845.808,319.992");
        }
        router2: Router {
            @display("p=411.648,321.6");
        }
        destination2: StandardHost {
            @display("p=283.008,651.24");
        }
        source2: StandardHost {
            @display("p=987.312,75.576004");
        }
        destination1: StandardHost {
            @display("p=898.872,520.992");
        }
        etherSwitch0: EtherSwitch {
            @display("p=308.736,186.528");
        }
        etherSwitch1: EtherSwitch {
            @display("p=845.808,160.8");
        }
        etherSwitch2: EtherSwitch {
            @display("p=295.872,438.984");
        }
        etherSwitch3: EtherSwitch {
            @display("p=709.128,557.976");
        }
    connections:
        router0.pppg++ <--> Eth100M <--> router4.pppg++;
        source1.ethg++ <--> Eth100M <--> etherSwitch0.ethg++;
        etherSwitch0.ethg++ <--> Eth100M <--> router0.ethg++;
        router3.ethg++ <--> Eth100M <--> etherSwitch3.ethg++;
        etherSwitch3.ethg++ <--> Eth100M <--> destination1.ethg++;
        router4.pppg++ <--> Eth100M <--> router1.pppg++;
        etherSwitch1.ethg++ <--> Eth100M <--> router1.ethg++;
        etherSwitch1.ethg++ <--> Eth100M <--> source2.ethg++;
        router2.ethg++ <--> Eth100M <--> etherSwitch2.ethg++;
        destination2.ethg++ <--> Eth100M <--> etherSwitch2.ethg++;
        router4.pppg++ <--> Eth100M <--> router2.pppg++;
        router4.pppg++ <--> Eth100M <--> router3.pppg++;
        router0.pppg++ <--> Eth100M <--> router2.pppg++;
        router1.pppg++ <--> Eth100M <--> router0.pppg++;
}

//
// TODO documentation
//
network ServerClientsNetwork
{
    @display("bgb=600,400");
    submodules:
        configurator: IPv4NetworkConfigurator {
            parameters:
                @display("p=80,50");
        }
        visualizer: IntegratedVisualizer {
            parameters:
                @display("p=80,120");
        }
        server: StandardHost {
            @display("p=158,228;i=device/server2");
        }
        host1: StandardHost {
            @display("p=545,58");
        }
        host2: StandardHost {
            @display("p=545,128");
        }
        admin: StandardHost {
            @display("p=545,341");
        }
        switch1: EtherSwitch {
            @display("p=414,84");
        }
        switch2: EtherSwitch {
            @display("p=414,341");
        }
        router: Router {
            @display("p=296,228");
        }
    connections:
        switch1.ethg++ <--> Eth100M <--> host1.ethg++;
        switch1.ethg++ <--> Eth100M <--> host2.ethg++;
        switch2.ethg++ <--> Eth100M <--> admin.ethg++;
        router.ethg++ <--> Eth100M <--> switch1.ethg++;
        router.ethg++ <--> Eth100M <--> switch2.ethg++;
        server.ethg++ <--> Eth100M <--> router.ethg++;
}

