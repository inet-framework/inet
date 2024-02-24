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

package inet.showcases.general.tcp;

import inet.networks.base.WiredNetworkBase;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;

network TcpCongestionShowcase extends WiredNetworkBase
{
    @display("bgb=545,513");
    submodules:
        source: StandardHost {
            @display("p=250,250");
        }
        router: Router {
            @display("p=350,250");
        }
        sink: StandardHost {
            @display("p=450,250");
        }
    connections:
        source.pppg++ <--> {  datarate = 100Mbps; delay = 2ms; } <--> router.pppg++;
        router.pppg++ <--> {  datarate = 10Mbps; delay = 2ms; } <--> sink.pppg++;
}
