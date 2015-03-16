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

package inet.examples.wireless.dynamic;

import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.HostAutoConfigurator;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;

module DynamicHost extends AdhocHost
{
    parameters:
        networkLayer.configurator.networkConfiguratorModule = "";
    submodules:
        autoConfigurator: HostAutoConfigurator {
            @display("p=171,335");
        }
}

network DynamicRadioNetwork
{
    submodules:
        destinationNode: DynamicHost {
            parameters:
                @display("p=250,50");
        }
        sourceNode[0]: DynamicHost {
            parameters:
                @dynamic(true);
                @display("p=250,150");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,50");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                @display("p=100,150");
        }
        scenarioManager: ScenarioManager {
            parameters:
                @display("p=100,250");
        }
}
