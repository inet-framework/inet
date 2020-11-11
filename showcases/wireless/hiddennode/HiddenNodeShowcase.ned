//
// Copyright (C) 2005 OpenSim Ltd.
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
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

package inet.showcases.wireless.hiddennode;

import inet.environment.common.PhysicalEnvironment;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.WirelessHost;
import inet.physicallayer.wireless.unitdisk.UnitDiskRadioMedium;
import inet.visualizer.integrated.IntegratedCanvasVisualizer;


network HiddenNodeShowcase
{
    parameters:
        @display("b=297,203;bgb=736.6,557.784");
        @figure[rcvdPkText](type=indicatorText; pos=420,20; anchor=w; font=,20; textFormat="packets received: %g"; initialValue=0);
        @statistic[packetReceived](source=hostB.udp.packetReceived; record=figure(count); targetFigure=rcvdPkText);
    submodules:

        hostA: WirelessHost {
            @display("p=300,272");
        }
        hostB: WirelessHost {
            @display("p=450,100");
        }
        hostC: WirelessHost {
            @display("p=600,272");
        }
        radioMedium: UnitDiskRadioMedium {
            @display("p=100,100");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,200");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=100,300");
        }
        physicalEnvironment: PhysicalEnvironment {
            @display("p=100,400");
        }
}

