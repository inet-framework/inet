//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.vlan;

//
// Implements a comprehensive VLAN policy enforcement layer that processes both
// incoming and outgoing packets. This module combines VLAN mapping and filtering
// capabilities to enforce network segmentation, implement VLAN translation, and
// control VLAN-based traffic flows.
//
// The layer consists of four key components:
// - outboundMapper: Transforms VLAN IDs on outgoing packets (typically VlanReqMapper)
// - outboundFilter: Filters outgoing packets based on VLAN IDs (typically VlanReqFilter)
// - inboundFilter: Filters incoming packets based on VLAN IDs (typically VlanIndFilter)
// - inboundMapper: Optional component for transforming incoming packets' VLAN information
//
// This module is typically inserted between the network layer and the MAC layer
// to enforce VLAN policies before packets are transmitted or after they are received.
//
// @see ~VlanReqMapper, ~VlanReqFilter, ~VlanIndFilter
//
import inet.protocolelement.contract.IProtocolLayer;
import inet.queueing.contract.IPacketFilter;
import inet.queueing.contract.IPacketFlow;

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
