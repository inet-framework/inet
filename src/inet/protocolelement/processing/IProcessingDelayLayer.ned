//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.processing;

//
// Defines the interface for protocol layers that model processing delays
// in network devices. These delays represent the time required for packet
// processing operations in real hardware.
//
// Operation:
// - Introduces configurable delays for both incoming and outgoing packets
// - Processes packets in both directions (upper to lower and lower to upper)
// - Maintains separate delay components for each direction
// - Preserves packet order within each processing path
// - Can be inserted at any point in a protocol stack
//
// @see ~ProcessingDelayLayer, ~OmittedProcessingDelayLayer
//
moduleinterface IProcessingDelayLayer
{
    parameters:
        @omittedTypename(OmittedProcessingDelayLayer);
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
}
