//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.linklayer.vlan;

import inet.protocolelement.contract.IProtocolLayer;
import inet.queueing.contract.IPacketFilter;
import inet.queueing.contract.IPacketFlow;

//
// Implements VLAN (Virtual LAN) policy functionality for filtering and mapping
// VLAN tags. Processes both outbound and inbound packets, applying VLAN ID
// mapping and filtering based on configured policies. Supports interface-specific
// VLAN configurations for network segmentation and traffic control in Ethernet
// networks.
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
