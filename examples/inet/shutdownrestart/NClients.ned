//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//





package inet.examples.inet.shutdownrestart;

import inet.common.misc.ThruputMeteringChannel;
import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.EthernetSwitch;
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
        r1: Router {
            @display("p=500,150");
        }
        r2: Router {
            @display("p=650,150");
        }
        r3: Router {
            @display("p=800,150");
        }
        s: EthernetSwitch {
            @display("p=350,150");
        }
        cli[n]: StandardHost {
            @display("i=device/laptop;p=250,100,col,100");
        }
        srv: StandardHost {
            @display("p=950,150;i=device/server");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,100;is=s");
        }
        scenarioManager: ScenarioManager {
            @display("p=100,200;is=s");
        }
    connections:
        for i=0..n-1 {
            cli[i].ethg++ <--> ethernetline <--> s.ethg++;
        }
        s.ethg++ <--> ethernetline <--> r1.ethg++;
        r1.pppg++ <--> ethernetline <--> r2.pppg++;
        r2.pppg++ <--> ethernetline <--> r3.pppg++;
        r3.pppg++ <--> ethernetline <--> srv.pppg++;
}

