//
// Copyright (C) 2010 Kyeong Soo (Joseph) Kim
// Copyright (C) 2004 Andras Varga
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


package inet.examples.ngoa.ecr2;

import inet.networklayer.autorouting.FlatNetworkConfigurator;
//import inet.nodes.ethernet.EtherSwitch3;
import inet.nodes.inet.StandardHost3WithEthernetInterface2;
import inet.nodes.inet.StandardServerWithEthernetInterface2;


//
// Defines a test network for UDP video streaming model.
//
// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
// @date   2010-04-05
//
network TestOfUDPVideoStreaming
{
    parameters:
        @display("bgb=519,184");
    submodules:
        configurator: FlatNetworkConfigurator {
            parameters:
                @display("p=52,42");
        }
        host: StandardHost3WithEthernetInterface2 {
            parameters:
                @display("i=device/laptop;p=46,119");
        }
        server: StandardServerWithEthernetInterface2 {
            parameters:
                @display("p=464,119;i=device/server_l");
        }
    connections:
        host.ethg++ <--> { datarate = 100Mbps; delay = 10ms; per = 1e-3; } <--> server.ethg++;
//        host.ethg++ <--> { datarate = 100Mbps; delay = 10ms; per = 0.0; } <--> server.ethg++;
}
