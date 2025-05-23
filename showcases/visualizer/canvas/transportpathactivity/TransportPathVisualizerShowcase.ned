//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.showcases.visualizer.canvas.transportpathactivity;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.AdhocHost;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211RadioMedium;
import inet.visualizer.canvas.integrated.IntegratedCanvasVisualizer;
import inet.visualizer.canvas.transportlayer.TransportRouteCanvasVisualizer;


network TransportPathVisualizerSimpleWiredShowcase
{
    @display("bgb=500,250");
    submodules:
        pathVisualizer: TransportRouteCanvasVisualizer {
            @display("p=80,80");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=80,150");
        }
        source: StandardHost {
            @display("p=200,110");
        }
        destination: StandardHost {
            @display("p=400,110");
        }
    connections:
        source.ethg++ <--> Eth100M <--> destination.ethg++;
}


network TransportPathVisualizerSimpleWirelessShowcase
{
    @display("bgb=500,250");
    submodules:
        pathVisualizer: TransportRouteCanvasVisualizer {
            @display("p=80,50");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=80,120");
        }
        radioMedium: Ieee80211RadioMedium {
            @display("p=80,190");
        }
        source: AdhocHost {
            @display("p=200,120");
        }
        destination: AdhocHost {
            @display("p=400,120");
        }
}

network TransportPathVisualizerFilteringShowcase
{
    @display("bgb=1024,700");
    submodules:
        visualizer: IntegratedCanvasVisualizer {
            parameters:
                @display("p=80,50");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=80,140");
        }
        source1: StandardHost {
            @display("p=205.824,75.576004");
        }
        source2: StandardHost {
            @display("p=947.112,62.712");
        }
        destination1: StandardHost {
            @display("p=895.656,466.32");
        }
        destination2: StandardHost {
            @display("p=143.112,556.368");
        }
        host1: StandardHost {
            @display("p=356.976,75.576004");
        }
        host2: StandardHost {
            @display("p=947.112,205.824");
        }
        videoStreamServer: StandardHost {
            @display("p=868.32,598.176");
        }
        videoStreamClient: StandardHost {
            @display("p=78.792,427.728");
        }
        switch0: EthernetSwitch {
            @display("p=268.536,173.664");
        }
        switch1: EthernetSwitch {
            @display("p=805.60803,147.936");
        }
        switch2: EthernetSwitch {
            @display("p=255.672,426.12");
        }
        switch3: EthernetSwitch {
            @display("p=668.928,545.112");
        }
        router0: Router {
            @display("p=593.352,149.544");
        }
        router1: Router {
            @display("p=805.60803,307.128");
        }
        router2: Router {
            @display("p=371.448,308.736");
        }
        router3: Router {
            @display("p=405.216,508.128");
        }
        router4: Router {
            @display("p=593.352,389.136");
        }
    connections:
        router0.pppg++ <--> Eth100M <--> router4.pppg++;
        source1.ethg++ <--> Eth100M <--> switch0.ethg++;
        switch0.ethg++ <--> Eth100M <--> router0.ethg++;
        router3.ethg++ <--> Eth100M <--> switch3.ethg++;
        switch3.ethg++ <--> Eth100M <--> destination1.ethg++;
        router4.pppg++ <--> Eth100M <--> router1.pppg++;
        switch1.ethg++ <--> Eth100M <--> router1.ethg++;
        switch1.ethg++ <--> Eth100M <--> source2.ethg++;
        router2.ethg++ <--> Eth100M <--> switch2.ethg++;
        destination2.ethg++ <--> Eth100M <--> switch2.ethg++;
        router4.pppg++ <--> Eth100M <--> router2.pppg++;
        router4.pppg++ <--> Eth100M <--> router3.pppg++;
        router0.pppg++ <--> Eth100M <--> router2.pppg++;
        router1.pppg++ <--> Eth100M <--> router0.pppg++;
        switch0.ethg++ <--> Eth100M <--> host1.ethg++;
        switch1.ethg++ <--> Eth100M <--> host2.ethg++;
        switch3.ethg++ <--> Eth100M <--> videoStreamServer.ethg++;
        switch2.ethg++ <--> Eth100M <--> videoStreamClient.ethg++;
        router2.pppg++ <--> Eth100M <--> router3.pppg++;
}
