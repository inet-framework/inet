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

//networkA
import inet.visualizer.integrated.IntegratedVisualizer;
import inet.common.geometry.common.OsgGeographicCoordinateSystem;

//networkB
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;

import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;

//networkC
import inet.node.ethernet.EtherSwitch;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;

import inet.node.ethernet.Eth100M;
import inet.node.ethernet.Eth1G;

//networkE
import inet.environment.common.PhysicalEnvironment;

//
// TODO documentation
//
network VisualizationA
{
    parameters:
        @display("bgb=0,0");
    submodules:
        visualizer: IntegratedVisualizer {
            parameters:
                @display("p=100,50");
        }
        coordinateSystem: OsgGeographicCoordinateSystem {
            parameters:
                @display("p=100,150");
        }
}


network VisualizationB extends VisualizationA
{
    parameters:
        @display("bgb=0,0");
    submodules:
        pedestrian0: WirelessHost {
            parameters:
                @display("p=395,53;i=device/cellphone");
        }
        pedestrian1: WirelessHost {
            parameters:
                @display("p=600,85;i=device/cellphone");
        }
        accessPoint0: AccessPoint {
            parameters:
                @display("p=487,163");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                @display("p=100,250");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,350");
        }
}


network VisualizationC extends VisualizationB
{
    parameters:
        @display("bgb=0,0");
    submodules:
        pedestrianVideo: WirelessHost {
            parameters:
                @display("p=366,150");
        }
        router0: Router {
            parameters:
                @display("p=511,234;i=abstract/router");
        }
        switch0: EtherSwitch {
            parameters:
                @display("p=416,327");
        }
        server0: StandardHost {
            parameters:
                @display("p=443,405");
        }
    connections:
        accessPoint0.ethg++ <--> Eth100M <--> router0.ethg++;
        switch0.ethg++ <--> Eth100M <--> router0.ethg++;
        server0.ethg++ <--> Eth100M <--> switch0.ethg++;
}


network VisualizationD extends VisualizationC
{
    parameters:
        @display("bgb=0,0");
    submodules:
        accessPoint1: AccessPoint {
            parameters:
                @display("p=630,205");
        }
        router1: Router {
            parameters:
                @display("p=616,244;i=abstract/router");
        }
    connections:
        router0.pppg++ <--> Eth100M <--> router1.pppg++;
        accessPoint1.ethg++ <--> Eth100M <--> router1.ethg++;
}


network VisualizationE extends VisualizationD
{
    submodules:
        physicalEnvironment: PhysicalEnvironment {
            parameters:
                @display("p=100,450");
        }
}
