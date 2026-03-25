//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networks;

import inet.networks.base.WiredNetworkBase;
import inet.node.contract.IEthernetNetworkNode;
import inet.common.ILink;

//
// A parameterizable 2D torus network topology.
//
// Like a 2D grid but with wraparound links on all edges,
// giving every node exactly 4 neighbors.
// Commonly used in HPC interconnects (e.g. IBM Blue Gene, Cray).
//
// <pre>
//   [0,0]---[0,1]---[0,2]---...-+
//     |       |       |         |  (wraparound)
//   [1,0]---[1,1]---[1,2]---...-+
//     |       |       |         |
//    ...     ...     ...       ...
//     |       |       |         |
//     +-------+-------+---...-[rows-1,cols-1]
//   (wraparound)
// </pre>
//
// Node at (r,c) is stored at index r*cols + c.
//
network TorusNetwork extends WiredNetworkBase
{
    parameters:
        int rows; // number of rows
        int cols; // number of columns
        volatile int numHostsPerSwitch = default(0); // number of hosts connected to each switch
        object numHosts = default(repeatPar("numHostsPerSwitch", rows * cols));
    submodules:
        node[rows * cols]: <default("EthernetSwitch")> like IEthernetNetworkNode {
            @display("p=300,100,matrix,$cols,120,120");
        }
        host[sum(numHosts)]: <default("StandardHost")> like IEthernetNetworkNode {
        }
    connections:
        // Horizontal links (including wraparound)
        for r=0..rows-1, for c=0..cols-1 {
            node[r*cols+c].ethg++ <--> <default("EthernetLink")> like ILink <--> node[r*cols + (c+1) % cols].ethg++;
        }
        // Vertical links (including wraparound)
        for r=0..rows-1, for c=0..cols-1 {
            node[r*cols+c].ethg++ <--> <default("EthernetLink")> like ILink <--> node[((r+1) % rows)*cols + c].ethg++;
        }
        for i=0..sizeof(host)-1 {
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node[groupIndex(numHosts,i)].ethg++;
        }
}

//
// Small 3x3 torus: 9 nodes, 18 links.
//
network SmallTorusNetwork extends TorusNetwork
{
    parameters:
        rows = default(3);
        cols = default(3);
}

//
// Medium 10x10 torus: 100 nodes, 200 links.
//
network MediumTorusNetwork extends TorusNetwork
{
    parameters:
        rows = default(10);
        cols = default(10);
}

//
// Large 32x32 torus: 1024 nodes, 2048 links.
//
network LargeTorusNetwork extends TorusNetwork
{
    parameters:
        rows = default(32);
        cols = default(32);
}
