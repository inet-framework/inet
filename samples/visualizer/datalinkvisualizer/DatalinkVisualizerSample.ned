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
import inet.node.ethernet.EtherSwitch;
import inet.node.inet.AdhocHost;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedVisualizer;
import inet.visualizer.linklayer.DataLinkVisualizer;


//
// TODO documentation
//
network DatalinkVisualizerWirelessSimple
{
    @display("bgb=700,350");
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
        wirelessSource: AdhocHost {
            @display("p=300,150");
        }
        wirelessDestination: AdhocHost {
            @display("p=600,150");
        }
}

//
// TODO documentation
//
network DatalinkVisualizerWiredSimple
{
    @display("bgb=700,350");
    submodules:
        linkVisualizer: DataLinkVisualizer {
            parameters:
                @display("p=100,50");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                @display("p=100,150");
        }
        wiredSource: StandardHost {
            @display("p=300,150");
        }
        wiredDestination: StandardHost {
            @display("p=600,150");
        }
    connections:
        wiredSource.ethg++ <--> Eth100M <--> wiredDestination.ethg++;
}


//
// TODO documentation
//
network DatalinkVisualizerComplex
{
    @display("bgb=900,600");
    submodules:
        visualizer: IntegratedVisualizer {
            @display("p=80,50");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                @display("p=80,120");
        }
        etherSwitch1: EtherSwitch {
            @display("p=150,200");
        }
        etherSwitch3: EtherSwitch {
            @display("p=750,200");
        }
        host1: StandardHost {
            @display("p=50,400");
        }
        etherSwitch4: EtherSwitch {
            @display("p=450,150");
        }
        host3: StandardHost {
            @display("p=350,350");
        }
        host5: StandardHost {
            @display("p=750,500");
        }
        host2: StandardHost {
            @display("p=150,500");
        }
        host4: StandardHost {
            @display("p=550,350");
        }
        host6: StandardHost {
            @display("p=850,400");
        }
        etherSwitch2: EtherSwitch {
            @display("p=450,250");
        }
    connections:
        etherSwitch1.ethg++ <--> Eth100M <--> host1.ethg++;
        etherSwitch1.ethg++ <--> Eth100M <--> etherSwitch4.ethg++;
        etherSwitch4.ethg++ <--> Eth100M <--> etherSwitch3.ethg++;
        etherSwitch1.ethg++ <--> Eth100M <--> host2.ethg++;
        etherSwitch3.ethg++ <--> Eth100M <--> host6.ethg++;
        etherSwitch3.ethg++ <--> Eth100M <--> host5.ethg++;
        etherSwitch4.ethg++ <--> Eth100M <--> etherSwitch2.ethg++;
        etherSwitch2.ethg++ <--> Eth100M <--> host3.ethg++;
        etherSwitch2.ethg++ <--> Eth100M <--> host4.ethg++;
}

