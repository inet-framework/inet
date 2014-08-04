//
// Copyright (C) 2014 OpenSim Ltd.
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
package inet.examples.objectcache;

import inet.environment.PhysicalEnvironment;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.physicallayer.ieee80211.Ieee80211RadioMedium;


network ObjectCacheTest
{
    parameters:
        int numHosts;
        @display("bgb=2000,2000");
    submodules:
        environment: PhysicalEnvironment {
            parameters:
                @display("p=50,50");
        }
        radioMedium: Ieee80211RadioMedium {
            parameters:
                @display("p=50,150");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                config = xml("<config><interface hosts='*' address='192.168.x.x' netmask='255.255.0.0'/></config>");
                @display("p=50,250");
        }
        host[numHosts]: AdhocHost {
            parameters:
                @display("r=,,#707070;p=250,58");
        }
}
