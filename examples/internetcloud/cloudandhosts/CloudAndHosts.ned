//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//



package inet.examples.internetcloud.cloudandhosts;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.StandardHost;
import inet.node.internetcloud.InternetCloud;
import ned.DatarateChannel;


network CloudAndHosts
{
    parameters:
        int numSenders;

    types:
        channel C extends DatarateChannel
        {
            delay = 10ms;
            datarate = 5Mbps;
        }
    submodules:
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,100;is=s");
        }

        sender[numSenders]: StandardHost {
            @display("p=250,100,row,100");
        }
        recip: StandardHost {
            @display("p=250,500");
        }
        internet: InternetCloud {
            @display("p=250,300;is=vl");
        }
    connections:
        recip.pppg++ <--> C <--> internet.pppg++;
        for i=0..numSenders-1 {
            sender[i].pppg++ <--> C <--> internet.pppg++;
        }
}

