//
// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

// @file finalTest.ned
// @date 21.10.2011
// @author: Tomas Prochazka (mailto:xproch21@stud.fit.vutbr.cz), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
// @brief Topology with PIM network

package inet.examples.pim.dm.basic;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.inet.MulticastRouter;
import inet.node.inet.StandardHost;


network pimDMFinal
{
    submodules:
        R1: MulticastRouter {
            @display("p=425,190");
        }
        R2: MulticastRouter {
            @display("p=275,280");
        }
        R3: MulticastRouter {
            @display("p=575,280");
        }

        Source1: StandardHost {
            @display("p=425,100");
        }
        Host1: StandardHost {
            @display("p=200,370;i=device/pc");
        }
        Source2: StandardHost {
            @display("p=500,370");
        }

        Host2: StandardHost {
            @display("p=350,370;i=device/pc");
        }
        Host3: StandardHost {
            @display("p=650,370;i=device/pc");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,100;is=s");
        }
    //        scenarioManager: ScenarioManager {
    //            @display("p=376,26");
    //            script = xmldoc("scenario.xml");
    //        }
    connections allowunconnected:
        R1.ethg++ <--> Eth100M <--> R2.ethg++;
        R1.ethg++ <--> Eth100M <--> R3.ethg++;
        R1.ethg++ <--> Eth100M <--> Source1.ethg++;

        R2.ethg++ <--> Eth100M <--> Host1.ethg++;
        R2.ethg++ <--> Eth100M <--> Host2.ethg++;

        R3.ethg++ <--> Eth100M <--> Source2.ethg++;
        R3.ethg++ <--> Eth100M <--> Host3.ethg++;
}
