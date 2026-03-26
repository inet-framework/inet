//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.topology;

import inet.common.ILink;
import inet.networks.base.WiredNetworkBase;
import inet.node.contract.IEthernetNetworkNode;

//
// A parameterizable dragonfly network topology.
//
// The dragonfly (Kim et al., 2008) organizes switches into groups
// with three levels of links:
//
//  - Local links: full mesh among switches within each group
//  - Global links: one link between each pair of groups
//  - Host links: hosts attached to each switch
//
// Parameters:
//  - numGroups (g): number of groups
//  - numSwitchesPerGroup (a): switches per group
//  - numHostsPerSwitch (p): hosts per switch
//
// Requires: numGroups - 1 <= numSwitchesPerGroup (each switch handles
// at most one global link per remote group).
//
// <pre>
//   Group 0                    Group 1                    Group g-1
//  ,---------.              ,---------.              ,---------.
//  |[sw0]-[sw1]|--global-->|[sw0]-[sw1]|   ...     |[sw0]-[sw1]|
//  | |  X  | | |           | |  X  | | |           | |  X  | | |
//  |[sw2]..  | |           |[sw2]..  | |           |[sw2]..  | |
//  '-|---|---'             '-|---|---'             '-|---|---'
//   [h][h]                  [h][h]                  [h][h]
// </pre>
//
network DragonflyNetwork extends WiredNetworkBase
{
    parameters:
        int numGroups; // number of groups (g)
        int numSwitchesPerGroup; // switches per group (a), must be >= numGroups-1
        int numHostsPerSwitch; // hosts per switch (p)
    submodules:
        sw[numGroups * numSwitchesPerGroup]: <default("EthernetSwitch")> like IEthernetNetworkNode {
            @display("p=300,200,matrix,$numSwitchesPerGroup,120,120");
        }
        host[numGroups * numSwitchesPerGroup * numHostsPerSwitch]: <default("StandardHost")> like IEthernetNetworkNode {
            @display("p=300,500,row,60");
        }
    connections:
        // Local links: full mesh within each group
        for g=0..numGroups-1, for i=0..numSwitchesPerGroup-2, for j=i+1..numSwitchesPerGroup-1 {
            sw[g*numSwitchesPerGroup+i].ethg++ <--> <default("EthernetLink")> like ILink <--> sw[g*numSwitchesPerGroup+j].ethg++;
        }
        // Global links: connect group g1 to group g2 (g1 < g2)
        // Group g1 uses switch (g2-1) and group g2 uses switch g1
        for g1=0..numGroups-2, for g2=g1+1..numGroups-1 {
            sw[g1*numSwitchesPerGroup + (g2-1)].ethg++ <--> <default("EthernetLink")> like ILink <--> sw[g2*numSwitchesPerGroup + g1].ethg++;
        }
        // Host links
        for s=0..numGroups*numSwitchesPerGroup-1, for h=0..numHostsPerSwitch-1 {
            sw[s].ethg++ <--> <default("EthernetLink")> like ILink <--> host[s*numHostsPerSwitch+h].ethg++;
        }
}

//
// Small dragonfly: 4 groups, 4 switches/group, 2 hosts/switch = 32 hosts.
//
network SmallDragonflyNetwork extends DragonflyNetwork
{
    parameters:
        numGroups = default(4);
        numSwitchesPerGroup = default(4);
        numHostsPerSwitch = default(2);
}

//
// Medium dragonfly: 8 groups, 8 switches/group, 4 hosts/switch = 256 hosts.
//
network MediumDragonflyNetwork extends DragonflyNetwork
{
    parameters:
        numGroups = default(8);
        numSwitchesPerGroup = default(8);
        numHostsPerSwitch = default(4);
}

//
// Large dragonfly: 16 groups, 16 switches/group, 4 hosts/switch = 1024 hosts.
//
network LargeDragonflyNetwork extends DragonflyNetwork
{
    parameters:
        numGroups = default(16);
        numSwitchesPerGroup = default(16);
        numHostsPerSwitch = default(4);
}
