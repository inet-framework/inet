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

package inet.samples.throughput;

import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.inet.WirelessHost;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;


network Throughput
{
    @display("bgb=606,325");
    submodules:
        sourceHost: WirelessHost {
            @display("p=327,160");
        }
        destinationHost: WirelessHost {
            @display("p=328,160");
        }
        configurator: IPv4NetworkConfigurator {
            @display("p=100,100");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=100,200");
        }
}
