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
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

package inet.showcases.ethernet.vlan;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.StandardHost;
import inet.visualizer.canvas.integrated.IntegratedCanvasVisualizer;


network EthernetVlanShowcase
{
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,100;is=s");
        }
        host1: StandardHost {
            @display("p=250,150");
        }
        switch1: EthernetSwitch {
            @display("p=450,150");
        }
        switch2: EthernetSwitch {
            @display("p=650,150");
        }
        host2: StandardHost {
            @display("p=850,150");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=100,200;is=s");
        }
    connections:
        host1.ethg++ <--> Eth100M <--> switch1.ethg++;
        host2.ethg++ <--> Eth100M <--> switch2.ethg++;
        switch1.ethg++ <--> Eth100M <--> switch2.ethg++;
}

network EthernetVlanShowcase2
{
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,100;is=s");
        }
        host1: StandardHost {
            @display("p=251,69;i=,#CC0000");
        }
        switch1: EthernetSwitch {
            @display("p=450,150");
        }
        switch2: EthernetSwitch {
            @display("p=650,150");
        }
        host3: StandardHost {
            @display("p=850,57;i=,#CC0000");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=100,200;is=s");
        }
        host6: StandardHost {
            @display("p=850,215;i=,#204A87");
        }
        host4: StandardHost {
            @display("p=251,208;i=,#204A87");
        }
        host2: StandardHost {
            @display("p=744,57;i=,#CC0000");
        }
        host5: StandardHost {
            @display("p=744,215;i=,#204A87");
        }
    connections:

        switch1.ethg++ <--> Eth100M <--> host1.ethg++;
        switch1.ethg++ <--> Eth100M <--> host4.ethg++;
        switch1.ethg++ <--> Eth100M <--> switch2.ethg++;
        switch2.ethg++ <--> Eth100M <--> host2.ethg++;
        switch2.ethg++ <--> Eth100M <--> host3.ethg++;
        switch2.ethg++ <--> Eth100M <--> host5.ethg++;
        switch2.ethg++ <--> Eth100M <--> host6.ethg++;
}

