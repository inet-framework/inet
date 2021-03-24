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

package inet.examples.ethernet.redundancy;

import inet.networks.WiredNetworkBase;
import inet.node.ethernet.EthernetLink;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.StandardHost;

network EthernetRedundancyExample extends WiredNetworkBase
{
    types:
        channel C extends EthernetLink
        {
            per = 0.2;
        }
    submodules:
        source: StandardHost {
            @display("p=300,200");
        }
        s1: EthernetSwitch {
            @display("p=500,200");
        }
        s2a: EthernetSwitch {
            @display("p=700,100");
        }
        s3a: EthernetSwitch {
            @display("p=900,100");
        }
        s2b: EthernetSwitch {
            @display("p=700,300");
        }
        s3b: EthernetSwitch {
            @display("p=900,300");
        }
        destination: StandardHost {
            @display("p=1100,200");
        }
    connections:
        s3a.ethg++ <--> C <--> destination.ethg++;
        s3b.ethg++ <--> C <--> destination.ethg++;
        s2a.ethg++ <--> C <--> s3a.ethg++;
        s2b.ethg++ <--> C <--> s3b.ethg++;
        s2a.ethg++ <--> C <--> s2b.ethg++;
        s1.ethg++ <--> C <--> s2a.ethg++;
        s1.ethg++ <--> C <--> s2b.ethg++;
        source.ethg++ <--> C <--> s1.ethg++;
}

