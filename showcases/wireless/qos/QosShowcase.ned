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

package inet.showcases.wireless.qos;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedVisualizer;


network QosShowcase
{
    parameters:
        @display("b=297,203;bgb=511,302");
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=62,143");
        }
        ap: AccessPoint {
            parameters:
                @display("p=300,200;r=,,#707070");
        }
        server: WirelessHost {
            parameters:
                @display("p=400,100;r=,,#707070");
        }
        client: WirelessHost {
            parameters:
                @display("p=200,100;r=,,#707070");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=61,46");
        }
        visualizer: IntegratedVisualizer {
            @display("p=61,236");
        }
}

