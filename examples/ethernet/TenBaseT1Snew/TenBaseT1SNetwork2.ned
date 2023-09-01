//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.ethernet.TenBaseT1Snew;

import inet.node.ethernet.EthernetLink;
import inet.physicallayer.wired.common.WireJunction;

network TenBaseT1SNetwork2
{
    parameters:
        int numNodes;
    submodules:
        controller: TenBaseT1SNode {
            parameters:
                eth.plca.plca_node_count = parent.numNodes + 1;
                eth.plca.local_nodeID = 0;
                @display("p=150,150");
        }
        node[numNodes]: TenBaseT1SNode {
            parameters:
                eth.plca.plca_node_count = parent.numNodes + 1;
                eth.plca.local_nodeID = this.index + 1;
                @display("p=300,300,row,150");
        }
        j[numNodes]: WireJunction {
            parameters:
                @display("p=300,150,row,150");
        }
    connections:
        controller.ethg <--> EthernetLink {  length = 100cm; datarate = 10Mbps; } <--> j[0].port++;
        for i=0..numNodes-1 {
            j[i].port++ <--> EthernetLink {  length = 100cm; datarate = 10Mbps; } <--> j[i + 1].port++ if i < numNodes - 1;
            j[i].port++ <--> EthernetLink {  length = 50cm; datarate = 10Mbps; } <--> node[i].ethg;
        }
}
