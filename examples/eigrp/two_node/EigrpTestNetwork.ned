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


package inet.examples.eigrp.two_node;

import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.eigrp.EigrpRouter;
import inet.node.ethernet.Eth100M;
import inet.node.inet.StandardHost;


network EigrpTestNetwork
{
    @display("bgb=540,254");
    submodules:
        configurator: Ipv4NetworkConfigurator {
            parameters:
                config = xml("<config>"+
                            "<interface among='R1 R2' address='10.0.12.x' netmask='255.255.255.252' />"+
                            "<interface among='R1 LAN1' address='10.0.1.x' netmask='255.255.255.0' />"+
                            "<interface among='R2 LAN2' address='10.0.2.x' netmask='255.255.255.0' />"+
                            "</config>");
                addStaticRoutes = false;
                addDefaultRoutes = false;
                addSubnetRoutes = false;
                optimizeRoutes = false;
                @display("p=29,213");
        }

        R1: EigrpRouter {
            parameters:
                @display("p=67,38");
            gates:
                ethg[2];
        }
        R2: EigrpRouter {
            parameters:
                @display("p=296,38");
            gates:
                ethg[2];
        }
        LAN1: StandardHost {
            @display("p=67,151;i=misc/cloud");
        }
        LAN2: StandardHost {
            @display("p=296,151;i=misc/cloud");
        }
        scenarioManager: ScenarioManager {
            @display("p=117,213");
        }
    connections:
        R1.ethg[0] <--> Eth100M <--> R2.ethg[0];
        R1.ethg[1] <--> Eth100M <--> LAN1.ethg++;
        R2.ethg[1] <--> Eth100M <--> LAN2.ethg++;
}

