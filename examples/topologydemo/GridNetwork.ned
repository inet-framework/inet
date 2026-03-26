//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.topologydemo;

import inet.networks.base.WiredNetworkBase;
import inet.node.contract.IEthernetNetworkNode;
import inet.node.ethernet.EthernetLink;

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
    submodules:
        node[rows * cols]: <default("EthernetSwitch")> like IEthernetNetworkNode {
            @display("p=100,100,matrix,$cols,120,120");
        }
    connections:
        // Horizontal links
        for r=0..rows-1, for c=0..cols-2 {
            node[r*cols+c].ethg++ <--> EthernetLink <--> node[r*cols+c+1].ethg++;
        }
        // Vertical links
        for r=0..rows-2, for c=0..cols-1 {
            node[r*cols+c].ethg++ <--> EthernetLink <--> node[(r+1)*cols+c].ethg++;
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
// Medium 4x4 grid: 16 nodes, 24 links.
//
network MediumGridNetwork extends GridNetwork
{
    parameters:
        rows = default(4);
        cols = default(4);
}

//
// Large 8x8 grid: 64 nodes, 112 links.
//
network LargeGridNetwork extends GridNetwork
{
    parameters:
        rows = default(8);
        cols = default(8);
}
