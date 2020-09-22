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

// Created: 2012-6-10 for project inet

package inet.examples.voip;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.StandardHost;
import inet.node.internetcloud.InternetCloud;
import ned.DatarateChannel;


//
// A generated network with star topology.
//
network Voip
{
    types:
        channel C extends DatarateChannel
        {
            datarate = 100Mbps;
            delay = 0.1us;
        }
    submodules:
        senderHost: StandardHost {
            parameters:
                @display("p=250,100");
        }
        receiverHost: StandardHost {
            parameters:
                @display("p=650,100");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,100;is=s");
        }
        internetCloud: InternetCloud {
            @display("p=450,100;is=vl");
        }
    connections:
        senderHost.pppg++ <--> C <--> internetCloud.pppg++;
        internetCloud.pppg++ <--> C <--> receiverHost.pppg++;
}
