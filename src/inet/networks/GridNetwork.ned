//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networks;

import inet.networks.base.WiredNetworkBase;
import inet.node.contract.IEthernetNetworkNode;
import inet.common.ILink;

//
// A parameterizable 2D grid (mesh) network topology.
//
// Nodes are arranged in a rows x cols grid. Each node connects to its
// horizontal and vertical neighbors (no diagonal, no wraparound).
// Commonly used in on-chip networks (NoC) and sensor grids.
//
// <pre>
//   [0,0]---[0,1]---[0,2]--- ... ---[0,cols-1]
//     |       |       |                  |
//   [1,0]---[1,1]---[1,2]--- ... ---[1,cols-1]
//     |       |       |                  |
//    ...     ...     ...                ...
//     |       |       |                  |
//   [rows-1,0] --- ... ---[rows-1,cols-1]
// </pre>
//
// Node at (r,c) is stored at index r*cols + c.
//
network GridNetwork extends WiredNetworkBase
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
        // Horizontal links
        for r=0..rows-1, for c=0..cols-2 {
            node[r*cols+c].ethg++ <--> <default("EthernetLink")> like ILink <--> node[r*cols+c+1].ethg++;
        }
        // Vertical links
        for r=0..rows-2, for c=0..cols-1 {
            node[r*cols+c].ethg++ <--> <default("EthernetLink")> like ILink <--> node[(r+1)*cols+c].ethg++;
        }
        for i=0..sizeof(host)-1 {
            host[i].ethg++ <--> <default("EthernetLink")> like ILink <--> node[groupIndex(numHosts,i)].ethg++;
        }
}

//
// Small 3x3 grid: 9 nodes, 12 links.
//
network SmallGridNetwork extends GridNetwork
{
    parameters:
        rows = default(3);
        cols = default(3);
}

//
// Medium 10x10 grid: 100 nodes, 180 links.
//
network MediumGridNetwork extends GridNetwork
{
    parameters:
        rows = default(10);
        cols = default(10);
}

//
// Large 32x32 grid: 1024 nodes, 1984 links.
//
network LargeGridNetwork extends GridNetwork
{
    parameters:
        rows = default(32);
        cols = default(32);
}
