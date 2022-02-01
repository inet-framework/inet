//
// Copyright (C) 2004 Emin Ilker Cetinbas (niw3@yahoo.com)
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.ethernet.arptest2;

import inet.node.ethernet.EthernetHub;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth10M;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;


network ARPTest
{
    types:
        channel ethline extends Eth10M
        {
        }
    submodules:
        client: StandardHost {
            @display("p=100,400;i=device/laptop");
        }
        server: StandardHost {
            @display("p=925,200;i=device/server");
        }
        router1: Router {
            numEthInterfaces = 1;
            eth[0].duplexMode = false;
            @display("p=250,400");
        }
        router2: Router {
            numEthInterfaces = 1;
            eth[0].duplexMode = false;
            @display("p=400,400");
        }
        router3: Router {
            numEthInterfaces = 1;
            eth[0].duplexMode = false;
            @display("p=550,400");
        }
        router4: Router {
            numEthInterfaces = 1;
            eth[0].duplexMode = false;
            @display("p=700,400");
        }
        router5: Router {
            numEthInterfaces = 2;
            eth[0..1].duplexMode = false;
            @display("p=475,200");
        }
        router6: Router {
            numEthInterfaces = 1;
            eth[0].duplexMode = false;
            @display("p=325,200");
        }
        router7: Router {
            numEthInterfaces = 1;
            eth[0].duplexMode = false;
            @display("p=625,200");
        }
        router8: Router {
            numEthInterfaces = 1;
            @display("p=775,200");
        }
        hub1: EthernetHub {
            @display("p=475,300;b=500,5");
        }
        hub2: EthernetHub {
            @display("p=475,100;b=350,5");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,100;is=s");
        }
    connections:
        //half-duplex
        hub1.ethg++ <--> ethline <--> router1.ethg[0];
        hub1.ethg++ <--> ethline <--> router2.ethg[0];
        hub1.ethg++ <--> ethline <--> router3.ethg[0];
        hub1.ethg++ <--> ethline <--> router4.ethg[0];
        hub1.ethg++ <--> ethline <--> router5.ethg[0];
        hub2.ethg++ <--> ethline <--> router5.ethg[1];
        hub2.ethg++ <--> ethline <--> router6.ethg[0];
        hub2.ethg++ <--> ethline <--> router7.ethg[0];

        //full-duplex
        client.ethg++ <--> ethline <--> router1.ethg++;
        server.ethg++ <--> ethline <--> router8.ethg++;
        router8.ethg++ <--> ethline <--> router7.ethg++;
}

