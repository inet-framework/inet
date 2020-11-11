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

package inet.showcases.wireless.handover;

import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedCanvasVisualizer;


network HandoverShowcase
{
    parameters:
        @display("bgb=640,420");
    submodules:
        visualizer: IntegratedCanvasVisualizer {
            parameters:
                @display("p=100,200");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,100");
        }
        host: WirelessHost {
            parameters:
                @display("p=50,280;r=,,#707070");
        }
        ap1: AccessPoint {
            parameters:
                @display("p=100,350;r=,,#707070");
        }
        ap2: AccessPoint {
            parameters:
                @display("p=500,350;r=,,#707070");
        }
}

