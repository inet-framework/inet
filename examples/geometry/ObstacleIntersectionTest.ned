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

package inet.examples.geometry;

import inet.environment.common.PhysicalEnvironment;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;

network ObstacleIntersectionTest
{
    parameters:
        int numHosts;
        @display("bgb=2000,1000");
    submodules:
        physicalEnvironment: PhysicalEnvironment {
            parameters:
                @display("is=s;p=100,200");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("is=s;p=100,300");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                config = xml("<config><interface hosts='*' address='192.168.x.x' netmask='255.255.0.0'/></config>");
                @display("is=s;p=100,100");
        }
        host[numHosts]: AdhocHost {
            parameters:
                @display("r=,,#707070");
        }
}
