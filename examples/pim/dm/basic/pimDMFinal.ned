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

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.EtherLink;
import inet.node.inet.MulticastRouter;
import inet.node.inet.StandardHost;


network pimDMFinal
{
    parameters:
        *.eth[*].bitrate = 100 Mbps;
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
        R1.ethg++ <--> EtherLink <--> R2.ethg++;
        R1.ethg++ <--> EtherLink <--> R3.ethg++;
        R1.ethg++ <--> EtherLink <--> Source1.ethg++;

        R2.ethg++ <--> EtherLink <--> Host1.ethg++;
        R2.ethg++ <--> EtherLink <--> Host2.ethg++;

        R3.ethg++ <--> EtherLink <--> Source2.ethg++;
        R3.ethg++ <--> EtherLink <--> Host3.ethg++;
}
