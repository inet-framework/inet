// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
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
// @file finalTest.ned
// @date 21.10.2011
// @author: Tomas Prochazka (mailto:xproch21@stud.fit.vutbr.cz), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
// @brief Topology with PIM network

package inet.examples.pim.dm.basic;

import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.inet.MulticastRouter;
import inet.node.inet.StandardHost;


network pimDMFinal
{
    @display("bgb=448,352,white");

    submodules:
        R1: MulticastRouter {
            @display("p=208,102");
        }
        R2: MulticastRouter {
            @display("p=143,163");
        }
        R3: MulticastRouter {
            @display("p=276,163");
        }

        Source1: StandardHost {
            @display("p=208,33");
        }
        Host1: StandardHost {
            @display("p=98,235;i=device/pc");
        }
        Source2: StandardHost {
            @display("p=242,235");
        }

        Host2: StandardHost {
            @display("p=177,235;i=device/pc");
        }
        Host3: StandardHost {
            @display("p=317,235;i=device/pc");
        }
        configurator: IPv4NetworkConfigurator {
            @display("p=44,41");
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
