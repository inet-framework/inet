//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.redundancy;

import inet.protocolelement.contract.IProtocolLayer;
import inet.queueing.contract.IPacketFlow;

//
// This module wraps a stream identifier into a protocol layer so that it can
// be used in a layered compound module.
//
module StreamIdentifierLayer like IProtocolLayer
{
    parameters:
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        identifier: <default("StreamIdentifier")> like IPacketFlow if typename != "" {
            parameters:
                @display("p=200,150;i=block/blackboard");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> identifier.in;
        identifier.out --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> upperLayerOut;
}
