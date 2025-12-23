//
// Copyright (C) 2006 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

package inet.tutorials.visualization;

import inet.common.geometry.common.OsgGeographicCoordinateSystem;
import inet.environment.common.PhysicalEnvironment;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth1G;
import inet.node.inet.AdhocHost;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedVisualizer;

network VisualizationTutorialA
{
    parameters:
        @display("bgb=0,0");
    submodules:
        physicalEnvironment: PhysicalEnvironment {
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
        server[2]: StandardHost {
            parameters:
                @display("p=300,100");
        }
        router[3]: Router {
            parameters:
                @display("p=300,200");
        }
        accessPoint[3]: AccessPoint {
            parameters:
                @display("p=300,300");
        }
        car[2]: WirelessHost {
            parameters:
                @display("p=300,400");
        }
        pedestrian[2]: WirelessHost {
            parameters:
                @display("p=300,500;i=device/cellphone");
        }
    connections:
        accessPoint[0].ethg++ <--> Eth1G <--> router[0].ethg++;
        accessPoint[1].ethg++ <--> Eth1G <--> router[1].ethg++;
        accessPoint[2].ethg++ <--> Eth1G <--> router[2].ethg++;
        router[0].ethg++ <--> Eth1G <--> server[0].ethg++;
        router[1].ethg++ <--> Eth1G <--> server[1].ethg++;
        router[0].pppg++ <--> Eth1G <--> router[1].pppg++;
        router[1].pppg++ <--> Eth1G <--> router[2].pppg++;
}

network VisualizationTutorialB extends VisualizationTutorialA
{
    parameters:
        @display("bgb=0,0");
    submodules:
        visualizer: IntegratedVisualizer {
            parameters:
                @display("p=100,350");
        }
}

network VisualizationTutorialC extends VisualizationTutorialB
{
    parameters:
        @display("bgb=0,0");
    submodules:
        coordinateSystem: OsgGeographicCoordinateSystem {
            parameters:
                @display("p=100,450");
        }
}
