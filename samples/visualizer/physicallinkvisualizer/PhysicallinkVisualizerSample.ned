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

package inet.samples.visualizer.physicallinkvisualizer;

import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.physicallayer.PhysicalLinkVisualizer;


//
// TODO documentation
//
network PhysicallinkVisualizerSimple
{
    @display("bgb=900,300");
    submodules:
        linkVisualizer: PhysicalLinkVisualizer {
            parameters:
                @display("p=100,50");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                @display("p=100,150");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,250");
        }
        wirelessSource: AdhocHost {
            @display("p=316,80");
        }
        wirelessDestination: AdhocHost {
            @display("p=630,80");
        }
}


//
// TODO documentation
//
network PhysicalLinkVisualizerSimpleV2
{
    @display("bgb=780,450");
    submodules:
        linkVisualizer: PhysicalLinkVisualizer {
            parameters:
                @display("p=100,50");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                @display("p=100,150");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,250");
        }
        accessPoint: AccessPoint {
            @display("p=500,200");
        }
        wirelesshost1: WirelessHost {
            @display("p=500,50");
        }
        wirelesshost2: WirelessHost {
            @display("p=680, 350");
        }
        wirelesshost3: WirelessHost {
            @display("p=320,350");
        }
}
