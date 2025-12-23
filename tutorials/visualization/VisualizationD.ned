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

package inet.tutorials.visualization;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;


network VisualizationD extends VisualizationC
{
    submodules:
        pedestrian0: WirelessHost {
            parameters:
                @display("p=398,89;i=device/cellphone");
        }
        pedestrian1: WirelessHost {
            parameters:
                @display("p=548,89;i=device/cellphone");
        }
        accessPoint0: AccessPoint {
            parameters:
                @display("p=487,163");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,250");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,350");
        }
}
