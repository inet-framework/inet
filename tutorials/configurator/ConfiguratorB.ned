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

package inet.tutorials.configurator;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth10M;
import inet.node.ethernet.EtherSwitch;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.visualizer.integrated.IntegratedCanvasVisualizer;


network ConfiguratorB
{
    @display("bgb=3225.915,1993.86");
    submodules:
        area1lan1host0: StandardHost {
            @display("p=492.195,413.82");
        }
        area1lan1host1: StandardHost {
            @display("p=492.195,147.345");
        }
        area1lan1host2: StandardHost {
            @display("p=865.26,147.345");
        }
        area1switch1: EtherSwitch {
            @display("p=865.26,410.685");
        }
        area2lan1host1: StandardHost {
            @display("p=1357.455,1840.245");
        }
        area2lan1host2: StandardHost {
            @display("p=946.77,1840.245");
        }
        area2lan1host3: StandardHost {
            @display("p=558.03,1840.245");
        }
        area2switch1: EtherSwitch {
            @display("p=1119.1951,1514.205");
        }
        area3lan1host1: StandardHost {
            @display("p=2868.5251,1583.175");
        }
        area3lan1host2: StandardHost {
            @display("p=2868.5251,1322.97");
        }
        area3lan1host3: StandardHost {
            @display("p=2868.5251,1078.4401");
        }
        area3switch1: EtherSwitch {
            @display("p=2385.735,1075.305");
        }
        area1lan2host1: StandardHost {
            @display("p=410.685,1097.25");
        }
        area1lan2host2: StandardHost {
            @display("p=410.685,1322.97");
        }
        area1lan2host3: StandardHost {
            @display("p=410.685,1583.175");
        }
        area1switch2: EtherSwitch {
            @display("p=865.26,1094.115");
        }
        area2lan2host1: StandardHost {
            @display("p=2555.025,1840.245");
        }
        area2lan2host2: StandardHost {
            @display("p=2153.745,1840.245");
        }
        area2lan2host3: StandardHost {
            @display("p=1743.06,1840.245");
        }
        area2switch2: EtherSwitch {
            @display("p=2094.18,1511.0701");
        }
        area3lan2host1: StandardHost {
            @display("p=2551.8901,137.94");
        }
        area3lan2host2: StandardHost {
            @display("p=2997.06,137.94");
        }
        area3lan2host3: StandardHost {
            @display("p=2997.06,354.255");
        }
        area3switch2: EtherSwitch {
            @display("p=2385.735,410.685");
        }
        area1router: Router {
            @display("p=1232.055,755.53503");
        }
        area2router: Router {
            @display("p=1620.795,1322.97");
        }
        area3router: Router {
            @display("p=2012.67,755.53503");
        }
        backbonerouter: Router {
            @display("p=1620.795,934.23;i=abstract/router2");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=150,100");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=150,250");
        }
    connections:
        area2lan1host1.ethg++ <--> Eth10M <--> area2switch1.ethg++;
        area2lan1host2.ethg++ <--> Eth10M <--> area2switch1.ethg++;
        area2lan1host3.ethg++ <--> Eth10M <--> area2switch1.ethg++;
        area1switch2.ethg++ <--> Eth10M <--> area1lan2host1.ethg++;
        area1lan2host2.ethg++ <--> Eth10M <--> area1switch2.ethg++;
        area1switch2.ethg++ <--> Eth10M <--> area1lan2host3.ethg++;
        area1switch1.ethg++ <--> Eth10M <--> area1lan1host0.ethg++;
        area1switch1.ethg++ <--> Eth10M <--> area1lan1host1.ethg++;
        area1switch1.ethg++ <--> Eth10M <--> area1lan1host2.ethg++;
        area1switch1.ethg++ <--> Eth10M <--> area1router.ethg++;
        area1switch2.ethg++ <--> Eth10M <--> area1router.ethg++;
        area2switch1.ethg++ <--> Eth10M <--> area2router.ethg++;
        area2router.ethg++ <--> Eth10M <--> area2switch2.ethg++;
        area2switch2.ethg++ <--> Eth10M <--> area2lan2host1.ethg++;
        area2switch2.ethg++ <--> Eth10M <--> area2lan2host2.ethg++;
        area2switch2.ethg++ <--> Eth10M <--> area2lan2host3.ethg++;
        area3switch1.ethg++ <--> Eth10M <--> area3lan1host1.ethg++;
        area3switch1.ethg++ <--> Eth10M <--> area3lan1host2.ethg++;
        area3switch1.ethg++ <--> Eth10M <--> area3lan1host3.ethg++;
        area3switch1.ethg++ <--> Eth10M <--> area3router.ethg++;
        area3router.ethg++ <--> Eth10M <--> area3switch2.ethg++;
        area3switch2.ethg++ <--> Eth10M <--> area3lan2host1.ethg++;
        area3switch2.ethg++ <--> Eth10M <--> area3lan2host2.ethg++;
        area3switch2.ethg++ <--> Eth10M <--> area3lan2host3.ethg++;
        area2router.ethg++ <--> Eth10M <--> backbonerouter.ethg++;
        area1router.ethg++ <--> Eth10M <--> backbonerouter.ethg++;
        backbonerouter.ethg++ <--> Eth10M <--> area3router.ethg++;
}
