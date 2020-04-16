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

package inet.showcases.visualizer.styling;
import inet.node.ethernet.Eth100M;
import inet.node.inet.WirelessHost;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth10M;
import inet.node.inet.AdhocHost;
import inet.node.inet.StandardHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211RadioMedium;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedVisualizer;


network StylingShowcase
{
    @display("bgb=600,250");
    submodules:
        host1: AdhocHost {
            @display("p=205,150");
        }
        host2: AdhocHost {
            @display("p=465,150");
        }
        visualizer: IntegratedVisualizer {
            @display("p=50,50");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=50,100");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=50,150");
        }
}

network AnnotationShowcase
{
    @display("bgb=600,250");
    submodules:
        host1: WirelessHost {
            @display("p=205,150");
        }
        host2: WirelessHost {
            @display("p=465,150");
        }
        visualizer: IntegratedVisualizer {
            @display("p=50,50");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=50,100");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=50,150");
        }
        accessPoint: AccessPoint {
            @display("p=329,67");
        }
}

