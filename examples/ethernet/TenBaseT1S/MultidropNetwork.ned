//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.ethernet.TenBaseT1S;

import inet.node.contract.IEthernetNetworkNode;
import inet.node.ethernet.EthernetMultidropLink;
import inet.physicallayer.wired.common.WireJunction;

//
// This module contains a single 10BASE-T1S multidrop link with a separate
// controller node and a configurable number of additional nodes. The network
// node type can be configured for all network nodes on the multidrop link.
//
network MultidropNetwork
{
    parameters:
        int numNodes;
    submodules:
        controller: <> like IEthernetNetworkNode {
            parameters:
                eth[*].plca.plca_node_count = default(parent.numNodes + 1);
                eth[*].plca.local_nodeID = default(0);
                @display("p=150,150;i=device/card");
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
        controller.ethg++ <--> EthernetMultidropLink { length = 100cm; } <--> j[0].port++;
        for i=0..numNodes-1 {
            j[i].port++ <--> EthernetMultidropLink { length = 100cm; } <--> j[i + 1].port++ if i < numNodes - 1;
            j[i].port++ <--> EthernetMultidropLink { length = 50cm; } <--> node[i].ethg++;
        }
}
