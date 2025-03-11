//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.fragmentation.contract;

//
// Defines the interface for packet fragmentation policy modules that determine
// how packets should be split into fragments. Fragmentation policies provide
// the rules and parameters for dividing packets into smaller units.
//
// Operation:
// - Determines fragment sizes based on configurable parameters
// - Provides fragmentation rules to fragmenter modules
// - Calculates optimal fragment boundaries
// - Ensures fragments meet size constraints
// - Maintains consistency in fragmentation decisions
//
// Particularly useful for:
// - Implementing protocol-specific fragmentation algorithms
// - Adapting packet sizes to network constraints
// - Creating custom fragmentation strategies
// - Supporting different fragmentation schemes in the same simulation
// - Separating fragmentation logic from packet processing
//
// @see ~LengthBasedFragmenterPolicy, ~IPacketFragmenter
//
moduleinterface IFragmenterPolicy
{
    parameters:
        @display("i=block/star");
}
