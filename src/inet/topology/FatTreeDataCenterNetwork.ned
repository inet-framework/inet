//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.topology;

//
// Fat-tree topology pre-configured with typical datacenter parameters.
//
// Default links: 25 Gbps / 2 m access, 100 Gbps / 10 m edge-aggr, 100 Gbps / 20 m aggr-core.
// Size: k=4 -> 4 core, 8 aggr, 8 edge switches, 16 hosts.
//
network FatTreeDataCenterNetwork extends FatTreeNetwork
{
    parameters:
        k = default(4);
        *.host[*].eth[*].channel.datarate = default(25Gbps); // access
        *.host[*].eth[*].channel.length = default(2m); // within rack
        *.aggr[*].eth[*].channel.datarate = default(100Gbps); // edge-aggr
        *.aggr[*].eth[*].channel.length = default(10m); // across rows
        *.core[*].eth[*].channel.datarate = default(100Gbps); // aggr-core
        *.core[*].eth[*].channel.length = default(20m); // across DC floor
}

//
// Small datacenter fat-tree: k=4 -> 16 hosts.
// 10 Gbps / 1 m access, 40 Gbps / 5 m edge-aggr, 40 Gbps / 10 m aggr-core.
//
network SmallFatTreeDataCenterNetwork extends FatTreeNetwork
{
    parameters:
        k = default(4);
        *.host[*].eth[*].channel.datarate = default(10Gbps); // access
        *.host[*].eth[*].channel.length = default(1m); // within rack
        *.aggr[*].eth[*].channel.datarate = default(40Gbps); // edge-aggr
        *.aggr[*].eth[*].channel.length = default(5m); // across rows
        *.core[*].eth[*].channel.datarate = default(40Gbps); // aggr-core
        *.core[*].eth[*].channel.length = default(10m); // across DC floor
}

//
// Large datacenter fat-tree: k=8 -> 128 hosts.
// 100 Gbps / 3 m access, 400 Gbps / 15 m edge-aggr, 400 Gbps / 50 m aggr-core.
//
network LargeFatTreeDataCenterNetwork extends FatTreeNetwork
{
    parameters:
        k = default(8);
        *.host[*].eth[*].channel.datarate = default(100Gbps); // access
        *.host[*].eth[*].channel.length = default(3m); // within rack
        *.aggr[*].eth[*].channel.datarate = default(400Gbps); // edge-aggr
        *.aggr[*].eth[*].channel.length = default(15m); // across rows
        *.core[*].eth[*].channel.datarate = default(400Gbps); // aggr-core
        *.core[*].eth[*].channel.length = default(50m); // across DC floor
}
