//
// Protocol Test Framework for INET -- Phase 9 PLCA (10BASE-T1S) demo network.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.protocoltest;

import inet.node.ethernet.EthernetPlcaHost;
import inet.node.ethernet.EthernetMultidropLink;
import inet.physicallayer.wired.common.WireJunction;

//
// An Ethernet 10BASE-T1S multidrop bus: a controller (PLCA local_nodeID 0, which
// generates the periodic BEACON) and `numNodes` data nodes, connected through a chain
// of WireJunctions by EthernetMultidropLink segments. Every interface runs the
// EthernetPlca control + data state machines; the ProtocolTester observes their
// state-change signals (controlStateChanged / dataStateChanged / curID / rxCmd / txCmd).
//
// All nodes share plca_node_count = numNodes + 1 and get a unique local_nodeID; this is
// the same wiring INET's examples/ethernet/TenBaseT1S/MultidropNetwork uses.
//
network PlcaMultidropDemo
{
    parameters:
        int numNodes;
    submodules:
        tester: ProtocolTester {
            @display("p=100,100");
        }
        controller: EthernetPlcaHost {
            eth[*].plca.plca_node_count = default(parent.numNodes + 1);
            eth[*].plca.local_nodeID = default(0);
            @display("p=250,100;i=device/card");
        }
        node[numNodes]: EthernetPlcaHost {
            eth[*].plca.plca_node_count = default(parent.numNodes + 1);
            eth[*].plca.local_nodeID = default(this.index + 1);
            @display("p=400,220,row,160;i=device/card");
        }
        j[numNodes]: WireJunction {
            @display("p=400,100,row,160");
        }
    connections:
        controller.ethg++ <--> EthernetMultidropLink { length = 100cm; } <--> j[0].port++;
        for i=0..numNodes-1 {
            j[i].port++ <--> EthernetMultidropLink { length = 100cm; } <--> j[i + 1].port++ if i < numNodes - 1;
            j[i].port++ <--> EthernetMultidropLink { length = 50cm; } <--> node[i].ethg++;
        }
}
