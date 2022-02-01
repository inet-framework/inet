//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.internetcloud.cloudandrouters;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.node.internetcloud.InternetCloud;
import ned.DatarateChannel;


network CloudAndRouters
{
    parameters:
        int numSenders;

    types:
        channel C extends DatarateChannel
        {
            delay = 20ms;
            datarate = 5Mbps;
        }
    submodules:
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,70;is=s");
        }

        srouter[numSenders]: Router {
            @display("p=250,200,row,100");
        }
        sender[numSenders]: StandardHost {
            @display("p=250,100,row,100");
        }
        rrouter: Router {
            @display("p=250,600");
        }
        recip: StandardHost {
            @display("p=250,700");
        }
        internet: InternetCloud {
            @display("p=250,400;is=vl");
        }
    connections:
        rrouter.pppg++ <--> C <--> internet.pppg++;
        recip.ethg++ <--> Eth100M <--> rrouter.ethg++;
        for i=0..numSenders-1 {
            srouter[i].pppg++ <--> C <--> internet.pppg++;
            sender[i].ethg++ <--> Eth100M <--> srouter[i].ethg++;
        }
}

