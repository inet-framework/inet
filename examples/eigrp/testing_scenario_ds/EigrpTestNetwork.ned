//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// @file EigrpTestNetwork.ned
// @date 27.10.2020
// @author Jan Zavrel (honza.zavrel96@gmail.com)
// @author Jan Bloudicek (jbloudicek@gmail.com)
// @author Vit Rek (rek@kn.vutbr.cz)
// @author Vladimir Vesely (ivesely@fit.vutbr.cz)
// @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3

package inet.examples.eigrp.testing_scenario_ds;

import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.eigrp.EigrpRouter;
import inet.node.ethernet.Eth100M;
import inet.node.inet.StandardHost;


network EigrpTestNetwork
{
    @display("bgb=521,431");
    submodules:
        configurator: Ipv4NetworkConfigurator {
            parameters:
                config = xml("<config>"+
                            "<interface among='R1 R2' address='10.0.12.x' netmask='255.255.255.252' />"+
                            "<interface among='R2 R3' address='10.0.23.x' netmask='255.255.255.252' />"+
                            "<interface among='R1 R3' address='10.0.13.x' netmask='255.255.255.252' />"+
                            "<interface among='R2 R4' address='10.0.24.x' netmask='255.255.255.252' />"+
                            "<interface among='R1 LAN1' address='10.0.1.x' netmask='255.255.255.0' />"+
                            "<interface among='R2 LAN2' address='10.0.2.x' netmask='255.255.255.0' />"+
                            "<interface among='R3 LAN3' address='10.0.3.x' netmask='255.255.255.0' />"+
                            "<interface among='R4 LAN4' address='10.0.4.x' netmask='255.255.255.0' />"+
                            "</config>");
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

