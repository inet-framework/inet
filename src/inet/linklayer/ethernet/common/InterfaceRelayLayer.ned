//
// Copyright (C) 2021 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.common;

import inet.protocolelement.contract.IProtocolLayer;
import inet.queueing.contract.IPacketFlow;

//
// Implements a key component of Ethernet switching that handles both MAC address
// learning and interface selection in a single layer. This module is essential
// for the proper functioning of bridges and switches, enabling them to forward
// frames to the correct destination ports.
//
// The InterfaceRelayLayer consists of two main components:
// - Learner: Examines incoming frames and updates the MAC forwarding table with
//   mappings between source MAC addresses and the interfaces they were received on.
//   This implements the "learning" aspect of learning bridges.
// - Selector: Determines the appropriate outgoing interface(s) for frames based on
//   their destination MAC addresses by consulting the MAC forwarding table.
//   This implements the "forwarding" aspect of learning bridges.
//
// The module works with a MAC forwarding table that maintains the mappings between
// MAC addresses and interfaces. For unicast frames with known destinations, the
// selector forwards the frame only to the specific interface. For unknown destinations
// or broadcast/multicast addresses, the selector typically floods the frame to all
// interfaces except the one it was received on.
//
// @see ~MacForwardingTable, ~RelayInterfaceLearner, ~RelayInterfaceSelector, ~BridgingLayer
//
module InterfaceRelayLayer like IProtocolLayer
{
    parameters:
        string macTableModule;
        string interfaceTableModule;
        *.macTableModule = default(absPath(this.macTableModule));
        *.interfaceTableModule = default(absPath(this.interfaceTableModule));
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        learner: <default("RelayInterfaceLearner")> like IPacketFlow {
            @display("p=200,150");
        }
        selector: <default("RelayInterfaceSelector")> like IPacketFlow {
            @display("p=500,150");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> selector.in;
        selector.out --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> learner.in;
        learner.out --> { @display("m=n"); } --> upperLayerOut;
}
