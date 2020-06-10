//
// Copyright (C) 2004 Emin Ilker Cetinbas (niw3@yahoo.com)
//
// This library is free software, you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//



package inet.examples.ethernet.arptest2;

import inet.linklayer.ethernet.EtherBus;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import ned.TransmissionChannel;


network ARPTest
{
    parameters:
        *.eth[*].bitrate = 10 Mbps;
    types:
        channel ethline extends TransmissionChannel
        {
            delay = 500ns;
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
        bus1: EtherBus {
            positions = "";
            propagationSpeed = 2e8 mps;
            @display("p=475,300;b=500,5");
        }
        bus2: EtherBus {
            positions = "";
            propagationSpeed = 2e8 mps;
            @display("p=475,100;b=350,5");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,100;is=s");
        }
    connections:
        //half-duplex
        bus1.ethg++ <--> ethline <--> router1.ethg[0];
        bus1.ethg++ <--> ethline <--> router2.ethg[0];
        bus1.ethg++ <--> ethline <--> router3.ethg[0];
        bus1.ethg++ <--> ethline <--> router4.ethg[0];
        bus1.ethg++ <--> ethline <--> router5.ethg[0];
        bus2.ethg++ <--> ethline <--> router5.ethg[1];
        bus2.ethg++ <--> ethline <--> router6.ethg[0];
        bus2.ethg++ <--> ethline <--> router7.ethg[0];

        //full-duplex
        client.ethg++ <--> ethline <--> router1.ethg++;
        server.ethg++ <--> ethline <--> router8.ethg++;
        router8.ethg++ <--> ethline <--> router7.ethg++;
}

