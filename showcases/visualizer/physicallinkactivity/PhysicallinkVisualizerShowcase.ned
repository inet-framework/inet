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

package inet.showcases.visualizer.physicallinkactivity;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedVisualizer;
import inet.visualizer.physicallayer.PhysicalLinkVisualizer;


network PhysicalLinkVisualizerEnablingShowcase
{
    @display("bgb=600,300");
    submodules:
        linkVisualizer: PhysicalLinkVisualizer {
            parameters:
                @display("p=100,50");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,150");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,250");
        }
        source: AdhocHost {
            @display("p=250,150");
        }
        destination: AdhocHost {
            @display("p=500,150");
        }
}


network PhysicalLinkVisualizerFilteringShowcase
{
    @display("bgb=780,330");
    submodules:
        visualizer: IntegratedVisualizer {
            parameters:
                @display("p=100,50");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,150");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,250");
        }
        source: WirelessHost {
            @display("p=648,92");
        }
        destination: WirelessHost {
            @display("p=268,150");
        }
        host1: WirelessHost {
            @display("p=648,216");
        }
        accessPoint: AccessPoint {
            @display("p=471,149");
        }
}

network PhysicalLinkVisualizerMobileShowcase
{
    @display("bgb=660,630");
    submodules:
        visualizer: IntegratedVisualizer {
            @display("p=100,50");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,150");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=100,250");
        }
        host1: AdhocHost {
            @display("p=509.736,103.976");
        }
        host2: AdhocHost {
            @display("p=365.184,213.024");
        }
        host3: AdhocHost {
            @display("p=498.324,229.508");
        }
        host4: AdhocHost {
            @display("p=280.228,365.184");
        }
        host5: AdhocHost {
            @display("p=365.184,517.344");
        }
        host6: AdhocHost {
            @display("p=523.684,372.792");
        }
        host7: AdhocHost {
            @display("p=267.548,139.48");
        }
}

