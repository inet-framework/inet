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

import inet.node.ethernet.Eth100M;
import inet.node.ethernet.Eth10M;
import inet.node.inet.StandardHost;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedCanvasVisualizer;

channel Eth100MLong extends Eth100M
{
    length = 100m;
}
network ConfiguratorC extends ConfiguratorB
{
    submodules:
        area1AP: AccessPoint {
            @display("p=611.64716,928.3376");
        }
        area3AP: AccessPoint {
            @display("p=2552.152,881.76544");
        }
        area1lan3host1: WirelessHost {
            @display("p=211.3514,585.5005");
        }
        area1lan3host2: WirelessHost {
            @display("p=211.3514,754.0104");
        }
        area1lan3host3: WirelessHost {
            @display("p=211.3514,911.0958");
        }
        area3lan3host1: WirelessHost {
            @display("p=2825.3418,551.44696");
        }
        area3lan3host2: WirelessHost {
            @display("p=2936.0706,725.44934");
        }
        area3lan3host3: WirelessHost {
            @display("p=2824.6829,885.3909");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=150,400");
        }
    connections:
        area1router.ethg++ <--> Eth10M <--> area1AP.ethg++;
        area3router.ethg++ <--> Eth10M <--> area3AP.ethg++;
}
