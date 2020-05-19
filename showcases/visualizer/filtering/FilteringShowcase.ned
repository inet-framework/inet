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

package inet.showcases.visualizer.filtering;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.inet.AdhocHost;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedVisualizer;


network FilteringShowcase
{
    @display("bgb=1357.62,798.49005");
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=57,43");
        }
        visualizer: IntegratedVisualizer {
            @display("p=57,108");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=57,175");
        }
        router1: Router {
            @display("p=510,560");
        }
        router2: Router {
            @display("p=868,560");
        }
        router3: Router {
            @display("p=704,350");
        }
        AP2: AccessPoint {
            @display("p=1074,558");
        }
        AP3: AccessPoint {
            @display("p=826,192");
        }
        AP1: AccessPoint {
            @display("p=286,558");
        }
        host1: StandardHost {
            @display("p=150,484");
        }
        host2: StandardHost {
            @display("p=112,628");
        }
        host6: StandardHost {
            @display("p=1226,492");
        }
        host9: WirelessHost {
            @display("p=668,134");
        }
        host7: WirelessHost {
            @display("p=996,272");
        }
        host8: WirelessHost {
            @display("p=978,110");
        }
        host5: WirelessHost {
            @display("p=1210,666");
        }
        host4: WirelessHost {
            @display("p=1002,658");
        }
        host3: WirelessHost {
            @display("p=380,666");
        }
    connections:
        router2.ethg++ <--> Eth100M <--> AP2.ethg++;
        router1.ethg++ <--> Eth100M <--> router2.ethg++;
        AP1.ethg++ <--> Eth100M <--> router1.ethg++;
        router1.ethg++ <--> Eth100M <--> router3.ethg++;
        router3.ethg++ <--> Eth100M <--> router2.ethg++;
        router3.ethg++ <--> Eth100M <--> AP3.ethg++;
        AP2.ethg++ <--> Eth100M <--> host6.ethg++;
        host2.ethg++ <--> Eth100M <--> AP1.ethg++;
        host1.ethg++ <--> Eth100M <--> AP1.ethg++;
}
