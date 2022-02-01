//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.inet.hierarchical;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100G;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.Eth1G;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;


network Hierarchical
{
    parameters:
        int columns = default(3);
        int rows = default(3);
    types:
        module LAN
        {
            parameters:
                int numHosts = default(3);
                @display("i=misc/cloud");
            gates:
                inout ethg[];
            submodules:
                switch: EthernetSwitch;
                host[numHosts]: StandardHost;
            connections:
                for i=0..numHosts-1 {
                    host[i].ethg++ <--> Eth100M <--> switch.ethg++;
                }
                for i=0..sizeof(ethg) - 1 {
                    switch.ethg++ <--> ethg++;
                }
        }

        module Area
        {
            parameters:
                int numAreas = default(3);
                @display("i=misc/cloud");
            gates:
                inout ethg[];
            submodules:
                lan[numAreas]: LAN;
                router[numAreas]: Router;
            connections:
                for i=0..numAreas-1 {
                    router[i].ethg++ <--> Eth1G <--> router[(i+1)%numAreas].ethg++ if numAreas > 1;
                    router[i].ethg++ <--> Eth1G <--> lan[i].ethg++;
                }
                router[0].ethg++ <--> ethg++;
        }

    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,100;is=s");
            config = xmldoc("hierarchical.xml");
        }
        router[columns*rows]: Router {
            @display("p=250,100,m,$columns,200,200");
        }
        area[columns*rows]: Area;
    connections:
        for i=0..rows-1, for j=0..columns-1 {
            router[i*columns+j].ethg++ <--> Eth100G <--> router[(i+1)*columns+j].ethg++ if i!=rows-1;
            router[i*columns+j].ethg++ <--> Eth100G <--> router[i*columns+j+1].ethg++ if j!=columns-1;
        }
        for i=0..rows*columns-1 {
            router[i].ethg++ <--> Eth100G <--> area[i].ethg++;
        }
}
