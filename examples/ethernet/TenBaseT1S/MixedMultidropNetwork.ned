//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.ethernet.TenBaseT1S;

import inet.node.contract.IEthernetNetworkNode;
import inet.node.ethernet.Ethernet10BaseT1sCable;
import inet.node.ethernet.EthernetLink;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.StandardHost;
import inet.physicallayer.wired.common.WireJunction;

//
// This module contains an ~EthernetSwitch connected to a ~StandardHost and a
// 10BASE-T1S multidrop link with a configurable number of nodes. The switch
// port acts as the controller of the multidrop link. The network node type can
// be configured for all the nodes on the multidrop link.
//
network MixedMultidropNetwork
{
    parameters:
        int numNodes;
    submodules:
        switch: EthernetSwitch {
            parameters:
                eth[0].plca.plca_node_count = default(parent.numNodes + 1);
                eth[0].plca.local_nodeID = default(0);
                @display("p=150,150");
        }
        host: StandardHost {
            parameters:
                @display("p=150,300;i=device/device");
        }
        node[numNodes]: <> like IEthernetNetworkNode {
            parameters:
                eth[*].plca.plca_node_count = default(parent.numNodes + 1);
                eth[*].plca.local_nodeID = default(this.index + 1);
                @display("p=300,300,row,150;i=device/card");
        }
        j[numNodes]: WireJunction {
            parameters:
                @display("p=300,150,row,150");
        }
    connections:
        switch.ethg++ <--> Ethernet10BaseT1sCable { length = 100cm; } <--> j[0].port++;
        switch.ethg++ <--> EthernetLink { length = 50cm; datarate = 100Mbps; } <--> host.ethg++;
        for i=0..numNodes-1 {
            j[i].port++ <--> Ethernet10BaseT1sCable { length = 100cm; } <--> j[i + 1].port++ if i < numNodes - 1;
            j[i].port++ <--> Ethernet10BaseT1sCable { length = 50cm; } <--> node[i].ethg++;
        }
}
