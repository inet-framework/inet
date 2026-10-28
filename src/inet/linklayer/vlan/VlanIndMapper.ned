//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.vlan;

import inet.queueing.base.PacketFlowBase;
import inet.queueing.contract.IPacketFlow;

//
// Updates the `CVlanInd` tag on received packets, per incoming network
// interface. This is the ingress counterpart of ~VlanReqMapper: it assigns the
// VLAN that a received frame belongs to inside the node.
//
// The `mappedVlanIds` parameter maps the name of the incoming network interface
// to a map from the received VLAN id to the VLAN id that the packet gets. The
// received VLAN id is -1 for an untagged frame and 0 for a priority-tagged
// frame. A mapped VLAN id of -1 removes the `CVlanInd` tag. The interface name
// '*' matches every interface that has no entry of its own. Packets from an
// interface without an entry, and packets whose VLAN id has no entry, pass
// unchanged.
//
// For example, `{"eth0" : {"-1" : 10, "0" : 10}}` classifies the untagged and
// priority-tagged frames received on eth0 into VLAN 10, which is the port VLAN
// id (PVID) based classification of IEEE 802.1Q-2018 clause 6.9.1 f.
//
// Classifying untagged frames requires the mapper to see them. In a bridge,
// place it in the `vlanPolicy` submodule of ~BridgingLayer, which sees every
// frame between the relay and the network interfaces. The `policy` submodule
// of ~Ieee8021qLayer sits above the tag checker and only sees frames that
// carried a tag.
//
simple VlanIndMapper extends PacketFlowBase like IPacketFlow
{
    parameters:
        string interfaceTableModule;
        object mappedVlanIds = default({}); // Maps network interface name to another map which maps received VLAN ids to replacement VLAN ids, '*' as interface name matches all interfaces, -1 VLAN id matches if no tag is present, 0 if the frame is priority-tagged
        @class(VlanIndMapper);
        @display("i=block/switch");
}
