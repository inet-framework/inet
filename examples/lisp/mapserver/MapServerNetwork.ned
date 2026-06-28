//
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.lisp.mapserver;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;

//
// Dynamic LISP: two xTRs with no static map-cache resolve remote EIDs through a
// Map-Server/Map-Resolver. Each xTR registers its EID site, then on a data-plane
// cache miss queries the Map-Resolver, caches the reply and encapsulates.
//
//   host1 --- xtr1 --\
//                     switch --- ms (Map-Server/Resolver)
//   host2 --- xtr2 --/
//
network MapServerNetwork
{
    @display("bgb=420,260");
    submodules:
        configurator: Ipv4NetworkConfigurator { @display("p=50,30"); }
        host1: StandardHost { @display("p=40,90"); }
        xtr1: Router { numApps = 1; numTunInterfaces = 1; @display("p=150,90"); }
        host2: StandardHost { @display("p=40,200"); }
        xtr2: Router { numApps = 1; numTunInterfaces = 1; @display("p=150,200"); }
        switch: EthernetSwitch { @display("p=270,145"); }
        ms: Router { numApps = 1; @display("p=380,145"); }
    connections:
        host1.ethg++ <--> Eth100M <--> xtr1.ethg++;     // xtr1 eth0 = EID LAN
        xtr1.ethg++ <--> Eth100M <--> switch.ethg++;    // xtr1 eth1 = RLOC
        host2.ethg++ <--> Eth100M <--> xtr2.ethg++;     // xtr2 eth0 = EID LAN
        xtr2.ethg++ <--> Eth100M <--> switch.ethg++;    // xtr2 eth1 = RLOC
        ms.ethg++ <--> Eth100M <--> switch.ethg++;      // ms eth0 = RLOC
}
