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

package inet.examples.sctp.cmttest;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import ned.DatarateChannel;


network multihomed
{
    parameters:
        double testTimeout @unit(s) = default(0s);
        bool testing = default(false);

    submodules:
        sctp_client: StandardHost {
            parameters:
                forwarding = false;
                ipv4.configurator.networkConfiguratorModule = "";
                @display("p=100,200;i=device/laptop");
            gates:
                pppg[2];
        }
        sctp_server: StandardHost {
            parameters:
                forwarding = false;
                ipv4.configurator.networkConfiguratorModule = "";
                @display("p=450,200;i=device/server2");
            gates:
                pppg[2];

        }
        router1: Router {
            parameters:
                ipv4.configurator.networkConfiguratorModule = "";
                @display("p=200,100;i=abstract/router");
            gates:
                pppg[4];
        }
        router2: Router {
            parameters:
                ipv4.configurator.networkConfiguratorModule = "";
                @display("p=350,100;i=abstract/router");
            gates:
                pppg[4];
        }
    connections:
        sctp_client.pppg[0] <--> NormalPath <--> router1.pppg[0];
        sctp_client.pppg[1] <--> NormalPath <--> router1.pppg[1];
        router1.pppg[2] <--> BottlePath <--> router2.pppg[0];
        router1.pppg[3] <--> BottlePath <--> router2.pppg[1];
        router2.pppg[2] <--> NormalPath <--> sctp_server.pppg[0];
        router2.pppg[3] <--> NormalPath <--> sctp_server.pppg[1];
}

channel BottlePath extends DatarateChannel
{
    parameters:
        delay = 0s;
        datarate = 1Mbps;
        per = 0.005;
}

channel NormalPath extends DatarateChannel
{
    parameters:
        delay = 0ms;
        datarate = 1Gbps;
}
