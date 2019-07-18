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

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
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
network VisuA
{
    parameters:
        @display("bgb=0,0");
    submodules:
        visualizer: IntegratedVisualizer {
            parameters:
                @display("p=100,50");
        }
}

network VisuB extends VisuA
{
    submodules:
        coordinateSystem: OsgGeographicCoordinateSystem {
            parameters:
                @display("p=100,150");
        }
}

network VisuC extends VisuB
{
    parameters:
        @display("bgb=800,800");
}

network VisuD extends VisuC
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

network VisuE extends VisuD
{
    submodules:
        pedestrianVideo: WirelessHost {
            parameters:
                @display("p=351,164;i=device/wifilaptop");
        }
        router0: Router {
            parameters:
                @display("p=487,234;i=abstract/router");
        }
        switch0: EtherSwitch {
            parameters:
                @display("p=416,327");
        }
        videoStreamServer: StandardHost {
            parameters:
                @display("p=487,470");
        }
        server1: StandardHost {
            @display("p=350.544,438.984");
        }
    connections:
        accessPoint0.ethg++ <--> Eth100M <--> router0.ethg++;
        switch0.ethg++ <--> Eth100M <--> router0.ethg++;
        videoStreamServer.ethg++ <--> Eth100M <--> switch0.ethg++;
        server1.ethg++ <--> Eth100M <--> switch0.ethg++;
}

network VisuF extends VisuE
{
    submodules:
        accessPoint1: AccessPoint {
            parameters:
                @display("p=601,163");
        }
        router1: Router {
            parameters:
                @display("p=601,234;i=abstract/router");
        }
    connections:
        router0.pppg++ <--> Eth100M <--> router1.pppg++;
        accessPoint1.ethg++ <--> Eth100M <--> router1.ethg++;
}

network VisuG extends VisuF
{
    submodules:
        physicalEnvironment: PhysicalEnvironment {
            parameters:
                @display("p=100,450");
        }
}

network VisuH extends VisuG
{
    submodules:
        car: WirelessHost {
            @display("p=215,122");
        }
        accessPoint2: AccessPoint {
            @display("p=215,233");
        }
        router2: Router {
            @display("p=316,234");
        }
        accessPoint3: AccessPoint {
            @display("p=215,298");
        }
        tcpHost[2]: StandardHost {
            @display("p=650,431");
        }
        switch1: EtherSwitch {
            @display("p=660,250");
        }
        router3: Router {
            @display("p=316,299");
        }
    connections:
        router0.pppg++ <--> Eth100M <--> router2.pppg++;
        router2.ethg++ <--> Eth100M <--> accessPoint2.ethg++;
        switch0.ethg++ <--> Eth100M <--> tcpHost[0].ethg++;
        switch1.ethg++ <--> Eth100M <--> tcpHost[1].ethg++;
        router1.ethg++ <--> Eth100M <--> switch1.ethg++;
        router0.pppg++ <--> Eth100M <--> router3.pppg++;
        accessPoint3.ethg++ <--> Eth100M <--> router3.ethg++;
}
