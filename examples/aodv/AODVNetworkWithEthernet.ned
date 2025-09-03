//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.aodv;

import inet.node.ethernet.Eth1G;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.visualizer.contract.IIntegratedVisualizer;


//
// Extended AODV network with an additional Ethernet-connected host
// that can receive traffic from the wireless AODV network.
//
network AODVNetworkWithEthernet extends AODVNetwork
{
    submodules:
        visualizer: <default("IntegratedCanvasVisualizer")> like IIntegratedVisualizer {
            parameters:
                @display("p=100,150;is=s");
        }
        router: Router {
            @display("p=500,150");
        }
        ethernetSwitch: EthernetSwitch {
            @display("p=600,300");
        }
        ethernetHost: StandardHost {
            @display("p=500,400;i=device/server");
        }
    connections:
        host[0].ethg++ <--> Eth1G <--> router.ethg++;
        router.ethg++ <--> Eth1G <--> ethernetSwitch.ethg++;
        ethernetHost.ethg++ <--> Eth1G <--> ethernetSwitch.ethg++;
}
