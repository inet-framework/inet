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

package inet.showcases.visualizer.advanced;

import inet.visualizer.integrated.IntegratedMultiCanvasVisualizer;
import inet.node.inet.AdhocHost;
import inet.node.inet.WirelessHost;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.Eth10G;
import inet.node.ethernet.EtherSwitch;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedCanvasVisualizer;
import inet.visualizer.integrated.IntegratedVisualizer;
import inet.visualizer.networklayer.RoutingTableCanvasVisualizer;
import inet.visualizer.networklayer.RoutingTableOsgVisualizer;


network UsingMultipleVisualizerModulesShowcaseA
{
    @display("bgb=846,526");
    submodules:
        router1: Router {
            @display("p=319.8125,375.0625");
        }
        router3: Router {
            @display("p=427.125,241.1875");
        }
        router2: Router {
            @display("p=530.1875,375.0625");
        }
        switch2: EtherSwitch {
            @display("p=643.875,374");
        }
        switch3: EtherSwitch {
            @display("p=427.125,167.875");
        }
        switch1: EtherSwitch {
            @display("p=210.375,374");
        }
        host4: StandardHost {
            @display("p=745.875,308.125");
        }
        host5: StandardHost {
            @display("p=745.875,434.5625");
            @statistic[rcvdBytes](title="bytes received"; source="sum(packetBytes(app[0].packetReceived))"; record=last; checkSignals=false);
        }
        host1: StandardHost {
            @display("p=365.5,78.625");
        }
        host2: StandardHost {
            @display("p=522.75,78.625");
        }
        host3: StandardHost {
            @display("p=574.8125,168.9375");
            @statistic[rcvdBytes](title="bytes received"; source="sum(packetBytes(app[0].packetReceived))"; record=last; checkSignals=false);
        }
        server: StandardHost {
            @display("p=110.5,375.0625;i=device/server");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=60,60");
        }
        visualizer: IntegratedMultiCanvasVisualizer {
            @display("p=60,120");
        }
    connections:
        router1.ethg++ <--> Eth10G <--> router3.ethg++;
        router1.ethg++ <--> Eth10G <--> router2.ethg++;
        router3.ethg++ <--> Eth10G <--> router2.ethg++;
        router2.ethg++ <--> Eth100M <--> switch2.ethg++;
        router3.ethg++ <--> Eth100M <--> switch3.ethg++;
        switch1.ethg++ <--> Eth100M <--> router1.ethg++;
        switch3.ethg++ <--> Eth100M <--> host1.ethg++;
        switch3.ethg++ <--> Eth100M <--> host2.ethg++;
        switch3.ethg++ <--> Eth100M <--> host3.ethg++;
        switch2.ethg++ <--> Eth100M <--> host4.ethg++;
        switch2.ethg++ <--> Eth100M <--> host5.ethg++;
        switch1.ethg++ <--> Eth100M <--> server.ethg++;
}

network UsingMultipleVisualizerModulesShowcaseB
{
    @display("bgb=846,526");
    submodules:
        router1: Router {
            @display("p=319.8125,375.0625");
        }
        router3: Router {
            @display("p=427.125,241.1875");
        }
        router2: Router {
            @display("p=530.1875,375.0625");
        }
        AP1: AccessPoint {
            @display("p=643.875,374");
        }
        switch3: EtherSwitch {
            @display("p=427.125,167.875");
        }
        switch1: EtherSwitch {
            @display("p=210.375,374");
        }
        wifiHost1: WirelessHost {
            @display("p=745.875,308.125");
        }
        wifiHost2: WirelessHost {
            @display("p=745.875,434.5625");
        }
        host1: StandardHost {
            @display("p=365.5,78.625");
        }
        host2: StandardHost {
            @display("p=522.75,78.625");
        }
        host3: StandardHost {
            @display("p=574.8125,168.9375");
        }
        server: StandardHost {
            @display("p=110.5,375.0625;i=device/server");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=60,60");
        }
        visualizer: IntegratedMultiCanvasVisualizer {
            @display("p=60,120");
        }
        adhocHost2: AdhocHost {
            @display("p=195.5,257.125");
        }
        adhocHost1: AdhocHost {
            @display("p=287.9375,161.5");
        }
        radioMedium1: Ieee80211ScalarRadioMedium {
            @display("p=58.4375,179.5625");
        }
        radioMedium2: Ieee80211ScalarRadioMedium {
            @display("p=58.4375,239.0625");
        }
    connections:
        router1.ethg++ <--> Eth10G <--> router3.ethg++;
        router1.ethg++ <--> Eth10G <--> router2.ethg++;
        router3.ethg++ <--> Eth10G <--> router2.ethg++;
        router2.ethg++ <--> Eth100M <--> AP1.ethg++;
        router3.ethg++ <--> Eth100M <--> switch3.ethg++;
        switch1.ethg++ <--> Eth100M <--> router1.ethg++;
        switch3.ethg++ <--> Eth100M <--> host1.ethg++;
        switch3.ethg++ <--> Eth100M <--> host2.ethg++;
        switch3.ethg++ <--> Eth100M <--> host3.ethg++;
        switch1.ethg++ <--> Eth100M <--> server.ethg++;
}
