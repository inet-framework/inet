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

package inet.showcases.visualizer.transportpathactivity;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EtherSwitch;
import inet.node.inet.AdhocHost;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedVisualizer;
import inet.visualizer.transportlayer.TransportRouteVisualizer;
import ned.DatarateChannel;


network TransportPathVisualizerSimpleWiredShowcase
{
    @display("bgb=500,250");
    submodules:
        pathVisualizer: TransportRouteVisualizer {
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
        pathVisualizer: TransportRouteVisualizer {
            @display("p=80,50");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=80,120");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
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
    types:
        channel Ppp100M extends DatarateChannel
        {
            double length @unit(m) = default(10m);
            delay = replaceUnit(length / 2e8, "s");
            datarate = 100 Mbps;
        }
    submodules:
        visualizer: IntegratedVisualizer {
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
        switch0: EtherSwitch {
            @display("p=268.536,173.664");
        }
        switch1: EtherSwitch {
            @display("p=805.60803,147.936");
        }
        switch2: EtherSwitch {
            @display("p=255.672,426.12");
        }
        switch3: EtherSwitch {
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
        router0.pppg++ <--> Ppp100M <--> router4.pppg++;
        source1.ethg++ <--> Eth100M <--> switch0.ethg++;
        switch0.ethg++ <--> Eth100M <--> router0.ethg++;
        router3.ethg++ <--> Eth100M <--> switch3.ethg++;
        switch3.ethg++ <--> Eth100M <--> destination1.ethg++;
        router4.pppg++ <--> Ppp100M <--> router1.pppg++;
        switch1.ethg++ <--> Eth100M <--> router1.ethg++;
        switch1.ethg++ <--> Eth100M <--> source2.ethg++;
        router2.ethg++ <--> Eth100M <--> switch2.ethg++;
        destination2.ethg++ <--> Eth100M <--> switch2.ethg++;
        router4.pppg++ <--> Ppp100M <--> router2.pppg++;
        router4.pppg++ <--> Ppp100M <--> router3.pppg++;
        router0.pppg++ <--> Ppp100M <--> router2.pppg++;
        router1.pppg++ <--> Ppp100M <--> router0.pppg++;
        switch0.ethg++ <--> Eth100M <--> host1.ethg++;
        switch1.ethg++ <--> Eth100M <--> host2.ethg++;
        switch3.ethg++ <--> Eth100M <--> videoStreamServer.ethg++;
        switch2.ethg++ <--> Eth100M <--> videoStreamClient.ethg++;
        router2.pppg++ <--> Ppp100M <--> router3.pppg++;
}
