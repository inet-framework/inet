//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021q;

import inet.linklayer.contract.IIeee8021qLayer;
import inet.protocolelement.contract.IProtocolLayer;

//
// Implements IEEE 802.1Q protocol functionality as a layered architecture with
// policy and protocol components. The policy submodule handles VLAN filtering
// and mapping, while the protocol submodule manages tag encapsulation/
// decapsulation. Supports both C-VLAN (0x8100) and S-VLAN
// (0x88A8) tag types through configuration. Enables network segmentation and
// traffic control in Ethernet networks by processing VLAN tags according to
// configured policies.
//
module Ieee8021qLayer like IIeee8021qLayer
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
        policy: <default("VlanPolicyLayer")> like IProtocolLayer {
            @display("p=300,150");
        }
        protocol: <default("Ieee8021qProtocol")> like IIeee8021qLayer {
            @display("p=300,300");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> policy.upperLayerIn;
        policy.lowerLayerOut --> protocol.upperLayerIn;
        protocol.lowerLayerOut --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> protocol.lowerLayerIn;
        protocol.upperLayerOut --> policy.lowerLayerIn;
        policy.upperLayerOut --> { @display("m=n"); } --> upperLayerOut;
}
