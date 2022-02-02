//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//

package inet.examples.inet.routerperf;

import inet.common.misc.ThruputMeteringChannel;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.Router;


network RouterPerfNetwork
{
    parameters:
        int nodeNo;
    types:
        channel C extends ThruputMeteringChannel
        {
            thruputDisplayFormat = "u";
        }
    submodules:
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,100;is=s");
        }
        sender[nodeNo]: BurstHost {
            parameters:
                @display("p=250,300,row,150");
        }
        recip[nodeNo]: BurstHost {
            parameters:
                @display("p=250,100,row,150");
        }
        router: Router {
            parameters:
                @display("p=400,200");
        }
    connections:
        for i=0..nodeNo-1 {
            sender[i].pppg++ <--> C {  datarate = 1Mbps; } <--> router.pppg++;
            recip[i].pppg++ <--> C {  datarate = 1Mbps; } <--> router.pppg++;
        }
}

