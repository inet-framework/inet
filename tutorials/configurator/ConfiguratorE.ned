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
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedCanvasVisualizer;


network ConfiguratorE
{
    @display("bgb=1426.212,959.112");
    submodules:
        area1router: Router {
            @display("p=519.75,535.8375");
        }
        area1host1: WirelessHost {
            @display("p=208.008,369.792");
        }
        area1host3: WirelessHost {
            @display("p=404.6625,506.1375");
        }
        area1host2: WirelessHost {
            @display("p=379.422,369.792");
        }
        area3host3: StandardHost {
            @display("p=973,770");
        }
        area3host2: StandardHost {
            @display("p=780,859");
        }
        area3host1: StandardHost {
            @display("p=599,770");
        }
        area3router: Router {
            @display("p=780.8625,535.8375");
        }
        area3Switch: EtherSwitch {
            @display("p=805.6125,665.775");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,50");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=100,150");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=100,250");
        }
        area2router: Router {
            @display("p=657.1125,356.4");
        }
        area2AP: AccessPoint {
            @display("p=763.53754,280.9125");
        }
        area2host1: WirelessHost {
            @display("p=855.144,175.26599");
        }
        area2host2: WirelessHost {
            @display("p=934.11,281.19598");
        }
        area2host3: WirelessHost {
            @display("p=855.144,390.978");
        }
    connections:
        area3host1.ethg++ <--> Eth10M <--> area3Switch.ethg++;
        area3host2.ethg++ <--> Eth10M <--> area3Switch.ethg++;
        area3Switch.ethg++ <--> Eth10M <--> area3host3.ethg++;
        area3Switch.ethg++ <--> Eth10M <--> area3router.ethg++;
        area3router.ethg++ <--> Eth10M <--> area1router.ethg++;
        area1router.ethg++ <--> Eth10M <--> area1host3.ethg++;
        area1router.ethg++ <--> Eth10M <--> area2router.ethg++;
        area2router.ethg++ <--> Eth10M <--> area3router.ethg++;
        area2router.ethg++ <--> Eth10M <--> area2AP.ethg++;
}
