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

package inet.samples.visualizer.datalinkvisualizer;

import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.inet.AdhocHost;
import inet.node.inet.StandardHost;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedVisualizer;
import inet.visualizer.linklayer.DataLinkVisualizer;


//
// TODO documentation
//
network DatalinkVisualizerSimple
{
    @display("bgb=900,300");
    submodules:
        wirelessSource: AdhocHost {
            @display("p=316,80");
        }
        wirelessDestination: AdhocHost {
            @display("p=630,80");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                @display("p=100,150");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,250");
        }
        linkVisualizer: DataLinkVisualizer {
            parameters:
                @display("p=100,50");
        }
        wiredSource: StandardHost {
            @display("p=316,200");
        }
        wiredDestination: StandardHost {
            @display("p=630,200");
        }
    connections:
        wiredSource.ethg++ <--> Eth100M <--> wiredDestination.ethg++;
}


//
// TODO documentation
//
network DatalinkVisualizerExtended
{
    @display("bgb=900,300");
    submodules:
        linkVisualizer: DataLinkVisualizer {
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
            @display("p=600,120");
        }
        wirelessSource: WirelessHost {
            @display("p=380,55");
        }
        wirelessDestination: WirelessHost {
            @display("p=820,55");
        }
}


//
// TODO documentation
//
network ComplexDataLinkVisualizerInfrastructure
{
    @display("bgb=900,600");
    submodules:
        linkVisualizer1: DataLinkVisualizer {
            parameters:
                @display("p=100,50");
        }
        linkVisualizer2: DataLinkVisualizer {
            parameters:
                @display("p=100,150");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                @display("p=100,250");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,350");
        }
        accessPoint: AccessPoint {
            @display("p=545,266");
        }
        wirelessSource1: WirelessHost {
            @display("p=305,182");
        }
        wirelessDestination1: WirelessHost {
            @display("p=785,182");
        }
        wirelessSource2: WirelessHost {
            @display("p=305,351");
        }
        wirelessDestination2: WirelessHost {
            @display("p=785,351");
        }
}

