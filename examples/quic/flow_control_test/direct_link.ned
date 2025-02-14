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

package inet.examples.quic.flow_control_test;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;

network direct_link
{
    parameters:
        double linkDelay @unit(s) = default(0ms);
        double linkDatarate @unit(bps) = default(100Mbps); //0Mbps - infinite

    types:
        channel etherline extends ned.DatarateChannel
        {
            delay = linkDelay;
            datarate = linkDatarate;
        }
        
    submodules:
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=222,32");
        }     

        client: StandardHost {
            parameters:
                @display("i=device/laptop;p=57,137");
        }


        server: StandardHost {
            parameters:
                @display("p=390,137;i=device/server_l");
        }

    connections:
        client.pppg++ <--> etherline <--> server.pppg++;
}