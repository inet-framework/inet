//
// Copyright (C) 2008 Irene Ruengeler
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

package inet.examples.sctp.multihomed;
import inet.nodes.inet.StandardHost;
import inet.nodes.inet.Router;
import ned.DatarateChannel;

network multi
{
    parameters:
        double testTimeout @unit(s) = default(0s);
        bool testing = default(false);
    types:
        channel ppp1line extends DatarateChannel
        {
            delay = 10ms;
            datarate = 1 Mbps;
        }
        channel ppp2line extends DatarateChannel
        {
            delay = 0ms;
            datarate = 1 Gbps;
        }
    submodules:
        cli1: StandardHost {
            parameters:
                IPForward = false;
                routingFile = "cli1.mrt";
                @display("p=61,142;i=device/laptop");
            gates:
                pppg[1];
        }
        srv1: StandardHost {
            parameters:
                IPForward = false;
                routingFile = "srv1.mrt";
                @display("p=426,135;i=device/server2");
            gates:
                pppg[1];

        }
        router1: Router {
            parameters:
                routingFile = "router1.mrt";
                @display("p=p=188,77;i=abstract/router");
            gates:
                pppg[2];
        }
        router2: Router {
            parameters:
                routingFile = "router2.mrt";
                @display("p=323,75;i=abstract/router");
            gates:
                pppg[2];
        }
        router3: Router {
            parameters:
                routingFile = "router3.mrt";
                @display("p=188,192;i=abstract/router");
            gates:
                pppg[2];
        }
        router4: Router {
            parameters:
                routingFile = "router4.mrt";
                @display("p=328,188;i=abstract/router");
            gates:
                pppg[2];
        }
    connections:
        cli1.pppg++ <--> ppp2line <--> router1.pppg++;
        router1.pppg++ <--> ppp1line <--> router2.pppg++;
        router2.pppg++ <--> ppp2line <--> srv1.pppg++;
        cli1.pppg++ <--> ppp2line <--> router3.pppg++;
        router3.pppg++ <--> ppp1line <--> router4.pppg++;
        router4.pppg++ <--> ppp2line <--> srv1.pppg++;
}


