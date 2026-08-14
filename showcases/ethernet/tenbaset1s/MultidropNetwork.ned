//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.showcases.ethernet.tenbaset1s;

import inet.node.contract.IEthernetNetworkNode;
import inet.node.ethernet.EthernetMultidropLink;
import inet.physicallayer.wired.common.WireJunction;

//
// This network contains a single 10BASE-T1S multidrop link (mixing segment)
// with a zonal controller node and a configurable number of edge nodes. The
// controller is PLCA node 0; edge node i is PLCA node i+1.
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
                @display("p=300,300,row,100;i=device/card");
        }
        j[numNodes]: WireJunction {
            parameters:
                @display("p=300,150,row,100");
        }
    connections:
        controller.ethg++ <--> EthernetMultidropLink { length = 100cm; } <--> j[0].port++;
        for i=0..numNodes-1 {
            j[i].port++ <--> EthernetMultidropLink { length = 100cm; } <--> j[i + 1].port++ if i < numNodes - 1;
            j[i].port++ <--> EthernetMultidropLink { length = 10cm; } <--> node[i].ethg++;
        }
}
