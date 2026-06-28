//
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.lisp.registration;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.inet.StandardHost;

//
// An ETR registers its EID-to-RLOC mappings with a Map-Server (LISP control
// plane, UDP 4342).
//
network RegNetwork
{
    @display("bgb=260,100");
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=50,40");
        }
        xtr: StandardHost {
            numApps = 1;
            @display("p=80,80");
        }
        ms: StandardHost {
            numApps = 1;
            @display("p=200,80");
        }
    connections:
        xtr.ethg++ <--> Eth100M <--> ms.ethg++;
}
