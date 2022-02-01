//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.showcases.general.diffserv;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth10M;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import ned.DatarateChannel;

network DiffservNetwork
{
    types:
        channel line extends DatarateChannel
        {
            delay = normal(0.004s, 0.0018s);
            datarate = 128kbps;
        }
    submodules:
        router: Router {
            @display("p=550,150");
        }
        voipPhone1: StandardHost {
            @display("p=250,100;i=misc/voipPhone");
        }
        voipPhone2: StandardHost {
            @display("p=850,100;i=misc/voipPhone");
        }
        client: StandardHost {
            @display("p=250,200");
        }
        server: StandardHost {
            @display("p=850,200");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("is=s;p=100,100");
        }
        etherSwitch: EthernetSwitch {
            @display("p=400,150");
        }
        internet: Router {
            @display("p=700,150;i=misc/cloud_l");
        }
    connections:
        voipPhone1.ethg++ <--> Eth10M <--> etherSwitch.ethg++;
        client.ethg++ <--> Eth10M <--> etherSwitch.ethg++;
        etherSwitch.ethg++ <--> Eth10M <--> router.ethg++;
        router.pppg++ <--> line <--> internet.pppg++;
        internet.pppg++ <--> line <--> server.pppg++;
        internet.pppg++ <--> line <--> voipPhone2.pppg++;
}
