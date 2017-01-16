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
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EtherSwitch;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedVisualizer;


network Simple
{
    @display("bgb=822,636");
    submodules:
        configurator: IPv4NetworkConfigurator {
            @display("p=746.24,53.76");
        }
        visualizer: IntegratedVisualizer {
            @display("p=746.24,129.28");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=744.96,204.8");
        }
        host0: StandardHost {
            @display("p=71.68,157.44");
        }
        host1: StandardHost {
            @display("p=66.56,259.84");
        }
        host2: StandardHost {
            @display("p=208.64,62.72");
        }
        host3: StandardHost {
            @display("p=354.56,53.76");
        }
        host4: WirelessHost {
            @display("p=71.68,448");
        }
        host5: WirelessHost {
            @display("p=66.56,559.36");
        }
        switch0: EtherSwitch {
            @display("p=163.84,232.96");
        }
        switch1: EtherSwitch {
            @display("p=281.6,140.8");
        }
        router0: Router {
            @display("p=289.28,231.68");
        }
        router1: Router {
            @display("p=271.36,500.48");
        }
        router2: Router {
            @display("p=375.04,368.64");
        }
        switch2: EtherSwitch {
            @display("p=497.92,458.24");
        }
        host6: StandardHost {
            @display("p=647.68,401.92");
        }
        host7: StandardHost {
            @display("p=643.84,528.64");
        }
        host8: StandardHost {
            @display("p=439.04,559.36");
        }
        accessPoint: AccessPoint {
            @display("p=174.08,499.2");
        }
    connections:
        router0.ethg++ <--> Eth100M <--> switch1.ethg++;
        host3.ethg++ <--> Eth100M <--> switch1.ethg++;
        host2.ethg++ <--> Eth100M <--> switch1.ethg++;
        switch0.ethg++ <--> Eth100M <--> host0.ethg++;
        switch0.ethg++ <--> Eth100M <--> host1.ethg++;
        router0.ethg++ <--> Eth100M <--> switch0.ethg++;
        router0.ethg++ <--> Eth100M <--> router2.ethg++;
        router2.ethg++ <--> Eth100M <--> router1.ethg++;
        router2.ethg++ <--> Eth100M <--> switch2.ethg++;
        switch2.ethg++ <--> Eth100M <--> host6.ethg++;
        switch2.ethg++ <--> Eth100M <--> host7.ethg++;
        switch2.ethg++ <--> Eth100M <--> host8.ethg++;
        accessPoint.ethg++ <--> Eth100M <--> router1.ethg++;
}
