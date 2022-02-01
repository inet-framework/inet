//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.inet.hierarchical99;

import inet.networklayer.configurator.contract.IL3NetworkConfigurator;
import inet.networklayer.ipv4.RoutingTableRecorder;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.Eth10G;
import inet.node.ethernet.Eth1G;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;


//
// This hierarchical network topology contains 3 levels, 72 hosts and 27 routers.
//
network Hierarchical99
{
    parameters:
        @display("bgl=22");
    types:
        module LAN
        {
            @display("bgl=12;i=misc/cloud;is=vl");
            gates:
                inout ethg[];
            submodules:
                switch: EthernetSwitch {
                    @display("p=250,100");
                }
                host1: StandardHost {
                    @display("p=100,200;t=10.x.x.1");
                }
                host2: StandardHost {
                    @display("p=250,200;t=10.x.x.2");
                }
                host3: StandardHost {
                    @display("p=400,200;t=10.x.x.3");
                }
            connections:
                host1.ethg++ <--> Eth100M <--> switch.ethg++;
                host2.ethg++ <--> Eth100M <--> switch.ethg++;
                host3.ethg++ <--> Eth100M <--> switch.ethg++;
                for i=0..sizeof(ethg) - 1 {
                    switch.ethg++ <--> ethg++;
                }
        }
        module Area
        {
            @display("bgl=12;bgb=506,453;i=misc/cloud;is=vl");
            gates:
                inout ethg[];
            submodules:
                lan1: LAN {
                    @display("p=400,100;t=10.x.1.x");
                }
                lan2: LAN {
                    @display("p=400,300;t=10.x.2.x");
                }
                lan3: LAN {
                    @display("p=100,300;t=10.x.3.x");
                }
                lan4: LAN {
                    @display("p=100,100;t=10.x.4.x");
                }
                router1: Router {
                    @display("p=250,100");
                }
                router2: Router {
                    @display("p=400,200");
                }
                router3: Router {
                    @display("p=250,300");
                }
                router4: Router {
                    @display("p=100,200");
                }
            connections:
                lan1.ethg++ <--> Eth1G <--> router1.ethg++;
                lan2.ethg++ <--> Eth1G <--> router2.ethg++;
                lan3.ethg++ <--> Eth1G <--> router3.ethg++;
                lan4.ethg++ <--> Eth1G <--> router4.ethg++;
                router1.ethg++ <--> Eth1G <--> router2.ethg++;
                router2.ethg++ <--> Eth1G <--> router3.ethg++;
                router3.ethg++ <--> Eth1G <--> router4.ethg++;
                router4.ethg++ <--> Eth1G <--> router1.ethg++;
                router1.ethg++ <--> ethg++;
        }
    submodules:
        recorder: RoutingTableRecorder {
            @display("p=100,200;is=s");
        }
        networkConfigurator: <> like IL3NetworkConfigurator {
            @display("p=100,100;is=s");
        }
        area11: Area {
            @display("p=400,100;t=10.11.x.x");
        }
        area12: Area {
            @display("p=600,100;t=10.12.x.x");
        }
        area21: Area {
            @display("p=750,300;t=10.21.x.x");
        }
        area22: Area {
            @display("p=600,400;t=10.22.x.x");
        }
        area31: Area {
            @display("p=250,300;t=10.31.x.x");
        }
        area32: Area {
            @display("p=400,400;t=10.32.x.x");
        }
        router1: Router {
            @display("p=500,200");
        }
        router2: Router {
            @display("p=600,300");
        }
        router3: Router {
            @display("p=400,300");
        }
    connections:
        router1.ethg++ <--> Eth10G <--> router2.ethg++;
        router2.ethg++ <--> Eth10G <--> router3.ethg++;
        router3.ethg++ <--> Eth10G <--> router1.ethg++;
        router1.ethg++ <--> Eth10G <--> area11.ethg++;
        router1.ethg++ <--> Eth10G <--> area12.ethg++;
        router2.ethg++ <--> Eth10G <--> area21.ethg++;
        router2.ethg++ <--> Eth10G <--> area22.ethg++;
        router3.ethg++ <--> Eth10G <--> area31.ethg++;
        router3.ethg++ <--> Eth10G <--> area32.ethg++;
}
