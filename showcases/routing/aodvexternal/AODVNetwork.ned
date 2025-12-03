//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.showcases.routing.aodvexternal;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.networklayer.ipv4.RoutingTableRecorder;
import inet.node.aodv.AodvRouter;
import inet.node.ethernet.Eth1G;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.StandardHost;
import inet.physicallayer.wireless.common.medium.UnitDiskRadioMedium;
import inet.visualizer.contract.IIntegratedVisualizer;


//
// AODV network with an external Ethernet-connected host that can receive
// traffic from the wireless AODV network. The router acts as the gateway
// between the wireless AODV network and the wired Ethernet network.
//
network AODVNetwork
{
    parameters:
        int numHosts;
        @display("bgb=700,700");
    submodules:
        visualizer: <default("IntegratedCanvasVisualizer")> like IIntegratedVisualizer {
            parameters:
                @display("p=50,150;is=s");
        }
        radioMedium: UnitDiskRadioMedium {
            parameters:
                @display("p=50,200;is=s");
        }
        routingTableRecorder: RoutingTableRecorder {
            parameters:
                @display("p=50,250;is=s");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                config = default(xml("<config><interface hosts='*' address='145.236.x.x' netmask='255.255.0.0'/></config>"));
                @display("p=50,300;is=s");
        }
        host[numHosts]: AodvRouter {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070");
        }
        router: AodvRouter {
            @display("p=450,150;i=device/wifilaptop");
        }
        ethernetSwitch: EthernetSwitch {
            @display("p=350,50");
        }
        ethernetHost: StandardHost {
            @display("p=50,50;i=device/server");
        }
    connections:
        router.ethg++ <--> Eth1G <--> ethernetSwitch.ethg++;
        ethernetHost.ethg++ <--> Eth1G <--> ethernetSwitch.ethg++;
}
