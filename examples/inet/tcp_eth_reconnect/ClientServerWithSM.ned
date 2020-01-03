//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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

package inet.examples.inet.tcp_eth_reconnect;

import inet.common.misc.NetAnimTrace;
import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.StandardHost;
import inet.visualizer.integrated.IntegratedVisualizer;
import ned.DatarateChannel;


network ClientServerWithSM
{
    types:
        channel C extends DatarateChannel
        {
            datarate = 10Mbps;
            delay = 0.1us;
        }
    submodules:
        visualizer: IntegratedVisualizer {
            @display("p=50,50");
        }
        client1: StandardHost {
            parameters:
                @display("p=250,200;i=device/pc3");
        }
        server: StandardHost {
            parameters:
                @display("p=400,200;i=device/pc2");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,100;is=s");
        }
        netAnimTrace: NetAnimTrace {
            @display("p=100,200;is=s");
        }
        scenarioManager: ScenarioManager {
            @display("p=100,300;is=s");
        }
    connections:
        client1.ethg++ <--> C <--> server.ethg++;
}

