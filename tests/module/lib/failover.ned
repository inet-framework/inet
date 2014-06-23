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

import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import ned.DatarateChannel;
import inet.common.scenario.ScenarioManager;


network failover
{
    parameters:
        double testTimeout @unit(s) = default(0s);
        bool testing = default(false);
    types:
      channel BottlePath extends DatarateChannel
      {
        parameters:
          datarate = 1Mbps;
	      per = 0.005;
      }

	channel NormalPath extends DatarateChannel
	{
		parameters:
			datarate = 1Gbps;
	}
    submodules:
        sctp_client: StandardHost {
            parameters:
                routingTable.routingFile = "../../lib/multi_client.mrt";
                networkLayer.configurator.networkConfiguratorModule = "";
            gates:
                pppg[2];
        }
        sctp_server: StandardHost {
            parameters:
                routingTable.routingFile = "../../lib/multi_server.mrt";
                networkLayer.configurator.networkConfiguratorModule = "";
            gates:
                pppg[2];

        }
        router1: Router {
            parameters:
                routingTable.routingFile = "../../lib/multi_router1.mrt";
                networkLayer.configurator.networkConfiguratorModule = "";
            gates:
                pppg[4];
        }
        router2: Router {
            parameters:
                routingTable.routingFile = "../../lib/multi_router2.mrt";
                networkLayer.configurator.networkConfiguratorModule = "";
            gates:
                pppg[4];
        }
        scenarioManager: ScenarioManager {
            @display("p=184,180");
        }
    connections:
        sctp_client.pppg[0] <--> NormalPath <--> router1.pppg[0];
        sctp_client.pppg[1] <--> NormalPath <--> router1.pppg[1];
        router1.pppg[2] <--> BottlePath <--> router2.pppg[0];
        router1.pppg[3] <--> BottlePath <--> router2.pppg[1];
        router2.pppg[2] <--> NormalPath <--> sctp_server.pppg[0];
        router2.pppg[3] <--> NormalPath <--> sctp_server.pppg[1];
}

