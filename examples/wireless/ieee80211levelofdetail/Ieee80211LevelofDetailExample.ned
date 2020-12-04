//
// Copyright (C) 2014 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.examples.wireless.ieee80211levelofdetail;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.physicallayer.wireless.common.contract.packetlevel.IRadioMedium;
import inet.visualizer.canvas.integrated.IntegratedCanvasVisualizer;

network Ieee80211LevelofDetailShowcase
{
    submodules:
        visualizer: IntegratedCanvasVisualizer {
            parameters:
                @display("p=100,250");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,150");
        }
        radioMedium: <> like IRadioMedium {
            parameters:
                @display("p=100,50");
        }
        sourceHost: AdhocHost {
            parameters:
                @display("p=200,100");
        }
        destinationHost: AdhocHost {
            parameters:
                @display("p=400,100");
        }
}
