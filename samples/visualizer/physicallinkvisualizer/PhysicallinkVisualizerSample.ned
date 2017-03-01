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
import inet.node.aodv.AODVRouter;
import inet.node.inet.AdhocHost;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedVisualizer;
import inet.visualizer.physicallayer.PhysicalLinkVisualizer;


//
// TODO documentation
//
network PhysicalLinkVisualizerSimple
{
    @display("bgb=600,300");
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
        source: AdhocHost {
            @display("p=250,150");
        }
        destination: AdhocHost {
            @display("p=500,150");
        }
}


//
// TODO documentation
//
network PhysicalLinkVisualizerFiltering
{
    @display("bgb=780,450");
    submodules:
        visualizer: IntegratedVisualizer {
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
        source: WirelessHost {
            @display("p=517,89");
        }
        host1: WirelessHost {
            @display("p=642,178");
        }
        destination: WirelessHost {
            @display("p=320,350");
        }
}

//
// TODO documentation
//
network PhysicalLinkVisualizerDynamic
{
    @display("bgb=660,630");
    submodules:
        visualizer: IntegratedVisualizer {
            @display("p=100,50");
        }
        configurator: IPv4NetworkConfigurator {
            @display("p=100,150");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=100,250");
        }
        destination: AODVRouter {
            @display("p=488.832,512.952");
        }
        source: AODVRouter {
            @display("p=229,46");
        }
        aodvRouter1: AODVRouter {
            @display("p=421.296,295.872");
        }
        aodvRouter2: AODVRouter {
            @display("p=273.36002,440.592");
        }
        aodvRouter3: AODVRouter {
            @display("p=305.52,188.136");
        }
        aodvRouter4: AODVRouter {
            @display("p=562.8,328.032");
        }
        aodvRouter5: AODVRouter {
            @display("p=421.296,77.184");
        }
}

