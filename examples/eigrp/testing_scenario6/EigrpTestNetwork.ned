//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
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

// @file EigrpTestNetwork.ned
// @date 27.10.2020
// @author Jan Zavrel (honza.zavrel96@gmail.com)
// @author Jan Bloudicek (jbloudicek@gmail.com)
// @author Vit Rek (rek@kn.vutbr.cz)
// @author Vladimir Vesely (ivesely@fit.vutbr.cz)
// @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3


package inet.examples.eigrp.testing_scenario6;

import inet.node.eigrp.EigrpRouter;
import inet.node.inet.StandardHost;
import inet.node.ethernet.Eth100M;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.networklayer.configurator.ipv6.Ipv6FlatNetworkConfigurator;
import inet.common.scenario.ScenarioManager;

network EigrpTestNetwork
{
    @display("bgb=521,431");
    submodules:
        configurator: Ipv4NetworkConfigurator {
            parameters:
                config = xml("<config></config>");
                assignAddresses = false;
                assignDisjunctSubnetAddresses = false;
                addStaticRoutes = false;
                addDefaultRoutes = false;
                addSubnetRoutes = false;
                optimizeRoutes = false;
                @display("p=450,290");
        }

        R1: EigrpRouter {
            parameters:
                @display("p=250,357");
            gates:
                ethg[3];
        }
        R2: EigrpRouter {
            parameters:
                @display("p=250,207");
            gates:
                ethg[4];
        }
        R3: EigrpRouter {
            parameters:
                @display("p=450,207");
            gates:
                ethg[3];
        }
        R4: EigrpRouter {
            parameters:
                @display("p=50,207");
            gates:
                ethg[2];
        }
        LAN1: StandardHost {
            @display("p=50,357;i=misc/cloud");
        }
        LAN2: StandardHost {
            @display("p=250,57;i=misc/cloud");
        }
        scenarioManager: ScenarioManager {
            @display("p=450,357");
        }
        LAN3: StandardHost {
            @display("p=450,57;i=misc/cloud");
        }
        LAN4: StandardHost {
            @display("p=50,57;i=misc/cloud");
        }
    connections:
        R1.ethg[0] <--> Eth100M <--> R2.ethg[0];
        R1.ethg[1] <--> Eth100M <--> R3.ethg[0];
        R2.ethg[1] <--> Eth100M <--> R3.ethg[1];
        R2.ethg[2] <--> Eth100M <--> R4.ethg[0];

        R1.ethg[2] <--> Eth100M <--> LAN1.ethg++;
        R2.ethg[3] <--> Eth100M <--> LAN2.ethg++;
        R3.ethg[2] <--> Eth100M <--> LAN3.ethg++;
        R4.ethg[1] <--> Eth100M <--> LAN4.ethg++;
}