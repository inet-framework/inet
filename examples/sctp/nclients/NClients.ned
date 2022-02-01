//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//





package inet.examples.sctp.nclients;

import inet.common.misc.ThruputMeteringChannel;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;


network NClients
{
    parameters:
        int n;
    types:
        channel fiberline extends ThruputMeteringChannel
        {
            delay = 1us;
            datarate = 512Mbps;
            thruputDisplayFormat = "u";
        }
        channel ethernetline extends ThruputMeteringChannel
        {
            delay = 0.1us;
            datarate = 10Mbps;
            thruputDisplayFormat = "u";
        }
    submodules:
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,100;is=s");
        }
        r1: Router {
            parameters:
                @display("p=400,100");
        }
        r2: Router {
            parameters:
                @display("p=550,100");
        }
        r3: Router {
            parameters:
                @display("p=700,100");
        }
        cli[n]: StandardHost {
            parameters:
                @display("i=device/laptop;p=250,100,col,100");
        }
        srv: StandardHost {
            parameters:
                @display("p=850,100;i=device/server");
        }
    connections:
        for i=0..n-1 {
            cli[i].pppg++ <--> ethernetline <--> r1.pppg++;
        }
        r1.pppg++ <--> ethernetline <--> r2.pppg++;
        r2.pppg++ <--> ethernetline <--> r3.pppg++;
        r3.pppg++ <--> ethernetline <--> srv.pppg++;
}

