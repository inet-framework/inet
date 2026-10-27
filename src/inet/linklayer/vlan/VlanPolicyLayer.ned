//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.vlan;

import inet.protocolelement.contract.IProtocolLayer;
import inet.queueing.contract.IPacketFilter;
import inet.queueing.contract.IPacketFlow;

//
// Applies the VLAN policy of a node to the packets that pass between its
// upper layers and its network interfaces.
//
// Outbound, the `outboundMapper` (~VlanReqMapper) updates the `VlanReq` tag per
// outgoing interface, for example to tag or untag the frame, and then the
// `outboundFilter` (~VlanReqFilter) drops the packets whose resulting VLAN id
// the outgoing interface does not accept. Inbound, the `inboundFilter`
// (~VlanIndFilter) drops the packets whose received VLAN id the incoming
// interface does not accept, and then the optional `inboundMapper`
// (~VlanIndMapper) updates the `VlanInd` tag per incoming interface, for
// example to classify untagged frames into the VLAN of the port. Both filters
// thus check the VLAN id as it is on the wire: -1 for an untagged frame, 0 for
// a priority-tagged frame.
//
// The layer sees the packets of the slot where it is placed. In
// ~BridgingLayer, as `vlanPolicy`, it sees every frame between the relay and
// the network interfaces, tagged or not, so it can classify untagged frames.
// In ~Ieee8021qLayer, as `policy`, it sits above the 802.1Q protocol and sees
// only frames that are tagged or are about to be tagged.
//
module VlanPolicyLayer like IProtocolLayer
{
    parameters:
        @display("i=block/layer");
        string interfaceTableModule;
        *.interfaceTableModule = default(absPath(this.interfaceTableModule));
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        outboundMapper: <default("VlanReqMapper")> like IPacketFlow {
            @display("p=200,150");
        }
        outboundFilter: <default("VlanReqFilter")> like IPacketFilter {
            @display("p=200,300");
        }
        inboundMapper: <default("")> like IPacketFlow {
            @display("p=500,150");
        }
        inboundFilter: <default("VlanIndFilter")> like IPacketFilter {
            @display("p=500,300");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> outboundMapper.in;
        outboundMapper.out --> outboundFilter.in;
        outboundFilter.out --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> inboundFilter.in;
        inboundFilter.out --> inboundMapper.in;
        inboundMapper.out --> { @display("m=n"); } --> upperLayerOut;
}
