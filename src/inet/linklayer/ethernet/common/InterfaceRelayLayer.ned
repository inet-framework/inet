//
// Copyright (C) 2021 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.common;

import inet.protocolelement.contract.IProtocolLayer;
import inet.queueing.contract.IPacketFlow;

//
// This module combines the interface MAC address learning from incoming packets
// with the outgoing interface selection for outgoing packets into a single layer.
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
