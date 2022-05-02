//
// Copyright (C) 2008 Irene Ruengeler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//

package inet.test.moduletest.lib;

import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import ned.DatarateChannel;


network multihomed
{
    parameters:
        double testTimeout @unit(s) = default(0s);
        bool testing = default(false);
    types:
      channel BottlePath extends DatarateChannel
      {
        parameters:
          datarate = 1Mbps;
          per = 0.01;
      }

    channel NormalPath extends DatarateChannel
    {
        parameters:
            datarate = 1Gbps;
    }
    submodules:
        sctp_client: StandardHost {
            parameters:
                ipv4.routingTable.routingFile = "../../lib/multi_client.mrt";
                ipv4.configurator.networkConfiguratorModule = "";
            gates:
                pppg[2];
        }
        sctp_server: StandardHost {
            parameters:
                ipv4.routingTable.routingFile = "../../lib/multi_server.mrt";
                ipv4.configurator.networkConfiguratorModule = "";
            gates:
                pppg[2];

        }
        router1: Router {
            parameters:
                ipv4.routingTable.routingFile = "../../lib/multi_router1.mrt";
                ipv4.configurator.networkConfiguratorModule = "";
            gates:
                pppg[4];
        }
        router2: Router {
            parameters:
                ipv4.routingTable.routingFile = "../../lib/multi_router2.mrt";
                ipv4.configurator.networkConfiguratorModule = "";
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

