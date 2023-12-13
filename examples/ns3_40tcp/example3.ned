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

package inet.examples.ns3_40tcp;

import inet.networks.base.WiredNetworkBase;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;

network example3 extends WiredNetworkBase
{
    submodules:
        source: StandardHost;
        router: Router;
        sink: StandardHost;
    connections:
        source.pppg++ <--> {  datarate = 100Mbps; delay = 2ms; } <--> router.pppg++;
        router.pppg++ <--> {  datarate = 10Mbps; delay = 2ms; } <--> sink.pppg++;
}
