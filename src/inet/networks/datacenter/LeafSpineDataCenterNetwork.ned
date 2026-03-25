//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networks.datacenter;

import inet.topology.LeafSpineNetwork;

//
// Leaf-spine topology pre-configured with typical datacenter parameters.
//
// Default links: 25 Gbps / 2 m access, 100 Gbps / 10 m fabric.
// Sizes: 2 spines, 4 leaves, 4 hosts/leaf = 16 hosts.
//
network LeafSpineDataCenterNetwork extends LeafSpineNetwork
{
    parameters:
        numSpines = default(2);
        numLeaves = default(4);
        numHostsPerLeaf = default(4);
        *.host[*].eth[*].channel.datarate = default(25Gbps); // access
        *.host[*].eth[*].channel.length = default(2m); // within rack
        *.spine[*].eth[*].channel.datarate = default(100Gbps); // fabric
        *.spine[*].eth[*].channel.length = default(10m); // across rack rows
}

//
// Small datacenter leaf-spine: 2 spines, 2 leaves, 2 hosts/leaf = 4 hosts.
// 10 Gbps / 1 m access, 40 Gbps / 5 m fabric.
//
network SmallLeafSpineDataCenterNetwork extends LeafSpineNetwork
{
    parameters:
        numSpines = default(2);
        numLeaves = default(2);
        numHostsPerLeaf = default(2);
        *.host[*].eth[*].channel.datarate = default(10Gbps); // access
        *.host[*].eth[*].channel.length = default(1m); // within rack
        *.spine[*].eth[*].channel.datarate = default(40Gbps); // fabric
        *.spine[*].eth[*].channel.length = default(5m); // across rack rows
}

//
// Large datacenter leaf-spine: 4 spines, 8 leaves, 8 hosts/leaf = 64 hosts.
// 100 Gbps / 3 m access, 400 Gbps / 30 m fabric.
//
network LargeLeafSpineDataCenterNetwork extends LeafSpineNetwork
{
    parameters:
        numSpines = default(4);
        numLeaves = default(8);
        numHostsPerLeaf = default(8);
        *.host[*].eth[*].channel.datarate = default(100Gbps); // access
        *.host[*].eth[*].channel.length = default(3m); // within rack
        *.spine[*].eth[*].channel.datarate = default(400Gbps); // fabric
        *.spine[*].eth[*].channel.length = default(30m); // across rack rows
}
