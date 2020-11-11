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

package inet.examples.wireless.multiradio;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.Router;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.contract.IIntegratedVisualizer;
import ned.DatarateChannel;

network MultiRadio
{
    parameters:
        bool useEthernetSwitching = default(true);
        bool useRouting = default(false);
    submodules:
        visualizer: <default("IntegratedCanvasVisualizer")> like IIntegratedVisualizer if hasVisualizer() {
            parameters:
                @display("p=100,300;is=s");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,200;is=s");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,100;is=s");
        }
        host1: WirelessHost {
            parameters:
                @display("p=85,203");
        }
        host2: WirelessHost {
            parameters:
                @display("p=454,203");
        }
        accessPoint1: AccessPoint {
            parameters:
                @display("p=150,99");
        }
        accessPoint2: AccessPoint {
            parameters:
                @display("p=354,99");
        }
        router: Router if useRouting {
            parameters:
                @display("p=255,168");
        }
    connections:
        accessPoint1.ethg++ <--> DatarateChannel {  datarate = 100Mbps; } <--> accessPoint2.ethg++ if useEthernetSwitching;
}
