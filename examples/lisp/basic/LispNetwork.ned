//
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.lisp.basic;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;

//
// Two LISP xTRs connect two EID sites (192.168.1.0/24 and 192.168.2.0/24) across
// an RLOC core (10.0.0.0/24). Each host uses its site router as gateway; traffic
// to the remote EID site is LISP-encapsulated over the RLOC link.
//
//   host1 --- xtr1 ===(RLOC)=== xtr2 --- host2
//
network LispNetwork
{
    @display("bgb=520,160");
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=50,40");
        }
        host1: StandardHost {
            @display("p=60,110");
        }
        xtr1: Router {
            numApps = 1;
            numTunInterfaces = 1;
            @display("p=180,110");
        }
        xtr2: Router {
            numApps = 1;
            numTunInterfaces = 1;
            @display("p=340,110");
        }
        host2: StandardHost {
            @display("p=460,110");
        }
    connections:
        host1.ethg++ <--> Eth100M <--> xtr1.ethg++;
        xtr1.ethg++ <--> Eth100M <--> xtr2.ethg++;
        xtr2.ethg++ <--> Eth100M <--> host2.ethg++;
}
