//
// Copyright (C) 2005 Andras Varga
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

package inet.showcases.wireless.qos;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
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

