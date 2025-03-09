//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.redundancy;

import inet.protocolelement.contract.IProtocolLayer;
import inet.queueing.contract.IPacketFlow;

//
// Wraps a stream identifier module into a protocol layer for use in layered
// network architectures. Provides stream identification capabilities while
// maintaining compatibility with the protocol layer interface.
//
// Operation:
// - Processes outgoing packets through the stream identifier
// - Attaches stream identification tags based on packet content
// - Passes incoming packets directly to upper layers
// - Maintains separate processing paths for upward and downward packet flows
// - Integrates stream identification into layered protocol stacks
//
// @see ~StreamIdentifier, ~StreamCoderLayer, ~StreamRelayLayer
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
